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
	unwind->listIdx = 0;
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
	return OP_OK;
}

Record UnwindConsume(OpBase *opBase) {
	OpUnwind *op = (OpUnwind *)opBase;
	// check if dynamic or static unwind
	if(op->op.childCount) {
		OpBase *child = op->op.children[0];
		// if the current list finished
		if(op->listIdx == array_len(op->list.array)) {
			Record r;
			// if there are new lists to unwind
			if((r = OpBase_Consume(child))) {
				// set the list
				op->list = AR_EXP_Evaluate(op->expressions, r);
				assert(op->list.type == T_ARRAY);
				// resed index
				op->listIdx = 0;
			}
			// no more new lists
			else return NULL;
		}
	} // op has no children, op has expressions to evaluate
	else if(op->expressions) {
		Record r = Record_New(opBase->record_map->record_len);
		// set the list
		op->list = AR_EXP_Evaluate(op->expressions, r);
		assert(op->list.type == T_ARRAY);
		// no need to evalute again, use as flag for re-entrance
		op->expressions = NULL;
	}
	// check if the static list has returned completly
	if(op->listIdx == array_len(op->list.array)) return NULL;

	Record r = Record_New(opBase->record_map->record_len);
	Record_AddScalar(r, op->unwindRecIdx, op->list.array[op->listIdx]);
	op->listIdx++;
	return r;
}

OpResult UnwindReset(OpBase *ctx) {
	OpUnwind *unwind = (OpUnwind *)ctx;
	unwind->listIdx = 0;
	return OP_OK;
}

void UnwindFree(OpBase *ctx) {
	OpUnwind *unwind = (OpUnwind *)ctx;

	if(unwind->expressions) {
		AR_EXP_Free(unwind->expressions);
	}
	SIValue_Free(&unwind->list);
}
