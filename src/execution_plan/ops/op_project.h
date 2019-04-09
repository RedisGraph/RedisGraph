/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#ifndef __OP_PROJECT_H
#define __OP_PROJECT_H

#include "op.h"
#include "../../parser/ast.h"
#include "../../arithmetic/arithmetic_expression.h"

typedef struct {
    OpBase op;
    const NEWAST *ast;
    char **aliases;                 // Aliases attached to projected expressions.
    AR_ExpNode **exps;              // Projected expressions.
    AR_ExpNode **order_exps;        // Order by expressions.
    bool singleResponse;            // When no child operations, return NULL after a first response.
    unsigned short exp_count;       // Number of projected expressions.
    unsigned short order_exp_count; // Number of order by expressions.
} OpProject;

OpBase* NewProjectOp(AR_ExpNode **exps, char **aliases);

OpResult ProjectInit(OpBase *opBase);

Record ProjectConsume(OpBase *op);

OpResult ProjectReset(OpBase *ctx);

void ProjectFree(OpBase *ctx);

#endif
