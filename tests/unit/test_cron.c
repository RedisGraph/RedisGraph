/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "src/util/cron.h"
#include "src/util/rmalloc.h"

#include <time.h>

void setup();
void tearDown();

#define TEST_INIT setup();
#define TEST_FINI tearDown();
#include "acutest.h"

int X = 1;

static int mssleep(uint ms) {
	struct timespec req;

	req.tv_sec = ms / 1000;
	req.tv_nsec = (ms % 1000) * 1000000;

	return nanosleep(&req, NULL);
}

static void TearDownTestCase() {
}

void add_task(void *pdata) {
	int *Y = (int*)pdata;
	X += *Y;
}

void mul_task(void *pdata) {
	int *Y = (int*)pdata;
	X *= *Y;
}

void long_running_task(void *pdata) {
	// sleep for 'n' milliseconds
	int *ms = (int*)pdata;
	mssleep(*ms);
}

void setup() {
	// Use the malloc family for allocations
	Alloc_Reset();
	Cron_Start();
}

void tearDown() {
	Cron_Stop();
}

void test_cronExec() {
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

	Cron_AddTask(15, add_task, &Z);
	Cron_AddTask(5, mul_task, &Y);

	mssleep(100); // sleep for 100 ms

	// verify X = (X * 2) + 2
	TEST_ASSERT(X == 4);
}

void test_cronAbort() {
	// reset X = 1
	// issue a task X += 2
	// abort task
	// validate X = 1
	
	X = 1;
	int Y = 2;

	// issue task X += 2
	CronTaskHandle task_handle = Cron_AddTask(15, add_task, &Y);

	// abort task
	Cron_AbortTask(task_handle);
	
	mssleep(100); // sleep for 100 ms

	// task should have been aborted prior to its execution
	// expecting X = 1
	TEST_ASSERT(X == 1);
}

void test_cronLateAbort() {
	// reset X = 1
	// issue a task X += 2
	// abort task AFTER task been performed
	// validate X = 3
	
	X = 1;
	int Y = 2;

	// issue task X += 2
	CronTaskHandle task_handle = Cron_AddTask(15, add_task, &Y);

	mssleep(100); // sleep for 100 ms

	// task should have been executed, expecting X = 1
	TEST_ASSERT(X == 3);

	// abort task, should note hang/crash
	Cron_AbortTask(task_handle);
}

void test_MultiAbort() {
	// reset X = 1
	// issue a task X += 2
	// abort task multiple times
	// validate X = 1
	
	X = 1;
	int Y = 2;

	// issue task X += 2
	CronTaskHandle task_handle = Cron_AddTask(15, add_task, &Y);

	// abort task multiple times, should not crash hang
	for(int i = 0; i < 20; i++) {
		Cron_AbortTask(task_handle);
	}

	mssleep(100); // sleep for 100 ms

	// task should have been aborted prior to its execution
	// expecting X = 1
	TEST_ASSERT(X == 1);
}

void test_abortNoneExistingTask() {
	// reset X = 1
	// issue a task X += 2
	// abort none existing task
	// validate X = 3
	
	X = 1;
	int Y = 2;

	// issue task X += 2
	CronTaskHandle task_handle = Cron_AddTask(15, add_task, &Y);
	CronTaskHandle none_existing_task_handle = task_handle + 1; 

	// abort task, should not crash hang
	Cron_AbortTask(none_existing_task_handle);
	
	mssleep(100); // sleep for 100 ms

	// task should have been executed
	// expecting X = 3
	TEST_ASSERT(X == 3);
}

void test_AbortRunningTask() {
	// issue a long running task ~100ms
	// issue abort 20ms into execution
	// validate call to Cron_AbortTask returns in less than ~10 ms
	
	// issue a long running task, task will sleep for 100 'ms'
	int ms = 100;
	CronTaskHandle task_handle = Cron_AddTask(0, long_running_task, &ms);

	mssleep(20); // sleep for 20 ms

	clock_t t = clock(); // start timer

	// task should be running
	// abort task, call should return only after task is completed
	Cron_AbortTask(task_handle);

	t = clock() - t; // stop timer
	double time_taken_sec = ((double)t)/CLOCKS_PER_SEC;

	// expecting Cron_AbortTask to return before at-most 10 ms
	TEST_ASSERT(time_taken_sec < 0.01);
}

TEST_LIST = {
	{"cronExec", test_cronExec},
	{"cronAbort", test_cronAbort},
	{"cronLateAbort", test_cronLateAbort},
	{"MultiAbort", test_MultiAbort},
	{"abortNoneExistingTask", test_abortNoneExistingTask},
	{"AbortRunningTask", test_AbortRunningTask},
	{NULL, NULL}
};

