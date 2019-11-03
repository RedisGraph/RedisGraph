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
#include "../../query_ctx.h"

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
		Graph *g = QueryCtx_GetGraph();

		// Get relation matrix.
		if(e->relationID == GRAPH_UNKNOWN_RELATION) {
			e->mat = Graph_GetZeroMatrix(g);
		} else {
			e->mat = Graph_GetRelationMatrix(g, e->relationID);
		}
	}

	return e->mat;
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

void Edge_ToString(const Edge *e, char **buffer, size_t *bufferLen, size_t *bytesWritten,
				   GraphEntityStringFromat format) {
	GraphEntity_ToString((const GraphEntity *)e, buffer, bufferLen, bytesWritten, format,
						 GETYPE_EDGE);
}

void Edge_Free(Edge *edge) {
	if(!edge) return;

	free(edge);
}

