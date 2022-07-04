/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "gtest.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include "../../src/util/pipe.h"
#include "../../src/util/rmalloc.h"

#ifdef __cplusplus
}
#endif

class PIPETest: public ::testing::Test {
  protected:
	static void SetUpTestCase() {
		// use the malloc family for allocations
		Alloc_Reset();
	}
};

TEST_F(PIPETest, PipeReadWrite) {
	Pipe *p = Pipe_Create();

	//--------------------------------------------------------------------------
	// unsigned
	//--------------------------------------------------------------------------

	uint64_t unsigned_in  = 30;
	uint64_t unsigned_out = 0;
	Pipe_WriteUnsigned(p, unsigned_in);
	Pipe_ReadUnsigned(p, &unsigned_out);
	ASSERT_TRUE(unsigned_in == unsigned_out);

	//--------------------------------------------------------------------------
	// signed
	//--------------------------------------------------------------------------

	int64_t signed_in  = -30;
	int64_t signed_out = 0;
	Pipe_WriteSigned(p, signed_in);
	Pipe_ReadSigned(p, &signed_out);
	ASSERT_TRUE(signed_in == signed_out);

	//--------------------------------------------------------------------------
	// double
	//--------------------------------------------------------------------------

	double double_in  = 7.12;
	double double_out = 0;
	Pipe_WriteDouble(p, double_in);
	Pipe_ReadDouble(p, &double_out);
	ASSERT_TRUE(double_in == double_out);

	//--------------------------------------------------------------------------
	// float
	//--------------------------------------------------------------------------

	float float_in  = 876.52;
	float float_out = 0;
	Pipe_WriteFloat(p, float_in);
	Pipe_ReadFloat(p, &float_out);
	ASSERT_TRUE(float_in == float_out);

	//--------------------------------------------------------------------------
	// long double
	//--------------------------------------------------------------------------

	long double long_double_in  = -3131.512;
	long double long_double_out = 0;
	Pipe_WriteLongDouble(p, long_double_in);
	Pipe_ReadLongDouble(p, &long_double_out);
	ASSERT_TRUE(long_double_in == long_double_out);

	//--------------------------------------------------------------------------
	// string buffer
	//--------------------------------------------------------------------------

	char str_in[5] = {0};
	sprintf(str_in, "data");
	Pipe_WriteStringBuffer(p, str_in, 5);
	char     *str_out    = NULL;
	uint64_t str_out_len = 0;
	Pipe_ReadStringBuffer(p, &str_out_len, &str_out);
	ASSERT_TRUE(str_out_len == 5);
	ASSERT_TRUE(strcmp(str_in, str_out) == 0);
	
	Pipe_Free(p);
}

