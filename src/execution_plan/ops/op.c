/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "op.h"
#include "RG.h"
#include "../record.h"
#include "../../util/arr.h"
#include "../execution_plan.h"
#include "../../util/rmalloc.h"
#include "../../util/simple_timer.h"

void OpBase_Init(OpBase *op, OPType type, const char *name,
				 fpFree free, bool writer,
				 const struct ExecutionPlan *plan) {

	op->type = type;
	op->name = name;
	op->plan = plan;
	op->parent = NULL;
	op->childCount = 0;
	op->children = NULL;
	op->parent = NULL;
	op->modifies = NULL;
	op->writer = writer;

	// Function pointers.
	op->free = free;
}

static inline rax *ExecutionPlan_GetMappings(const ExecutionPlan *plan) {
	ASSERT(plan && plan->record_map);
	return plan->record_map;
}

int OpBase_Modifies(OpBase *op, const char *alias) {
	if(!op->modifies) op->modifies = array_new(const char *, 1);
	array_append(op->modifies, alias);

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

int OpBase_AliasModifier(OpBase *op, const char *modifier, const char *alias) {
	rax *mapping = ExecutionPlan_GetMappings(op->plan);
	void *id = raxFind(mapping, (unsigned char *)modifier, strlen(modifier));
	ASSERT(id != raxNotFound);

	// Make sure to not introduce the same modifier twice.
	if(raxInsert(mapping, (unsigned char *)alias, strlen(alias), id, NULL)) {
		array_append(op->modifies, alias);
	}

	return (intptr_t)id;
}

bool OpBase_Aware(OpBase *op, const char *alias, uint *idx) {
	rax *mapping = ExecutionPlan_GetMappings(op->plan);
	void *rec_idx = raxFind(mapping, (unsigned char *)alias, strlen(alias));
	if(idx) *idx = (uintptr_t)rec_idx;
	return (rec_idx != raxNotFound);
}

void OpBase_PropagateFree(OpBase *op) {
	if(op->free) op->free(op);
	for(int i = 0; i < op->childCount; i++) OpBase_PropagateFree(op->children[i]);
}

bool OpBase_IsWriter(OpBase *op) {
	return op->writer;
}

inline OPType OpBase_Type(const OpBase *op) {
	ASSERT(op != NULL);
	return op->type;
}

void OpBase_Free(OpBase *op) {
	// Free internal operation
	if(op->free) op->free(op);
	if(op->children) rm_free(op->children);
	if(op->modifies) array_free(op->modifies);
	rm_free(op);
}
