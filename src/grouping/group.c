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
Group* NewGroup(int key_count, SIValue* keys, AR_ExpNode** funcs) {
    Group* g = rm_malloc(sizeof(Group));
    g->keys = keys;
    g->key_count = key_count;
    g->aggregationFunctions = funcs;
    return g;
}

void FreeGroup(Group* group) {
    if(group == NULL) return;
    if(group->key_count) rm_free(group->keys);
    if(group->aggregationFunctions) {
        for(uint32_t i = 0; i < array_len(group->aggregationFunctions); i++) {
            AR_ExpNode *exp = group->aggregationFunctions[i];
            AR_EXP_Free(exp);
        }
        array_free(group->aggregationFunctions);
    }
    rm_free(group);
}
