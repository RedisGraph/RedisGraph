/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include "op_project.h"
#include "../../util/arr.h"

static void _buildExpressions(Project *op) {
    // Compute projected record length:
    // Number of returned expressions + number of order-by expressions.
    const AST *ast = op->ast;
    uint orderByExpCount = 0;
    uint returnExpCount = array_len(ast->returnNode->returnElements);
    if(ast->orderNode) orderByExpCount = array_len(ast->orderNode->expressions);
    uint expressionCount = returnExpCount + orderByExpCount;
    
    op->projectedRecordLen = expressionCount;
    op->expressions = array_new(AR_ExpNode*, expressionCount);

    // Compose RETURN clause expressions.
    for(uint i = 0; i < returnExpCount; i++) {
        AST_ArithmeticExpressionNode *exp = ast->returnNode->returnElements[i]->exp;
        op->expressions = array_append(op->expressions, AR_EXP_BuildFromAST(ast, exp));
    }

    // Compose ORDER BY expressions.
    for(uint i = 0; i < orderByExpCount; i++) {
        AST_ArithmeticExpressionNode *exp = ast->orderNode->expressions[i];
        op->expressions = array_append(op->expressions, AR_EXP_BuildFromAST(ast, exp));
    }
}

OpBase* NewProjectOp(AST *ast) {
    Project *project = malloc(sizeof(Project));
    project->ast = ast;
    project->singleResponse = false;    

    _buildExpressions(project);

    // Set our Op operations
    OpBase_Init(&project->op);
    project->op.name = "Project";
    project->op.type = OPType_PROJECT;
    project->op.consume = ProjectConsume;
    project->op.reset = ProjectReset;
    project->op.free = ProjectFree;

    return (OpBase*)project;
    return NULL;
}

Record ProjectConsume(OpBase *opBase) {
    Project *op = (Project*)opBase;
    Record r = NULL;

    if(op->op.childCount) {
        OpBase *child = op->op.children[0];
        r = child->consume(child);
        if(!r) return NULL;
    } else {
        // QUERY: RETURN 1+2
        // Return a single record followed by NULL
        // on the second call.
        if(op->singleResponse) return NULL;
        op->singleResponse = true;
    }

    Record projectedRec = Record_New(op->projectedRecordLen);

    uint expIdx = 0;
    uint expCount = array_len(op->expressions);
    uint returnExpCount = array_len(op->ast->returnNode->returnElements);

    // Evaluate RETURN clause expressions.
    for(; expIdx < returnExpCount; expIdx++) {
        SIValue v = AR_EXP_Evaluate(op->expressions[expIdx], r);
        Record_AddScalar(projectedRec, expIdx, v);

        // Incase expression is aliased, add it to record
        // as it might be referenced by other expressions:
        // e.g. RETURN n.v AS X ORDER BY X * X
        char *alias = op->ast->returnNode->returnElements[expIdx]->alias;
        if(alias) Record_AddScalar(r, AST_GetAliasID(op->ast, alias), v);
    }

    // Evaluate ORDER BY clause expressions.
    for(; expIdx < expCount; expIdx++) {
        SIValue v = AR_EXP_Evaluate(op->expressions[expIdx], r);
        Record_AddScalar(projectedRec, expIdx, v);
    }

    Record_Free(r);
    return projectedRec;
}

OpResult ProjectReset(OpBase *ctx) {
    return OP_OK;
}

void ProjectFree(OpBase *opBase) {
    Project *op = (Project*)opBase;
    uint expCount = array_len(op->expressions);
    for(uint i = 0; i < expCount; i++) AR_EXP_Free(op->expressions[i]);
    array_free(op->expressions);
}
