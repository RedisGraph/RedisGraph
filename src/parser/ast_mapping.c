/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "ast.h"
#include "../util/arr.h"
#include "../arithmetic/arithmetic_expression.h"
#include <assert.h>

unsigned int AST_GetAliasID(const AST *ast, char *alias) {
    AR_ExpNode *exp = TrieMap_Find(ast->entity_map, alias, strlen(alias));
    return exp->record_idx;
}

void AST_MapEntity(const AST *ast, AST_IDENTIFIER identifier, AR_ExpNode *exp) {
    TrieMap_Add(ast->entity_map, (char*)&identifier, sizeof(identifier), exp, TrieMap_DONT_CARE_REPLACE);
}

void AST_MapAlias(const AST *ast, char *alias, AR_ExpNode *exp) {
    TrieMap_Add(ast->entity_map, alias, strlen(alias), exp, TrieMap_DONT_CARE_REPLACE);
}

AR_ExpNode* AST_GetEntity(const AST *ast, AST_IDENTIFIER entity) {
    AR_ExpNode *v = TrieMap_Find(ast->entity_map, (char*)&entity, sizeof(entity));
    if (v == TRIEMAP_NOTFOUND) return NULL;
    return v;
}

AR_ExpNode* AST_GetEntityFromAlias(const AST *ast, char *alias) {
    void *v = TrieMap_Find(ast->entity_map, alias, strlen(alias));
    if (v == TRIEMAP_NOTFOUND) return NULL;
    return v;
}

unsigned int AST_GetEntityRecordIdx(const AST *ast, const cypher_astnode_t *entity) {
    AR_ExpNode *exp = AST_GetEntity(ast, entity);
    return exp->record_idx;
}

unsigned int AST_RecordLength(const AST *ast) {
    return ast->record_length;
}

unsigned int AST_AddRecordEntry(AST *ast) {
    // Increment Record length and return a valid record index
    return ast->record_length ++;
}

void AST_RecordAccommodateExpression(AST *ast, AR_ExpNode *exp) {
    // Do nothing if expression already has a Record index
    if (exp->record_idx != NOT_IN_RECORD) return;

    // Register a new Record index
    exp->record_idx = ast->record_length ++;

    // Add entity to the set of entities to be populated
    ast->defined_entities = array_append(ast->defined_entities, exp);
}

unsigned int AST_AddAnonymousRecordEntry(AST *ast) {
    uint id = ast->record_length ++;
    
    AR_ExpNode *exp = AR_EXP_NewAnonymousEntity(id);
    ast->defined_entities = array_append(ast->defined_entities, exp);

    return id;
}
