/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "op.h"
#include "../../util/rmalloc.h"
#include "../../util/simple_timer.h"

#include <assert.h>

void OpBase_Init(OpBase *op) {
	op->modifies = NULL;
	op->childCount = 0;
	op->children = NULL;
	op->parent = NULL;
	op->stats = NULL;
	op->record_map = NULL;
	op->op_initialized = false;
	op->dangling_records = NULL;

	// Function pointers.
	op->init = NULL;
	op->free = NULL;
	op->reset = NULL;
	op->consume = NULL;
	op->toString = NULL;
}

inline Record OpBase_Consume(OpBase *op) {
	return op->consume(op);
}

void OpBase_PropagateReset(OpBase *op) {
	assert(op->reset(op) == OP_OK);
	for(int i = 0; i < op->childCount; i++) OpBase_PropagateReset(op->children[i]);
}

void OpBase_PropagateFree(OpBase *op) {
	/* TODO: decide rather or not we want to perform
	 * op->free or OpBase_Free. */
	op->free(op);
	for(int i = 0; i < op->childCount; i++) OpBase_PropagateFree(op->children[i]);
	// OpBase_Free(op);
}

static int _OpBase_StatsToString(const OpBase *op, char *buff, uint buff_len) {
	return snprintf(buff, buff_len,
					" | Records produced: %d, Execution time: %f ms",
					op->stats->profileRecordCount,
					op->stats->profileExecTime);
}

int OpBase_ToString(const OpBase *op, char *buff, uint buff_len) {
	int bytes_written = 0;

	if(op->toString) bytes_written = op->toString(op, buff, buff_len);
	else bytes_written = snprintf(buff, buff_len, "%s", op->name);

	if(op->stats) {
		bytes_written += _OpBase_StatsToString(op,
											   buff + bytes_written,
											   buff_len - bytes_written);
	}

	return bytes_written;
}

Record OpBase_Profile(OpBase *op) {
	double tic [2];
	// Start timer.
	simple_tic(tic);
	Record r = op->profile(op);
	// Stop timer and accumulate.
	op->stats->profileExecTime += simple_toc(tic);
	if(r) op->stats->profileRecordCount++;
	return r;
}

void OpBase_AddVolatileRecord(OpBase *op, const Record r) {
	if(op->dangling_records == NULL) op->dangling_records = array_new(Record, 1);
	op->dangling_records = array_append(op->dangling_records, r);
}

void OpBase_RemoveVolatileRecords(OpBase *op) {
	if(!op->dangling_records) return;

	array_clear(op->dangling_records);
}

void OpBase_Free(OpBase *op) {
	// Free internal operation
	op->free(op);
	if(op->children) rm_free(op->children);
	if(op->modifies) array_free(op->modifies);
	if(op->stats) rm_free(op->stats);
	// If we are storing dangling references to Records, free them now.
	if(op->dangling_records) {
		uint count = array_len(op->dangling_records);
		for(uint i = 0; i < count; i ++) {
			Record_Free(op->dangling_records[i]);
		}
		array_free(op->dangling_records);
	}
	free(op);
}

