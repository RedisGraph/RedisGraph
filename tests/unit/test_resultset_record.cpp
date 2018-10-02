/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include "../../deps/googletest/include/gtest/gtest.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include "../../src/resultset/resultset_record.h"
#include "../../src/value.h"

#ifdef __cplusplus
}
#endif

TEST(ResultsetRecordTest, RecordToString) {
    ResultSetRecord *record = NewResultSetRecord(6);
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
    size_t record_str_len = ResultSetRecord_ToString(record, &record_str, &record_str_cap);

    EXPECT_EQ(strcmp(record_str, "Hello,-24,24,0.314000,NULL,true"), 0);
    EXPECT_EQ(record_str_len, 31);
    
    SIValue_Free(&v_string);
    free(record_str);
    ResultSetRecord_Free(record);
}
