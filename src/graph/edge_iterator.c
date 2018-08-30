/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include "edge_iterator.h"
#include <assert.h>

EdgeIterator *EdgeIterator_New() {
    EdgeIterator *iter = malloc(sizeof(EdgeIterator));
    iter->edgeIdx = 0;
    iter->edgeCap = EDGE_ITERATOR_EDGE_CAP;
    iter->edgeCount = 0;
    iter->edges = malloc(sizeof(Edge*) * iter->edgeCap);

    return iter;
}

Edge *EdgeIterator_Next(EdgeIterator *iter) {
    assert(iter);
    if(iter->edgeIdx >= iter->edgeCount) return NULL;

    Edge *e = iter->edges[iter->edgeIdx++];
    return e;
}

void EdgeIterator_Reset(EdgeIterator *iter) {
    assert(iter);
    iter->edgeIdx = 0;
}

void EdgeIterator_Reuse(EdgeIterator *iter) {
    assert(iter);
    iter->edgeIdx = 0;
    iter->edgeCount = 0;
}

void EdgeIterator_Free(EdgeIterator *iter) {
    assert(iter);
    free(iter->edges);
    free(iter);
}
