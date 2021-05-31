/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include <stdint.h>
#include "graph_defs.h"
#include "../util/arr.h"

/* Graph related statistics */

typedef struct {
	uint64_t *edge_count; // Array of relationship matrice's edges counters.
} GraphStatistics;

// Initialize the edge_count array
static inline void GraphStatistics_init(GraphStatistics *stats) {
    stats->edge_count = array_new(uint64_t, GRAPH_DEFAULT_RELATION_TYPE_CAP);
}

// Increment the edge counter by amount
static inline void GraphStatistics_IncEdgeCounter(GraphStatistics *stats, int relation_idx, uint64_t amount) {
    ASSERT(relation_idx < array_len(stats->edge_count));
    stats->edge_count[relation_idx] += amount;
}

// Decrement the edge counter by amount
static inline void GraphStatistics_DecEdgeCounter(GraphStatistics *stats, int relation_idx, uint64_t amount) {
    ASSERT(relation_idx < array_len(stats->edge_count) && stats->edge_count[relation_idx] >= amount);
    stats->edge_count[relation_idx] -= amount;
}

static inline void GraphStatistics_free(GraphStatistics *stats) {
    if(stats->edge_count) array_free(stats->edge_count);
    stats->edge_count = NULL;
}
