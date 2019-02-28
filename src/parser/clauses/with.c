/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include "with.h"
#include "../../util/arr.h"
#include "../../util/rmalloc.h"

static void _free_AST_WithElementNode(AST_WithElementNode *withElementNode) {
    Free_AST_ArithmeticExpressionNode(withElementNode->exp);
    free(withElementNode->alias);
    rm_free(withElementNode);
}

AST_WithNode* New_AST_WithNode(AST_WithElementNode **exps) {
    AST_WithNode *node = rm_malloc(sizeof(AST_WithNode));
    node->exps = exps;
    return node;
}

AST_WithElementNode* New_AST_WithElementNode(AST_ArithmeticExpressionNode *exp, char* alias) {
    AST_WithElementNode *withElementNode = rm_malloc(sizeof(AST_WithElementNode));
    withElementNode->alias = alias;
    withElementNode->exp = exp;
    return withElementNode;
}

void WithClause_ReferredEntities(const AST_WithNode *withNode, TrieMap *referred_nodes) {
    if(!withNode) return;
    
    uint elemCount = array_len(withNode->exps);
    for (uint i = 0; i < elemCount; i++) {
		AST_WithElementNode *elem = withNode->exps[i];
        
        AST_ArithmeticExpressionNode *exp = elem->exp;
        if(exp) AST_AR_EXP_GetAliases(exp, referred_nodes);
    }
}

void WithClause_DefinedEntities(const AST_WithNode *withNode, TrieMap *definedEntities) {
    if(!withNode) return;

    uint elemCount = array_len(withNode->exps);
    for (uint i = 0; i < elemCount; i++) {
		AST_WithElementNode *elem = withNode->exps[i];
        TrieMap_Add(definedEntities, elem->alias, strlen(elem->alias), NULL, TrieMap_DONT_CARE_REPLACE);
    }
}

char** WithClause_GetAliases(const AST_WithNode *withNode) {
    if(!withNode) return NULL;

    uint elemCount = array_len(withNode->exps);
    char **aliases = array_new(char*, elemCount);

    for (uint i = 0; i < elemCount; i++) {
		AST_WithElementNode *elem = withNode->exps[i];
        aliases = array_append(aliases, elem->alias);
    }

    return aliases;
}

/* Checks if return clause uses aggregation. */
int WithClause_ContainsAggregation(const AST_WithNode *withNode) {
    if(!withNode) return 0;

    uint elemCount = array_len(withNode->exps);
	for (uint i = 0; i < elemCount; i++) {
		AST_WithElementNode *elem = withNode->exps[i];
        AST_ArithmeticExpressionNode *exp = elem->exp;
        if(AST_AR_EXP_ContainsAggregation(exp)) return 1;
    }

    return 0;
}

void Free_AST_WithNode(AST_WithNode *withNode) {
    if(!withNode) return;

    int exp_count = array_len(withNode->exps);
    for(int i = 0; i < exp_count; i++) {
        AST_WithElementNode *exp = withNode->exps[i];
        _free_AST_WithElementNode(exp);
    }
    array_free(withNode->exps);
    rm_free(withNode);
}
