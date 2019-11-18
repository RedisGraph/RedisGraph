/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "op_anti_semi_apply.h"
#include "../execution_plan.h"

// Forward declarations.
OpResult AntiSemiApplyInit(OpBase *opBase);
Record AntiSemiApplyConsume(OpBase *opBase);
OpResult AntiSemiApplyReset(OpBase *opBase);
void AntiSemiApplyFree(OpBase *opBase);

OpBase *NewAntiSemiApplyOp(const ExecutionPlan *plan) {
	AntiSemiApply *op = malloc(sizeof(AntiSemiApply));
	op->r = NULL;
	op->op_arg = NULL;

	// Set our Op operations
	OpBase_Init((OpBase *)op, OPType_ANTI_SEMI_APPLY, "Anti Semi Apply", AntiSemiApplyInit,
				AntiSemiApplyConsume, AntiSemiApplyReset, NULL, AntiSemiApplyFree, plan);

	return (OpBase *) op;
}

OpResult AntiSemiApplyInit(OpBase *opBase) {
	assert(opBase->childCount == 2);

	AntiSemiApply *op = (AntiSemiApply *)opBase;
	// Locate right-hand side Argument op tap.
	OpBase *right_handside = op->op.children[1];
	op->op_arg = (Argument *)ExecutionPlan_LocateOp(right_handside, OPType_ARGUMENT);
	if(op->op_arg) assert(op->op_arg->op.childCount == 0);
	return OP_OK;
}

static inline Record _pullFromStream(OpBase *branch) {
	return OpBase_Consume(branch);
}

static Record _pullFromRightStream(AntiSemiApply *op) {
	OpBase *right_handside = op->op.children[1];
	OpBase_PropagateReset(right_handside);
	// Propegate record to the top of the right-hand side stream.
	if(op->op_arg) Argument_AddRecord(op->op_arg, Record_Clone(op->r));
	return _pullFromStream(right_handside);
}

static Record _pullFromLeftStream(AntiSemiApply *op) {
	OpBase *left_handside = op->op.children[0];
	return _pullFromStream(left_handside);
}

Record AntiSemiApplyConsume(OpBase *opBase) {
	AntiSemiApply *op = (AntiSemiApply *)opBase;

	while(true) {
		// Try to get a record from left stream.
		op->r = _pullFromLeftStream(op);
		if(!op->r) return NULL; // Depleted.

		/* Try pulling right stream
		 * return left handside record if right handside returned NULL. */
		Record righthand_record = _pullFromRightStream(op);
		if(righthand_record) {
			/* managed to get a record from right handside,
			 * free it and pull again from left handside. */
			Record_Free(righthand_record);
			Record_Free(op->r);
			op->r = NULL;
		} else {
			// Right stream returned NULL, return left handside record.
			Record r = op->r;
			op->r = NULL;   // Null to avoid double free.
			return r;
		}
	}
}

OpResult AntiSemiApplyReset(OpBase *opBase) {
	AntiSemiApply *op = (AntiSemiApply *)opBase;
	if(op->r) {
		Record_Free(op->r);
		op->r = NULL;
	}
	return OP_OK;
}

void AntiSemiApplyFree(OpBase *opBase) {
	AntiSemiApply *op = (AntiSemiApply *)opBase;

	if(op->r) {
		Record_Free(op->r);
		op->r = NULL;
	}
}
