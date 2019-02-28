/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#ifndef __OP_H__
#define __OP_H__

#include "../record.h"
#include "../../redismodule.h"
#include "../../graph/query_graph.h"
#include "../../graph/entities/node.h"
#include "../../graph/entities/edge.h"
#include "../../schema/schema.h"
#include "../../util/vector.h"

#define OP_REQUIRE_NEW_DATA(opRes) (opRes & (OP_DEPLETED | OP_REFRESH)) > 0

typedef enum {
    OPType_AGGREGATE = 0x00,
    OPType_ALL_NODE_SCAN = 0x01,
    OPType_TRAVERSE = 0x02,
    OPType_CONDITIONAL_TRAVERSE = 0x04,
    OPType_CONDITIONAL_VAR_LEN_TRAVERSE = 0x08,
    OPType_FILTER = 0x10,
    OPType_NODE_BY_LABEL_SCAN = 0x20,
    OPType_INDEX_SCAN = 0x40,
    OPType_ProduceResults = 0x80,
    OPType_RESULTS = 0x100,
    OPType_CREATE = 0x200,
    OPType_UPDATE = 0x400,
    OPType_DELETE = 0x800,
    OPType_CARTESIAN_PRODUCT = 0x1000,
    OPType_MERGE = 0x2000,
    OPType_UNWIND = 0x4000,
    OPType_SORT = 0x8000,
    OPType_PROJECT = 0x10000,
    OPType_SKIP = 0x20000,
    OPType_LIMIT = 0x40000,
    OPType_DISTINCT = 0x80000,
} OPType;

#define OP_SCAN (OPType_ALL_NODE_SCAN | OPType_NODE_BY_LABEL_SCAN | OPType_INDEX_SCAN)

typedef enum {
    OP_DEPLETED = 1,
    OP_REFRESH = 2,
    OP_OK = 4,
    OP_ERR = 8,
} OpResult;

struct OpBase;

typedef OpResult (*fpInit)(struct OpBase*);
typedef Record (*fpConsume)(struct OpBase*);
typedef OpResult (*fpReset)(struct OpBase*);
typedef void (*fpFree)(struct OpBase*);

struct OpBase {
    OPType type;                // Type of operation
    fpInit init;                // Called once before execution.
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
