/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#ifndef NODE_H_
#define NODE_H_

#include "../../value.h"
#include "graph_entity.h"
#include "../../../deps/GraphBLAS/Include/GraphBLAS.h"

typedef struct {
	Entity *entity;    /* MUST be the first property of Edge. */
	const char *label; /* Label attached to node */
	int labelID;       /* Label ID. */
	GrB_Matrix mat;    /* Label matrix, associated with node. */
} Node;

/* Creates a new node. */
Node *Node_New(const char *label);

/* Retrieves node matrix. */
GrB_Matrix Node_GetMatrix(Node *n); // AE

/* Clones given node. */
Node *Node_Clone(const Node *n); // QG

/* Prints a string representation of the node to buffer, return the string length. */
void Node_ToString(const Node *n, char **buffer, size_t *bufferLen, size_t *bytesWritten,
				   GraphEntityStringFromat format);

/* Frees allocated space by given node. */
void Node_Free(Node *node);

#endif
