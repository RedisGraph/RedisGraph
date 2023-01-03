/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "src/util/rmalloc.h"
#include "src/util/thpool/pools.h"
#include "src/configuration/config.h"

#include <assert.h>

#define READER_COUNT 4
#define WRITER_COUNT 1

void setup() {
	Alloc_Reset();
}

#define TEST_INIT setup();
#include "acutest.h"

static void get_thread_friendly_id(void *arg) {
	int *threadID = (int*)arg;
	*threadID = ThreadPools_GetThreadID();	
}

void test_threadPools_threadID() {
	ThreadPools_CreatePools(READER_COUNT, WRITER_COUNT, UINT64_MAX);

	// verify thread count equals to the number of reader and writer threads
	TEST_ASSERT(READER_COUNT + WRITER_COUNT == ThreadPools_ThreadCount());

	volatile int thread_ids[READER_COUNT + WRITER_COUNT + 1] = {-1, -1, -1, -1, -1, -1};

	// get main thread friendly id
	thread_ids[0] = ThreadPools_GetThreadID();

	// get reader threads friendly ids
	for(int i = 0; i < READER_COUNT; i++) {
		int offset = i + 1;
		TEST_ASSERT(0 == 
				ThreadPools_AddWorkReader(get_thread_friendly_id,
					(int*)(thread_ids + offset)));
	}

	// get writer threads friendly ids
	for(int i = 0; i < WRITER_COUNT; i++) {
		int offset = i + READER_COUNT + 1;
		TEST_ASSERT(0 ==
				ThreadPools_AddWorkWriter(get_thread_friendly_id,
					(int*)(thread_ids + offset), 0));
	}

	// wait for all threads
	for(int i = 0; i < READER_COUNT + WRITER_COUNT + 1; i++) {
		while(thread_ids[i] == -1) { i = i; }
	}

	// main thread
	int main_thread_id = 0;
	TEST_ASSERT(thread_ids[0] == main_thread_id);

	// reader thread ids should be > main thread id and < writer thread id
	for(int i = 0; i < READER_COUNT; i++) {
		TEST_ASSERT(thread_ids[i+1] >= main_thread_id);
		TEST_ASSERT(thread_ids[i+1] <= thread_ids[1+READER_COUNT]);
	}

	// writer thread ids should be > reader thread ids
	for(int i = 0; i < WRITER_COUNT; i++) {
		int offset = i + READER_COUNT + 1;
		TEST_ASSERT(thread_ids[offset] >= thread_ids[1]);
	}
}

TEST_LIST = {
	{"threadPools_threadID", test_threadPools_threadID},
	{NULL, NULL}
};

