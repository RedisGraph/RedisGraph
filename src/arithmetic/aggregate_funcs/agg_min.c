/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "RG.h"
#include "agg_funcs.h"

//------------------------------------------------------------------------------
// Min
//------------------------------------------------------------------------------

AggregateResult AGG_MIN(SIValue *argv, int argc) {
	SIValue v = argv[0];
	if(SI_TYPE(v) == T_NULL) return AGGREGATE_OK;
	AggregateCtx *ctx = argv[1].ptrval;

	// Update the result if the current element is lesser.
	int compared_null;
	if((SIValue_Compare(ctx->result, v, &compared_null) > 0) ||
	   (compared_null == COMPARED_NULL)) {
		ctx->result = v;
	}

	return AGGREGATE_OK;
}

Register_MIN(void) {
	SIType *types;
	AR_FuncDesc *func_desc;

	types = array_new(SIType, 2);
	array_append(types, SI_ALL);
	array_append(types, T_PTR);
	func_desc = AR_AggFuncDescNew("min", AGG_MIN, 2, 2, types, Aggregate_Free,
			NULL, SI_NullVal);
	AR_RegFunc(func_desc);
}

