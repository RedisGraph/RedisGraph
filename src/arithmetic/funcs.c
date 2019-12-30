
/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "funcs.h"
#include "../../deps/rax/rax.h"
#include <assert.h>

extern rax *__aeRegisteredFuncs;

void AR_RegisterFuncs() {
	assert(!__aeRegisteredFuncs);
	__aeRegisteredFuncs = raxNew();

	Register_ListFuncs();
	Register_TimeFuncs();
	Register_EntityFuncs();
	Register_StringFuncs();
	Register_NumericFuncs();
	Register_BooleanFuncs();
	Register_ConditionalFuncs();
	Register_PathFuncs();
	Register_PlaceholderFuncs();
}

