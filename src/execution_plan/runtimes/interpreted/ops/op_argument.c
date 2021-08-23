/*
 * Copyright 2018-2021 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "RG.h"
#include "op_argument.h"

// Forward declarations
static Record ArgumentConsume(RT_OpBase *opBase);
static RT_OpResult ArgumentReset(RT_OpBase *opBase);
static RT_OpBase *ArgumentClone(const RT_ExecutionPlan *plan, const RT_OpBase *opBase);
static void ArgumentFree(RT_OpBase *opBase);

RT_OpBase *RT_NewArgumentOp(const RT_ExecutionPlan *plan) {
	RT_Argument *op = rm_malloc(sizeof(RT_Argument));
	op->r = NULL;

	// Set our Op operations
	RT_OpBase_Init((RT_OpBase *)op, OPType_ARGUMENT, NULL,
				ArgumentConsume, ArgumentReset, ArgumentClone, ArgumentFree, false, plan);

	return (RT_OpBase *)op;
}

static Record ArgumentConsume(RT_OpBase *opBase) {
	RT_Argument *arg = (RT_Argument *)opBase;

	// Emit the record only once.
	// arg->r can already be NULL if the op is depleted.
	Record r = arg->r;
	arg->r = NULL;
	return r;
}

static RT_OpResult ArgumentReset(RT_OpBase *opBase) {
	// Reset operation, freeing the Record if one is held.
	RT_Argument *arg = (RT_Argument *)opBase;

	if(arg->r) {
		RT_OpBase_DeleteRecord(arg->r);
		arg->r = NULL;
	}

	return OP_OK;
}

void Argument_AddRecord(RT_Argument *arg, Record r) {
	ASSERT(!arg->r && "tried to insert into a populated Argument op");
	arg->r = r;
}

static inline RT_OpBase *ArgumentClone(const RT_ExecutionPlan *plan, const RT_OpBase *opBase) {
	ASSERT(opBase->type == OPType_ARGUMENT);
	return RT_NewArgumentOp(plan);
}

static void ArgumentFree(RT_OpBase *opBase) {
	RT_Argument *arg = (RT_Argument *)opBase;
	if(arg->r) {
		RT_OpBase_DeleteRecord(arg->r);
		arg->r = NULL;
	}
}
