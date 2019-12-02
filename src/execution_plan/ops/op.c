/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "op.h"
#include "../../util/rmalloc.h"
#include "../../util/simple_timer.h"

#include <assert.h>

/* Forward declarations */
rax *ExecutionPlan_GetMappings(const struct ExecutionPlan *plan);

void OpBase_Init(OpBase *op, OPType type, const char *name, fpInit init, fpConsume consume,
				 fpReset reset, fpToString toString, fpFree free, const struct ExecutionPlan *plan) {

	op->type = type;
	op->name = name;
	op->plan = plan;
	op->stats = NULL;
	op->parent = NULL;
	op->childCount = 0;
	op->children = NULL;
	op->parent = NULL;
	op->stats = NULL;
	op->op_initialized = false;
	op->dangling_records = NULL;
	op->modifies = NULL;

	// Function pointers.
	op->init = init;
	op->consume = consume;
	op->reset = reset;
	op->toString = toString;
	op->free = free;
}

inline Record OpBase_Consume(OpBase *op) {
	return op->consume(op);
}

int OpBase_Modifies(OpBase *op, const char *alias) {
	if(!op->modifies) op->modifies = array_new(const char *, 1);
	op->modifies = array_append(op->modifies, alias);

	/* Make sure alias has an entry associated with it
	 * within the record mapping. */
	rax *mapping = ExecutionPlan_GetMappings(op->plan);

	void *id = raxFind(mapping, (unsigned char *)alias, strlen(alias));
	if(id == raxNotFound) {
		id = (void *)raxSize(mapping);
		raxInsert(mapping, (unsigned char *)alias, strlen(alias), id, NULL);
	}

	return (intptr_t)id;
}

bool OpBase_Aware(OpBase *op, const char *alias, int *idx) {
	rax *mapping = ExecutionPlan_GetMappings(op->plan);
	void *rec_idx = raxFind(mapping, (unsigned char *)alias, strlen(alias));
	if(idx) *idx = (intptr_t)rec_idx;
	return (rec_idx != raxNotFound);
}

void OpBase_PropagateFree(OpBase *op) {
	if(!op) return;
	if(op->free) op->free(op);
	for(int i = 0; i < op->childCount; i++) OpBase_PropagateFree(op->children[i]);
}

void OpBase_PropagateReset(OpBase *op) {
	if(!op) return;
	if(op->reset) assert(op->reset(op) == OP_OK);
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

void OpBase_AddVolatileRecord(OpBase *op, const Record r) {
	if(op->dangling_records == NULL) op->dangling_records = array_new(Record, 1);
	op->dangling_records = array_append(op->dangling_records, r);
}

void OpBase_RemoveVolatileRecords(OpBase *op) {
	if(!op->dangling_records) return;

	array_clear(op->dangling_records);
}

Record OpBase_CreateRecord(const OpBase *op) {
	rax *mapping = ExecutionPlan_GetMappings(op->plan);
	return Record_New(mapping);
}

void OpBase_Free(OpBase *op) {
	// Free internal operation
	if(op->free) op->free(op);
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

