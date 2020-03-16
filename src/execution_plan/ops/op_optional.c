/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "op_optional.h"

/* Forward declarations. */
static Record OptionalConsume(OpBase *opBase);
static OpResult OptionalReset(OpBase *opBase);
static OpBase *OptionalClone(const ExecutionPlan *plan, const OpBase *opBase);
static void OptionalFree(OpBase *opBase);

OpBase *NewOptionalOp(const ExecutionPlan *plan) {
	Optional *op = rm_malloc(sizeof(Optional));
	op->emitted_record = false;

	// Set our Op operations
	OpBase_Init((OpBase *)op, OPType_OPTIONAL, "Optional", NULL, OptionalConsume,
				OptionalReset, NULL, OptionalClone, OptionalFree, false, plan);

	return (OpBase *)op;
}

static Record OptionalConsume(OpBase *opBase) {
	Optional *op = (Optional *)opBase;
	Record r = OpBase_Consume(opBase->children[0]);
	if(!r && op->emitted_record == false) r = OpBase_CreateRecord(opBase);
	op->emitted_record = true;

	return r;
}

static OpResult OptionalReset(OpBase *opBase) {
	Optional *op = (Optional *)opBase;
	op->emitted_record = false;
	return OP_OK;
}

static inline OpBase *OptionalClone(const ExecutionPlan *plan, const OpBase *opBase) {
	return NewOptionalOp(plan);
}

static void OptionalFree(OpBase *ctx) {
	Optional *op = (Optional *)ctx;
	op->emitted_record = false;
}

