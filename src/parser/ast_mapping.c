/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "ast.h"
#include <assert.h>

#include "../../deps/xxhash/xxhash.h"
#include "../util/arr.h"
#include "../arithmetic/repository.h"
#include "../arithmetic/arithmetic_expression.h"

unsigned int AST_GetAliasID(const AST *ast, char *alias) {
    AR_ExpNode *exp = TrieMap_Find(ast->entity_map, alias, strlen(alias));
    return exp->record_idx;
}

void AST_MapEntityHash(const AST *ast, AST_IDENTIFIER identifier, AR_ExpNode *exp) {
    TrieMap_Add(ast->entity_map, (char*)&identifier, sizeof(identifier), exp, TrieMap_DONT_CARE_REPLACE);
}

void AST_MapAlias(const AST *ast, char *alias, AR_ExpNode *exp) {
    TrieMap_Add(ast->entity_map, alias, strlen(alias), exp, TrieMap_DONT_CARE_REPLACE);
}

unsigned int AST_GetEntityID(const AST *ast, const cypher_astnode_t *entity) {
    AST_IDENTIFIER identifier = AST_EntityHash(entity);
    AR_ExpNode *v = TrieMap_Find(ast->entity_map, (char*)&identifier, sizeof(identifier));
    assert(v != TRIEMAP_NOTFOUND);
    return v->operand.variadic.entity_alias_idx;
}


AR_ExpNode* AST_GetEntity(const AST *ast, const cypher_astnode_t *entity) {
    AST_IDENTIFIER identifier = AST_EntityHash(entity);
    AR_ExpNode *v = TrieMap_Find(ast->entity_map, (char*)&identifier, sizeof(identifier));
    assert(v != TRIEMAP_NOTFOUND);
    return v;
}

AR_ExpNode* AST_GetEntityFromAlias(const AST *ast, char *alias) {
    void *v = TrieMap_Find(ast->entity_map, alias, strlen(alias));
    if (v == TRIEMAP_NOTFOUND) return NULL;
    return v;
}

AR_ExpNode* AST_GetEntityFromHash(const AST *ast, AST_IDENTIFIER id) {
    void *v = TrieMap_Find(ast->entity_map, (char*)&id, sizeof(id));
    if (v == TRIEMAP_NOTFOUND) return NULL;
    return v;
}

AST_IDENTIFIER AST_EntityHash(const cypher_astnode_t *entity) {
    // TODO can we just use pointers instead of hashed pointers?
    return XXH64(&entity, sizeof(entity), 0);
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
    
    // TODO which function?
    AR_ExpNode *exp = calloc(1, sizeof(AR_ExpNode));
    exp->type = AR_EXP_OPERAND;
    exp->record_idx = id;
    exp->operand.type = AR_EXP_VARIADIC;
    exp->operand.variadic.entity_alias_idx = id;
    ast->defined_entities = array_append(ast->defined_entities, exp);

    return id;
}
