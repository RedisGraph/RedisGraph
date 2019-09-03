/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#ifndef __OP_PROJECT_H
#define __OP_PROJECT_H

#include "op.h"
#include "../execution_plan.h"
#include "../../arithmetic/arithmetic_expression.h"

typedef struct {
	OpBase op;
	const AST *ast;
	AR_ExpNode **exps;              // Projected expressions.
	AR_ExpNode **order_exps;        // Order by expressions.
	bool singleResponse;            // When no child operations, return NULL after a first response.
	unsigned short exp_count;       // Number of projected expressions.
	unsigned short order_exp_count; // Number of order by expressions.
} OpProject;

OpBase *NewProjectOp(const ExecutionPlan *plan, AR_ExpNode **exps);

OpResult ProjectInit(OpBase *opBase);

Record ProjectConsume(OpBase *op);

OpResult ProjectReset(OpBase *ctx);

void ProjectFree(OpBase *ctx);

#endif
