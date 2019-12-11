/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "op_apply_multiplexer.h"
#include "apply_ops_utils.h"

// Forward declerations.
OpResult OpApplyMultiplexerInit(OpBase *opBase);
OpResult OpApplyMultiplexerReset(OpBase *opBase);
Record OpApplyMultiplexerConsume(OpBase *opBase);
void OpApplyMultiplexerFree(OpBase *opBase);


static inline Record _pullFromStream(OpBase *branch) {
	return OpBase_Consume(branch);
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
	OpBase_Init((OpBase *)op, OPType_APPLY_MULTIPLEXER, name, OpApplyMultiplexerInit,
				OpApplyMultiplexerConsume, OpApplyMultiplexerReset, NULL, OpApplyMultiplexerFree, plan);

	return (OpBase *) op;
}

Record OpApplyMultiplexerConsume(OpBase *opBase) {
	OpApplyMultiplexer *op;
	return op->apply_func(op);
}

bool _isApplyBranch(OpBase *branch) {
	if(ExecutionPlan_LocateOp(branch, OPType_SEMI_APPLY | OPType_APPLY_MULTIPLEXER)) return true;
	return false;
}

OpResult OpApplyMultiplexerInit(OpBase *opBase) {
	int childCount = opBase->childCount;
	OpApplyMultiplexer *apply_multiplexer = (OpApplyMultiplexer *) opBase;
	apply_multiplexer->filters = array_new(OpBase *, childCount);
	apply_multiplexer->filters_arguments = array_new(Argument *, childCount);
	apply_multiplexer->branches = array_new(OpBase *, childCount);
	apply_multiplexer->branches_arguments = array_new(Argument *, childCount);
	for(int i = 0; i < childCount; i++) {
		OpBase *child = opBase->children[i];
		if(ApplyOpUtils_IsBoundBranch(child)) {
			assert(apply_multiplexer->bound_branch == NULL);
			apply_multiplexer->bound_branch = child;
			continue;
		}
		if(_isApplyBranch(child)) {

		} else {

		}


	}

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