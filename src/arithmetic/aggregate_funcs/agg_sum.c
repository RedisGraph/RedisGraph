/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "RG.h"
#include "agg_funcs.h"

//------------------------------------------------------------------------------
// Sum
//------------------------------------------------------------------------------

AggregateResult AGG_SUM(SIValue *argv, int argc) {
	AggregateCtx *ctx = argv[1].ptrval;

	SIValue v = argv[0];
	if(SI_TYPE(v) == T_NULL) return AGGREGATE_OK;

	// Update the total.
	if(SI_TYPE(v) != T_NULL) ctx->result.doubleval += SI_GET_NUMERIC(v);

	return AGGREGATE_OK;
}

Register_SUM(void) {
	SIType *types;
	AR_FuncDesc *func_desc;

	types = array_new(SIType, 2);
	array_append(types, T_NULL | T_INT64 | T_DOUBLE);
	array_append(types, T_PTR);
	func_desc = AR_AggFuncDescNew("sum", AGG_SUM, 2, 2, types, Aggregate_Free,
			NULL, Default_Double);
	AR_RegFunc(func_desc);
}

