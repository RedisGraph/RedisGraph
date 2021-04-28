/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "op_results.h"
#include "RG.h"
#include "../../util/arr.h"
#include "../../query_ctx.h"
#include "configuration/config.h"
#include "../../arithmetic/arithmetic_expression.h"

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

static OpResult ResultsInit(OpBase *opBase) {
	Results *op = (Results *)opBase;
	op->result_set = QueryCtx_GetResultSet();
	Config_Option_get(Config_RESULTSET_MAX_SIZE, &op->result_set_size_limit);
	return OP_OK;
}

/* Results consume operation
 * called each time a new result record is required */
static Record ResultsConsume(OpBase *opBase) {
	Record r = NULL;
	Results *op = (Results *)opBase;

	// enforce result-set size limit
	if(op->result_set_size_limit == 0) return NULL;
	op->result_set_size_limit--;

	OpBase *child = op->op.children[0];
	r = OpBase_Consume(child);
	if(!r) return NULL;

	// append to final result set
	ResultSet_AddRecord(op->result_set, r);
	return r;
}

static inline OpBase *ResultsClone(const ExecutionPlan *plan, const OpBase *opBase) {
	ASSERT(opBase->type == OPType_RESULTS);
	return NewResultsOp(plan);
}
