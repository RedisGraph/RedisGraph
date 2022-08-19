/*
 * Copyright 2018-2022 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "resultset_formatters.h"
#include "../../util/binary_buffer.h"
#include "../../datatypes/datatypes.h"

static void _ResultSet_BinaryReplyWithString
(
	const char *s,
	BinaryBuffer *buf
) {
	BinaryBuffer_Write(buf, (unsigned char)VALUE_STRING);
	BinaryBuffer_Write(buf, strlen(s));
	BinaryBuffer_Write(buf, s);
}

static void _ResultSet_BinaryReplyWithInt64
(
	int64_t n,
	BinaryBuffer *buf	
) {
	BinaryBuffer_Write(buf, (unsigned char)VALUE_INTEGER);
	BinaryBuffer_Write(buf, n);
}

static void _ResultSet_BinaryReplyWithDobule
(
	double n,
	BinaryBuffer *buf	
) {
	BinaryBuffer_Write(buf, (unsigned char)VALUE_DOUBLE);
	BinaryBuffer_Write(buf, n);
}

static void _ResultSet_BinaryReplyWithSIValue
(
	GraphContext *gc,
	SIValue *v,
	BinaryBuffer *buf
) {
	switch(v->type) {
	case T_STRING:
		_ResultSet_BinaryReplyWithString(v->stringval, buf);
		return;
	case T_INT64:
		_ResultSet_BinaryReplyWithInt64(v->longval, buf);
		return;
	case T_DOUBLE:
		_ResultSet_BinaryReplyWithDobule(v->doubleval, buf);
		return;
	case T_BOOL:
		assert("not implemented" && false);
		return;
	case T_NULL:
		return;
	case T_NODE:
		assert("not implemented" && false);
		return;
	case T_EDGE:
		assert("not implemented" && false);
		return;
	case T_ARRAY:
		assert("not implemented" && false);
		return;
	case T_PATH:
		assert("not implemented" && false);
		return;
	case T_MAP:
		assert("not implemented" && false);
		return;
	case T_POINT:
		assert("not implemented" && false);
		return;
	default:
		RedisModule_Assert("Unhandled value type" && false);
	}
}

void ResultSet_ReplyWithBinaryHeader
(
	RedisModuleCtx *ctx,
	const char **columns,
	uint *col_rec_map,
	void *pdata
) {

}

void ResultSet_EmitBinaryRow
(
	RedisModuleCtx *ctx,
	GraphContext *gc,
	SIValue **row,
	uint numcols,
	void *pdata
) {
	for(int i = 0; i < numcols; i++) {
		SIValue *v = row[i];
		_ResultSet_BinaryReplyWithSIValue(gc, v, NULL);
	}
}

void *ResultSet_CreateBinaryPData(void) {
	return BinaryBuffer_New();
}

void ResultSet_FreeBinaryPData
(
	void *pdata
) {
	BinaryBuffer buff = (BinaryBuffer)pdata;
	BinaryBuffer_Free(&buff);
}

