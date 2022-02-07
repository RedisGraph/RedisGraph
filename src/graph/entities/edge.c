/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include <stdio.h>
#include <stdlib.h>

#include "edge.h"
#include "graph_entity.h"
#include "../graphcontext.h"
#include "../../query_ctx.h"

NodeID Edge_GetSrcNodeID
(
	const Edge *edge
) {
	ASSERT(edge);
	return edge->srcNodeID;
}

NodeID Edge_GetDestNodeID
(
	const Edge *edge
) {
	ASSERT(edge);
	return edge->destNodeID;
}

int Edge_GetRelationID
(
	const Edge *edge
) {
	ASSERT(edge);
	return edge->relationID;
}

Node *Edge_GetSrcNode
(
	Edge *e
) {
	ASSERT(e);
	return e->src;
}

Node *Edge_GetDestNode
(
	Edge *e
) {
	ASSERT(e);
	return e->dest;
}

RG_Matrix Edge_GetMatrix
(
	Edge *e
) {
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

void Edge_SetSrcNode
(
	Edge *e,
	Node *src
) {
	ASSERT(e && src);
	e->src = src;
	e->srcNodeID = ENTITY_GET_ID(src);
}

void Edge_SetDestNode
(
	Edge *e,
	Node *dest
) {
	ASSERT(e && dest);
	e->dest = dest;
	e->destNodeID = ENTITY_GET_ID(dest);
}

void Edge_SetRelationID
(
	Edge *e,
	int relationID
) {
	ASSERT(e);
	e->relationID = relationID;
}

void Edge_ToString
(
	const Edge *e,
	char **buffer,
	size_t *bufferLen,
	size_t *bytesWritten,
	GraphEntityStringFromat format
) {
	GraphEntity_ToString((const GraphEntity *)e, buffer, bufferLen, bytesWritten, format, GETYPE_EDGE);
}

void Edge_Clone
(
	const Edge *e,
	Edge *clone
) {
	ASSERT(e != NULL);
	ASSERT(clone != NULL);

	clone->id                     = e->id;
	clone->srcNodeID              = e->srcNodeID;
	clone->destNodeID             = e->destNodeID;
	clone->relationID             = e->relationID;
	clone->attributes             = rm_malloc(sizeof(AttributeSet));
	clone->attributes->prop_count = ENTITY_PROP_COUNT(e);
	clone->attributes->properties = rm_malloc(sizeof(EntityProperty) * ENTITY_PROP_COUNT(e));
	for (uint i = 0; i < ENTITY_PROP_COUNT(e); i++) {
		EntityProperty *prop       = ENTITY_PROPS(e) + i;
		EntityProperty *clone_prop = ENTITY_PROPS(clone) + i;
		clone_prop->id             = prop->id;
		clone_prop->value          = SI_CloneValue(prop->value);
	}
}

void Edge_Free
(
	Edge *edge
) {
	if(!edge) return;

	free(edge);
}

