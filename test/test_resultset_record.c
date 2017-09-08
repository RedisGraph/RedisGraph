#include <stdio.h>
#include "assert.h"
#include "../src/resultset/record.h"
#include "../src/value.h"

size_t Record_ToString(const Record *record, char **record_str);

void test_record_to_string () {
    Record *record = NewRecord(6);    
    SIValue v_string = SI_StringValC("Hello");
    SIValue v_int = SI_IntVal(-24);
    SIValue v_uint = SI_UintVal(24);
    SIValue v_float = SI_FloatVal(0.314);
    SIValue v_null = SI_NullVal();
    SIValue v_bool = SI_BoolVal(1);

    Vector_Push(record->values, &v_string);
    Vector_Push(record->values, &v_int);
    Vector_Push(record->values, &v_uint);
    // Vector_Push(record->values, &v_float);
    Vector_Push(record->values, &v_null);
    Vector_Push(record->values, &v_bool);

    char *record_str;
    size_t record_str_len = Record_ToString(record, &record_str);

    assert(strcmp(record_str, "\"Hello\",-24,24,NULL,true") == 0);
    assert(record_str_len == 24);
    
    free(record_str);
    Record_Free(record);
}

int main(int argc, char **argv) {
    test_record_to_string();
    printf("PASS!");
    return 0;
}