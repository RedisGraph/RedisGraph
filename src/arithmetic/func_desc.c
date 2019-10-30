/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "func_desc.h"
#include "../util/rmalloc.h"
#include "../../deps/rax/rax.h"
#include <ctype.h>
#include <assert.h>

/* Arithmetic function repository. */
rax *__aeRegisteredFuncs = NULL;

static void inline _toLower(const char *str, char *lower, short *lower_len) {
	size_t str_len = strlen(str);
	/* Avoid overflow. */
	assert(*lower_len > str_len);

	/* Update lower len*/
	*lower_len = str_len;

	int i = 0;
	for(; i < str_len; i++) lower[i] = tolower(str[i]);
	lower[i] = 0;
}

AR_FuncDesc *AR_FuncDescNew(const char *name, AR_Func func, uint argc, SIType *types,
							bool reducible) {
	AR_FuncDesc *desc = rm_malloc(sizeof(AR_FuncDesc));
	desc->name = name;
	desc->func = func;
	desc->argc = argc;
	desc->types = types;
	desc->reducible = reducible;
	return desc;
}

/* Register an arithmetic function. */
void AR_RegFunc(AR_FuncDesc *func) {
	char lower_func_name[32] = {0};
	short lower_func_name_len = 32;
	_toLower(func->name, &lower_func_name[0], &lower_func_name_len);
	assert(raxInsert(__aeRegisteredFuncs, (unsigned char *)lower_func_name, lower_func_name_len, func,
					 NULL) == 1);
}

/* Get arithmetic function. */
AR_FuncDesc *AR_GetFunc(const char *func_name) {
	char lower_func_name[32] = {0};
	short lower_func_name_len = 32;
	_toLower(func_name, &lower_func_name[0], &lower_func_name_len);
	void *f = raxFind(__aeRegisteredFuncs, (unsigned char *)lower_func_name, lower_func_name_len);

	if(f != raxNotFound) return f;
	return NULL;
}

bool AR_FuncExists(const char *func_name) {
	return (AR_GetFunc(func_name) != NULL);
}

