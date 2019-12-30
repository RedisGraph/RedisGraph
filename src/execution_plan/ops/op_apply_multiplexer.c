/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "op_apply_multiplexer.h"

// Forward declerations.
OpResult OpApplyMultiplexerInit(OpBase *opBase);
Record OrMultiplexer_Consume(OpBase *opBase);
Record AndMultiplexer_Consume(OpBase *opBase);
OpResult OpApplyMultiplexerReset(OpBase *opBase);
void OpApplyMultiplexerFree(OpBase *opBase);

static Record _pullFromBranchStream(OpApplyMultiplexer *op, int branch_index) {
	// Propegate record to the top of the match stream.
	Argument_AddRecord(op->branch_arguments[branch_index - 1],
					   Record_Clone(op->r));
	return OpBase_Consume(op->op.children[branch_index]);
}

OpBase *NewApplyMultiplexerOp(ExecutionPlan *plan, AST_Operator boolean_operator) {

	OpApplyMultiplexer *op = rm_calloc(1, sizeof(OpApplyMultiplexer));
	op->boolean_operator = boolean_operator;
	// Set our Op operations
	if(boolean_operator == OP_OR) {
		OpBase_Init((OpBase *)op, OPType_OR_APPLY_MULTIPLEXER, "OR Apply Multiplexer",
					OpApplyMultiplexerInit,
					OrMultiplexer_Consume, OpApplyMultiplexerReset, NULL, OpApplyMultiplexerFree, false, plan);
	} else if(boolean_operator == OP_AND) {
		OpBase_Init((OpBase *)op, OPType_AND_APPLY_MULTIPLEXER, "AND Apply Multiplexer",
					OpApplyMultiplexerInit,
					AndMultiplexer_Consume, OpApplyMultiplexerReset, NULL, OpApplyMultiplexerFree, false, plan);
	} else {
		assert("apply multiplexer boolean operator should be AND or OR only" && false);
	}
	return (OpBase *) op;
}

/* Sorts the multiplexer children. Apply operations to the very end (rightmost),
 * filter operations to the beginning (leftmost).
 * The filter operations evaluted faster then the apply operations so we want to evaluate them first. */
static void _OpApplyMultiplexer_SortChildren(OpBase *op) {
	for(int i = 1; i < op->childCount; i++) {
		OpBase *child = op->children[i];
		// Push apply ops to the end.
		if(child->type & (OPType_OR_APPLY_MULTIPLEXER | OPType_AND_APPLY_MULTIPLEXER | OPType_SEMI_APPLY |
						  OpType_ANTI_SEMI_APPLY)) {
			// From current position to the end, search for filter op.
			bool swapped = false;
			for(int j = i + 1; j < op->childCount; j++) {
				OpBase *candidate = op->children[j];
				if(candidate->type & OPType_FILTER) {
					OpBase *tmp = candidate;
					op->children[i] = candidate;
					op->children[j] = child;
					swapped = true;
					break;
				}
			}
			// No swap occurred, everything is sorted.
			if(!swapped) return;
		}
	}
}

/* In this init function, the children 1..n are sorted such that branches which are filter ops will be placed at
 * the begining of the children array, and branches which are apply ops will be placed at the end of the children array,
 * since filters are easier to evalute. For each branch the init function will collect it argument op, for the injection of
 * the bounded branch record. */
OpResult OpApplyMultiplexerInit(OpBase *opBase) {
	// Sort children.
	_OpApplyMultiplexer_SortChildren(opBase);
	OpApplyMultiplexer *apply_multiplexer = (OpApplyMultiplexer *) opBase;
	/* Set up bounded branch. The bounded branch is set as the first child during the operation building procedure at
	 * execution_plan_reduce_to_apply.c */
	apply_multiplexer->bound_branch = opBase->children[0];
	assert(apply_multiplexer->bound_branch);
	int childCount = opBase->childCount;
	// For every child, find its argument op for record injection.
	apply_multiplexer->branch_arguments = array_new(Argument *, childCount - 1);
	for(int i = 1; i < childCount; i++) {
		OpBase *child = opBase->children[i];
		Argument *arg = (Argument *)ExecutionPlan_LocateFirstOp(child, OPType_ARGUMENT);
		assert(arg);
		apply_multiplexer->branch_arguments = array_append(apply_multiplexer->branch_arguments, arg);
	}
	return OP_OK;
}

Record OrMultiplexer_Consume(OpBase *opBase) {
	OpApplyMultiplexer *op = (OpApplyMultiplexer *)opBase;
	while(true) {
		// Try to get a record from bound stream.
		op->r = OpBase_Consume(op->bound_branch);
		if(!op->r) return NULL; // Depleted.

		// Try to get a record from some stream.
		for(int i = 1; i < op->op.childCount; i++) {
			Record branch_record = _pullFromBranchStream(op, i);
			if(branch_record) {
				// Don't care about the branch record.
				Record_Free(branch_record);
				Record r = op->r;
				op->r = NULL;   // Null to avoid double free.
				return r;
			}
		}
		// Did not managed to get a record from any branch, loop back and restart.
		Record_Free(op->r);
		op->r = NULL;
	}
}

Record AndMultiplexer_Consume(OpBase *opBase) {
	OpApplyMultiplexer *op = (OpApplyMultiplexer *)opBase;
	while(true) {
		// Try to get a record from bound stream.
		op->r = OpBase_Consume(op->bound_branch);
		if(!op->r) return NULL; // Depleted.

		// Try to get a record from some stream.
		for(int i = 1; i < op->op.childCount; i++) {
			Record branch_record = _pullFromBranchStream(op, i);
			// Don't care about the branch record.
			if(branch_record) Record_Free(branch_record);
			else {
				// Did not managed to get a record from some branch, loop back and restart.
				Record_Free(op->r);
				op->r = NULL;
				break;
			}
		}
		// All branches returned record => all filters are satisfied by the bounded record.
		Record r = op->r;
		op->r = NULL;   // Null to avoid double free.
		return r;
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

	if(op->branch_arguments) {
		array_free(op->branch_arguments);
		op->branch_arguments = NULL;
	}

	if(op->r) {
		Record_Free(op->r);
		op->r = NULL;
	}
}
