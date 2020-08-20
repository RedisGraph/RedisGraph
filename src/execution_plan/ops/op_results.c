/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "op_results.h"
#include "../../util/arr.h"
#include "../../arithmetic/arithmetic_expression.h"
#include "../../query_ctx.h"

/* Forward declarations. */
static Record ResultsConsume(OpBase *opBase);
static OpResult ResultsInit(OpBase *opBase);
static OpBase *ResultsClone(const ExecutionPlan *plan, const OpBase *opBase);

OpBase *NewResultsOp(const ExecutionPlan *plan) {
	Results *op = rm_malloc(sizeof(Results));

	// Set our Op operations
	OpBase_Init((OpBase *)op, OPType_RESULTS, "Results", ResultsInit, ResultsConsume,
				NULL, NULL, ResultsClone, NULL, false, plan);

	return (OpBase *)op;
}

/* Results consume operation
 * called each time a new result record is required */
static Record ResultsConsume(OpBase *opBase) {
	Record r = NULL;
	Results *op = (Results *)opBase;

	if(op->op.childCount) {
		OpBase *child = op->op.children[0];
		r = OpBase_Consume(child);
		if(!r) return NULL;
	}

	/* Append to final result set. */
	ResultSet_AddRecord(op->result_set, r);
	return r;
}

static OpResult ResultsInit(OpBase *opBase) {
	Results *op = (Results *)opBase;
	op->result_set = QueryCtx_GetResultSet();
	return OP_OK;
}

static inline OpBase *ResultsClone(const ExecutionPlan *plan, const OpBase *opBase) {
	assert(opBase->type == OPType_RESULTS);
	return NewResultsOp(plan);
}
