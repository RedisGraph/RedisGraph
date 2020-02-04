/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "op_semi_apply.h"
#include "../execution_plan.h"

// Forward declarations.
void SemiApplyFree(OpBase *opBase);
OpResult SemiApplyInit(OpBase *opBase);
Record SemiApplyConsume(OpBase *opBase);
Record AntiSemiApplyConsume(OpBase *opBase);
OpResult SemiApplyReset(OpBase *opBase);

static Record _pullFromMatchStream(OpSemiApply *op) {
	// Propegate record to the top of the match stream.
	if(op->op_arg) Argument_AddRecord(op->op_arg, OpBase_CloneRecord(op->r));
	return OpBase_Consume(op->match_branch);
}

OpBase *NewSemiApplyOp(ExecutionPlan *plan, bool anti) {
	OpSemiApply *op = rm_malloc(sizeof(OpSemiApply));
	op->r = NULL;
	op->op_arg = NULL;
	op->bound_branch = NULL;
	op->match_branch = NULL;
	// Set our Op operations
	if(anti) {
		OpBase_Init((OpBase *)op, OpType_ANTI_SEMI_APPLY, "Anti Semi Apply", SemiApplyInit,
					AntiSemiApplyConsume, SemiApplyReset, NULL, SemiApplyFree, false, plan);
	} else {
		OpBase_Init((OpBase *)op, OPType_SEMI_APPLY, "Semi Apply", SemiApplyInit, SemiApplyConsume,
					SemiApplyReset, NULL, SemiApplyFree, false, plan);
	}
	return (OpBase *) op;
}

OpResult SemiApplyInit(OpBase *opBase) {
	assert(opBase->childCount == 2);

	OpSemiApply *op = (OpSemiApply *)opBase;
	/* The op bounded branch and match branch are set to be the first and second child, respectively,
	 * during the operation building procedure at execution_plan_reduce_to_apply.c */
	op->bound_branch = opBase->children[0];
	op->match_branch = opBase->children[1];
	assert(op->bound_branch && op->match_branch);

	// Locate branch's Argument op tap.
	op->op_arg = (Argument *)ExecutionPlan_LocateFirstOp(op->match_branch, OPType_ARGUMENT);
	assert(op->op_arg && op->op_arg->op.childCount == 0);
	return OP_OK;
}

/* This function pulls a record from the op's bounded branch, set it as an argument for the op match branch
 * and consumes a record from the match branch. If there is a record from the match branch,
 * the bounded branch record is returned. */
Record SemiApplyConsume(OpBase *opBase) {
	OpSemiApply *op = (OpSemiApply *)opBase;
	while(true) {
		// Try to get a record from bound stream.
		op->r = OpBase_Consume(op->bound_branch);
		if(!op->r) return NULL; // Depleted.

		// Try to get a record from match stream.
		Record righthand_record = _pullFromMatchStream(op);
		if(righthand_record) {
			// Don't care about matched record.
			OpBase_DeleteRecord(righthand_record);
			Record r = op->r;
			op->r = NULL;   // Null to avoid double free.
			return r;
		}
		// Did not managed to get a record from right-hand side, loop back and restart.
		OpBase_DeleteRecord(op->r);
		op->r = NULL;
	}
}

/* This function pulls a record from the op's bounded branch, set it as an argument for the op match branch
 * and consumes a record from the match branch. If there is no record from the match branch,
 * the bounded branch record is returned. */
Record AntiSemiApplyConsume(OpBase *opBase) {
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
			OpBase_DeleteRecord(righthand_record);
			OpBase_DeleteRecord(op->r);
			op->r = NULL;
		} else {
			// Right stream returned NULL, return left handside record.
			Record r = op->r;
			op->r = NULL;   // Null to avoid double free.
			return r;
		}
	}
}

OpResult SemiApplyReset(OpBase *opBase) {
	OpSemiApply *op = (OpSemiApply *)opBase;
	if(op->r) {
		OpBase_DeleteRecord(op->r);
		op->r = NULL;
	}
	return OP_OK;
}

void SemiApplyFree(OpBase *opBase) {
	OpSemiApply *op = (OpSemiApply *)opBase;

	if(op->r) {
		OpBase_DeleteRecord(op->r);
		op->r = NULL;
	}
}
