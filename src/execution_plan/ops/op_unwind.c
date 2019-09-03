/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include <assert.h>
#include "op_unwind.h"
#include "../../util/arr.h"
#include "../../arithmetic/arithmetic_expression.h"

/* Forward declarations */
static void Free(OpBase *ctx);
static OpResult Reset(OpBase *ctx);
static OpResult Init(OpBase *opBase);
static Record Consume(OpBase *opBase);

OpBase *NewUnwindOp(const ExecutionPlan *plan, const char *alias, AR_ExpNode **exps) {
	OpUnwind *op = malloc(sizeof(OpUnwind));
	op->expIdx = 0;
	op->expressions = exps;

	// Set our Op operations
	OpBase_Init((OpBase *)op, OPType_UNWIND, "Unwind", Init, Consume, Reset, NULL, Free, plan);

	// Handle introduced entity
	OpBase_Modifies((OpBase *)op, alias);
	op->unwindRecIdx = -1;

	return (OpBase *)op;
}

static OpResult Init(OpBase *opBase) {
	return OP_OK;
}

static Record Consume(OpBase *opBase) {
	OpUnwind *op = (OpUnwind *)opBase;

	// Evaluated and returned all expressions.
	if(op->expIdx == array_len(op->expressions)) return NULL;

	AR_ExpNode *exp = op->expressions[op->expIdx];
	Record r = OpBase_CreateRecord((OpBase *)op);
	SIValue v = AR_EXP_Evaluate(exp, r);
	Record_AddScalar(r, op->unwindRecIdx, v);
	op->expIdx++;

	return r;
}

static OpResult Reset(OpBase *ctx) {
	OpUnwind *unwind = (OpUnwind *)ctx;
	unwind->expIdx = 0;
	return OP_OK;
}

static void Free(OpBase *ctx) {
	OpUnwind *unwind = (OpUnwind *)ctx;

	if(unwind->expressions) {
		uint expCount = array_len(unwind->expressions);
		for(uint i = 0; i < expCount; i++) AR_EXP_Free(unwind->expressions[i]);
		array_free(unwind->expressions);
		unwind->expressions = NULL;
	}
}
