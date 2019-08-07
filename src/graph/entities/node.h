/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#ifndef NODE_H_
#define NODE_H_

#include "../../value.h"
#include "graph_entity.h"
#include "../../../deps/GraphBLAS/Include/GraphBLAS.h"

typedef struct
{
    Entity *entity;    /* MUST be the first property of Edge. */
    const char *label; /* Label attached to node */
    int labelID;       /* Label ID. */
    GrB_Matrix mat;    /* Label matrix, associated with node. */
} Node;

/* Creates a new node. */
Node *Node_New(const char *label);

/* Sets node relation type. */
void Node_SetLabelID(Node *n, int labelID); // QG only

/* Retrieves node matrix. */
GrB_Matrix Node_GetMatrix(Node *n); // AE

/* Clones given node. */
Node *Node_Clone(const Node *n); // QG

/* prints a string representation of the node to buffer, return the string length*/
int Node_ToString(const Node *n, char *buffer, int bufferLen, GraphEntityStringFromat format);

/* Frees allocated space by given node. */
void Node_Free(Node *node);

#endif