/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#ifndef __OP_H__
#define __OP_H__

#include "../../redismodule.h"
#include "../../graph/query_graph.h"
#include "../../graph/node.h"
#include "../../graph/edge.h"
#include "../../stores/store.h"
#include "../../rmutil/vector.h"

#define OP_REQUIRE_NEW_DATA(opRes) (opRes & (OP_DEPLETED | OP_REFRESH)) > 0

typedef enum {
OPType_AGGREGATE,
OPType_ALL_NODE_SCAN,
OPType_TRAVERSE,
OPType_CONDITIONAL_TRAVERSE,
OPType_EXPAND_ALL,
OPType_EXPAND_INTO,
OPType_FILTER,
OPType_NODE_BY_LABEL_SCAN,
OPType_PRODUCE_RESULTS,
OPType_CREATE,
OPType_UPDATE,
OPType_DELETE,
OPType_CARTESIAN_PRODUCT,
OPType_MERGE
} OPType;

typedef enum {
    OP_DEPLETED = 1,
    OP_REFRESH = 2,
    OP_OK = 4,
    OP_ERR = 8,
} OpResult;

struct OpBase;

typedef OpResult (*fpConsume)(struct OpBase*, QueryGraph* graph);
typedef OpResult (*fpReset)(struct OpBase*);
typedef void (*fpFree)(struct OpBase*);

struct OpBase {
    OPType type;
    fpConsume consume;
    fpReset reset;
    fpFree free;
    char *name;
    Vector *modifies;   // List of aliases, this op modifies.
};
typedef struct OpBase OpBase;

#endif