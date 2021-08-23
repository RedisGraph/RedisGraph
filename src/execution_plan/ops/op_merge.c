/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "RG.h"
#include "op_merge.h"
#include "../../errors.h"
#include "op_merge_create.h"
#include "../../query_ctx.h"
#include "../../schema/schema.h"
#include "../../util/thpool/pools.h"
#include "../../util/rax_extensions.h"
#include "../../arithmetic/arithmetic_expression.h"

OpBase *NewMergeOp(const ExecutionPlan *plan, rax *on_match, rax *on_create) {

	/* merge is an operator with two or three children
	 * they will be created outside of here,
	 * as with other multi-stream operators (see CartesianProduct and ValueHashJoin) */
	OpMerge *op = rm_calloc(1, sizeof(OpMerge));
	op->on_match         =  on_match;
	op->on_create        =  on_create;
	// set our Op operations
	OpBase_Init((OpBase *)op, OPType_MERGE, "Merge", NULL, NULL, true,
		plan);

	return (OpBase *)op;
}
