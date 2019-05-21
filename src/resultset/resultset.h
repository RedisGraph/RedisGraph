/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#ifndef __GRAPH_RESULTSET_H__
#define __GRAPH_RESULTSET_H__

#include "resultset_header.h"
#include "resultset_formatters.h"
#include "resultset_statistics.h"
#include "../parser/ast.h"
#include "../redismodule.h"
#include "../util/vector.h"
#include "../execution_plan/record.h"
#include "../util/triemap/triemap.h"
#include "../util/arr.h"

#define RESULTSET_UNLIMITED 0
#define RESULTSET_OK 1
#define RESULTSET_FULL 0

typedef struct {
    RedisModuleCtx *ctx;
    GraphContext *gc;           /* Context used for mapping attribute strings and IDs */
    ResultSetHeader *header;    /* Describes how records should look like. */
    bool distinct;              /* Whether or not each record is unique. */
    bool compact;               /* Whether records should be returned in compact form. */
    size_t recordCount;         /* Number of records introduced. */
    char *buffer;               /* Reusable buffer for record streaming. */
    size_t bufferLen;           /* Size of buffer in bytes. */
    ResultSetStatistics stats;  /* ResultSet statistics. */
    EmitRecordFunc EmitRecord;  /* Function pointer to Record reply routine. */
    Record *records;            /* Dynamic array to store records, for re-transimitting*/
} ResultSet;

typedef struct {
    ResultSet *resultSet;
    RedisModuleBlockedClient *bc;
    double tic[2];
} ResultSet_RetransmitParams;

ResultSet *
NewResultSet(AST *ast, RedisModuleCtx *ctx, bool compact);

void ResultSet_ReplyWithPreamble(ResultSet *set, AST **ast);

int ResultSet_AddRecord(ResultSet* set, Record r);

void ResultSet_Replay(ResultSet* set);

void ResultSet_Retransmit(ResultSet_RetransmitParams *retransmitParams);

void ResultSet_Free(ResultSet* set);

ResultSet_RetransmitParams *ResultSet_RetransmitParams_New();
void ResultSet_RetransmitParams_Free(ResultSet_RetransmitParams *retransmitParams);

#endif
