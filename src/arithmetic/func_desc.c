/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "func_desc.h"
#include "../RG.h"
#include "../util/rmalloc.h"
#include "../util/strutil.h"
#include "../../deps/rax/rax.h"
#include "aggregate_funcs/agg_funcs.h"
#include <ctype.h>

// Arithmetic function repository
rax *__aeRegisteredFuncs = NULL;

AR_FuncDesc *AR_FuncDescNew
(
	const char *name,
	AR_Func func,
	uint min_argc,
	uint max_argc,
	SIType *types,
	bool reducible
) {
	AR_FuncDesc *desc = rm_calloc(1, sizeof(AR_FuncDesc));

	desc->name                    =  name;
	desc->func                    =  func;
	desc->types                   =  types;
	desc->min_argc                =  min_argc;
	desc->max_argc                =  max_argc;
	desc->aggregate               =  false;
	desc->reducible               =  reducible;

	return desc;
}

AR_FuncDesc *AR_AggFuncDescNew
(
	const char *name,                   // function name
	AR_Func func,                       // pointer to function
	uint min_argc,                      // minimum number of arguments
	uint max_argc,                      // maximum number of arguments
	SIType *types,                      // acceptable types
	AR_Func_Free free,                  // free aggregation callback
	AR_Func_Clone clone,                // clone aggregation callback
	AR_Func_Finalize finalize,          // finalize aggregation callback
	AR_Func_DefaultValue default_value  // default value callback
) {
	AR_FuncDesc *desc = rm_calloc(1, sizeof(AR_FuncDesc));

	desc->name                         =  name;
	desc->func                         =  func;
	desc->types                        =  types;
	desc->min_argc                     =  min_argc;
	desc->max_argc                     =  max_argc;
	desc->aggregate                    =  true;
	desc->reducible                    =  false;
	desc->agg_callbacks.bfree          =  free;
	desc->agg_callbacks.bclone         =  clone;
	desc->agg_callbacks.finalize       =  finalize;
	desc->agg_callbacks.default_value  =  default_value;

	return desc;
}

// register an arithmetic function
void AR_RegFunc
(
	AR_FuncDesc *func
) {
	char lower_func_name[32];
	size_t lower_func_name_len = 32;
	str_tolower(func->name, lower_func_name, &lower_func_name_len);
	int res = raxInsert(__aeRegisteredFuncs, (unsigned char *)lower_func_name,
			lower_func_name_len, func, NULL);
	ASSERT(res == 1);
}

// get arithmetic function
AR_FuncDesc *AR_GetFunc
(
	const char *func_name
) {
	size_t len = strlen(func_name);
	char lower_func_name[len];
	str_tolower(func_name, lower_func_name, &len);
	void *f = raxFind(__aeRegisteredFuncs, (unsigned char *)lower_func_name, len);

	if(f == raxNotFound) return NULL;

	AR_FuncDesc *func = (AR_FuncDesc*)f;

	if(func->aggregate) {
		// clone function descriptor
		func = rm_malloc(sizeof(AR_FuncDesc));
		memcpy(func, f, sizeof(AR_FuncDesc));

		// create aggregation context
		AggregateCtx *ctx = rm_malloc(sizeof(AggregateCtx));
		ctx->private_ctx  =  NULL;
		ctx->result       =  SI_CloneValue(func->default_value);

		func->privdata = ctx;
	}

	return func;
}

bool AR_FuncExists
(
	const char *func_name
) {
	size_t len = strlen(func_name);
	char lower_func_name[len];
	str_tolower(func_name, lower_func_name, &len);
	void *f = raxFind(__aeRegisteredFuncs, (unsigned char *)lower_func_name, len);

	return f != raxNotFound;
}

bool AR_FuncIsAggregate
(
	const char *func_name
) {
	size_t len = strlen(func_name);
	char lower_func_name[len];
	str_tolower(func_name, lower_func_name, &len);
	AR_FuncDesc *f = raxFind(__aeRegisteredFuncs,
			(unsigned char *)lower_func_name, len);

	if(f == raxNotFound) return false;

	return f->aggregate;
}

void AR_Finalize
(
	AR_FuncDesc *func_desc
) {
	if(func_desc->agg_callbacks.finalize) {
		func_desc->agg_callbacks.finalize(func_desc->privdata);
	}
}

// TODO: might need to be removed
AR_FuncDesc *AR_SetPrivateData
(
	const AR_FuncDesc *orig,
	void *privdata
) {
	// create a new function descriptor
	AR_FuncDesc *func = rm_malloc(sizeof(AR_FuncDesc));
	memcpy(func, orig, sizeof(AR_FuncDesc));

	// set the private data pointer
	func->privdata = privdata;

	return func;
}

// TODO: might need to be removed
AR_FuncDesc *AR_CloneFuncDesc
(
	const AR_FuncDesc *orig
) {
	ASSERT(orig->agg_callbacks.bclone);

	// perform a shallow copy of the input function descriptor
	AR_FuncDesc *clone = rm_malloc(sizeof(AR_FuncDesc));
	memcpy(clone, orig, sizeof(AR_FuncDesc));

	// clone the function's private data
	clone->privdata = orig->agg_callbacks.bclone(orig->privdata);

	return clone;
}

