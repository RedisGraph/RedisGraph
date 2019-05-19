#include "./lru_queue.h"
#include <stdlib.h>
#include <stdio.h>

LRUQueue *LRUQueue_New(size_t size)
{
  LRUQueue *lruQueue = rm_malloc(sizeof(LRUQueue));
  lruQueue->queue = rm_calloc(size, sizeof(LRUNode));

  return initLRUQueue(lruQueue, size);
}

LRUQueue* initLRUQueue(LRUQueue* lruQueue, size_t capacity){
  lruQueue->capacity = capacity;
  lruQueue->size = 0;
  lruQueue->head = NULL;
  lruQueue->tail = NULL;
  lruQueue->emptySpace = lruQueue->queue;
  lruQueue->fullCapacity = false;
  return lruQueue;
}

void LRUQueue_Free(LRUQueue *lruQueue)
{
  while(lruQueue->head != NULL){
    ResultSet_Free(lruQueue->head->cacheData.resultSet);
    lruQueue->head = lruQueue->head->next;
  }
  rm_free(lruQueue->queue);
  rm_free(lruQueue);
}

bool isEmptyQueue(LRUQueue *queue)
{
  return queue->tail == NULL;
}
bool isFullQueue(LRUQueue *queue)
{
  return queue->capacity == queue->size;
}

LRUNode *dequeue(LRUQueue *queue)
{
  if (isEmptyQueue(queue))
  {
    return NULL;
  }
  if (queue->head == queue->tail)
  {
    queue->head = NULL;
  }
  LRUNode *tmp = queue->tail;
  queue->tail = tmp->prev;
  if (queue->tail)
  {
    queue->tail->next = NULL;
  }
  queue->emptySpace = tmp;
  queue->size--;
  return tmp;
}



LRUNode *setNodeInQueue(LRUQueue *queue, LRUNode *newNode )
{
  newNode->next = queue->head;
  if (isEmptyQueue(queue))
  {
    queue->head = queue->tail = newNode;
  }
  else
  {
    queue->head->prev = newNode;
    queue->head = newNode;
  }
  queue->size++;
  if(isFullQueue(queue)){
    queue->fullCapacity = true;
  }
  return newNode;
}

LRUNode *enqueue(LRUQueue *queue, const char *key, ResultSet* resultSet)
{
  LRUNode *node = initLRUNode(queue->emptySpace, key, resultSet);

  
  //will be false until array is full
  if (!queue->fullCapacity ){
    queue->emptySpace++;
  }

  return setNodeInQueue(queue, node);
}

void moveToHead(LRUQueue *queue, LRUNode *node)
{
  if (node != queue->head)
  {
    node->prev->next = node->next;
    if (node->next)
    {
      node->next->prev = node->prev;
    }
    if (node == queue->tail)
    {
      queue->tail = node->prev;
      queue->tail->next = NULL;
    }
    node->next = queue->head;
    node->prev = NULL;
    node->next->prev = node;
    queue->head = node;
  }
}

void emptyQueue(LRUQueue *queue){
  initLRUQueue(queue, queue->capacity);
}