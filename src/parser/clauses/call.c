/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "./call.h"
#include "../../util/arr.h"
#include "../../util/rmalloc.h"

AST_ProcedureCallNode* New_AST_ProcedureCallNode(char *procedure, char **arguments, char **yield) {
    AST_ProcedureCallNode *procCall = rm_malloc(sizeof(AST_ProcedureCallNode));
    procCall->procedure = procedure;
    procCall->arguments = arguments;
    procCall->yield = yield;
    return procCall;
}

void ProcedureCallClause_ReferredEntities(const AST_ProcedureCallNode *node, TrieMap *referred_entities) {
    // Procedure call doesn't refers to any entities.
    return;
}

void ProcedureCallClause_DefinedEntities(const AST_ProcedureCallNode *node, TrieMap *definedEntities) {
    if(!node) return;
    for(int i = 0; i < array_len(node->yield); i++) {
        TrieMap_Add(definedEntities,
                    node->yield[i],
                    strlen(node->yield[i]),
                    NULL,
                    TrieMap_DONT_CARE_REPLACE);    
    }
}

void Free_AST_ProcedureCallNode(AST_ProcedureCallNode *node) {
    if(!node) return;

    rm_free(node->procedure);

    if(node->arguments) {
        for(int i = 0; i < array_len(node->arguments); i++) rm_free(node->arguments[i]);        
        array_free(node->arguments);
    }

    if(node->yield) {
        for(int i = 0; i < array_len(node->yield); i++) rm_free(node->yield[i]);
        array_free(node->yield);
    }

    rm_free(node);
}
