/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include "arr.h"
#include "linked_list.h"
#include <stdbool.h>

// Signature of callback for freeing the stored values given by the user.
typedef void (*QueueDataFreeFunc)(void *);

/**
 * @brief  A struct that wraps cache data, for PriorityQueue usage
 */
typedef struct QueueItem {
	LinkedListNode linked_list_node;
	bool isDirty;           // Indication for written entry, for memory release.
	char data[];            // Data to be stored in a queue node.
} QueueItem;

/**
 * @brief  Struct for priority queue.
 * The priority queue interface is implemented over an array of double linked list nodes.
 * The tail is the node with the highest priority, and the head is the node with the lowest.
 * Nodes are removed by their priority.
 */
typedef struct PriorityQueue {
	size_t size;                // Current queue size
	size_t capacity;            // Maximum queue capacity
	LinkedList linked_list;     // The underlying data structure.
	QueueItem *emptySpace;      // Next empty place in the queue
	bool stopLinearInsertion;   // Indication if linear insertion is possible
	QueueItem **freeList;       // Contains previously removed nodes, for recycle.
	QueueDataFreeFunc freeCB;   // Node data free callback.
	size_t dataSize;            // Node data size.
	QueueItem *buffer;          // Array of QueueItem
} PriorityQueue;

/**
 * @brief  Initialize an empty priority queue with a given capacity.
 * @param  capacity: Queue's maximal capacity
 * @param  dataSize: The size of the stored data item.
 * @param  freeCB: freeCB: callback for freeing a stored value.
 * @retval Initialized Queue (pointer).
 */
PriorityQueue *PriorityQueue_Create(size_t capacity, size_t dataSize, QueueDataFreeFunc freeCB);

#define PriorityQueue_New(capacity, T, T_freeCB) PriorityQueue_Create(capacity, sizeof(T), T_freeCB)

/**
 * @brief  Returns true if the given queue is full.
 * @param  *queue: Priority Queue pointer.
 * @retval Returns if the given queue is full.
 */
bool PriorityQueue_IsFull(const PriorityQueue *queue);

/**
 * @brief  Returns true if the given queue is empty.
 * @param  *queue: Priority Queue pointer.
 * @retval Returns if the given queue is empty.
 */
bool PriorityQueue_IsEmpty(const PriorityQueue *queue);

/**
 * @brief  Removes the lowest priority node from the queue.
 * @param  *queue: Priority Queue pointer.
 * @retval The removed item, or NULL incase the queue is empty.
 */
void *PriorityQueue_Dequeue(PriorityQueue *queue);

/**
 * @brief  Enques a new node with given value.
 * @param  *queue: Priority Queue pointer.
 * @param  dataValue: New node's value.
 * @retval Pointer to the stored data item inside the queue. NULL if the queue is full.
 */
void *PriorityQueue_Enqueue(PriorityQueue *queue, void *dataValue);

/**
 * @brief  Increases the priority of a node in Priority Queue.
 * @param  *queue: Priority Queue pointer.
 * @param  *node: data object address inside the queue.
 */
void PriorityQueue_IncreasePriority(PriorityQueue *queue, void *data);

/**
 * @brief  Decreases the priority of a node in Priority Queue..
 * @param  *queue: Priority Queue pointer.
 * @param  *node: data object address inside the queue.
 */
void PriorityQueue_DecreasePriority(PriorityQueue *queue, void *data);

/**
 * @brief  Moves a node to the head of the Priority Queue.
 * @param  *queue: Priority Queue pointer.
 * @param  *node: data object address inside the queue.
 */
void PriorityQueue_AggressiveDemotion(PriorityQueue *queue, void *data);

/**
 * @brief  Moves a node to the tail of the Priority Queue.
 * @param  *queue: Priority Queue pointer.
 * @param  *node: data object address inside the queue.
 */
void PriorityQueue_AggressivePromotion(PriorityQueue *queue, void *data);

/**
 * @brief  Removes a single data value from the queue, regardless to its priority.
 * @param  *queue: Priority Queue.
 * @param  *data: data object address inside the queue to be removed.
 */
void PriorityQueue_RemoveFromQueue(PriorityQueue *queue, void *data);

/**
 * @brief  Destructor
 * @param  *queue: Priority Queue pointer.
 */
void PriorityQueue_Free(PriorityQueue *queue);
