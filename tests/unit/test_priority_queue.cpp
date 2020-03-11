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
	static void SetUpTestCase() {// Use the malloc family for allocations
		Alloc_Reset();
	}
};

TEST_F(PriorityQueueTest, NewQueueTest) {
	PriorityQueue *priority_queue = PriorityQueue_New(QUEUE_SIZE, int, NULL);
	ASSERT_TRUE(priority_queue != NULL);
	ASSERT_TRUE(PriorityQueue_IsEmpty(priority_queue));
	PriorityQueue_Free(priority_queue);
}

TEST_F(PriorityQueueTest, SimpleEnqueueDequeue) {
	int i;
	PriorityQueue *priority_queue = PriorityQueue_New(QUEUE_SIZE, int, NULL);
	// Enqueue.
	for(i = 0; i < QUEUE_SIZE; i++)
		ASSERT_EQ(i, *(int *)PriorityQueue_Enqueue(priority_queue, &i));
	ASSERT_TRUE(PriorityQueue_IsFull(priority_queue));
	ASSERT_TRUE(PriorityQueue_Enqueue(priority_queue, &i) == NULL);
	// Dequeue.
	for(i = 0; i < QUEUE_SIZE; i++)
		ASSERT_EQ(i, *(int *)PriorityQueue_Dequeue(priority_queue));
	PriorityQueue_Free(priority_queue);

}

TEST_F(PriorityQueueTest, RemoveFromQueue) {
	int i;
	int *data_ptr;
	PriorityQueue *priority_queue = PriorityQueue_New(QUEUE_SIZE, int, NULL);
	// Enqueue.
	for(i = 0; i < QUEUE_SIZE - 1; i++) PriorityQueue_Enqueue(priority_queue, &i);
	data_ptr = (int *)PriorityQueue_Enqueue(priority_queue, &i);
	ASSERT_TRUE(PriorityQueue_IsFull(priority_queue));
	PriorityQueue_RemoveFromQueue(priority_queue, (void *)data_ptr);
	ASSERT_FALSE(PriorityQueue_IsFull(priority_queue));
	ASSERT_FALSE(PriorityQueue_IsEmpty(priority_queue));
	// Dequeue.
	for(i = 0; i < QUEUE_SIZE - 1; i++)
		ASSERT_EQ(i, *(int *)PriorityQueue_Dequeue(priority_queue));
	PriorityQueue_Free(priority_queue);
}

TEST_F(PriorityQueueTest, DecreasePriorityFullQueue) {
	int i;
	int *data_ptr;
	PriorityQueue *priority_queue = PriorityQueue_New(QUEUE_SIZE, int, NULL);
	// Enqueue.
	for(i = 0; i < QUEUE_SIZE - 1; i++) PriorityQueue_Enqueue(priority_queue, &i);
	data_ptr = (int *)PriorityQueue_Enqueue(priority_queue, &i);
	// Queue is [0,1,2,3,4,5,6,7,8,9]
	for(i = 0; i < QUEUE_SIZE - 1; i++)
		PriorityQueue_DecreasePriority(priority_queue, (void *)data_ptr);
	// Queue is [9,0,1,2,3,4,5,6,7,8]
	ASSERT_EQ(i, *(int *)PriorityQueue_Dequeue(priority_queue));
	// Dequeue.
	for(i = 0; i < QUEUE_SIZE - 1; i++)
		ASSERT_EQ(i, *(int *)PriorityQueue_Dequeue(priority_queue));
	PriorityQueue_Free(priority_queue);
}

TEST_F(PriorityQueueTest, MoveToTailFullQueue) {
	int i;
	int *data_ptr;
	PriorityQueue *priority_queue = PriorityQueue_New(QUEUE_SIZE, int, NULL);
	// Enqueue.
	for(i = 0; i < QUEUE_SIZE - 1; i++) PriorityQueue_Enqueue(priority_queue, &i);
	data_ptr = (int *)PriorityQueue_Enqueue(priority_queue, &i);
	// Queue is [0,1,2,3,4,5,6,7,8,9]
	PriorityQueue_AggressiveDemotion(priority_queue, data_ptr);
	// Queue is [9,0,1,2,3,4,5,6,7,8]
	ASSERT_EQ(i, *(int *)PriorityQueue_Dequeue(priority_queue));
	// Dequeue.
	for(i = 0; i < QUEUE_SIZE - 1; i++)
		ASSERT_EQ(i, *(int *)PriorityQueue_Dequeue(priority_queue));
	PriorityQueue_Free(priority_queue);
}

