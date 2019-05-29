/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "../util/arr.h"
#include <stdbool.h>

typedef void (*lruDataFreeFunc)(void *);

/**
 * @brief  A struct that wraps cache data, for LRUQueue usage
 */
typedef struct LRUNode {
  struct LRUNode *prev;                  // Next Node in the queue
  struct LRUNode *next;                  // Previous node in the queue
  bool isDirty;                          // Indication for written entry, for memeory release
  char data[];                           // data to be stored in a LRU node
} LRUNode;


/**
 * @brief  Struct for LRU queue. 
 * The LRU queue interface is implementd over an array of doubly linked list nodes
 */
typedef struct LRUQueue {
  LRUNode *buffer;              // Array of LRUnode
  size_t size;                  // Current queue size
  size_t capacity;              // Maximum queue capacity
  LRUNode *head;                // Queue head
  LRUNode *tail;                // Queue tail
  LRUNode *emptySpace;          // Next empty place in the queue
  bool stopLinearInsertion;         // Indication if linear insertion is possible
  LRUNode **freeList;
  lruDataFreeFunc freeCB;
  size_t dataSize;
} LRUQueue;

/**
 * @brief  Initilize an empty LRU queue with a given capacity 
 * @param  capacity: Queue's maximal cacpaicty
 * @param  freeCB: freeCB: callback for freeing the stored values. 
 *                 Note: if the original object is a nested compound object, 
 *                       supply an appropriate function to avoid double resource releasing
 * @retval Initialized Queue (pointer)
 */
LRUQueue *lruQueueNew(size_t capacity, size_t dataSize, lruDataFreeFunc freeCB);

/**
 * @brief  Returns if the given queue is full
 * @param  *queue: LRU Queue address
 * @retval Returns if the given queue is full
 */
bool lruQueueIsFull(LRUQueue *queue);

/**
 * @brief  Returns if the given queue is empty
 * @param  *queue: LRU Queue address
 * @retval Returns if the given queue is empty
 */
bool lruQueueIsEmpty(LRUQueue *queue);

/**
 * @brief  Removes the least recently used LRU Node from the queue
 * @param  *queue: LRU Queue address (pointer)
 * @retval The removed item
 */
void *lruQueueDequeue(LRUQueue *queue);

/**
 * @brief  Enqueues a new node with given value 
 * @param  *queue: LRU Queue address (pointer)
 * @param  dataValue: New node's value
 * @retval pointer to the stored data item inside the queue
 */
void *lruQueueEnqueue(LRUQueue *queue, void *dataValue);

/**
 * @brief  Emptys a LRU Queue
 * @param  *queue: LRUQueue address (poiner)
 * @retval None
 */
void lruQueueEmptyQueue(LRUQueue *queue);

/**
 * @brief  Moves a LRU Node to the head of the LRU Queue
 * @param  *queue: LRU Queue address
 * @param  *node: data object address inside the queue
 * @retval None
 */
void lruQueueMoveToHead(LRUQueue *queue, void *data);


/**
 * @brief  removes a single data value from the queue, regardless to its position
 * @note   
 * @param  *queue:  LRU Queue
 * @param  *data: data object address inside the queue to be removed
 * @retval None
 */
void lruQueueRemoveFromQueue(LRUQueue *queue, void *data);

/**
 * @brief  Destructor  
 * @param  *lruQueue: LRU Queue address (pointer) 
 * @retval None
 */
void lruQueueFree(LRUQueue *lruQueue);

/**
 * @brief  returns the size of lru node given a size of data value
 * @note   
 * @param  dataSize: 
 * @retval 
 */
size_t sizeOfLRUNode(size_t dataSize);