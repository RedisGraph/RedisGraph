/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "graph_statistics.h"

// Initialize the node_count and edge_count arrays
void GraphStatistics_init(GraphStatistics *stats) {
	ASSERT(stats);
	stats->node_count = array_new(uint64_t, 0);
	stats->edge_count = array_new(uint64_t, 0);
}

void GraphStatistics_IntroduceRelationship(GraphStatistics *stats) {
	ASSERT(stats && stats->edge_count);
	array_append(stats->edge_count, 0);
}

void GraphStatistics_IntroduceLabel(GraphStatistics *stats) {
	ASSERT(stats && stats->node_count);
	array_append(stats->node_count, 0);
}

uint64_t GraphStatistics_EdgeCount(const GraphStatistics *stats,
								   int relation_idx) {
	ASSERT(stats);
	ASSERT(relation_idx < array_len(stats->edge_count));
	return stats->edge_count[relation_idx];
}

uint64_t GraphStatistics_NodeCount(const GraphStatistics *stats,
								   int label_idx) {
	ASSERT(stats);
	ASSERT(label_idx < array_len(stats->node_count));
	return stats->node_count[label_idx];
}

void GraphStatistics_FreeInternals(GraphStatistics *stats) {
	ASSERT(stats);
	if(stats->node_count) array_free(stats->node_count);
	if(stats->edge_count) array_free(stats->edge_count);
}

