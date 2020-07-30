/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "comprehension_funcs.h"
#include "../../RG.h"
#include "../func_desc.h"
#include "../../value.h"
#include "../../util/arr.h"
#include "../../datatypes/array.h"
#include "../../execution_plan/record.h"

/* Routine for freeing a list comprehension's subtree of arithmetic expressions.
 * The predicate and eval routines require special handling to be freed properly. */
void ListComprehension_Free(void *exp_ptr) {
	AR_ExpNode *exp = exp_ptr;
	// The child at index 0 contains the list comprehension's context.
	AR_ExpNode *ctx_node = exp->op.children[0];
	ASSERT(ctx_node->type == AR_EXP_OPERAND && ctx_node->operand.type == AR_EXP_CONSTANT);
	AR_ComprehensionCtx *ctx = ctx_node->operand.constant.ptrval;

	// Free the variadic.
	AR_EXP_Free(ctx->variable);

	// If this list comprehension has a filter tree, free it.
	if(ctx->ft) FilterTree_Free(ctx->ft);

	// If this list comprehension has an eval routine, free it.
	if(ctx->eval_exp) AR_EXP_Free(ctx->eval_exp);

	rm_free(ctx);
}

/* Routine for cloning a list comprehension's subtree of arithmetic expressions.
 * The predicate and eval routines require special handling to be cloned properly. */
void ListComprehension_Clone(void *orig_ptr, void *clone_ptr) {
	AR_ExpNode *orig = orig_ptr;
	AR_ExpNode *clone = clone_ptr;

	// Use the normal clone routine for all children that are not pointers.
	clone->op.children[1] = AR_EXP_Clone(orig->op.children[1]);
	clone->op.children[2] = AR_EXP_Clone(orig->op.children[2]);

	// The child at index 0 contains the list comprehension's context.
	AR_ExpNode *ctx_node = orig->op.children[0];
	ASSERT(ctx_node->type == AR_EXP_OPERAND && ctx_node->operand.type == AR_EXP_CONSTANT);
	AR_ComprehensionCtx *ctx = ctx_node->operand.constant.ptrval;

	AR_ComprehensionCtx *ctx_clone = rm_malloc(sizeof(AR_ComprehensionCtx));

	ctx_clone->variable = AR_EXP_Clone(ctx->variable);

	// Clone the predicate filter tree.
	ctx_clone->ft = FilterTree_Clone(ctx->ft);

	// Clone the eval routine.
	ctx_clone->eval_exp = AR_EXP_Clone(ctx->eval_exp);

	clone->op.children[0] = AR_EXP_NewConstOperandNode(SI_PtrVal(ctx_clone));
}

SIValue AR_LIST_COMPREHENSION(SIValue *argv, int argc) {
	// Retrieve the context.
	AR_ComprehensionCtx *ctx = argv[0].ptrval;
	SIValue list = argv[1];
	Record r = argv[2].ptrval;

	int elem_idx = ctx->variable->operand.variadic.entity_alias_idx;

	// Instantiate the array to be returned.
	SIValue retval = SI_Array(0);

	uint len = SIArray_Length(list);
	for(uint i = 0; i < len; i++) {
		// Retrieve the current element.
		SIValue current_elem = SIArray_Get(list, i);
		// Add the current element to the record at position elem_idx.
		Record_AddScalar(r, elem_idx, current_elem);

		if(ctx->ft) {
			// If the comprehension has a filter tree, run the current element through it.
			bool passed_predicate = FilterTree_applyFilters(ctx->ft, r);
			// If it did not pass, skip this element.
			if(!passed_predicate) continue;
		}

		if(ctx->eval_exp) {
			// Compute the current element to append to the return list.
			SIValue newval = AR_EXP_Evaluate(ctx->eval_exp, r);
			SIArray_Append(&retval, newval);
			SIValue_Free(newval);
		} else {
			// If the comprehension has no eval routine, add each element unmodified.
			SIArray_Append(&retval, current_elem);
		}
	}

	return retval;
}

void Register_ComprehensionFuncs() {
	SIType *types;
	AR_FuncDesc *func_desc;

	types = array_new(SIType, 3);
	types = array_append(types, T_PTR);
	types = array_append(types, T_ARRAY | T_NULL);
	types = array_append(types, T_PTR);
	func_desc = AR_FuncDescNew("list_comprehension", AR_LIST_COMPREHENSION, 3, 3, types, false);
	AR_RegFunc(func_desc);
	func_desc->bfree = ListComprehension_Free;
	func_desc->bclone = ListComprehension_Clone;
}

