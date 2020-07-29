/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "comprehension_funcs.h"
#include "../../RG.h"
#include "../func_desc.h"
#include "../arithmetic_expression.h"
#include "../../value.h"
#include "../../util/arr.h"
#include "../../datatypes/array.h"
#include "../../execution_plan/record.h"
#include "../../filter_tree/filter_tree.h"

/* Routine for freeing a list comprehension's subtree of arithmetic expressions.
 * The predicate and eval routines require special handling to be freed properly. */
void ListComprehension_Free(AR_ExpNode *exp) {
	// The child at index 0 contains the local variable as a variadic node.
	AR_ExpNode *variable = exp->op.children[0];
	ASSERT(variable->type == AR_EXP_OPERAND && variable->operand.type == AR_EXP_CONSTANT);
	SIValue variable_val = variable->operand.constant;
	AR_EXP_Free(variable_val.ptrval);

	// The child at index 2 contains the filter tree, if any.
	AR_ExpNode *predicate = exp->op.children[2];
	ASSERT(predicate->type == AR_EXP_OPERAND && predicate->operand.type == AR_EXP_CONSTANT);
	SIValue predicate_val = predicate->operand.constant;
	// If this list comprehension has a filter tree, free it.
	if(SI_TYPE(predicate_val) & T_PTR) FilterTree_Free(predicate_val.ptrval);

	// The child at index 3 contains the eval routine, if any.
	AR_ExpNode *eval = exp->op.children[3];
	assert(eval->type == AR_EXP_OPERAND && eval->operand.type == AR_EXP_CONSTANT);
	SIValue eval_val = eval->operand.constant;
	// If this list comprehension has an eval routine, free it.
	if(SI_TYPE(eval_val) & T_PTR) AR_EXP_Free(eval_val.ptrval);
}

/* Routine for cloning a list comprehension's subtree of arithmetic expressions.
 * The predicate and eval routines require special handling to be cloned properly. */
void ListComprehension_Clone(AR_ExpNode *orig, AR_ExpNode *clone) {
	// Use the normal clone routine for all children that are not pointers.
	clone->op.children[1] = AR_EXP_Clone(orig->op.children[1]);
	clone->op.children[4] = AR_EXP_Clone(orig->op.children[4]);

	// Clone the variadic representing the local variable.
	AR_ExpNode *variable_exp = orig->op.children[0];
	ASSERT(variable->type == AR_EXP_OPERAND && variable->operand.type == AR_EXP_CONSTANT);
	AR_ExpNode *variable_clone = AR_EXP_Clone(variable_exp->operand.constant.ptrval);
	clone->op.children[0] = AR_EXP_NewConstOperandNode(SI_PtrVal(variable_clone));

	// Clone the predicate filter tree.
	AR_ExpNode *predicate = orig->op.children[2];
	ASSERT(predicate->type == AR_EXP_OPERAND && predicate->operand.type == AR_EXP_CONSTANT);
	SIValue predicate_val = predicate->operand.constant;
	if(SI_TYPE(predicate_val) & T_PTR) {
		/* If the comprehension has a filter tree, clone it and wrap it in a
		 * constant node in the new tree. */
		FT_FilterNode *ft_clone = FilterTree_Clone(predicate->operand.constant.ptrval);
		clone->op.children[2] = AR_EXP_NewConstOperandNode(SI_PtrVal(ft_clone));
	} else {
		// If the comprehension has no filters, simply clone the NULL placeholder.
		clone->op.children[2] = AR_EXP_Clone(orig->op.children[2]);
	}

	// Clone the eval routine.
	AR_ExpNode *eval = orig->op.children[3];
	ASSERT(eval->type == AR_EXP_OPERAND && eval->operand.type == AR_EXP_CONSTANT);
	SIValue eval_val = eval->operand.constant;
	if(SI_TYPE(eval_val) & T_PTR) {
		/* If the comprehension has an eval routine, clone it and wrap it in a
		 * constant node in the new tree. */
		AR_ExpNode *eval_clone = AR_EXP_Clone(eval->operand.constant.ptrval);
		clone->op.children[3] = AR_EXP_NewConstOperandNode(SI_PtrVal(eval_clone));
	} else {
		// Otherwise, simply clone the NULL placeholder.
		clone->op.children[3] = AR_EXP_Clone(orig->op.children[3]);
	}
}

SIValue AR_LIST_COMPREHENSION(SIValue *argv, int argc) {
	// Unpack the arguments to the list comprehension.
	AR_ExpNode *variable = argv[0].ptrval;
	SIValue list = argv[1];
	FT_FilterNode *ft = SIValue_IsNull(argv[2]) ? NULL : argv[2].ptrval;
	AR_ExpNode *eval_exp = SIValue_IsNull(argv[3]) ? NULL : argv[3].ptrval;
	Record r = argv[4].ptrval;

	int elem_idx = variable->operand.variadic.entity_alias_idx;

	// Instantiate the array to be returned.
	SIValue retval = SI_Array(0);

	uint len = SIArray_Length(list);
	for(uint i = 0; i < len; i++) {
		// Retrieve the current element.
		SIValue current_elem = SIArray_Get(list, i);
		// Add the current element to the record at position elem_idx.
		Record_AddScalar(r, elem_idx, current_elem);

		if(ft) {
			// If the comprehension has a filter tree, run the current element through it.
			bool passed_predicate = FilterTree_applyFilters(ft, r);
			// If it did not pass, skip this element.
			if(!passed_predicate) continue;
		}

		if(eval_exp) {
			// Compute the current element to append to the return list.
			SIValue newval = AR_EXP_Evaluate(eval_exp, r);
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

	types = array_new(SIType, 5);
	types = array_append(types, T_PTR);
	types = array_append(types, T_ARRAY | T_NULL);
	types = array_append(types, T_PTR | T_NULL);
	types = array_append(types, T_PTR | T_NULL);
	types = array_append(types, T_PTR);
	func_desc = AR_FuncDescNew("list_comprehension", AR_LIST_COMPREHENSION, 5, 5, types, false);
	AR_RegFunc(func_desc);
	func_desc->bfree = ListComprehension_Free;
	func_desc->bclone = ListComprehension_Clone;
}

