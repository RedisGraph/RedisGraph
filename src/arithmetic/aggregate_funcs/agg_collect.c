/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "RG.h"
#include "agg_funcs.h"
#include "../func_desc.h"
#include "../../util/arr.h"
#include "../../datatypes/array.h"

//------------------------------------------------------------------------------
// Collect
//------------------------------------------------------------------------------

AggregateResult AGG_COLLECT(SIValue *argv, int argc, void *private_data) {
	AggregateCtx *ctx = private_data;

	SIValue v = argv[0];
	if(SI_TYPE(v) == T_NULL) return AGGREGATE_OK;

	// SIArray_Append will clone the added value, ensuring it can be
	// safely accessed for the lifetime of the Collect context.
	SIArray_Append(&ctx->result, v);

	return AGGREGATE_OK;
}

AggregateCtx *Collect_PrivateData(void)
{
	AggregateCtx *ctx = rm_malloc(sizeof(AggregateCtx));

	ctx->result = SI_Array(0);  // collect default value is an empty array
	ctx->private_data = NULL;

	return ctx;
}

void Register_COLLECT(void) {
	SIType *types;
	AR_FuncDesc *func_desc;

	types = array_new(SIType, 2);
	array_append(types, SI_ALL);
	func_desc = AR_AggFuncDescNew("collect", AGG_COLLECT, 1, 1, types,
			NULL, NULL, Collect_PrivateData);
	AR_RegFunc(func_desc);
}

