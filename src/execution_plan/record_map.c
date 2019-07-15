/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "record_map.h"
#include "../util/rmalloc.h"
#include <assert.h>

static void* _BuildMapValue(uint64_t id) {
    // TODO so many unnecessary allocs
    return (void*)id;
}

uint RecordMap_GetRecordIDFromReference(RecordMap *record_map, AST_IDENTIFIER entity) {
    void *id = TrieMap_Find(record_map->map, (char*)&entity, sizeof(entity));
    if (id == TRIEMAP_NOTFOUND) return IDENTIFIER_NOT_FOUND;
    return (uint64_t)id;
}

// TODO unused except in op_procedure_call
uint RecordMap_LookupAlias(const RecordMap *record_map, const char *alias) {
    void *id_ptr = TrieMap_Find(record_map->map, (char*)alias, strlen(alias));
    if (id_ptr == TRIEMAP_NOTFOUND) return IDENTIFIER_NOT_FOUND;

    return (uint64_t)id_ptr;
}

uint RecordMap_LookupEntityID(const RecordMap *record_map, uint id) {
    void *id_ptr = TrieMap_Find(record_map->map, (char*)&id, sizeof(id));
    if (id_ptr == TRIEMAP_NOTFOUND) return IDENTIFIER_NOT_FOUND;

    return (uint64_t)id_ptr;
}

uint RecordMap_FindOrAddASTEntity(RecordMap *record_map, const AST *ast, const cypher_astnode_t *entity) {
    // Retrieve the AST ID
    uint ast_id = AST_GetEntityIDFromReference(ast, entity);

    // If the AST ID has a corresponding Record ID, return it.
    uint id = RecordMap_LookupEntityID(record_map, ast_id);
    if (id != IDENTIFIER_NOT_FOUND) return id;

    // We're adding a new Record mapping; create a new ID
    id = record_map->record_len++;

    // Map the AST ID
    void *id_ptr = _BuildMapValue(id);
    TrieMap_Add(record_map->map, (char*)&ast_id, sizeof(ast_id), id_ptr, TrieMap_DONT_CARE_REPLACE);

    // Map AST pointer
    id_ptr = _BuildMapValue(id);
    TrieMap_Add(record_map->map, (char*)&entity, sizeof(entity), id_ptr, TrieMap_DONT_CARE_REPLACE);

    // Map alias if one is provided. 
    const char *alias = AST_GetEntityAlias(entity);

    // Entities like (:node {val: 5}) will be mapped by AST ID and pointer, but not alias.
    if (alias) {
        id_ptr = _BuildMapValue(id);
        TrieMap_Add(record_map->map, (char*)alias, strlen(alias), id_ptr, TrieMap_DONT_CARE_REPLACE);
    }

    return id;
}

uint RecordMap_FindOrAddID(RecordMap *record_map, uint entity_id) {
    void *id_ptr = TrieMap_Find(record_map->map, (char*)&entity_id, sizeof(entity_id));
    if (id_ptr != TRIEMAP_NOTFOUND) return (uint64_t)id_ptr;

    uint id = record_map->record_len++;

    // Map ID value
    id_ptr = _BuildMapValue(id);
    TrieMap_Add(record_map->map, (char*)&entity_id, sizeof(entity_id), id_ptr, TrieMap_DONT_CARE_REPLACE);

    return id;
}

uint RecordMap_FindOrAddAlias(RecordMap *record_map, const char *alias) {
    // This alias may already be represented in the Record map
    void *id_ptr = TrieMap_Find(record_map->map, (char*)alias, strlen(alias));
    if (id_ptr != TRIEMAP_NOTFOUND) return (uint64_t)id_ptr;

    // TODO this logic could be improved. 
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

    return id;
}

void RecordMap_AssociateAliasWithID(RecordMap *record_map, char *alias, uint id) {
    void *id_ptr = _BuildMapValue(id);
    TrieMap_Add(record_map->map, alias, strlen(alias), id_ptr, TrieMap_DONT_CARE_REPLACE);
}

RecordMap *RecordMap_New() {
    RecordMap *record_map = rm_malloc(sizeof(RecordMap));
    record_map->map = NewTrieMap();
    record_map->record_len = 0;

    return record_map;
}

void RecordMap_Free(RecordMap *record_map) {
    TrieMap_Free(record_map->map, TrieMap_NOP_CB);
    rm_free(record_map);
}

