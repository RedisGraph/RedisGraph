#ifndef __GRAPH_RECORD_H__
#define __GRAPH_RECORD_H__

#include "../redismodule.h"
#include "../parser/ast.h"
#include "../rmutil/vector.h"
#include "../grouping/group.h"
#include "../graph/graph.h"

typedef struct {
    Vector* values; // Vector of SIValue*
} Record;

/* Creates a new record which will hold len elements. */
Record* NewRecord(size_t len);

/* Creates a new record from graph. */
Record* Record_FromGraph(RedisModuleCtx *ctx, const AST_QueryExpressionNode *ast, const Graph *g);

/* Creates a new record from an aggregated group. */
Record* Record_FromGroup(RedisModuleCtx *ctx, const AST_QueryExpressionNode *ast, const Group *g);

/* Get a string representation of record. */
size_t Record_ToString(const Record *record, char **record_str);

/* Compares the two records, using the values at given indeces
 * Returns 1 if A >= B, -1 if A <= B, 0 if A = B */
int Records_Compare(const Record *A, const Record *B, int *compareIndices, size_t compareIndicesLen);

/* Frees given record. */
void Record_Free(Record *r);

#endif