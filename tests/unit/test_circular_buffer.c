/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "src/util/rmalloc.h"
#include "src/util/arr.h"
#include "src/util/circular_buffer.h"

void setup() {
	Alloc_Reset();
}

#define TEST_INIT setup();
#include "acutest.h"

void test_CircularBufferInit(void) {
	CircularBuffer buff = CircularBuffer_New(sizeof(int), 16, NULL);

	// a new circular buffer should be empty
	TEST_ASSERT(CircularBuffer_Empty(buff) == true);

	// item count of an empty circular buffer should be 0
	TEST_ASSERT(CircularBuffer_ItemCount(buff) == 0);

	// buffer should have available slots in it i.e. not full
	TEST_ASSERT(CircularBuffer_Full(buff) == false);

	// clean up
	CircularBuffer_Free(buff);
}

void test_CircularBufferPopulation(void) {
	int n;
	int cap = 16;
	CircularBuffer buff = CircularBuffer_New(sizeof(int), cap, NULL);

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
	CircularBuffer_Free(buff);
}

void test_CircularBuffer_Circularity(void) {
	int n;
	int cap = 16;
	CircularBuffer buff = CircularBuffer_New(sizeof(int), cap, NULL);

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
	CircularBuffer_Free(buff);
}

void _free_int
(
	void *item
) {
	free(item);
}

void test_CircularBuffer_free(void) {
	// -------------------------------------------------------------------------
	// fill a buffer of size 16 with int *
	// -------------------------------------------------------------------------
	uint cap = 16;
	CircularBuffer buff = CircularBuffer_New(sizeof(int *), cap, _free_int);
	for(int i = 0; i < cap; i++) {
		int *j = malloc(sizeof(int));
		*j = i;
		CircularBuffer_Add(buff, &j);
	}

	// -------------------------------------------------------------------------
	// free the buffer
	// -------------------------------------------------------------------------
	CircularBuffer_Free(buff);
}

void test_CircularBuffer_ForceWrite(void) {
	// -------------------------------------------------------------------------
	// fill a buffer of size 16 with 32 integers
	// -------------------------------------------------------------------------
	uint cap = 16;
	CircularBuffer buff = CircularBuffer_New(sizeof(int), cap, NULL);
	for(int i = 0; i < 2 * cap; i++) {
		CircularBuffer_AddForce(buff, &i);
	}

	// -------------------------------------------------------------------------
	// assert override correctness
	// -------------------------------------------------------------------------
	for(uint i = 0; i < 16; i++) {
		int item;
		int res = CircularBuffer_Remove(buff, &item);
		TEST_ASSERT(res == 1);
		TEST_ASSERT(item == (i + 16));
	}

	// -------------------------------------------------------------------------
	// free the buffer
	// -------------------------------------------------------------------------
	CircularBuffer_Free(buff);
}

void _assert_val_cb
(
	const void *item,
	void *user_data
) {
	int n = array_pop((int *)user_data);
	TEST_ASSERT(n == *(int *)item);
}

void test_CircularBuffer_traverseCallback(void) {
	uint cap = 16;
	CircularBuffer buff = CircularBuffer_New(sizeof(int), cap, NULL);

	// -------------------------------------------------------------------------
	// fill a circular buffer with 16 integers
	// -------------------------------------------------------------------------
	for(int i = 0; i < 16; i++) {
		CircularBuffer_Add(buff, &i);
	}

	// -------------------------------------------------------------------------
	// traverse the components of the buffer, asserting the values (via cb)
	// -------------------------------------------------------------------------
	int *arr = array_new(int, cap);
	for(int i = cap-1; i >= 0; i--) {
		array_append(arr, i);
	}
	CircularBuffer_TraverseCBFromLast(buff, cap, _assert_val_cb, arr);

	// -------------------------------------------------------------------------
	// free the buffer and auxilary array
	// -------------------------------------------------------------------------
	array_free(arr);
	CircularBuffer_Free(buff);
}

TEST_LIST = {
	{"CircularBuffer_Init", test_CircularBufferInit},
	{"CircularBuffer_Population", test_CircularBufferPopulation},
	{"CircularBuffer_Circularity", test_CircularBuffer_Circularity},
	{"CircularBuffer_Free", test_CircularBuffer_free},
	{"CircularBuffer_ForceWrite", test_CircularBuffer_ForceWrite},
	{"CircularBuffer_TraverseCB", test_CircularBuffer_traverseCallback},
	{NULL, NULL}
};
