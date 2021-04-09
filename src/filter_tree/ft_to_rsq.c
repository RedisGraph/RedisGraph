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
// forward declarations
//------------------------------------------------------------------------------

static bool _FilterTreeToQueryNode(RSQNode**root, FT_FilterNode *tree,
		RSIndex *idx);

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

RSQNode *_FilterTreeToDistanceQueryNode(FT_FilterNode *filter, RSIndex *idx) {
	char     *field  =  NULL;         // field being filtered
	SIValue  origin  =  SI_NullVal(); // center of circle
	SIValue  radius  =  SI_NullVal(); // circle radius

	_extractOriginAndRadius(filter, &origin, &radius, &field);

	return RediSearch_CreateGeoNode(idx, field, Point_lat(origin),
									Point_lon(origin), SI_GET_NUMERIC(radius), RS_GEO_DISTANCE_M);
}

// creates a RediSearch query node out of given IN filter
static RSQNode *_FilterTreeToInQueryNode(FT_FilterNode *filter, RSIndex *idx) {
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

	SIValue c = tree->pred.rhs->operand.constant;
	SIType  t = SI_TYPE(c);

	// make sure constant is an indexable type
	if(!(t & SI_INDEXABLE)) return false;

	int           op        =  tree->pred.op;
	StringRange   *sr       =  NULL;
	NumericRange  *nr       =  NULL;
	uint          prop_len  =  strlen(prop);

	// get or create range object for alias.prop
	// constant is either numeric or boolean
	if(t & SI_NUMERIC || t == T_BOOL) {
		nr = raxFind(numeric_ranges, (unsigned char *)prop, prop_len);
		// create if doesn't exists
		if(nr == raxNotFound) {
			nr = NumericRange_New();
			raxTryInsert(numeric_ranges, (unsigned char *)prop, prop_len,
					nr, NULL);
		}
		NumericRange_TightenRange(nr, op, SI_GET_NUMERIC(c));
	} else if(t == T_STRING) {
		sr = raxFind(string_ranges, (unsigned char *)prop, prop_len);
		// create if doesn't exists
		if(sr == raxNotFound) {
			sr = StringRange_New();
			raxTryInsert(string_ranges, (unsigned char *)prop, prop_len, sr, NULL);
		}
		StringRange_TightenRange(sr, op, c.stringval);
	}

	return true;
}

// connect all RediSearch query nodes
static RSQNode *_concat_query_nodes(RSIndex *idx, RSQNode **nodes, uint count) {
	// no nodes, can not utilize the index
	if(count == 0) return NULL;

	// just a single filter
	if(count == 1) return array_pop(nodes);

	// multiple filters, combine using AND
	RSQNode *root = RediSearch_CreateIntersectNode(idx, false);
	for(uint i = 0; i < count; i++) {
		RSQNode *qnode = nodes[i];
		RediSearch_QueryNodeAddChild(root, qnode);
	}

	return root;
}

RSQNode *_ranges_to_query_nodes(RSIndex *idx, rax *string_ranges,
		rax *numeric_ranges) {
	ASSERT(string_ranges != NULL);
	ASSERT(numeric_ranges != NULL);

	// build RediSearch query tree
	// convert each range object to RediSearch query node
	raxIterator it;
	bool valid = true;  // false if there's a range conflict
	char query_field_name[1024];

	//--------------------------------------------------------------------------
	// validate ranges
	//--------------------------------------------------------------------------
	
	// validate string ranges
	raxStart(&it, string_ranges);
	raxSeek(&it, "^", NULL, 0);

	while(raxNext(&it)) {
		/* make sure each property is bound to either numeric or string type
		 * but not to both, e.g. a.v = 1 AND a.v = 'a'
		 * in which case use an empty RSQueryNode. */
		if(raxFind(numeric_ranges, (unsigned char *)field, (int)it.key_len) != raxNotFound) {
			valid = false;
			break;
		}

		StringRange *sr = (StringRange *) it.data;
		if(!StringRange_IsValid(sr)) {
			valid = false;
			break;
		}
	}
	raxStop(&it);
	
	if(valid == false) return RediSearch_CreateEmptyNode(idx);

	// validate numeric ranges
	raxStart(&it, numeric_ranges);
	raxSeek(&it, "^", NULL, 0);

	while(raxNext(&it)) {
		NumericRange *nr = (NumericRange *) it.data;
		if(!NumericRange_IsValid(nr)) {
			valid = false;
			break;
		}
	}
	raxStop(&it);

	if(valid == false) return RediSearch_CreateEmptyNode(idx);

	//--------------------------------------------------------------------------
	// construct index range queries
	//--------------------------------------------------------------------------

	// detemine number of ranges
	uint idx = 0;
	uint range_count = raxSize(numeric_ranges) + raxSize(string_ranges);
	RSQNode *rsqnodes[range_count];

	raxSeek(&it, "^", NULL, 0);
	while(raxNext(&it)) {
		char *field = (char *)it.key;
		NumericRange *nr = (NumericRange *) it.data;

		sprintf(query_field_name, "%.*s", (int)it.key_len, field);
		RSQNode *rsqn = _NumericRangeToQueryNode(idx, query_field_name, nr);
		rsqnodes[idx++] = rsqn;
	}
	raxStop(&it);

	raxStart(&it, string_ranges);
	raxSeek(&it, "^", NULL, 0);
	while(raxNext(&it)) {
		char *field = (char *)it.key;
		StringRange *sr = (StringRange *) it.data;

		sprintf(query_field_name, "%.*s", (int)it.key_len, field);
		RSQNode *rsqn = _StringRangeToQueryNode(idx, query_field_name, sr);
		rsqnodes[idx++] = rsqn;
	}
	raxStop(&it);

	RSQNode *root = _concat_query_nodes(idx, rsqnodes, range_count);
	return root;
}

