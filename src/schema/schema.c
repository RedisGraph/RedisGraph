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
    return schema;
}

Attribute_ID Attribute_GetID(const char *attribute) {
    GraphContext *gc = GraphContext_GetFromTLS();
    if(!gc) return ATTRIBUTE_NOTFOUND;

    Attribute_ID *id = TrieMap_Find(gc->attributes, (char*)attribute, strlen(attribute));
    if (id == TRIEMAP_NOTFOUND) return ATTRIBUTE_NOTFOUND;
    return *id;
}

unsigned short Schema_IndexCount(const Schema *s) {
    assert(s);
    return (unsigned short)array_len(s->indices);
}

Index* Schema_GetIndex(Schema *s, const char *attribute) {
    assert(s && attribute);

    // See if attribute exists
    Attribute_ID id = Attribute_GetID(attribute);
    if(id == ATTRIBUTE_NOTFOUND) return NULL;
    
    // Search for index.
    unsigned short index_count = (unsigned short)array_len(s->indices);
    for(int i = 0; i < index_count; i++) {
        Index *idx = s->indices[i];
        if(idx->attr_id == id) return idx;
    }

    // Couldn't locate index.
    return NULL;
}

void Schema_AddIndex(Schema *s, char *attribute, Index *idx) {    
    // See if attribute exists
    Attribute_ID id = Attribute_GetID(attribute);
    if(id == ATTRIBUTE_NOTFOUND) return;

    // Make sure attribute isn't already indexed.
    assert(Schema_GetIndex(s, attribute) == NULL);

    // Add index to schema.
    s->indices = array_append(s->indices, idx);
}

void Schema_RemoveIndex(Schema *s, const char *attribute) {
    // See if attribute exists
    Attribute_ID id = Attribute_GetID(attribute);
    if(id == ATTRIBUTE_NOTFOUND) return;

    // Search for index.
    unsigned short index_count = (unsigned short)array_len(s->indices);
    for(int i = 0; i < index_count; i++) {
        Index *idx = s->indices[i];
        if(idx->attr_id == id) {
            // Pop the last stored index
            Index *last_idx = array_pop(s->indices);
            if (idx != last_idx) {
                // If the index being deleted is not the last, swap the last into the newly-emptied position
                s->indices[i] = last_idx;                          
            }
            // Remove index.
            Index_Free(idx);
            break;
        }
    }
}

void Schema_Free(Schema *schema) {
    if(schema->name) rm_free(schema->name);

    // Free indicies.
    uint32_t index_count = array_len(schema->indices);
    for(int i = 0; i < index_count; i++) Index_Free(schema->indices[i]);
    array_free(schema->indices);

    rm_free(schema);
}
