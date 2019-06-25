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

/* Forward declaration of edge */
struct Edge;

typedef struct {
    Entity *entity;                 /* MUST be the first property of Edge. */
    char *label;                    /* Label attached to node */
    int labelID;                    /* Label ID. */
    char *alias;                    /* Alias attached to node */
    GrB_Matrix mat;                 /* Label matrix, associated with node. */
    struct Edge** outgoing_edges;   /* Array of incoming edges (ME)<-(SRC) */
    struct Edge** incoming_edges;   /* Array of outgoing edges (ME)->(DEST) */
} Node;

/* Creates a new node. */
Node* Node_New(const char *label, const char *alias);

/* Checks if nodes are "equal". */
int Node_Compare(const Node *a, const Node *b);

/* Returns number of edges pointing into node. */
int Node_IncomeDegree(const Node *n);

/* Returns number of edges pointing out of node. */
int Node_OutgoingDegree(const Node *n);

/* Returns to total number of edges (Incoming&Outgoing). */
int Node_EdgeCount(const Node *n);

/* Connects source node to destination node by edge. */
void Node_ConnectNode(Node* src, Node* dest, struct Edge* e);

/* Removes given Incoming edge from node. */
void Node_RemoveIncomingEdge(Node *n, struct Edge *e);

/* Removes given Outgoing edge from node. */
void Node_RemoveOutgoingEdge(Node *n, struct Edge *e);

/* Sets node relation type. */
void Node_SetLabelID(Node *n, int labelID);

/* Retrieves node matrix. */
GrB_Matrix Node_GetMatrix(Node *n);

/* Clones given node. */
Node* Node_Clone(const Node *n);

/* Gets a string representation of given node. */
int Node_ToString(const Node *n, char *buff, int buff_len);

/* Frees allocated space by given node. */
void Node_Free(Node* node);

#endif