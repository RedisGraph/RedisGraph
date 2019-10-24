/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "op_semi_apply.h"
#include "../execution_plan.h"

// Forward declarations.
void SemiApplyFree(OpBase *opBase);
OpResult SemiApplyInit(OpBase *opBase);
Record SemiApplyConsume(OpBase *opBase);
OpResult SemiApplyReset(OpBase *opBase);

OpBase *NewSemiApplyOp(ExecutionPlan *plan) {
	SemiApply *op = malloc(sizeof(SemiApply));
	op->r = NULL;
	op->op_arg = NULL;

	// Set our Op operations
	OpBase_Init((OpBase *)op, OPType_SEMI_APPLY, "Semi Apply", SemiApplyInit, SemiApplyConsume,
				SemiApplyReset, NULL, SemiApplyFree, plan);

	return (OpBase *) op;
}

OpResult SemiApplyInit(OpBase *opBase) {
	assert(opBase->childCount == 2);

	SemiApply *op = (SemiApply *)opBase;
	// Locate right-hand side Argument op tap.
	OpBase *right_handside = op->op.children[1];
	op->op_arg = (Argument *)ExecutionPlan_LocateOp(right_handside, OPType_ARGUMENT);
	if(op->op_arg) assert(op->op_arg->op.childCount == 0);
	return OP_OK;
}

static inline Record _pullFromStream(OpBase *branch) {
	return OpBase_Consume(branch);
}

static Record _pullFromRightStream(SemiApply *op) {
	OpBase *right_handside = op->op.children[1];
	OpBase_PropagateReset(right_handside);
	// Propegate record to the top of the right-hand side stream.
	if(op->op_arg) ArgumentSetRecord(op->op_arg, Record_Clone(op->r));
	return _pullFromStream(right_handside);
}

static Record _pullFromLeftStream(SemiApply *op) {
	OpBase *left_handside = op->op.children[0];
	return _pullFromStream(left_handside);
}

Record SemiApplyConsume(OpBase *opBase) {
	SemiApply *op = (SemiApply *)opBase;

	while(true) {
		// Try to get a record from left stream.
		op->r = _pullFromLeftStream(op);
		if(!op->r) return NULL; // Depleted.

		// Try to get a record from right stream.
		Record righthand_record = _pullFromRightStream(op);
		if(righthand_record) {
			// Don't care for righthand record.
			Record_Free(righthand_record);
			Record r = op->r;
			op->r = NULL;   // Null to avoid double free.
			return r;
		}
		// Did not managed to get a record from right-hand side, loop back and restart.
		Record_Free(op->r);
		op->r = NULL;
	}

	/* Out of "infinity" loop either both left and right streams managed to produce data
	 * or we're depleted. */
}

OpResult SemiApplyReset(OpBase *opBase) {
	SemiApply *op = (SemiApply *)opBase;
	if(op->r) {
		Record_Free(op->r);
		op->r = NULL;
	}
	return OP_OK;
}

void SemiApplyFree(OpBase *opBase) {
	SemiApply *op = (SemiApply *)opBase;

	if(op->r) {
		Record_Free(op->r);
		op->r = NULL;
	}
}
