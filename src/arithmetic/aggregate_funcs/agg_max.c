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
// Max
//------------------------------------------------------------------------------

AggregateResult AGG_MAX(SIValue *argv, int argc) {
	SIValue v = argv[0];
	if(SI_TYPE(v) == T_NULL) return AGGREGATE_OK;
	AggregateCtx *ctx = argv[1].ptrval;

	// Update the result if the current element is greater.
	int compared_null;
	if((SIValue_Compare(ctx->result, v, &compared_null) < 0) ||
	   (compared_null == COMPARED_NULL)) {
		ctx->result = v;
	}

	return AGGREGATE_OK;
}

AggregateCtx *Max_PrivateData(void)
{
	AggregateCtx *ctx = rm_malloc(sizeof(AggregateCtx));

	ctx->result = SI_NullVal();  // max default value is NULL
	ctx->private_data = NULL;

	return ctx;
}

void Register_MAX(void) {
	SIType *types;
	AR_FuncDesc *func_desc;

	types = array_new(SIType, 1);
	array_append(types, SI_ALL);
	func_desc = AR_AggFuncDescNew("max", AGG_MAX, 1, 1, types, NULL, NULL,
			Max_PrivateData);
	AR_RegFunc(func_desc);
}

