/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

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
    RedisModuleCtx *ctx;
    Vector *records;            /* Vector of Records. */
    heap_t *heap;               /* Holds top n records. */
    TrieMap *groups;            /* When aggregating, stores groups by group key. */
    TrieMap *trie;              /* When using distinct, used to identify unique records. */
    ResultSetHeader *header;    /* Describes how records should look like. */
    bool aggregated;            /* Rather or not this is an aggregated result set. */
    bool ordered;               /* Rather or not this result set is ordered. */
    int direction;              /* Sort direction ASC/DESC. */
    int limit;                  /* Max number of records in result-set. */
    bool distinct;              /* Rather or not each record is unique. */
    bool streaming;             /* Streams records back to client. */
    size_t recordCount;         /* Number of records introduced. */
    char *buffer;               /* Reusable buffer for record streaming. */
    size_t bufferLen;           /* Size of buffer in bytes. */
    ResultSetStatistics stats;  /* ResultSet statistics. */
    size_t skip;                /* Number of records to skip . */
    size_t skipped;             /* Number of records been skipped. */
} ResultSet;

ResultSet* NewResultSet(AST_Query* ast, RedisModuleCtx *ctx);

bool ResultSet_Limited(const ResultSet* set);

bool ResultSet_Full(const ResultSet* set);

int ResultSet_AddRecord(ResultSet* set, Record *record);

void ResultSet_Replay(ResultSet* set);

void ResultSet_Free(ResultSet* set);

#endif
