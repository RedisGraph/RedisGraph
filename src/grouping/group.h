/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#ifndef GROUP_H_
#define GROUP_H_

#include "../util/vector.h"
#include "../arithmetic/agg_ctx.h"

typedef struct {
    int key_count;
    SIValue* keys;
    Vector* aggregationFunctions;   /* Vector of AR_ExpNode*, where the root is an aggregation function. */
} Group;

/* Creates a new group */
Group* NewGroup(int key_count, SIValue* keys, Vector* funcs);

void FreeGroup(Group* group);

#endif
