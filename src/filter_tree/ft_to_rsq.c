/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "ft_to_rsq.h"
#include "RG.h"
#include "../util/arr.h"
#include "filter_tree_utils.h"
#include "../datatypes/point.h"
#include "../datatypes/array.h"
#include "../util/range/string_range.h"
#include "../util/range/numeric_range.h"

//------------------------------------------------------------------------------
// To RediSearch query node
//------------------------------------------------------------------------------

// create a RediSearch query node out of a numeric range object
RSQNode *_NumericRangeToQueryNode(RSIndex *idx, const char *field, const NumericRange *range) {
	double max = (range->max == INFINITY) ? RSRANGE_INF : range->max;
	double min = (range->min == -INFINITY) ? RSRANGE_NEG_INF : range->min;
	return RediSearch_CreateNumericNode(idx, field, max, min, range->include_max, range->include_min);
}

// create a RediSearch query node out of a string range object
RSQNode *_StringRangeToQueryNode(RSIndex *idx, const char *field, const StringRange *range) {
	const char *max = (range->max == NULL) ? RSLECRANGE_INF : range->max;
	const char *min = (range->min == NULL) ? RSLEXRANGE_NEG_INF : range->min;
	RSQNode *root = RediSearch_CreateTagNode(idx, field);
	RSQNode *child = RediSearch_CreateLexRangeNode(idx, field, min, max,
												   range->include_min,
												   range->include_max);
	RediSearch_QueryNodeAddChild(root, child);
	return root;
}

RSQNode *_filterTreeToDistanceQueryNode(FT_FilterNode *filter, RSIndex *idx) {
	char     *field  =  NULL;         // field being filtered
	SIValue  origin  =  SI_NullVal(); // center of circle
	SIValue  radius  =  SI_NullVal(); // circle radius

	_extractOriginAndRadius(filter, &origin, &radius, &field);

	return RediSearch_CreateGeoNode(idx, field, Point_lat(origin),
									Point_lon(origin), SI_GET_NUMERIC(radius), RS_GEO_DISTANCE_M);
}

// creates a RediSearch query node out of given IN filter
static RSQNode *_filterTreeToInQueryNode(FT_FilterNode *filter, RSIndex *idx) {
	ASSERT(_isInFilter(filter));

	// n.v IN [1,2,3]
	// a single union node should hold a number of token/numeric nodes
	// one for each element in the array.

	// extract both field name and list from expression
	AR_ExpNode *inOp = filter->exp.exp;

	char *field;
	bool attribute = AR_EXP_IsAttribute(inOp->op.children[0], &field);
	ASSERT(attribute == true);

	SIValue list = inOp->op.children[1]->operand.constant;
	uint list_len = SIArray_Length(list);

	if(list_len == 0) {
		// Special case: "WHERE a.v in []"
		return RediSearch_CreateEmptyNode(idx);
	}

	RSQNode *node = NULL;
	RSQNode *parent = NULL;
	RSQNode *U = RediSearch_CreateUnionNode(idx);

	for(uint i = 0; i < list_len; i ++) {
		double d;
		SIValue v = SIArray_Get(list, i);
		switch(SI_TYPE(v)) {
		case T_STRING:
			parent = RediSearch_CreateTagNode(idx, field);
			node = RediSearch_CreateTokenNode(idx, field, v.stringval);
			RediSearch_QueryNodeAddChild(parent, node);
			node = parent;
			break;
		case T_DOUBLE:
		case T_INT64:
		case T_BOOL:
			d = SI_GET_NUMERIC(v);
			node = RediSearch_CreateNumericNode(idx, field, d, d, true, true);
			break;
		default:
			ASSERT(false && "unexpected conditional operation");
			break;
		}
		RediSearch_QueryNodeAddChild(U, node);
	}

	return U;
}

// reduce filter into a range object
// return true if filter was reduce, false otherwise
bool _predicateTreeToRange(const FT_FilterNode *tree, rax *string_ranges,
		rax *numeric_ranges) {
	// simple predicate trees are used to build up a range object
	ASSERT(AR_EXP_IsConstant(tree->pred.rhs));

	char *prop;
	if(!AR_EXP_IsAttribute(tree->pred.lhs, &prop)) return false;

	int op = tree->pred.op;
	SIValue c = tree->pred.rhs->operand.constant;

	uint prop_len = strlen(prop);
	StringRange *sr = raxFind(string_ranges, (unsigned char *)prop, prop_len);
	NumericRange *nr = raxFind(numeric_ranges, (unsigned char *)prop, prop_len);

	// get or create range object for alias.prop
	if(SI_TYPE(c) & SI_NUMERIC || SI_TYPE(c) == T_BOOL) {
		// create if doesn't exists
		if(nr == raxNotFound) {
			nr = NumericRange_New();
			raxTryInsert(numeric_ranges, (unsigned char *)prop, prop_len,
					nr, NULL);
		}
		NumericRange_TightenRange(nr, op, SI_GET_NUMERIC(c));
	} else if(SI_TYPE(c) == T_STRING) {
		// create if doesn't exists
		if(sr == raxNotFound) {
			sr = StringRange_New();
			raxTryInsert(string_ranges, (unsigned char *)prop, prop_len, sr, NULL);
		}
		StringRange_TightenRange(sr, op, c.stringval);
	} else {
		ASSERT(false);
	}
	return true;
}

