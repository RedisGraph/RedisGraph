/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include "./path.h"
#include "../util/arr.h"

Path Path_new(size_t len) {
    Path p = array_new(Edge*, len);
    return p;
}

Path Path_append(Path p, Edge *e) {
    p = array_append(p, e);
    return p;
}

Edge *Path_pop(Path p) {
    size_t pathLen = array_len(p);
    if(pathLen == 0) return NULL;
    
    Edge *e = p[pathLen-1];
    array_pop(p);
    return e;
}

size_t Path_len(const Path p) {
    return array_len(p);
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
    Edge *e;
    NodeID n;
    int pathLen = Path_len(p);

    for(int i = 0; i < pathLen; i++) {
        e = p[i];
        n = Edge_GetSrcNodeID(e);
        printf("%llu", n);
    }

    n = Edge_GetDestNodeID(e);
    printf("%llu", n);
}

void Path_free(Path p) {
    array_free(p);
}
