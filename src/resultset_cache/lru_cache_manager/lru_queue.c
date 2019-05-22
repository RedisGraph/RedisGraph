/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "./lru_queue.h"
#include <stdlib.h>
#include <stdio.h>

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
  lruQueue->emptySpace = lruQueue->queue;
  // queue hasn't reaced full cacpacity, a linear insertion is possible
  lruQueue->fullCapacity = false;
  array_clear(lruQueue->emptyCells);

  return lruQueue;
}

LRUQueue *LRUQueue_New(size_t capacity)
{
  // memory allocations
  LRUQueue *lruQueue = rm_malloc(sizeof(LRUQueue));
  lruQueue->queue = rm_calloc(capacity, sizeof(LRUNode));
  lruQueue->emptyCells = array_new(LRUNode *, capacity);
  // initialization
  return initLRUQueue(lruQueue, capacity);
}

void LRUQueue_Free(LRUQueue *lruQueue)
{
  // go over each entry and free its result set
  while (lruQueue->head != NULL)
  {
    ResultSet_Free(lruQueue->head->cacheData.cacheValue);
    lruQueue->head = lruQueue->head->next;
  }

  for (int i = 0; i < array_len(lruQueue->emptyCells); i++)
  {
    ResultSet_Free(lruQueue->emptyCells[i]->cacheData.cacheValue);
  }

  // memeory release


  array_free(lruQueue->emptyCells);
  rm_free(lruQueue->queue);
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
  array_append(queue->emptyCells, tmp);
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
    queue->fullCapacity = true;
  }
  return newNode;
}

LRUNode *enqueue(LRUQueue *queue, unsigned long long const key, ResultSet *resultSet)
{
  // init new node
  LRUNode *emptyNode;
  //see if nodes where removed from the queue
  if (array_len(queue->emptyCells) > 0)
  {
    emptyNode = array_tail(queue->emptyCells);
    array_pop(queue->emptyCells);
  }
  else
  {
    emptyNode = queue->emptySpace;
  }

  LRUNode *node = initLRUNode(emptyNode, key, resultSet);
  //will be false until array is full - for linear insertion over the array
  if (!queue->fullCapacity && emptyNode == queue->emptySpace)
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

  array_append(queue->emptyCells, node);
  queue->size--;
}