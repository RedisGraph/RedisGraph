/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "gtest.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "assert.h"
#include "../../src/util/rmalloc.h"
#include "../../src/util/thpool/pools.h"
#include "../../src/configuration/config.h"

#ifdef __cplusplus
}
#endif

#define READER_COUNT 4
#define WRITER_COUNT 1

class ThreadPoolsTest: public ::testing::Test {
	protected:
	// Use the malloc family for allocations
	static void SetUpTestCase() {
		Alloc_Reset();
		ThreadPools_CreatePools(READER_COUNT, WRITER_COUNT, UINT64_MAX);
	}

	static void get_thread_friendly_id(void *arg) {
		int *threadID = (int*)arg;
		*threadID = ThreadPools_GetThreadID();	
	}
};

TEST_F(ThreadPoolsTest, ThreadPools_ThreadID) {
	// verify thread count equals to the number of reader and writer threads
	ASSERT_EQ (READER_COUNT + WRITER_COUNT, ThreadPools_ThreadCount());

	int thread_ids[READER_COUNT + WRITER_COUNT + 1] = {-1, -1, -1, -1, -1, -1};

	// get main thread friendly id
	thread_ids[0] = ThreadPools_GetThreadID();

	// get reader threads friendly ids
	for(int i = 0; i < READER_COUNT; i++) {
		int offset = i + 1;
		ASSERT_EQ(0,
				ThreadPools_AddWorkReader(get_thread_friendly_id,
					thread_ids + offset));
	}

	// get writer threads friendly ids
	for(int i = 0; i < WRITER_COUNT; i++) {
		int offset = i + READER_COUNT + 1;
		ASSERT_EQ(0,
				ThreadPools_AddWorkWriter(get_thread_friendly_id,
					thread_ids + offset));
	}

	// wait for all threads
	for(int i = 0; i < READER_COUNT + WRITER_COUNT + 1; i++) {
		while(thread_ids[i] == -1) { i = i; }
	}

	// main thread
	int main_thread_id = 0;
	ASSERT_EQ(thread_ids[0], main_thread_id);

	// reader thread ids should be > main thread id and < writer thread id
	for(int i = 0; i < READER_COUNT; i++) {
		ASSERT_GT(thread_ids[i+1], main_thread_id);
		ASSERT_LT(thread_ids[i+1], thread_ids[1+READER_COUNT]);
	}

	// writer thread ids should be > reader thread ids
	for(int i = 0; i < WRITER_COUNT; i++) {
		int offset = i + READER_COUNT + 1;
		ASSERT_GT(thread_ids[offset], thread_ids[1]);
	}
}

