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
	op->records = array_new(Record, 1);

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

	assert(arg->records); // TODO tmp

	if(array_len(arg->records) == 0) return NULL;

	return array_pop(arg->records);
}

OpResult ArgumentReset(OpBase *opBase) {
	/* Reset operation
	 * free record if set, this brings us back to our initial state. */
	Argument *arg = (Argument *)opBase;

	/*
	uint record_count = array_len(arg->records);
	for(uint i = 0; i < record_count; i ++) {
		Record_Free(arg->records[i]); // TODO use if cloning is moved here,
	}
	*/

	array_free(arg->records);
	arg->records = array_new(Record, 1);

	return OP_OK;
}

void Argument_AddRecord(Argument *arg, Record r) {
	arg->records = array_append(arg->records, r);
}

void ArgumentFree(OpBase *opBase) {
	Argument *arg = (Argument *)opBase;
	if(arg->records) {
		/*
		uint record_count = array_len(arg->records);
		for(uint i = 0; i < record_count; i ++) {
			Record_Free(arg->records[i]); // TODO use if cloning is moved here,
		}
		*/
		array_free(arg->records);
		arg->records = NULL;
	}
}

