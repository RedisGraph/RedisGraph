/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "./priority_queue.h"
#include <stdlib.h>
#include <stdio.h>

/**
 * @brief  Initialize a Queue node, and its undelying CacheData.
 * @param  node: Node pointer.
 * @param  hashKey: Node's cache key (for CacheData).
 * @param  resultSet: Node's result value (for CacheData).
 * @retval Initialized Queue node (pointer)
 */
static QueueItem *_PriorityQueue_InitQueueItem(QueueItem *node, void *cacheValue, size_t dataSize,
											   QueueDataFreeFunc freeCB) {
	// If node was in use before, call its destructor.
	if(node->isDirty) {
		void *data = (void *)node->data;
		if(freeCB) freeCB(data);
	}
	// Set new value and mark as used.
	memcpy(&node->data, cacheValue, dataSize);
	node->isDirty = true;

	return node;
}

/**
 * @brief  Returns the size of node given a size of data value.
 * @param  dataSize:
 * @retval The size of the queue node.
 */
static inline size_t _PriorityQueue_SizeOfQueueItem(size_t dataSize) {
	return dataSize + sizeof(LinkedListNode) + sizeof(bool);
}

PriorityQueue *PriorityQueue_Create(size_t capacity, size_t dataSize, QueueDataFreeFunc freeCB) {
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
	// Queue hasn't reached full capacity, a linear insertion is possible.
	queue->stopLinearInsertion = false;
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
	array_append(queue->freeList, queue_item);
	// Reduce queue size.
	queue->size--;
	return queue_item->data;
}

void *PriorityQueue_Enqueue(PriorityQueue *queue, void *cacheValue) {
	if(PriorityQueue_IsFull(queue)) return NULL;
	// Init new node
	QueueItem *emptyNode;
	// See if nodes where removed from the queue.
	if(array_len(queue->freeList) > 0) {
		emptyNode = array_tail(queue->freeList);
		array_pop(queue->freeList);
	} else {
		emptyNode = queue->emptySpace;
	}

	QueueItem *node = _PriorityQueue_InitQueueItem(emptyNode, cacheValue, queue->dataSize,
												   (QueueDataFreeFunc)queue->freeCB);
	// Will be false until array is full - for linear insertion over the array.
	if(!queue->stopLinearInsertion && emptyNode == queue->emptySpace)
		queue->emptySpace = (QueueItem *)((char *)queue->emptySpace + _PriorityQueue_SizeOfQueueItem(
											  queue->dataSize));

	LinkedList_AddNode(&queue->linked_list, (LinkedListNode *) node);
	// Increase queue size.
	queue->size++;
	// Queue is full. Linear inseration is no longer an option.
	if(PriorityQueue_IsFull(queue))queue->stopLinearInsertion = true;

	return node->data;
}

static inline QueueItem *_PriorityQueue_GetNodeFromData(void *data, size_t dataSize) {
	size_t metaDataSize = _PriorityQueue_SizeOfQueueItem(dataSize) - dataSize;
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
