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
	op->records = array_new(Record, 1);

	// Set our Op operations
	OpBase_Init((OpBase *)op, OPType_ARGUMENT, "Argument", NULL,
				ArgumentConsume, ArgumentReset, NULL, NULL, ArgumentFree, false, plan);

	uint variable_count = array_len(variables);
	for(uint i = 0; i < variable_count; i ++) {
		OpBase_Modifies((OpBase *)op, variables[i]);
	}

	return (OpBase *)op;
}

Record ArgumentConsume(OpBase *opBase) {
	Argument *arg = (Argument *)opBase;

	// Return NULL if the Records array has been depleted.
	if(array_len(arg->records) == 0) return NULL;

	// Return a single Record from the held array.
	return array_pop(arg->records);
}

OpResult ArgumentReset(OpBase *opBase) {
	// Reset operation, freeing any held Records.
	Argument *arg = (Argument *)opBase;

	// I don't believe there is any situation in which this loop should trigger,
	// but keeping it for safety.
	uint record_count = array_len(arg->records);
	for(uint i = 0; i < record_count; i ++) OpBase_DeleteRecord(arg->records[i]);
	array_clear(arg->records);

	return OP_OK;
}

void Argument_AddRecord(Argument *arg, Record r) {
	arg->records = array_append(arg->records, r);
}

void ArgumentFree(OpBase *opBase) {
	Argument *arg = (Argument *)opBase;
	if(arg->records) {
		uint record_count = array_len(arg->records);
		for(uint i = 0; i < record_count; i ++) OpBase_DeleteRecord(arg->records[i]);
		array_free(arg->records);
		arg->records = NULL;
	}
}

