/*
 * Copyright 2018-2022 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "op_project.h"
#include "RG.h"
#include "op_sort.h"
#include "../../util/arr.h"
#include "../../query_ctx.h"
#include "../../util/rmalloc.h"

/* Forward declarations. */
static Record ProjectConsume(OpBase *opBase);
static OpBase *ProjectClone(const ExecutionPlan *plan, const OpBase *opBase);
static void ProjectFree(OpBase *opBase);

OpBase *NewProjectOp(const ExecutionPlan *plan, AR_ExpNode **exps, AR_ExpNode **order_exps) {
	OpProject *op        = rm_malloc(sizeof(OpProject));
	op->exps             = exps;
	op->order_exps       = order_exps;
	op->singleResponse   = false;
	op->exp_count        = array_len(exps);
	op->order_exp_count  = array_len(order_exps);
	op->record_offsets   = array_new(uint, op->exp_count);
	op->r                = NULL;
	op->projection       = NULL;
	op->intermidiate     = NULL;
	op->intermidiate_rax = NULL;

	// Set our Op operations
	OpBase_Init((OpBase *)op, OPType_PROJECT, "Project", NULL, ProjectConsume,
				NULL, NULL, ProjectClone, ProjectFree, false, plan);

	rax *rax = raxNew();
	for(uint i = 0; i < op->exp_count; i ++) {
		const char *resolved_name = AR_EXP_GetResolvedName(op->exps[i]);
		raxInsert(rax, (unsigned char *)resolved_name, strlen(resolved_name), NULL, NULL);
		// The projected record will associate values with their resolved name
		// to ensure that space is allocated for each entry.
		int record_idx = OpBase_Modifies((OpBase *)op, resolved_name);
		array_append(op->record_offsets, record_idx);
	}

	for(uint i = 0; i < op->order_exp_count; i ++) {
		// The projected record will associate values with their resolved name
		// to ensure that space is allocated for each entry.
		const char *resolved_name = AR_EXP_GetResolvedName(op->order_exps[i]);
		if(raxFind(rax, (unsigned char *)resolved_name, strlen(resolved_name)) != raxNotFound) {
			array_append(op->record_offsets, -1);
		} else {
			int record_idx = OpBase_Modifies((OpBase *)op, resolved_name);
			array_append(op->record_offsets, record_idx);
		}
	}

	raxFree(rax);

	return (OpBase *)op;
}

static void _init_intermidiate_record(OpProject *op) {
	rax *input_rax = op->r->mapping;
	rax *output_rax = op->op.plan->record_map;

	op->intermidiate_rax = raxNew();
	raxIterator it;

	uint64_t id = 0;

	raxStart(&it, output_rax);
	raxSeek(&it, "^", NULL, 0);
	while(raxNext(&it)) {
		raxInsert(op->intermidiate_rax, it.key, it.key_len, (void*)id++, NULL);
	}
	raxStop(&it);

	raxStart(&it, input_rax);
	raxSeek(&it, "^", NULL, 0);
	while(raxNext(&it)) {
		if(raxTryInsert(op->intermidiate_rax, it.key, it.key_len, (void*)id, NULL) != 0){
			id++;
		}
	}
	raxStop(&it);

	op->intermidiate = Record_New(op->intermidiate_rax);
}

static void _update_intermidiate_record(OpProject *op, rax *rax, Record r) {
	raxIterator it;
	raxStart(&it, op->intermidiate_rax);
	raxSeek(&it, "^", NULL, 0);
	while(raxNext(&it)) {
		void *data = raxFind(rax, it.key, it.key_len);

		if(data == raxNotFound) {
			continue;
		}

		uint from_idx = (uint)(uint64_t)data;
		uint to_idx = (uint)(uint64_t)it.data;
		if(r->entries[from_idx].type != REC_TYPE_UNKNOWN) {
			Record_Add(op->intermidiate, to_idx, SI_CloneValue(Record_Get(r, from_idx)));
		}
	}
	raxStop(&it);
}

static void _update_projection_record(OpProject *op) {
	raxIterator it;
	raxStart(&it, op->op.plan->record_map);
	raxSeek(&it, "^", NULL, 0);
	while(raxNext(&it)) {
		void *data = raxFind(op->r->mapping, it.key, it.key_len);

		if(data == raxNotFound) {
			continue;
		}

		uint from_idx = (uint)(uint64_t)data;
		uint to_idx = (uint)(uint64_t)it.data;
		if(op->r->entries[from_idx].type != REC_TYPE_UNKNOWN) {
			Record_Add(op->projection, to_idx, SI_CloneValue(Record_Get(op->r, from_idx)));
		}
	}
	raxStop(&it);
}

