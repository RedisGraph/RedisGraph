/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#ifndef __OP_PROJECT_H
#define __OP_PROJECT_H

#include "op.h"
#include "../../parser/ast.h"
#include "../../resultset/resultset.h"
#include "../../util/triemap/triemap.h"
#include "../../arithmetic/arithmetic_expression.h"

typedef struct {
    OpBase op;
    AST *ast;
    ResultSet *resultset;
    uint projectedRecordLen;    // Length of projected record.
    AR_ExpNode **expressions;   // Array of expressions to evaluate.
    bool singleResponse;        // When no child operations, return NULL after a first response.
} Project;

OpBase* NewProjectOp(ResultSet *resultset);

Record ProjectConsume(OpBase *op);

OpResult ProjectReset(OpBase *ctx);

void ProjectFree(OpBase *ctx);

#endif
