/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "repository.h"
#include "../util/rmalloc.h"
#include "rax.h"
#include <ctype.h>
#include <assert.h>

static rax *__aggRegisteredFuncs = NULL;

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

static void __agg_initRegistry() {
	if(__aggRegisteredFuncs == NULL) {
		__aggRegisteredFuncs = raxNew();
	}
}

void Agg_RegisterFunc(const char *name, AggFuncInit f) {
	__agg_initRegistry();
	char lower_func_name[32] = {0};
	short lower_func_name_len = 32;
	_toLower(name, &lower_func_name[0], &lower_func_name_len);
	raxInsert(__aggRegisteredFuncs, (unsigned char *)lower_func_name, lower_func_name_len, f, NULL);
}

bool Agg_FuncExists(const char *name) {
	if(!__aggRegisteredFuncs) return false;

	char lower_func_name[32] = {0};
	short lower_func_name_len = 32;
	_toLower(name, &lower_func_name[0], &lower_func_name_len);
	return raxFind(__aggRegisteredFuncs, (unsigned char *)lower_func_name,
				   lower_func_name_len) != raxNotFound;
}

void Agg_GetFunc(const char *name, bool distinct, AggCtx **ctx) {
	*ctx = NULL;
	if(!__aggRegisteredFuncs) return;

	char lower_func_name[32] = {0};
	short lower_func_name_len = 32;
	_toLower(name, &lower_func_name[0], &lower_func_name_len);
	AggFuncInit f = raxFind(__aggRegisteredFuncs, (unsigned char *)lower_func_name,
							lower_func_name_len);
	if(f != raxNotFound) *ctx = f(distinct);
}

