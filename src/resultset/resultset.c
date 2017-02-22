#include "resultset.h"
#include "../value.h"
#include "../grouping/group_cache.h"

void _RedisModuleStringToNum(const RedisModuleString* value, SIValue* numValue) {
    // Cast to numeric.
    size_t len;
    const char* strValue = RedisModule_StringPtrLen(value, &len);
    
    // Convert to double SIValue.
    numValue->type = T_DOUBLE;
    SI_ParseValue(numValue, strValue, len);
}

int _heap_elem_compare(const void * A, const void * B, const void *udata) {
    ResultSet* set = (ResultSet*)udata;
    int direction = set->direction;
    const Record *aRec = (Record*)A;
    const Record *bRec = (Record*)B;

    return Records_Compare(aRec, bRec, set->header->orderBys, set->header->orderByLen) * direction;
}

/*
Checks if we've already seen given records
Returns 1 if the string did not exist otherwise 0
*/
int __encounteredRecord(const ResultSet* set, const Record* record) {
    char* str = Record_ToString(record);
    tm_len_t len = strlen(str);

    // Returns 1 if the string did NOT exist otherwise 0
    int newRecord = TrieMapNode_Add(&set->trie, str, len,  NULL, NULL);
    free(str);

    return !newRecord;
}

Column* NewColumn(const char* name, const char* alias) {
    Column* column = malloc(sizeof(Column));
    column->name = NULL;
    column->alias = NULL;

    if(name != NULL) {
        column->name = malloc(sizeof(char) * (strlen(name) + 1));
        strcpy(column->name, name);
    }

    if(alias != NULL) {
        column->alias = malloc(sizeof(char) * (strlen(alias) + 1));
        strcpy(column->alias, alias);
    }

    return column;
}

void Column_Free(Column* column) {
    free(column->name);
    free(column->alias);
    free(column);
}

ResultSetHeader* NewResultSetHeader(const QueryExpressionNode *ast) {
    ResultSetHeader* header = malloc(sizeof(ResultSetHeader));
    header->columnsLen = 0;
    header->orderByLen = 0;
    header->columns = NULL;
    header->orderBys = NULL;

    if(ast->returnNode != NULL) {
        header->columnsLen = Vector_Size(ast->returnNode->returnElements);
        header->columns = malloc(sizeof(Column*) * header->columnsLen);
    }

    if(ast->orderNode != NULL) {
        header->orderByLen = Vector_Size(ast->orderNode->columns);
        header->orderBys = malloc(sizeof(int) * header->orderByLen);
    }

    for(int i = 0; i < header->columnsLen; i++) {
        ReturnElementNode* returnElementNode;
        Vector_Get(ast->returnNode->returnElements, i, &returnElementNode);

        size_t columnNameLen = strlen(returnElementNode->variable->alias) + strlen(returnElementNode->variable->property) + 1;
        char* columnName = malloc(sizeof(char) * columnNameLen);
        sprintf(columnName, "%s.%s", returnElementNode->variable->alias, returnElementNode->variable->property);

        Column* column = NewColumn(columnName, returnElementNode->alias);
        free(columnName);

        header->columns[i] = column;
    }

    // Scan through order-by clause
    // foreach order by node locate its coresponding
    // element within the return clause
    for(int i = 0; i < header->orderByLen; i++) {
        ColumnNode* orderBy = NULL;
        Vector_Get(ast->orderNode->columns, i, &orderBy);

        // Search return elements for orderBy
        for(int j = 0; j < header->columnsLen; j++) {
            Column* col = header->columns[j];
            int match = 1;

            if(orderBy->type == N_VARIABLE) {
                char* orderByColumnName = malloc(sizeof(char) * (strlen(orderBy->alias), strlen(orderBy->property) + 1));
                sprintf(orderByColumnName, "%s.%s", orderBy->alias, orderBy->property);
                match = strcmp(orderByColumnName, col->name);
                free(orderByColumnName);
            } else {
                // Alias (AS alias)
                if(col->alias != NULL) {
                    match = strcmp(orderBy->alias, col->alias);
                }
            }

            if(match == 0) {
                header->orderBys[i] = j;
                break;
            }
        }
    }

    return header;
}

void ResultSetHeader_Free(ResultSetHeader* header) {
    for(int i = 0; i < header->columnsLen; i++) Column_Free(header->columns[i]);

    if(header->columns != NULL) {
        free(header->columns);
    }
    if(header->orderBys != NULL) {
        free(header->orderBys);
    }
    free(header);
}

