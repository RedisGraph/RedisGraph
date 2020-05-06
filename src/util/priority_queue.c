/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "./priority_queue.h"
#include <stdlib.h>
#include <stdio.h>

/**
 * @brief  Initialize a queue item.
 * @note   The data is being copied in the queue item.
 * @param  *node: Queue node
 * @param  *data: pointer to the data to be stored.
 * @param  dataSize: Size of the data.
 * @param  freeCB: Data callback.
 * @retval The queue node with its own copy of the data.
 */
static QueueItem *_PriorityQueue_InitQueueItem(QueueItem *node, void *data, uint dataSize,
											   QueueDataFreeFunc freeCB) {
	// If node was in use before, call its destructor.
	if(node->isDirty && freeCB) {
		freeCB((void *)node->data);
	}
	// Set new value and mark as used.
	memcpy(&node->data, data, dataSize);
	node->isDirty = true;
	return node;
}

/**
 * @brief  Returns the size of node given a size of data value.
 * @param  dataSize:
 * @retval The size of the queue node.
 */
static inline uint _PriorityQueue_SizeOfQueueItem(uint dataSize) {
	return dataSize + sizeof(LinkedListNode) + sizeof(bool);
	// uint queue_item_size = sizeof(QueueItem);
	// return dataSize + queue_item_size;
}

PriorityQueue *PriorityQueue_Create(uint capacity, uint dataSize, QueueDataFreeFunc freeCB) {
	// Memory allocations.
	PriorityQueue *queue = rm_malloc(sizeof(PriorityQueue));
	queue->buffer = rm_calloc(capacity, _PriorityQueue_SizeOfQueueItem(dataSize));
	queue->freeList = array_new(QueueItem *, capacity);
	queue->dataSize = dataSize;
	queue->freeCB = freeCB;
	// Maximal capacity.
	queue->capacity = capacity;
	// Initial size is 0.
	queue->size = 0;
	// First empty space is the first place in the queue array.
	queue->emptySpace = queue->buffer;
	LinkedList_Init(&queue->linked_list);
	return queue;
}

inline bool PriorityQueue_IsFull(const PriorityQueue *queue) {
	// Maximal capacity reached.
	return queue->capacity == queue->size;
}

inline bool PriorityQueue_IsEmpty(const PriorityQueue *queue) {
	return queue->size == 0;
}

inline void *PriorityQueue_Dequeue(PriorityQueue *queue) {
	// For empty queue return null.
	if(PriorityQueue_IsEmpty(queue)) return NULL;
	LinkedListNode *head = queue->linked_list.head;
	LinkedList_RemoveNode(&queue->linked_list, head);
	QueueItem *queue_item = (QueueItem *) head;
	queue->freeList = array_append(queue->freeList, queue_item);
	// Reduce queue size.
	queue->size--;
	return queue_item->data;
}

void *PriorityQueue_Enqueue(PriorityQueue *queue, void *cacheValue) {
	// TODO consider removing; impossible scenario for cache. (tested in unit test, though)
	if(PriorityQueue_IsFull(queue)) return NULL;
	// Init new node
	QueueItem *emptyNode;
	// Try to reuse a removed node if one is available.
	if(array_len(queue->freeList) > 0) {
		// If there are previously removed elements, get one from the list.
		emptyNode = array_pop(queue->freeList);
	} else {
		// There are no removed elements. We are in linear insertion mode, and queue->emptySpace is valid.
		assert(queue->emptySpace);
		emptyNode = queue->emptySpace;
	}

	QueueItem *node = _PriorityQueue_InitQueueItem(emptyNode, cacheValue, queue->dataSize,
												   (QueueDataFreeFunc)queue->freeCB);
	// Will be false until array is full - for linear insertion over the array.
	if(queue->emptySpace) {
		queue->emptySpace = (QueueItem *)((char *)queue->emptySpace + _PriorityQueue_SizeOfQueueItem(
											  queue->dataSize));
	}

	LinkedList_AddNode(&queue->linked_list, (LinkedListNode *) node);
	// Increase queue size.
	queue->size++;
	// Queue is full. Linear insertion is no longer an option.
	if(PriorityQueue_IsFull(queue)) queue->emptySpace = NULL;

	return node->data;
}

static inline QueueItem *_PriorityQueue_GetNodeFromData(void *data, uint dataSize) {
	uint metaDataSize = _PriorityQueue_SizeOfQueueItem(dataSize) - dataSize;
	QueueItem *node = (QueueItem *)((char *)data - metaDataSize);
	return node;
}

inline void PriorityQueue_DecreasePriority(PriorityQueue *queue, void *data) {
	QueueItem *node = _PriorityQueue_GetNodeFromData(data, queue->dataSize);
	LinkedList_MoveForward(&queue->linked_list, (LinkedListNode *)node);
}

inline void PriorityQueue_IncreasePriority(PriorityQueue *queue, void *data) {
	QueueItem *node = _PriorityQueue_GetNodeFromData(data, queue->dataSize);
	LinkedList_MoveBack(&queue->linked_list, (LinkedListNode *)node);
}

inline void PriorityQueue_AggressiveDemotion(PriorityQueue *queue, void *data) {
	QueueItem *node = _PriorityQueue_GetNodeFromData(data, queue->dataSize);
	LinkedList_MoveToHead(&queue->linked_list, (LinkedListNode *)node);
}

inline void PriorityQueue_AggressivePromotion(PriorityQueue *queue, void *data) {
	QueueItem *node = _PriorityQueue_GetNodeFromData(data, queue->dataSize);
	LinkedList_MoveToTail(&queue->linked_list, (LinkedListNode *)node);
}

inline void PriorityQueue_RemoveFromQueue(PriorityQueue *queue, void *data) {
	QueueItem *node = _PriorityQueue_GetNodeFromData(data, queue->dataSize);
	LinkedList_RemoveNode(&queue->linked_list, (LinkedListNode *)node);
	array_append(queue->freeList, node);
	queue->size--;
}

void PriorityQueue_Free(PriorityQueue *queue) {
	// Go over each entry and free its result set.
	while(queue->linked_list.head != NULL) {
		LinkedListNode *head = queue->linked_list.head;
		QueueItem *queue_item = (QueueItem *)head;
		void *data = (void *)queue_item->data;
		if(queue->freeCB) queue->freeCB(data);
		LinkedList_RemoveNode(&queue->linked_list, head);
	}

	if(queue->freeCB)
		for(int i = 0; i < array_len(queue->freeList); i++) queue->freeCB(queue->freeList[i]->data);

	// Memory release.
	array_free(queue->freeList);
	rm_free(queue->buffer);
	rm_free(queue);
}

