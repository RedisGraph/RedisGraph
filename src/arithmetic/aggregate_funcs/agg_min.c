/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "RG.h"
#include "agg_funcs.h"
#include "../func_desc.h"
#include "../../util/arr.h"

//------------------------------------------------------------------------------
// Min
//------------------------------------------------------------------------------

AggregateResult AGG_MIN(SIValue *argv, int argc, void *private_data) {
	SIValue v = argv[0];
	if(SI_TYPE(v) == T_NULL) return AGGREGATE_OK;
	AggregateCtx *ctx = private_data;

	// Update the result if the current element is lesser.
	int compared_null;
	if((SIValue_Compare(ctx->result, v, &compared_null) > 0) ||
	   (compared_null == COMPARED_NULL)) {
		SIValue_Free(ctx->result);
		ctx->result = SI_CloneValue(v);
	}

	return AGGREGATE_OK;
}

AggregateCtx *Min_PrivateData(void)
{
	AggregateCtx *ctx = rm_malloc(sizeof(AggregateCtx));

	ctx->result = SI_NullVal();  // min default value is NULL
	ctx->private_data = NULL;

	return ctx;
}

void Register_MIN(void) {
	SIType *types;
	SIType ret_type;
	AR_FuncDesc *func_desc;

	types = array_new(SIType, 2);
	array_append(types, SI_ALL);
	ret_type = SI_ALL;
	func_desc = AR_AggFuncDescNew("min", AGG_MIN, 1, 1, types, ret_type, NULL,
			NULL, Min_PrivateData);
	AR_RegFunc(func_desc);
}