// reduce filters into ranges
// we differentiate between numeric filters and string filters
void _compose_ranges(FT_FilterNode **trees, rax *string_ranges,
		rax *numeric_ranges) {
	ASSERT(trees != NULL);
	ASSERT(string_ranges != NULL);
	ASSERT(numeric_ranges != NULL);

	uint count = array_len(trees);
	for(uint i = 0; i < count; i++) {
		FT_FilterNode *tree = trees[i];
		if(tree->t == FT_N_PRED) {
			if(_predicateTreeToRange(tree, string_ranges, numeric_ranges)) {
				// managed to convert tree into range
				// discard tree and update loop index
				FilterTree_Free(tree);
				array_del_fast(trees, i);
				i--;
				count--;
			}
		}
	}
}

// tries to convert filter tree to a RediSearch query
// return true if tree was converted, false otherwise
// a conversion might fail if tree contains a none indexable type e.g. array
static bool _FilterTreeConditionToQueryNode
(
	RSQNode **root,
	FT_FilterNode *tree,
	RSIndex *idx
) {

	ASSERT(idx != NULL);
	ASSERT(root != NULL);
	ASSERT(tree != NULL);
	ASSERT(tree->t == FT_N_COND);


	*root = NULL; // initialize output to NULL
	// validate operator
	fST_Operator op = tree->cond.op;
	ASSERT(op == OP_OR || op == OP_AND);

	RSQNode  *node   =  NULL;
	RSQNode  *left   =  NULL;
	RSQNode  *right  =  NULL;

	//--------------------------------------------------------------------------
	// convert left and right hand sides
	//--------------------------------------------------------------------------

	// process left branch
	if(!_FilterTreeToQueryNode(&left, tree->cond.left, idx)) return false;

	// process right branch
	if(!_FilterTreeToQueryNode(&right, tree->cond.right, idx)) {
		RediSearch_QueryNodeFree(left);
		return false;
	}

	if(op == OP_OR) node = RediSearch_CreateUnionNode(idx);
	else node = RediSearch_CreateIntersectNode(idx, false);

	RediSearch_QueryNodeAddChild(node, left);
	RediSearch_QueryNodeAddChild(node, right);

	*root = node;
	return true;
}

