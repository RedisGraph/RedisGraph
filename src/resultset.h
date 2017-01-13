#ifndef __GRAPH_RESULTSET_H__
#define __GRAPH_RESULTSET_H__

#include "parser/ast.h"
#include "redismodule.h"
#include "rmutil/vector.h"

#define RESULTSET_UNLIMITED 0
#define RESULTSET_OK 1
#define RESULTSET_FULL 0

typedef struct {
    Vector* records;        // Records forming this result-set
    Vector* returnElements; // Which elements we wish to return
    int aggregated;         // Rather or not this is an aggregated result set
    int limit;              // Max number of records in result-set
} ResultSet;

ResultSet* NewResultSet(const QueryExpressionNode* ast);

int ResultSet_AddRecord(ResultSet* set, const Vector* record);

void ResultSet_Free(RedisModuleCtx* ctx, ResultSet* set);

int ResultSet_Full(const ResultSet* set);

void ResultSet_Replay(RedisModuleCtx* ctx, ResultSet* set);

#endif