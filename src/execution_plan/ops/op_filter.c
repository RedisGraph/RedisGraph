/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "op_filter.h"

/* Forward declarations. */
static Record FilterConsume(OpBase *opBase);
static OpBase *FilterClone(const ExecutionPlan *plan, const OpBase *opBase);
static void FilterFree(OpBase *opBase);

// Extend the record mapping with local variables in filter tree expressions if necessary.
static inline void _Filter_MapLocalVariables(OpBase *op, FT_FilterNode *ft) {
	// If the filter tree contains a list comprehension node, retrieve a pointer to it.
	FT_FilterNode *list_comp_node;
	if(!FilterTree_ContainsFunc(ft, "LIST_COMPREHENSION", &list_comp_node)) return;

	AR_ExpNode *exp = list_comp_node->exp.exp;
	// Collect all local variable names in the arithmetic expression.
	const char **expression_variables = AR_EXP_CollectLocalVariables(exp);
	uint variable_count = array_len(expression_variables);
	// Add each variable name to the Record mapping.
	for(uint i = 0; i < variable_count; i ++) OpBase_Modifies(op, expression_variables[i]);
	array_free(expression_variables);
}

OpBase *NewFilterOp(const ExecutionPlan *plan, FT_FilterNode *filterTree) {
	OpFilter *op = rm_malloc(sizeof(OpFilter));
	op->filterTree = filterTree;

	// Set our Op operations
	OpBase_Init((OpBase *)op, OPType_FILTER, "Filter", NULL, FilterConsume,
				NULL, NULL, FilterClone, FilterFree, false, plan);

	_Filter_MapLocalVariables((OpBase *)op, filterTree);

	return (OpBase *)op;
}

/* FilterConsume next operation
 * returns OP_OK when graph passes filter tree. */
static Record FilterConsume(OpBase *opBase) {
	Record r;
	OpFilter *filter = (OpFilter *)opBase;
	OpBase *child = filter->op.children[0];

	while(true) {
		r = OpBase_Consume(child);
		if(!r) break;

		/* Pass graph through filter tree */
		if(FilterTree_applyFilters(filter->filterTree, r) == FILTER_PASS) break;
		else OpBase_DeleteRecord(r);
	}

	return r;
}

static inline OpBase *FilterClone(const ExecutionPlan *plan, const OpBase *opBase) {
	assert(opBase->type == OPType_FILTER);
	OpFilter *op = (OpFilter *)opBase;
	return NewFilterOp(plan, FilterTree_Clone(op->filterTree));
}

/* Frees OpFilter*/
static void FilterFree(OpBase *ctx) {
	OpFilter *filter = (OpFilter *)ctx;
	if(filter->filterTree) {
		FilterTree_Free(filter->filterTree);
		filter->filterTree = NULL;
	}
}

