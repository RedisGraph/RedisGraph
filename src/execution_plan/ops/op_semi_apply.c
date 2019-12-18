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

static Record _pullFromMatchStream(OpSemiApply *op) {
	// Propegate record to the top of the match stream.
	if(op->op_arg) Argument_AddRecord(op->op_arg, Record_Clone(op->r));
	return OpBase_Consume(op->match_branch);
}

/* This function pulls a record from the op's bounded branch, set it as an argument for the op branch stream
 * (the match branch) and consumes a record from the match branch.
 * If there is a record from the match branch, the bounded branch record is returned. */
static Record _SemiApplyConsume(OpBase *opBase) {
	OpSemiApply *op = (OpSemiApply *)opBase;
	while(true) {
		// Try to get a record from bound stream.
		op->r = OpBase_Consume(op->bound_branch);
		if(!op->r) return NULL; // Depleted.

		// Try to get a record from match stream.
		Record righthand_record = _pullFromMatchStream(op);
		if(righthand_record) {
			// Don't care for matched record.
			Record_Free(righthand_record);
			Record r = op->r;
			op->r = NULL;   // Null to avoid double free.
			return r;
		}
		// Did not managed to get a record from right-hand side, loop back and restart.
		Record_Free(op->r);
		op->r = NULL;
	}
}

/* This function pulls a record from the op's bounded branch, set it as an argument for the op branch stream
 * (the match branch) and consumes a record from the match branch.
 * If there is no record from the match branch, the bounded branch record is returned. */
static Record _AntiSemiApplyConsume(OpBase *opBase) {
	OpSemiApply *op = (OpSemiApply *)opBase;
	while(true) {
		// Try to get a record from bound stream.
		op->r = OpBase_Consume(op->bound_branch);
		if(!op->r) return NULL; // Depleted.

		/* Try pulling right stream
		 * return bounded stream record if match stream returns NULL. */
		Record righthand_record = _pullFromMatchStream(op);
		if(righthand_record) {
			/* managed to get a record from match stream,
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

OpBase *NewSemiApplyOp(ExecutionPlan *plan, bool anti) {
	OpSemiApply *op = rm_malloc(sizeof(OpSemiApply));
	op->r = NULL;
	op->op_arg = NULL;
	// Set our Op operations
	if(anti) {
		OpBase_Init((OpBase *)op, OPType_SEMI_APPLY, "Anti Semi Apply", SemiApplyInit,
					_AntiSemiApplyConsume,
					SemiApplyReset, NULL, SemiApplyFree, false, plan);
	} else {
		OpBase_Init((OpBase *)op, OPType_SEMI_APPLY, "Semi Apply", SemiApplyInit,
					_SemiApplyConsume,
					SemiApplyReset, NULL, SemiApplyFree, false, plan);
	}
	return (OpBase *) op;
}

OpResult SemiApplyInit(OpBase *opBase) {
	assert(opBase->childCount == 2);

	OpSemiApply *op = (OpSemiApply *)opBase;
	op->bound_branch = opBase->children[0];
	op->match_branch = opBase->children[1];

	// Locate branch's Argument op tap.
	op->op_arg = (Argument *)ExecutionPlan_LocateFirstOp(op->match_branch, OPType_ARGUMENT);
	if(op->op_arg) assert(op->op_arg->op.childCount == 0);
	return OP_OK;
}

OpResult SemiApplyReset(OpBase *opBase) {
	OpSemiApply *op = (OpSemiApply *)opBase;
	if(op->r) {
		Record_Free(op->r);
		op->r = NULL;
	}
	return OP_OK;
}

void SemiApplyFree(OpBase *opBase) {
	OpSemiApply *op = (OpSemiApply *)opBase;

	if(op->r) {
		Record_Free(op->r);
		op->r = NULL;
	}
}
