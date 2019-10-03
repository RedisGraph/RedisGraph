/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "path_funcs.h"
#include "../func_desc.h"
#include "../../util/arr.h"
#include <assert.h>

SIValue AR_TRAVERSE(SIValue *argv, int argc) {
	assert(false);
	return SI_NullVal();
}

void Register_PathFuncs() {
	SIType *types;
	AR_FuncDesc *func_desc;

	types = array_new(SIType, 0);
	func_desc = AR_FuncDescNew("traverse", AR_TRAVERSE, 1, types, false);
	AR_RegFunc(func_desc);
}
