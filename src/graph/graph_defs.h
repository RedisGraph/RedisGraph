
/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#define GRAPH_DEFAULT_NODE_CAP 16384            // Default number of nodes a graph can hold before resizing.
#define GRAPH_DEFAULT_EDGE_CAP 16384            // Default number of edges a graph can hold before resizing.
#define GRAPH_DEFAULT_RELATION_TYPE_CAP 16      // Default number of different relationship types a graph can hold before resizing.
#define GRAPH_DEFAULT_LABEL_CAP 16              // Default number of different labels a graph can hold before resizing.
#define GRAPH_NO_LABEL -1                       // Labels are numbered [0-N], -1 represents no label.
#define GRAPH_UNKNOWN_LABEL -2                  // Labels are numbered [0-N], -2 represents an unknown relation.
#define GRAPH_NO_RELATION -1                    // Relations are numbered [0-N], -1 represents no relation.
#define GRAPH_UNKNOWN_RELATION -2               // Relations are numbered [0-N], -2 represents an unknown relation.

// Mask with most significat bit on 10000...
#define MSB_MASK (1UL << (sizeof(EntityID) * 8 - 1))
// Mask complement 01111...
#define MSB_MASK_CMP ~MSB_MASK
// Set X's most significat bit on.
#define SET_MSB(x) (x) | MSB_MASK
// Clear X's most significat bit on.
#define CLEAR_MSB(x) (x) & MSB_MASK_CMP
// Checks if X represents edge ID.
#define SINGLE_EDGE(x) (x) & MSB_MASK
// Returns edge ID.
#define SINGLE_EDGE_ID(x) CLEAR_MSB(x)
