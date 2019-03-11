/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include <assert.h>

#include "util/arr.h"
#include "util/rmalloc.h"

#include "schema.h"
#include "graph/graphcontext.h"
#include "graph/entities/graph_entity.h"

Schema* Schema_New(const char *name, int id) {
    Schema *schema = rm_malloc(sizeof(Schema));
    schema->id = id;
    schema->name = rm_strdup(name);
    schema->attributes = NewTrieMap();
    schema->indices = array_new(Index*, 4);
    return schema;
}

unsigned short Schema_AttributeCount(const Schema *s) {
    assert(s);
    return (unsigned short)s->attributes->cardinality;
}

bool Schema_ContainsAttribute(const Schema *s, const char *attribute) {
    assert(s && attribute);
    Attribute_ID *pId = TrieMap_Find(s->attributes, (char*)attribute, strlen(attribute));
    return (pId != TRIEMAP_NOTFOUND);
}

Attribute_ID Attribute_GetID(SchemaType t, const char *attribute) {
    GraphContext *gc = GraphContext_GetFromTLS();
    if(!gc) return ATTRIBUTE_NOTFOUND;

    Schema *s = GraphContext_GetUnifiedSchema(gc, t);
    return Schema_GetAttributeID(s, attribute);
}

unsigned short Schema_IndexCount(const Schema *s) {
    assert(s);
    return (unsigned short)array_len(s->indices);
}

Attribute_ID Schema_GetAttributeID(Schema *s, const char *attribute) {
    Attribute_ID *id = TrieMap_Find(s->attributes, (char*)attribute, strlen(attribute));
    if(id == TRIEMAP_NOTFOUND) return ATTRIBUTE_NOTFOUND;
    return *id;
}

Attribute_ID Schema_AddAttribute(Schema *s, SchemaType t, const char *attribute) {
    assert(s && attribute);

    // See if attribute already exists.
    Attribute_ID *pAttribute_id = NULL;
    Attribute_ID attribute_id = Attribute_GetID(t, attribute);

    if(attribute_id == ATTRIBUTE_NOTFOUND) {
        /* First time we encounter attribute
         * add it to both current schema and unified schema. */
        Schema *unified_schema = GraphContext_GetUnifiedSchema(GraphContext_GetFromTLS(), t);
        attribute_id = Schema_AttributeCount(unified_schema);
        pAttribute_id = malloc(sizeof(Attribute_ID));
        *pAttribute_id = attribute_id;

        TrieMap_Add(unified_schema->attributes,
                    (char*)attribute,
                    strlen(attribute),
                    pAttribute_id,
                    TrieMap_DONT_CARE_REPLACE);

        if(s != unified_schema) {
            pAttribute_id = malloc(sizeof(Attribute_ID));
            *pAttribute_id = attribute_id;
            TrieMap_Add(s->attributes,
                        (char*)attribute,
                        strlen(attribute),
                        pAttribute_id,
                        TrieMap_DONT_CARE_REPLACE);
        }
    } else {
        /* We've encounter this attribute before,
         * make sure it's part of schema. */
        if(!Schema_ContainsAttribute(s, attribute)) {
            /* Attribute isn't part of given schema, this is possible
             * when multiple schemas contains the same attribute,
             * use attribute reference from unified schema. */
            pAttribute_id = malloc(sizeof(Attribute_ID));
            *pAttribute_id = attribute_id;
            TrieMap_Add(s->attributes,
                        (char*)attribute,
                        strlen(attribute),
                        pAttribute_id,
                        TrieMap_DONT_CARE_REPLACE);
        }
    }

    return attribute_id;
}

Index* Schema_GetIndex(Schema *s, const char *attribute) {
    assert(s && attribute);

    // See if attribute is part of schema.
    Attribute_ID id = Schema_GetAttributeID(s, attribute);
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
    // Make sure attribute is part of schema.
    Attribute_ID id = Schema_GetAttributeID(s, attribute);
    assert(id != ATTRIBUTE_NOTFOUND);

    // Make sure attribute isn't already indexed.
    assert(Schema_GetIndex(s, attribute) == NULL);

    // Add index to schema.
    array_append(s->indices, idx);
}

void Schema_RemoveIndex(Schema *s, const char *attribute) {
    // See if attribute is part of schema.
    Attribute_ID id = Schema_GetAttributeID(s, attribute);
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

// Create a map from attribute ID to attribute name
char** Schema_AttributeMap(Schema *s, unsigned short *attr_count) {
    *attr_count = Schema_AttributeCount(s);
    char **map = malloc(sizeof(char*) * (*attr_count));
    
    char *ptr;
    tm_len_t len;
    Attribute_ID *attr_id;
    TrieMapIterator *it = TrieMap_Iterate(s->attributes, "", 0);

    while(TrieMapIterator_Next(it, &ptr, &len, (void**)&attr_id)) {
        map[*attr_id] = malloc(sizeof(char) * len+1);
        memcpy(map[*attr_id], ptr, len);
        map[*attr_id][len] = '\0';
    }

    TrieMapIterator_Free(it);

    return map;
}

// Free attribute map created by Schema_AttributeMapping
void Schema_FreeAttributeMap(char **map, unsigned short map_len) {
    for(unsigned short i = 0; i < map_len; i++) free(map[i]);
    free(map);
}

void Schema_Free(Schema *schema) {
    if(schema->name) rm_free(schema->name);
    TrieMap_Free(schema->attributes, NULL);

    // Free indicies.
    uint32_t index_count = array_len(schema->indices);
    for(int i = 0; i < index_count; i++) Index_Free(schema->indices[i]);
    array_free(schema->indices);

    rm_free(schema);
}
