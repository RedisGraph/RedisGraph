/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "gtest.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include "../../src/execution_plan/record.h"
#include "../../src/util/rmalloc.h"
#include "../../src/value.h"

#ifdef __cplusplus
}
#endif

class RecordTest: public ::testing::Test {
  protected:
	static void SetUpTestCase() {// Use the malloc family for allocations
		Alloc_Reset();
	}
};

TEST_F(RecordTest, RecordToString) {
	rax *_rax = raxNew();
	for(int i = 0; i < 6; i++) {
		char buf[2] = {(char)i, '\0'};
		raxInsert(_rax, (unsigned char *)buf, 2, NULL, NULL);
	}

	Record r = Record_New(_rax);
	SIValue v_string = SI_ConstStringVal("Hello");
	SIValue v_int = SI_LongVal(-24);
	SIValue v_uint = SI_LongVal(24);
	SIValue v_double = SI_DoubleVal(0.314);
	SIValue v_null = SI_NullVal();
	SIValue v_bool = SI_BoolVal(1);

	Record_AddScalar(r, 0, v_string);
	Record_AddScalar(r, 1, v_int);
	Record_AddScalar(r, 2, v_uint);
	Record_AddScalar(r, 3, v_double);
	Record_AddScalar(r, 4, v_null);
	Record_AddScalar(r, 5, v_bool);

	size_t record_str_cap = 0;
	char *record_str = NULL;
	size_t record_str_len = Record_ToString(r, &record_str, &record_str_cap);

	ASSERT_EQ(strcmp(record_str, "Hello,-24,24,0.314,NULL,true"), 0);
	ASSERT_EQ(record_str_len, 28);

	rm_free(record_str);
	Record_Free(r);
}

