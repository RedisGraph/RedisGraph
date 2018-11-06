/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include "store.h"
#include "../util/rmalloc.h"
#include <assert.h>

/* Creates a new LabelStore. */
LabelStore* LabelStore_New(const char *label, int id) {
    LabelStore *store = rm_malloc(sizeof(LabelStore));
    store->properties = NewTrieMap();
    store->label = rm_strdup(label);
    store->id = id;
    
    return store;
}

void LabelStore_UpdateSchema(LabelStore *store, int prop_count, char **properties) {
    for(int idx = 0; idx < prop_count; idx++) {
        char *property = properties[idx];
        TrieMap_Add(store->properties, property, strlen(property), NULL, NULL);
    }
}

void LabelStore_Free(LabelStore *store) {
    TrieMap_Free(store->properties, TrieMap_NOP_CB);
    if(store->label) rm_free(store->label);
    rm_free(store);
}
