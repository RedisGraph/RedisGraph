/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#ifndef __OP_H__
#define __OP_H__

#include "../record.h"
#include "../../redismodule.h"
#include "../../graph/query_graph.h"
#include "../../graph/entities/node.h"
#include "../../graph/entities/edge.h"
#include "../../stores/store.h"
#include "../../util/vector.h"

#define OP_REQUIRE_NEW_DATA(opRes) (opRes & (OP_DEPLETED | OP_REFRESH)) > 0

typedef enum {
OPType_AGGREGATE,
OPType_ALL_NODE_SCAN,
OPType_TRAVERSE,
OPType_CONDITIONAL_TRAVERSE,
OPType_CONDITIONAL_VAR_LEN_TRAVERSE,
OPType_FILTER,
OPType_NODE_BY_LABEL_SCAN,
OPType_INDEX_SCAN,
OPType_PRODUCE_RESULTS,
OPType_CREATE,
OPType_UPDATE,
OPType_DELETE,
OPType_CARTESIAN_PRODUCT,
OPType_MERGE,
OPType_UNWIND,
OPType_SORT,
OPType_PROJECT,
} OPType;

typedef enum {
    OP_DEPLETED = 1,
    OP_REFRESH = 2,
    OP_OK = 4,
    OP_ERR = 8,
} OpResult;

struct OpBase;

// typedef OpResult (*fpConsume)(struct OpBase*, Record r);
typedef Record (*fpConsume)(struct OpBase*);
typedef OpResult (*fpReset)(struct OpBase*);
typedef void (*fpFree)(struct OpBase*);

struct OpBase {
    OPType type;                // Type of operation
    fpConsume consume;          // Produce next record.
    fpReset reset;              // Reset operation state.
    fpFree free;                // Free operation.
    char *name;                 // Operation name.
    Vector *modifies;           // List of aliases, this op modifies.
    struct OpBase **children;   // Child operations.
    int childCount;             // Number of children.
    struct OpBase *parent;      // Parent operations.
};
typedef struct OpBase OpBase;

void OpBase_Init(OpBase *op);
void OpBase_Reset(OpBase *op);
void OpBase_Free(OpBase *op);

#endif
