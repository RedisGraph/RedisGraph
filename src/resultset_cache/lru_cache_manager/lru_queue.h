#ifndef LRU_QUEUE_H
#define LRU_QUEUE_H
#include "../result_set_cache_includes.h"
#include "./lru_node.h"


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
LRUNode *enqueue(LRUQueue *queue, const char *hashKey, ResultSet* resultset);
void emptyQueue(LRUQueue *queue);
void moveToHead(LRUQueue *queue, LRUNode *node);
LRUQueue *initLRUQueue(LRUQueue *lruQueue, size_t capacity);
#endif