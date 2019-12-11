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

static Record _pullFromBranchStream(OpSemiApply *op) {
	// Propegate record to the top of the match stream.
	if(op->op_arg) Argument_AddRecord(op->op_arg, Record_Clone(op->r));
	return OpBase_Consume(op->match_branch);
}

static Record _OpSemiApply_SemiApplyLogic(OpSemiApply *op) {
	while(true) {
		// Try to get a record from bound stream.
		op->r = OpBase_Consume(op->bound_branch);
		if(!op->r) return NULL; // Depleted.

		// Try to get a record from match stream.
		Record righthand_record = _pullFromBranchStream(op);
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

static Record _OpSemiApply_AntiSemiApplyLogic(OpSemiApply *op) {
	while(true) {
		// Try to get a record from bound stream.
		op->r = OpBase_Consume(op->bound_branch);
		if(!op->r) return NULL; // Depleted.

		/* Try pulling right stream
		 * return bounded stream record if match stream returns NULL. */
		Record righthand_record = _pullFromBranchStream(op);
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

	const char *name;
	if(anti) {
		name = "Anti Semi Apply";
		op->apply_func = _OpSemiApply_AntiSemiApplyLogic;
	} else {
		name = "Semi Apply";
		op->apply_func = _OpSemiApply_SemiApplyLogic;
	}

	// Set our Op operations
	OpBase_Init((OpBase *)op, OPType_SEMI_APPLY, name, SemiApplyInit, SemiApplyConsume,
				SemiApplyReset, NULL, SemiApplyFree, false, plan);

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

Record SemiApplyConsume(OpBase *opBase) {
	OpSemiApply *op = (OpSemiApply *)opBase;
	return op->apply_func(op);



	/* Out of "infinity" loop either both left and right streams managed to produce data
	 * or we're depleted. */
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
