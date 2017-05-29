#ifndef __GRAPH_RECORD_H__
#define __GRAPH_RECORD_H__

#include "../redismodule.h"
#include "../parser/ast.h"
#include "../rmutil/vector.h"
#include "../grouping/group.h"
#include "../graph/graph.h"

typedef struct {
    Vector* values; // Vector of *RedisModuleStrings
} Record;

Record* NewRecord(size_t len);

// Creates a new result-set record from graph.
Record* Record_FromGraph(RedisModuleCtx *ctx, const QueryExpressionNode* ast, const Graph* g);

// Creates a new result-set record from an aggregated group.
Record* Record_FromGroup(RedisModuleCtx* ctx, const QueryExpressionNode* ast, const Group* g);

void Record_ToString(const Record* record, char **strRecord);

// Compares the two records, Using the values at given indeces
// Returns 1 if A >= B, -1 if A <= B, 0 if A = B
int Records_Compare(const Record *A, const Record *B, int* compareIndices, size_t compareIndicesLen);

// Frees given record.
void Record_Free(RedisModuleCtx* ctx, Record* r);

#endif