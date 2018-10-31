/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "edge.h"
#include "graph_entity.h"

Edge* Edge_New(EdgeID id, Node *src, Node *dest, const char *relationship, const char *alias) {
	assert(src && dest);

	Edge* e = (Edge*)calloc(1, sizeof(Edge));
	e->id = id;
	Edge_SetSrcNode(e, src);
	Edge_SetDestNode(e, dest);
	e->prop_count = 0;

	if(relationship != NULL) {
		e->relationship = strdup(relationship);
	}
	if(alias != NULL) {
		e->alias = strdup(alias);
	}

	return e;
}

NodeID Edge_GetSrcNodeID(const Edge* edge) {
	assert(edge);
	return edge->edgeDesc.srcId;
}

NodeID Edge_GetDestNodeID(const Edge* edge) {
	assert(edge);
	return edge->edgeDesc.destId;
}

int Edge_GetRelationID(const Edge *edge) {
	assert(edge);
	return edge->edgeDesc.relationId;
}

Node* Edge_GetSrcNode(Edge *e) {
	assert(e);
	return e->src;
}

Node* Edge_GetDestNode(Edge *e) {
	assert(e);
	return e->dest;
}

void Edge_SetSrcNode(Edge *e, Node *src) {
	assert(e && src);
	e->src = src;
	e->edgeDesc.srcId = src->id;
}

void Edge_SetDestNode(Edge *e, Node *dest) {
	assert(e && dest);
	e->dest = dest;
	e->edgeDesc.destId = dest->id;
}

void Edge_SetRelationID(Edge *e, int relationId) {
	assert(e);
	e->edgeDesc.relationId = relationId;
}

void Edge_Add_Properties(Edge *edge, int prop_count, char **keys, SIValue *values) {
	GraphEntity_Add_Properties((GraphEntity*)edge, prop_count, keys, values);
}

SIValue* Edge_Get_Property(const Edge *edge, const char* key) {
	return GraphEntity_Get_Property((GraphEntity*)edge, key);
}

void Edge_Free(Edge* edge) {
	if(!edge) return;

	FreeGraphEntity((GraphEntity*)edge);

	if(edge->alias != NULL) free(edge->alias);
	if(edge->relationship != NULL) free(edge->relationship);
}
