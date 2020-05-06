#include "gtest.h"

#ifdef __cplusplus
extern "C" {
#endif
#include "../../src/util/rmalloc.h"
#include "../../src/util/priority_queue.h"
#ifdef __cplusplus
}
#endif

#define QUEUE_SIZE 10

class PriorityQueueTest:
	public ::testing::Test {
  protected:
	static void SetUpTestCase() { // Use the malloc family for allocations
		Alloc_Reset();
	}
};

TEST_F(PriorityQueueTest, NewQueueTest) {
	PriorityQueue *priority_queue = PriorityQueue_Create(QUEUE_SIZE, sizeof(int), NULL);
	ASSERT_TRUE(priority_queue != NULL);
	ASSERT_TRUE(PriorityQueue_IsEmpty(priority_queue));
	PriorityQueue_Free(priority_queue);
}

// This test validates data integrity over simple insertion and evection for the queue in FIFO manner.
TEST_F(PriorityQueueTest, SimpleEnqueueDequeue) {
	int i;
	PriorityQueue *priority_queue = PriorityQueue_Create(QUEUE_SIZE, sizeof(int), NULL);
	// Enqueue the range [0,9].
	for(i = 0; i < QUEUE_SIZE; i++) {
		ASSERT_EQ(i, *(int *)PriorityQueue_Enqueue(priority_queue, &i));
	}
	ASSERT_TRUE(PriorityQueue_IsFull(priority_queue));
	ASSERT_TRUE(PriorityQueue_Enqueue(priority_queue, &i) == NULL);
	// Dequeue. Validate the range [0,9]
	for(i = 0; i < QUEUE_SIZE; i++) {
		ASSERT_EQ(i, *(int *)PriorityQueue_Dequeue(priority_queue));
	}
	PriorityQueue_Free(priority_queue);
}

// This test validates data integrity after removing an object from the queue.
TEST_F(PriorityQueueTest, RemoveFromQueue) {
	int i;
	int *data_ptr;
	PriorityQueue *priority_queue = PriorityQueue_Create(QUEUE_SIZE, sizeof(int), NULL);
	// Enqueue the range [0,9]. Save the last value in data_ptr.
	for(i = 0; i < QUEUE_SIZE; i++) {
		data_ptr = (int *)PriorityQueue_Enqueue(priority_queue, &i);
	}
	ASSERT_TRUE(PriorityQueue_IsFull(priority_queue));
	PriorityQueue_RemoveFromQueue(priority_queue, (void *)data_ptr);
	ASSERT_FALSE(PriorityQueue_IsFull(priority_queue));
	ASSERT_FALSE(PriorityQueue_IsEmpty(priority_queue));
	// Dequeue.
	for(i = 0; i < QUEUE_SIZE - 1; i++) {
		ASSERT_EQ(i, *(int *)PriorityQueue_Dequeue(priority_queue));
	}
	PriorityQueue_Free(priority_queue);
}

// This test validates the data integrity after decreasing priority of a stored object, multiple times.
TEST_F(PriorityQueueTest, DecreasePriorityFullQueue) {
	int i;
	int *data_ptr;
	PriorityQueue *priority_queue = PriorityQueue_Create(QUEUE_SIZE, sizeof(int), NULL);
	// Enqueue the range [0,9]. Save the last value in data_ptr.
	for(i = 0; i < QUEUE_SIZE; i++) {
		data_ptr = (int *)PriorityQueue_Enqueue(priority_queue, &i);
	}
	// Queue is [0,1,2,3,4,5,6,7,8,9]. Decrease the priority of the last element 9 times.
	for(i = 0; i < QUEUE_SIZE - 1; i++) {
		PriorityQueue_DecreasePriority(priority_queue, (void *)data_ptr);
	}
	// Queue is [9,0,1,2,3,4,5,6,7,8]. Validate that the least prioritized item is the head of the queue.
	ASSERT_EQ(data_ptr, PriorityQueue_Dequeue(priority_queue));
	// Dequeue everything.
	for(i = 0; i < QUEUE_SIZE - 1; i++) {
		ASSERT_EQ(i, *(int *)PriorityQueue_Dequeue(priority_queue));
	}
	PriorityQueue_Free(priority_queue);
}

// This test validates the data integrity after an aggressive decreasing priority of a stored object.
TEST_F(PriorityQueueTest, AgressiveDecreasePriorityFullQueue) {
	int i;
	int *data_ptr;
	PriorityQueue *priority_queue = PriorityQueue_Create(QUEUE_SIZE, sizeof(int), NULL);
	// Enqueue the range [0,9]. Save the last value in data_ptr.
	for(i = 0; i < QUEUE_SIZE; i++) {
		data_ptr = (int *)PriorityQueue_Enqueue(priority_queue, &i);
	}
	// Queue is [0,1,2,3,4,5,6,7,8,9]. Aggressively decrease the priority of the last element.
	PriorityQueue_AggressiveDemotion(priority_queue, data_ptr);
	// Queue is [9,0,1,2,3,4,5,6,7,8]. Validate that the least prioritized item is the head of the queue.
	ASSERT_EQ(data_ptr, PriorityQueue_Dequeue(priority_queue));
	// Dequeue.
	for(i = 0; i < QUEUE_SIZE - 1; i++) {
		ASSERT_EQ(i, *(int *)PriorityQueue_Dequeue(priority_queue));
	}
	PriorityQueue_Free(priority_queue);
}

