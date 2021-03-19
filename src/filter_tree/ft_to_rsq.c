#include "ft_to_rsq.h"

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

static inline bool _isInFilter(const FT_FilterNode *filter) {
	return (filter->t == FT_N_EXP &&
			filter->exp.exp->type == AR_EXP_OP &&
			strcasecmp(filter->exp.exp->op.func_name, "in") == 0);
}

// extracts both origin and radius from a distance filter
// distance(n.location, origin) < radius
static bool _extractOriginAndRadius(const FT_FilterNode *filter,
									SIValue *origin, SIValue *radius, char **point) {
	// distance (n.location, origin) < radius

	ASSERT(filter != NULL);

	if(filter->t != FT_N_PRED) return false;

	char        *p             =  NULL;
	SIValue     d              =  SI_NullVal();      // radius
	AR_ExpNode  *lhs           =  filter->pred.lhs;
	AR_ExpNode  *rhs           =  filter->pred.rhs;
	AR_ExpNode  *radius_exp    =  NULL;
	AR_ExpNode  *distance_exp  =  NULL;

	// find distance expression
	if(AR_EXP_IsOperation(lhs) &&
	   strcasecmp(lhs->op.func_name, "distance") == 0) {
		radius_exp = rhs;
		distance_exp = lhs;
	} else if(AR_EXP_IsOperation(rhs) &&
			  strcasecmp(rhs->op.func_name, "distance") == 0) {
		radius_exp = lhs;
		distance_exp = rhs;
	}

	// could not find 'distance' function call
	if(distance_exp == NULL) return false;

	// make sure radius is constant
	bool scalar = AR_EXP_ReduceToScalar(radius_exp, true, &d);
	if(!scalar) return false;

	if(!(SI_TYPE(d) & SI_NUMERIC)) {
		SIValue_Free(d);
		return false;
	}

	// find origin
	// distance expression should have 2 arguments
	lhs = distance_exp->op.children[0];
	rhs = distance_exp->op.children[1];

	SIValue  l         =  SI_NullVal();
	SIValue  r         =  SI_NullVal();
	bool     res       =  false;
	bool     l_scalar  =  AR_EXP_ReduceToScalar(lhs, true, &l);
	bool     r_scalar  =  AR_EXP_ReduceToScalar(rhs, true, &r);

	if(l_scalar && !r_scalar) {
		res = AR_EXP_IsAttribute(rhs, &p);
		if(point) *point = p;
		if(origin) *origin = l;
		if(radius) *radius = d;
	} else if(!l_scalar && r_scalar) {
		res = AR_EXP_IsAttribute(lhs, &p);
		if(point) *point = p;
		if(origin) *origin = r;
		if(radius) *radius = d;
	} else {
		res = false;
		SIValue_Free(d);
		if(l_scalar) SIValue_Free(l);
		if(r_scalar) SIValue_Free(r);
	}

	return res;
}

