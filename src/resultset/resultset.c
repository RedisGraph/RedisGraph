/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include "resultset.h"
#include "../value.h"
#include "../grouping/group_cache.h"
#include "../arithmetic/aggregate.h"
#include "../query_executor.h"

int _heap_elem_compare(const void * A, const void * B, const void *udata) {
    ResultSet* set = (ResultSet*)udata;
    int direction = set->direction;
    const Record *aRec = (Record*)A;
    const Record *bRec = (Record*)B;
    return Records_Compare(aRec, bRec, set->header->orderBys, set->header->orderby_len) * direction;
}

/* Checks if we've already seen given records
 * Returns 1 if the string did not exist otherwise 0. */
int __encounteredRecord(ResultSet* set, const Record* record) {
    char *str = NULL;
    size_t len = 0;
    len = Record_ToString(record, &str, &len);

    // Returns 1 if the string did NOT exist otherwise 0
    int newRecord = TrieMap_Add(set->trie, str, len, NULL, NULL);
    free(str);
    return !newRecord;
}

Column* NewColumn(const char* name, const char* alias, int aggregated) {
    Column* column = malloc(sizeof(Column));
    column->name = NULL;
    column->alias = NULL;
    column->aggregated = aggregated;

    if(name != NULL) {
        column->name = strdup(name);
    }

    if(alias != NULL) {
        column->alias = strdup(alias);
    }

    return column;
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

ResultSetHeader* NewResultSetHeader(const AST_Query *ast) {
    ResultSetHeader* header = malloc(sizeof(ResultSetHeader));
    header->columns_len = 0;
    header->orderby_len = 0;
    header->columns = NULL;
    header->orderBys = NULL;

    if(ast->returnNode != NULL) {
        header->columns_len = Vector_Size(ast->returnNode->returnElements);
        header->columns = malloc(sizeof(Column*) * header->columns_len);
    }

    if(ast->orderNode != NULL) {
        header->orderby_len = Vector_Size(ast->orderNode->columns);
        header->orderBys = malloc(sizeof(int) * header->orderby_len);
    }

    for(int i = 0; i < header->columns_len; i++) {
        AST_ReturnElementNode* returnElementNode;
        Vector_Get(ast->returnNode->returnElements, i, &returnElementNode);

        AR_ExpNode* ar_exp = AR_EXP_BuildFromAST(returnElementNode->exp, NULL);

        char* column_name;
        AR_EXP_ToString(ar_exp, &column_name);

        Column* column = NewColumn(column_name,
                                   returnElementNode->alias,
                                   AR_EXP_ContainsAggregation(ar_exp, NULL));
        free(column_name);
        AR_EXP_Free(ar_exp);

        header->columns[i] = column;
    }

    // Scan through order-by clause
    // foreach order by node locate its coresponding
    // element within the return clause
    for(int i = 0; i < header->orderby_len; i++) {
        AST_ColumnNode* orderBy = NULL;
        Vector_Get(ast->orderNode->columns, i, &orderBy);

        char *orderByColumnName;
        asprintf(&orderByColumnName, "%s.%s", orderBy->alias, orderBy->property);

        // Search return elements for orderBy
        for(int j = 0; j < header->columns_len; j++) {
            Column* col = header->columns[j];
            int match = 0;

            if(orderBy->type == N_VARIABLE) {                
                match = !(strcmp(orderByColumnName, col->name));
            } else if(col->alias != NULL) {
                // Alias (AS alias)
                match = !(strcmp(orderBy->alias, col->alias));
            }

            if(match) {
                header->orderBys[i] = j;
                break;
            }
        }
        free(orderByColumnName);
    }

    return header;
}

char *ResultSetHeader_ToString(const ResultSetHeader *header, size_t *strLen) {
    size_t len = 0;

    // Determine required buffer length.
    for(int i = 0; i < header->columns_len; i++) {
        Column *c = header->columns[i];
        if(c->alias != NULL) {
            len += strlen(c->alias)+1;
        } else {
            len += strlen(c->name)+1;
        }
    }

    char *str = calloc(len + 1, sizeof(char));
    len = 0;

    for(int i = 0; i < header->columns_len; i++) {
        Column *c = header->columns[i];
        if(c->alias != NULL) {
            len += sprintf(str+len, "%s,", c->alias);
        } else {
            len += sprintf(str+len, "%s,", c->name);
        }
    }

    str[len] = 0; // Replace last comma with NULL.
    len--;
    if(strLen != NULL) *strLen = len;

    return str;
}

void ResultSetHeader_Free(ResultSetHeader* header) {
    for(int i = 0; i < header->columns_len; i++) Column_Free(header->columns[i]);

    if(header->columns != NULL) {
        free(header->columns);
    }
    if(header->orderBys != NULL) {
        free(header->orderBys);
    }
    free(header);
}

ResultSet* NewResultSet(AST_Query* ast) {
    ResultSet* set = (ResultSet*)malloc(sizeof(ResultSet));
    set->ast = ast;
    set->heap = NULL;
    set->trie = NULL;
    set->aggregated = ReturnClause_ContainsAggregation(ast->returnNode);
    set->ordered = (ast->orderNode != NULL);
    set->limit = RESULTSET_UNLIMITED;
    set->direction = DIR_ASC;
    set->distinct = (ast->returnNode && ast->returnNode->distinct);
    set->header = NewResultSetHeader(ast);
    set->records = NewVector(Record*, 0);
    set->stats.labels_added = 0;
    set->stats.nodes_created = 0;
    set->stats.properties_set = 0;
    set->stats.relationships_created = 0;
    set->stats.nodes_deleted = 0;
    set->stats.relationships_deleted = 0;

    if(set->ordered && ast->orderNode->direction == ORDER_DIR_DESC) {
        set->direction = DIR_DESC;
    }

    if(ast->limitNode != NULL) {
        set->limit = ast->limitNode->limit;
    }

    if(set->limit != RESULTSET_UNLIMITED && set->ordered) {
        set->heap = heap_new(_heap_elem_compare, set);
    }

    if(set->distinct) {
        set->trie = NewTrieMap();
    }

    set->records = NewVector(Record*, set->limit);
    return set;
}

int ResultSet_AddRecord(ResultSet* set, Record* record) {
    if(ResultSet_Full(set)) {
        return RESULTSET_FULL;
    }
    
    /* TODO: Incase of an aggregated query, there's no need to distinct check */
    /* groups are already distinct by key. */
    if(set->distinct && __encounteredRecord(set, record)) {
        /* TODO: indicate we've skipped record. */
        return RESULTSET_OK;
    }

    if(set->ordered && set->limit != RESULTSET_UNLIMITED) {
        if(heap_count(set->heap) < set->limit) {
            /* There's room in heap for record */
            heap_offer(&set->heap, record);            
        } else {
            if(_heap_elem_compare(heap_peek(set->heap), record, set) > 0) {                
                Record *replaced = heap_poll(set->heap);
                heap_offer(&set->heap, record);
            }
        }
    } else {
        /* Not using a heap and there's room for record. */
        Vector_Push(set->records, record);
    }

    return RESULTSET_OK;
}

int ResultSet_Full(const ResultSet* set) {
    if(set->ordered) {
        /* Must process all records. */
        return 0;
    } else {
        return (set->limit != RESULTSET_UNLIMITED && Vector_Size(set->records) >= set->limit);
    }
}

void _aggregateResultSet(RedisModuleCtx* ctx, ResultSet* set) {
    char *key;
    Group *group;
    CacheGroupIterator *iter = CacheGroupIter();

    /* Scan entire groups cache. */
    while(CacheGroupIterNext(iter, &key, &group) != 0) {
        /* Construct response */
        Record* record = Record_FromGroup(set->header, group);
        if(ResultSet_AddRecord(set, record) == RESULTSET_FULL) {
            break;
        }
    }
    
    FreeGroupCache();
    InitGroupCache();
}

/* TODO: Drop heap, use some sort algo. */
Record** _sortResultSet(const ResultSet *set, const Vector* records) {
    size_t len = Vector_Size(records);

    Record **arrRecords = malloc(sizeof(Record*) * len);
    heap_t *heap = heap_new(_heap_elem_compare, set);

    /* Push records to heap. */
    for(int i = 0; i < len; i++) {
        Record *r = NULL;
        Vector_Get(records, i, &r);
        heap_offer(&heap, r);
    }

    /* Pop items from heap. */
    int i = 0;
    while(heap_count(heap) > 0) {
        Record *record = heap_poll(heap);
        arrRecords[i] = record;
        i++;
    }
    return arrRecords;
}

void _ResultSet_ReplayStats(RedisModuleCtx* ctx, ResultSet* set) {
    char buff[512] = {0};
    size_t resultset_size = 1; /* query execution time. */

    if(set->stats.labels_added > 0) resultset_size++;
    if(set->stats.nodes_created > 0) resultset_size++;
    if(set->stats.properties_set > 0) resultset_size++;
    if(set->stats.relationships_created > 0) resultset_size++;
    if(set->stats.nodes_deleted > 0) resultset_size++;
    if(set->stats.relationships_deleted > 0) resultset_size++;

    RedisModule_ReplyWithArray(ctx, resultset_size);

    if(set->stats.labels_added > 0) {
        sprintf(buff, "Labels added: %d", set->stats.labels_added);
        RedisModule_ReplyWithSimpleString(ctx, (const char*)buff);
    }

    if(set->stats.nodes_created > 0) {
        sprintf(buff, "Nodes created: %d", set->stats.nodes_created);
        RedisModule_ReplyWithSimpleString(ctx, (const char*)buff);
    }

    if(set->stats.properties_set > 0) {
        sprintf(buff, "Properties set: %d", set->stats.properties_set);
        RedisModule_ReplyWithSimpleString(ctx, (const char*)buff);
    }

    if(set->stats.relationships_created > 0) {
        sprintf(buff, "Relationships created: %d", set->stats.relationships_created);
        RedisModule_ReplyWithSimpleString(ctx, (const char*)buff);
    }

    if(set->stats.nodes_deleted > 0) {
        sprintf(buff, "Nodes deleted: %d", set->stats.nodes_deleted);
        RedisModule_ReplyWithSimpleString(ctx, (const char*)buff);
    }

    if(set->stats.relationships_deleted > 0) {
        sprintf(buff, "Relationships deleted: %d", set->stats.relationships_deleted);
        RedisModule_ReplyWithSimpleString(ctx, (const char*)buff);
    }
}

void ResultSet_Replay(RedisModuleCtx* ctx, ResultSet* set) {
    if(set->aggregated) {
        _aggregateResultSet(ctx, set);
    }
        
    size_t resultset_size = 0;
    bool results_available = false;
    int redis_response_size = 1;    /* Statistics. */
    
    if(set->ordered && set->limit != RESULTSET_UNLIMITED) {
        resultset_size += heap_count(set->heap);
    } else {
        resultset_size += Vector_Size(set->records);
    }

    results_available = resultset_size > 0;

    if(results_available) {
        resultset_size++; /* Header row. */
        redis_response_size++;   /* Result-set. */
    }

    /* Resultset + statistics. */
    RedisModule_ReplyWithArray(ctx, redis_response_size);
    
    if(results_available) {
        /* First element, resultset. */
        RedisModule_ReplyWithArray(ctx, resultset_size);

        /* Replay with table header. */
        size_t str_header_len = 0;
        size_t str_record_len = 0;
        char *str_header = ResultSetHeader_ToString(set->header, &str_header_len);
        RedisModule_ReplyWithStringBuffer(ctx, str_header, str_header_len);
        free(str_header);

        size_t str_record_cap = 2048;
        char *str_record = malloc(str_record_cap);

        if(set->ordered) {
            if(set->limit != RESULTSET_UNLIMITED) {
                /* Responses need to be reversed. */
                Record* record;
                int record_idx = 0;
                int records_count = heap_count(set->heap);
                Record **records = malloc(sizeof(Record*) * records_count);

                /* Pop items from heap */
                while(records_count > 0) {
                    record = heap_poll(set->heap);
                    records[record_idx++] = record;
                    records_count--;
                }

                /* Replay records. */
                while(record_idx) {
                    record = records[--record_idx];
                    str_record_len = Record_ToString(record, &str_record, &str_record_cap);
                    RedisModule_ReplyWithStringBuffer(ctx, str_record, str_record_len);
                    Record_Free(record);
                }

                free(records);
            } else {
                /* ordered, not limited, sort. */
                Record **sorted_records = _sortResultSet(set, set->records);

                for(int i = Vector_Size(set->records)-1; i >=0;  i--) {
                    Record* record = sorted_records[i];
                    str_record_len = Record_ToString(record, &str_record, &str_record_cap);
                    RedisModule_ReplyWithStringBuffer(ctx, str_record, str_record_len);
                }
                free(sorted_records);
            }
        } else {
            for(int i = 0; i < Vector_Size(set->records); i++) {
                Record* record = NULL;
                Vector_Get(set->records, i, &record);
                
                str_record_len = Record_ToString(record, &str_record, &str_record_cap);
                RedisModule_ReplyWithStringBuffer(ctx, str_record, str_record_len);
            }
        }

        free(str_record);
    }
    _ResultSet_ReplayStats(ctx, set);
}

void ResultSet_Free(RedisModuleCtx *ctx, ResultSet *set) {
    if(set != NULL) {
        /* Free each record */
        for(int i = 0; i < Vector_Size(set->records); i++) {
            Record *record;
            Vector_Get(set->records, i, &record);
            Record_Free(record);
            record = NULL;
        }
        Vector_Free(set->records);

        if(set->heap != NULL) {
            while(heap_count(set->heap) > 0) {
                Record* record = heap_poll(set->heap);
                Record_Free(record);
            }
            heap_free(set->heap);
        }

        if(set->trie != NULL) {
            TrieMap_Free(set->trie, TrieMap_NOP_CB);
        }

        ResultSetHeader_Free(set->header);
        free(set);
    }
}
