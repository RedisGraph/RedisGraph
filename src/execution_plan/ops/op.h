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
#include "../../util/arr.h"

#define OP_REQUIRE_NEW_DATA(opRes) (opRes & (OP_DEPLETED | OP_REFRESH)) > 0

typedef enum {
    OPType_AGGREGATE = 0,
    OPType_ALL_NODE_SCAN = (1<<0),
    OPType_TRAVERSE = (1<<1),
    OPType_CONDITIONAL_TRAVERSE = (1<<2),
    OPType_CONDITIONAL_VAR_LEN_TRAVERSE = (1<<3),
    OPType_FILTER = (1<<4),
    OPType_NODE_BY_LABEL_SCAN = (1<<5),
    OPType_INDEX_SCAN = (1<<6),
    OPType_ProduceResults = (1<<7),
    OPType_RESULTS = (1<<8),
    OPType_CREATE = (1<<9),
    OPType_UPDATE = (1<<10),
    OPType_DELETE = (1<<11),
    OPType_CARTESIAN_PRODUCT = (1<<12),
    OPType_MERGE = (1<<13),
    OPType_UNWIND = (1<<14),
    OPType_SORT = (1<<15),
    OPType_PROJECT = (1<<16),
    OPType_SKIP = (1<<17),
    OPType_LIMIT = (1<<18),
    OPType_DISTINCT = (1<<19),
    OPType_EXPAND_INTO = (1<<20),
    OPType_NODE_BY_ID_SEEK = (1<<21),
    OPType_HANDOFF = (1<<22),
} OPType;

#define OP_SCAN (OPType_ALL_NODE_SCAN | OPType_NODE_BY_LABEL_SCAN | OPType_INDEX_SCAN | OPType_NODE_BY_ID_SEEK)

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
    uint *modifies;             // List of Record indices this op modifies.
    struct OpBase **children;   // Child operations.
    int childCount;             // Number of children.
    struct OpBase *parent;      // Parent operations.
};
typedef struct OpBase OpBase;

void OpBase_Init(OpBase *op);
void OpBase_Reset(OpBase *op);
void OpBase_Free(OpBase *op);

#endif
