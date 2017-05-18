#ifndef __OP_H__
#define __OP_H__

#include "../../redismodule.h"
#include "../../graph/graph.h"
#include "../../graph/node.h"
#include "../../graph/edge.h"
#include "../../stores/store.h"

#define OP_REQUIRE_NEW_DATA(opRes) (opRes & (OP_DEPLETED | OP_REFRESH)) > 0

typedef enum {
    OP_DEPLETED = 1,
    OP_REFRESH = 2,
    OP_OK = 4,
    OP_ERR = 8,
} OpResult;

struct OpBase;

typedef OpResult (*fpConsume)(struct OpBase*, Graph* graph);
typedef OpResult (*fpReset)(struct OpBase*);
typedef void (*fpFree)(struct OpBase*);

typedef struct  {
    fpConsume next;
    fpReset reset;
    fpFree free;
    char *name;
} OpBase;

#endif