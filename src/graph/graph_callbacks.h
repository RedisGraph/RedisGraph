/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include <pthread.h>

#include "entities/node.h"
#include "entities/edge.h"

typedef struct Graph Graph;

typedef void (*NodeCreatedFunc)(const Graph *, Node *);
typedef void (*EdgeCreatedFunc)(const Graph *, Edge *);
typedef void (*NodeDeletedFunc)(const Graph *, Node *, LabelID *, uint);
typedef void (*EdgeDeletedFunc)(const Graph *, Edge *);
typedef void (*NodeUpdatedFunc)(const Graph *, Node *, Attribute_ID, SIValue *);
typedef void (*EdgeUpdatedFunc)(const Graph *, Edge *, Attribute_ID, SIValue *);