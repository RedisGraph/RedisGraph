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

static void _InitializeUpdates(OpMerge *op, rax *updates) {
	raxIterator it;
	// if we have ON MATCH / ON CREATE directives, set the appropriate record IDs of entities to be updated
	raxStart(&it, updates);
	raxSeek(&it, "^", NULL, 0);
	// iterate over all expressions
	while(raxNext(&it)) {
		EntityUpdateEvalCtx *ctx = it.data;
		// set the record index for every entity modified by this operation
		ctx->record_idx = OpBase_Modifies((OpBase *)op, ctx->alias);
	}

	raxStop(&it);
}

OpBase *NewMergeOp(const ExecutionPlan *plan, rax *on_match, rax *on_create) {

	/* merge is an operator with two or three children
	 * they will be created outside of here,
	 * as with other multi-stream operators (see CartesianProduct and ValueHashJoin) */
	OpMerge *op = rm_calloc(1, sizeof(OpMerge));
	op->on_match         =  on_match;
	op->on_create        =  on_create;
	// set our Op operations
	OpBase_Init((OpBase *)op, OPType_MERGE, "Merge", NULL, NULL, true, plan);

	if(op->on_match) _InitializeUpdates(op, op->on_match);
	if(op->on_create) _InitializeUpdates(op, op->on_create);

	return (OpBase *)op;
}
