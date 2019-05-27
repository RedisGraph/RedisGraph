/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "./lru_queue.h"
#include <stdlib.h>
#include <stdio.h>


/**
 * @brief  Initialize a LRU node, and its undelying CacheData
 * @note   
 * @param  node: Node's address (pointer)
 * @param  hashKey: Node's cache key (for CacheData)
 * @param  resultSet: Node's result value (for CacheData)
 * @retval Initilized LRU node (pointer)
 */
LRUNode *initLRUNode(LRUNode *node, hash_key_t const hashKey, void *cacheValue, cacheValueFreeFunc freeCB)
{
  // new node - no next and prev
  node->next = NULL;
  node->prev = NULL;
  // copy key
  node->cacheData.hashKey = hashKey;
  
  // if node was in use before, release the result set
  if (node->cacheData.isDirty)
  {
    freeCB(node->cacheData.cacheValue);
  }
  // set new value and mark as used
  node->cacheData.cacheValue = cacheValue;
  node->cacheData.isDirty = true;

  return node;
}


LRUQueue *initLRUQueue(LRUQueue *lruQueue, size_t capacity)
{
  // maximal capacity
  lruQueue->capacity = capacity;
  // initial size is 0
  lruQueue->size = 0;
  // no head or tail
  lruQueue->head = NULL;
  lruQueue->tail = NULL;
  // first empty space is the first place in the queue array
  lruQueue->emptySpace = lruQueue->buffer;
  // queue hasn't reaced full cacpacity, a linear insertion is possible
  lruQueue->stopLinearInsertion = false;
  array_clear(lruQueue->freeList);

  return lruQueue;
}

LRUQueue *LRUQueue_New(size_t capacity, cacheValueFreeFunc freeCB)
{
  // memory allocations
  LRUQueue *lruQueue = rm_malloc(sizeof(LRUQueue));
  lruQueue->buffer = rm_calloc(capacity, sizeof(LRUNode));
  lruQueue->freeList = array_new(LRUNode *, capacity);
  lruQueue->freeCB = freeCB;
  // initialization
  return initLRUQueue(lruQueue, capacity);
}

void LRUQueue_Free(LRUQueue *lruQueue)
{
  // go over each entry and free its result set
  while (lruQueue->head != NULL)
  {
    lruQueue->freeCB(lruQueue->head->cacheData.cacheValue);
    lruQueue->head = lruQueue->head->next;
  }

  for (int i = 0; i < array_len(lruQueue->freeList); i++)
  {
    lruQueue->freeCB(lruQueue->freeList[i]->cacheData.cacheValue);
  }

  // memeory release


  array_free(lruQueue->freeList);
  rm_free(lruQueue->buffer);
  rm_free(lruQueue);
}

bool isEmptyQueue(LRUQueue *queue)
{
  // no tail - nothing inside
  return queue->tail == NULL;
}
bool isFullQueue(LRUQueue *queue)
{
  // maximal capacity reached
  return queue->capacity == queue->size;
}

LRUNode *dequeue(LRUQueue *queue)
{
  // for empty queue return null
  if (isEmptyQueue(queue))
  {
    return NULL;
  }
  // one object in the queue
  if (queue->head == queue->tail)
  {
    // nullify head
    queue->head = NULL;
  }
  // get last object
  LRUNode *tmp = queue->tail;
  // move tail one object back
  queue->tail = tmp->prev;
  // if new tail is not null, make its next point to null
  if (queue->tail)
  {
    queue->tail->next = NULL;
  }
  // add evicted cell to empty cells list
  array_append(queue->freeList, tmp);
  // reduce queue size
  queue->size--;
  return tmp;
}

LRUNode *setNodeInQueue(LRUQueue *queue, LRUNode *newNode)
{
  // new node next is the queue (previous) head
  newNode->next = queue->head;
  if (isEmptyQueue(queue))
  {
    // empty queue - the new node is both head and tail
    queue->head = queue->tail = newNode;
  }
  else
  {
    // non empty queue, linke previous head with the new node and move head to point at node
    queue->head->prev = newNode;
    queue->head = newNode;
  }
  // increase queue size
  queue->size++;
  // queue is full. linear inseration is no longer an option
  if (isFullQueue(queue))
  {
    queue->stopLinearInsertion = true;
  }
  return newNode;
}

LRUNode *enqueue(LRUQueue *queue, hash_key_t const key, void *cacheValue)
{
  // init new node
  LRUNode *emptyNode;
  //see if nodes where removed from the queue
  if (array_len(queue->freeList) > 0)
  {
    emptyNode = array_tail(queue->freeList);
    array_pop(queue->freeList);
  }
  else
  {
    emptyNode = queue->emptySpace;
  }

  LRUNode *node = initLRUNode(emptyNode, key, cacheValue, queue->freeCB);
  //will be false until array is full - for linear insertion over the array
  if (!queue->stopLinearInsertion && emptyNode == queue->emptySpace)
  {
    queue->emptySpace++;
  }
  // put node in queue
  return setNodeInQueue(queue, node);
}

void moveToHead(LRUQueue *queue, LRUNode *node)
{
  if (node != queue->head)
  {
    // pull node out from its place (line its prev and next)
    node->prev->next = node->next;
    if (node->next)
    {
      node->next->prev = node->prev;
    }
    // in case node is tail,  move tail to point at its prev and nullfy tail's next
    if (node == queue->tail)
    {
      queue->tail = node->prev;
      queue->tail->next = NULL;
    }
    // put node in the head and link with previous head
    node->next = queue->head;
    node->prev = NULL;
    node->next->prev = node;
    queue->head = node;
  }
}

void emptyQueue(LRUQueue *queue)
{
  // move all pointers and meta data to their defualt values
  initLRUQueue(queue, queue->capacity);
}

void removeFromQueue(LRUQueue* queue, LRUNode *node)
{
  if (node->prev)
  {
    node->prev->next = node->next;
  }
  if (node->next)
  {
    node->next->prev = node->prev;
  }

  array_append(queue->freeList, node);
  queue->size--;
}