// This test validates the data integrity after increasing priority of a stored object, multiple times.
TEST_F(PriorityQueueTest, IncreasePriorityFullQueue) {
	int i;
	int *data_ptr;
	PriorityQueue *priority_queue = PriorityQueue_Create(QUEUE_SIZE, sizeof(int), NULL);
	// Enqueue first entry and save it in data_ptr.
	i = 0;
	data_ptr = (int *)PriorityQueue_Enqueue(priority_queue, &i);
	// Enqueue the range [1-9]
	for(i = 1; i < QUEUE_SIZE; i++) {
		PriorityQueue_Enqueue(priority_queue, &i);
	}
	// Queue is [0,1,2,3,4,5,6,7,8,9]. Increase the priority of the first element 9 times.
	for(i = 0; i < QUEUE_SIZE - 1; i++) {
		PriorityQueue_IncreasePriority(priority_queue, (void *)data_ptr);
	}
	// Queue is [1,2,3,4,5,6,7,8,9,0]. Dequeue all elements except for the element with the highest priority.
	for(i = 1; i < QUEUE_SIZE; i++) {
		ASSERT_EQ(i, *(int *)PriorityQueue_Dequeue(priority_queue));
	}
	// Expected head/tail (single item) == 0
	ASSERT_EQ(0, *(int *)PriorityQueue_Dequeue(priority_queue));
	PriorityQueue_Free(priority_queue);
}

// This test validates the data integrity after agressive increasing priority of a stored object.
TEST_F(PriorityQueueTest, AgressiveIncreasePriorityFullQueue) {
	int i;
	int *data_ptr;
	PriorityQueue *priority_queue = PriorityQueue_Create(QUEUE_SIZE, sizeof(int), NULL);
	// Enqueue first entry and save it in data_ptr.
	i = 0;
	data_ptr = (int *)PriorityQueue_Enqueue(priority_queue, &i);
	// Enqueue the range [1-9].
	for(i = 1; i < QUEUE_SIZE; i++) {
		PriorityQueue_Enqueue(priority_queue, &i);
	}
	// Queue is [0,1,2,3,4,5,6,7,8,9]. Agressive Increase the priority of the first element.
	PriorityQueue_AggressivePromotion(priority_queue, data_ptr);
	// Queue is [1,2,3,4,5,6,7,8,9,0]. Dequeue all elements except for the element with the highest priority.
	for(i = 1; i < QUEUE_SIZE; i++) {
		ASSERT_EQ(i, *(int *)PriorityQueue_Dequeue(priority_queue));
	}
	// Expected head/tail (single item) == 0
	ASSERT_EQ(0, *(int *)PriorityQueue_Dequeue(priority_queue));
	PriorityQueue_Free(priority_queue);
}

// This test validates the data integrity after decreasing priority of a stored object, multiple times, after another item has been removed.
TEST_F(PriorityQueueTest, DecreasePriorityAfterRemoval) {
	int i;
	int *data_ptr_removed;
	int *data_ptr_modified;
	PriorityQueue *priority_queue = PriorityQueue_Create(QUEUE_SIZE, sizeof(int), NULL);
	// Enqueue first entry and save it in data_ptr_removed, for later removal.
	i = 0;
	data_ptr_removed = (int *)PriorityQueue_Enqueue(priority_queue, &i);
	// Enqueue the range [1-9]. Save the last entry at data_ptr_modified.
	for(i = 1; i < QUEUE_SIZE; i++) {
		data_ptr_modified = (int *)PriorityQueue_Enqueue(priority_queue, &i);
	}
	// Queue is [0,1,2,3,4,5,6,7,8,9], remove the entry at data_ptr_removed (0).
	PriorityQueue_RemoveFromQueue(priority_queue, (void *)data_ptr_removed);
	// Queue is [1,2,3,4,5,6,7,8,9]. Decrease the priority of the last element 8 times.
	for(i = 0; i < QUEUE_SIZE - 2; i++) {
		PriorityQueue_DecreasePriority(priority_queue, (void *)data_ptr_modified);
	}
	// Queue is [9,1,2,3,4,5,6,7,8].  Validate that the least prioritized item is the head of the queue.
	ASSERT_EQ(data_ptr_modified, PriorityQueue_Dequeue(priority_queue));
	// Dequeue.
	for(i = 1; i < QUEUE_SIZE - 2; i++) {
		ASSERT_EQ(i, *(int *)PriorityQueue_Dequeue(priority_queue));
	}
	PriorityQueue_Free(priority_queue);
}