TEST_F(PriorityQueueTest, IncreasePriorityFullQueue) {
	int i;
	int *data_ptr;
	PriorityQueue *priority_queue = PriorityQueue_New(QUEUE_SIZE, int, NULL);
	// Enqueue.
	i = 0;
	data_ptr = (int *)PriorityQueue_Enqueue(priority_queue, &i);
	for(i = 1; i < QUEUE_SIZE; i++) PriorityQueue_Enqueue(priority_queue, &i);
	// Queue is [0,1,2,3,4,5,6,7,8,9]
	for(i = 0; i < QUEUE_SIZE - 1; i++)
		PriorityQueue_IncreasePriority(priority_queue, (void *)data_ptr);
	// Queue is [1,2,3,4,5,6,7,8,9,0]
	// Dequeue.
	for(i = 1; i < QUEUE_SIZE; i++)
		ASSERT_EQ(i, *(int *)PriorityQueue_Dequeue(priority_queue));
	ASSERT_EQ(0, *(int *)PriorityQueue_Dequeue(priority_queue));
	PriorityQueue_Free(priority_queue);
}

TEST_F(PriorityQueueTest, MoveToHeadFullQueue) {
	int i;
	int *data_ptr;
	PriorityQueue *priority_queue = PriorityQueue_New(QUEUE_SIZE, int, NULL);
	// Enqueue.
	i = 0;
	data_ptr = (int *)PriorityQueue_Enqueue(priority_queue, &i);
	for(i = 1; i < QUEUE_SIZE; i++) PriorityQueue_Enqueue(priority_queue, &i);
	// Queue is [0,1,2,3,4,5,6,7,8,9]
	PriorityQueue_AggressivePromotion(priority_queue, data_ptr);
	// Queue is [1,2,3,4,5,6,7,8,9,0]
	// Dequeue.
	for(i = 1; i < QUEUE_SIZE; i++)
		ASSERT_EQ(i, *(int *)PriorityQueue_Dequeue(priority_queue));
	ASSERT_EQ(0, *(int *)PriorityQueue_Dequeue(priority_queue));
	PriorityQueue_Free(priority_queue);
}

TEST_F(PriorityQueueTest, DecreasePriorityAfterRemoval) {
	int i;
	int *data_ptr_removed;
	int *data_ptr_modified;
	PriorityQueue *priority_queue = PriorityQueue_New(QUEUE_SIZE, int, NULL);
	i = 0;
	data_ptr_removed = (int *)PriorityQueue_Enqueue(priority_queue, &i);
	// Enqueue.
	for(i = 1; i < QUEUE_SIZE - 1; i++) PriorityQueue_Enqueue(priority_queue, &i);
	data_ptr_modified = (int *)PriorityQueue_Enqueue(priority_queue, &i);
	// Queue is [0,1,2,3,4,5,6,7,8,9]
	PriorityQueue_RemoveFromQueue(priority_queue, (void *)data_ptr_removed);
	// Queue is [1,2,3,4,5,6,7,8,9]
	for(i = 0; i < QUEUE_SIZE - 2; i++)
		PriorityQueue_DecreasePriority(priority_queue, (void *)data_ptr_modified);
	// Queue is [9,1,2,3,4,5,6,7,8]
	ASSERT_EQ(QUEUE_SIZE - 1, *(int *)PriorityQueue_Dequeue(priority_queue));
	// Dequeue.
	for(i = 1; i < QUEUE_SIZE - 2; i++)
		ASSERT_EQ(i, *(int *)PriorityQueue_Dequeue(priority_queue));
	PriorityQueue_Free(priority_queue);
}

