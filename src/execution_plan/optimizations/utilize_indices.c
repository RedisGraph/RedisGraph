/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "RG.h"
#include "../../value.h"
#include "../../util/arr.h"
#include "../../query_ctx.h"
#include "../ops/op_filter.h"
#include "../ops/op_index_scan.h"
#include "../../ast/ast_shared.h"
#include "../../datatypes/array.h"
#include "../../datatypes/point.h"
#include "../ops/op_node_by_label_scan.h"
#include "../ops/op_conditional_traverse.h"
#include "../../arithmetic/arithmetic_op.h"
#include "../../filter_tree/filter_tree_utils.h"
#include "../../arithmetic/algebraic_expression.h"
#include "../../arithmetic/algebraic_expression/utils.h"
#include "../execution_plan_build/execution_plan_modify.h"

//------------------------------------------------------------------------------
// Filter normalization
//------------------------------------------------------------------------------

// modifies filter tree such that the left hand side performs
// attribute lookup on 'filtered_entity'
static void _normalize_filter(const char *filtered_entity,
		FT_FilterNode **filter) {
	FT_FilterNode *filter_tree = *filter;
	bool swap = false;
	rax *entities = NULL;

	// normalize, left hand side should be variadic, right hand side const
	switch(filter_tree->t) {
	case FT_N_PRED:
		entities = raxNew();
		AR_ExpNode *rhs = (*filter)->pred.rhs;
		AR_EXP_CollectEntities(rhs, entities);
		swap = raxFind(entities, (unsigned char *)filtered_entity,
				strlen(filtered_entity)) != raxNotFound;
		raxFree(entities);

		if(swap) {
			AR_ExpNode *tmp = filter_tree->pred.rhs;
			filter_tree->pred.rhs = filter_tree->pred.lhs;
			filter_tree->pred.lhs = tmp;
			filter_tree->pred.op = ArithmeticOp_ReverseOp(filter_tree->pred.op);
		}

		break;
	case FT_N_COND:
		_normalize_filter(filtered_entity, &filter_tree->cond.left);
		_normalize_filter(filtered_entity, &filter_tree->cond.right);
		break;
	case FT_N_EXP:
		// NOP, expression already normalized
		break;
	default:
		ASSERT(false);
		break;
	}
}

//------------------------------------------------------------------------------
// Validation functions
//------------------------------------------------------------------------------

static bool _validateInExpression(AR_ExpNode *exp) {
	ASSERT(exp->op.child_count == 2);

	AR_ExpNode *list = exp->op.children[1];
	SIValue listValue = SI_NullVal();
	AR_EXP_ReduceToScalar(list, true, &listValue);
	if(SI_TYPE(listValue) != T_ARRAY) return false;

	uint list_len = SIArray_Length(listValue);
	for(uint i = 0; i < list_len; i++) {
		SIValue v = SIArray_Get(listValue, i);
		// Ignore everything other than number, strings and booleans.
		if(!(SI_TYPE(v) & (SI_NUMERIC | T_STRING | T_BOOL))) return false;
	}
	return true;
}

// return true if filter can be resolved by an index query
static bool _applicable_predicate(const char* filtered_entity,
		FT_FilterNode *filter) {

	SIValue v;
	bool res              =  false;
	AR_ExpNode  *exp      =  NULL;
	AR_ExpNode  *lhs_exp  =  NULL;
	AR_ExpNode  *rhs_exp  =  NULL;

	if(isInFilter(filter)) {
		return _validateInExpression(filter->exp.exp);
	}

	if(isDistanceFilter(filter)) return true;

	switch(filter->t) {
	case FT_N_PRED:
		lhs_exp = filter->pred.lhs;
		rhs_exp = filter->pred.rhs;
		// filter should be in the form of:
		//
		// attr_lookup OP exp
		// or
		// exp OP attr_lookup
		//
		// find out which part of the filter performs entity attribute access

		// make sure filtered entity isn't mentioned on both ends of the filter
		// n.v = n.x
		rax *aliases = raxNew();
		bool mentioned_on_lhs = false;
		bool mentioned_on_rhs = false;

		AR_EXP_CollectEntities(lhs_exp, aliases);
		mentioned_on_lhs = raxFind(aliases, (unsigned char *)filtered_entity,
				strlen(filtered_entity)) != raxNotFound;

		raxRemove(aliases, (unsigned char *)filtered_entity,
				strlen(filtered_entity), NULL);

		AR_EXP_CollectEntities(rhs_exp, aliases);
		mentioned_on_rhs = raxFind(aliases, (unsigned char *)filtered_entity,
				strlen(filtered_entity)) != raxNotFound;

		raxFree(aliases);

		if(mentioned_on_lhs == true && mentioned_on_rhs == true) {
			res = false;
			break;
		}

		if(AR_EXP_IsAttribute(lhs_exp, NULL)) exp = rhs_exp;      // n.v = exp
		else if(AR_EXP_IsAttribute(rhs_exp, NULL)) exp = lhs_exp; // exp = n.v
		// filter is not of the form n.v = exp or exp = n.v
		if(exp == NULL) {
			res = false;
			break;
		}

		// determine whether 'exp' represents a scalar
		bool scalar = AR_EXP_ReduceToScalar(exp, true, &v);
		if(scalar) {
			// validate constant type
			SIType t = SI_TYPE(v);
			res = (t & (SI_NUMERIC | T_STRING | T_BOOL));
		} else {
			// value type can only be determined at runtime!
			res = true;
		}
		break;
	case FT_N_COND:
		// require both ends of the filter to be applicable
		res = (_applicable_predicate(filtered_entity, filter->cond.left) &&
				_applicable_predicate(filtered_entity, filter->cond.right));
		break;
	default:
		break;
	}

	return res;
}

