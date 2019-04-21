/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "schema.h"
#include "../util/arr.h"
#include "../util/rmalloc.h"
#include "../graph/graphcontext.h"
#include <assert.h>

Schema* Schema_New(const char *name, int id) {
    Schema *schema = rm_malloc(sizeof(Schema));
    schema->id = id;
    schema->name = rm_strdup(name);
    schema->indices = array_new(Index*, 4);
    schema->fulltextIdx = NULL;
    return schema;
}

const char *Schema_GetName(const Schema *s) {
    assert(s);
    return s->name;
}

unsigned short Schema_IndexCount(const Schema *s) {
    assert(s);
    return (unsigned short)array_len(s->indices);
}

Index* Schema_GetIndex(Schema *s, Attribute_ID id) {
    // Search for index.
    unsigned short index_count = (unsigned short)array_len(s->indices);
    for(unsigned short i = 0; i < index_count; i++) {
        Index *idx = s->indices[i];
        if(idx->attr_id == id) return idx;
    }

    // Couldn't locate index.
    return NULL;
}

void Schema_SetFullTextIndex(Schema *s, RSIndex *idx) {
    assert(s && idx);
    // Overriding previouse index.
    if(s->fulltextIdx && idx != s->fulltextIdx) {
        RediSearch_DropIndex(s->fulltextIdx);
    }
    s->fulltextIdx = idx;
}

RSIndex *Schema_GetFullTextIndex(const Schema *s) {
    assert(s);
    return s->fulltextIdx;
}

int Schema_AddIndex(Schema *s, Attribute_ID attr_id) {
    // Make sure attribute isn't already indexed.
    if(Schema_GetIndex(s, attr_id) != NULL) return INDEX_FAIL;

    // Populate an index for the label-attribute pair using the Graph interfaces.
    GraphContext *gc = GraphContext_GetFromTLS();
    const char *attribute = GraphContext_GetAttributeString(gc, attr_id);
    Index *idx = Index_Create(gc->g, s->name, s->id, attribute, attr_id);

    // Add index to schema.
    s->indices = array_append(s->indices, idx);

    return INDEX_OK;
}

int Schema_RemoveIndex(Schema *s, Attribute_ID attr_id) {
    // Search for index.
    unsigned short index_count = (unsigned short)array_len(s->indices);
    for(int i = 0; i < index_count; i++) {
        Index *idx = s->indices[i];
        if(idx->attr_id == attr_id) {
            // Pop the last stored index
            Index *last_idx = array_pop(s->indices);
            if (idx != last_idx) {
                // If the index being deleted is not the last, swap the last into the newly-emptied position
                s->indices[i] = last_idx;                          
            }
            // Remove index.
            Index_Free(idx);
            return INDEX_OK;
        }
    }
    return INDEX_FAIL;
}

void Schema_Free(Schema *schema) {
    if(schema->name) rm_free(schema->name);

    // Free indicies.
    uint32_t index_count = array_len(schema->indices);
    for(int i = 0; i < index_count; i++) Index_Free(schema->indices[i]);
    array_free(schema->indices);
    if(schema->fulltextIdx) RediSearch_DropIndex(schema->fulltextIdx);
    rm_free(schema);
}
