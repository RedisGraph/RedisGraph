/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#ifndef _PATH_H_
#define _PATH_H_

#include "../graph/entities/node.h"

typedef Node* Path;

Path Path_new(size_t len);
Path Path_append(Path p, Node n);
Node Path_pop(Path p);
Node Path_head(Path p);
size_t Path_len(const Path p);
bool Path_empty(const Path p);
bool Path_containsNode(const Path p, Node *n);
Path Path_clone(const Path p);
void Path_free(Path p);
void Path_print(Path p);

#endif
