/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include "unwind.h"
#include "../ast_arithmetic_expression.h"

AST_UnwindNode* New_AST_UnwindNode(Vector *expressions, char *alias) {
    AST_UnwindNode *unwindNode = malloc(sizeof(AST_UnwindNode));
    unwindNode->expressions = expressions;
    unwindNode->alias = alias;
    return unwindNode;
}

void UnwindClause_ReferredEntities(const AST_UnwindNode *unwindNode, TrieMap *referredEntities) {
    if(!unwindNode) return;

    int expression_count = Vector_Size(unwindNode->expressions);

    for(int i = 0; i < expression_count; i++) {
        AST_ArithmeticExpressionNode *exp;
        Vector_Get(unwindNode->expressions, i, &exp);
        AR_EXP_GetAliases(exp, referredEntities);
    }
}

void UnwindClause_DefinedEntities(const AST_UnwindNode *unwindNode, TrieMap *definedEntities) {
    if(!unwindNode) return;
    TrieMap_Add(definedEntities,
                unwindNode->alias,
                strlen(unwindNode->alias),
                NULL,
                TrieMap_DONT_CARE_REPLACE);
}

void Free_AST_UnwindNode(AST_UnwindNode *unwindNode) {
    if(!unwindNode) return;
    if(unwindNode->expressions) {
        AST_ArithmeticExpressionNode *expNode;
        while(Vector_Pop(unwindNode->expressions, &expNode)) {
            Free_AST_ArithmeticExpressionNode(expNode);
        }
        Vector_Free(unwindNode->expressions);
    }    
    free(unwindNode);
}
