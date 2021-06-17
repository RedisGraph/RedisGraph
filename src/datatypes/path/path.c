/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "./path.h"
#include "../../util/arr.h"
#include "../../util/rmalloc.h"

Path *Path_New(size_t len) {
	Path *path = rm_malloc(sizeof(Path));
	path->edges = array_new(Edge, len);
	path->nodes = array_new(Node, len + 1);
	return path;
}

void Path_AppendNode(Path *p, Node n) {
	p->nodes = array_append(p->nodes, n);
}

void Path_AppendEdge(Path *p, Edge e) {
	p->edges = array_append(p->edges, e);
}

Node *Path_GetNode(const Path *p, int index) {
	ASSERT(index >= 0 && index < Path_NodeCount(p));
	return &p->nodes[index];
}

Edge *Path_GetEdge(const Path *p, int index) {
	ASSERT(index >= 0 && index < Path_EdgeCount(p));
	return &p->edges[index];
}

Node Path_PopNode(Path *p) {
	return array_pop(p->nodes);
}
Edge Path_PopEdge(Path *p) {
	return array_pop(p->edges);
}

size_t Path_NodeCount(const Path *p) {
	return array_len(p->nodes);
}

size_t Path_EdgeCount(const Path *p) {
	return array_len(p->edges);
}

Node Path_Head(Path *p) {
	return p->nodes[array_len(p->nodes) - 1];
}

size_t Path_Len(const Path *p) {
	return Path_EdgeCount(p);
}

bool Path_ContainsNode(const Path *p, Node *n) {
	uint32_t pathDepth = Path_NodeCount(p);
	EntityID nId = ENTITY_GET_ID(n);
	for(int i = 0; i < pathDepth; i++) {
		if(ENTITY_GET_ID(p->nodes + i) == nId) return true;
	}
	return false;
}

Path *Path_Clone(const Path *p) {
	Path *clone = rm_malloc(sizeof(Path));
	array_clone(clone->nodes, p->nodes);
	array_clone(clone->edges, p->edges);
	return clone;
}

void Path_Reverse(Path *p) {
	array_reverse(p->nodes);
	array_reverse(p->edges);
}

void Path_Free(Path *p) {
	array_free(p->nodes);
	array_free(p->edges);
	rm_free(p);
}
