#include "record.h"
#include "../query_executor.h"


Record* Record_FromGraph(RedisModuleCtx *ctx, const QueryExpressionNode* ast, const Graph* g) {
    Vector* elements = NULL;
    Vector* orderBys = NULL;

    elements = ReturnClause_RetrievePropValues(ctx, ast->returnNode, g);
    if(elements == NULL) {
        return NULL;
    }

    if(ast->orderNode != NULL) {
        orderBys = OrderClause_RetrievePropValues(ctx, ast->orderNode, g);
    }

    Record *r = (Record*)RedisModule_Alloc(sizeof(Record));
    r->elements = elements;
    r->orderBys = orderBys;

    return r;
}

// Creates a new result-set record from an aggregated group.
Record* Record_FromGroup(RedisModuleCtx* ctx, const QueryExpressionNode* ast, const Group* group) {
    Record *r = (Record*)RedisModule_Alloc(sizeof(Record));
    Vector* returnElements = ast->returnNode->returnElements;

    r->elements = NewVector(RedisModuleString*, Vector_Size(returnElements));
    r->orderBys = NULL;
    
    int keyIdx = 0;
    int aggIdx = 0;

    // Add group elements according to specified return order.
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
            Vector_Push(r->elements, rmStrAggValue);

            aggIdx++;
        } else {
            RedisModuleString* key;
            Vector_Get(group->keys, keyIdx, &key);
            Vector_Push(r->elements, key);

            keyIdx++;
        }
    }

    if(ast->orderNode != NULL) {
        r->orderBys = NewVector(RedisModuleString*, Vector_Size(ast->orderNode->variables));

        // TODO: support order by aggregated values. e.g. SUM(f.age)
        for(int i = 0; i < Vector_Size(ast->orderNode->variables); i++) {
            Variable* var = NULL;
            Vector_Get(ast->orderNode->variables, i, &var);

            keyIdx = 0;

            for(int j = 0; j < Vector_Size(ast->returnNode->returnElements); j++) {
                ReturnElementNode *retElem = NULL;
                Vector_Get(ast->returnNode->returnElements, j, &retElem);

                if(retElem->type == N_PROP) {
                    if(strcmp(retElem->alias, var->alias) == 0 && strcmp(retElem->property, var->property) == 0) {
                        RedisModuleString* key;
                        Vector_Get(group->keys, keyIdx, &key);
                        Vector_Push(r->orderBys, key);
                        break;
                    }
                    keyIdx++;
                }
            } // end of inner loop
        } // end of outter loop
    } // end of order clause existence check

    return r;
}

char* Record_ToString(const Record* record) {
    char* strRecord = NULL;
    _concatRMStrings(record->elements, ",", &strRecord);
    return strRecord;
}

// Frees given record.
void Record_Free(RedisModuleCtx* ctx, Record* r) {
    if(r != NULL) {
        if(r->elements) {
            for(int i = 0; i < Vector_Size(r->elements); i++) {
                RedisModuleString* element;
                Vector_Get(r->elements, i, &element);
                RedisModule_FreeString(ctx, element);
            }
            Vector_Free(r->elements);
        }

        if(r->orderBys) {
            for(int i = 0; i < Vector_Size(r->orderBys); i++) {
                RedisModuleString* orderBy;
                Vector_Get(r->orderBys, i, &orderBy);
                RedisModule_FreeString(ctx, orderBy);
            }
            Vector_Free(r->orderBys);
        }
        
        RedisModule_Free(r);
    }
}