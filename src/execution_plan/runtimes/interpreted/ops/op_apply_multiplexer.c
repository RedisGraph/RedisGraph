/*
 * Copyright 2018-2021 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "op_apply_multiplexer.h"

// Forward declerations.
static RT_OpResult OpApplyMultiplexerInit(RT_OpBase *opBase);
static Record OrMultiplexer_Consume(RT_OpBase *opBase);
static Record AndMultiplexer_Consume(RT_OpBase *opBase);
static RT_OpResult OpApplyMultiplexerReset(RT_OpBase *opBase);
static RT_OpBase *OpApplyMultiplexerClone(const RT_ExecutionPlan *plan, const RT_OpBase *opBase);
static void OpApplyMultiplexerFree(RT_OpBase *opBase);

static Record _pullFromBranchStream(RT_OpApplyMultiplexer *op, int branch_index) {
	// Propegate record to the top of the match stream.
	Argument_AddRecord(op->branch_arguments[branch_index - 1], RT_OpBase_CloneRecord(op->r));
	return RT_OpBase_Consume(op->op.children[branch_index]);
}

RT_OpBase *RT_NewApplyMultiplexerOp(const RT_ExecutionPlan *plan, AST_Operator boolean_operator) {

	RT_OpApplyMultiplexer *op = rm_calloc(1, sizeof(RT_OpApplyMultiplexer));
	op->boolean_operator = boolean_operator;
	// Set our Op operations
	if(boolean_operator == OP_OR) {
		RT_OpBase_Init((RT_OpBase *)op, OPType_OR_APPLY_MULTIPLEXER,
					OpApplyMultiplexerInit, OrMultiplexer_Consume, OpApplyMultiplexerReset,
					OpApplyMultiplexerClone, OpApplyMultiplexerFree, false, plan);
	} else if(boolean_operator == OP_AND) {
		RT_OpBase_Init((RT_OpBase *)op, OPType_AND_APPLY_MULTIPLEXER,
					OpApplyMultiplexerInit, AndMultiplexer_Consume, OpApplyMultiplexerReset,
					OpApplyMultiplexerClone, OpApplyMultiplexerFree, false, plan);
	} else {
		ASSERT("apply multiplexer boolean operator should be AND or OR only" && false);
	}
	return (RT_OpBase *) op;
}

/* Sorts the multiplexer children. Apply operations to the very end (rightmost),
 * filter operations to the beginning (leftmost).
 * The filter operations evaluted faster then the apply operations so we want to evaluate them first. */
static void _OpApplyMultiplexer_SortChildren(RT_OpBase *op) {
	// The 0 child is the bounded branch of this operation, which consumes the record and is set on execution_plan_reduce_to_apply.c
	for(int i = 1; i < op->childCount; i++) {
		RT_OpBase *child = op->children[i];
		// Push apply ops to the end.
		if(OP_IS_APPLY(child)) {
			// From current position to the end, search for filter op.
			bool swapped = false;
			for(int j = i + 1; j < op->childCount; j++) {
				RT_OpBase *candidate = op->children[j];
				if(candidate->type == OPType_FILTER) {
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
static RT_OpResult OpApplyMultiplexerInit(RT_OpBase *opBase) {
	// Sort children.
	_OpApplyMultiplexer_SortChildren(opBase);
	RT_OpApplyMultiplexer *apply_multiplexer = (RT_OpApplyMultiplexer *) opBase;
	/* Set up bounded branch. The bounded branch is set as the first child during the operation building procedure at
	 * execution_plan_reduce_to_apply.c */
	apply_multiplexer->bound_branch = opBase->children[0];
	ASSERT(apply_multiplexer->bound_branch);
	int childCount = opBase->childCount;
	// For every child, find its argument op for record injection.
	apply_multiplexer->branch_arguments = array_new(RT_Argument *, childCount - 1);
	for(int i = 1; i < childCount; i++) {
		RT_OpBase *child = opBase->children[i];
		RT_Argument *arg = (RT_Argument *)RT_ExecutionPlan_LocateOp(child, OPType_ARGUMENT);
		ASSERT(arg);
		array_append(apply_multiplexer->branch_arguments, arg);
	}
	return OP_OK;
}

static Record OrMultiplexer_Consume(RT_OpBase *opBase) {
	RT_OpApplyMultiplexer *op = (RT_OpApplyMultiplexer *)opBase;
	while(true) {
		// Try to get a record from bound stream.
		op->r = RT_OpBase_Consume(op->bound_branch);
		if(!op->r) return NULL; // Depleted.

		// Try to get a record from some stream.
		for(int i = 1; i < op->op.childCount; i++) {
			Record branch_record = _pullFromBranchStream(op, i);
			if(branch_record) {
				// Don't care about the branch record.
				RT_OpBase_DeleteRecord(branch_record);
				Record r = op->r;
				op->r = NULL;   // Null to avoid double free.
				return r;
			}
		}
		// Did not managed to get a record from any branch, loop back and restart.
		RT_OpBase_DeleteRecord(op->r);
		op->r = NULL;
	}
}

static Record AndMultiplexer_Consume(RT_OpBase *opBase) {
	RT_OpApplyMultiplexer *op = (RT_OpApplyMultiplexer *)opBase;
	while(true) {
		// Try to get a record from bound stream.
		op->r = RT_OpBase_Consume(op->bound_branch);
		if(!op->r) return NULL; // Depleted.

		// Try to get a record from some stream.
		for(int i = 1; i < op->op.childCount; i++) {
			Record branch_record = _pullFromBranchStream(op, i);
			// Don't care about the branch record.
			if(branch_record) RT_OpBase_DeleteRecord(branch_record);
			else {
				// Did not managed to get a record from some branch, loop back and restart.
				RT_OpBase_DeleteRecord(op->r);
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

static RT_OpResult OpApplyMultiplexerReset(RT_OpBase *opBase) {
	RT_OpApplyMultiplexer *op = (RT_OpApplyMultiplexer *)opBase;
	if(op->r) {
		RT_OpBase_DeleteRecord(op->r);
		op->r = NULL;
	}
	return OP_OK;
}

static inline RT_OpBase *OpApplyMultiplexerClone(const RT_ExecutionPlan *plan, const RT_OpBase *opBase) {
	ASSERT(opBase->type == OPType_OR_APPLY_MULTIPLEXER || opBase->type == OPType_AND_APPLY_MULTIPLEXER);
	RT_OpApplyMultiplexer *op = (RT_OpApplyMultiplexer *)opBase;
	return RT_NewApplyMultiplexerOp(plan, op->boolean_operator);
}

static void OpApplyMultiplexerFree(RT_OpBase *opBase) {
	RT_OpApplyMultiplexer *op = (RT_OpApplyMultiplexer *)opBase;

	if(op->branch_arguments) {
		array_free(op->branch_arguments);
		op->branch_arguments = NULL;
	}

	if(op->r) {
		RT_OpBase_DeleteRecord(op->r);
		op->r = NULL;
	}
}
