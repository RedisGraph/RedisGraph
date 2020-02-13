/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "op_argument.h"

// Forward declarations
Record ArgumentConsume(OpBase *opBase);
OpResult ArgumentReset(OpBase *opBase);
void ArgumentFree(OpBase *opBase);

OpBase *NewArgumentOp(const ExecutionPlan *plan, const char **variables) {
	Argument *op = rm_malloc(sizeof(Argument));
	op->r = NULL;

	// Set our Op operations
	OpBase_Init((OpBase *)op, OPType_ARGUMENT, "Argument", NULL,
				ArgumentConsume, ArgumentReset, NULL, ArgumentFree, false, plan);

	uint variable_count = array_len(variables);
	for(uint i = 0; i < variable_count; i ++) {
		OpBase_Modifies((OpBase *)op, variables[i]);
	}

	return (OpBase *)op;
}

Record ArgumentConsume(OpBase *opBase) {
	Argument *arg = (Argument *)opBase;

	// Emit the record only once.
	// arg->r can already be NULL if the op is depleted.
	Record r = arg->r;
	arg->r = NULL;
	return r;
}

OpResult ArgumentReset(OpBase *opBase) {
	// Reset operation, freeing the Record if one is held.
	Argument *arg = (Argument *)opBase;

	if(arg->r) {
		OpBase_DeleteRecord(arg->r);
		arg->r = NULL;
	}

	return OP_OK;
}

void Argument_AddRecord(Argument *arg, Record r) {
	assert(!arg->r && "tried to insert into a populated Argument op");
	arg->r = r;
}

void ArgumentFree(OpBase *opBase) {
	Argument *arg = (Argument *)opBase;
	if(arg->r) {
		OpBase_DeleteRecord(arg->r);
		arg->r = NULL;
	}
}

