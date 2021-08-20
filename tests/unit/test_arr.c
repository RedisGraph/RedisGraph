/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "./acutest.h"
#include "../../src/util/rmalloc.h"
#include "../../src/util/arr.h"

void setup() {
	// use the malloc family for allocations
	Alloc_Reset();
}

int int_identity(int x) {
	return x;
}

void test_arr_clone_with_cb(void) {
	setup();

	int *arr = array_new(int, 10);
	for(int i = 0; i < 10; i++) array_append(arr, i);

	int *arr_clone;
	array_clone_with_cb(arr_clone, arr, int_identity);

	TEST_CHECK(array_len(arr) == array_len(arr_clone));
	for(int i = 0; i < 10; i++) {
		TEST_CHECK(arr[i] == arr_clone[i]);
	}

	array_free(arr);
	array_free(arr_clone);
}

TEST_LIST = {
	{ "arr_clone_with_cb", test_arr_clone_with_cb },
	{ NULL, NULL }     // zeroed record marking the end of the list
};

