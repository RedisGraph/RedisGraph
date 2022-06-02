/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
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
		// Use the malloc family for allocations
		Alloc_Reset();
	}
};

TEST_F(PIPETest, PipeUnsync) {
	// 
}

TEST_F(PIPETest, PipeReadWrite) {
	Pipe *p = Pipe_Create();

	//--------------------------------------------------------------------------
	// unsigned
	//--------------------------------------------------------------------------

	uint64_t unsigned_in = 30;
	Pipe_WriteUnsigned(p, unsigned_in);
	uint64_t unsigned_out = Pipe_ReadUnsigned(p);
	ASSERT_TRUE(unsigned_in == unsigned_out);

	//--------------------------------------------------------------------------
	// signed
	//--------------------------------------------------------------------------

	int64_t signed_in = -30;
	Pipe_WriteSigned(p, signed_in);
	int64_t signed_out = Pipe_ReadSigned(p);
	ASSERT_TRUE(signed_in == signed_out);

	//--------------------------------------------------------------------------
	// double
	//--------------------------------------------------------------------------

	double double_in = 7.12;
	Pipe_WriteDouble(p, double_in);
	double double_out = Pipe_ReadDouble(p);
	ASSERT_TRUE(double_in == double_out);

	//--------------------------------------------------------------------------
	// float
	//--------------------------------------------------------------------------

	float float_in = 876.52;
	Pipe_WriteFloat(p, float_in);
	float float_out = Pipe_ReadFloat(p);
	ASSERT_TRUE(float_in == float_out);

	//--------------------------------------------------------------------------
	// long double
	//--------------------------------------------------------------------------

	long double long_double_in = -3131.512;
	Pipe_WriteLongDouble(p, long_double_in);
	long double long_double_out = Pipe_ReadLongDouble(p);
	ASSERT_TRUE(long_double_in == long_double_out);

	//--------------------------------------------------------------------------
	// string buffer
	//--------------------------------------------------------------------------

	char str_in[5] = {0};
	sprintf(str_in, "data");
	Pipe_WriteStringBuffer(p, str_in, 5);
	size_t str_out_len = 5;
	char *str_out = Pipe_ReadStringBuffer(p, &str_out_len);
	ASSERT_TRUE(str_out_len == 5);
	ASSERT_TRUE(strcmp(str_in, str_out) == 0);

	//void Pipe_WriteString
	//(
	//	Pipe p,
	//	RedisModuleString *s
	//);
	
	Pipe_Free(p);
}

