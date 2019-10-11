/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "op_project.h"
#include "op_sort.h"
#include "../../util/arr.h"
#include "../../query_ctx.h"
#include "../../util/rmalloc.h"

/* Forward declarations. */
static OpResult ProjectInit(OpBase *opBase);
static Record ProjectConsume(OpBase *opBase);
static void ProjectFree(OpBase *opBase);

OpBase *NewProjectOp(const ExecutionPlan *plan, AR_ExpNode **exps, AR_ExpNode **order_exps) {
	OpProject *op = malloc(sizeof(OpProject));
	op->exps = exps;
	op->singleResponse = false;
	op->exp_count = array_len(exps);
	op->record_offsets = array_new(uint, op->exp_count);
	op->project_all = (exps == NULL); // WITH/RETURN * projection, expressions will be populated later

	// Set our Op operations
	OpBase_Init((OpBase *)op, OPType_PROJECT, "Project", ProjectInit, ProjectConsume,
				NULL, NULL, ProjectFree, plan);

	for(uint i = 0; i < op->exp_count; i ++) {
		// The projected record will associate values with their resolved name
		// to ensure that space is allocated for each entry.
		int record_idx = OpBase_Modifies((OpBase *)op, op->exps[i]->resolved_name);
		op->record_offsets = array_append(op->record_offsets, record_idx);
	}

	uint order_exp_count = array_len(order_exps);
	if(order_exp_count && exps == NULL) op->exps = array_new(AR_ExpNode *, order_exp_count); // TODO tmp
	for(uint i = 0; i < order_exp_count; i ++) {
		// If an ORDER BY alias is already being projected, it does not need to be added again.
		bool evaluated = OpBase_Aware((OpBase *)op, order_exps[i]->resolved_name, NULL);
		if(evaluated) continue;

		// Otherwise, append it the projection arrays.
		// TODO issues if we populate a STAR projection after this point
		op->exps = array_append(op->exps, order_exps[i]);
		int record_idx = OpBase_Modifies((OpBase *)op, order_exps[i]->resolved_name);
		op->record_offsets = array_append(op->record_offsets, record_idx);
	}

	return (OpBase *)op;
}

static OpResult ProjectInit(OpBase *opBase) { // TODO delete if unused
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

	Record projection = OpBase_CreateRecord(opBase);
	// Track the inherited Record and the newly-allocated Record so that they may be freed if execution fails.
	OpBase_AddVolatileRecord(opBase, r);
	OpBase_AddVolatileRecord(opBase, projection);

	uint count = array_len(op->record_offsets);
	for(uint i = 0; i < count; i++) {
		AR_ExpNode *exp = op->exps[i];
		SIValue v = AR_EXP_Evaluate(exp, r);
		// int rec_idx = op->record_offsets[i]; // TODO TODO
		int rec_idx = i;
		/* Persisting a value is only necessary here if 'v' refers to a scalar held in Record 'r'.
		 * Graph entities don't need to be persisted here as Record_Add will copy them internally.
		 * The RETURN projection here requires persistence:
		 * MATCH (a) WITH toUpper(a.name) AS e RETURN e
		 * TODO This is a rare case; the logic of when to persist can be improved.  */
		if(!(v.type & SI_GRAPHENTITY)) SIValue_Persist(&v);
		Record_Add(projection, rec_idx, v);
	}

	Record_Free(r);
	OpBase_RemoveVolatileRecords(opBase); // No exceptions encountered, Records are not dangling.
	return projection;
}

static void ProjectFree(OpBase *ctx) {
	OpProject *op = (OpProject *)ctx;

	if(op->exps) {
		// Only free projection expressions (exclude order expressions).
		for(uint i = 0; i < op->exp_count; i ++) {
			// Expression names need to be freed if this was a * projection.
			if(op->project_all) rm_free((char *)op->exps[i]->resolved_name); // TODO remove soon
			AR_EXP_Free(op->exps[i]);
		}
		array_free(op->exps);
		op->exps = NULL;
	}

	if(op->record_offsets) {
		array_free(op->record_offsets);
		op->record_offsets = NULL;
	}
}

