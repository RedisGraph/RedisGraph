/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "op_select_or_semi_apply.h"
#include "../execution_plan.h"

// Forward declarations.
void SelectOrSemiApplyFree(OpBase *opBase);
OpResult SelectOrSemiApplyInit(OpBase *opBase);
Record SelectOrSemiApplyConsume(OpBase *opBase);
OpResult SelectOrSemiApplyReset(OpBase *opBase);

OpBase *NewSelectOrSemiApplyOp(ExecutionPlan *plan, FT_FilterNode *filter, bool anti) {
	SelectOrSemiApply *op = malloc(sizeof(SelectOrSemiApply));
	op->r = NULL;
	op->anti = anti;
	op->op_arg = NULL;
	op->filter = filter;

	OPType op_type;
	const char *op_name;

	if(anti) {
		op_name = "Select Or Semi Apply";
		op_type = OPType_SELECT_OR_SEMI_APPLY;
	} else {
		op_name = "Select Or Anti Semi Apply";
		op_type = OPType_SELECT_OR_ANTI_SEMI_APPLY;
	}

	// Set our Op operations
	OpBase_Init((OpBase *)op, op_type, op_name, SelectOrSemiApplyInit, SelectOrSemiApplyConsume,
				SelectOrSemiApplyReset, NULL, SelectOrSemiApplyFree, plan);

	return(OpBase *) op;
}

OpResult SelectOrSemiApplyInit(OpBase *opBase) {
	assert(opBase->childCount == 2);

	SelectOrSemiApply *op = (SelectOrSemiApply *)opBase;
	// Locate right-hand side Argument op tap.
	OpBase *right_handside = op->op.children[1];
	op->op_arg = (Argument *)ExecutionPlan_LocateOp(right_handside, OPType_ARGUMENT);
	if(op->op_arg) assert(op->op_arg->op.childCount == 0);
	return OP_OK;
}

static inline Record _pullFromStream(OpBase *branch) {
	return OpBase_Consume(branch);
}

static Record _pullFromRightStream(SelectOrSemiApply *op) {
	OpBase *right_handside = op->op.children[1];
	OpBase_PropagateReset(right_handside);
	// Propegate record to the top of the right-hand side stream.
	if(op->op_arg) ArgumentSetRecord(op->op_arg, Record_Clone(op->r));
	return _pullFromStream(right_handside);
}

static Record _pullFromLeftStream(SelectOrSemiApply *op) {
	OpBase *left_handside = op->op.children[0];
	return _pullFromStream(left_handside);
}

Record SelectOrSemiApplyConsume(OpBase *opBase) {
	SelectOrSemiApply *op = (SelectOrSemiApply *)opBase;

	while(true) {
		// Try to get a record from left stream.
		op->r = _pullFromLeftStream(op);
		if(!op->r) return NULL; // Depleted.

		// See if record passes filter.
		if(op->filter && FilterTree_applyFilters(op->filter, op->r) == FILTER_PASS) {
			// Passed filter, no need to evaluate right stream.
			Record r = op->r;
			op->r = NULL;   // Null to avoid double free.
			return r;
		}

		// Filter did not pass, try right stream.
		Record righthand_record = _pullFromRightStream(op);
		if((!op->anti && righthand_record) ||
		   (op->anti && !righthand_record)) {
			// Don't care for righthand record.
			Record_Free(righthand_record);
			Record r = op->r;
			op->r = NULL;   // Null to avoid double free.
			return r;
		}

		// Did not managed to pass filter, loop back and retry.
		Record_Free(op->r);
		op->r = NULL;
	}
}

OpResult SelectOrSemiApplyReset(OpBase *opBase) {
	SelectOrSemiApply *op = (SelectOrSemiApply *)opBase;
	if(op->r) {
		Record_Free(op->r);
		op->r = NULL;
	}
	return OP_OK;
}

void SelectOrSemiApplyFree(OpBase *opBase) {
	SelectOrSemiApply *op = (SelectOrSemiApply *)opBase;

	if(op->r) {
		Record_Free(op->r);
		op->r = NULL;
	}
	if(op->filter) {
		FilterTree_Free(op->filter);
		op->filter = NULL;
	}
}
