#ifndef __GRAPH_RECORD_H__
#define __GRAPH_RECORD_H__

#include "../redismodule.h"
#include "../parser/ast.h"
#include "../rmutil/vector.h"
#include "../grouping/group.h"
#include "../graph/graph.h"

typedef struct {
    Vector* elements;   // Desired elements specified in return clause
    Vector* orderBys;   // Values to sort on
} Record;

// Creates a new result-set record from graph.
Record* Record_FromGraph(RedisModuleCtx *ctx, const QueryExpressionNode* ast, const Graph* g);

// Creates a new result-set record from an aggregated group.
Record* Record_FromGroup(RedisModuleCtx* ctx, const QueryExpressionNode* ast, const Group* group);

char* Record_ToString(const Record* record);

// Frees given record.
void Record_Free(RedisModuleCtx* ctx, Record* r);

#endif