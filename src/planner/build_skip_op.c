/*
 * Copyright 2018-2022 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "runtime/ops/op_skip.h"
#include "IR/execution_plan/execution_plan.h"
#include "IR/arithmetic_expression/arithmetic_expression_construct.h"

OpBase *buildSkipOp(ExecutionPlan *plan, const cypher_astnode_t *skip_clause) {
	// build skip expression
	AR_ExpNode *exp = AR_EXP_FromASTNode(skip_clause);
	OpBase *op = NewSkipOp(plan, exp);
	return op;
}

