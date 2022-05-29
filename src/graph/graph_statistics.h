/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include <stdint.h>
#include "../util/arr.h"

// Graph related statistics

typedef struct {
	uint64_t *node_count; // array of node count per label matrix
	uint64_t *edge_count; // array of edge count per relationship matrix
} GraphStatistics;

// initialize the node_count and edge_count arrays
void GraphStatistics_init
(
	GraphStatistics *stats
);

// clone a GraphStatistics object
GraphStatistics GraphStatistics_Clone
(
	const GraphStatistics *orig_stats
);

// new relationship is added, resize the edge_count array
void GraphStatistics_IntroduceRelationship
(
	GraphStatistics *stats
);

// new label is added, resize the node_count array
void GraphStatistics_IntroduceLabel
(
	GraphStatistics *stats
);

// increment the edge counter by amount
static inline void GraphStatistics_IncEdgeCount
(
	GraphStatistics *stats,
	int relation_idx,
	uint64_t amount
)
{
	ASSERT(relation_idx < array_len(stats->edge_count));
	stats->edge_count[relation_idx] += amount;
}

// decrement the edge counter by amount
static inline void GraphStatistics_DecEdgeCount
(
	GraphStatistics *stats,
	int relation_idx,
	uint64_t amount
) {
	ASSERT(relation_idx < array_len(stats->edge_count));
	ASSERT(stats->edge_count[relation_idx] >= amount);
	stats->edge_count[relation_idx] -= amount;
}

// increment the node counter by amount
static inline void GraphStatistics_IncNodeCount
(
	GraphStatistics *stats,
	int label_idx,
	uint64_t amount
) {
	ASSERT(label_idx < array_len(stats->node_count));
	stats->node_count[label_idx] += amount;
}

// decrement the node counter by amount
static inline void GraphStatistics_DecNodeCount
(
	GraphStatistics *stats,
	int label_idx,
	uint64_t amount
) {
	ASSERT(label_idx < array_len(stats->node_count));
	ASSERT(stats->node_count[label_idx] >= amount);

	stats->node_count[label_idx] -= amount;
}

// retrieves edge count for given relationship type
uint64_t GraphStatistics_EdgeCount
(
	const GraphStatistics *stats,
	int relation_idx
);

// retrieves node count for given label
uint64_t GraphStatistics_NodeCount
(
	const GraphStatistics *stats,
	int label_idx
);

// free the internal structures
void GraphStatistics_FreeInternals
(
	GraphStatistics *stats
);

