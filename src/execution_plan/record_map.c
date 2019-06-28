/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "record_map.h"
#include "../util/rmalloc.h"
#include <assert.h>

static uint* _BuildMapValue(uint id) {
    // TODO so many unnecessary allocs
    uint *id_ptr = rm_malloc(sizeof(uint));
    *id_ptr = id;
    return id_ptr;
}

uint RecordMap_GetRecordIDFromReference(RecordMap *record_map, AST_IDENTIFIER entity) {
    uint *id = TrieMap_Find(record_map->map, (char*)&entity, sizeof(entity));
    if (id == TRIEMAP_NOTFOUND) return IDENTIFIER_NOT_FOUND;
    return *id;
}

// uint RecordMap_ReferenceToRecordID(RecordMap *record_map, AST_IDENTIFIER identifier) {
    // uint *id_ptr = TrieMap_Find(record_map->map, (char*)&identifier, sizeof(identifier));
    // if (id_ptr != TRIEMAP_NOTFOUND) return *id_ptr;

    // uint id = record_map->record_len++;
    // id_ptr = _BuildMapValue(id);
    // TrieMap_Add(record_map->map, (char*)&identifier, sizeof(identifier), id_ptr, TrieMap_DONT_CARE_REPLACE);

    // return *id_ptr;
// }

// uint RecordMap_ExpressionToRecordID(RecordMap *record_map, AR_ExpNode *exp) {
    // uint *id_ptr = TrieMap_Find(record_map->map, (char*)&exp, sizeof(AR_ExpNode));
    // if (id_ptr != TRIEMAP_NOTFOUND) return *id_ptr;

    // uint id = IDENTIFIER_NOT_FOUND;
    // // If the expression contains an alias, map it first, and re-use its Record ID if one is already assigned
    // if (exp->type == AR_EXP_OPERAND && exp->operand.type == AR_EXP_VARIADIC && exp->operand.variadic.entity_alias) {
        // id = RecordMap_LookupAlias(record_map, exp->operand.variadic.entity_alias);
    // }

    // if (id == IDENTIFIER_NOT_FOUND) id = record_map->record_len;

    // id_ptr = rm_malloc(sizeof(uint));
    // *id_ptr = id;
    // TrieMap_Add(record_map->map, (char*)&exp, sizeof(exp), id_ptr, TrieMap_DONT_CARE_REPLACE);

    // return *id_ptr;
// }

// uint RecordMap_LookupEntity(const RecordMap *record_map, AST_IDENTIFIER entity) {
    // uint *id = TrieMap_Find(record_map->map, (char*)&entity, sizeof(entity));
    // if (id == TRIEMAP_NOTFOUND) return IDENTIFIER_NOT_FOUND;
    // return *id;
// }

uint RecordMap_LookupAlias(const RecordMap *record_map, const char *alias) {
    uint *id_ptr = TrieMap_Find(record_map->map, (char*)alias, strlen(alias));
    if (id_ptr == TRIEMAP_NOTFOUND) return IDENTIFIER_NOT_FOUND;

    return *id_ptr;
}

uint RecordMap_LookupEntityID(const RecordMap *record_map, uint id) {
    uint *id_ptr = TrieMap_Find(record_map->map, (char*)&id, sizeof(id));
    if (id_ptr == TRIEMAP_NOTFOUND) return IDENTIFIER_NOT_FOUND;

    return *id_ptr;
}

uint RecordMap_FindOrAddASTEntity(RecordMap *record_map, const AST *ast, const cypher_astnode_t *entity) {
    // Ensure this is a new entity
    // assert(TrieMap_Find(record_map->map, (char*)&entity, sizeof(entity)) == TRIEMAP_NOTFOUND);

    uint id;
    // Return the ID immediately if this entity is already in the Record Map
    // id = RecordMap_LookupEntity(record_map, entity);
    // if (id != IDENTIFIER_NOT_FOUND) return id;

    // Retrieve the AST ID
    uint ast_id = AST_GetEntityIDFromReference(ast, entity);
    // assert(ast_id != IDENTIFIER_NOT_FOUND); // TODO fix

    // If the AST ID has a corresponding Record ID, return it.
    id = RecordMap_LookupEntityID(record_map, ast_id);
    if (id != IDENTIFIER_NOT_FOUND) return id;

    // We're adding a new Record mapping; create a new ID
    id = record_map->record_len++;

    // Map the AST ID
    uint *id_ptr = _BuildMapValue(id);
    TrieMap_Add(record_map->map, (char*)&ast_id, sizeof(ast_id), id_ptr, TrieMap_DONT_CARE_REPLACE);

    // Map AST pointer
    id_ptr = _BuildMapValue(id);
    TrieMap_Add(record_map->map, (char*)&entity, sizeof(entity), id_ptr, TrieMap_DONT_CARE_REPLACE);

    // Map alias - TODO this kinda sucks
    const char *alias = NULL;
    cypher_astnode_type_t type = cypher_astnode_type(entity);
    if (type == CYPHER_AST_IDENTIFIER) {
        alias = cypher_ast_identifier_get_name(entity);
    } else if (type == CYPHER_AST_NODE_PATTERN) {
        const cypher_astnode_t *ast_alias = cypher_ast_node_pattern_get_identifier(entity);
        if (ast_alias) alias = cypher_ast_identifier_get_name(ast_alias);
    } else if (type == CYPHER_AST_REL_PATTERN) {
        const cypher_astnode_t *ast_alias = cypher_ast_rel_pattern_get_identifier(entity);
        if (ast_alias) alias = cypher_ast_identifier_get_name(ast_alias);
    } else if (type == CYPHER_AST_PROPERTY_OPERATOR) {
        const cypher_astnode_t *ast_expr = cypher_ast_property_operator_get_expression(entity);
        if (cypher_astnode_type(ast_expr) == CYPHER_AST_IDENTIFIER) {
            alias = cypher_ast_identifier_get_name(ast_expr);
        }
    } else {
        assert(false);
    }
    if (alias) {
        id_ptr = _BuildMapValue(id);
        TrieMap_Add(record_map->map, (char*)alias, strlen(alias), id_ptr, TrieMap_DONT_CARE_REPLACE);
        // printf("Alias %s at %d from FindOrAddASTEntity\n", alias, id);
    }
    // Map AR_ExpNode?

    return id;
}

