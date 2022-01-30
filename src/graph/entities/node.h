/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "../../value.h"
#include "graph_entity.h"

typedef int LabelID;

// helper macro that instantiates 'labels' as a stack array
// and updates 'label_count'
#define NODE_GET_LABELS(g, n, label_count)                              \
    LabelID labels[(label_count) = Graph_LabelTypeCount((g))];          \
    label_count = Graph_GetNodeLabels((g), (n), labels, (label_count))

// instantiate a new unpopulated node
#define GE_NEW_NODE()           \
(Node) {                        \
    .entity = NULL,             \
    .id = INVALID_ENTITY_ID,    \
}

// struct representing a node in the graph
typedef struct {
	Entity *entity;       // MUST be the first member of Node
	EntityID id;          // unique id, MUST be the second member
} Node;

void Node_Clone(const Node *n, Node *clone);

// prints a string representation of the node to buffer
// return the string length
void Node_ToString(const Node *n, char **buffer, size_t *bufferLen,
				   size_t *bytesWritten, GraphEntityStringFromat format);

