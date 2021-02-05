/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "../../value.h"
#include "graph_entity.h"
#include "../../../deps/GraphBLAS/Include/GraphBLAS.h"

typedef struct {
	Entity *entity;     // MUST be the first member of Node
	EntityID id;        // Unique id, MUST be the second member
	const char *label;  // Label attached to Node
	int labelID;        // Label ID
} Node;

// Instantiate a new unpopulated node.
#define GE_NEW_NODE()           \
(Node) {                        \
    .entity = NULL,             \
    .id = INVALID_ENTITY_ID,    \
    .label = NULL,              \
    .labelID = GRAPH_NO_LABEL   \
}

// Instantiate a new node with label data.
#define GE_NEW_LABELED_NODE(l_str, l_id)    \
(Node) {                                    \
    .entity = NULL,                         \
    .id = INVALID_ENTITY_ID,                \
    .label = (l_str),                       \
    .labelID = (l_id)                       \
}                                           \

/* Resolves to the label string of the given Node. */
#define NODE_GET_LABEL(n) (n)->label

/* Resolves to the label ID of the given Node.
 * We first attempt to retrieve it from the local entity, then check the graph if not found.
 * If the Node is unlabeled, the return value will be GRAPH_NO_LABEL. */
#define NODE_GET_LABEL_ID(n, g)                                                                   \
({                                                                                                \
    if ((n)->labelID == GRAPH_NO_LABEL) (n)->labelID = Graph_GetNodeLabel((g), ENTITY_GET_ID(n)); \
    (n)->labelID;                                                                                 \
})

/* Prints a string representation of the node to buffer, return the string length. */
void Node_ToString(const Node *n, char **buffer, size_t *bufferLen, size_t *bytesWritten,
				   GraphEntityStringFromat format);

