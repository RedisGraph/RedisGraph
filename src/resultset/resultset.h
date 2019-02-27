/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "redismodule.h"
#include "resultset_header.h"
#include "resultset_statistics.h"
#include "parser/ast.h"
#include "execution_plan/record.h"
#include "util/vector.h"
#include "util/triemap/triemap.h"

#define RESULTSET_UNLIMITED 0
#define RESULTSET_OK 1
#define RESULTSET_FULL 0

typedef struct {
    RedisModuleCtx *ctx;
    ResultSetHeader *header;    // Describes how records should look like
    int limit;                  // Max number of records in result-set
    bool distinct;              // Rather or not each record is unique
    size_t recordCount;         // Number of records introduced
    char *buffer;               // Reusable buffer for record streaming
    size_t bufferLen;           // Size of buffer in bytes
    ResultSetStatistics stats;  // ResultSet statistics
    size_t skip;                // Number of records to skip
    size_t skipped;             // Number of records been skipped
} ResultSet;

ResultSet* NewResultSet(AST* ast, RedisModuleCtx *ctx);

void ResultSet_CreateHeader(ResultSet* set);

bool ResultSet_Limited(const ResultSet* set);

bool ResultSet_Full(const ResultSet* set);

int ResultSet_AddRecord(ResultSet* set, Record r);

void ResultSet_Replay(ResultSet* set);

void ResultSet_Free(ResultSet* set);
