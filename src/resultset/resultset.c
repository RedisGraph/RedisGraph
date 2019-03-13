/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "resultset.h"
#include "../value.h"
#include "../util/arr.h"
#include "../util/rmalloc.h"
#include "../query_executor.h"
#include "../grouping/group_cache.h"
#include "../arithmetic/aggregate.h"

static void _ResultSet_ReplayHeader(const ResultSet *set, const ResultSetHeader *header) {    
    RedisModule_ReplyWithArray(set->ctx, header->columns_len);
    for(int i = 0; i < header->columns_len; i++) {
        Column *c = header->columns[i];
        if(c->alias) {
            RedisModule_ReplyWithStringBuffer(set->ctx, c->alias, strlen(c->alias));
        } else {
            RedisModule_ReplyWithStringBuffer(set->ctx, c->name, strlen(c->name));
        }
    }
}

// Prepare replay.
static void _ResultSet_SetupReply(ResultSet *set) {
    // resultset + statistics, in that order.
    RedisModule_ReplyWithArray(set->ctx, 2);

    // We don't know at this point the number of records, we're about to return.
    RedisModule_ReplyWithArray(set->ctx, REDISMODULE_POSTPONED_ARRAY_LEN);
}

static void _ResultSet_ReplayStats(RedisModuleCtx* ctx, ResultSet* set) {
    char buff[512] = {0};
    size_t resultset_size = 1; /* query execution time. */
    int buflen;

    if(set->stats.labels_added > 0) resultset_size++;
    if(set->stats.nodes_created > 0) resultset_size++;
    if(set->stats.properties_set > 0) resultset_size++;
    if(set->stats.relationships_created > 0) resultset_size++;
    if(set->stats.nodes_deleted > 0) resultset_size++;
    if(set->stats.relationships_deleted > 0) resultset_size++;

    RedisModule_ReplyWithArray(ctx, resultset_size);

    if(set->stats.labels_added > 0) {
        buflen = sprintf(buff, "Labels added: %d", set->stats.labels_added);
        RedisModule_ReplyWithStringBuffer(ctx, (const char*)buff, buflen);
    }

    if(set->stats.nodes_created > 0) {
        buflen = sprintf(buff, "Nodes created: %d", set->stats.nodes_created);
        RedisModule_ReplyWithStringBuffer(ctx, (const char*)buff, buflen);
    }

    if(set->stats.properties_set > 0) {
        buflen = sprintf(buff, "Properties set: %d", set->stats.properties_set);
        RedisModule_ReplyWithStringBuffer(ctx, (const char*)buff, buflen);
    }

    if(set->stats.relationships_created > 0) {
        buflen = sprintf(buff, "Relationships created: %d", set->stats.relationships_created);
        RedisModule_ReplyWithStringBuffer(ctx, (const char*)buff, buflen);
    }

    if(set->stats.nodes_deleted > 0) {
        buflen = sprintf(buff, "Nodes deleted: %d", set->stats.nodes_deleted);
        RedisModule_ReplyWithStringBuffer(ctx, (const char*)buff, buflen);
    }

    if(set->stats.relationships_deleted > 0) {
        buflen = sprintf(buff, "Relationships deleted: %d", set->stats.relationships_deleted);
        RedisModule_ReplyWithStringBuffer(ctx, (const char*)buff, buflen);
    }
}

static Column* _NewColumn(char *name, char *alias) {
    Column* column = rm_malloc(sizeof(Column));
    column->name = name;
    column->alias = alias;
    return column;
}

void static _Column_Free(Column* column) {
    /* No need to free alias,
     * it will be freed as part of AST_Free. */
    rm_free(column->name);
    rm_free(column);
}

void ResultSet_CreateHeader(ResultSet *resultset, const AST *ast) {    
    if(!ast->returnNode) return;
    assert(resultset->header == NULL && resultset->recordCount == 0);

    ResultSetHeader* header = rm_malloc(sizeof(ResultSetHeader));
    header->columns_len = 0;
    header->columns = NULL;

    if(ast->returnNode != NULL) {
        header->columns_len = array_len(ast->returnNode->returnElements);
        header->columns = rm_malloc(sizeof(Column*) * header->columns_len);
    }

    for(int i = 0; i < header->columns_len; i++) {
        AST_ReturnElementNode *returnElementNode = ast->returnNode->returnElements[i];

        AR_ExpNode *ar_exp = AR_EXP_BuildFromAST(ast, returnElementNode->exp);

        char *column_name;
        AR_EXP_ToString(ar_exp, &column_name);
        Column *column = _NewColumn(column_name, returnElementNode->alias);
        AR_EXP_Free(ar_exp);

        header->columns[i] = column;
    }

    resultset->header = header;
    /* Replay with table header. */
    _ResultSet_ReplayHeader(resultset, header);
}

static void _ResultSetHeader_Free(ResultSetHeader* header) {
    if(!header) return;

    for(int i = 0; i < header->columns_len; i++) _Column_Free(header->columns[i]);

    if(header->columns != NULL) {
        rm_free(header->columns);
    }

    rm_free(header);
}

ResultSet* NewResultSet(AST* ast, RedisModuleCtx *ctx, bool reply_compact) {
    ResultSet* set = (ResultSet*)malloc(sizeof(ResultSet));
    set->ctx = ctx;
    set->distinct = (ast->returnNode && ast->returnNode->distinct);
    set->recordCount = 0;    
    set->header = NULL;
    set->bufferLen = 2048;
    set->buffer = malloc(set->bufferLen);

    set->stats.labels_added = 0;
    set->stats.nodes_created = 0;
    set->stats.properties_set = 0;
    set->stats.relationships_created = 0;
    set->stats.nodes_deleted = 0;
    set->stats.relationships_deleted = 0;

    set->EmitRecord = ResultSet_SetReplyFormatter(reply_compact);

    _ResultSet_SetupReply(set);

    return set;
}

int ResultSet_AddRecord(ResultSet* set, Record r) {
    set->recordCount++;

    // Output the current record using the defined formatter
    set->EmitRecord(set->ctx, r, set->header->columns_len);

    return RESULTSET_OK;
}

void ResultSet_Replay(ResultSet* set) {
    // Ensure that we're returning a valid number of records.
    size_t resultset_size = set->recordCount;
    if(set->header) resultset_size++;

    RedisModule_ReplySetArrayLength(set->ctx, resultset_size);
    _ResultSet_ReplayStats(set->ctx, set);
}

void ResultSet_Free(ResultSet *set) {
    if(!set) return;

    free(set->buffer);
    if(set->header) _ResultSetHeader_Free(set->header);
    free(set);
}
