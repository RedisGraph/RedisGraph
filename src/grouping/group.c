#include <stdio.h>
#include "group.h"
#include "../redismodule.h"

// Creates a new group
// arguments specify group's key.
Group* NewGroup(Vector* keys, Vector* funcs) {
    Group* g = malloc(sizeof(Group));
    g->aggregationFunctions = funcs;
    g->keys = keys;
    return g;
}

void FreeGroup(Group* group) {
    if(group == NULL) return;
    
    // Vector_Free(group->keys);
    // Vector_Free(group->aggregationFunctions);
    free(group);
}