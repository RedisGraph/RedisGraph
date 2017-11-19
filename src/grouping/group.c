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