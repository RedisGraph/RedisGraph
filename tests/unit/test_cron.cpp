/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "gtest.h"

#ifdef __cplusplus
extern "C"
{
#endif

#include <time.h>
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
		int *Y = (int*)pdata;
		X += *Y;
	}

	static void mul_task(void *pdata) {
		int *Y = (int*)pdata;
		X *= *Y;
	}

	static void long_running_task(void *pdata) {
		// sleep for 'n' seconds
		int *sec = (int*)pdata;
		sleep(*sec);
	}
};

TEST_F(CRONTest, Exec) {
	// Add two tasks to CRON
	// one adds 2 to X
	// second multiply X by 2

	X = 1;
	int Y = 2;
	int Z = 2;

	// Introduce tasks
	// X = 1.
	// X *= Y, X = 2
	// X += Z, X = 4
	// if CRON executes the tasks in the wrong order
	// X += Z, X = 3
	// X *= Y, X = 6

	Cron_AddTask(150, add_task, &Z);
	Cron_AddTask(10, mul_task, &Y);
	sleep(1); // sleep for 1 sec

	// verify X = (X * 2) + 2
	ASSERT_EQ(X, 4);
}

TEST_F(CRONTest, Abort) {
	// reset X = 1
	// issue a task X += 2
	// abort task
	// validate X = 1
	
	X = 1;
	int Y = 2;

	// issue task X += 2
	CronTaskHandle task_handle = Cron_AddTask(150, add_task, &Y);

	// abort task
	Cron_AbortTask(task_handle);
	
	sleep(1); // sleep for 1 sec

	// task should have been aborted prior to its execution
	// expecting X = 1
	ASSERT_EQ(X, 1);
}

TEST_F(CRONTest, LateAbort) {
	// reset X = 1
	// issue a task X += 2
	// abort task AFTER task been performed
	// validate X = 3
	
	X = 1;
	int Y = 2;

	// issue task X += 2
	CronTaskHandle task_handle = Cron_AddTask(150, add_task, &Y);

	sleep(1); // sleep for 1 sec

	// task should have been executed, expecting X = 1
	ASSERT_EQ(X, 3);

	// abort task, should note hang/crash
	Cron_AbortTask(task_handle);
}

TEST_F(CRONTest, MultiAbort) {
	// reset X = 1
	// issue a task X += 2
	// abort task multiple times
	// validate X = 1
	
	X = 1;
	int Y = 2;

	// issue task X += 2
	CronTaskHandle task_handle = Cron_AddTask(150, add_task, &Y);

	// abort task multiple times, should not crash hang
	for(int i = 0; i < 20; i++) Cron_AbortTask(task_handle);
	
	sleep(1); // sleep for 1 sec

	// task should have been aborted prior to its execution
	// expecting X = 1
	ASSERT_EQ(X, 1);
}

TEST_F(CRONTest, AbortNoneExistingTask) {
	// reset X = 1
	// issue a task X += 2
	// abort none existing task
	// validate X = 3
	
	X = 1;
	int Y = 2;

	// issue task X += 2
	CronTaskHandle task_handle = Cron_AddTask(150, add_task, &Y);
	CronTaskHandle none_existing_task_handle = task_handle + 1; 

	// abort task, should not crash hang
	Cron_AbortTask(none_existing_task_handle);
	
	sleep(1); // sleep for 1 sec

	// task should have been executed
	// expecting X = 3
	ASSERT_EQ(X, 3);
}

TEST_F(CRONTest, AbortRunningTask) {
	// issue a long running task ~4 seconds
	// issue abort 1 second into execution
	// validate call to Cron_AbortTask returns after ~2 seconds
	
	// issue a long running task, task will sleep for 'sec' seconds
	int sec = 4;
	CronTaskHandle task_handle = Cron_AddTask(0, long_running_task, &sec);

	sleep(1); // sleep for 1 sec

	clock_t t = clock(); // start timer

	// task should be running
	// abort task, call should return only after task is completed
	Cron_AbortTask(task_handle);

	t = clock() - t; // stop timer
	double time_taken_sec = ((double)t)/CLOCKS_PER_SEC;
	
	// expecting Cron_AbortTask to return after at-least 2 seconds
	ASSERT_GT(time_taken_sec, 2);
}

