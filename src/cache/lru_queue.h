/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "cache_includes.h"
#include "../util/arr.h"

/**
 * @brief  A struct that wraps cache data, for LRUQueue usage
 */
typedef struct LRUNode {
  struct CacheData cacheData;     // CacheData
  struct LRUNode *prev;                  // Next Node in the queue
  struct LRUNode *next;                  // Previous node in the queue
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
  cacheValueFreeFunc freeCB;
} LRUQueue;

/**
 * @brief  Initilize an empty LRU queue with a given capacity 
 * @param  capacity: Queue's maximal cacpaicty
 * @param  freeCB: the call back to free stored values in the cache
 * @retval Initialized Queue (pointer)
 */
LRUQueue *LRUQueue_New(size_t capacity, cacheValueFreeFunc freeCB);

/**
 * @brief  Destructor  
 * @param  *lruQueue: LRU Queue address (pointer) 
 * @retval None
 */
void LRUQueue_Free(LRUQueue *lruQueue);

/**
 * @brief  Returns if the given queue is full
 * @param  *queue: LRU Queue address
 * @retval Returns if the given queue is full
 */
bool isFullQueue(LRUQueue *queue);

/**
 * @brief  Returns if the given queue is empty
 * @param  *queue: LRU Queue address
 * @retval Returns if the given queue is empty
 */
bool isEmptyQueue(LRUQueue *queue);

/**
 * @brief  Removes the least recently used LRU Node from the queue
 * @param  *queue: LRU Queue address (pointer)
 * @retval The removed LRU Node
 */
LRUNode *dequeue(LRUQueue *queue);

/**
 * @brief  Enqueues a new node with given key and value 
 * @param  *queue: LRU Queue address (pointer)
 * @param  *hashKey: New node's key - charecter array in size of HASH_KEY_LENGTH
 * @param  cacheValue: New node's value
 * @retval Newly genereted LRU Node, which is in the tail of the LRU queue
 */
LRUNode *enqueue(LRUQueue *queue, hash_key_t const key, void *cacheValue);

/**
 * @brief  Emptys a LRU Queue
 * @param  *queue: LRUQueue address (poiner)
 * @retval None
 */
void emptyQueue(LRUQueue *queue);

/**
 * @brief  Moves a LRU Node to the head of the LRU Queue
 * @param  *queue: LRU Queue address
 * @param  *node: LRU Node address 
 * @retval None
 */
void moveToHead(LRUQueue *queue, LRUNode *node);


/**
 * @brief  removes a single LRU Node from the queue, regardless to its position
 * @note   
 * @param  *queue:  LRU Queue
 * @param  *node: The node to be removed
 * @retval None
 */
void removeFromQueue(LRUQueue *queue, LRUNode *node);