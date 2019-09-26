/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "op_argument.h"

OpBase *NewArgumentOp(void) {
	Argument *arg = malloc(sizeof(Argument));
	arg->r = NULL;

	// Set our Op operations
	OpBase_Init(&arg->op);
	arg->op.name = "Argument";
	arg->op.type = OPType_ARGUMENT;
	arg->op.consume = ArgumentConsume;
	arg->op.reset = ArgumentReset;
	arg->op.free = ArgumentFree;

	return (OpBase *)arg;
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
