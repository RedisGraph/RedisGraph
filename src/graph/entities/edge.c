/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include <stdio.h>
#include <stdlib.h>

#include "edge.h"
#include "graph_entity.h"
#include "../graphcontext.h"
#include "../../query_ctx.h"

NodeID Edge_GetSrcNodeID(const Edge *edge) {
	ASSERT(edge);
	return edge->srcNodeID;
}

NodeID Edge_GetDestNodeID(const Edge *edge) {
	ASSERT(edge);
	return edge->destNodeID;
}

int Edge_GetRelationID(const Edge *edge) {
	ASSERT(edge);
	return edge->relationID;
}

Node *Edge_GetSrcNode(Edge *e) {
	ASSERT(e);
	return e->src;
}

Node *Edge_GetDestNode(Edge *e) {
	ASSERT(e);
	return e->dest;
}

RG_Matrix Edge_GetMatrix(Edge *e) {
	ASSERT(e);

	// retrieve matrix from graph if edge matrix isn't set
	if(!e->mat) {
		Graph *g = QueryCtx_GetGraph();

		// get relation matrix
		if(e->relationID == GRAPH_UNKNOWN_RELATION) {
			e->mat = Graph_GetZeroMatrix(g);
		} else {
			e->mat = Graph_GetRelationMatrix(g, e->relationID, false);
		}
	}

	return e->mat;
}

void Edge_SetSrcNode(Edge *e, Node *src) {
	ASSERT(e && src);
	e->src = src;
	e->srcNodeID = ENTITY_GET_ID(src);
}

void Edge_SetDestNode(Edge *e, Node *dest) {
	ASSERT(e && dest);
	e->dest = dest;
	e->destNodeID = ENTITY_GET_ID(dest);
}

void Edge_SetRelationID(Edge *e, int relationID) {
	ASSERT(e);
	e->relationID = relationID;
}

void Edge_ToString(const Edge *e, char **buffer, size_t *bufferLen, size_t *bytesWritten,
				   GraphEntityStringFromat format) {
	GraphEntity_ToString((const GraphEntity *)e, buffer, bufferLen, bytesWritten, format, GETYPE_EDGE);
}

void Edge_Free(Edge *edge) {
	if(!edge) return;

	free(edge);
}