TEST_F(PriorityQueueTest, MoveToTailAfterRemoval) {
	int i;
	int *data_ptr_removed;
	int *data_ptr_modified;
	PriorityQueue *priority_queue = PriorityQueue_New(QUEUE_SIZE, int, NULL);
	i = 0;
	data_ptr_removed = (int *)PriorityQueue_Enqueue(priority_queue, &i);
	// Enqueue.
	for(i = 1; i < QUEUE_SIZE - 1; i++) PriorityQueue_Enqueue(priority_queue, &i);
	data_ptr_modified = (int *)PriorityQueue_Enqueue(priority_queue, &i);
	// Queue is [0,1,2,3,4,5,6,7,8,9]
	PriorityQueue_RemoveFromQueue(priority_queue, (void *)data_ptr_removed);
	// Queue is [1,2,3,4,5,6,7,8,9]
	PriorityQueue_AggressiveDemotion(priority_queue, (void *)data_ptr_modified);
	// Queue is [9,1,2,3,4,5,6,7,8]
	ASSERT_EQ(*data_ptr_modified, *(int *)PriorityQueue_Dequeue(priority_queue));
	// Dequeue.
	for(i = 1; i < QUEUE_SIZE - 2; i++)
		ASSERT_EQ(i, *(int *)PriorityQueue_Dequeue(priority_queue));
	PriorityQueue_Free(priority_queue);
}

TEST_F(PriorityQueueTest, IncreasePriorityAfterRemoval) {
	int i;
	int *data_ptr_removed;
	int *data_ptr_modified;
	PriorityQueue *priority_queue = PriorityQueue_New(QUEUE_SIZE, int, NULL);
	i = 0;
	data_ptr_modified = (int *)PriorityQueue_Enqueue(priority_queue, &i);
	// Enqueue.
	for(i = 1; i < QUEUE_SIZE - 1; i++) PriorityQueue_Enqueue(priority_queue, &i);
	data_ptr_removed = (int *)PriorityQueue_Enqueue(priority_queue, &i);
	// Queue is [0,1,2,3,4,5,6,7,8,9]
	PriorityQueue_RemoveFromQueue(priority_queue, (void *)data_ptr_removed);
	// Queue is [0,1,2,3,4,5,6,7,8]
	for(i = 0; i < QUEUE_SIZE - 2; i++)
		PriorityQueue_IncreasePriority(priority_queue, (void *)data_ptr_modified);
	// Queue is [1,2,3,4,5,6,7,8,0]
	// Dequeue.
	for(i = 1; i < QUEUE_SIZE - 1; i++)
		ASSERT_EQ(i, *(int *)PriorityQueue_Dequeue(priority_queue));
	ASSERT_EQ(*data_ptr_modified, *(int *)PriorityQueue_Dequeue(priority_queue));
	PriorityQueue_Free(priority_queue);
}

TEST_F(PriorityQueueTest, MoveToHeadAfterRemoval) {
	int i;
	int *data_ptr_removed;
	int *data_ptr_modified;
	PriorityQueue *priority_queue = PriorityQueue_New(QUEUE_SIZE, int, NULL);
	i = 0;
	data_ptr_modified = (int *)PriorityQueue_Enqueue(priority_queue, &i);
	// Enqueue.
	for(i = 1; i < QUEUE_SIZE - 1; i++) PriorityQueue_Enqueue(priority_queue, &i);
	data_ptr_removed = (int *)PriorityQueue_Enqueue(priority_queue, &i);
	// Queue is [0,1,2,3,4,5,6,7,8,9]
	PriorityQueue_RemoveFromQueue(priority_queue, (void *)data_ptr_removed);
	// Queue is [0,1,2,3,4,5,6,7,8]
	PriorityQueue_AggressivePromotion(priority_queue, (void *)data_ptr_modified);
	// Queue is [1,2,3,4,5,6,7,8,0]
	// Dequeue.
	for(i = 1; i < QUEUE_SIZE - 1; i++)
		ASSERT_EQ(i, *(int *)PriorityQueue_Dequeue(priority_queue));
	ASSERT_EQ(*data_ptr_modified, *(int *)PriorityQueue_Dequeue(priority_queue));
	PriorityQueue_Free(priority_queue);
}

