#include "resultset.h"
#include "grouping/group_cache.h"

Vector* _buildAggQueryRecord(RedisModuleCtx* ctx, const Vector* returnElements, const Group* group) {
    int keyIdx = 0;
    int aggIdx = 0;
    Vector* record = NewVector(RedisModuleString*, Vector_Size(returnElements));

    // Sort group elements according to specified return order.
    for(int i = 0; i < Vector_Size(returnElements); i++) {
        ReturnElementNode* retElem;
        Vector_Get(returnElements, i, &retElem);

        if(retElem->type == N_AGG_FUNC) {
            AggCtx* aggCtx;
            Vector_Get(group->aggregationFunctions, aggIdx, &aggCtx);

            // get string representation of double
            char strAggValue[32];
            SIValue_ToString(aggCtx->result, strAggValue, 32);

            RedisModuleString* rmStrAggValue = RedisModule_CreateString(ctx, strAggValue, 32);
            Vector_Push(record, rmStrAggValue);

            aggIdx++;
        } else {
            RedisModuleString* key;
            Vector_Get(group->keys, keyIdx, &key);
            Vector_Push(record, key);

            keyIdx++;
        }
    }

    return record;
}

ResultSet* NewResultSet(const QueryExpressionNode* ast) {
    ResultSet* set = (ResultSet*)malloc(sizeof(ResultSet));
    set->returnElements = ast->returnNode->returnElements;
    set->aggregated = ReturnClause_ContainsAggregation(ast->returnNode);
    set->limit = RESULTSET_UNLIMITED;
    
    if(ast->limitNode != NULL) {
        set->limit = ast->limitNode->limit;
    }

    set->records = NewVector(Vector*, set->limit);

    return set;
}

int ResultSet_AddRecord(ResultSet* set, const Vector* record) {
    if(!ResultSet_Full(set)) {
        Vector_Push(set->records, record);
        return RESULTSET_OK;
    }
    return RESULTSET_FULL;
}

int ResultSet_Full(const ResultSet* set) {
    return (set->limit != RESULTSET_UNLIMITED && Vector_Size(set->records) >= set->limit);
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
        Vector* record = _buildAggQueryRecord(ctx, set->returnElements, group);
        
        if(ResultSet_AddRecord(set, record) == RESULTSET_FULL) {
            break;
        }
    }
}

void ResultSet_Replay(RedisModuleCtx* ctx, ResultSet* set) {
    if(set->aggregated) {
        _aggregateResultSet(ctx, set);
    }
    
    // Replay final result set.
    size_t resultSetSize = Vector_Size(set->records) + 1; // Additional one for time measurement
    RedisModule_ReplyWithArray(ctx, resultSetSize);
    
    for(int i = 0; i < Vector_Size(set->records); i++) {
        Vector* record = NULL;
        Vector_Get(set->records, i, &record);
        
        char* strRecord = NULL;
        _concatRMStrings(record, ",", &strRecord);
        
        RedisModule_ReplyWithStringBuffer(ctx, strRecord, strlen(strRecord));

        // TODO: free strRecord
        // free(strRecord);
    }
}


void ResultSet_Free(RedisModuleCtx* ctx, ResultSet* set) {
    if(set != NULL) {
        // Free each record
        for(int i = 0; i < Vector_Size(set->records); i++) {
            Vector* record;
            Vector_Get(set->records, i, &record);

            // Free each record element
            for(int j = 0; j < Vector_Size(record); j++) {
                RedisModuleString* elem = NULL;
                Vector_Get(record, j, &elem);
                RedisModule_FreeString(ctx, elem);
            }            
            free(record);
        }
        free(set->records);
        free(set);
    }
}