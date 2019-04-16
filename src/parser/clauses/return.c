/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "./return.h"
#include "../../util/arr.h"
#include "../../util/rmalloc.h"

AST_ReturnElementNode* New_AST_ReturnElementNode(AST_ArithmeticExpressionNode *exp, char* alias) {
	AST_ReturnElementNode *returnElementNode = (AST_ReturnElementNode*)malloc(sizeof(AST_ReturnElementNode));
	returnElementNode->exp = exp;
	returnElementNode->alias = alias ? strdup(alias) : NULL;
    returnElementNode->asterisks = 0;
	
	return returnElementNode;
}

AST_ReturnElementNode* New_AST_ReturnElementExpandALL() {
    AST_ReturnElementNode *returnElementNode = New_AST_ReturnElementNode(NULL, NULL);
    returnElementNode->asterisks = 1;
	return returnElementNode;
}

AST_ReturnNode* New_AST_ReturnNode(AST_ReturnElementNode **returnElements, int distinct) {
	AST_ReturnNode *returnNode = (AST_ReturnNode*)rm_malloc(sizeof(AST_ReturnNode));
    returnNode->distinct = distinct;
	returnNode->returnElements = returnElements;
    return returnNode;
}

int ReturnClause_ContainsCollapsedNodes(const AST_ReturnNode *returnNode) {
    if(!returnNode) return 0;

    uint elemCount = array_len(returnNode->returnElements);
	for (uint i = 0; i < elemCount; i++) {
		AST_ReturnElementNode *retElem = returnNode->returnElements[i];
        if(retElem->asterisks) return 1;

        AST_ArithmeticExpressionNode *exp = retElem->exp;
        /* Detect collapsed entity,
         * A collapsed entity is represented by an arithmetic expression
         * of AST_AR_EXP_OPERAND type, 
         * The operand type should be AST_AR_EXP_VARIADIC,
         * lastly property should be missing. */
        if(exp &&
            exp->type == AST_AR_EXP_OPERAND &&
            exp->operand.type == AST_AR_EXP_VARIADIC &&
            exp->operand.variadic.property == NULL) {
                return 1;
        }
    }
    return 0;
}

/* Checks if return clause uses aggregation. */
int ReturnClause_ContainsAggregation(const AST_ReturnNode *returnNode) {
    if(!returnNode) return 0;

    uint elemCount = array_len(returnNode->returnElements);
	for (uint i = 0; i < elemCount; i++) {
		AST_ReturnElementNode *retElem = returnNode->returnElements[i];
        AST_ArithmeticExpressionNode *exp = retElem->exp;
        if(AST_AR_EXP_ContainsAggregation(exp)) return 1;
    }

    return 0;
}

void ReturnClause_ReferredEntities(const AST_ReturnNode *returnNode, TrieMap *referred_nodes) {
    if(!returnNode) return;
    
    uint elemCount = array_len(returnNode->returnElements);
    for (uint i = 0; i < elemCount; i++) {
		AST_ReturnElementNode *retElem = returnNode->returnElements[i];
        
        AST_ArithmeticExpressionNode *exp = retElem->exp;
        if(exp) AST_AR_EXP_GetAliases(exp, referred_nodes);
    }
}

void ReturnClause_ReferredFunctions(const AST_ReturnNode *returnNode, TrieMap *referred_funcs) {
    if(!returnNode) return;
    
    uint elemCount = array_len(returnNode->returnElements);
    for (uint i = 0; i < elemCount; i++) {
		AST_ReturnElementNode *retElem = returnNode->returnElements[i];
        
        AST_ArithmeticExpressionNode *exp = retElem->exp;
        if(exp) AST_AR_EXP_GetFunctions(exp, referred_funcs);
    }
}

// Scan through RETURN clause expressions,
// aliased expressions can be referred to by other clauses (ORDER BY)
// these will be added to the definedEntities triemap.
void ReturnClause_DefinedEntities(const AST_ReturnNode *returnNode, TrieMap *definedEntities) {
    if(!returnNode) return;
    
    uint elemCount = array_len(returnNode->returnElements);
    for (uint i = 0; i < elemCount; i++) {
		AST_ReturnElementNode *retElem = returnNode->returnElements[i];
        if(retElem->alias) {
            TrieMap_Add(definedEntities,
                        retElem->alias,
                        strlen(retElem->alias),
                        NULL,
                        TrieMap_DONT_CARE_REPLACE);
        }
    }
}

char** ReturnClause_GetAliases(const AST_ReturnNode *return_node) {
    assert(return_node);
    int return_elem_count = array_len(return_node->returnElements);
    char **aliases = array_new(char*, return_elem_count);

    for(int i = 0; i < return_elem_count; i++) {
        AST_ReturnElementNode *ret_elem = return_node->returnElements[i];
        aliases = array_append(aliases, ret_elem->alias);
    }
    return aliases;
}

void Free_AST_ReturnElementNode(AST_ReturnElementNode *returnElementNode) {
	if(!returnElementNode) return;
    if(returnElementNode->exp) Free_AST_ArithmeticExpressionNode(returnElementNode->exp);
    if(returnElementNode->alias) free(returnElementNode->alias);
    free(returnElementNode);
}

void Free_AST_ReturnNode(AST_ReturnNode *returnNode) {
	if (!returnNode) return;
    
    uint elemCount = array_len(returnNode->returnElements);
	for (uint i = 0; i < elemCount; i++) {
		AST_ReturnElementNode *node = returnNode->returnElements[i];
		Free_AST_ReturnElementNode(node);
	}
	
	array_free(returnNode->returnElements);
	rm_free(returnNode);
}
