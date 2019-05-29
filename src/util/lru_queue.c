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
LRUNode *initLRUNode(LRUNode *node, void *cacheValue, size_t dataSize, lruDataFreeFunc freeCB)
{
  // new node - no next and prev
  node->next = NULL;
  node->prev = NULL;

  // if node was in use before, call its destructor
  if (node->isDirty)
  {
    void *data = (void *)node->data;
    freeCB(data);
  }
  // set new value and mark as used
  memcpy(&node->data, cacheValue, dataSize);
  node->isDirty = true;

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

size_t sizeOfLRUNode(size_t dataSize)
{
  return dataSize + 2 * sizeof(LRUNode *) + sizeof(bool);
}

LRUQueue *lruQueueNew(size_t capacity, size_t dataSize, lruDataFreeFunc freeCB)
{
  // memory allocations
  LRUQueue *lruQueue = rm_malloc(sizeof(LRUQueue));
  lruQueue->buffer = rm_calloc(capacity, sizeOfLRUNode(dataSize));
  lruQueue->freeList = array_new(LRUNode *, capacity);
  lruQueue->dataSize = dataSize;
  lruQueue->freeCB = freeCB;
  // initialization
  return initLRUQueue(lruQueue, capacity);
}

bool lruQueueIsFull(LRUQueue *queue)
{
  // maximal capacity reached
  return queue->capacity == queue->size;
}

bool lruQueueIsEmpty(LRUQueue *queue)
{
  // no tail - nothing inside
  return queue->tail == NULL;
}

void *lruQueueDequeue(LRUQueue *queue)
{
  // for empty queue return null
  if (lruQueueIsEmpty(queue))
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
  return tmp->data;
}

LRUNode *setNodeInQueue(LRUQueue *queue, LRUNode *newNode)
{
  // new node next is the queue (previous) head
  newNode->next = queue->head;
  if (lruQueueIsEmpty(queue))
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
  if (lruQueueIsFull(queue))
  {
    queue->stopLinearInsertion = true;
  }
  return newNode;
}

void *lruQueueEnqueue(LRUQueue *queue, void *cacheValue)
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

  LRUNode *node = initLRUNode(emptyNode, cacheValue, queue->dataSize, (lruDataFreeFunc)queue->freeCB);
  //will be false until array is full - for linear insertion over the array
  if (!queue->stopLinearInsertion && emptyNode == queue->emptySpace)
  {
    queue->emptySpace = (LRUNode*)((char*)queue->emptySpace + sizeOfLRUNode(queue->dataSize));
  }
  // put node in queue
  return (setNodeInQueue(queue, node)->data);
}

void lruQueueEmptyQueue(LRUQueue *queue)
{
  // move all pointers and meta data to their defualt values
  initLRUQueue(queue, queue->capacity);
}

LRUNode *getNodeFromData(void *data, size_t dataSize)
{
  size_t metaDataSize = sizeOfLRUNode(dataSize) - dataSize;
  LRUNode *node = (LRUNode *)((char *)data - metaDataSize);
  return node;
}

void lruQueueMoveToHead(LRUQueue *queue, void *data)
{
  LRUNode *node = getNodeFromData(data, queue->dataSize);
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

void lruQueueRemoveFromQueue(LRUQueue *queue, void *data)
{
  LRUNode *node = getNodeFromData(data, queue->dataSize);
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

void lruQueueFree(LRUQueue *lruQueue)
{
  // go over each entry and free its result set
  while (lruQueue->head != NULL)
  {
    void* data = (void*)lruQueue->head->data;
    lruQueue->freeCB(data);
    lruQueue->head = lruQueue->head->next;
  }

  for (int i = 0; i < array_len(lruQueue->freeList); i++)
  {
    lruQueue->freeCB(lruQueue->freeList[i]->data);
  }

  // memeory release
  array_free(lruQueue->freeList);
  rm_free(lruQueue->buffer);
  rm_free(lruQueue);
}