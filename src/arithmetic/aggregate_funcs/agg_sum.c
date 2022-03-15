/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "RG.h"
#include "agg_funcs.h"
#include "../func_desc.h"
#include "../../util/arr.h"

//------------------------------------------------------------------------------
// Sum
//------------------------------------------------------------------------------

AggregateResult AGG_SUM(SIValue *argv, int argc, void *private_data) {
	AggregateCtx *ctx = private_data;

	SIValue v = argv[0];
	if(SI_TYPE(v) == T_NULL) return AGGREGATE_OK;

	// Update the total.
	if(SI_TYPE(v) != T_NULL) ctx->result.doubleval += SI_GET_NUMERIC(v);

	return AGGREGATE_OK;
}

AggregateCtx *SUM_PrivateData(void)
{
	AggregateCtx *ctx = rm_malloc(sizeof(AggregateCtx));

	ctx->result = SI_DoubleVal(0);  // SUM default value is 0
	ctx->private_data = NULL;

	return ctx;
}

void Register_SUM(void) {
	SIType *types;
	AR_FuncDesc *func_desc;

	types = array_new(SIType, 2);
	array_append(types, T_NULL | T_INT64 | T_DOUBLE);
	func_desc = AR_AggFuncDescNew("sum", AGG_SUM, 1, 1, types, NULL, NULL,
			SUM_PrivateData);
	AR_RegFunc(func_desc);
}

