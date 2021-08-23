/*
 * Copyright 2018-2021 Redis Labs Ltd. and Contributors
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
static void ProjectFree(OpBase *opBase);

OpBase *NewProjectOp(const ExecutionPlan *plan, AR_ExpNode **exps) {
	OpProject *op = rm_malloc(sizeof(OpProject));
	op->exps = exps;
	op->exp_count = array_len(exps);

	// Set our Op operations
	OpBase_Init((OpBase *)op, OPType_PROJECT, "Project", NULL,
				ProjectFree, false, plan);

	for(uint i = 0; i < op->exp_count; i ++) {
		// The projected record will associate values with their resolved name
		// to ensure that space is allocated for each entry.
		OpBase_Modifies((OpBase *)op, op->exps[i]->resolved_name);
	}

	return (OpBase *)op;
}

static void ProjectFree(OpBase *ctx) {
	OpProject *op = (OpProject *)ctx;

	if(op->exps) {
		for(uint i = 0; i < op->exp_count; i ++) AR_EXP_Free(op->exps[i]);
		array_free(op->exps);
		op->exps = NULL;
	}
}
