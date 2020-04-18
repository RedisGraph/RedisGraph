/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "encoder_common.h"

static void _RdbSaveSIArray(RedisModuleIO *rdb, const SIValue list) {
	/* saves array as
	   unsigned : array legnth
	   array[0]
	   .
	   .
	   .
	   array[array length -1]
	 */
	uint arrayLen = SIArray_Length(list);
	RedisModule_SaveUnsigned(rdb, arrayLen);
	for(uint i = 0; i < arrayLen; i ++) {
		SIValue value = SIArray_Get(list, i);
		RdbSaveSIValue(rdb, &value);
	}
}

void RdbSaveSIValue(RedisModuleIO *rdb, const SIValue *v) {
	/* Format:
	 * SIType
	 * Value */
	RedisModule_SaveUnsigned(rdb, v->type);
	switch(v->type) {
	case T_BOOL:
	case T_INT64:
		RedisModule_SaveSigned(rdb, v->longval);
		return;
	case T_DOUBLE:
		RedisModule_SaveDouble(rdb, v->doubleval);
		return;
	case T_STRING:
		RedisModule_SaveStringBuffer(rdb, v->stringval, strlen(v->stringval) + 1);
		return;
	case T_ARRAY:
		_RdbSaveSIArray(rdb, *v);
		return;
	case T_NULL:
		return; // No data beyond the type needs to be encoded for a NULL value.
	default:
		assert(0 && "Attempted to serialize value of invalid type.");
	}
}
