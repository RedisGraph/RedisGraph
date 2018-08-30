/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#ifndef _EDGE_ITERATOR_H
#define _EDGE_ITERATOR_H

#include <stdlib.h>
#include "edge.h"

#define EDGE_ITERATOR_EDGE_CAP 32
// Iterator over an array of edges.
typedef struct {
    size_t edgeIdx;     // Position within edges array.
    size_t edgeCap;     // Size of edges array.
    size_t edgeCount;   // Number of edges.
    Edge **edges;       // Array of edges.
} EdgeIterator;

// Create a new edge iterator.
EdgeIterator *EdgeIterator_New();

// Return next iteratable edge, or NULL if iterator
// reached its end.
Edge *EdgeIterator_Next(EdgeIterator *iter);

// Resets iterator, after which additional calls to EdgeIterator_Next
// can be made.
void EdgeIterator_Reset(EdgeIterator *iter);

// Clear iterator, such that new array of edges can be iterated.
void EdgeIterator_Reuse(EdgeIterator *iter);

// Free iterator.
void EdgeIterator_Free(EdgeIterator *iter);

#endif
