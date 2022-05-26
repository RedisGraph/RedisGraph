/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "graph_statistics.h"

// Initialize the node_count and edge_count arrays
void GraphStatistics_init
(
	GraphStatistics *stats
) {
	ASSERT(stats != NULL);
	stats->node_count = array_new(uint64_t, 0);
	stats->edge_count = array_new(uint64_t, 0);
}

GraphStatistics GraphStatistics_Clone
(
	const GraphStatistics *orig_stats
) {
	ASSERT(orig_stats);

	GraphStatistics stats;
	array_clone(stats.node_count, orig_stats->node_count);
	array_clone(stats.edge_count, orig_stats->edge_count);

	return stats;
}

void GraphStatistics_IntroduceRelationship
(
	GraphStatistics *stats
) {
	ASSERT(stats != NULL);
	ASSERT(stats->edge_count != NULL);

	array_append(stats->edge_count, 0);
}

void GraphStatistics_IntroduceLabel
(
	GraphStatistics *stats
) {
	ASSERT(stats != NULL);
	ASSERT(stats->node_count != NULL);

	array_append(stats->node_count, 0);
}

uint64_t GraphStatistics_EdgeCount
(
	const GraphStatistics *stats,
	int relation_idx
) {
	ASSERT(stats != NULL);
	ASSERT(relation_idx < array_len(stats->edge_count));

	return stats->edge_count[relation_idx];
}

uint64_t GraphStatistics_NodeCount
(
	const GraphStatistics *stats,
	int label_idx
) {
	ASSERT(stats != NULL);
	ASSERT(label_idx < (int)array_len(stats->node_count));

	if(label_idx < 0) return 0;
	return stats->node_count[label_idx];
}

void GraphStatistics_FreeInternals
(
	GraphStatistics *stats
) {
	ASSERT(stats != NULL);

	if(stats->node_count) array_free(stats->node_count);
	if(stats->edge_count) array_free(stats->edge_count);
}

