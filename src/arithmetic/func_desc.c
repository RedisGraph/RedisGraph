/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
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
	bool reducible,
	bool aggregate
) {
	AR_FuncDesc *desc = rm_malloc(sizeof(AR_FuncDesc));

	desc->name       =  name;
	desc->func       =  func;
	desc->bfree      =  NULL;
	desc->bclone     =  NULL;
	desc->types      =  types;
	desc->finalize   =  NULL;
	desc->privdata   =  NULL;
	desc->min_argc   =  min_argc;
	desc->max_argc   =  max_argc;
	desc->aggregate  =  aggregate;
	desc->reducible  =  reducible;

	return desc;
}

// register an arithmetic function
void AR_RegFunc(AR_FuncDesc *func) {
	char lower_func_name[32];
	size_t lower_func_name_len = 32;
	str_tolower(func->name, lower_func_name, &lower_func_name_len);
	int res = raxInsert(__aeRegisteredFuncs, (unsigned char *)lower_func_name, lower_func_name_len,
						func, NULL);
	ASSERT(res == 1);
}

// get arithmetic function
AR_FuncDesc *AR_GetFunc(const char *func_name) {

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
		ctx->result       =  SI_NullVal();

		func->privdata = ctx;
	}

	return func;

}

bool AR_FuncExists(const char *func_name) {
	size_t len = strlen(func_name);
	char lower_func_name[len];
	str_tolower(func_name, lower_func_name, &len);
	void *f = raxFind(__aeRegisteredFuncs, (unsigned char *)lower_func_name, len);

	return f != raxNotFound;
}

bool AR_FuncIsAggregate(const char *func_name) {
	size_t len = strlen(func_name);
	char lower_func_name[len];
	str_tolower(func_name, lower_func_name, &len);
	AR_FuncDesc *f = raxFind(__aeRegisteredFuncs, (unsigned char *)lower_func_name, len);

	if(f == raxNotFound) return false;

	return f->aggregate;
}

inline void AR_SetPrivateDataRoutines(AR_FuncDesc *func_desc, AR_Func_Free bfree,
									  AR_Func_Clone bclone) {
	func_desc->bfree = bfree;
	func_desc->bclone = bclone;
}

void AR_SetFinalizeRoutine(AR_FuncDesc *func_desc, AR_Func_Finalize finalize) {
	func_desc->finalize = finalize;
}

void AR_Finalize(AR_FuncDesc *func_desc) {
	if(func_desc->finalize) func_desc->finalize(func_desc->privdata);
}

AR_FuncDesc *AR_SetPrivateData(const AR_FuncDesc *orig, void *privdata) {
	// create a new function descriptor
	AR_FuncDesc *func = rm_malloc(sizeof(AR_FuncDesc));
	memcpy(func, orig, sizeof(AR_FuncDesc));

	// set the private data pointer
	func->privdata = privdata;

	return func;
}

AR_FuncDesc *AR_CloneFuncDesc(const AR_FuncDesc *orig) {
	ASSERT(orig->bclone);
	// Perform a shallow copy of the input function descriptor.
	AR_FuncDesc *clone = rm_malloc(sizeof(AR_FuncDesc));
	memcpy(clone, orig, sizeof(AR_FuncDesc));
	// Clone the function's private data.
	clone->privdata = orig->bclone(orig->privdata);

	return clone;
}

