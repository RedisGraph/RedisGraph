/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "graph_statistics.h"

// Initialize the edge_count array
void GraphStatistics_init(GraphStatistics *stats) {
    ASSERT(stats);
    stats->edge_count = array_new(uint64_t, 0);
}

void GraphStatistics_IntroduceRelationship(GraphStatistics *stats) {
    ASSERT(stats && stats->edge_count);
    stats->edge_count = array_append(stats->edge_count, 0);
}

void GraphStatistics_FreeInternals(GraphStatistics *stats) {
    ASSERT(stats);
    if(stats->edge_count) array_free(stats->edge_count);
}

uint64_t GraphStatistics_EdgeCount(GraphStatistics *stats, int relation_idx) {
    ASSERT(relation_idx < array_len(stats->edge_count));
    return stats->edge_count[relation_idx];
}
