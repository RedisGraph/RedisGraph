#include <stdio.h>
#include "assert.h"
#include "../../src/resultset/record.h"
#include "../../src/value.h"

void test_record_to_string() {
    Record *record = NewRecord(6);    
    SIValue v_string = SI_StringVal("Hello");
    SIValue v_int = SI_IntVal(-24);
    SIValue v_uint = SI_UintVal(24);
    SIValue v_float = SI_FloatVal(0.314);
    SIValue v_null = SI_NullVal();
    SIValue v_bool = SI_BoolVal(1);

    record->values[0] = v_string;
    record->values[1] = v_int;
    record->values[2] = v_uint;
    record->values[3] = v_float;
    record->values[4] = v_null;
    record->values[5] = v_bool;

    size_t record_str_cap = 0;
    char *record_str = NULL;
    size_t record_str_len = Record_ToString(record, &record_str, &record_str_cap);

    assert(strcmp(record_str, "Hello,-24,24,0.314000,NULL,true") == 0);
    assert(record_str_len == 31);
    
    SIValue_Free(&v_string);
    free(record_str);
    Record_Free(record);
}

int main(int argc, char **argv) {
    test_record_to_string();
    printf("test_resultset_record - PASS!\n");
    return 0;
}