static RSQNode *_concat_query_nodes(RSIndex *idx, RSQNode **nodes) {
	// connect all RediSearch query nodes
	uint count = array_len(nodes);

	// no way to utilize the index
	if(count == 0) return RediSearch_CreateEmptyNode(idx);

	// just a single filter
	if(count == 1) return array_pop(nodes);

	// multiple filters, combine using AND
	RSQNode *root = RediSearch_CreateIntersectNode(idx, false);
	for(uint i = 0; i < count; i++) {
		RSQNode *qnode = array_pop(nodes);
		RediSearch_QueryNodeAddChild(root, qnode);
	}

	return root;
}

RSQNode *_ranges_to_query_nodes(RSIndex *idx, rax *string_ranges,
		rax *numeric_ranges) {
	ASSERT(string_ranges != NULL);
	ASSERT(numeric_ranges != NULL);
	RSQNode **rsqnodes = array_new(RSQNode*, 0);

	// build RediSearch query tree
	// convert each range object to RediSearch query node
	raxIterator it;
	raxStart(&it, string_ranges);
	raxSeek(&it, "^", NULL, 0);
	char query_field_name[1024];
	while(raxNext(&it)) {
		char *field = (char *)it.key;

		/* make sure each property is bound to either numeric or string type
		 * but not to both, e.g. a.v = 1 AND a.v = 'a'
		 * in which case use an empty RSQueryNode. */
		if(raxFind(numeric_ranges, (unsigned char *)field, (int)it.key_len) != raxNotFound) {
			goto cleanup;
		}

		StringRange *sr = raxFind(string_ranges, (unsigned char *)field, (int)it.key_len);
		if(!StringRange_IsValid(sr)) goto cleanup;

		sprintf(query_field_name, "%.*s", (int)it.key_len, field);
		RSQNode *rsqn = _StringRangeToQueryNode(idx, query_field_name, sr);
		rsqnodes = array_append(rsqnodes, rsqn);
	}
	raxStop(&it);

	raxStart(&it, numeric_ranges);
	raxSeek(&it, "^", NULL, 0);
	while(raxNext(&it)) {
		char *field = (char *)it.key;
		NumericRange *nr = raxFind(numeric_ranges, (unsigned char *)field, (int)it.key_len);

		// return empty RSQueryNode.
		if(!NumericRange_IsValid(nr)) goto cleanup;

		sprintf(query_field_name, "%.*s", (int)it.key_len, field);
		RSQNode *rsqn = _NumericRangeToQueryNode(idx, query_field_name, nr);
		rsqnodes = array_append(rsqnodes, rsqn);
	}
	raxStop(&it);

	RSQNode *root = _concat_query_nodes(idx, rsqnodes);
	array_free(rsqnodes);
	return root;

cleanup:
	// encountered invalid range, free constructed query nodes
	// return empty query node
	for(uint i = 0; i < array_len(rsqnodes); i++) {
		RediSearch_QueryNodeFree(rsqnodes[i]);
	}
	array_free(rsqnodes);
	return RediSearch_CreateEmptyNode(idx);
}

// reduce filters into ranges
// we differentiate between numeric filters and string filters
void _compose_ranges(FT_FilterNode **trees, rax *string_ranges,
		rax *numeric_ranges) {
	uint count = array_len(trees);
	for(uint i = 0; i < count; i++) {
		FT_FilterNode *tree = trees[i];
		if(tree->t == FT_N_PRED) {
			if(_predicateTreeToRange(tree, string_ranges, numeric_ranges)) {
				FilterTree_Free(tree);
				array_del_fast(trees, i);
				i--;
				count--;
			}
		}
	}
}

