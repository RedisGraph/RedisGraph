/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "../../value.h"
#include "GraphBLAS.h"
#include "graph_entity.h"

// Node label
typedef struct {
	int id;               // label id
	const char *name;     // label name
} Label;

typedef struct {
	Entity *entity;       // MUST be the first member of Node
	EntityID id;          // Unique id, MUST be the second member
} Node;

// Instantiate a new unpopulated node.
#define GE_NEW_NODE()			\
(Node) {						\
	.entity = NULL,				\
	.id = INVALID_ENTITY_ID,	\
}

/* Prints a string representation of the node to buffer, return the string length. */
void Node_ToString(const Node *n, char **buffer, size_t *bufferLen, size_t *bytesWritten,
				   GraphEntityStringFromat format);

uint Node_GetLabels(const Node *n, Label *l, uint ln);

// Retreive all labels associated with node
#define NODE_LABELS(n, l, c)             \
	Label _lbls[16];                     \
	c = Node_GetLabels(n, _lbls, 16);    \
	l = (Label*)_lbls;                   \