// returns true if predicate filter been converted to an index query
static bool _FilterTreePredicateToQueryNode
(
	RSQNode **root,      // array of query nodes to populate
	FT_FilterNode *tree, // filter to convert
	RSIndex *idx         // queried index
) {

	ASSERT(idx  != NULL);
	ASSERT(root != NULL);
	ASSERT(tree != NULL);
	ASSERT(tree->t == FT_N_PRED);

	// expecting left hand side to be an attribute access
	RSQNode  *node      =  NULL;
	char     *field     =  NULL;
	bool     attribute  =  AR_EXP_IsAttribute(tree->pred.lhs,  &field);
	ASSERT(attribute == true);

	// validate const type
	ASSERT(AR_EXP_IsConstant(tree->pred.rhs));
	SIValue v = tree->pred.rhs->operand.constant;
	SIType t = SI_TYPE(v);
	if(!(t & SI_INDEXABLE)) return false;

	// validate operation, we can handle <, <=, =, >, >=
	AST_Operator op = tree->pred.op;
	if(!(op == OP_LT ||
		 op == OP_LE ||
		 op == OP_GT ||
		 op == OP_GE ||
		 op == OP_EQUAL)) {
		return false;
	}

	if(t == T_STRING) {
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
			default:
				ASSERT(false && "unexpected operation");
		}

		RSQNode *parent = RediSearch_CreateTagNode(idx, field);
		RediSearch_QueryNodeAddChild(parent, node);
		node = parent;
	} else {
		ASSERT(t & SI_NUMERIC || t == T_BOOL);
		double d = SI_GET_NUMERIC(v);
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
			default:
				ASSERT(false && "unexpected operation");
		}
	}

	*root = node;
	return true;
}

// returns true if 'tree' been converted into an index query, false otherwise
static bool _FilterTreeToQueryNode
(
	RSQNode**root,       // array of query nodes to populate
	FT_FilterNode *tree, // filter to convert into an index query
	RSIndex *idx         // queried index
) {
	ASSERT(idx  != NULL);
	ASSERT(root != NULL);
	ASSERT(tree != NULL);

	// initialize 'root' to NULL
	*root = NULL;

	if(_isInFilter(tree)) {
		*root = _FilterTreeToInQueryNode(tree, idx);
		return true;
	}

	if(_isDistanceFilter(tree)) {
		*root = _FilterTreeToDistanceQueryNode(tree, idx);
		return true;
	}

	FT_FilterNodeType t = tree->t;

	if(t == FT_N_COND) {
		return _FilterTreeConditionToQueryNode(root, tree, idx);
	} else if(t == FT_N_PRED) {
		return _FilterTreePredicateToQueryNode(root, tree, idx);
	} else {
		ASSERT("unknown filter tree node type");
		return false;
	}
}

// creates a RediSearch query node out of given filter tree
RSQNode *FilterTreeToQueryNode
(
	FT_FilterNode **none_converted_filters, // [output] none convertable filters
	const FT_FilterNode *tree,              // filter tree to convert
	RSIndex *idx                            // index to query
) {
	ASSERT(idx != NULL);
	ASSERT(tree != NULL);
	ASSERT(none_converted_filters != NULL);

	// clone filter tree, as it is about to be modified
	FT_FilterNode  *t       =  FilterTree_Clone(tree);
	RSQNode        **nodes  =  array_new(RSQNode*, 1);  // intermidate nodes
	FT_FilterNode  **trees  =  FilterTree_SubTrees(t);  // break tree down to individual subtrees

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
		RSQNode *node = NULL;
		if(_FilterTreeToQueryNode(&node, trees[i], idx)) {
			nodes = array_append(nodes, node);
			FilterTree_Free(trees[i]);

			// remove converted filter from filters array
			array_del_fast(trees, i);
			i--;
			tree_count--;
		}
	}

	//--------------------------------------------------------------------------
	// combine remaining filters
	//--------------------------------------------------------------------------

	// filters that can not be converted into an index query will be returned
	// to caller as a single filter tree
	// this might happen when the value compared against is a runtime value of
	// none indexable type e.g. array
	*none_converted_filters = FilterTree_Combine(trees, tree_count);

	// compose root query node by intersecting individual query nodes
	RSQNode *root = _concat_query_nodes(idx, nodes);

	// at this point there are 3 options:
	// 1. all filters been converted into index queries
	// in which case 'root' is set to an index query
	// 'none_converted_filters' is set to NULL
	//
	// 2. none of the filters been converted to index queries
	// in which case 'root' is set to NULL
	// 'none_converted_filters' is the same as the input 'tree'
	//
	// 3. some filters been converted into index queries
	// in which case 'root' is set to an index query
	// 'none_converted_filters' contains none converted filters

	//--------------------------------------------------------------------------
	// clean up
	//--------------------------------------------------------------------------

	array_free(nodes);
	array_free(trees);
	raxFreeWithCallback(string_ranges, (void(*)(void *))StringRange_Free);
	raxFreeWithCallback(numeric_ranges, (void(*)(void *))NumericRange_Free);

	return root;
}

