/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "comprehension_funcs.h"
#include "../../RG.h"
#include "../../value.h"
#include "../func_desc.h"
#include "../../errors.h"
#include "../../util/arr.h"
#include "../../query_ctx.h"
#include "../../datatypes/array.h"
#include "../../util/rax_extensions.h"
#include "../../execution_plan/record.h"

// Routine for freeing a comprehension function's private data.
void ListComprehension_Free(void *ctx_ptr) {
	ListComprehensionCtx *ctx = ctx_ptr;

	// If this comprehension has a filter tree, free it.
	if(ctx->ft) FilterTree_Free(ctx->ft);
	// If this comprehension has an eval routine, free it.
	if(ctx->eval_exp) AR_EXP_Free(ctx->eval_exp);

	if(ctx->local_record) {
		rax *mapping = ctx->local_record->mapping;
		Record_Free(ctx->local_record);
		raxFree(mapping);
	}

	rm_free(ctx);
}

// Routine for cloning a comprehension function's private data.
void *ListComprehension_Clone(void *orig) {
	ListComprehensionCtx *ctx = orig;
	// Allocate space for the clone.
	ListComprehensionCtx *ctx_clone = rm_malloc(sizeof(ListComprehensionCtx));

	// Clone the variadic node.
	ctx_clone->variable_str = ctx->variable_str;
	ctx_clone->variable_idx = ctx->variable_idx;
	ctx_clone->local_record = NULL;

	// Clone the predicate filter tree, if present.
	ctx_clone->ft = FilterTree_Clone(ctx->ft);
	// Clone the eval routine, if present.
	ctx_clone->eval_exp = AR_EXP_Clone(ctx->eval_exp);

	return ctx_clone;
}

static void _PopulateComprehensionCtx(ListComprehensionCtx *ctx, Record outer_record) {
	rax *local_record_map = raxClone(outer_record->mapping);
	intptr_t id = raxSize(local_record_map);
	int rc = raxTryInsert(local_record_map, (unsigned char *)ctx->variable_str,
						  strlen(ctx->variable_str), (void *)id, NULL);
	if(rc == 0) {
		// The local variable's name shadows an outer variable, emit an error.
		ErrorCtx_RaiseRuntimeException("Variable '%s' redefined inside of list comprehension",
						  (unsigned char *)ctx->variable_str);
	}

	ctx->local_record = Record_New(local_record_map);

	// This could just be assigned to 'id', but for safety we'll use a Record lookup.
	ctx->variable_idx = Record_GetEntryIdx(ctx->local_record, ctx->variable_str);
	ASSERT(ctx->variable_idx != INVALID_INDEX);
}


SIValue AR_ANY(SIValue *argv, int argc) {
	if(SI_TYPE(argv[0]) == T_NULL) return SI_NullVal();
	/* ANY comprehensions are invoked with three children:
	 * The list to iterate over.
	 * The current Record.
	 * The function context. */
	SIValue list = argv[0];
	Record outer_record = argv[1].ptrval;
	ListComprehensionCtx *ctx = argv[2].ptrval;

	// On the first invocation, build the local Record.
	if(ctx->local_record == NULL) _PopulateComprehensionCtx(ctx, outer_record);
	Record r = ctx->local_record;

	// Populate the local Record with the contents of the outer Record.
	Record_Clone(outer_record, r);

	uint len = SIArray_Length(list);
	for(uint i = 0; i < len; i++) {
		// Retrieve the current element.
		SIValue current_elem = SIArray_Get(list, i);
		// Add the current element to the record at its allocated position.
		Record_AddScalar(r, ctx->variable_idx, current_elem);

		// If any element in an ANY function passes the predicate, return true.
		if(FilterTree_applyFilters(ctx->ft, r)) return SI_BoolVal(true);

	}

	// No element passed, return false.
	return SI_BoolVal(false);
}

