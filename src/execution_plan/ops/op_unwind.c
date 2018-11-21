/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include <assert.h>
#include "op_unwind.h"
#include "../../arithmetic/arithmetic_expression.h"
#include "../../util/arr.h"

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

OpResult UnwindConsume(OpBase *opBase, Record *r) {
    OpUnwind *unwind = (OpUnwind*)opBase;

    // Init
    if(unwind->expressions == NULL) {
        uint expCount = Vector_Size(unwind->unwindClause->expressions);
        unwind->expressions = array_new(AR_ExpNode*, expCount);

        for(int i = 0; i < expCount; i++) {
            AST_ArithmeticExpressionNode *exp;
            Vector_Get(unwind->unwindClause->expressions, i, &exp);
            unwind->expressions = array_append(unwind->expressions, AR_EXP_BuildFromAST(exp));
        }
    }

    if(unwind->expIdx == array_len(unwind->expressions)) {
        return OP_DEPLETED;
    }

    AR_ExpNode *exp = unwind->expressions[unwind->expIdx];
    SIValue v = AR_EXP_Evaluate(exp, *r);
    Record_AddEntry(r, unwind->unwindClause->alias, v);

    unwind->expIdx++;
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
