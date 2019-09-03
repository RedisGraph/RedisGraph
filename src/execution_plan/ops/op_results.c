/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "op_results.h"
#include "../../util/arr.h"
#include "../../arithmetic/arithmetic_expression.h"

/* Forward declarations */
static void Free(OpBase *opBase);
static OpResult Reset(OpBase *op);
static Record Consume(OpBase *opBase);

OpBase *NewResultsOp(const ExecutionPlan *plan, ResultSet *result_set, QueryGraph *graph) {
	Results *op = malloc(sizeof(Results));
	op->result_set = result_set;

	// Set our Op operations
	OpBase_Init((OpBase *)op, OPType_VALUE_HASH_JOIN, "Results", NULL, Consume, Reset, NULL, Free,
				plan);
	return (OpBase *)op;
}

/* Results consume operation
 * called each time a new result record is required */
static Record Consume(OpBase *opBase) {
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

/* Restart */
static OpResult Reset(OpBase *op) {
	return OP_OK;
}

/* Frees Results */
static void Free(OpBase *opBase) {
}
