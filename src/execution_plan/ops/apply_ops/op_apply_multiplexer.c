/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "op_apply_multiplexer.h"

// Forward declerations.
OpResult OpApplyMultiplexerReset(OpBase *opBase);
Record OpApplyMultiplexerConsume(OpBase *opBase);
void OpApplyMultiplexerFree(OpBase *opBase);


static inline Record _pullFromStream(OpBase *branch) {
	return OpBase_Consume(branch);
}

static Record _pullFromBranchStream(OpSemiApply *op) {
	OpBase *branch = op->match_branch;
	// Propegate record to the top of the right-hand side stream.
	if(op->op_arg) Argument_AddRecord(op->op_arg, Record_Clone(op->r));
	return _pullFromStream(branch);
}

static Record _pullFromMainStream(OpApplyMultiplexer *op) {
	OpBase *main_stream = op->execution_plan_branch;
	return _pullFromStream(main_stream);
}

static Record _OpApplyMultiplexer_OrApplyLogic(OpApplyMultiplexer *apply_multiplexer) {

}

static Record _OpApplyMultiplexer_AndApplyLogic(OpApplyMultiplexer *apply_multiplexer) {

}

OpBase *NewApplyMultiplexerOp(ExecutionPlan *plan, AST_Operator boolean_operator) {

	OpApplyMultiplexer *op = rm_calloc(1, sizeof(OpApplyMultiplexer));
	op->boolean_operator = boolean_operator;
	op->filter = NULL;
	const char *name;
	if(boolean_operator == OP_OR) {
		name = "OR Apply Multiplexer";
		op->apply_func = _OpApplyMultiplexer_OrApplyLogic;
	} else if(boolean_operator == OP_AND) {
		name = "OR Apply Multiplexer";
		op->apply_func = _OpApplyMultiplexer_AndApplyLogic;
	} else {
		assert(false);
	}

	// Set our Op operations
	OpBase_Init((OpBase *)op, OPType_APPLY_MULTIPLEXER, name, NULL, OpApplyMultiplexerConsume,
				OpApplyMultiplexerReset, NULL, OpApplyMultiplexerFree, plan);

	return (OpBase *) op;
}

Record OpApplyMultiplexerConsume(OpBase *opBase) {
	OpApplyMultiplexer *op;
	return op->apply_func(op);
}

OpResult OpApplyMultiplexerReset(OpBase *opBase) {
	OpApplyMultiplexer *op = (OpApplyMultiplexer *)opBase;
	if(op->r) {
		Record_Free(op->r);
		op->r = NULL;
	}
	return OP_OK;
}

void OpApplyMultiplexerFree(OpBase *opBase) {
	OpApplyMultiplexer *op = (OpApplyMultiplexer *)opBase;

	if(op->r) {
		Record_Free(op->r);
		op->r = NULL;
	}
}