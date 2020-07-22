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
	Entity *entity;    /* MUST be the first member of Node. */
	const char *label; /* Label attached to Node. */
	int labelID;       /* Label ID. */
} Node;

/* Instantiate a new unpopulated node. */
Node Node_New(void);

/* Set the label and associated ID of the given node. */
void Node_SetLabel(Node *n, const char *label, int label_id);

/* Prints a string representation of the node to buffer, return the string length. */
void Node_ToString(const Node *n, char **buffer, size_t *bufferLen, size_t *bytesWritten,
				   GraphEntityStringFromat format);

