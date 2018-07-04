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
            r->values[i] = AR_EXP_Evaluate(agg_exp);
            agg_idx++;
        } else {
            r->values[i] = g->keys[key_idx];
            key_idx++;
        }
    }

    return r;
}

size_t Record_ToString(const Record *record, char **buf, size_t *buf_cap) {
    size_t required_len = SIValue_StringConcatLen(record->values, record->len);

    if(*buf_cap < required_len) {
        *buf = realloc(*buf, sizeof(char) * required_len);
        *buf_cap = required_len;
    }

    return SIValue_StringConcat(record->values, record->len, *buf, *buf_cap);
}

int Records_Compare(const Record *A, const Record *B, int* compareIndices, size_t compareIndicesLen) {
    SIValue a;
    SIValue b;

    for(int i = 0; i < compareIndicesLen; i++) {
        /* Get element index to comapre. */
        int index = compareIndices[i];
        a = A->values[index];
        b = B->values[index];
        int relation = SIValue_Compare(a, b);
        if(relation) return relation;
    }

    return 0;
}

/* Frees given record. */
void Record_Free(Record *r) {
    if(r == NULL) return;
    free(r->values);
    free(r);
}
