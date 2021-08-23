/*
 * Copyright 2018-2021 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "RG.h"
#include "op_argument.h"

OpBase *NewArgumentOp(const ExecutionPlan *plan, const char **variables) {
	Argument *op = rm_malloc(sizeof(Argument));

	// Set our Op operations
	OpBase_Init((OpBase *)op, OPType_ARGUMENT, "Argument", NULL, NULL, false, plan);

	return (OpBase *)op;
}
