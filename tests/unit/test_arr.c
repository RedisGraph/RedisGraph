/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "../../src/util/rmalloc.h"
#include "../../src/util/arr.h"

void setup() {
	Alloc_Reset();
}

#define TEST_INIT setup();
#include "acutest.h"

int int_identity(int x) {
	return x;
}

void test_arrCloneWithCB() {
	int *arr = array_new(int, 10);
	for(int i = 0; i < 10; i++) {
		array_append(arr, i);
	}

	int *arr_clone;
	array_clone_with_cb(arr_clone, arr, int_identity);

	TEST_ASSERT(array_len(arr) == array_len(arr_clone));

	for(int i = 0; i < 10; i++) {
		TEST_ASSERT(arr[i] == arr_clone[i]);
	}

	array_free(arr);
	array_free(arr_clone);
}

TEST_LIST = {
	{ "arrCloneWithCB", test_arrCloneWithCB},
	{ NULL, NULL }
};

