#include "record.h"
#include "../query_executor.h"

Record* NewRecord(int* valuesCount) {
    Record* r = (Record*)RedisModule_Alloc(sizeof(Record));

    int len = 0;
    if(valuesCount != NULL) {
        len = *valuesCount;
    }

    r->values = NewVector(RedisModuleString*, len);
    return r;
}

Record* Record_FromGraph(RedisModuleCtx *ctx, const QueryExpressionNode* ast, const Graph* g) {
    Vector* elements = ReturnClause_RetrievePropValues(ctx, ast->returnNode, g);
    if(elements == NULL) {
        return NULL;
    }

    Record *r = (Record*)RedisModule_Alloc(sizeof(Record));
    r->values = elements;

    return r;
}

// Creates a new result-set record from an aggregated group.
Record* Record_FromGroup(RedisModuleCtx* ctx, const QueryExpressionNode* ast, const Group* g) {
    size_t elementsCount = Vector_Size(ast->returnNode->returnElements);
    Record *r = NewRecord(&elementsCount);
    Vector* returnElements = ast->returnNode->returnElements;
    
    int keyIdx = 0;
    int aggIdx = 0;

    // Add group elements according to specified return order.
    for(int i = 0; i < Vector_Size(returnElements); i++) {
        ReturnElementNode* retElem;
        Vector_Get(returnElements, i, &retElem);

        if(retElem->type == N_AGG_FUNC) {
            AggCtx* aggCtx;
            Vector_Get(g->aggregationFunctions, aggIdx, &aggCtx);

            // get string representation of double
            char strAggValue[32];
            SIValue_ToString(aggCtx->result, strAggValue, 32);

            RedisModuleString* rmStrAggValue = RedisModule_CreateString(ctx, strAggValue, 32);
            Vector_Push(r->values, rmStrAggValue);

            aggIdx++;
        } else {
            RedisModuleString* key;
            Vector_Get(g->keys, keyIdx, &key);
            Vector_Push(r->values, key);

            keyIdx++;
        }
    }

    return r;
}

char* Record_ToString(const Record* record) {
    char* strRecord = NULL;
    _concatRMStrings(record->values, ",", &strRecord);
    return strRecord;
}

int Records_Compare(const Record *A, const Record *B, int* compareIndices, size_t compareIndicesLen) {
    SIValue siAValue;
    SIValue siBValue;
    double dAValue;
    double dBValue;

    for(int i = 0; i < compareIndicesLen; i++) {
        RedisModuleString* aValue = NULL;
        RedisModuleString* bValue = NULL;

        // Get element index to comapre
        int index = compareIndices[i];
        Vector_Get(A->values, index, &aValue);
        Vector_Get(B->values, index, &bValue);

        _RedisModuleStringToNum(aValue, &siAValue);
        _RedisModuleStringToNum(bValue, &siBValue);

        SIValue_ToDouble(&siAValue, &dAValue);
        SIValue_ToDouble(&siBValue, &dBValue);

        if(dAValue > dBValue) {
            return 1;
        } else if(dAValue < dBValue) {
            return -1;
        }
    }

    return 0;
}

// Frees given record.
void Record_Free(RedisModuleCtx* ctx, Record* r) {
    if(r != NULL) {
        for(int i = 0; i < Vector_Size(r->values); i++) {
            RedisModuleString* value = NULL;
            Vector_Get(r->values, i, &value);
            RedisModule_FreeString(ctx, value);
        }
        Vector_Free(r->values);
    }
}