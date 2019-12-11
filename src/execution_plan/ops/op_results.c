/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "op_results.h"
#include "../../util/arr.h"
#include "../../arithmetic/arithmetic_expression.h"

/* Forward declarations. */
static Record ResultsConsume(OpBase *opBase);

OpBase *NewResultsOp(const ExecutionPlan *plan, ResultSet *result_set) {
	Results *op = rm_malloc(sizeof(Results));
	op->result_set = result_set;

	// Set our Op operations
	OpBase_Init((OpBase *)op, OPType_RESULTS, "Results", NULL, ResultsConsume,
				NULL, NULL, NULL, false, plan);

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
