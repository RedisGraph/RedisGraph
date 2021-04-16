/*
 * Copyright 2018-2021 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "op_rollup_apply.h"
#include "../../datatypes/array.h"
#include "../execution_plan_build/execution_plan_modify.h"

/* Forward declarations. */
static OpResult RollUpApplyInit(OpBase *opBase);
static Record RollUpApplyConsume(OpBase *opBase);
static OpResult RollUpApplyReset(OpBase *opBase);
static OpBase *RollUpApplyClone(const ExecutionPlan *plan, const OpBase *opBase);
static void RollUpApplyFree(OpBase *opBase);

OpBase *NewRollUpApplyOp(const ExecutionPlan *plan, const char *alias) {
	RollUpApply *op = rm_malloc(sizeof(RollUpApply));
	op->op_arg = NULL;
	op->bound_branch = NULL;
	op->rhs_branch = NULL;
	op->alias = alias;

	// Set our Op operations
	OpBase_Init((OpBase *)op, OPType_APPLY, "RollUpApply", RollUpApplyInit,
				RollUpApplyConsume, NULL, NULL, RollUpApplyClone,
				NULL, false, plan);

	op->alias_idx = OpBase_Modifies((OpBase *)op, alias);

	return (OpBase *)op;
}

static OpResult RollUpApplyInit(OpBase *opBase) {
	ASSERT(opBase->childCount == 2);

	RollUpApply *op = (RollUpApply *)opBase;
	/* The op's bound branch and optional match branch have already been built as
	 * the RollUpApply op's first and second child ops, respectively. */
	op->bound_branch = opBase->children[0];
	op->rhs_branch = opBase->children[1];

	// Locate branch's Argument op tap.
	op->op_arg = (Argument *)ExecutionPlan_LocateOp(op->rhs_branch, OPType_ARGUMENT);
	ASSERT(op->op_arg);

	return OP_OK;
}

static Record RollUpApplyConsume(OpBase *opBase) {
	RollUpApply *op = (RollUpApply *)opBase;

	while(true) {
		// Retrieve a Record from the bound branch.
		Record r = OpBase_Consume(op->bound_branch);
		if(r == NULL) return NULL; // Bound branch and this op are depleted.

		// Successfully pulled a new Record, propagate to the top of the RHS branch.
		Argument_AddRecord(op->op_arg, OpBase_CloneRecord(r));

		Record rhs_record;
		SIValue rolled_array = SI_Array(1);
		// Pull and aggregate all records from the RHS branch.
		while((rhs_record = OpBase_Consume(op->rhs_branch))) {
			// Retrieve the value being aggregated from the RHS branch.
			SIValue rollup_val = Record_Get(rhs_record, op->alias_idx);
			// Add the value to the concatenation array.
			SIArray_Append(&rolled_array, rollup_val);
			// Release the record.
			OpBase_DeleteRecord(rhs_record);
		}

		// Reset the RHS branch.
		OpBase_PropagateReset(op->rhs_branch);

		// Merge the concatenated array into the bound Record and return it..
		Record_AddScalar(r, op->alias_idx, rolled_array);

		return r;
	}

	return NULL;
}

static OpBase *RollUpApplyClone(const ExecutionPlan *plan, const OpBase *opBase) {
	RollUpApply *op = (RollUpApply *)opBase;
	return NewRollUpApplyOp(plan, op->alias);
}

