/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "acutest.h"
#include "../../src/util/arr.h"
#include "../../src/util/heap.h"
#include <time.h>
#include <stdlib.h>

static int cmp
(
	const void *a,
	const void *b,
	void *ud
) {
	int _a = *(int*)a;
	int _b = *(int*)b;

	return _b - _a;
	//return _a - _b;
}

static int qsort_cmp
(
	const void *a,
	const void *b
) {
	int _a = *(int*)a;
	int _b = *(int*)b;

	return _a - _b;
}

static void test_heapCreate(void) {
	void *elem;
	// create a new heap
	heap_t *heap = Heap_new(cmp, NULL);

	TEST_ASSERT(Heap_count(heap) == 0);  // expecting heap to be empty

	//--------------------------------------------------------------------------
	// try to peek top of heap
	//--------------------------------------------------------------------------

	elem = Heap_peek(heap);
	TEST_ASSERT(elem == NULL);  // expecting NULL for empty heap

	//--------------------------------------------------------------------------
	// try to get element from heap
	//--------------------------------------------------------------------------

	elem = Heap_poll(heap);
	TEST_ASSERT(elem == NULL);  // expecting NULL for empty heap

	//--------------------------------------------------------------------------
	// try to locate a none existing element
	//--------------------------------------------------------------------------

	int x = 2;
	TEST_ASSERT(Heap_contains_item(heap, &x) == 0);

	//--------------------------------------------------------------------------
	// try to remove a none existing element
	//--------------------------------------------------------------------------

	elem = Heap_remove_item(heap, &x);
	TEST_ASSERT(elem == NULL);

	// free heap
	Heap_free(heap);
}

static void test_heapPopulate(void) {
	// create a new heap
	heap_t *heap = Heap_new(cmp, NULL);

	int n = 10;
	int elements[n];

	//--------------------------------------------------------------------------
	// populate heap
	//--------------------------------------------------------------------------

	// insert elements 0..9
	for(int i = 0; i < n; i++) {
		elements[i] = i;
		TEST_ASSERT(Heap_offer(&heap, elements + i) == 0);
	}

	// validate number of elements in heap
	TEST_ASSERT(Heap_count(heap) == n);

	// validate expected elements are indeed in heap
	for(int i = 0; i < n; i++) {
		TEST_ASSERT(Heap_contains_item(heap, elements + i) == 1);
	}

	//--------------------------------------------------------------------------
	// empty heap
	//--------------------------------------------------------------------------

	// empty heap by polling until empty
	int *elem;
	for(int i = 0; i < n; i++) {
		elem = (int*)Heap_peek(heap);
		TEST_ASSERT(*elem == i);

		elem = Heap_poll(heap);
		TEST_ASSERT(*elem == i);
	}

	//--------------------------------------------------------------------------
	// validate heap is empty
	//--------------------------------------------------------------------------

	TEST_ASSERT(Heap_count(heap) == 0);

	// free heap
	Heap_free(heap);
}

static void test_heapPopulateRand(void) {
	// seed random
	srand(time(NULL));

	// create a new heap
	heap_t *heap = Heap_new(cmp, NULL);

	int n = 10;
	int elements[n];

	//--------------------------------------------------------------------------
	// generate n random numbers within the range 0..999
	//--------------------------------------------------------------------------

	// insert elements 0..n
	for(int i = 0; i < n; i++) {
		elements[i] = rand() % 1000;
		TEST_ASSERT(Heap_offer(&heap, elements + i) == 0);
	}

	// validate number of elements in heap
	TEST_ASSERT(Heap_count(heap) == n);

	// validate expected elements are indeed in heap
	for(int i = 0; i < n; i++) {
		TEST_ASSERT(Heap_contains_item(heap, elements + i) == 1);
	}

	//--------------------------------------------------------------------------
	// sort elements
	//--------------------------------------------------------------------------

	qsort(elements, n, sizeof(int), qsort_cmp);

	//--------------------------------------------------------------------------
	// validate heap poll ordering
	//--------------------------------------------------------------------------

	int *elem;
	for(int i = 0; i < n; i++) {
		elem = (int*)Heap_peek(heap);
		TEST_ASSERT(*elem == elements[i]);

		elem = Heap_poll(heap);
		TEST_ASSERT(*elem == elements[i]);
	}

	//--------------------------------------------------------------------------
	// validate heap is empty
	//--------------------------------------------------------------------------

	TEST_ASSERT(Heap_count(heap) == 0);

	// free heap
	Heap_free(heap);
}

