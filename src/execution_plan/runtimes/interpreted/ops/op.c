/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "op.h"
#include "RG.h"
#include "../runtime_execution_plan.h"
#include "../../../../util/arr.h"
#include "../../../../util/rmalloc.h"
#include "../../../../util/simple_timer.h"

void RT_OpBase_Init(RT_OpBase *op, OPType type, RT_fpInit init, RT_fpConsume consume,
				 RT_fpReset reset, RT_fpClone clone, RT_fpFree free, bool writer,
				 const struct RT_ExecutionPlan *plan) {

	op->type = type;
	op->plan = plan;
	op->parent = NULL;
	op->childCount = 0;
	op->children = NULL;
	op->parent = NULL;
	op->stats = NULL;
	op->op_initialized = false;
	op->modifies = NULL;
	op->writer = writer;

	// Function pointers.
	op->init = init;
	op->consume = consume;
	op->reset = reset;
	op->clone = clone;
	op->free = free;
	op->profile = NULL;
}

inline Record RT_OpBase_Consume(RT_OpBase *op) {
	return op->consume(op);
}

bool RT_OpBase_Aware(RT_OpBase *op, const char *alias, uint *idx) {
	rax *mapping = RT_ExecutionPlan_GetMappings(op->plan);
	void *rec_idx = raxFind(mapping, (unsigned char *)alias, strlen(alias));
	if(idx) *idx = (uintptr_t)rec_idx;
	return (rec_idx != raxNotFound);
}

void RT_OpBase_PropagateFree(RT_OpBase *op) {
	if(op->free) op->free(op);
	for(int i = 0; i < op->childCount; i++) RT_OpBase_PropagateFree(op->children[i]);
}

void RT_OpBase_PropagateReset(RT_OpBase *op) {
	if(op->reset) {
		RT_OpResult res = op->reset(op);
		ASSERT(res == OP_OK);
	}
	for(int i = 0; i < op->childCount; i++) RT_OpBase_PropagateReset(op->children[i]);
}

static void _OpBase_StatsToString(const RT_OpBase *op, sds *buff) {
	*buff = sdscatprintf(*buff,
					" | Records produced: %d, Execution time: %f ms",
					op->stats->profileRecordCount,
					op->stats->profileExecTime);
}

Record RT_OpBase_Profile(RT_OpBase *op) {
	double tic [2];
	// Start timer.
	simple_tic(tic);
	Record r = op->profile(op);
	// Stop timer and accumulate.
	op->stats->profileExecTime += simple_toc(tic);
	if(r) op->stats->profileRecordCount++;
	return r;
}

bool RT_OpBase_IsWriter(RT_OpBase *op) {
	return op->writer;
}

void RT_OpBase_UpdateConsume(RT_OpBase *op, RT_fpConsume consume) {
	ASSERT(op != NULL);
	/* If Operation is profiled, update profiled function.
	 * otherwise update consume function. */
	if(op->profile != NULL) op->profile = consume;
	else op->consume = consume;
}

inline Record RT_OpBase_CreateRecord(const RT_OpBase *op) {
	return RT_ExecutionPlan_BorrowRecord((struct RT_ExecutionPlan *)op->plan);
}

Record RT_OpBase_CloneRecord(Record r) {
	Record clone = RT_ExecutionPlan_BorrowRecord((struct RT_ExecutionPlan *)r->owner);
	Record_Clone(r, clone);
	return clone;
}

inline OPType RT_OpBase_Type(const RT_OpBase *op) {
	ASSERT(op != NULL);
	return op->type;
}

inline void RT_OpBase_DeleteRecord(Record r) {
	RT_ExecutionPlan_ReturnRecord(r->owner, r);
}

RT_OpBase *RT_OpBase_Clone(const struct RT_ExecutionPlan *plan, const RT_OpBase *op) {
	if(op->clone) return op->clone(plan, op);
	return NULL;
}

void RT_OpBase_Free(RT_OpBase *op) {
	// Free internal operation
	if(op->free) op->free(op);
	if(op->children) rm_free(op->children);
	if(op->modifies) array_free(op->modifies);
	if(op->stats) rm_free(op->stats);
	rm_free(op);
}
