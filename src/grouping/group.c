/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include <stdio.h>
#include "group.h"
#include "../redismodule.h"

// Creates a new group
// arguments specify group's key.
Group* NewGroup(int key_count, SIValue* keys, Vector* funcs) {
    Group* g = malloc(sizeof(Group));
    g->keys = keys;
    g->key_count = key_count;
    g->aggregationFunctions = funcs;
    return g;
}

void FreeGroup(Group* group) {
    if(group == NULL) return;

    /* TODO: Free keys. */
    free(group);
}