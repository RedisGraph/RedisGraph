/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "time_funcs.h"
#include "../func_desc.h"
#include "../../util/arr.h"
#include "../../datatypes/temporal_value.h"
#include <assert.h>

/* returns a timestamp - millis from epoch */
SIValue AR_TIMESTAMP(SIValue *argv, int argc) {
	return SI_LongVal(TemporalValue_NewTimestamp());
}

void Register_TimeFuncs() {
	SIType *types;
	AR_FuncDesc *func_desc;

	types = array_new(SIType, 0);
	func_desc = AR_FuncDescNew("timestamp", AR_TIMESTAMP, 0, types, false);
	AR_RegFunc(func_desc);
}

