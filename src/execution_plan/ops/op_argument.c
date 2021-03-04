/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "op_argument.h"
#include "RG.h"

// Forward declarations
static Record ArgumentConsume(OpBase *opBase);
static OpResult ArgumentReset(OpBase *opBase);
static OpBase *ArgumentClone(const ExecutionPlan *plan, const OpBase *opBase);
static void ArgumentFree(OpBase *opBase);

OpBase *NewArgumentOp(const ExecutionPlan *plan, const char **variables) {
	Argument *op = rm_malloc(sizeof(Argument));
	op->r = NULL;

	// Set our Op operations
	OpBase_Init((OpBase *)op, OPType_ARGUMENT, "Argument", NULL,
				ArgumentConsume, ArgumentReset, NULL, ArgumentClone, ArgumentFree, false, plan);

	uint variable_count = array_len(variables);
	for(uint i = 0; i < variable_count; i ++) {
		OpBase_Modifies((OpBase *)op, variables[i]);
	}

	return (OpBase *)op;
}

static Record ArgumentConsume(OpBase *opBase) {
	Argument *arg = (Argument *)opBase;

	// Emit the record only once.
	// arg->r can already be NULL if the op is depleted.
	Record r = arg->r;
	arg->r = NULL;
	return r;
}

static OpResult ArgumentReset(OpBase *opBase) {
	// Reset operation, freeing the Record if one is held.
	Argument *arg = (Argument *)opBase;

	if(arg->r) {
		OpBase_DeleteRecord(arg->r);
		arg->r = NULL;
	}

	return OP_OK;
}

void Argument_AddRecord(Argument *arg, Record r) {
	/* In normal scenarios, arg->r will always be NULL at this point.
	 * However, if the op tree has been modified by an external actor like a query timeout,
	 * it is possible for this Record to have never been retrieved.
	 * In this case simply free the held Record, as it won't be used. */
	if(arg->r) Record_Free(arg->r);
	arg->r = r;
}

static inline OpBase *ArgumentClone(const ExecutionPlan *plan, const OpBase *opBase) {
	ASSERT(opBase->type == OPType_ARGUMENT);
	return NewArgumentOp(plan, opBase->modifies);
}

static void ArgumentFree(OpBase *opBase) {
	Argument *arg = (Argument *)opBase;
	if(arg->r) {
		OpBase_DeleteRecord(arg->r);
		arg->r = NULL;
	}
}

