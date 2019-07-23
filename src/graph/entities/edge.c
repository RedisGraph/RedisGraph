/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "edge.h"
#include "graph_entity.h"
#include "../graphcontext.h"

Edge *Edge_New(Node *src, Node *dest, const char *relationship,
			   const char *alias) {
	assert(src && dest);

	Edge *e = calloc(1, sizeof(Edge));
	Edge_SetSrcNode(e, src);
	Edge_SetDestNode(e, dest);
	e->minHops = 1;
	e->maxHops = 1;
	e->relationID = GRAPH_UNKNOWN_RELATION;

	if(relationship != NULL) e->relationship = strdup(relationship);
	if(alias != NULL) e->alias = strdup(alias);

	return e;
}

NodeID Edge_GetSrcNodeID(const Edge *edge) {
	assert(edge);
	return edge->srcNodeID;
}

NodeID Edge_GetDestNodeID(const Edge *edge) {
	assert(edge);
	return edge->destNodeID;
}

int Edge_GetRelationID(const Edge *edge) {
	assert(edge);
	return edge->relationID;
}

Node *Edge_GetSrcNode(Edge *e) {
	assert(e);
	return e->src;
}

Node *Edge_GetDestNode(Edge *e) {
	assert(e);
	return e->dest;
}

GrB_Matrix Edge_GetMatrix(Edge *e) {
	assert(e);

	// Retrieve matrix from graph if edge matrix isn't set.
	if(!e->mat) {
		GraphContext *gc = GraphContext_GetFromTLS();
		Graph *g = gc->g;

		// Get relation matrix.
		if(e->relationID == GRAPH_UNKNOWN_RELATION) {
			e->mat = Graph_GetZeroMatrix(g);
		} else {
			e->mat = Graph_GetRelationMatrix(g, e->relationID);
		}
	}

	return e->mat;
}

bool Edge_VariableLength(const Edge *e) {
	assert(e);
	return (e->minHops != e->maxHops);
}

void Edge_Reverse(Edge *e) {
	Node *t;
	Node *src = e->src;
	Node *dest = e->dest;

	Node_RemoveOutgoingEdge(src, e);
	Node_RemoveIncomingEdge(dest, e);

	// Swap and reconnect.
	t = e->src;
	e->src = e->dest;
	e->dest = t;
	Node_ConnectNode(e->src, e->dest, e);
}

void Edge_SetSrcNode(Edge *e, Node *src) {
	assert(e && src);
	e->src = src;
	e->srcNodeID = ENTITY_GET_ID(src);
}

void Edge_SetDestNode(Edge *e, Node *dest) {
	assert(e && dest);
	e->dest = dest;
	e->destNodeID = ENTITY_GET_ID(dest);
}

void Edge_SetRelationID(Edge *e, int relationID) {
	assert(e);
	e->relationID = relationID;
}

int Edge_ToString(const Edge *e, char *buff, int buff_len) {
	assert(e && buff);

	int offset = 0;
	offset += snprintf(buff + offset, buff_len - offset, "[");

	if(e->alias)
		offset += snprintf(buff + offset, buff_len - offset, "%s", e->alias);
	if(e->relationship)
		offset += snprintf(buff + offset, buff_len - offset, ":%s", e->relationship);
	if(e->minHops != 1 || e->maxHops != 1) {
		if(e->maxHops == EDGE_LENGTH_INF)
			offset += snprintf(buff + offset, buff_len - offset, "*%u..INF", e->minHops);
		else
			offset += snprintf(buff + offset, buff_len - offset, "*%u..%u", e->minHops,
							   e->maxHops);
	}

	offset += snprintf(buff + offset, buff_len - offset, "]");
	return offset;
}

void Edge_Free(Edge *edge) {
	if(!edge) return;

	if(edge->alias != NULL) free(edge->alias);
	if(edge->relationship != NULL) free(edge->relationship);
	free(edge);
}
