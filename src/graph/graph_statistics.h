/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include <stdint.h>
#include "../util/arr.h"

// Graph related statistics

typedef struct {
	uint64_t *edge_count; // Array of edge count per relationship matrix
} GraphStatistics;

// Initialize the edge_count array
void GraphStatistics_init(GraphStatistics *stats);

// New relationship is added, resize the edge_count array.
void GraphStatistics_IntroduceRelationship(GraphStatistics *stats);

// Increment the edge counter by amount
static inline void GraphStatistics_IncEdgeCount(GraphStatistics *stats, int relation_idx, uint64_t amount) {
    ASSERT(relation_idx < array_len(stats->edge_count));
    stats->edge_count[relation_idx] += amount;
}

// Decrement the edge counter by amount
static inline void GraphStatistics_DecEdgeCount(GraphStatistics *stats, int relation_idx, uint64_t amount) {
    ASSERT(relation_idx < array_len(stats->edge_count) && stats->edge_count[relation_idx] >= amount);
    stats->edge_count[relation_idx] -= amount;
}

// Retrieves edge count for given relationship type
uint64_t GraphStatistics_EdgeCount(const GraphStatistics *stats, int relation_idx);

// Free the internal structures.
void GraphStatistics_FreeInternals(GraphStatistics *stats);
