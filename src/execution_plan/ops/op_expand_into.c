/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "op_expand_into.h"
#include "shared/print_functions.h"
#include "../../query_ctx.h"

// default number of records to accumulate before traversing
#define BATCH_SIZE 16

/* Forward declarations. */
static void ExpandIntoFree(OpBase *opBase);

OpBase *NewExpandIntoOp(const ExecutionPlan *plan, AlgebraicExpression *ae) {
	OpExpandInto *op = rm_malloc(sizeof(OpExpandInto));
	op->ae = ae;

	// Set our Op operations
	OpBase_Init((OpBase *)op, OPType_EXPAND_INTO, "Expand Into", ExpandIntoFree,
		false, plan);

	// Make sure that all entities are represented in Record
	bool aware;
	UNUSED(aware);
	aware = OpBase_Aware((OpBase *)op, AlgebraicExpression_Source(ae), NULL);
	ASSERT(aware);
	aware = OpBase_Aware((OpBase *)op, AlgebraicExpression_Destination(ae), NULL);
	ASSERT(aware);

	const char *edge = AlgebraicExpression_Edge(ae);
	if(edge) {
		/* This operation will populate an edge in the Record.
		 * Prepare all necessary information for collecting matching edges. */
		OpBase_Modifies((OpBase *)op, edge);
	}

	return (OpBase *)op;
}

// Frees ExpandInto
static void ExpandIntoFree(OpBase *ctx) {
	OpExpandInto *op = (OpExpandInto *)ctx;
	if(op->ae) {
		AlgebraicExpression_Free(op->ae);
		op->ae = NULL;
	}
}
