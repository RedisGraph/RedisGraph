/*
 * Copyright 2018-2021 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "op_project.h"
#include "RG.h"
#include "op_sort.h"
#include "../../../../util/arr.h"
#include "../../../../query_ctx.h"
#include "../../../../util/rmalloc.h"

/* Forward declarations. */
static Record ProjectConsume(RT_OpBase *opBase);
static void ProjectFree(RT_OpBase *opBase);

RT_OpBase *RT_NewProjectOp(const RT_ExecutionPlan *plan, const OpProject *op_desc) {
	RT_OpProject *op = rm_malloc(sizeof(RT_OpProject));
	op->op_desc = op_desc;
	array_clone_with_cb(op->exps, op_desc->exps, AR_EXP_Clone);
	op->singleResponse = false;
	op->record_offsets = array_new(uint, op_desc->exp_count);
	op->r = NULL;
	op->projection = NULL;

	// Set our Op operations
	RT_OpBase_Init((RT_OpBase *)op, (const OpBase *)&op_desc->op, NULL, NULL,
		ProjectConsume, NULL, ProjectFree, plan);

	for(uint i = 0; i < op_desc->exp_count; i ++) {
		// The projected record will associate values with their resolved name
		// to ensure that space is allocated for each entry.
		uint record_idx;
		bool aware = RT_OpBase_Aware((RT_OpBase *)op, op->exps[i]->resolved_name, &record_idx);
		ASSERT(aware);
		array_append(op->record_offsets, record_idx);
	}

	return (RT_OpBase *)op;
}

static Record ProjectConsume(RT_OpBase *opBase) {
	RT_OpProject *op = (RT_OpProject *)opBase;

	if(op->op.childCount) {
		RT_OpBase *child = op->op.children[0];
		op->r = RT_OpBase_Consume(child);
		if(!op->r) return NULL;
	} else {
		// QUERY: RETURN 1+2
		// Return a single record followed by NULL on the second call.
		if(op->singleResponse) return NULL;
		op->singleResponse = true;
		op->r = RT_OpBase_CreateRecord(opBase);
	}

	op->projection = RT_OpBase_CreateRecord(opBase);

	for(uint i = 0; i < op->op_desc->exp_count; i++) {
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

	RT_OpBase_DeleteRecord(op->r);
	op->r = NULL;

	// Emit the projected Record once.
	Record projection = op->projection;
	op->projection = NULL;
	return projection;
}

static void ProjectFree(RT_OpBase *ctx) {
	RT_OpProject *op = (RT_OpProject *)ctx;
	if(op->record_offsets) {
		array_free(op->record_offsets);
		op->record_offsets = NULL;
	}

	if(op->r) {
		RT_OpBase_DeleteRecord(op->r);
		op->r = NULL;
	}

	if(op->projection) {
		RT_OpBase_DeleteRecord(op->projection);
		op->projection = NULL;
	}

	if(op->exps) {
		for(uint i = 0; i < op->op_desc->exp_count; i ++) AR_EXP_Free(op->exps[i]);
		array_free(op->exps);
		op->exps = NULL;
	}
}
