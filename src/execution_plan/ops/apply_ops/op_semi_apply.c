/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "op_semi_apply.h"
#include "../../execution_plan.h"
#include "../op_argument.h"

// Forward declarations.
void SemiApplyFree(OpBase *opBase);
OpResult SemiApplyInit(OpBase *opBase);
Record SemiApplyConsume(OpBase *opBase);
OpResult SemiApplyReset(OpBase *opBase);


static inline Record _pullFromStream(OpBase *branch) {
	return OpBase_Consume(branch);
}

static Record _pullFromBranchStream(OpSemiApply *op) {
	OpBase *branch = op->op.children[1];
<<<<<<< HEAD
=======
	OpBase_PropagateReset(branch);
>>>>>>> introduced semi-apply
	// Propegate record to the top of the right-hand side stream.
	if(op->op_arg) Argument_AddRecord(op->op_arg, Record_Clone(op->r));
	return _pullFromStream(branch);
}

static Record _pullFromMainStream(OpSemiApply *op) {
	OpBase *main_stream = op->op.children[0];
	return _pullFromStream(main_stream);
}


static Record _OpSemiApply_SemiApplyLogic(OpSemiApply *op) {
	while(true) {
		// Try to get a record from left stream.
		op->r = _pullFromMainStream(op);
		if(!op->r) return NULL; // Depleted.

		// Try to get a record from right stream.
		Record righthand_record = _pullFromBranchStream(op);
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
}

static Record _OpSemiApply_AntiSemiApplyLogic(OpSemiApply *op) {
	while(true) {
		// Try to get a record from left stream.
		op->r = _pullFromMainStream(op);
		if(!op->r) return NULL; // Depleted.

		/* Try pulling right stream
		 * return left handside record if right handside returned NULL. */
		Record righthand_record = _pullFromBranchStream(op);
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

OpBase *NewSemiApplyOp(ExecutionPlan *plan, bool anti) {
	OpSemiApply *op = malloc(sizeof(OpSemiApply));
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
				SemiApplyReset, NULL, SemiApplyFree, plan);

	return (OpBase *) op;
}

OpResult SemiApplyInit(OpBase *opBase) {
	assert(opBase->childCount == 2);

	OpSemiApply *op = (OpSemiApply *)opBase;

	// If we only have 2 streams, we simply need to determine which has a MergeCreate op.
	if(ExecutionPlan_LocateOp(opBase->children[0], OPType_ARGUMENT)) {
		// If the Create op is in the first stream, swap the children.
		// Otherwise, the order is already correct.
		OpBase *tmp = opBase->children[0];
		opBase->children[0] = opBase->children[1];
		opBase->children[1] = tmp;
	}

	// Locate branch's Argument op tap.
	OpBase *branch = op->op.children[1];
	op->op_arg = (Argument *)ExecutionPlan_LocateOp(branch, OPType_ARGUMENT);
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
