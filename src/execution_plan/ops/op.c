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

	// Function pointers.
	op->init = NULL;
	op->free = NULL;
	op->reset = NULL;
	op->consume = NULL;
	op->toString = NULL;

	op->record_map = NULL;
}

inline Record OpBase_Consume(OpBase *op) {
	return op->consume(op);
}

void OpBase_Reset(OpBase *op) {
	assert(op->reset(op) == OP_OK);
	for(int i = 0; i < op->childCount; i++) OpBase_Reset(op->children[i]);
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

void OpBase_Free(OpBase *op) {
	// Free internal operation
	op->free(op);
	if(op->children) rm_free(op->children);
	if(op->modifies) array_free(op->modifies);
	if(op->stats) rm_free(op->stats);
	free(op);
}
