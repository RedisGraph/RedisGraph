/*
 * Copyright 2018-2021 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "op_apply.h"

/* Forward declarations. */
static RT_OpResult ApplyInit(RT_OpBase *opBase);
static Record ApplyConsume(RT_OpBase *opBase);
static RT_OpResult ApplyReset(RT_OpBase *opBase);
static RT_OpBase *ApplyClone(const RT_ExecutionPlan *plan, const RT_OpBase *opBase);
static void ApplyFree(RT_OpBase *opBase);

RT_OpBase *RT_NewApplyOp(const RT_ExecutionPlan *plan, const Apply *op_desc) {
	RT_Apply *op = rm_malloc(sizeof(RT_Apply));
	op->op_desc = op_desc;
	op->r = NULL;
	op->op_arg = NULL;
	op->bound_branch = NULL;
	op->rhs_branch = NULL;

	// Set our Op operations
	RT_OpBase_Init((RT_OpBase *)op, OPType_APPLY, ApplyInit, ApplyConsume, ApplyReset,
				ApplyClone, ApplyFree, false, plan);

	return (RT_OpBase *)op;
}

static RT_OpResult ApplyInit(RT_OpBase *opBase) {
	ASSERT(opBase->childCount == 2);

	RT_Apply *op = (RT_Apply *)opBase;
	/* The op's bound branch and optional match branch have already been built as
	 * the Apply op's first and second child ops, respectively. */
	op->bound_branch = opBase->children[0];
	op->rhs_branch = opBase->children[1];

	// Locate branch's Argument op tap.
	op->op_arg = (RT_Argument *)RT_ExecutionPlan_LocateOp(op->rhs_branch, OPType_ARGUMENT);
	ASSERT(op->op_arg);

	return OP_OK;
}

static Record ApplyConsume(RT_OpBase *opBase) {
	RT_Apply *op = (RT_Apply *)opBase;

	while(true) {
		if(op->r == NULL) {
			// Retrieve a Record from the bound branch if we're not currently holding one.
			op->r = RT_OpBase_Consume(op->bound_branch);
			if(!op->r) return NULL; // Bound branch and this op are depleted.

			// Successfully pulled a new Record, propagate to the top of the RHS branch.
			Argument_AddRecord(op->op_arg, RT_OpBase_CloneRecord(op->r));
		}

		// Pull a Record from the RHS branch.
		Record rhs_record = RT_OpBase_Consume(op->rhs_branch);

		if(rhs_record == NULL) {
			/* RHS branch depleted for the current bound Record;
			 * free it and loop back to retrieve a new one. */
			RT_OpBase_DeleteRecord(op->r);
			op->r = NULL;
			// Reset the RHS branch.
			RT_OpBase_PropagateReset(op->rhs_branch);
			continue;
		}

		// Clone the bound Record and merge the RHS Record into it.
		Record r = RT_OpBase_CloneRecord(op->r);
		Record_Merge(r, rhs_record);
		// Delete the RHS record, as it has been merged into r.
		RT_OpBase_DeleteRecord(rhs_record);

		return r;
	}

	return NULL;
}

static RT_OpResult ApplyReset(RT_OpBase *opBase) {
	RT_Apply *op = (RT_Apply *)opBase;
	if(op->r) {
		RT_OpBase_DeleteRecord(op->r);
		op->r = NULL;
	}
	return OP_OK;
}

static RT_OpBase *ApplyClone(const RT_ExecutionPlan *plan, const RT_OpBase *opBase) {
	return RT_NewApplyOp(plan, ((RT_Apply *)opBase)->op_desc);
}

static void ApplyFree(RT_OpBase *opBase) {
	RT_Apply *op = (RT_Apply *)opBase;
	if(op->r) {
		RT_OpBase_DeleteRecord(op->r);
		op->r = NULL;
	}
}
