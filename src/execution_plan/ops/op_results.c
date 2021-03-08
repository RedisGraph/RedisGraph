/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "op_results.h"
#include "RG.h"
#include "config.h"
#include "../../util/arr.h"
#include "../../arithmetic/arithmetic_expression.h"
#include "../../query_ctx.h"

// forward declarations
static RecordBatch ResultsConsume(OpBase *opBase);
static OpResult ResultsInit(OpBase *opBase);
static OpBase *ResultsClone(const ExecutionPlan *plan, const OpBase *opBase);

OpBase *NewResultsOp(const ExecutionPlan *plan) {
	Results *op = rm_malloc(sizeof(Results));

	// set our Op operations
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

/* results consume operation
 * called each time a new result record is required */
static RecordBatch ResultsConsume(OpBase *opBase) {
	Results *op = (Results *)opBase;

	OpBase       *child      =  op->op.children[0];
	RecordBatch  batch       =  OpBase_Consume(child);
	uint         batch_size  =  RecordBatch_Len(batch);

	if(op->result_set_size_limit >= batch_size) {
		op->result_set_size_limit -= batch_size;
	} else {
		batch_size = op->result_set_size_limit;
		op->result_set_size_limit = 0;
	}

	for(uint i = 0; i < batch_size; i++) {
		Record r = batch[i];
		ResultSet_AddRecord(op->result_set, r); // append to final result set
	}

	return batch;
}

static inline OpBase *ResultsClone(const ExecutionPlan *plan, const OpBase *opBase) {
	ASSERT(opBase->type == OPType_RESULTS);
	return NewResultsOp(plan);
}
