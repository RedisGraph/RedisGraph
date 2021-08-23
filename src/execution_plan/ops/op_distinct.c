/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "op_distinct.h"
#include "op_project.h"
#include "op_aggregate.h"
#include "xxhash.h"
#include "../../util/arr.h"
#include "../execution_plan_build/execution_plan_modify.h"

/* Forward declarations. */
static void DistinctFree(OpBase *opBase);

OpBase *NewDistinctOp(const ExecutionPlan *plan, const char **aliases, uint alias_count) {
	ASSERT(aliases != NULL);
	ASSERT(alias_count > 0);

	OpDistinct *op = rm_malloc(sizeof(OpDistinct));

	op->aliases         =  rm_malloc(alias_count * sizeof(const char *));

	// Copy aliases into heap array managed by this op
	memcpy(op->aliases, aliases, alias_count * sizeof(const char *));

	OpBase_Init((OpBase *)op, OPType_DISTINCT, "Distinct", NULL, DistinctFree, 
		false, plan);

	return (OpBase *)op;
}

static void DistinctFree(OpBase *ctx) {
	OpDistinct *op = (OpDistinct *)ctx;
	if(op->aliases) {
		rm_free(op->aliases);
		op->aliases = NULL;
	}
}
