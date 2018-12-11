/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include <assert.h>
#include "op_unwind.h"
#include "../../util/arr.h"
#include "../../parser/ast.h"
#include "../../arithmetic/arithmetic_expression.h"

OpBase* NewUnwindOp(AST_UnwindNode *unwindClause) {
    OpUnwind *unwind = malloc(sizeof(OpUnwind));
    unwind->unwindClause = unwindClause;
    unwind->expressions = NULL;    
    unwind->expIdx = 0;

    // Set our Op operations
    OpBase_Init(&unwind->op);
    unwind->op.name = "Unwind";
    unwind->op.type = OPType_UNWIND;
    unwind->op.consume = UnwindConsume;
    unwind->op.reset = UnwindReset;
    unwind->op.free = UnwindFree;
    unwind->op.modifies = NewVector(char*, 1);
    Vector_Push(unwind->op.modifies, unwindClause->alias);

    return (OpBase*)unwind;
}

OpResult UnwindConsume(OpBase *opBase, Record r) {
    OpUnwind *op = (OpUnwind*)opBase;
    
    // Init
    if(op->expressions == NULL) {
        AST *ast = AST_GetFromLTS();
        uint expCount = Vector_Size(op->unwindClause->expressions);
        op->expressions = array_new(AR_ExpNode*, expCount);

        for(int i = 0; i < expCount; i++) {
            AST_ArithmeticExpressionNode *exp;
            Vector_Get(op->unwindClause->expressions, i, &exp);
            op->expressions = array_append(op->expressions, AR_EXP_BuildFromAST(ast, exp));
        }

        op->unwindRecIdx = AST_GetAliasID(ast, op->unwindClause->alias);
    }

    if(op->expIdx == array_len(op->expressions)) {
        return OP_DEPLETED;
    }

    AR_ExpNode *exp = op->expressions[op->expIdx];
    SIValue v = AR_EXP_Evaluate(exp, r);

    Record_AddScalar(r, op->unwindRecIdx, v);

    op->expIdx++;
    return OP_OK;
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
        for(int i = 0; i < expCount; i++) AR_EXP_Free(unwind->expressions[i]);
        array_free(unwind->expressions);
    }
}
