/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "op_argument.h"

// Forward declarations
Record ArgumentConsume(OpBase *opBase);
OpResult ArgumentReset(OpBase *opBase);
void ArgumentFree(OpBase *opBase);

OpBase *NewArgumentOp(const ExecutionPlan *plan, const char **variables) {
	Argument *op = malloc(sizeof(Argument));
	op->r = NULL;

	// Set our Op operations
	OpBase_Init((OpBase *)op, OPType_ARGUMENT, "Argument", NULL,
				ArgumentConsume, ArgumentReset, NULL, ArgumentFree, plan);

	uint variable_count = array_len(variables);
	for(uint i = 0; i < variable_count; i ++) {
		// TODO I think it's unnecessary to track record offsets here; validate assumption.
		OpBase_Modifies((OpBase *)op, variables[i]);
	}

	return (OpBase *)op;
}

Record ArgumentConsume(OpBase *opBase) {
	Argument *arg = (Argument *)opBase;
	Record r = NULL;

	/* If record is set, return it only once! */
	if(arg->r) {
		r = arg->r;
		arg->r = NULL;
	}

	return r;
}

OpResult ArgumentReset(OpBase *opBase) {
	/* Reset operation
	 * free record if set, this brings us back to our initial state. */
	Argument *arg = (Argument *)opBase;
	if(arg->r) {
		Record_Free(arg->r);
		arg->r = NULL;
	}

	return OP_OK;
}

void ArgumentSetRecord(Argument *arg, Record r) {
	/* Set operation's record, expecting:
	 * 1. Operation's record to be NULL.
	 * 2. Given record != NULL. */
	assert(arg->r == NULL && r != NULL);
	arg->r = r;
}

void ArgumentFree(OpBase *opBase) {
	Argument *arg = (Argument *)opBase;
	if(arg->r) {
		Record_Free(arg->r);
		arg->r = NULL;
	}
}

