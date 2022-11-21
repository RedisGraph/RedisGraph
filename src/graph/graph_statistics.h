/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#pragma once

#include <stdint.h>
#include "../util/arr.h"

// Graph related statistics

typedef struct {
	uint64_t *node_count; // Array of node count per label matrix
	uint64_t *edge_count; // Array of edge count per relationship matrix
} GraphStatistics;

// Initialize the node_count and edge_count arrays
void GraphStatistics_init(GraphStatistics *stats);

// New relationship is added, resize the edge_count array.
void GraphStatistics_IntroduceRelationship(GraphStatistics *stats);

// New label is added, resize the node_count array.
void GraphStatistics_IntroduceLabel(GraphStatistics *stats);

// Increment the edge counter by amount
static inline void GraphStatistics_IncEdgeCount(GraphStatistics *stats,
												int relation_idx, uint64_t amount) {
	ASSERT(relation_idx < array_len(stats->edge_count));
	stats->edge_count[relation_idx] += amount;
}

// Decrement the edge counter by amount
static inline void GraphStatistics_DecEdgeCount(GraphStatistics *stats,
												int relation_idx, uint64_t amount) {
	ASSERT(relation_idx < array_len(stats->edge_count) &&
		   stats->edge_count[relation_idx] >= amount);
	stats->edge_count[relation_idx] -= amount;
}

// Increment the node counter by amount
static inline void GraphStatistics_IncNodeCount(GraphStatistics *stats,
												int label_idx, uint64_t amount) {
	ASSERT(label_idx < array_len(stats->node_count));
	stats->node_count[label_idx] += amount;
}

// Decrement the node counter by amount
static inline void GraphStatistics_DecNodeCount(GraphStatistics *stats,
												int label_idx, uint64_t amount) {
	ASSERT(label_idx < array_len(stats->node_count) &&
		   stats->node_count[label_idx] >= amount);
	stats->node_count[label_idx] -= amount;
}

// Retrieves edge count for given relationship type
uint64_t GraphStatistics_EdgeCount(const GraphStatistics *stats,
								   int relation_idx);

// Retrieves node count for given label
uint64_t GraphStatistics_NodeCount(const GraphStatistics *stats,
								   int label_idx);

// Free the internal structures.
void GraphStatistics_FreeInternals(GraphStatistics *stats);

