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

/* Redis prints doubles with up to 17 digits of precision, which captures
 * the inaccuracy of many floating-point numbers (such as 0.1).
 * By using the %g format and a precision of 15 significant digits, we avoid many
 * awkward representations like RETURN 0.1 emitting "0.10000000000000001",
 * though we're still subject to many of the typical issues with floating-point error. */
static inline void _ResultSet_ReplyWithRoundedDouble(RedisModuleCtx *ctx, double d) {
    // Get length required to print number
    int len = snprintf(NULL, 0, "%.15g", d);
    char str[len + 1]; // TODO a reusable buffer would be far preferable
    sprintf(str, "%.15g", d);
    // Output string-formatted number
    RedisModule_ReplyWithStringBuffer(ctx, str, len);
}

/* This function handles emitting SIValue types through the Redis RESP protocol.
 * This protocol has unique support for strings, 8-byte integers, and NULL values. */
static void _ResultSet_ReplyWithScalar(RedisModuleCtx *ctx, const SIValue v) {
    // Emit the actual value, then the value type (to facilitate client-side parsing)
    switch (SI_TYPE(v)) {
        case T_STRING:
        case T_CONSTSTRING:
            RedisModule_ReplyWithStringBuffer(ctx, v.stringval, strlen(v.stringval));
            return;
        case T_INT64:
            RedisModule_ReplyWithLongLong(ctx, v.longval);
            return;
        case T_DOUBLE:
            _ResultSet_ReplyWithRoundedDouble(ctx, v.doubleval);
            return;
        case T_BOOL:
            if (v.longval != 0) RedisModule_ReplyWithStringBuffer(ctx, "true", 4);
            else RedisModule_ReplyWithStringBuffer(ctx, "false", 5);
            return;
        case T_NULL:
            RedisModule_ReplyWithNull(ctx);
            return;
        default:
            assert("Unhandled value type" && false);
      }
}

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

static void _ResultSet_ReplayRecord(ResultSet *s, const Record r) {
    uint column_count = s->header->columns_len;
    RedisModule_ReplyWithArray(s->ctx, column_count);

    for(uint i = 0; i < column_count; i++) {
        _ResultSet_ReplyWithScalar(s->ctx, Record_GetScalar(r, i));
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

    const NEWAST *ast = NEWAST_GetFromLTS();
    ResultSetHeader* header = rm_malloc(sizeof(ResultSetHeader));
    header->columns_len = 0;
    header->columns = NULL;

    unsigned int return_expression_count = array_len(ast->return_expressions);
    if(return_expression_count > 0) {
        header->columns_len = return_expression_count;
        header->columns = rm_malloc(sizeof(Column*) * header->columns_len);
    }

    for(int i = 0; i < header->columns_len; i++) {
        // AST_Entity *elem = ast->return_expressions[i];
        ReturnElementNode *elem = ast->return_expressions[i];

        // TODO this seems really pointless
        char *column_name;
        AR_EXP_ToString(elem->exp, &column_name);
        char *alias;
        if (elem->alias) {
            alias = (char*)elem->alias;
        } else {
            alias = column_name;
        }
        Column* column = _NewColumn(column_name, alias);

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

ResultSet* NewResultSet(AST* ast, RedisModuleCtx *ctx) {
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

    _ResultSet_SetupReply(set);

    return set;
}

int ResultSet_AddRecord(ResultSet* set, Record r) {
    set->recordCount++;
    _ResultSet_ReplayRecord(set, r);
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
