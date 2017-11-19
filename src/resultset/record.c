#include "record.h"
#include "../rmutil/strings.h"
#include "../query_executor.h"

Record* NewRecord(size_t len) {
    Record *r = (Record*)malloc(sizeof(Record));
    r->len = len;
    r->values = malloc(sizeof(SIValue) * len);
    return r;
}

/* Creates a new result-set record from an aggregated group. */
Record* Record_FromGroup(const ResultSetHeader *resultset_header, const Group *g) {
    Record *r = NewRecord(resultset_header->columns_len);
    
    int key_idx = 0;
    int agg_idx = 0;

    /* Add group elements according to specified return order. */
    for(int i = 0; i < resultset_header->columns_len; i++) {
        if(resultset_header->columns[i]->aggregated) {
            AR_ExpNode *agg_exp;
            Vector_Get(g->aggregationFunctions, agg_idx, &agg_exp);
            AR_EXP_Reduce(agg_exp);
            SIValue agg_result = AR_EXP_Evaluate(agg_exp);
            r->values[i] = agg_result;

            agg_idx++;
        } else {
            r->values[i] = g->keys[key_idx];
            key_idx++;
        }
    }

    return r;
}

size_t Record_ToString(const Record *record, char **record_str) {
    return SIValue_StringConcat(record->values, record->len, record_str);
}

int Records_Compare(const Record *A, const Record *B, int* compareIndices, size_t compareIndicesLen) {
    SIValue aValue;
    SIValue bValue;

    for(int i = 0; i < compareIndicesLen; i++) {
        /* Get element index to comapre. */
        int index = compareIndices[i];
        aValue = A->values[index];
        bValue = B->values[index];
        
        /* Asuuming both values are of type double. */
        if(aValue.doubleval > bValue.doubleval) {
            return 1;
        } else if(aValue.doubleval < bValue.doubleval) {
            return -1;
        }
    }

    return 0;
}

/* Frees given record. */
void Record_Free(Record *r) {
    if(r == NULL) return;
    free(r->values);
    free(r);
}
