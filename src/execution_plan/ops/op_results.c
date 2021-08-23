/*
 * Copyright 2018-2021 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "op_results.h"
#include "RG.h"
#include "../../util/arr.h"
#include "../../query_ctx.h"
#include "../../configuration/config.h"
#include "../../arithmetic/arithmetic_expression.h"

OpBase *NewResultsOp(const ExecutionPlan *plan) {
	Results *op = rm_malloc(sizeof(Results));

	// Set our Op operations
	OpBase_Init((OpBase *)op, OPType_RESULTS, "Results", NULL, NULL, false,
		plan);

	return (OpBase *)op;
}
