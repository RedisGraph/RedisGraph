/*
 * Copyright 2018-2021 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "op_semi_apply.h"

// Forward declarations.
static RT_OpResult SemiApplyInit(RT_OpBase *opBase);
static Record SemiApplyConsume(RT_OpBase *opBase);
static Record AntiSemiApplyConsume(RT_OpBase *opBase);
static RT_OpResult SemiApplyReset(RT_OpBase *opBase);
static void SemiApplyFree(RT_OpBase *opBase);

static inline Record _pullFromMatchStream(RT_OpSemiApply *op) {
	return RT_OpBase_Consume(op->match_branch);
}

RT_OpBase *RT_NewSemiApplyOp(const RT_ExecutionPlan *plan, const OpSemiApply *op_desc) {
	RT_OpSemiApply *op = rm_malloc(sizeof(RT_OpSemiApply));
	op->op_desc = op_desc;
	op->r = NULL;
	op->op_arg = NULL;
	op->bound_branch = NULL;
	op->match_branch = NULL;
	// Set our Op operations
	if(op_desc->op.type == OPType_ANTI_SEMI_APPLY) {
		RT_OpBase_Init((RT_OpBase *)op, (const OpBase *)&op_desc->op, SemiApplyInit,
					AntiSemiApplyConsume, SemiApplyReset, SemiApplyFree, plan);
	} else {
		RT_OpBase_Init((RT_OpBase *)op, (const OpBase *)&op_desc->op, SemiApplyInit, SemiApplyConsume,
					SemiApplyReset, SemiApplyFree, plan);
	}
	return (RT_OpBase *) op;
}

static RT_OpResult SemiApplyInit(RT_OpBase *opBase) {
	ASSERT(opBase->childCount == 2);

	RT_OpSemiApply *op = (RT_OpSemiApply *)opBase;
	/* The op bounded branch and match branch are set to be the first and second child, respectively,
	 * during the operation building procedure at execution_plan_reduce_to_apply.c */
	op->bound_branch = opBase->children[0];
	op->match_branch = opBase->children[1];
	ASSERT(op->bound_branch && op->match_branch);

	// Locate branch's Argument op tap.
	op->op_arg = (RT_Argument *)RT_ExecutionPlan_LocateOp(op->match_branch, OPType_ARGUMENT);
	ASSERT(op->op_arg && op->op_arg->op.childCount == 0);
	return OP_OK;
}

/* This function pulls a record from the op's bounded branch, set it as an argument for the op match branch
 * and consumes a record from the match branch. If there is a record from the match branch,
 * the bounded branch record is returned. */
static Record SemiApplyConsume(RT_OpBase *opBase) {
	RT_OpSemiApply *op = (RT_OpSemiApply *)opBase;
	while(true) {
		// Try to get a record from bound stream.
		op->r = RT_OpBase_Consume(op->bound_branch);
		if(!op->r) return NULL; // Depleted.
		// Propagate Record to the top of the Match stream.
		if(op->op_arg) Argument_AddRecord(op->op_arg, RT_OpBase_CloneRecord(op->r));

		Record rhs_record = _pullFromMatchStream(op);
		// Reset the match branch to maintain parity with the bound branch.
		RT_OpBase_PropagateReset(op->match_branch);
		if(rhs_record) {
			/* Successfully retrieved a Record from the match stream,
			 * free it and return the bound Record. */
			RT_OpBase_DeleteRecord(rhs_record);
			Record r = op->r;
			op->r = NULL;   // Null to avoid double free.
			return r;
		}
		// Did not manage to get a record from right-hand side, loop back and restart.
		RT_OpBase_DeleteRecord(op->r);
	}
}

/* This function pulls a record from the op's bounded branch, set it as an argument for the op match branch
 * and consumes a record from the match branch. If there is no record from the match branch,
 * the bounded branch record is returned. */
static Record AntiSemiApplyConsume(RT_OpBase *opBase) {
	RT_OpSemiApply *op = (RT_OpSemiApply *)opBase;
	while(true) {
		// Try to get a record from bound stream.
		op->r = RT_OpBase_Consume(op->bound_branch);
		if(!op->r) return NULL; // Depleted.

		// Propagate record to the top of the Match stream.
		// (Must clone the Record, as it will be freed in the Match stream.)
		if(op->op_arg) Argument_AddRecord(op->op_arg, RT_OpBase_CloneRecord(op->r));
		/* Try to pull data from the right stream,
		 * returning the bound stream record if unsuccessful. */
		Record rhs_record = _pullFromMatchStream(op);
		// Reset the match branch to maintain parity with the bound branch.
		RT_OpBase_PropagateReset(op->match_branch);
		if(rhs_record) {
			/* Successfully retrieved a Record from the match stream,
			 * free it and pull again from the bound stream. */
			RT_OpBase_DeleteRecord(rhs_record);
			RT_OpBase_DeleteRecord(op->r);
		} else {
			// Right stream returned NULL, return left handside record.
			Record r = op->r;
			op->r = NULL;   // Null to avoid double free.
			return r;
		}
	}
}

static RT_OpResult SemiApplyReset(RT_OpBase *opBase) {
	RT_OpSemiApply *op = (RT_OpSemiApply *)opBase;
	if(op->r) {
		RT_OpBase_DeleteRecord(op->r);
		op->r = NULL;
	}
	return OP_OK;
}

static void SemiApplyFree(RT_OpBase *opBase) {
	RT_OpSemiApply *op = (RT_OpSemiApply *)opBase;

	if(op->r) {
		RT_OpBase_DeleteRecord(op->r);
		op->r = NULL;
	}
}

