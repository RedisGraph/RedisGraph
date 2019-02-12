/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include <stdio.h>
#include "group.h"
#include "../redismodule.h"
#include "../util/arr.h"
#include "../util/rmalloc.h"

// Creates a new group
// arguments specify group's key.
Group* NewGroup(int key_count, SIValue* keys, AR_ExpNode** funcs, Record r) {
    Group* g = rm_malloc(sizeof(Group));
    g->keys = keys;
    g->key_count = key_count;
    g->aggregationFunctions = funcs;
    if(r) g->r = Record_Clone(r);
    else g->r = NULL;
    return g;
}

void FreeGroup(Group* g) {
    if(g == NULL) return;
    if(g->r) Record_Free(g->r);
    if(g->keys) {
        for (int i = 0; i < g->key_count; i ++) {
            SIValue_Free(&g->keys[i]);
        }
        rm_free(g->keys);
    }
    if(g->aggregationFunctions) {
        for(uint32_t i = 0; i < array_len(g->aggregationFunctions); i++) {
            AR_ExpNode *exp = g->aggregationFunctions[i];
            AR_EXP_Free(exp);
        }
        array_free(g->aggregationFunctions);
    }
    rm_free(g);
}
