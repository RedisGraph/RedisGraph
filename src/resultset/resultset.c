/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include "resultset.h"
#include "../value.h"
#include "../util/arr.h"
#include "../util/rmalloc.h"
#include "../query_executor.h"
#include "../grouping/group_cache.h"
#include "../arithmetic/aggregate.h"

/* Checks if we've already seen given records
 * Returns 1 if the string did not exist otherwise 0. */
int __encounteredRecord(ResultSet *set, const ResultSetRecord *record) {
    char *str = NULL;
    size_t len = 0;
    len = ResultSetRecord_ToString(record, &str, &len);

    // Returns 1 if the string did NOT exist otherwise 0
    int newRecord = TrieMap_Add(set->trie, str, len, NULL, NULL);
    free(str);
    return !newRecord;
}

void _ResultSet_ReplayHeader(const ResultSet *set, const ResultSetHeader *header) {    
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

void _ResultSet_ReplayRecord(ResultSet *s, const ResultSetRecord* r) {
    // Skip record.
    if(s->skipped < s->skip) {
        s->skipped++;
        return;
    }

    char value[2048] = {0};
    RedisModule_ReplyWithArray(s->ctx, r->len);

    for(int i = 0; i < r->len; i++) {
        int written = SIValue_ToString(r->values[i], value, 2048);
        RedisModule_ReplyWithStringBuffer(s->ctx, value, written);
    }
}

// Prepare replay.
void _ResultSet_SetupReply(ResultSet *set) {
    // resultset + statistics, in that order.
    RedisModule_ReplyWithArray(set->ctx, 2);

    if(set->streaming) {
        // We don't know at this point the number of records, we're about to return.
        RedisModule_ReplyWithArray(set->ctx, REDISMODULE_POSTPONED_ARRAY_LEN);
        if(set->header) {
            /* Replay with table header. */
            _ResultSet_ReplayHeader(set, set->header);
        }
    }
}

// Checks to see if resultset can acecpt additional records.
bool ResultSet_Full(const ResultSet* set) {
    return (set->limit != RESULTSET_UNLIMITED && set->recordCount >= set->limit);
}

// Add record to response buffer.
void _ResultSet_StreamRecord(ResultSet *set, const ResultSetRecord *record) {
    _ResultSet_ReplayRecord(set, record);
}

void _ResultSet_ReplayStats(RedisModuleCtx* ctx, ResultSet* set) {
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

Column* NewColumn(const char *name, const char *alias) {
    Column* column = malloc(sizeof(Column));
    column->name = NULL;
    column->alias = NULL;

    if(name != NULL) {
        column->name = strdup(name);
    }

    if(alias != NULL) {
        column->alias = strdup(alias);
    }

    return column;
}

ResultSetHeader* NewResultSetHeader(const AST *ast) {
    if(!ast->returnNode) return NULL;

    ResultSetHeader* header = rm_malloc(sizeof(ResultSetHeader));
    header->columns_len = 0;
    header->columns = NULL;

    if(ast->returnNode != NULL) {
        header->columns_len = array_len(ast->returnNode->returnElements);
        header->columns = rm_malloc(sizeof(Column*) * header->columns_len);
    }

    for(int i = 0; i < header->columns_len; i++) {
        AST_ReturnElementNode* returnElementNode = ast->returnNode->returnElements[i];

        AR_ExpNode* ar_exp = AR_EXP_BuildFromAST(ast, returnElementNode->exp);

        char* column_name;
        AR_EXP_ToString(ar_exp, &column_name);

        Column* column = NewColumn(column_name, returnElementNode->alias);
        rm_free(column_name);
        AR_EXP_Free(ar_exp);

        header->columns[i] = column;
    }

    return header;
}

ResultSet* NewResultSet(AST* ast, RedisModuleCtx *ctx) {
    ResultSet* set = (ResultSet*)malloc(sizeof(ResultSet));
    set->ctx = ctx;
    set->trie = NULL;
    set->limit = RESULTSET_UNLIMITED;
    set->skip = (ast->skipNode) ? ast->skipNode->skip : 0;
    set->skipped = 0;
    set->distinct = (ast->returnNode && ast->returnNode->distinct);
    set->recordCount = 0;    
    set->header = NewResultSetHeader(ast);
    set->bufferLen = 2048;
    set->buffer = malloc(set->bufferLen);
    set->streaming = set->header;
    set->stats.labels_added = 0;
    set->stats.nodes_created = 0;
    set->stats.properties_set = 0;
    set->stats.relationships_created = 0;
    set->stats.nodes_deleted = 0;
    set->stats.relationships_deleted = 0;

    if(ast->limitNode != NULL) {
        // Account for skipped records.
        set->limit = set->skip + ast->limitNode->limit;
    }

    if(set->distinct) {
        set->trie = NewTrieMap();
    }

    _ResultSet_SetupReply(set);

    set->records = NewVector(ResultSetRecord*, set->limit);

    return set;
}

bool ResultSet_Limited(const ResultSet* set) {
    return (set && set->limit != RESULTSET_UNLIMITED);
}

int ResultSet_AddRecord(ResultSet* set, ResultSetRecord* record) {
    if(ResultSet_Full(set)) {
        return RESULTSET_FULL;
    }
    
    /* TODO: Incase of an aggregated query, there's no need to distinct check */
    /* groups are already distinct by key. */
    if(set->distinct && __encounteredRecord(set, record)) {
        /* TODO: indicate we've skipped record. */
        return RESULTSET_OK;
    }
    
    if(set->streaming) {
        set->recordCount++;
        _ResultSet_StreamRecord(set, record);
        ResultSetRecord_Free(record);
        return RESULTSET_OK;
    }

    /*There's room for record. */
    Vector_Push(set->records, record);
    set->recordCount++;
    return RESULTSET_OK;
}

void ResultSet_Replay(ResultSet* set) {
    // Ensure that we're returning a valid number of records.
    size_t resultset_size;
    if (set->skip < set->recordCount) {
      resultset_size = set->recordCount - set->skip;
    } else {
      resultset_size = 0;
    }

    if(set->header) resultset_size++;

    if(set->streaming) {
        /* ResultSet been streamed, all that's left to do
         * is let redis know how many records been sent
         * and append some statistics. */
        RedisModule_ReplySetArrayLength(set->ctx, resultset_size);
        _ResultSet_ReplayStats(set->ctx, set);
        return;
    }
    
    RedisModule_ReplyWithArray(set->ctx, resultset_size);

    if(set->header) {
        /* Replay with table header. */
        _ResultSet_ReplayHeader(set, set->header);
    }

    if(set->recordCount) {
        size_t str_record_len = 0;
        size_t str_record_cap = 2048;
        char *str_record = malloc(str_record_cap);

        for(int i = 0; i < Vector_Size(set->records); i++) {
            ResultSetRecord* record = NULL;
            Vector_Get(set->records, i, &record);
            _ResultSet_ReplayRecord(set, record);
        }
        free(str_record);
    }

    _ResultSet_ReplayStats(set->ctx, set);
}

void Column_Free(Column* column) {
    if(column->name != NULL) {
        free(column->name);
    }
    if(column->alias != NULL) {
        free(column->alias);
    }
    free(column);
}

void ResultSetHeader_Free(ResultSetHeader* header) {
    if(!header) return;

    for(int i = 0; i < header->columns_len; i++) Column_Free(header->columns[i]);

    if(header->columns != NULL) {
        rm_free(header->columns);
    }

    rm_free(header);
}

void ResultSet_Free(ResultSet *set) {
    if(!set) return;
    /* Free each record */
    for(int i = 0; i < Vector_Size(set->records); i++) {
        ResultSetRecord *record;
        Vector_Get(set->records, i, &record);
        ResultSetRecord_Free(record);
        record = NULL;
    }
    Vector_Free(set->records);

    if(set->trie != NULL) {
        TrieMap_Free(set->trie, TrieMap_NOP_CB);
    }

    free(set->buffer);
    ResultSetHeader_Free(set->header);
    free(set);
}