// checks to see if given filter can be resolved by index
bool _applicableFilter
(
	const char* filtered_entity,
	const Index *idx,
	FT_FilterNode **filter
) {
	bool           res           =  true;
	rax            *attr         =  NULL;
	rax            *entities     =  NULL;
	FT_FilterNode  *filter_tree  =  *filter;

	// make sure the filter root is not a function, other then IN or distance
	// make sure the "not equal, <>" operator isn't used
	if(FilterTree_containsOp(filter_tree, OP_NEQUAL)) {
		res = false;
		goto cleanup;
	}

	if(!_applicable_predicate(filtered_entity, filter_tree)) {
		res = false;
		goto cleanup;
	}

	uint idx_fields_count = Index_FieldsCount(idx);
	const char **idx_fields = Index_GetFields(idx);

	// make sure all filtered attributes are indexed
	attr = FilterTree_CollectAttributes(filter_tree);
	uint filter_attribute_count = raxSize(attr);

	// Filter refers to a greater number of attributes.
	if(filter_attribute_count > idx_fields_count) {
		res = false;
		goto cleanup;
	}

	for(uint i = 0; i < idx_fields_count; i++) {
		const char *field = idx_fields[i];
		if(raxFind(attr, (unsigned char *)field, strlen(field)) != raxNotFound) {
			filter_attribute_count--;
			// All filtered attributes are indexed.
			if(filter_attribute_count == 0) break;
		}
	}

	if(filter_attribute_count != 0) {
		res = false;
		goto cleanup;
	}

	// Filter is applicable, prepare it to use in index.
	_normalize_filter(filtered_entity, filter);

cleanup:
	if(attr) raxFree(attr);
	return res;
}

// returns an array of filter operations which can be
// reduced into a single index scan operation
OpFilter **_applicableFilters
(
	const NodeByLabelScan *scanOp,
	const Index *idx
) {
	OpFilter **filters = array_new(OpFilter *, 0);
	const char* filtered_entity = scanOp->n.alias; // entity being filtered

	OpBase *current = scanOp->op.parent;
	ASSERT(current->type == OPType_FILTER);

	// we begin with a LabelScan, and want to find predicate filters that modify
	// the active entity
	while(current->type == OPType_FILTER) {
		OpFilter *filter = (OpFilter *)current;

		if(_applicableFilter(filtered_entity, idx, &filter->filterTree)) {
			array_append(filters, filter);
		}

		// advance to the next operation
		current = current->parent;
	}

	return filters;
}

static FT_FilterNode *_Concat_Filters(OpFilter **filter_ops) {
	uint count = array_len(filter_ops);
	ASSERT(count >= 1);
	if(count == 1) return FilterTree_Clone(filter_ops[0]->filterTree);

	// concat using AND nodes
	FT_FilterNode *root = FilterTree_CreateConditionFilter(OP_AND);
	FilterTree_AppendLeftChild(root,
			FilterTree_Clone(filter_ops[0]->filterTree));
	FilterTree_AppendRightChild(root,
			FilterTree_Clone(filter_ops[1]->filterTree));

	for(uint i = 2; i < count; i++) {
		// new and root node
		FT_FilterNode *and = FilterTree_CreateConditionFilter(OP_AND);
		FilterTree_AppendLeftChild(and, root);
		root = and;
		FilterTree_AppendRightChild(root,
				FilterTree_Clone(filter_ops[i]->filterTree));
	}

	return root;
}

