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
static QueueNode *_PriorityQueue_InitQueueNode(QueueNode *node, void *cacheValue, size_t dataSize,
											   QueueDataFreeFunc freeCB) {
	// New node - no next and prev.
	node->next = NULL;
	node->prev = NULL;

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

static PriorityQueue *_PriorityQueue_InitPriorityQueue(PriorityQueue *queue, size_t capacity) {
	// Maximal capacity.
	queue->capacity = capacity;
	// Initial size is 0.
	queue->size = 0;
	// No head or tail.
	queue->head = NULL;
	queue->tail = NULL;
	// First empty space is the first place in the queue array.
	queue->emptySpace = queue->buffer;
	// Queue hasn't reached full capacity, a linear insertion is possible.
	queue->stopLinearInsertion = false;
	array_clear(queue->freeList);

	return queue;
}

/**
 * @brief  Returns the size of node given a size of data value.
 * @param  dataSize:
 * @retval The size of the queue node.
 */
static inline size_t _PriorityQueue_SizeOfQueueNode(size_t dataSize) {
	return dataSize + 2 * sizeof(QueueNode *) + sizeof(bool);
}

PriorityQueue *PriorityQueue_Create(size_t capacity, size_t dataSize, QueueDataFreeFunc freeCB) {
	// Memory allocations.
	PriorityQueue *queue = rm_malloc(sizeof(PriorityQueue));
	queue->buffer = rm_calloc(capacity, _PriorityQueue_SizeOfQueueNode(dataSize));
	queue->freeList = array_new(QueueNode *, capacity);
	queue->dataSize = dataSize;
	queue->freeCB = freeCB;
	// Initialization.
	return _PriorityQueue_InitPriorityQueue(queue, capacity);
}

inline bool PriorityQueue_IsFull(PriorityQueue *queue) {
	// Maximal capacity reached.
	return queue->capacity == queue->size;
}

inline bool PriorityQueue_IsEmpty(PriorityQueue *queue) {
	// No tail - nothing inside.
	return queue->tail == NULL;
}

void *PriorityQueue_Dequeue(PriorityQueue *queue) {
	// For empty queue return null.
	if(PriorityQueue_IsEmpty(queue)) return NULL;
	// One object in the queue.
	if(queue->head == queue->tail) queue->head = NULL;  // Nullify head.
	// Get last object.
	QueueNode *tmp = queue->tail;
	// Move tail one object back.
	queue->tail = tmp->prev;
	// If new tail is not null, make its next point to null.
	if(queue->tail) queue->tail->next = NULL;
	// Add evicted cell to empty cells list.
	array_append(queue->freeList, tmp);
	// Reduce queue size.
	queue->size--;
	return tmp->data;
}

QueueNode *_PriorityQueue_SetNodeInQueue(PriorityQueue *queue, QueueNode *newNode) {
	// New node next is the queue (previous) head.
	newNode->next = queue->head;
	if(PriorityQueue_IsEmpty(queue)) {
		// Empty queue - the new node is both head and tail.
		queue->head = queue->tail = newNode;
	} else {
		// Non empty queue, link previous head with the new node and move head to point at node.
		queue->head->prev = newNode;
		queue->head = newNode;
	}
	// Increase queue size.
	queue->size++;
	// Queue is full. Linear inseration is no longer an option.
	if(PriorityQueue_IsFull(queue))queue->stopLinearInsertion = true;
	return newNode;
}

void *PriorityQueue_Enqueue(PriorityQueue *queue, void *cacheValue) {
	if(PriorityQueue_IsFull(queue)) return NULL;
	// Init new node
	QueueNode *emptyNode;
	// See if nodes where removed from the queue.
	if(array_len(queue->freeList) > 0) {
		emptyNode = array_tail(queue->freeList);
		array_pop(queue->freeList);
	} else {
		emptyNode = queue->emptySpace;
	}

	QueueNode *node = _PriorityQueue_InitQueueNode(emptyNode, cacheValue, queue->dataSize,
												   (QueueDataFreeFunc)queue->freeCB);
	// Will be false until array is full - for linear insertion over the array.
	if(!queue->stopLinearInsertion && emptyNode == queue->emptySpace)
		queue->emptySpace = (QueueNode *)((char *)queue->emptySpace + _PriorityQueue_SizeOfQueueNode(
											  queue->dataSize));
	// Put node in queue.
	return (_PriorityQueue_SetNodeInQueue(queue, node)->data);
}

inline void PriorityQueue_EmptyQueue(PriorityQueue *queue) {
	// Move all pointers and meta data to their default values.
	_PriorityQueue_InitPriorityQueue(queue, queue->capacity);
}

static inline QueueNode *_PriorityQueue_GetNodeFromData(void *data, size_t dataSize) {
	size_t metaDataSize = _PriorityQueue_SizeOfQueueNode(dataSize) - dataSize;
	QueueNode *node = (QueueNode *)((char *)data - metaDataSize);
	return node;
}

void PriorityQueue_IncreasePriority(PriorityQueue *queue, void *data) {
	QueueNode *node = _PriorityQueue_GetNodeFromData(data, queue->dataSize);
	if(node != queue->head) {
		// Pull node out from its place (link its prev and next).
		node->prev->next = node->next;
		// Check if node is tail (next == NULL)
		if(node->next) node->next->prev = node->prev;
		else queue->tail = node->prev; // In case node is tail, move tail to point at its prev.

		node->next = node->prev;
		// Originally node->prev->prev
		node->prev = node->next->prev;
		node->next->prev = node;
		if(node->prev) node->prev->next = node;
		else queue->head = node;
	}
}

void PriorityQueue_DecreasePriority(PriorityQueue *queue, void *data) {
	QueueNode *node = _PriorityQueue_GetNodeFromData(data, queue->dataSize);
	if(node != queue->tail) {
		// Pull node out from its place (link its prev and next).
		node->next->prev = node->prev;
		// Check if node is head (prev == NULL)
		if(node->prev) node->prev->next = node->next;
		else queue->head = node->next; // In case node is head, move head to point at its next.

		node->prev = node->next;
		// Originally node->next->next
		node->next = node->prev->next;
		node->prev->next = node;
		if(node->next) node->next->prev = node;
		else queue->tail = node;
	}
}

void PriorityQueue_MoveToHead(PriorityQueue *queue, void *data) {
	QueueNode *node = _PriorityQueue_GetNodeFromData(data, queue->dataSize);
	if(node != queue->head) {
		// Pull node out from its place (link its prev and next).
		node->prev->next = node->next;
		// Check if node is tail (next == NULL)
		if(node->next) node->next->prev = node->prev;
		// In case node is tail,  move tail to point at its prev and nullify tail's next.
		if(node == queue->tail) {
			queue->tail = node->prev;
			queue->tail->next = NULL;
		}
		// Put node in the head and link with previous head.
		node->next = queue->head;
		node->prev = NULL;
		node->next->prev = node;
		queue->head = node;
	}
}

void PriorityQueue_MoveToTail(PriorityQueue *queue, void *data) {
	QueueNode *node = _PriorityQueue_GetNodeFromData(data, queue->dataSize);
	if(node != queue->tail) {
		// Pull node out from its place (link its prev and next).
		node->next->prev = node->prev;
		// Check if node is head (prev == NULL)
		if(node->prev) node->prev->next = node->next;
		else queue->head = node->next; // In case node is head, move head to point at its next.
		// Put node in the tail and link with previous tail.
		node->prev = queue->tail;
		node->next = NULL;
		node->prev->next = node;
		queue->tail = node;
	}
}

void PriorityQueue_RemoveFromQueue(PriorityQueue *queue, void *data) {
	QueueNode *node = _PriorityQueue_GetNodeFromData(data, queue->dataSize);
	if(node->prev) node->prev->next = node->next;
	if(node->next)node->next->prev = node->prev;
	if(node == queue->tail) queue->tail = node->prev;
	if(node == queue->head) queue->head = node->next;

	array_append(queue->freeList, node);
	queue->size--;
}

void PriorityQueue_Free(PriorityQueue *queue) {
	// Go over each entry and free its result set.
	while(queue->head != NULL) {
		void *data = (void *)queue->head->data;
		if(queue->freeCB) queue->freeCB(data);
		queue->head = queue->head->next;
	}

	if(queue->freeCB)
		for(int i = 0; i < array_len(queue->freeList); i++) queue->freeCB(queue->freeList[i]->data);

	// Memory release.
	array_free(queue->freeList);
	rm_free(queue->buffer);
	rm_free(queue);
}
