/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include <assert.h>
#include "op_unwind.h"
#include "../../util/arr.h"
#include "../../arithmetic/arithmetic_expression.h"

OpBase* NewUnwindOp(NEWAST *ast, const cypher_astnode_t *clause) {
    OpUnwind *unwind = malloc(sizeof(OpUnwind));
    unwind->ast = ast;
    unwind->expIdx = 0;
    unwind->expressions = NULL;

    // Set our Op operations
    OpBase_Init(&unwind->op);
    unwind->op.name = "Unwind";
    unwind->op.type = OPType_UNWIND;
    unwind->op.consume = UnwindConsume;
    unwind->op.init = UnwindInit;
    unwind->op.reset = UnwindReset;
    unwind->op.free = UnwindFree;

    unwind->unwindClause = clause;

    // Handle alias
    const cypher_astnode_t *alias_node = cypher_ast_unwind_get_alias(clause);
    const char *alias = cypher_ast_identifier_get_name(alias_node);
    unwind->op.modifies = NewVector(char*, 1);
    Vector_Push(unwind->op.modifies, (char*)alias);
    unwind->unwindRecIdx = NEWAST_GetAliasID(ast, (char*)alias); // TODO moved from Init, move back if necessary

    return (OpBase*)unwind;
}

// TODO some un
OpResult UnwindInit(OpBase *opBase) {
    OpUnwind *op = (OpUnwind*)opBase;
    const cypher_astnode_t *exprs = cypher_ast_unwind_get_expression(op->unwindClause);
    assert(cypher_astnode_type(exprs) == CYPHER_AST_COLLECTION);
    uint expCount = cypher_ast_collection_length(exprs);
    op->expressions = array_new(AR_ExpNode*, expCount);

    for(uint i = 0; i < expCount; i ++) {
        const cypher_astnode_t *exp_node = cypher_ast_collection_get(exprs, i);
        AR_ExpNode *exp = AR_EXP_FromExpression(op->ast, exp_node);
        op->expressions = array_append(op->expressions, exp);
    }
    return OP_OK;
}

Record UnwindConsume(OpBase *opBase) {
    OpUnwind *op = (OpUnwind*)opBase;

    // Evaluated and returned all expressions.
    if(op->expIdx == array_len(op->expressions)) return NULL;

    AR_ExpNode *exp = op->expressions[op->expIdx];
    Record r = Record_New(NEWAST_AliasCount(op->ast));
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
