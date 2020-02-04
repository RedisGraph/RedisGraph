/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "op_project.h"
#include "op_sort.h"
#include "../../util/arr.h"
#include "../../query_ctx.h"
#include "../../util/rmalloc.h"

/* Forward declarations. */
static Record ProjectConsume(OpBase *opBase);
static void ProjectFree(OpBase *opBase);

OpBase *NewProjectOp(const ExecutionPlan *plan, AR_ExpNode **exps) {
	OpProject *op = rm_malloc(sizeof(OpProject));
	op->exps = exps;
	op->singleResponse = false;
	op->exp_count = array_len(exps);
	op->record_offsets = array_new(uint, op->exp_count);

	// Set our Op operations
	OpBase_Init((OpBase *)op, OPType_PROJECT, "Project", NULL, ProjectConsume,
				NULL, NULL, ProjectFree, false, plan);

	for(uint i = 0; i < op->exp_count; i ++) {
		// The projected record will associate values with their resolved name
		// to ensure that space is allocated for each entry.
		int record_idx = OpBase_Modifies((OpBase *)op, op->exps[i]->resolved_name);
		op->record_offsets = array_append(op->record_offsets, record_idx);
	}

	return (OpBase *)op;
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

	Record projection = OpBase_CreateRecord(opBase);

	for(uint i = 0; i < op->exp_count; i++) {
		AR_ExpNode *exp = op->exps[i];
		SIValue v = AR_EXP_Evaluate(exp, r);
		int rec_idx = op->record_offsets[i];
		/* Persisting a value is only necessary here if 'v' refers to a scalar held in Record 'r'.
		 * Graph entities don't need to be persisted here as Record_Add will copy them internally.
		 * The RETURN projection here requires persistence:
		 * MATCH (a) WITH toUpper(a.name) AS e RETURN e
		 * TODO This is a rare case; the logic of when to persist can be improved.  */
		if(!(v.type & SI_GRAPHENTITY)) SIValue_Persist(&v);
		Record_Add(projection, rec_idx, v);
	}

	OpBase_DeleteRecord(r);
	return projection;
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
}

