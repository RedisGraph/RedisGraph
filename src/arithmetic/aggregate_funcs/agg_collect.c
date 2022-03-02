/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "RG.h"
#include "agg_funcs.h"

//------------------------------------------------------------------------------
// Collect
//------------------------------------------------------------------------------

AggregateResult AGG_COLLECT(SIValue *argv, int argc) {
	AggregateCtx *ctx = argv[1].ptrval;

	SIValue v = argv[0];
	if(SI_TYPE(v) == T_NULL) return AGGREGATE_OK;

	// SIArray_Append will clone the added value, ensuring it can be
	// safely accessed for the lifetime of the Collect context.
	SIArray_Append(&ctx->result, v);

	return AGGREGATE_OK;
}

Register_COLLECT(void) {
	SIType *types;
	AR_FuncDesc *func_desc;

	types = array_new(SIType, 2);
	array_append(types, SI_ALL);
	array_append(types, T_PTR);
	func_desc = AR_AggFuncDescNew("collect", AGG_COLLECT, 2, 2, types,
			Aggregate_Free, NULL, Default_Array);
	AR_RegFunc(func_desc);
}

