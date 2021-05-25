
/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "funcs.h"
#include "../RG.h"
#include "../../deps/rax/rax.h"

extern rax *__aeRegisteredFuncs;

void AR_RegisterFuncs() {
	ASSERT(__aeRegisteredFuncs == NULL);
	__aeRegisteredFuncs = raxNew();

	Register_AggFuncs();
	Register_MapFuncs();
	Register_PathFuncs();
	Register_ListFuncs();
	Register_TimeFuncs();
	Register_PointFuncs();
	Register_EntityFuncs();
	Register_StringFuncs();
	Register_NumericFuncs();
	Register_BooleanFuncs();
	Register_ConditionalFuncs();
	Register_ComprehensionFuncs();
	Register_PlaceholderFuncs();
}