ResultSet* NewResultSet(const QueryExpressionNode* ast) {
    ResultSet* set = (ResultSet*)malloc(sizeof(ResultSet));
    set->ast = ast;
    set->heap = NULL;
    set->trie = NULL;
    set->aggregated = ReturnClause_ContainsAggregation(ast->returnNode);
    set->ordered = (ast->orderNode != NULL);
    set->limit = RESULTSET_UNLIMITED;
    set->direction =  DIR_ASC;
    set->distinct = ast->returnNode->distinct;
    set->header = NewResultSetHeader(ast);
    set->records = NewVector(Record*, 0);

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

int ResultSet_AddRecord(ResultSet* set, const Record* record) {
    if(ResultSet_Full(set)) {
        return RESULTSET_FULL;
    }
    
    // TODO: Incase of an aggregated query, there's no need to distinct check
    // groups are alreay distinct by key.
    if(set->distinct && __encounteredRecord(set, record)) {
        // TODO: indicate we've skipped record.
        return RESULTSET_OK;
    }

    if(set->ordered && set->limit != RESULTSET_UNLIMITED) {
        if(heap_count(set->heap) < set->limit) {
            // There's room in heap for record
            heap_offer(&set->heap, record);
        } else {
            if(_heap_elem_compare(heap_peek(set->heap), record, set) == 1) {
                heap_poll(set->heap);
                heap_offer(&set->heap, record);
            }
        }
    } else {
        // Not using a heap and there's room for record.
        Vector_Push(set->records, record);
    }

    return RESULTSET_OK;
}

int ResultSet_Full(const ResultSet* set) {
    if(set->ordered) {
        // Must process all records
        return 0;
    } else {
        return (set->limit != RESULTSET_UNLIMITED && Vector_Size(set->records) >= set->limit);
    }
}

void _aggregateResultSet(RedisModuleCtx* ctx, ResultSet* set) {
    char* key;
    Group* group;
    khiter_t iter = CacheGroupIter();

    // Scan entire groups cache
    while(CacheGroupIterNext(&iter, &key, &group) != 0) {
        // Finalize each aggregation function
        for(int i = 0; i < Vector_Size(group->aggregationFunctions); i++) {
            AggCtx* aggCtx = NULL;
            Vector_Get(group->aggregationFunctions, i, &aggCtx);
            Agg_Finalize(aggCtx);
        }

        // Construct response
        Record* record = Record_FromGroup(ctx, set->ast, group);
        if(ResultSet_AddRecord(set, record) == RESULTSET_FULL) {
            break;
        }
    }
}

// TODO: Drop heap, use some sort algo
Record** _sortResultSet(const ResultSet *set, const Vector* records) {
    size_t len = Vector_Size(records);

    Record** arrRecords = malloc(sizeof(Record*) * len);
    heap_t* heap = heap_new(_heap_elem_compare, set);

    // Push records to heap
    for(int i = 0; i < len; i++) {
        Record* r = NULL;
        Vector_Get(records, i, &r);
        heap_offer(&heap, r);
    }

    // Pop items from heap
    int i = 0;
    while(heap_count(heap) > 0) {
        Record* record = heap_poll(heap);
        arrRecords[i] = record;
        i++;
    }
    return arrRecords;
}

void ResultSet_Replay(RedisModuleCtx* ctx, ResultSet* set) {
    if(set->aggregated) {
        _aggregateResultSet(ctx, set);
    }
    
    size_t resultSetSize;

    if(set->ordered && set->limit != RESULTSET_UNLIMITED) {
        resultSetSize = heap_count(set->heap);
    } else {
        resultSetSize = Vector_Size(set->records);
    }
    resultSetSize++; // Additional one for time measurement

    // Replay final result set.
    RedisModule_ReplyWithArray(ctx, resultSetSize);
    char *strRecord = NULL;
    if(set->ordered) {
        if(set->limit != RESULTSET_UNLIMITED) {
            // Responses need to be reversed.
            Vector *reversedResultSet = NewVector(char*, heap_count(set->heap));

            // Pop items from heap
            while(heap_count(set->heap) > 0) {
                Record* record = heap_poll(set->heap);
                strRecord = Record_ToString(record);
                Vector_Push(reversedResultSet, strRecord);

                // Free record here, as it was removed from set heap.
                Record_Free(ctx, record);
            }
            // Replay elements in reversed order
            for(int i = Vector_Size(reversedResultSet)-1; i >= 0; i--) {
                Vector_Get(reversedResultSet, i, &strRecord);
                RedisModule_ReplyWithStringBuffer(ctx, strRecord, strlen(strRecord));
                free(strRecord);
            }
            Vector_Free(reversedResultSet);
        } else {
            // ordered, not limited, sort.
            Record** sortedRecords = _sortResultSet(set, set->records);

            for(int i = Vector_Size(set->records)-1; i >=0;  i--) {
                Record* record = sortedRecords[i];
                char *strRecord = Record_ToString(record);
                RedisModule_ReplyWithStringBuffer(ctx, strRecord, strlen(strRecord));
                free(strRecord);
            }
            free(sortedRecords);
        }
    } else {
        for(int i = 0; i < Vector_Size(set->records); i++) {
            Record* record = NULL;
            Vector_Get(set->records, i, &record);
            
            char *strRecord = Record_ToString(record);
            RedisModule_ReplyWithStringBuffer(ctx, strRecord, strlen(strRecord));

            free(strRecord);
        }
    }
}

void ResultSet_Free(RedisModuleCtx* ctx, ResultSet* set) {
    if(set != NULL) {
        // Free each record
        for(int i = 0; i < Vector_Size(set->records); i++) {
            Vector* record;
            Vector_Get(set->records, i, &record);
            Record_Free(ctx, record);
            record = NULL;
        }
        Vector_Free(set->records);

        if(set->heap != NULL) {
            while(heap_count(set->heap) > 0) {
                Record* record = heap_poll(set->heap);
                Record_Free(ctx, record);
            }
            heap_free(set->heap);
        }

        if(set->trie != NULL) {
            // TODO: free trie.
            // TrieMapNode_Free(set->trie, NULL);
        }

        ResultSetHeader_Free(set->header);
        free(set);
    }
}
