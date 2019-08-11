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
}

inline Record OpBase_Consume(OpBase *op) {
	return op->consume(op);
}

int OpBase_Modifies(OpBase *op, const char *alias) {
	if(!op->modifies) op->modifies = array_new(char *, 1);
	op->modifies = array_append(op->modifies, alias);

	/* Make sure alias has an entry associated with it
	 * within the record mapping. */
	void *id = raxFind(op->record_map, (unsigned char *)alias, strlen(alias));
	if(id == raxNotFound) {
		id = (void *)raxSize(op->record_map);
		raxInsert(op->record_map, (unsigned char *)alias, strlen(alias), id, NULL);
	}

	return (int)id;
}

bool OpBase_Aware(OpBase *op, const char *alias, int *idx) {
	void *rec_idx = raxFind(op->record_map, (unsigned char *)alias, strlen(alias));
	if(idx) *idx = (int)rec_idx;
	return (rec_idx != raxNotFound);
}

void OpBase_PropagateFree(OpBase *op) {
	/* TODO: decide rather or not we want to perform
	    * op->free or OpBase_Free. */
	op->free(op);
	for(int i = 0; i < op->childCount; i++) OpBase_PropagateFree(op->children[i]);
	// OpBase_Free(op);
}

void OpBase_PropagateReset(OpBase *op) {
	assert(op->reset(op) == OP_OK);
	for(int i = 0; i < op->childCount; i++) OpBase_PropagateReset(op->children[i]);
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

Record OpBase_CreateRecord(const OpBase *op) {
	return Record_New(op->record_map);
}

void OpBase_Free(OpBase *op) {
	// Free internal operation
	op->free(op);
	if(op->children) rm_free(op->children);
	if(op->modifies) array_free(op->modifies);
	if(op->stats) rm_free(op->stats);
	free(op);
}