SIValue AR_ALL(SIValue *argv, int argc) {
	if(SI_TYPE(argv[0]) == T_NULL) return SI_NullVal();
	/* ALL comprehensions are invoked with three children:
	 * The list to iterate over.
	 * The current Record.
	 * The function context. */
	SIValue list = argv[0];
	Record outer_record = argv[1].ptrval;
	ListComprehensionCtx *ctx = argv[2].ptrval;

	// On the first invocation, build the local Record.
	if(ctx->local_record == NULL) _PopulateComprehensionCtx(ctx, outer_record);
	Record r = ctx->local_record;

	// Populate the local Record with the contents of the outer Record.
	Record_Clone(outer_record, r);

	uint len = SIArray_Length(list);
	for(uint i = 0; i < len; i++) {
		// Retrieve the current element.
		SIValue current_elem = SIArray_Get(list, i);
		// Add the current element to the record at its allocated position.
		Record_AddScalar(r, ctx->variable_idx, current_elem);

		// If any element in an ALL function does not pass the predicate, return false.
		if(!FilterTree_applyFilters(ctx->ft, r)) return SI_BoolVal(false);

	}

	// All elements passed, return true.
	return SI_BoolVal(true);
}


SIValue AR_LIST_COMPREHENSION(SIValue *argv, int argc) {
	if(SI_TYPE(argv[0]) == T_NULL) return SI_NullVal();
	/* List comprehensions are invoked with three children:
	 * The list to iterate over.
	 * The current Record.
	 * The function context. */
	SIValue list = argv[0];
	Record outer_record = argv[1].ptrval;
	ListComprehensionCtx *ctx = argv[2].ptrval;

	// On the first invocation, build the local Record.
	if(ctx->local_record == NULL) _PopulateComprehensionCtx(ctx, outer_record);
	Record r = ctx->local_record;

	// Populate the local Record with the contents of the outer Record.
	Record_Clone(outer_record, r);
	// Instantiate the array to be returned.
	SIValue retval = SI_Array(0);

	uint len = SIArray_Length(list);
	for(uint i = 0; i < len; i++) {
		// Retrieve the current element.
		SIValue current_elem = SIArray_Get(list, i);
		// Add the current element to the record at its allocated position.
		Record_AddScalar(r, ctx->variable_idx, current_elem);

		/* If the comprehension has a filter tree, run the current element through it.
		 * If it does not pass, skip this element. */
		if(ctx->ft && !(FilterTree_applyFilters(ctx->ft, r))) continue;

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
	array_append(types, T_ARRAY | T_NULL);
	array_append(types, T_PTR);
	array_append(types, T_PTR);
	func_desc = AR_FuncDescNew("any", AR_ANY, 3, 3, types, true, false);
	AR_SetPrivateDataRoutines(func_desc, ListComprehension_Free, ListComprehension_Clone);
	AR_RegFunc(func_desc);
	func_desc->bfree = ListComprehension_Free;
	func_desc->bclone = ListComprehension_Clone;

	types = array_new(SIType, 3);
	array_append(types, T_ARRAY | T_NULL);
	array_append(types, T_PTR);
	array_append(types, T_PTR);
	func_desc = AR_FuncDescNew("all", AR_ALL, 3, 3, types, true, false);
	AR_SetPrivateDataRoutines(func_desc, ListComprehension_Free, ListComprehension_Clone);
	AR_RegFunc(func_desc);
	func_desc->bfree = ListComprehension_Free;
	func_desc->bclone = ListComprehension_Clone;

	types = array_new(SIType, 3);
	array_append(types, T_ARRAY | T_NULL);
	array_append(types, T_PTR);
	array_append(types, T_PTR);
	func_desc = AR_FuncDescNew("list_comprehension", AR_LIST_COMPREHENSION, 3, 3, types, true, false);
	AR_SetPrivateDataRoutines(func_desc, ListComprehension_Free, ListComprehension_Clone);
	AR_RegFunc(func_desc);
	func_desc->bfree = ListComprehension_Free;
	func_desc->bclone = ListComprehension_Clone;
}

