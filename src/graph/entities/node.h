/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "../../value.h"
#include "graph_entity.h"

typedef GrB_Index LabelID;

// Helper macro that instantiates 'labels' as a stack array and updates 'label_count'.
#define NODE_GET_LABELS(g, n, labels, label_count)                      \
    GrB_Index labels[(label_count) = Graph_LabelTypeCount((g))];        \
    label_count = Graph_GetNodeLabels((g), (n), labels, (label_count))

typedef struct {
	Entity *entity;       // MUST be the first member of Node
	EntityID id;          // Unique id, MUST be the second member
} Node;

// Instantiate a new unpopulated node.
#define GE_NEW_NODE()           \
(Node) {                        \
    .entity = NULL,             \
    .id = INVALID_ENTITY_ID,    \
}

/* Prints a string representation of the node to buffer, return the string length. */
void Node_ToString(const Node *n, char **buffer, size_t *bufferLen, size_t *bytesWritten,
				   GraphEntityStringFromat format);

