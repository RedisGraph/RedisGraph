#include "record.h"
#include "../rmutil/strings.h"
#include "../query_executor.h"

Record* NewRecord(size_t len) {
    Record *r = (Record*)malloc(sizeof(Record));
    r->values = NewVector(SIValue*, len);
    return r;
}

Record* Record_FromGraph(RedisModuleCtx *ctx, const AST_QueryExpressionNode *ast, const Graph *g) {
    Vector* elements = ReturnClause_RetrievePropValues(ast->returnNode, g);
    if(elements == NULL) {
        return NULL;
    }

    Record *r = (Record*)malloc(sizeof(Record));
    r->values = elements;

    return r;
}

/* Creates a new result-set record from an aggregated group. */
Record* Record_FromGroup(RedisModuleCtx *ctx, const AST_QueryExpressionNode *ast, const Group *g) {
    size_t elements_count = Vector_Size(ast->returnNode->returnElements);
    Record *r = NewRecord(elements_count);
    Vector *return_elements = ast->returnNode->returnElements;
    
    int key_idx = 0;
    int agg_idx = 0;

    /* Add group elements according to specified return order. */
    for(int i = 0; i < Vector_Size(return_elements); i++) {
        AST_ReturnElementNode *ret_elem;
        Vector_Get(return_elements, i, &ret_elem);

        if(ret_elem->type == N_AGG_FUNC) {
            AggCtx *agg_ctx;
            Vector_Get(g->aggregationFunctions, agg_idx, &agg_ctx);
            Vector_Push(r->values, &agg_ctx->result);

            agg_idx++;
        } else {
            SIValue *key;
            Vector_Get(g->keys, key_idx, &key);
            Vector_Push(r->values, key);

            key_idx++;
        }
    }

    return r;
}

size_t Record_ToString(const Record *record, char **record_str) {
    return SIValue_StringConcat(record->values, record_str);
}

int Records_Compare(const Record *A, const Record *B, int* compareIndices, size_t compareIndicesLen) {
    SIValue* aValue;
    SIValue* bValue;

    for(int i = 0; i < compareIndicesLen; i++) {
        /* Get element index to comapre. */
        int index = compareIndices[i];
        Vector_Get(A->values, index, &aValue);
        Vector_Get(B->values, index, &bValue);
        
        /* Asuuming both values are of type double. */
        if(aValue->doubleval > bValue->doubleval) {
            return 1;
        } else if(aValue->doubleval < bValue->doubleval) {
            return -1;
        }
    }

    return 0;
}

/* Frees given record. */
void Record_Free(Record *r) {
    if(r == NULL) return;
    Vector_Free(r->values);
    free(r);
}
