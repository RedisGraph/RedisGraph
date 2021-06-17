/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "../execution_plan.h"
#include "../ops/op_limit.h"
#include "../../arithmetic/arithmetic_expression_construct.h"

OpBase *buildLimitOp(ExecutionPlan *plan, const cypher_astnode_t *limit_clause) {
	// build limit expression
	AR_ExpNode *exp = AR_EXP_FromASTNode(limit_clause);
	OpBase *op = NewLimitOp(plan, exp);
	return op;
}

