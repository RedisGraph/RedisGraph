/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "op_value_hash_join.h"
#include "../../value.h"
#include "../../util/arr.h"
#include "../../util/rmalloc.h"
#include "../../util/qsort.h"

/* Forward declarations. */
static void ValueHashJoinFree(OpBase *opBase);

/* Creates a new valueHashJoin operation */
OpBase *NewValueHashJoin(const ExecutionPlan *plan, AR_ExpNode *lhs_exp, AR_ExpNode *rhs_exp) {
	OpValueHashJoin *op = rm_malloc(sizeof(OpValueHashJoin));
	op->lhs_exp = lhs_exp;
	op->rhs_exp = rhs_exp;

	// Set our Op operations
	OpBase_Init((OpBase *)op, OPType_VALUE_HASH_JOIN, "Value Hash Join",
		ValueHashJoinFree, false, plan);

	OpBase_Modifies((OpBase *)op, "pivot");

	return (OpBase *)op;
}

/* Frees ValueHashJoin */
static void ValueHashJoinFree(OpBase *ctx) {
	OpValueHashJoin *op = (OpValueHashJoin *)ctx;
	if(op->lhs_exp) {
		AR_EXP_Free(op->lhs_exp);
		op->lhs_exp = NULL;
	}

	if(op->rhs_exp) {
		AR_EXP_Free(op->rhs_exp);
		op->rhs_exp = NULL;
	}
}