// This test validates the data integrity after aggressively decreasing priority of a stored object, after another item has been removed.
TEST_F(PriorityQueueTest, AggressiveDecreasePriorityAfterRemoval) {
	int i;
	int *data_ptr_removed;
	int *data_ptr_modified;
	PriorityQueue *priority_queue = PriorityQueue_Create(QUEUE_SIZE, sizeof(int), NULL);
	// Enqueue first entry and save it in data_ptr_removed, for later removal.
	i = 0;
	data_ptr_removed = (int *)PriorityQueue_Enqueue(priority_queue, &i);
	// Enqueue the range [1-9]. Save the last entry at data_ptr_modified.
	for(i = 1; i < QUEUE_SIZE; i++) {
		data_ptr_modified = (int *)PriorityQueue_Enqueue(priority_queue, &i);
	}
	// Queue is [0,1,2,3,4,5,6,7,8,9], remove the entry at data_ptr_removed (0).
	PriorityQueue_RemoveFromQueue(priority_queue, (void *)data_ptr_removed);
	// Queue is [1,2,3,4,5,6,7,8,9]. Aggressively decrease the priority of the last element.
	PriorityQueue_AggressiveDemotion(priority_queue, data_ptr_modified);
	// Queue is [9,1,2,3,4,5,6,7,8].  Validate that the least prioritized item is the head of the queue.
	ASSERT_EQ(data_ptr_modified, PriorityQueue_Dequeue(priority_queue));
	// Dequeue.
	for(i = 1; i < QUEUE_SIZE - 2; i++) {
		ASSERT_EQ(i, *(int *)PriorityQueue_Dequeue(priority_queue));
	}
	PriorityQueue_Free(priority_queue);
}

// This test validates the data integrity after increasing priority of a stored object, multiple times, after another item has been removed.
TEST_F(PriorityQueueTest, IncreasePriorityAfterRemoval) {
	int i;
	int *data_ptr_removed;
	int *data_ptr_modified;
	PriorityQueue *priority_queue = PriorityQueue_Create(QUEUE_SIZE, sizeof(int), NULL);
	// Enqueue first entry and save it in data_ptr_modified, for later modification.
	i = 0;
	data_ptr_modified = (int *)PriorityQueue_Enqueue(priority_queue, &i);
	// Enqueue the range [1-9]. Save the last entry at data_ptr_removed.
	for(i = 1; i < QUEUE_SIZE; i++) {
		data_ptr_removed = (int *)PriorityQueue_Enqueue(priority_queue, &i);
	}
	// Queue is [0,1,2,3,4,5,6,7,8,9], remove the entry at data_ptr_removed (9).
	PriorityQueue_RemoveFromQueue(priority_queue, (void *)data_ptr_removed);
	// Queue is [0,1,2,3,4,5,6,7,8]. Increase the priority of the first element 8 times.
	for(i = 0; i < QUEUE_SIZE - 2; i++) {
		PriorityQueue_IncreasePriority(priority_queue, (void *)data_ptr_modified);
	}
	// Queue is [1,2,3,4,5,6,7,8,0]
	// Dequeue all items other than the highest priority item.
	for(i = 1; i < QUEUE_SIZE - 1; i++) {
		ASSERT_EQ(i, *(int *)PriorityQueue_Dequeue(priority_queue));
	}
	// Expected head/tail (single item) == 0
	ASSERT_EQ(data_ptr_modified, PriorityQueue_Dequeue(priority_queue));
	PriorityQueue_Free(priority_queue);
}

// This test validates the data integrity after aggressively increasing priority of a stored object, after another item has been removed.
TEST_F(PriorityQueueTest, AgressiveIncreasePriorityAfterRemoval) {
	int i;
	int *data_ptr_removed;
	int *data_ptr_modified;
	PriorityQueue *priority_queue = PriorityQueue_Create(QUEUE_SIZE, sizeof(int), NULL);
	// Enqueue first entry and save it in data_ptr_modified, for later modification.
	i = 0;
	data_ptr_modified = (int *)PriorityQueue_Enqueue(priority_queue, &i);
	// Enqueue the range [1-9]. Save the last entry at data_ptr_removed.
	for(i = 1; i < QUEUE_SIZE; i++) {
		data_ptr_removed = (int *)PriorityQueue_Enqueue(priority_queue, &i);
	}
	// Queue is [0,1,2,3,4,5,6,7,8,9], remove the entry at data_ptr_removed (9).
	PriorityQueue_RemoveFromQueue(priority_queue, (void *)data_ptr_removed);
	// Queue is [0,1,2,3,4,5,6,7,8]. Aggressively increase the priority of the first element.
	PriorityQueue_AggressivePromotion(priority_queue, (void *)data_ptr_modified);
	// Queue is [1,2,3,4,5,6,7,8,0]
	// Dequeue all items other than the highest priority item.
	for(i = 1; i < QUEUE_SIZE - 1; i++) {
		ASSERT_EQ(i, *(int *)PriorityQueue_Dequeue(priority_queue));
	}
	// Expected head/tail (single item) == 0
	ASSERT_EQ(data_ptr_modified, PriorityQueue_Dequeue(priority_queue));
	PriorityQueue_Free(priority_queue);
}

