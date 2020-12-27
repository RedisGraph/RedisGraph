/*
*Copyright 2018 - 2019 Redis Labs Ltd. and Contributors
*
*This file is available under the Redis Labs Source Available License Agreement
*/

#include "placeholder_funcs.h"
#include "../func_desc.h"
#include "../../util/arr.h"

SIValue AR_PATH_FILTER(SIValue *argv, int argc) {
	ASSERT(false);
	return SI_NullVal();
}

void Register_PlaceholderFuncs() {
	SIType *types;
	AR_FuncDesc *func_desc;

	types = array_new(SIType, 0);
	func_desc = AR_FuncDescNew("path_filter", AR_PATH_FILTER, 0, 0, types, false, false);
	AR_RegFunc(func_desc);
}

