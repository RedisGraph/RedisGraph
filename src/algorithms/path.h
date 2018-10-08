/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#ifndef _PATH_H_
#define _PATH_H_

#include "../graph/entities/edge.h"

typedef Edge** Path;

Path Path_new(size_t len);
Path Path_append(Path p, Edge *e);
Edge* Path_pop(Path p);
size_t Path_len(const Path p);
Path Path_clone(const Path p);
void Path_free(Path p);
void Path_print(Path p);

#endif
