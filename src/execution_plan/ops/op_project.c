/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "op_project.h"
#include "shared/project_functions.h"
#include "../../util/arr.h"
#include "../../query_ctx.h"
#include "../../util/rmalloc.h"
#include "../../util/rax_extensions.h"

/* Forward declarations. */
static OpResult ProjectInit(OpBase *opBase);
static Record ProjectConsume(OpBase *opBase);
static void ProjectFree(OpBase *opBase);

OpBase *NewProjectOp(const ExecutionPlan *plan, AR_ExpNode **projection_exps,
					 AR_ExpNode **order_exps) {
	OpProject *op = rm_malloc(sizeof(OpProject));
	op->singleResponse = false;
	op->intermediate_record = NULL;
	op->intermediate_record_ids = NULL;

	// Migrate ORDER expressions to the projection array as appropriate.
	CombineProjectionArrays(&projection_exps, &order_exps);
	op->order_exps = order_exps;
	op->projection_exps = projection_exps;
	uint order_count = array_len(order_exps);
	uint projection_count = array_len(projection_exps);
	op->record_offsets = array_new(uint, projection_count + order_count);

	// Set our Op operations
	OpBase_Init((OpBase *)op, OPType_PROJECT, "Project", ProjectInit, ProjectConsume,
				NULL, NULL, ProjectFree, false, plan);

	for(uint i = 0; i < projection_count; i++) {
		AR_ExpNode *exp = op->projection_exps[i];
		// The projected record will associate values with their resolved name
		// to ensure that space is allocated for each entry.
		int record_idx = OpBase_Modifies((OpBase *)op, exp->resolved_name);
		op->record_offsets = array_append(op->record_offsets, record_idx);
	}

	for(uint i = 0; i < order_count; i++) {
		AR_ExpNode *exp = op->order_exps[i];
		// The projected record will associate values with their resolved name
		// to ensure that space is allocated for each entry.
		int record_idx = OpBase_Modifies((OpBase *)op, exp->resolved_name);
		op->record_offsets = array_append(op->record_offsets, record_idx);
	}

	return (OpBase *)op;
}

static OpResult ProjectInit(OpBase *opBase) {
	OpProject *op = (OpProject *)opBase;
	// Return early if all of our projections are in one array.
	if(!op->order_exps) return OP_OK;

	rax *intermediate_map = NULL;
	if(opBase->childCount == 0) {
		// No tap operation; we can build a new Record and mapping.
		intermediate_map = raxNew();
	} else {
		// Clone the previous map to append new bindings.
		rax *previous_map = ExecutionPlan_GetMappings(opBase->children[0]->plan);
		intermediate_map = raxClone(previous_map);
	}

	intptr_t id = raxSize(intermediate_map);
	intptr_t projection_count = array_len(op->projection_exps);
	op->intermediate_record_ids = rm_malloc(projection_count * sizeof(intptr_t));
	for(intptr_t i = 0; i < projection_count; i ++) {
		const char *name = op->projection_exps[i]->resolved_name;
		// Check if the current alias is already mapped.
		void *prev_id = raxFind(intermediate_map, (unsigned char *)name, strlen(name));
		if(prev_id != raxNotFound) {
			op->intermediate_record_ids[i] = (intptr_t)prev_id; // Use the already-set ID.
		} else {
			// Add a new ID to the mapping.
			raxTryInsert(intermediate_map, (unsigned char *)name, strlen(name), (void *)id, NULL);
			op->intermediate_record_ids[i] = id;
			id++;
		}
	}
	intptr_t order_count = array_len(op->order_exps);
	for(intptr_t i = 0; i < order_count; i ++) {
		const char *name = op->order_exps[i]->resolved_name;
		// Attempt to add the current alias to the mapping, incrementing the current ID if successful.
		// IDs do not need to be tracked for dependent ORDER expressions.
		id += raxTryInsert(intermediate_map, (unsigned char *)name, strlen(name), (void *)id, NULL);
	}

	// Build a reusable intermediate record for this operation.
	op->intermediate_record = Record_New(intermediate_map);

	return OP_OK;
}

static Record ProjectConsume(OpBase *opBase) {
	OpProject *op = (OpProject *)opBase;
	Record r = NULL;

	if(op->op.childCount) {
		OpBase *child = op->op.children[0];
		r = OpBase_Consume(child);
		if(!r) return NULL;
	} else {
		// QUERY: RETURN 1+2
		// Return a single record followed by NULL on the second call.
		if(op->singleResponse) return NULL;
		op->singleResponse = true;
		r = OpBase_CreateRecord(opBase);
	}

	// If we require an intermediate Record, populate it with the entries from the child.
	if(op->intermediate_record) Record_Clone(r, op->intermediate_record);
	Record projection = OpBase_CreateRecord(opBase);

	uint projection_count = array_len(op->projection_exps);
	for(uint i = 0; i < projection_count; i++) {
		AR_ExpNode *exp = op->projection_exps[i];
		SIValue v = AR_EXP_Evaluate(exp, r);
		int rec_idx = op->record_offsets[i];
		/* Persisting a value is only necessary here if 'v' refers to a scalar held in Record 'r'.
		 * Graph entities don't need to be persisted here as Record_Add will copy them internally.
		 * The RETURN projection here requires persistence:
		 * MATCH (a) WITH toUpper(a.name) AS e RETURN e
		 * TODO This is a rare case; the logic of when to persist can be improved.  */
		if(!(v.type & SI_GRAPHENTITY)) SIValue_Persist(&v);
		if(op->intermediate_record)
			Record_Add(op->intermediate_record, op->intermediate_record_ids[i], v);
		Record_Add(projection, rec_idx, v);
	}

	// Evaluate expressions on the intermediate Record, if any.
	uint order_count = array_len(op->order_exps);
	for(uint i = 0; i < order_count; i++) {
		AR_ExpNode *exp = op->order_exps[i];
		SIValue v = AR_EXP_Evaluate(exp, op->intermediate_record);
		int rec_idx = op->record_offsets[projection_count + i];
		Record_Add(projection, rec_idx, v);
	}

	OpBase_DeleteRecord(r);
	return projection;
}

static void ProjectFree(OpBase *ctx) {
	OpProject *op = (OpProject *)ctx;

	if(op->projection_exps) {
		uint exp_count = array_len(op->projection_exps);
		for(uint i = 0; i < exp_count; i ++) AR_EXP_Free(op->projection_exps[i]);
		array_free(op->projection_exps);
		op->projection_exps = NULL;
	}

	if(op->order_exps) {
		uint exp_count = array_len(op->order_exps);
		for(uint i = 0; i < exp_count; i ++) AR_EXP_Free(op->order_exps[i]);
		array_free(op->order_exps);
		op->order_exps = NULL;
	}

	if(op->record_offsets) {
		array_free(op->record_offsets);
		op->record_offsets = NULL;
	}

	if(op->intermediate_record) {
		raxFree(op->intermediate_record->mapping);
		Record_Free(op->intermediate_record);
		op->intermediate_record = NULL;
	}

	if(op->intermediate_record_ids) {
		rm_free(op->intermediate_record_ids);
		op->intermediate_record_ids = NULL;
	}
}