static RSQNode *_FilterTreeToQueryNode(FT_FilterNode *tree, RSIndex *idx) {
	RSQNode *node   = NULL;
	RSQNode *parent = NULL;

	if(_isInFilter(tree)) {
		return _filterTreeToInQueryNode(tree, idx);
	}

	if(_isDistanceFilter(tree)) {
		return _filterTreeToDistanceQueryNode(tree, idx);
	}

	switch(tree->t) {
		case FT_N_COND: {
			RSQNode *left = NULL;
			RSQNode *right = NULL;
			switch(tree->cond.op) {
				case OP_OR:
					node = RediSearch_CreateUnionNode(idx);
					break;
				case OP_AND:
					node = RediSearch_CreateIntersectNode(idx, false);
					break;
				default:
					ASSERT(false && "unexpected conditional operation");
					break;
			}

			left = _FilterTreeToQueryNode(tree->cond.left, idx);
			right = _FilterTreeToQueryNode(tree->cond.right, idx);
			RediSearch_QueryNodeAddChild(node, left);
			RediSearch_QueryNodeAddChild(node, right);
			break;
		}
		case FT_N_PRED: {
			double d;
			char *field;
			// expecting left hand side to be an attribute access
			bool attribute = AR_EXP_IsAttribute(tree->pred.lhs, &field);
			ASSERT(attribute == true);

			SIValue v = tree->pred.rhs->operand.constant;
			switch(SI_TYPE(v)) {
				case T_STRING:
					parent = RediSearch_CreateTagNode(idx, field);
					switch(tree->pred.op) {
						case OP_LT:    // <
							node = RediSearch_CreateLexRangeNode(idx, field, RSLEXRANGE_NEG_INF, v.stringval, 0, 0);
							break;
						case OP_LE:    // <=
							node = RediSearch_CreateLexRangeNode(idx, field, RSLEXRANGE_NEG_INF, v.stringval, 0, 1);
							break;
						case OP_GT:    // >
							node = RediSearch_CreateLexRangeNode(idx, field, v.stringval, RSLECRANGE_INF, 0, 0);
							break;
						case OP_GE:    // >=
							node = RediSearch_CreateLexRangeNode(idx, field, v.stringval, RSLECRANGE_INF, 1, 0);
							break;
						case OP_EQUAL:  // ==
							node = RediSearch_CreateTokenNode(idx, field, v.stringval);
							break;
						case OP_NEQUAL: // !=
							ASSERT(false && "Index can't utilize the 'not equals' operation.");
							break;
						default:
							ASSERT(false && "unexpected operation");
							break;
					}

					RediSearch_QueryNodeAddChild(parent, node);
					node = parent;
					break;

				case T_DOUBLE:
				case T_INT64:
				case T_BOOL:
					d = SI_GET_NUMERIC(v);
					switch(tree->pred.op) {
						case OP_LT:    // <
							node = RediSearch_CreateNumericNode(idx, field, d, RSRANGE_NEG_INF, false, false);
							break;
						case OP_LE:    // <=
							node = RediSearch_CreateNumericNode(idx, field, d, RSRANGE_NEG_INF, true, false);
							break;
						case OP_GT:    // >
							node = RediSearch_CreateNumericNode(idx, field, RSRANGE_INF, d, false, false);
							break;
						case OP_GE:    // >=
							node = RediSearch_CreateNumericNode(idx, field, RSRANGE_INF, d, false, true);
							break;
						case OP_EQUAL:  // ==
							node = RediSearch_CreateNumericNode(idx, field, d, d, true, true);
							break;
						case OP_NEQUAL: // !=
							ASSERT(false && "Index can't utilize the 'not equals' operation.");
							break;
						default:
							ASSERT(false && "unexpected operation");
							break;
					}
					break;
				default:
					ASSERT(false && "unexpected value type");
				break;
			}

			break;
		}
		case FT_N_EXP: {
			node = _filterTreeToInQueryNode(tree, idx);
			break;
		}
		default: {
			ASSERT("unknown filter tree node type");
			break;
		}
	}
	return node;
}

// creates a RediSearch query node out of given filter tree
RSQNode *FilterTreeToQueryNode(const FT_FilterNode *tree, RSIndex *idx) {
	ASSERT(idx != NULL);
	ASSERT(tree != NULL);

	FT_FilterNode *t = FilterTree_Clone(tree);
	RSQNode **nodes = array_new(RSQNode*, 1);

	// break down tree to individual subtrees
	FT_FilterNode **trees = FilterTree_SubTrees(t);

	//--------------------------------------------------------------------------
	// convert filters to numeric and string ranges
	//--------------------------------------------------------------------------

	rax *string_ranges  = raxNew();
	rax *numeric_ranges = raxNew();
	_compose_ranges(trees, string_ranges, numeric_ranges);
	if(raxSize(string_ranges) > 0 || raxSize(numeric_ranges) > 0) {
		RSQNode *ranges = _ranges_to_query_nodes(idx, string_ranges, numeric_ranges);
		// TODO: check for empty node RediSearch_CreateEmptyNode
		nodes = array_append(nodes, ranges);
	}

	//--------------------------------------------------------------------------
	// convert remaining filters into RediSearch query nodes
	//--------------------------------------------------------------------------

	uint tree_count = array_len(trees);
	for(uint i = 0; i < tree_count; i++) {
		nodes = array_append(nodes, _FilterTreeToQueryNode(trees[i], idx));
		FilterTree_Free(trees[i]);
	}

	// compose root query node by intersecting individual query nodes
	RSQNode *root = _concat_query_nodes(idx, nodes);

	array_free(nodes);
	array_free(trees);
	raxFreeWithCallback(string_ranges, (void(*)(void *))StringRange_Free);
	raxFreeWithCallback(numeric_ranges, (void(*)(void *))NumericRange_Free);

	return root;
}

