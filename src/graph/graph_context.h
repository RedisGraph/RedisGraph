#ifndef _GRAPHCONTEXT_H
#define _GRAPHCONTEXT_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

typedef struct {
  void **indices;                // Array of indices. TODO move
  size_t index_ctr;               // Next index ID to be attached to graph.
} GraphContext;

// TODO temporary global
GraphContext *gc;

void GraphContext_New();

bool GraphContext_HasIndices();

// Returns a valid index ID and increments the graph's index count.
size_t GraphContext_AddIndexID();

#endif
