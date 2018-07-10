#ifndef __GRAPH_RESULTSET_H__
#define __GRAPH_RESULTSET_H__

#include "record.h"
#include "resultset_header.h"
#include "resultset_statistics.h"
#include "../parser/ast.h"
#include "../redismodule.h"
#include "../rmutil/vector.h"
#include "../util/heap.h"
#include "../util/triemap/triemap.h"

#define RESULTSET_UNLIMITED 0
#define RESULTSET_OK 1
#define RESULTSET_FULL 0

#define DIR_DESC -1
#define DIR_ASC 1

typedef struct {
    Vector* records;            /* Vector of Records. */
    heap_t* heap;               /* Holds top n records. */
    TrieMap* trie;              /* When using distinct, used to identify unique records. */
    AST_Query* ast;
    ResultSetHeader* header;    /* Describes how records should look like. */
    int aggregated;             /* Rather or not this is an aggregated result set. */
    int ordered;                /* Rather or not this result set is ordered. */
    int direction;              /* Sort direction ASC/DESC. */
    int limit;                  /* Max number of records in result-set. */
    int distinct;               /* Rather or not each record is unique. */
    ResultSetStatistics stats;  /* ResultSet statistics. */
} ResultSet;

ResultSet* NewResultSet(AST_Query* ast);

int ResultSet_AddRecord(ResultSet* set, Record *record);

void ResultSet_Free(RedisModuleCtx* ctx, ResultSet* set);

int ResultSet_Full(const ResultSet* set);

void ResultSet_Replay(RedisModuleCtx* ctx, ResultSet* set);

#endif