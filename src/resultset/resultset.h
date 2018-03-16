#ifndef __GRAPH_RESULTSET_H__
#define __GRAPH_RESULTSET_H__

#include "record.h"
#include "resultset_header.h"
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
    AST_QueryExpressionNode* ast;
    ResultSetHeader* header;    /* Describes how records should look like. */
    int aggregated;             /* Rather or not this is an aggregated result set. */
    int ordered;                /* Rather or not this result set is ordered. */
    int direction;              /* Sort direction ASC/DESC. */
    int limit;                  /* Max number of records in result-set. */
    int distinct;               /* Rather or not each record is unique. */
    int labels_added;           /* Number of labels added as part of a create query. */
    int nodes_created;          /* Number of nodes created as part of a create query. */
    int properties_set;         /* Number of properties created as part of a create query. */
    int relationships_created;  /* Number of edges created as part of a create query. */
    int nodes_deleted;          /* Number of nodes removed as part of a delete query.*/
    int relationships_deleted;  /* Number of edges removed as part of a delete query.*/
} ResultSet;

ResultSet* NewResultSet(AST_QueryExpressionNode* ast);

int ResultSet_AddRecord(ResultSet* set, Record *record);

void ResultSet_Free(RedisModuleCtx* ctx, ResultSet* set);

int ResultSet_Full(const ResultSet* set);

void ResultSet_Replay(RedisModuleCtx* ctx, ResultSet* set);

#endif