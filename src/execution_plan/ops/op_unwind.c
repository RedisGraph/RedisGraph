/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include <assert.h>
#include "op_unwind.h"
#include "../../util/arr.h"
#include "../../arithmetic/arithmetic_expression.h"

OpBase *NewUnwindOp(uint record_idx, AR_ExpNode *exps) {
	OpUnwind *unwind = malloc(sizeof(OpUnwind));
	unwind->expIdx = 0;
	unwind->expressions = exps;

	// Set our Op operations
	OpBase_Init(&unwind->op);
	unwind->op.name = "Unwind";
	unwind->op.type = OPType_UNWIND;
	unwind->op.consume = UnwindConsume;
	unwind->op.init = UnwindInit;
	unwind->op.reset = UnwindReset;
	unwind->op.free = UnwindFree;

	// Handle introduced entity
	unwind->op.modifies = array_new(uint, 1);
	unwind->op.modifies = array_append(unwind->op.modifies, record_idx);
	unwind->unwindRecIdx = record_idx;

	return (OpBase *)unwind;
}

OpResult UnwindInit(OpBase *opBase) {
	OpUnwind *op = (OpUnwind *)opBase;
	Record r = Record_New(opBase->record_map->record_len);
	op->list = AR_EXP_Evaluate(op->expressions, r);
	return OP_OK;
}

Record UnwindConsume(OpBase *opBase) {
	OpUnwind *op = (OpUnwind *)opBase;

	// Evaluated and returned all expressions.
	if(op->expIdx == array_len(op->list.array)) return NULL;
	Record r = Record_New(opBase->record_map->record_len);
	Record_AddScalar(r, op->unwindRecIdx, op->list.array[op->expIdx]);
	op->expIdx++;

	return r;
}

OpResult UnwindReset(OpBase *ctx) {
	OpUnwind *unwind = (OpUnwind *)ctx;
	unwind->expIdx = 0;
	return OP_OK;
}

void UnwindFree(OpBase *ctx) {
	OpUnwind *unwind = (OpUnwind *)ctx;

	if(unwind->expressions) {
		AR_EXP_Free(unwind->expressions);
	}
	SIValue_Free(&unwind->list);
}
