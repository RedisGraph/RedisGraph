/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "gtest.h"

#ifdef __cplusplus
extern "C"
{
#endif

#include "../../src/util/cron.h"
#include "../../src/util/rmalloc.h"

#ifdef __cplusplus
}
#endif

int X = 1;

class CRONTest: public ::testing::Test {
  protected:
	static void SetUpTestCase() {
		// Use the malloc family for allocations
		Alloc_Reset();

		Cron_Start();
	}

	static void TearDownTestCase() {
		Cron_Stop();
	}

	static void add_task(void *pdata) {
		//printf("in add_task\n");
		int *Y = (int*)pdata;
		//printf("X: %d\n", X);
		//printf("Y: %d\n", *Y);
		X += *Y;
	}

	static void mul_task(void *pdata) {
		//printf("in mul_task\n");
		int *Y = (int*)pdata;
		//printf("X: %d\n", X);
		//printf("Y: %d\n", *Y);
		X *= *Y;
	}
};

TEST_F(CRONTest, CRONTaskExec) {
	// Add two tasks to CRON
	// one adds 2 to X
	// second multiply X by 2

	int *Y = (int*)rm_malloc(sizeof(Y));
	int *Z = (int*)rm_malloc(sizeof(Z));
	*Y = 2;
	*Z = 2;

	// Introduce tasks
	// X = 1.
	// X *= Y, X = 2
	// X += Z, X = 4
	// if CRON executes the tasks in the wrong order
	// X += Z, X = 3
	// X *= Y, X = 6

	Cron_AddTask(150, add_task, Z);
	Cron_AddTask(10, mul_task, Y);
	sleep(1); // sleep for 100ms

	//printf("X: %d\n", X);
	// verify X = (X * 2) + 2
	ASSERT_EQ(X, 4);

	//rm_free(Y);
	//rm_free(Z);
}

