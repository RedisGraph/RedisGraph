/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include <assert.h>
#include "op_unwind.h"
#include "../../util/arr.h"
#include "../../parser/ast.h"
#include "../../arithmetic/arithmetic_expression.h"

OpBase* NewUnwindOp(AST *ast) {
    OpUnwind *unwind = malloc(sizeof(OpUnwind));
    unwind->ast = ast;
    unwind->expIdx = 0;
    unwind->expressions = NULL;    
    unwind->unwindClause = ast->unwindNode;

    // Set our Op operations
    OpBase_Init(&unwind->op);
    unwind->op.name = "Unwind";
    unwind->op.type = OPType_UNWIND;
    unwind->op.consume = UnwindConsume;
    unwind->op.init = UnwindInit;
    unwind->op.reset = UnwindReset;
    unwind->op.free = UnwindFree;
    unwind->op.modifies = NewVector(char*, 1);
    Vector_Push(unwind->op.modifies, ast->unwindNode->alias);

    return (OpBase*)unwind;
}

OpResult UnwindInit(OpBase *opBase) {
    OpUnwind *op = (OpUnwind*)opBase;
    AST *ast = op->ast;
    uint expCount = Vector_Size(op->unwindClause->expressions);
    op->expressions = array_new(AR_ExpNode*, expCount);

    for(uint i = 0; i < expCount; i++) {
        AST_ArithmeticExpressionNode *exp;
        Vector_Get(op->unwindClause->expressions, i, &exp);
        op->expressions = array_append(op->expressions, AR_EXP_BuildFromAST(ast, exp));
    }
    op->unwindRecIdx = AST_GetAliasID(ast, op->unwindClause->alias);
    return OP_OK;
}

Record UnwindConsume(OpBase *opBase) {
    OpUnwind *op = (OpUnwind*)opBase;
    AST *ast = op->ast;

    // Evaluated and returned all expressions.
    if(op->expIdx == array_len(op->expressions)) return NULL;

    AR_ExpNode *exp = op->expressions[op->expIdx];
    Record r = Record_New(AST_AliasCount(ast));
    SIValue v = AR_EXP_Evaluate(exp, r);
    Record_AddScalar(r, op->unwindRecIdx, v);
    op->expIdx++;

    return r;
}

OpResult UnwindReset(OpBase *ctx) {
    OpUnwind *unwind = (OpUnwind*)ctx;
    unwind->expIdx = 0;
    return OP_OK;
}

void UnwindFree(OpBase *ctx) {
    OpUnwind *unwind = (OpUnwind*)ctx;
    
    AR_ExpNode *exp;
    if(unwind->expressions) {
        uint expCount = array_len(unwind->expressions);
        for(uint i = 0; i < expCount; i++) AR_EXP_Free(unwind->expressions[i]);
        array_free(unwind->expressions);
    }
}