static void test_heapRemoveElement(void) {
	// create a new heap
	heap_t *heap = Heap_new(cmp, NULL);

	//--------------------------------------------------------------------------
	// populate heap
	//--------------------------------------------------------------------------

	int n = 10;
	int elements[n];

	// insert elements 0..9
	for(int i = 0; i < n; i++) {
		elements[i] = i;
		TEST_ASSERT(Heap_offer(&heap, elements + i) == 0);
	}

	// validate number of elements in heap
	TEST_ASSERT(Heap_count(heap) == n);

	//--------------------------------------------------------------------------
	// remove heap head
	//--------------------------------------------------------------------------

	int *elem;
	elem = (int*)Heap_remove_item(heap, elements + 0);
	TEST_ASSERT(elem == elements + 0);

	// validate new head
	elem = (int*)Heap_peek(heap);
	TEST_ASSERT(*elem == 1);

	//--------------------------------------------------------------------------
	// remove 2 from heap
	//--------------------------------------------------------------------------

	elem = (int*)Heap_remove_item(heap, elements + 3);
	TEST_ASSERT(elem == elements + 3);

	// remove heap head
	elem = (int*)Heap_poll(heap);
	TEST_ASSERT(*elem == 1);

	// validate new head
	elem = (int*)Heap_peek(heap);
	TEST_ASSERT(*elem == 3);

	//--------------------------------------------------------------------------
	// remove last element
	//--------------------------------------------------------------------------

	elem = (int*)Heap_remove_item(heap, elements + 9);
	TEST_ASSERT(elem == elements + 9);

	// validate new head
	elem = (int*)Heap_peek(heap);
	TEST_ASSERT(*elem == 3);

	// free heap
	Heap_free(heap);
}

static void test_heapFuzz(void) {
	// seed random
	srand(time(NULL));

	// create a new heap
	heap_t *heap = Heap_new(cmp, NULL);

	int n = 1000;
	int *elements = array_new(int, 1000);

	// perform 2000 random operations:
	// 1. introduce a new random element
	// 2. remove a random element
	// 3. remove head
	// 
	// after each operation perform validation against head of heap
	for(int i = 0; i < 2000; i++) {
		int x;
		int idx;
		int elem;
		int op = rand() % 3;

		switch(op) {
			case 0:
				// introduce a new random element
				elem = rand() % 100;
				array_append(elements, elem);
				Heap_offer(&heap, elements + i);

				// sort elements
				qsort(elements, n, sizeof(int), qsort_cmp);
				break;
			case 1:
				// remove a random element
				idx = rand() & array_len(elements);
				Heap_remove_item(heap, elements + idx);
				array_del_fast(elements, idx);

				// sort elements
				qsort(elements, n, sizeof(int), qsort_cmp);
				break;
			case 2:
				Heap_poll(heap);
				break;
			default:
				break;
		}

		// validate head of heap contains the lowest value in elements array
		TEST_ASSERT(Heap_count(heap) == array_len(elements));
		if(Heap_count(heap) > 0) {
			x = (*(int*)Heap_peek(heap));
			TEST_ASSERT(x == elements[0]);
		}
	}

	// free heap
	Heap_free(heap);
}

TEST_LIST = {
	{"heapCreate", test_heapCreate},
	{"heapPopulate", test_heapPopulate},
	{"heapPopulateRand", test_heapPopulateRand},
	{"heapRemoveElement", test_heapRemoveElement},
	{"heapFuzz", test_heapFuzz},
	{NULL, NULL}
};

