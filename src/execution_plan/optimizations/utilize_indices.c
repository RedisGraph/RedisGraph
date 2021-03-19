/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "utilize_indices.h"
#include "RG.h"
#include "../../value.h"
#include "../../util/arr.h"
#include "../../query_ctx.h"
#include "../ops/op_index_scan.h"
#include "../execution_plan_build/execution_plan_modify.h"
#include "../../ast/ast_shared.h"
#include "../../util/range/string_range.h"
#include "../../util/range/numeric_range.h"
#include "../../datatypes/array.h"
#include "../../datatypes/point.h"
#include "../../arithmetic/arithmetic_op.h"

//------------------------------------------------------------------------------
// Filter normalization
//------------------------------------------------------------------------------

// modifies filter tree such that the right-hand side is of type constant
void _normalize_filter(FT_FilterNode **filter) {
	FT_FilterNode *filter_tree = *filter;
	// normalize, left hand side should be variadic, right hand side const
	switch(filter_tree->t) {
	case FT_N_PRED:
		if(filter_tree->pred.lhs->operand.type == AR_EXP_CONSTANT) {
			// swap
			AR_ExpNode *tmp = filter_tree->pred.rhs;
			filter_tree->pred.rhs = filter_tree->pred.lhs;
			filter_tree->pred.lhs = tmp;
			filter_tree->pred.op = ArithmeticOp_ReverseOp(filter_tree->pred.op);
		}
		break;
	case FT_N_COND:
		_normalize_filter(&filter_tree->cond.left);
		_normalize_filter(&filter_tree->cond.right);
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

/* Tests to see if given filter tree is a simple predicate
 * e.g. n.v = 2
 * one side is variadic while the other side is constant. */
bool _simple_predicates(FT_FilterNode *filter) {
	bool res = false;

	SIValue v;
	AR_ExpNode  *exp      =  NULL;
	AR_ExpNode  *lhs_exp  =  NULL;
	AR_ExpNode  *rhs_exp  =  NULL;

	if(_isInFilter(filter)) {
		return _validateInExpression(filter->exp.exp);
	}

	if(_isDistanceFilter(filter)) return true;

	switch(filter->t) {
	case FT_N_PRED:
		lhs_exp = filter->pred.lhs;
		rhs_exp = filter->pred.rhs;
		// filter should be in the form of variable=scalar or scalar=variable
		// find out which part of the filter performs entity attribute access

		// n.v = exp
		if(AR_EXP_IsAttribute(lhs_exp, NULL)) exp = rhs_exp;
		// exp = n.v
		if(AR_EXP_IsAttribute(rhs_exp, NULL)) exp = lhs_exp;
		// filter is not of the form n.v = exp or exp = n.v
		if(exp == NULL) break;

		// make sure 'exp' represents a scalar
		bool scalar = AR_EXP_ReduceToScalar(exp, true, &v);
		if(scalar == false) break;

		// validate constant type
		SIType t = SI_TYPE(v);
		res = (t & (SI_NUMERIC | T_STRING | T_BOOL));
		break;
	case FT_N_COND:
		res = (_simple_predicates(filter->cond.left) && _simple_predicates(filter->cond.right));
		break;
	default:
		break;
	}

	return res;
}

// checks to see if given filter can be resolved by index
bool _applicableFilter(Index *idx, FT_FilterNode **filter) {
	bool           res           =  true;
	rax            *attr         =  NULL;
	rax            *entities     =  NULL;
	FT_FilterNode  *filter_tree  =  *filter;

	/* Make sure the filter root is not a function, other then IN or distance
	 * Make sure the "not equal, <>" operator isn't used. */
	if(FilterTree_containsOp(filter_tree, OP_NEQUAL)) {
		res = false;
		goto cleanup;
	}

	if(!_simple_predicates(filter_tree)) {
		res = false;
		goto cleanup;
	}

	uint idx_fields_count = Index_FieldsCount(idx);
	const char **idx_fields = Index_GetFields(idx);

	// Make sure all filtered attributes are indexed.
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
	_normalize_filter(filter);

cleanup:
	if(attr) raxFree(attr);
	return res;
}

// returns an array of filter operation which can be
// reduced into a single index scan operation
OpFilter **_applicableFilters(NodeByLabelScan *scanOp, Index *idx) {
	OpFilter **filters = array_new(OpFilter *, 0);

	// we begin with a LabelScan, and want to find predicate filters that modify
	// the active entity
	OpBase *current = scanOp->op.parent;
	while(current->type == OPType_FILTER) {
		OpFilter *filter = (OpFilter *)current;

		if(_applicableFilter(idx, &filter->filterTree)) {
			// make sure all predicates are of type n.v = CONST.
			filters = array_append(filters, filter);
		}

		// Advance to the next operation.
		current = current->parent;
	}

	return filters;
}

// try to replace given Label Scan operation and a set of Filter operations with
// a single Index Scan operation
void reduce_scan_op(ExecutionPlan *plan, NodeByLabelScan *scan) {
	// make sure there's an index for scanned label
	const char *label = scan->n.label;
	GraphContext *gc = QueryCtx_GetGraphCtx();
	Index *idx = GraphContext_GetIndex(gc, label, NULL, IDX_EXACT_MATCH);
	if(idx == NULL) return;

	// get all applicable filter for index
	RSIndex *rs_idx = idx->idx;
	OpFilter **filters = _applicableFilters(scan, idx);

	// no filters, return
	uint filters_count = array_len(filters);
	if(filters_count == 0) goto cleanup;

	// TODO: concat trees (clone individual trees)
	OpBase *indexOp = NewIndexScanOp(scan->op.plan, scan->g, scan->n, rs_idx, root);
	// replace the redundant scan op with the newly-constructed Index Scan
	ExecutionPlan_ReplaceOp(plan, (OpBase *)scan, indexOp);
	OpBase_Free((OpBase *)scan);

	// remove and free all now-redundant filter ops
	// since this is a chain of single-child operations
	// all operations are replaced in-place
	// avoiding problems with stream-sensitive ops like SemiApply
	for(uint i = 0; i < filters_count; i++) {
		OpFilter *filter = filters[i];
		ExecutionPlan_RemoveOp(plan, (OpBase *)filter);
		OpBase_Free((OpBase *)filter);
	}
	array_free(filters);
}

void utilizeIndices(ExecutionPlan *plan) {
	GraphContext *gc = QueryCtx_GetGraphCtx();
	// return immediately if the graph has no indices
	if(!GraphContext_HasIndices(gc)) return;

	// collect all label scans
	OpBase **scanOps = ExecutionPlan_CollectOps(plan->root, OPType_NODE_BY_LABEL_SCAN);

	int scanOpCount = array_len(scanOps);
	for(int i = 0; i < scanOpCount; i++) {
		NodeByLabelScan *scanOp = (NodeByLabelScan *)scanOps[i];
		// try to reduce label scan + filter(s) to a single IndexScan operation
		reduce_scan_op(plan, scanOp);
	}

	// cleanup
	array_free(scanOps);
}

