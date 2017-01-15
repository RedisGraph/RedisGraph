#include "resultset.h"
#include "../grouping/group_cache.h"
#include "../value.h"

void _RedisModuleStringToNum(const RedisModuleString* value, SIValue* numValue) {
    // Cast to numeric.
    size_t len;
    const char* strValue = RedisModule_StringPtrLen(value, &len);
    
    // Convert to double SIValue.
    numValue->type = T_DOUBLE;
    SI_ParseValue(numValue, strValue, len);
}

int _heap_elem_compare(const void * A, const void * B, const void *udata) {
    int direction = *(int*)udata;
    Record *aRec = (Record*)A;
    Record *bRec = (Record*)B;
    SIValue siAValue;
    SIValue siBValue;
    double dAValue;
    double dBValue;

    // aRec and bRec should have the same number of orderBys.
    for(int i = 0; i < Vector_Size(aRec->orderBys); i++) {
        RedisModuleString* aValue = NULL;
        RedisModuleString* bValue = NULL;

        Vector_Get(aRec->orderBys, i, &aValue);
        Vector_Get(bRec->orderBys, i, &bValue);
        
        _RedisModuleStringToNum(aValue, &siAValue);
        _RedisModuleStringToNum(bValue, &siBValue);
        
        SIValue_ToDouble(&siAValue, &dAValue);
        SIValue_ToDouble(&siBValue, &dBValue);

        if(dAValue > dBValue) {
            return 1 * direction;
        } else if(dAValue < dBValue) {
            return -1 * direction;
        }
    }

    return 0;
}

ResultSet* NewResultSet(const QueryExpressionNode* ast) {
    ResultSet* set = (ResultSet*)malloc(sizeof(ResultSet));
    set->ast = ast;
    set->aggregated = ReturnClause_ContainsAggregation(ast->returnNode);
    set->ordered = (ast->orderNode != NULL);
    set->limit = RESULTSET_UNLIMITED;
    set->direction = DIR_MAX_HEAP;

    if(ast->limitNode != NULL) {
        set->limit = ast->limitNode->limit;
    }

    if(set->limit != RESULTSET_UNLIMITED && set->ordered) {
        set->heap = heap_new(_heap_elem_compare, &set->direction);
    }

    set->records = NewVector(Record*, set->limit);
    return set;
}

int ResultSet_AddRecord(ResultSet* set, const Record *record) {
    if(ResultSet_Full(set)) {
        return RESULTSET_FULL;
    }
    
    if(set->ordered && set->limit != RESULTSET_UNLIMITED) {
        if(heap_count(set->heap) < set->limit) {
            // There's room in heap for record
            heap_offer(&set->heap, record);
        } else {
            // No room in heap, see if we need to evit heap top.

            // In case of a max heap, the smallest record will be at the top of the heap.
            // if inserted record is larger then top, insert otherwise discard.
            if(_heap_elem_compare(record, heap_peek(set->heap), &set->direction) == 1 && set->direction == DIR_MAX_HEAP) {
                heap_offer(&set->heap, record);
            }

            // In case of a min heap, the largest record will be at the top of the heap.
            // if inserted record is smaller then top, insert otherwise discard.
            if(_heap_elem_compare(record, heap_peek(set->heap), &set->direction) == -1 && set->direction == DIR_MIN_HEAP) {
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
    
    if(set->ordered && set->limit != RESULTSET_UNLIMITED) {
        int limitCounter = set->limit;
        while(heap_count(set->heap) > 0 && limitCounter > 0) {
            Record* record = heap_poll(set->heap);
            char *strRecord = Record_ToString(record);
            RedisModule_ReplyWithStringBuffer(ctx, strRecord, strlen(strRecord));

            free(strRecord);
            limitCounter--;
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
            Record* record;
            Vector_Get(set->records, i, &record);
            Record_Free(ctx, record);
        }
        free(set->records);
        free(set);
    }
}