/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#ifndef NODE_H_
#define NODE_H_

#include "../../value.h"
#include "graph_entity.h"
#include "../../util/vector.h"
#include "../../../deps/GraphBLAS/Include/GraphBLAS.h"

/* Forward declaration of edge */
struct Edge;

typedef struct {
    Entity *entity;             /* MUST be the first property of Edge. */
    char *label;                /* label attached to node */
    int labelID;                /* Label ID. */
    char *alias;                /* alias attached to node */
    GrB_Matrix mat;             /* Label matrix, associated with node. */
    Vector* outgoing_edges;     /* list of incoming edges (ME)<-(SRC) */
    Vector* incoming_edges;     /* list on outgoing edges (ME)->(DEST) */
} Node;

/* Creates a new node. */
Node* Node_New(const char *label, const char *alias);

/* Checks if nodes are "equal" */
int Node_Compare(const Node *a, const Node *b);

/* Returns number of edges pointing into node */
int Node_IncomeDegree(const Node *n);

/* Connects source node to destination node by edge */
void Node_ConnectNode(Node* src, Node* dest, struct Edge* e);

/* Sets node relation type */
void Node_SetLabelID(Node *n, int labelID);

/* Retrieves node matrix */
GrB_Matrix Node_GetMatrix(Node *n);

/* Clones given node. */
Node* Node_Clone(const Node *n);

/* Frees allocated space by given node. */
void Node_Free(Node* node);

#endif