static Record ProjectConsume(OpBase *opBase) {
	OpProject *op = (OpProject *)opBase;

	if(op->op.childCount) {
		OpBase *child = op->op.children[0];
		op->r = OpBase_Consume(child);
		if(!op->r) return NULL;
	} else {
		// QUERY: RETURN 1+2
		// Return a single record followed by NULL on the second call.
		if(op->singleResponse) return NULL;
		op->singleResponse = true;
		op->r = OpBase_CreateRecord(opBase);
	}

	op->projection = OpBase_CreateRecord(opBase);

	rax *input_rax = op->r->mapping;
	rax *output_rax = op->op.plan->record_map;
	if(op->order_exp_count > 0) {
		 if(!op->intermidiate && input_rax != output_rax) {
			_init_intermidiate_record(op);
		}

		if(op->intermidiate) {
			_update_intermidiate_record(op, input_rax, op->r);
		} else {
			_update_projection_record(op);
		}
	}

	for(uint i = 0; i < op->exp_count; i++) {
		AR_ExpNode *exp = op->exps[i];
		SIValue v = AR_EXP_Evaluate(exp, op->r);
		int rec_idx = op->record_offsets[i];
		/* Persisting a value is only necessary here if 'v' refers to a scalar held in Record 'r'.
		 * Graph entities don't need to be persisted here as Record_Add will copy them internally.
		 * The RETURN projection here requires persistence:
		 * MATCH (a) WITH toUpper(a.name) AS e RETURN e
		 * TODO This is a rare case; the logic of when to persist can be improved.  */
		if(!(v.type & SI_GRAPHENTITY)) SIValue_Persist(&v);
		Record_Add(op->projection, rec_idx, v);
		/* If the value was a graph entity with its own allocation, as with a query like:
		 * MATCH p = (src) RETURN nodes(p)[0]
		 * Ensure that the allocation is freed here. */
		if((v.type & SI_GRAPHENTITY)) SIValue_Free(v);
	}

	if(op->order_exp_count > 0) {
		Record order_record = op->projection;

		if(op->intermidiate) {
			_update_intermidiate_record(op, output_rax, op->projection);

			order_record = op->intermidiate;
		}

		for(uint i = 0; i < op->order_exp_count; i++) {
			AR_ExpNode *exp = op->order_exps[i];
			int rec_idx = op->record_offsets[op->exp_count + i];
			if(rec_idx == -1) continue;

			SIValue v = AR_EXP_Evaluate(exp, order_record);
			/* Persisting a value is only necessary here if 'v' refers to a scalar held in Record 'r'.
			* Graph entities don't need to be persisted here as Record_Add will copy them internally.
			* The RETURN projection here requires persistence:
			* MATCH (a) WITH toUpper(a.name) AS e RETURN e
			* TODO This is a rare case; the logic of when to persist can be improved.  */
			if(!(v.type & SI_GRAPHENTITY)) SIValue_Persist(&v);
			Record_Add(op->projection, rec_idx, v);
			/* If the value was a graph entity with its own allocation, as with a query like:
			* MATCH p = (src) RETURN nodes(p)[0]
			* Ensure that the allocation is freed here. */
			if((v.type & SI_GRAPHENTITY)) SIValue_Free(v);
		}
	}

	OpBase_DeleteRecord(op->r);
	op->r = NULL;

	// Emit the projected Record once.
	Record projection = op->projection;
	op->projection = NULL;
	return projection;
}

static OpBase *ProjectClone(const ExecutionPlan *plan, const OpBase *opBase) {
	ASSERT(opBase->type == OPType_PROJECT);
	OpProject *op = (OpProject *)opBase;
	AR_ExpNode **exps;
	array_clone_with_cb(exps, op->exps, AR_EXP_Clone);
	AR_ExpNode **order_exps;
	array_clone_with_cb(order_exps, op->order_exps, AR_EXP_Clone);
	return NewProjectOp(plan, exps, order_exps);
}

static void ProjectFree(OpBase *ctx) {
	OpProject *op = (OpProject *)ctx;

	if(op->exps) {
		for(uint i = 0; i < op->exp_count; i ++) AR_EXP_Free(op->exps[i]);
		array_free(op->exps);
		op->exps = NULL;
	}

	if(op->record_offsets) {
		array_free(op->record_offsets);
		op->record_offsets = NULL;
	}

	if(op->r) {
		OpBase_DeleteRecord(op->r);
		op->r = NULL;
	}

	if(op->intermidiate) {
		Record_Free(op->intermidiate);
		op->intermidiate = NULL;
		raxFree(op->intermidiate_rax);
		op->intermidiate_rax = NULL;
	}

	if(op->projection) {
		OpBase_DeleteRecord(op->projection);
		op->projection = NULL;
	}
}