uint RecordMap_FindOrAddID(RecordMap *record_map, uint entity_id) {
    // Ensure this is a new entity
    // assert(TrieMap_Find(record_map->map, (char*)&entity, sizeof(entity)) == TRIEMAP_NOTFOUND);

    uint *id_ptr = TrieMap_Find(record_map->map, (char*)&entity_id, sizeof(entity_id));
    if (id_ptr != TRIEMAP_NOTFOUND) return *id_ptr;

    uint id = record_map->record_len++;

    // Map ID value
    id_ptr = _BuildMapValue(id);
    TrieMap_Add(record_map->map, (char*)&entity_id, sizeof(entity_id), id_ptr, TrieMap_DONT_CARE_REPLACE);

    return id;
}

uint RecordMap_FindOrAddAlias(RecordMap *record_map, const char *alias) {
    // This alias may already be represented in the Record map
    uint *id_ptr = TrieMap_Find(record_map->map, (char*)alias, strlen(alias));
    if (id_ptr != TRIEMAP_NOTFOUND) return *id_ptr;

    // TODO tmp
    AST *ast = AST_GetFromTLS();
    uint record_id;

    // Retrieve the AST ID
    uint ast_id = AST_GetEntityIDFromAlias(ast, alias);
    // The AST ID will not exist if this alias is projected by a WITH clause
    if (ast_id != IDENTIFIER_NOT_FOUND) {
        // This AST ID may already be represented in the record map
        record_id = RecordMap_LookupEntityID(record_map, ast_id);
        if (record_id != IDENTIFIER_NOT_FOUND) return record_id;
    }

    // if (record_id != IDENTIFIER_NOT_FOUND) {
        // id_ptr = _BuildMapValue(record_id);
        // TrieMap_Add(record_map->map, alias, strlen(alias), id_ptr, TrieMap_DONT_CARE_REPLACE);
        // return *id_ptr;
    // }

    // Generate new Record ID
    uint id = record_map->record_len++;

    // Map alias to Record ID
    id_ptr = _BuildMapValue(id);
    TrieMap_Add(record_map->map, (char*)alias, strlen(alias), id_ptr, TrieMap_DONT_CARE_REPLACE);

    // Map AST ID to Record ID
    if (ast_id != IDENTIFIER_NOT_FOUND) {
        id_ptr = _BuildMapValue(id);
        TrieMap_Add(record_map->map, (char*)&ast_id, sizeof(ast_id), id_ptr, TrieMap_DONT_CARE_REPLACE);
    }

    // printf("Alias %s at %d from FindOrAddAlias\n", alias, id);
    // Map ID value
    // id_ptr = _BuildMapValue(id);
    // TrieMap_Add(record_map->map, alias, strlen(alias), id_ptr, TrieMap_DONT_CARE_REPLACE);

    return id;
}

void RecordMap_AssociateAliasWithID(RecordMap *record_map, char *alias, uint id) {
    uint *id_ptr = _BuildMapValue(id);
    TrieMap_Add(record_map->map, alias, strlen(alias), id_ptr, TrieMap_DONT_CARE_REPLACE);
}

RecordMap *RecordMap_New() {
    RecordMap *record_map = rm_malloc(sizeof(RecordMap));
    record_map->map = NewTrieMap();
    record_map->record_len = 0;

    return record_map;
}

void RecordMap_Free(RecordMap *record_map) {
    TrieMap_Free(record_map->map, NULL);
    rm_free(record_map);
}