// try to replace given Label Scan operation and a set of Filter operations with
// a single Index Scan operation
void reduce_scan_op
(
	ExecutionPlan *plan,
	NodeByLabelScan *scan
) {
	// in the multi-label case, we want to pick the label which will allow us to
	// both utilize an index and iterate over the fewest values
	GraphContext *gc  = QueryCtx_GetGraphCtx();
	Graph        *g   =  QueryCtx_GetGraph();
	QueryGraph   *qg  =  scan->op.plan->query_graph;

	// find label with filtered indexed properties
	// that has the minimum NNZ entries
	int         min_label_id;                 // tracks min label ID
	uint64_t    min_nnz        = UINT64_MAX;  // tracks min entries
	RSIndex     *rs_idx        = NULL;        // the index to be applied
	OpFilter    **filters      = NULL;        // tracks indexed filters to apply
	uint        filters_count  = 0;           // number of matching filters
	const char  *min_label_str = NULL;        // tracks min label name

	// see if scanned node has multiple labels
	const char *node_alias = scan->n.alias;
	QGNode *qn = QueryGraph_GetNodeByAlias(qg, node_alias);
	ASSERT(qn != NULL);

	uint label_count = QGNode_LabelCount(qn);
	for(uint i = 0; i < label_count; i++) {
		Index *idx;
		uint64_t nnz;
		int label_id = QGNode_GetLabelID(qn, i);
		const char *label = QGNode_GetLabel(qn, i);

		// unknown label
		if(label_id == GRAPH_UNKNOWN_LABEL) continue;

		idx = GraphContext_GetIndexByID(gc, label_id, NULL, IDX_EXACT_MATCH);

		// no index for current label
		if(idx == NULL) continue;

		// get all applicable filter for index
		RSIndex *cur_idx = idx->idx;
		// TODO switch to reusable array
		OpFilter **cur_filters = _applicableFilters(scan, idx);

		// TODO consider heuristic which combines max
		// number / restrictiveness of applicable filters
		// vs. the label's NNZ?
		uint cur_filters_count = array_len(cur_filters);
		if(cur_filters_count == 0) {
			// no filters
			array_free(cur_filters);
			continue;
		}

		nnz = Graph_LabeledNodeCount(g, label_id);
		if(min_nnz > nnz) {
			rs_idx         =  cur_idx;
			min_nnz        =  nnz;
			min_label_str  =  label;
			min_label_id   =  label_id;

			// swap previously stored index and
			// filters array (if any) with current filters
			array_free(filters);
			filters = cur_filters;
			filters_count = cur_filters_count;
		}
	}

	// no label possessed indexed and filtered attributes, return early
	if(rs_idx == NULL) goto cleanup;

	// did we found a better label to utilize? if so swap
	if(scan->n.label_id != min_label_id) {
		// the scanned label does not match the one we will build an
		// index scan over, update the traversal expression to
		// remove the indexed label and insert the previously-scanned label
		OpBase *parent = scan->op.parent;
		// skip filters
		while(OpBase_Type(parent) == OPType_FILTER) parent = parent->parent;
		if(OpBase_Type(parent) == OPType_CONDITIONAL_TRAVERSE) {
			OpCondTraverse *op_traverse = (OpCondTraverse*)parent;
			AlgebraicExpression *ae = op_traverse->ae;
			AlgebraicExpression *operand;

			const char *row_domain = scan->n.alias;
			const char *column_domain = scan->n.alias;

			bool found = AlgebraicExpression_LocateOperand(ae, &operand, NULL,
					row_domain, column_domain, NULL, min_label_str);
			ASSERT(found == true);

			AlgebraicExpression *replacement = AlgebraicExpression_NewOperand(NULL,
					true, AlgebraicExpression_Src(operand),
					AlgebraicExpression_Dest(operand), NULL, scan->n.label);

			_AlgebraicExpression_InplaceRepurpose(operand, replacement);
		}

		scan->n.label = min_label_str;
		scan->n.label_id = min_label_id;
	}

	FT_FilterNode *root = _Concat_Filters(filters);
	OpBase *indexOp = NewIndexScanOp(scan->op.plan, scan->g, scan->n, rs_idx,
			root);

	// replace the redundant scan op with the newly-constructed Index Scan
	ExecutionPlan_ReplaceOp(plan, (OpBase *)scan, indexOp);
	OpBase_Free((OpBase *)scan);

	// remove and free all redundant filter ops
	// since this is a chain of single-child operations
	// all operations are replaced in-place
	// avoiding problems with stream-sensitive ops like SemiApply
	for(uint i = 0; i < filters_count; i++) {
		OpFilter *filter = filters[i];
		ExecutionPlan_RemoveOp(plan, (OpBase *)filter);
		OpBase_Free((OpBase *)filter);
	}

cleanup:
	array_free(filters);
}

void utilizeIndices
(
	ExecutionPlan *plan
) {
	GraphContext *gc = QueryCtx_GetGraphCtx();
	// return immediately if the graph has no indices
	if(!GraphContext_HasIndices(gc)) return;

	// collect all label scans
	OpBase **scanOps = ExecutionPlan_CollectOps(plan->root,
			OPType_NODE_BY_LABEL_SCAN);

	int scanOpCount = array_len(scanOps);
	for(int i = 0; i < scanOpCount; i++) {
		NodeByLabelScan *scanOp = (NodeByLabelScan *)scanOps[i];

		// make sure scan is followed by filter(s)
		OpBase *parent = scanOp->op.parent;
		if(parent->type != OPType_FILTER) {
			// no filters to utilize
			continue;
		}

		// try to reduce label scan + filter(s) to a single IndexScan operation
		reduce_scan_op(plan, scanOp);
	}

	// cleanup
	array_free(scanOps);
}

