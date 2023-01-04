/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "src/util/rmalloc.h"
#include "src/util/circular_buffer.h"

void setup() {
	Alloc_Reset();
}

#define TEST_INIT setup();
#include "acutest.h"

void test_CircularBufferInit(void) {
	CircularBuffer buff = CircularBuffer_New(sizeof(int), 16);

	// a new circular buffer should be empty
	TEST_ASSERT(CircularBuffer_Empty(buff) == true);

	// item count of an empty circular buffer should be 0
	TEST_ASSERT(CircularBuffer_ItemCount(buff) == 0);

	// buffer should have available slots in it i.e. not full
	TEST_ASSERT(CircularBuffer_Full(buff) == false);

	// clean up
	CircularBuffer_Free(&buff);
	TEST_ASSERT(buff == NULL);
}

void test_CircularBufferPopulation(void) {
	int n;
	int cap = 16;
	CircularBuffer buff = CircularBuffer_New(sizeof(int), cap);

	// remove item from an empty buffer should report failure
	TEST_ASSERT(CircularBuffer_Remove(buff, &n) == 0);

	//--------------------------------------------------------------------------
	// fill buffer
	//--------------------------------------------------------------------------
	for(int i = 0; i < cap; i++) {
		// make sure item was added
		TEST_ASSERT(CircularBuffer_Add(buff, &i) == 1);
		// validate buffer's item count
		TEST_ASSERT(CircularBuffer_ItemCount(buff) == i+1);
	}
	TEST_ASSERT(CircularBuffer_Full(buff) == true);

	// forcefully try to overflow buffer
	for(int i = 0; i < 10; i++) {
		TEST_ASSERT(CircularBuffer_Add(buff, &n) == 0);
	}

	//--------------------------------------------------------------------------
	// empty buffer
	//--------------------------------------------------------------------------
	for(int i = 0; i < cap; i++) {
		// get item from buffer
		TEST_ASSERT(CircularBuffer_Remove(buff, &n) == 1);

		// validate item's value
		TEST_ASSERT(n == i);
	}
	TEST_ASSERT(CircularBuffer_Empty(buff) == true);

	// forcefully try to remove an item from an empty buffer
	for(int i = 0; i < 10; i++) {
		TEST_ASSERT(CircularBuffer_Remove(buff, &n) == 0);
	}

	// clean up
	CircularBuffer_Free(&buff);
}

void test_CircularBuffer_Circularity(void) {
	int n;
	int cap = 16;
	CircularBuffer buff = CircularBuffer_New(sizeof(int), cap);

	//--------------------------------------------------------------------------
	// fill buffer
	//--------------------------------------------------------------------------
	for(int i = 0; i < cap; i++) {
		// make sure item was added
		TEST_ASSERT(CircularBuffer_Add(buff, &i) == 1);
	}
	TEST_ASSERT(CircularBuffer_Full(buff) == true);

	// try to overflow buffer
	TEST_ASSERT(CircularBuffer_Add(buff, &n) == 0);

	// removing an item should make space in the buffer
	TEST_ASSERT(CircularBuffer_Remove(buff, &n) == 1);
	TEST_ASSERT(CircularBuffer_Add(buff, &n) == 1);

	//--------------------------------------------------------------------------
	// clear buffer
	//--------------------------------------------------------------------------
	while(CircularBuffer_Empty(buff) == false) {
		CircularBuffer_Remove(buff, &n);
	}

	// add/remove elements cycling through the buffer multiple times
	for(int i = 0; i < cap * 4; i++) {
		TEST_ASSERT(CircularBuffer_Add(buff, &i) == 1);
		TEST_ASSERT(CircularBuffer_Remove(buff, &n) == 1);
		TEST_ASSERT(n == i);
	}
	TEST_ASSERT(CircularBuffer_Empty(buff) == true);

	// clean up
	CircularBuffer_Free(&buff);
}

TEST_LIST = {
	{"CircularBuffer_Init", test_CircularBufferInit},
	{"CircularBuffer_Population", test_CircularBufferPopulation},
	{"CircularBuffer_Circularity", test_CircularBuffer_Circularity},
	{NULL, NULL}
};

