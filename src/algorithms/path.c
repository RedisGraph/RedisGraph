/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include "./path.h"
#include "../util/arr.h"

Path Path_new(size_t len) {
    return array_new(Node, len);
}

Path Path_append(Path p, Node n) {
    return array_append(p, n);
}

Node Path_pop(Path p) {
    return array_pop(p);
}

size_t Path_len(const Path p) {
    return array_len(p);
}

bool Path_empty(const Path p) {
    return (array_len(p) == 0);
}

bool Path_containsNode(const Path p, Node *n) {
    uint32_t pathLen = Path_len(p);
	for(int i = 0; i < pathLen; i++) {
        if(ENTITY_GET_ID(p+i) == ENTITY_GET_ID(n)) return true;
    }
    return false;
}

Path Path_clone(const Path p) {
    int pathLen = Path_len(p);
    Path clone = Path_new(pathLen);

    for (int i = 0; i < pathLen; i++) {
        clone = Path_append(clone, p[i]);
    }

    return clone;
}

void Path_print(Path p) {
    assert(p);
    Node *n = NULL;
    int pathLen = Path_len(p);

    for(int i = 0; i < pathLen; i++) {
        n = p+i;
        printf("%llu", ENTITY_GET_ID(n));
    }
}

void Path_free(Path p) {
    array_free(p);
}
