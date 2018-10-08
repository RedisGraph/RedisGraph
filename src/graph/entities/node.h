/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#ifndef NODE_H_
#define NODE_H_

#include "../../value.h"
#include "graph_entity.h"
#include "../../rmutil/vector.h"
#include "../../../deps/GraphBLAS/Include/GraphBLAS.h"

/* Forward declaration of edge */
struct Edge;
typedef struct {
	// GraphEntity entity;
	struct {
		NodeID id;
		int prop_count;
		EntityProperty *properties;
	};
  // TODO These elements should separated into a QueryGraph_Node type
  // so that we can correct datablock sizing for nodes
	char *label;			/* label attached to node */
	char *alias;			/* alias attached to node */
	GrB_Matrix mat;			/* Label matrix, associated with node. */
	Vector* outgoing_edges;	/* list of incoming edges (ME)<-(SRC) */
	Vector* incoming_edges;	/* list on outgoing edges (ME)->(DEST) */
} Node;

/* Creates a new node. */
Node* Node_New(NodeID id, const char *label, const char *alias);

/* Checks if nodes are "equal" */
int Node_Compare(const Node *a, const Node *b);

/* Returns number of edges pointing into node */
int Node_IncomeDegree(const Node *n);

/* Connects source node to destination node by edge */
void Node_ConnectNode(Node* src, Node* dest, struct Edge* e);

/* Adds properties to node
 * prop_count - number of new properties to add 
 * keys - array of properties keys 
 * values - array of properties values */
void Node_Add_Properties(Node *node, int prop_count, char **keys, SIValue *values);

/* Retrieves node's property
 * NOTE: If the key does not exist, we return the special
 * constant value PROPERTY_NOTFOUND. */
SIValue* Node_Get_Property(const Node *node, const char *key);

/* Frees allocated space by given node. */
void Node_Free(Node* node);

#endif