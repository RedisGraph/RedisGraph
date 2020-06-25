/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "op_apply.h"

/* Forward declarations. */
static OpResult ApplyInit(OpBase *opBase);
static Record ApplyConsume(OpBase *opBase);
static OpResult ApplyReset(OpBase *opBase);
static OpBase *ApplyClone(const ExecutionPlan *plan, const OpBase *opBase);
static void ApplyFree(OpBase *opBase);

OpBase *NewApplyOp(const ExecutionPlan *plan) {
	Apply *op = rm_malloc(sizeof(Apply));
	op->r = NULL;
	op->op_arg = NULL;
	op->bound_branch = NULL;
	op->rhs_branch = NULL;

	// Set our Op operations
	OpBase_Init((OpBase *)op, OPType_APPLY, "Apply", ApplyInit, ApplyConsume, ApplyReset, NULL,
				ApplyClone, ApplyFree, false, plan);

	return (OpBase *)op;
}

static OpResult ApplyInit(OpBase *opBase) {
	assert(opBase->childCount == 2);

	Apply *op = (Apply *)opBase;
	/* The op's bound branch and optional match branch have already been built as
	 * the Apply op's first and second child ops, respectively. */
	op->bound_branch = opBase->children[0];
	op->rhs_branch = opBase->children[1];

	// Locate branch's Argument op tap.
	op->op_arg = (Argument *)ExecutionPlan_LocateOp(op->rhs_branch, OPType_ARGUMENT);
	assert(op->op_arg);

	return OP_OK;
}

static Record ApplyConsume(OpBase *opBase) {
	Apply *op = (Apply *)opBase;

	while(true) {
		if(op->r == NULL) {
			// Retrieve a Record from the bound branch if we're not currently holding one.
			op->r = OpBase_Consume(op->bound_branch);
			if(!op->r) return NULL; // Bound branch and this op are depleted.

			// Successfully pulled a new Record, propagate to the top of the RHS branch.
			Argument_AddRecord(op->op_arg, OpBase_CloneRecord(op->r));
		}

		// Pull a Record from the RHS branch.
		Record rhs_record = OpBase_Consume(op->rhs_branch);

		if(rhs_record == NULL) {
			/* RHS branch depleted for the current bound Record;
			 * free it and loop back to retrieve a new one. */
			OpBase_DeleteRecord(op->r);
			op->r = NULL;
			// Reset the RHS branch.
			OpBase_PropagateReset(op->rhs_branch);
			continue;
		}

		// Clone the bound Record and merge the RHS Record into it.
		Record r = OpBase_CloneRecord(op->r);
		Record_Merge(&r, rhs_record);

		return r;
	}

	return NULL;
}

static OpResult ApplyReset(OpBase *opBase) {
	Apply *op = (Apply *)opBase;
	if(op->r) {
		OpBase_DeleteRecord(op->r);
		op->r = NULL;
	}
	return OP_OK;
}

static OpBase *ApplyClone(const ExecutionPlan *plan, const OpBase *opBase) {
	return NewApplyOp(plan);
}

static void ApplyFree(OpBase *opBase) {
	Apply *op = (Apply *)opBase;
	if(op->r) {
		OpBase_DeleteRecord(op->r);
		op->r = NULL;
	}
}

