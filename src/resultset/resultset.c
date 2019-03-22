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

// Choose the appropriate reply formatter
EmitRecordFunc _ResultSet_SetReplyFormatter(bool compact) {
    if (compact) return ResultSet_EmitCompactRecord;
    return ResultSet_EmitVerboseRecord;
}

static void _ResultSet_ReplyWithStringMapping(RedisModuleCtx *ctx) {
    /* TODO The string mapping in its entirety is only required if the user has
     * requested any full entities.
     * Similarly, no or few label/reltype strings may be necessary depending on
     * what entity types the query requests and whether it provides labels.
     * This whole area should be refactored as soon as possible. */
    GraphContext *gc = GraphContext_GetFromTLS();

    int prop_string_count = array_len(gc->string_mapping);
    uint label_count = array_len(gc->node_schemas);
    uint reltype_count = array_len(gc->relation_schemas);

    // TODO if the query introduces new strings, we will be incapable of returning them
    RedisModule_ReplyWithArray(ctx, prop_string_count + label_count + reltype_count);
    for (int i = 0; i < prop_string_count; i ++) {
        const char *prop = gc->string_mapping[i];
        RedisModule_ReplyWithStringBuffer(ctx, prop, strlen(prop));
    }

    for (uint i = 0; i < label_count; i ++) {
        const char *label = gc->node_schemas[i]->name;
        RedisModule_ReplyWithStringBuffer(ctx, label, strlen(label));
    }

    for (uint i = 0; i < reltype_count; i ++) {
        const char *reltype = gc->relation_schemas[i]->name;
        RedisModule_ReplyWithStringBuffer(ctx, reltype, strlen(reltype));
    }
}

// Prepare replay and emit the string mapping if required.
static void _ResultSet_SetupReply(ResultSet *set) {
    // Send the string mapping, if required, as the first response
    if (set->compact) {
        // Compact replies will contain 4 top-level replies:
        // string mapping, header, records, statistics
        RedisModule_ReplyWithArray(set->ctx, 4);
        // Emit the string mapping
        _ResultSet_ReplyWithStringMapping(set->ctx);
    } else {
        // Verbose replies will contain 3 top-level replies:
        // header, records, statistics
        RedisModule_ReplyWithArray(set->ctx, 3);
    }
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

static void _Column_Free(Column* column) {
    /* No need to free alias,
     * it will be freed as part of AST_Free. */
    rm_free(column->name);
    rm_free(column);
}

static void _ResultSet_CreateHeader(ResultSet *set, AST **ast) {
    AST *final_ast = ast[array_len(ast)-1];

    assert(final_ast->returnNode && set->header == NULL && set->recordCount == 0);
    ResultSetHeader* header = rm_malloc(sizeof(ResultSetHeader));

    header->columns_len = array_len(final_ast->returnNode->returnElements);
    header->columns = rm_malloc(sizeof(Column*) * header->columns_len);

    for(int i = 0; i < header->columns_len; i++) {
        AST_ReturnElementNode *returnElementNode = final_ast->returnNode->returnElements[i];

        AR_ExpNode *ar_exp = AR_EXP_BuildFromAST(final_ast, returnElementNode->exp);

        char *column_name;
        AR_EXP_ToString(ar_exp, &column_name);
        Column *column = _NewColumn(column_name, returnElementNode->alias);
        AR_EXP_Free(ar_exp);

        header->columns[i] = column;
    }

    set->header = header;
    /* Replay with table header. */
    if (set->compact) {
        TrieMap *entities = AST_CollectEntityReferences(ast);
        ResultSet_ReplyWithCompactHeader(set->ctx, set->header, entities);
        TrieMap_Free(entities, TrieMap_NOP_CB);
    } else {
        ResultSet_ReplyWithVerboseHeader(set->ctx, set->header);
    }
}

static void _ResultSetHeader_Free(ResultSetHeader* header) {
    if(!header) return;

    for(int i = 0; i < header->columns_len; i++) _Column_Free(header->columns[i]);

    if(header->columns != NULL) {
        rm_free(header->columns);
    }

    rm_free(header);
}

ResultSet* NewResultSet(AST* ast, RedisModuleCtx *ctx, bool compact) {
    ResultSet* set = (ResultSet*)malloc(sizeof(ResultSet));
    set->ctx = ctx;
    set->gc = GraphContext_GetFromTLS();
    set->distinct = (ast->returnNode && ast->returnNode->distinct);
    set->compact = compact;
    set->EmitRecord = _ResultSet_SetReplyFormatter(set->compact);
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

    return set;
}

// Initialize the user-facing reply arrays,
// emit the string mapping if necessary and the header.
void ResultSet_ReplyWithPreamble(ResultSet *set, AST **ast) {
    // The last AST will contain the return clause, if one is specified
    AST *final_ast = ast[array_len(ast)-1];
    if (final_ast->returnNode == NULL) {
        // Queries that don't form result sets will only emit statistics
        RedisModule_ReplyWithArray(set->ctx, 1);
        return;
    }

    _ResultSet_SetupReply(set);

    _ResultSet_CreateHeader(set, ast);

    // We don't know at this point the number of records we're about to return.
    RedisModule_ReplyWithArray(set->ctx, REDISMODULE_POSTPONED_ARRAY_LEN);
}

int ResultSet_AddRecord(ResultSet *set, Record r) {
    set->recordCount++;

    // Output the current record using the defined formatter
    set->EmitRecord(set->ctx, set->gc, r, set->header->columns_len);

    return RESULTSET_OK;
}

void ResultSet_Replay(ResultSet* set) {
    // If we have emitted records, set the number of elements in the
    // preceding array
    if (set->header) {
        size_t resultset_size = set->recordCount;
        RedisModule_ReplySetArrayLength(set->ctx, resultset_size);
    }
    _ResultSet_ReplayStats(set->ctx, set);
}

void ResultSet_Free(ResultSet *set) {
    if(!set) return;

    free(set->buffer);
    if(set->header) _ResultSetHeader_Free(set->header);
    free(set);
}