// return true if filter performs distance filtering
// distance(n.location, point({lat:1.1, lon:2.2})) < 40
static bool _isDistanceFilter(FT_FilterNode *filter) {
	bool res = _extractOriginAndRadius(filter, NULL, NULL, NULL);
	if(res) {
		_normalize_filter(&filter);
		ASSERT(filter->t == FT_N_PRED);
		AST_Operator op = filter->pred.op;
		// make sure filter structure is: distance(point, origin) <= radius
		res = (op == OP_LT || op == OP_LE);
	}

	return res;
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
void _predicateTreeToRange(const FT_FilterNode *tree, rax *string_ranges, rax *numeric_ranges) {
	// Simple predicate trees are used to build up a range object.
	ASSERT(AR_EXP_IsConstant(tree->pred.rhs));

	char *prop;
	bool attribute = AR_EXP_IsAttribute(tree->pred.lhs, &prop);
	ASSERT(attribute == true);

	int op = tree->pred.op;
	SIValue c = tree->pred.rhs->operand.constant;

	StringRange *sr = raxFind(string_ranges, (unsigned char *)prop, strlen(prop));
	NumericRange *nr = raxFind(numeric_ranges, (unsigned char *)prop, strlen(prop));

	// Get or create range object for alias.prop.
	if(SI_TYPE(c) & SI_NUMERIC || SI_TYPE(c) == T_BOOL) {
		// Create if doesn't exists.
		if(nr == raxNotFound) {
			nr = NumericRange_New();
			raxTryInsert(numeric_ranges, (unsigned char *)prop, strlen(prop), nr, NULL);
		}
		NumericRange_TightenRange(nr, op, SI_GET_NUMERIC(c));
	} else if(SI_TYPE(c) == T_STRING) {
		// Create if doesn't exists.
		if(sr == raxNotFound) {
			sr = StringRange_New();
			raxTryInsert(string_ranges, (unsigned char *)prop, strlen(prop), sr, NULL);
		}
		StringRange_TightenRange(sr, op, c.stringval);
	} else {
		ASSERT(false);
	}
}

// creates a RediSearch query node out of given filter tree
RSQNode *_filterTreeToQueryNode(FT_FilterNode *filter, RSIndex *idx) {
	RSQNode *node   = NULL;
	RSQNode *parent = NULL;

	if(_isInFilter(filter_tree)) {
		return _filterTreeToInQueryNode(filter, idx);
	}

	if(_isDistanceFilter(filter_tree)) {
		return _filterTreeToDistanceQueryNode(filter, idx);
	}

	switch(filter->t) {
		case FT_N_COND: {
			RSQNode *left = NULL;
			RSQNode *right = NULL;
			switch(filter->cond.op) {
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
			left = _filterTreeToQueryNode(filter->cond.left, idx);
			right = _filterTreeToQueryNode(filter->cond.right, idx);
			RediSearch_QueryNodeAddChild(node, left);
			RediSearch_QueryNodeAddChild(node, right);
			break;
		}
		case FT_N_PRED: {
			double d;
			char *field;
			bool attribute = AR_EXP_IsAttribute(filter->pred.lhs, &field);
			ASSERT(attribute == true);

			SIValue v = filter->pred.rhs->operand.constant;
			switch(SI_TYPE(v)) {
				case T_STRING:
					parent = RediSearch_CreateTagNode(idx, field);
					switch(filter->pred.op) {
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
					switch(filter->pred.op) {
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
			node = _filterTreeToInQueryNode(filter, idx);
			break;
		}
		default: {
			ASSERT("unknown filter tree node type");
			break;
		}
	}
	return node;
}

void FilterTreeToSearchQuery(const FT_FilterNode *tree) {
	RSQNode  *root            =  NULL;
	uint     rsqnode_count    =  0;
	RSQNode  **rsqnodes       =  NULL;
	rax      *string_ranges   =  NULL;
	rax      *numeric_ranges  =  NULL;

	// reduce filters into ranges
	// we differentiate between numeric filters and string filters
	rsqnodes = array_new(RSQNode *, 1);

	string_ranges = raxNew();
	numeric_ranges = raxNew();

	for(uint i = 0; i < filters_count; i++) {
		RSQNode        *rsqnode      =  NULL;
		OpFilter       *filter       =  filters[i];
		FT_FilterNode  *filter_tree  =  filter->filterTree;

		switch(filter_tree->t) {
		case FT_N_PRED:
			_predicateTreeToRange(filter_tree, string_ranges, numeric_ranges);
			break;
		case FT_N_COND:
			// OR trees are directly converted into RSQnodes.
			rsqnode = _filterTreeToQueryNode(filter_tree, rs_idx);
			rsqnodes = array_append(rsqnodes, rsqnode);
			break;
		default:
			break;
		}
	}

	// build RediSearch query tree
	// convert each range object to RediSearch query node
	raxIterator it;
	raxStart(&it, string_ranges);
	raxSeek(&it, "^", NULL, 0);
	char query_field_name[1024];
	while(raxNext(&it)) {
		char *field = (char *)it.key;

		/* Make sure each property is bound to either numeric or string type
		 * but not to both, e.g. a.v = 1 AND a.v = 'a'
		 * in which case use an empty RSQueryNode. */
		if(raxFind(numeric_ranges, (unsigned char *)field, (int)it.key_len) != raxNotFound) {
			root = RediSearch_CreateEmptyNode(rs_idx);
			goto cleanup;
		}

		StringRange *sr = raxFind(string_ranges, (unsigned char *)field, (int)it.key_len);
		if(!StringRange_IsValid(sr)) {
			root = RediSearch_CreateEmptyNode(rs_idx);
			goto cleanup;
		}

		sprintf(query_field_name, "%.*s", (int)it.key_len, field);
		RSQNode *rsqn = _StringRangeToQueryNode(rs_idx, query_field_name, sr);
		rsqnodes = array_append(rsqnodes, rsqn);
	}
	raxStop(&it);

	raxStart(&it, numeric_ranges);
	raxSeek(&it, "^", NULL, 0);
	while(raxNext(&it)) {
		char *field = (char *)it.key;
		NumericRange *nr = raxFind(numeric_ranges, (unsigned char *)field, (int)it.key_len);

		// return empty RSQueryNode.
		if(!NumericRange_IsValid(nr)) {
			root = RediSearch_CreateEmptyNode(rs_idx);
			goto cleanup;
		}

		sprintf(query_field_name, "%.*s", (int)it.key_len, field);
		RSQNode *rsqn = _NumericRangeToQueryNode(rs_idx, query_field_name, nr);
		rsqnodes = array_append(rsqnodes, rsqn);
	}
	raxStop(&it);

	// Connect all RediSearch query nodes.
	rsqnode_count = array_len(rsqnodes);

	// No way to utilize the index.
	if(rsqnode_count == 0) goto cleanup;

	// Just a single filter.
	if(rsqnode_count == 1) {
		root = array_pop(rsqnodes);
	} else {
		// Multiple filters, combine using AND.
		root = RediSearch_CreateIntersectNode(rs_idx, false);
		for(uint i = 0; i < rsqnode_count; i++) {
			RSQNode *qnode = array_pop(rsqnodes);
			RediSearch_QueryNodeAddChild(root, qnode);
		}
	}

cleanup:
	if(string_ranges) raxFreeWithCallback(string_ranges, (void(*)(void *))StringRange_Free);
	if(numeric_ranges) raxFreeWithCallback(numeric_ranges, (void(*)(void *))NumericRange_Free);
	if(rsqnodes) array_free(rsqnodes);
}
