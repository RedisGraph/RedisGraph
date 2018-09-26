#include "graph_context.h"

void GraphContext_New() {
  gc = malloc(sizeof(GraphContext));
  gc->indices = NULL;
  gc->index_ctr = 0;
}

bool GraphContext_HasIndices() {
  return gc->index_ctr > 0;
}

size_t GraphContext_AddIndexID() {
    assert(gc);
    size_t latest_idx = gc->index_ctr ++;
    if (!gc->indices) {
      gc->indices = malloc(sizeof(void*));
    } else {
      gc->indices = realloc(gc->indices, gc->index_ctr * sizeof(void*));
    }
    return latest_idx;
}

