#ifndef LRU_QUEUE_H
#define LRU_QUEUE_H

#include "../../util/rmalloc.h"
#include "./lru_node.h"
#include <stdbool.h>

typedef struct LRUQueue {
  LRUNode *queue;
  size_t size;
  size_t capacity;
  LRUNode *head;
  LRUNode *tail;
  LRUNode *emptySpace;
  bool fullCapacity;
} LRUQueue;

LRUQueue *LRUQueue_New(size_t size);
void LRUQueue_Free(LRUQueue *lruQueue);
bool isFullQueue(LRUQueue *queue);
bool isEmptyQueue(LRUQueue *queue);
LRUNode *dequeue(LRUQueue *queue);
LRUNode *enqueue(const char *hashKey, LRUQueue *queue);
void moveToHead(LRUNode *node, LRUQueue *queue);
LRUQueue *initLRUQueue(LRUQueue *lruQueue, size_t capacity);
#endif