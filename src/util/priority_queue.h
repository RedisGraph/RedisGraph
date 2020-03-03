/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include "../util/arr.h"
#include <stdbool.h>

typedef void (*QueueDataFreeFunc)(void *);

/**
 * @brief  A struct that wraps cache data, for PriorityQueue usage
 */
typedef struct QueueNode {
	struct QueueNode *prev; // Next Node in the queue.
	struct QueueNode *next; // Previous node in the queue.
	bool isDirty;           // Indication for written entry, for memory release.
	char data[];            // Data to be stored in a queue node.
} QueueNode;


/**
 * @brief  Struct for priority queue.
 * The priority queue interface is implemented over an array of double linked list nodes.
 * The head is the node with the most priority, and the tail is the node with the lesser priority.
 * Nodes are removed by their priority.
 */
typedef struct PriorityQueue {
	QueueNode *buffer;          // Array of QueueNode
	size_t size;                // Current queue size
	size_t capacity;            // Maximum queue capacity
	QueueNode *head;            // Queue head
	QueueNode *tail;            // Queue tail
	QueueNode *emptySpace;      // Next empty place in the queue
	bool stopLinearInsertion;   // Indication if linear insertion is possible
	QueueNode **freeList;       // Contains previously removed nodes, for recycle.
	QueueDataFreeFunc freeCB;   // Node data free callback.
	size_t dataSize;            // Node data size.
} PriorityQueue;

/**
 * @brief  Initialize an empty priority queue with a given capacity.
 * @param  capacity: Queue's maximal capacity
 * @param  freeCB: freeCB: callback for freeing the stored values.
 *                 Note: if the original object is a nested compound object,
 *                       supply an appropriate function to avoid double resource releasing
 * @retval Initialized Queue (pointer).
 */
PriorityQueue *PriorityQueue_Create(size_t capacity, size_t dataSize, QueueDataFreeFunc freeCB);

#define PriorityQueue_New(capacity, T, T_freeCB) PriorityQueue_Create(capacity, sizeof(T), T_freeCB)

/**
 * @brief  Returns if the given queue is full.
 * @param  *queue: Priority Queue pointer.
 * @retval Returns if the given queue is full.
 */
bool PriorityQueue_IsFull(PriorityQueue *queue);

/**
 * @brief  Returns if the given queue is empty.
 * @param  *queue: Priority Queue pointer.
 * @retval Returns if the given queue is empty.
 */
bool PriorityQueue_IsEmpty(PriorityQueue *queue);

/**
 * @brief  Removes the lowest priority node from the queue.
 * @param  *queue: Priority Queue pointer.
 * @retval The removed item.
 */
void *PriorityQueue_Dequeue(PriorityQueue *queue);

/**
 * @brief  Enques a new node with given value.
 * @param  *queue: Priority Queue pointer.
 * @param  dataValue: New node's value.
 * @retval Pointer to the stored data item inside the queue.
 */
void *PriorityQueue_Enqueue(PriorityQueue *queue, void *dataValue);

/**
 * @brief  Empty a Priority Queue.
 * @param  *queue: PriorityQueue pointer.
 */
void PriorityQueue_EmptyQueue(PriorityQueue *queue);

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
void PriorityQueue_MoveToHead(PriorityQueue *queue, void *data);

/**
 * @brief  Moves a node to the tail of the Priority Queue.
 * @param  *queue: Priority Queue pointer.
 * @param  *node: data object address inside the queue.
 */
void PriorityQueue_MoveToTail(PriorityQueue *queue, void *data);

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
