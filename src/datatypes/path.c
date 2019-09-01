/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "./path.h"
#include "../util/arr.h"

Path Path_new(size_t len) {
	Path path;
	path.edges = array_new(Edge, len);
	path.nodes = array_new(Node, len);
	return path;
}

void Path_appendNode(Path *p, Node n) {
	p->nodes = array_append(p->nodes, n);
}

void Path_appendEdge(Path *p, Edge e) {
	p->edges = array_append(p->edges, e);
}

Node Path_popNode(Path p) {
	return array_pop(p.nodes);
}
Edge Path_popEdge(Path p) {
	return array_pop(p.edges);
}

size_t Path_nodeCount(Path p) {
	return array_len(p.nodes);
}

size_t Path_edgeCount(Path p) {
	return array_len(p.edges);
}

Node Path_head(Path p) {
	return p.nodes[array_len(p.nodes) - 1];
}

size_t Path_len(const Path p) {
	return Path_edgeCount(p);
}

bool Path_empty(const Path p) {
	return Path_len(p) == 0;
}

bool Path_containsNode(const Path p, Node *n) {
	uint32_t pathDepth = Path_nodeCount(p);
	for(int i = 0; i < pathDepth; i++) {
		if(ENTITY_GET_ID(p.nodes + i) == ENTITY_GET_ID(n)) return true;
	}
	return false;
}

Path Path_clone(const Path p) {
	int pathLen = Path_len(p);
	Path clone = Path_new(pathLen + 1);

	for(int i = 0; i < pathLen; i++) {
		Path_appendNode(&clone, p.nodes[i]);
	}

	return clone;
}

void Path_print(Path p) {
	Node *n = NULL;
	Edge *e = NULL;
	int pathLen = Path_len(p);
	// print node edge
	for(int i = 0; i < pathLen; i++) {
		n = p.nodes + i;
		printf("%llu ", ENTITY_GET_ID(n));
		e = p.edges + i;
		printf("%llu ", ENTITY_GET_ID(e));
	}
	// print last node
	n = p.nodes + pathLen + 1;
	printf("%llu ", ENTITY_GET_ID(n));
}

void Path_free(Path p) {
	array_free(p.nodes);
	array_free(p.edges);
}
