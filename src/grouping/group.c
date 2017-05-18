#include <stdio.h>
#include "group.h"
#include "../redismodule.h"

// Creates a new group
// arguments specify group's key.
Group* NewGroup(Vector* keys, Vector* funcs) {
    Group* g = malloc(sizeof(Group));

    g->aggregationFunctions = funcs;

    AggCtx* ctx;
    Vector_Get(g->aggregationFunctions, 0, &ctx);

    // Clone vector
    g->keys = NewVector(RedisModuleString*, Vector_Size(keys));
    for(int i = 0; i < Vector_Size(keys); i++) {
        RedisModuleString* key;
        Vector_Get(keys, i, &key);
        Vector_Push(g->keys, key);
    }

    return g;
}

void FreeGroup(Group* group) {
    if(group == NULL) return;
    
    // Vector_Free(group->keys);
    // Vector_Free(group->aggregationFunctions);
    free(group);
}