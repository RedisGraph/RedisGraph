/*
 * Copyright 2018-2021 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "op_optional.h"

/* Forward declarations. */
static Record OptionalConsume(RT_OpBase *opBase);
static RT_OpResult OptionalReset(RT_OpBase *opBase);
static RT_OpBase *OptionalClone(const RT_ExecutionPlan *plan, const RT_OpBase *opBase);

RT_OpBase *RT_NewOptionalOp(const RT_ExecutionPlan *plan, const Optional *op_desc) {
	RT_Optional *op = rm_malloc(sizeof(RT_Optional));
	op->op_desc = op_desc;
	op->emitted_record = false;

	// Set our Op operations
	RT_OpBase_Init((RT_OpBase *)op, OPType_OPTIONAL, NULL, OptionalConsume,
				OptionalReset, OptionalClone, NULL, false, plan);

	return (RT_OpBase *)op;
}

static Record OptionalConsume(RT_OpBase *opBase) {
	RT_Optional *op = (RT_Optional *)opBase;
	// Try to produce a Record from the child op.
	Record r = RT_OpBase_Consume(opBase->children[0]);

	// Create an empty Record if the child returned NULL and this op has not yet returned data.
	if(!r && !op->emitted_record) r = RT_OpBase_CreateRecord(opBase);

	// Don't produce multiple empty Records.
	op->emitted_record = true;

	return r;
}

static RT_OpResult OptionalReset(RT_OpBase *opBase) {
	RT_Optional *op = (RT_Optional *)opBase;
	op->emitted_record = false;
	return OP_OK;
}

static inline RT_OpBase *OptionalClone(const RT_ExecutionPlan *plan, const RT_OpBase *opBase) {
	return RT_NewOptionalOp(plan, ((RT_Optional *)opBase)->op_desc);
}
