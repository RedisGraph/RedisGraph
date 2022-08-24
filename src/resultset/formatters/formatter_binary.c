/*
 * Copyright 2018-2022 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "resultset_formatters.h"
#include "../../util/binary_buffer.h"
#include "../../datatypes/datatypes.h"

static void Formatter_Binary_ReplyWithString
(
	const char *s,
	BinaryBuffer buf
) {
	// format
	// type          1 byte
	// string length 8 bytes
	// string        n bytes

	unsigned char data[sizeof(size_t)+1];

	// write both type and length at once
	data[0] = (unsigned char)VALUE_STRING;
	*((size_t*)(data+1)) = strlen(s);

	//BinaryBuffer_Write(buf, (unsigned char)VALUE_STRING);
	//BinaryBuffer_Write(buf, strlen(s));

	BinaryBuffer_WriteBytes(buf, data, sizeof(size_t) + 1);
	BinaryBuffer_Write(buf, s);
}

static void Formatter_Binary_ReplyWithInt64
(
	int64_t n,
	BinaryBuffer buf	
) {
	// format
	// type   1 byte
	// number 8 bytes

	unsigned char data[9];
	data[0] = (unsigned char)VALUE_INTEGER;
	*((int64_t*)(data+1)) = n;

	BinaryBuffer_WriteBytes(buf, data, sizeof(size_t) + 1);

	// BinaryBuffer_Write(buf, (unsigned char)VALUE_INTEGER);
	// BinaryBuffer_Write(buf, n);
}

static void Formatter_Binary_ReplyWithDobule
(
	double n,
	BinaryBuffer buf	
) {
	// format
	// type   1 byte
	// number 4 bytes

	unsigned char data[sizeof(double) + 1];
	data[0] = (unsigned char)VALUE_DOUBLE;
	*((double*)(data+1)) = n;
	BinaryBuffer_WriteBytes(buf, data, sizeof(size_t) + 1);

	// BinaryBuffer_Write(buf, (unsigned char)VALUE_DOUBLE);
	// BinaryBuffer_Write(buf, n);
}

static void Formatter_Binary_ReplyWithSIValue
(
	SIValue *v,
	BinaryBuffer buf
) {
	switch(v->type) {
	case T_STRING:
		Formatter_Binary_ReplyWithString(v->stringval, buf);
		return;
	case T_INT64:
		Formatter_Binary_ReplyWithInt64(v->longval, buf);
		return;
	case T_DOUBLE:
		Formatter_Binary_ReplyWithDobule(v->doubleval, buf);
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

// create binary formatter private data
void *Formatter_Binary_CreatePData(void) {
	return BinaryBuffer_New();
}

// process result-set row
void Formatter_Binary_ProcessRow
(
	Record r,           // record containing projected values
	uint *columns_map,  // mapping between record entries to columns
	uint ncols,         // length of row
	void *pdata         // formatter's private data
) {
	BinaryBuffer buf = (BinaryBuffer)pdata;

	for(int i = 0; i < ncols; i++) {
		int idx = columns_map[i];
		SIValue v = Record_Get(r, idx);
		Formatter_Binary_ReplyWithSIValue(&v, buf);
	}
}

void Formatter_Binary_EmitHeader
(
	RedisModuleCtx *ctx,
	const char **columns
) {
	uint columns_len = array_len(columns);
	RedisModule_ReplyWithArray(ctx, columns_len);
	for(uint i = 0; i < columns_len; i++) {
		RedisModule_ReplyWithArray(ctx, 2);
		/* Because the types found in the first Record do not necessarily inform the types
		 * in subsequent records, we will always set the column type as scalar. */
		ColumnType t = COLUMN_SCALAR;
		RedisModule_ReplyWithLongLong(ctx, t);

		// Second, emit the identifier string associated with the column
		RedisModule_ReplyWithStringBuffer(ctx, columns[i], strlen(columns[i]));
	}
}

// emit formatted data
void Formatter_Binary_EmitData
(
	RedisModuleCtx *ctx,  // redis module context
	GraphContext *gc,     // graph context
	uint ncols,           // length of row
	void *pdata           // formatter's private data
) {
	BinaryBuffer buf = (BinaryBuffer)pdata;
	uint64_t n = BinaryBuffer_InternalBuffersCount(buf);

	// TODO: consider switching to a heap allocation for really large ns
	char *buffers[n]; 
	size_t caps[n];
	size_t counts[n];

	// TODO: maybe we should gain direct access to the buffers?
	// BinaryBuffer_InternalBuffers performs an internal loop...
	BinaryBuffer_InternalBuffers(buf, n, buffers, counts, caps);

	RedisModule_ReplyWithArray(ctx, n);
	for(uint i = 0; i < n; i++) {
		size_t len = counts[i];
		char *buffer = buffers[i];
		RedisModule_ReplyWithStringBuffer(ctx, buffer, len);
	}
}

// free binary formatter private data
void Formatter_Binary_FreePData
(
	void *pdata
) {
	BinaryBuffer buff = (BinaryBuffer)pdata;
	BinaryBuffer_Free(&buff);
}

