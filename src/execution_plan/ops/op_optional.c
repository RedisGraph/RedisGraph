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

OpBase *NewOptionalOp(const ExecutionPlan *plan) {
	Optional *op = rm_malloc(sizeof(Optional));
	op->emitted_record = false;
	op->use_default_value = false;
	op->default_value = SI_NullVal();
	op->alias = NULL;
	op->idx = 0;

	// Set our Op operations
	OpBase_Init((OpBase *)op, OPType_OPTIONAL, "Optional", NULL, OptionalConsume,
				OptionalReset, NULL, OptionalClone, NULL, false, plan);

	return (OpBase *)op;
}

void OptionalOp_DefaultValue(Optional *op, const char *alias, SIValue value) {
	op->use_default_value = true;
	op->default_value = value;
	op->alias = alias;
	bool aware = OpBase_Aware((OpBase *)op, alias, &op->idx);
	ASSERT(aware);
}

static Record OptionalConsume(OpBase *opBase) {
	Optional *op = (Optional *)opBase;
	// Try to produce a Record from the child op.
	Record r = OpBase_Consume(opBase->children[0]);

	// Create an empty Record if the child returned NULL and this op has not yet returned data.
	if(!r && !op->emitted_record) {
		r = OpBase_CreateRecord(opBase);
		if(op->use_default_value) {
			Record_Add(r, op->idx, SI_CloneValue(op->default_value));
		}
	}

	// Don't produce multiple empty Records.
	op->emitted_record = true;

	return r;
}

static OpResult OptionalReset(OpBase *opBase) {
	Optional *op = (Optional *)opBase;
	op->emitted_record = false;
	return OP_OK;
}

static inline OpBase *OptionalClone(const ExecutionPlan *plan, const OpBase *opBase) {
	Optional *op = (Optional *)opBase;
	OpBase *clone = NewOptionalOp(plan);
	if(op->use_default_value) {
		OptionalOp_DefaultValue((Optional *)clone, op->alias, op->default_value);
	}
	return clone;
}

