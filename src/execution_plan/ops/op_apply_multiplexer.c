/*
 * Copyright 2018-2021 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "op_apply_multiplexer.h"

// Forward declerations.
static void OpApplyMultiplexerFree(OpBase *opBase);

OpBase *NewApplyMultiplexerOp(const ExecutionPlan *plan, AST_Operator boolean_operator) {
	OpApplyMultiplexer *op = rm_calloc(1, sizeof(OpApplyMultiplexer));
	op->boolean_operator = boolean_operator;
	// Set our Op operations
	if(boolean_operator == OP_OR) {
		OpBase_Init((OpBase *)op, OPType_OR_APPLY_MULTIPLEXER, "OR Apply Multiplexer",
					NULL, OpApplyMultiplexerFree, false, plan);
	} else if(boolean_operator == OP_AND) {
		OpBase_Init((OpBase *)op, OPType_AND_APPLY_MULTIPLEXER, "AND Apply Multiplexer",
					NULL, OpApplyMultiplexerFree, false, plan);
	} else {
		ASSERT("apply multiplexer boolean operator should be AND or OR only" && false);
	}
	return (OpBase *) op;
}

static void OpApplyMultiplexerFree(OpBase *opBase) {
	OpApplyMultiplexer *op = (OpApplyMultiplexer *)opBase;

	if(op->branch_arguments) {
		array_free(op->branch_arguments);
		op->branch_arguments = NULL;
	}
}
