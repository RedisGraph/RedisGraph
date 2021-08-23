/*
 * Copyright 2018-2021 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "op_optional.h"

OpBase *NewOptionalOp(const ExecutionPlan *plan) {
	Optional *op = rm_malloc(sizeof(Optional));

	// Set our Op operations
	OpBase_Init((OpBase *)op, OPType_OPTIONAL, "Optional", NULL, NULL, false,
		plan);

	return (OpBase *)op;
}
