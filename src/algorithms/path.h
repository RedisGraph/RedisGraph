/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#ifndef _PATH_H_
#define _PATH_H_

#include "../graph/entities/node.h"
#include "../graph/entities/edge.h"

typedef struct
{
    Node *nodes;
    Edge *edges;
} Path;

Path Path_new(size_t len);
void Path_appendNode(Path p, Node n);
void Path_appendEdge(Path p, Edge e);
Node Path_popNode(Path p);
Edge Path_popEdge(Path p);
Node Path_head(Path p);
size_t Path_nodeCount(Path p);
size_t Path_edgeCount(Path p);
size_t Path_len(const Path p);
bool Path_empty(const Path p);
bool Path_containsNode(const Path p, Node *n);
bool Path_containsEdge(const Path p, Edge *e);
Path Path_clone(const Path p);
void Path_free(Path p);
void Path_print(Path p);

#endif
