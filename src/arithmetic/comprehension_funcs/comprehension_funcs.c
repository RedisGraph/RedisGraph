/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "comprehension_funcs.h"
#include "../func_desc.h"
#include "../arithmetic_expression.h"
#include "../../value.h"
#include "../../util/arr.h"
#include "../../datatypes/array.h"
#include "../../execution_plan/record.h"

void ListComprehension_Free(AR_ExpNode *exp) {
	// TODO
	// AR_ExpNode *predicate = exp->op.children[2];
	// assert(predicate->type == AR_EXP_OPERAND && predicate->operand.type == AR_EXP_CONSTANT);
	// SIValue predicate_val = predicate->operand.constant;
	// if(SI_TYPE(predicate_val) & T_PTR) AR_EXP_Free(predicate->operand.constant.ptrval);
	// predicate->operand.constant = SI_NullVal();

	// AR_ExpNode *eval = exp->op.children[3];
	// assert(eval->type == AR_EXP_OPERAND && eval->operand.type == AR_EXP_CONSTANT);
	// SIValue eval_val = eval->operand.constant;
	// if(SI_TYPE(eval_val) & T_PTR) AR_EXP_Free(eval->operand.constant.ptrval);
	// eval->operand.constant = SI_NullVal();
}

SIValue AR_LIST_COMPREHENSION(SIValue *argv, int argc) {
	SIValue variable_val = argv[0];
	SIValue list = argv[1];
	SIValue predicate = argv[2];
	SIValue eval = argv[3];
	SIValue record = argv[4];

	const char *variable = variable_val.stringval;
	AR_ExpNode *eval_exp = (SI_TYPE(eval) == T_NULL) ? NULL : eval.ptrval;
	AR_ExpNode *predicate_exp = (SI_TYPE(predicate) == T_NULL) ? NULL : predicate.ptrval;
	Record r = record.ptrval;
	int elem_idx = Record_GetEntryIdx(r, variable);

	SIValue retval = SI_Array(0);

	uint len = SIArray_Length(list);
	for(uint i = 0; i < len; i++) {
		// Retrieve the current element.
		SIValue current_elem = SIArray_Get(list, i);
		// Add the current element to the record at position elem_idx.
		Record_AddScalar(r, elem_idx, current_elem);

		if(predicate_exp) {
			SIValue passed_predicate = AR_EXP_Evaluate(predicate_exp, r);
			// TODO can be null? I don't think so...
			// if(SI_TYPE(passed_predicate) == T_NULL || passed_predicate.longval != 1) continue;
			assert(SI_TYPE(passed_predicate) & T_BOOL);
			// Skip failing elements
			if(passed_predicate.longval != 1) continue;
		}

		if(eval_exp) {
			// Compute the current element to return.
			SIValue newval = AR_EXP_Evaluate(eval_exp, r);
			SIArray_Append(&retval, newval);
			SIValue_Free(newval);
		} else {
			SIArray_Append(&retval, current_elem);
		}
		/* Free the record's current entries in case they were heap-allocated,
		 * as in cases like using reduce to concatenate strings. */
		// Record_FreeEntries(ctx->reduce_record);

		// Update the Record with the new accumulator value.
		// Record_AddScalar(ctx->reduce_record, 0, accumulator_val);
	}
	// TODO think about
	// Record_RemoveFromMapping(r, variable.stringval);
	return retval;
}

void Register_ComprehensionFuncs() {
	SIType *types;
	AR_FuncDesc *func_desc;

	types = array_new(SIType, 5);
	types = array_append(types, T_STRING);
	types = array_append(types, T_ARRAY | T_NULL);
	types = array_append(types, T_PTR | T_NULL);
	types = array_append(types, T_PTR | T_NULL);
	types = array_append(types, T_PTR);
	func_desc = AR_FuncDescNew("list_comprehension", AR_LIST_COMPREHENSION, 5, 5, types, false);
	AR_RegFunc(func_desc);
	func_desc->bfree = ListComprehension_Free;
}
