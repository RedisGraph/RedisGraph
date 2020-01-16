/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "op_apply.h"

/* Forward declarations. */
static Record ApplyConsume(OpBase *opBase);
static OpResult ApplyReset(OpBase *opBase);
static void ApplyFree(OpBase *opBase);

OpBase *NewApplyOp(const ExecutionPlan *plan) {
	Apply *op = rm_malloc(sizeof(Apply));
	op->init = true;
	op->rhs_idx = 0;
	op->lhs_record = NULL;
	op->rhs_records = array_new(Record, 32);

	// Set our Op operations
	OpBase_Init((OpBase *)op, OPType_APPLY, "Apply", NULL, ApplyConsume, ApplyReset, NULL,
				ApplyFree, false, plan);

	return (OpBase *)op;
}

static Record ApplyConsume(OpBase *opBase) {
	Apply *op = (Apply *)opBase;

	assert(op->op.childCount == 2);

	Record rhs_record;

	if(op->init) {
		// On the first call to ApplyConsume, we'll read the entirety of the
		// right-hand stream and buffer its outputs.
		op->init = false;

		OpBase *rhs = op->op.children[1];
		while((rhs_record = rhs->consume(rhs))) {
			op->rhs_records = array_append(op->rhs_records, rhs_record);
		}
	}

	uint rhs_count = array_len(op->rhs_records);

	OpBase *lhs = op->op.children[0];
	if(op->lhs_record == NULL) {
		// No pending data from left-hand stream
		op->rhs_idx = 0;
		op->lhs_record = lhs->consume(lhs);
	}

	if(op->lhs_record == NULL) {
		// Left-hand stream has been fully depleted
		return NULL;
	}

	// Clone the left-hand record
	Record r = OpBase_CloneRecord(op->lhs_record);

	// No records were produced by the right-hand stream
	if(rhs_count == 0) return r;

	rhs_record = op->rhs_records[op->rhs_idx++];

	if(op->rhs_idx == rhs_count) {
		// We've joined all data from the right-hand stream with the current
		// retrieval from the left-hand stream.
		// The next call to ApplyConsume will attempt to pull new data.
		OpBase_DeleteRecord(op->lhs_record);
		op->lhs_record = NULL;
		op->rhs_idx = 0;
	}

	Record_Merge(&r, rhs_record);

	return r;
}

static OpResult ApplyReset(OpBase *opBase) {
	Apply *op = (Apply *)opBase;
	op->init = true;
	return OP_OK;
}

static void ApplyFree(OpBase *opBase) {
	Apply *op = (Apply *)opBase;
	if(op->lhs_record) {
		OpBase_DeleteRecord(op->lhs_record);
		op->lhs_record = NULL;
	}
	if(op->rhs_records) {
		uint len = array_len(op->rhs_records);
		for(uint i = 0; i < len; i ++) {
			OpBase_DeleteRecord(op->rhs_records[i]);
		}
		array_free(op->rhs_records);
		op->rhs_records = NULL;
	}
}

