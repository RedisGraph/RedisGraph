/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include <assert.h>
#include "op_unwind.h"
#include "../../util/arr.h"
#include "../../arithmetic/arithmetic_expression.h"
#include "limits.h"

#define UNDEFIND UINT_MAX

OpBase *NewUnwindOp(uint record_idx, AR_ExpNode *exps) {
	OpUnwind *unwind = malloc(sizeof(OpUnwind));
	unwind->listIdx = UNDEFIND;
	unwind->expressions = exps;
	unwind->isStatic = false;
	unwind->currentRecord = NULL;

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
	OpUnwind *unwind = (OpUnwind *)opBase;

	// check if there are children - if not, add static record
	if(!unwind->op.childCount)
		unwind->currentRecord = Record_New(opBase->record_map->record_len);

	// check for modifiers in the AR_EXP for static or dynamic list
	rax *modifies = raxNew();
	AR_EXP_CollectEntityIDs(unwind->expressions, modifies);
	// if there aren't any modifiers - list is static
	if(!raxSize(modifies)) {
		// set the list
		unwind->list = AR_EXP_Evaluate(unwind->expressions, NULL);
		assert(unwind->list.type == T_ARRAY);
		unwind->isStatic = true;
		unwind->listIdx = 0;
	}

	return OP_OK;
}

// try to generate new value to return
// NULL will be returned if dynamic list is not evaluted (listIdx = UNDEFIND)
// or in case of the current list fully returned its memebers
Record _handoff(OpUnwind *op) {
	// if there is a new value ready, return it
	if(op->listIdx < array_len(op->list.array)) {
		Record r = Record_Clone(op->currentRecord);
		Record_AddScalar(r, op->unwindRecIdx, op->list.array[op->listIdx]);
		op->listIdx++;
		return r;
	}
	return NULL;
}

Record UnwindConsume(OpBase *opBase) {
	OpUnwind *op = (OpUnwind *)opBase;

	// check for new value
	Record res = _handoff(op);
	if(res) return res;

	// no new value - check if there are new lists to unwind
	// check if dynamic or static unwind
	if(op->op.childCount) {
		Record r;
		OpBase *child = op->op.children[0];
		// if there are new lists to unwind
		if((r = OpBase_Consume(child))) {
			Record_Free(op->currentRecord);
			op->currentRecord = r;

			// TODO: if expression is static or not
			if(!op->isStatic) {
				SIValue_Free(&op->list);
				// set the list
				op->list = AR_EXP_Evaluate(op->expressions, r);
				assert(op->list.type == T_ARRAY);
			}
			// resed index
			op->listIdx = 0;
		}
		// no more new lists
		else return NULL;
	} // op has no children, static list returned fully
	else return NULL;

	return _handoff(op);
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
	Record_Free(unwind->currentRecord);
	SIValue_Free(&unwind->list);
}
