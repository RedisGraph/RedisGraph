/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>

typedef void (*CacheItemFreeFunc)(void *);

/**
 * @brief  A struct for a double linked list node with a key and value.
 */
typedef struct CacheListNode_t {
	void *value;                    // Node stored value.
	uint64_t hashval;               // Key - retrived by applying hash function over the value.
	struct CacheListNode_t *prev;   // Previous node in the linked list.
	struct CacheListNode_t *next;   // Next node in the linked list.
} CacheListNode;

/**
 * @brief  Double linked list with values storing. The list is based over a heap allocated array.
 */
typedef struct {
	CacheListNode *buffer;          // Nodes array.
	CacheListNode *head;            // Linked list head.
	CacheListNode *tail;            // Linked list tail.
	uint buffer_len;                // Current occupied nodes in the buffer.
	uint buffer_cap;                // Buffer size (fixed).
	CacheItemFreeFunc ValueFree;    // Value free function.
} CacheList;

CacheList *CacheList_New(uint size, CacheItemFreeFunc freeCB);

// Return true if the cache list is at capacity.
bool CacheList_IsFull(const CacheList *list);

// Promote the given node to the head of the cache.
void CacheList_Promote(CacheList *list, CacheListNode *node);

// Delete the list's tail and free its contents.
CacheListNode *CacheList_RemoveTail(CacheList *list);

// Populate a new node and add it as the head of the list.
CacheListNode *CacheList_PopulateNode(CacheList *list, CacheListNode *node, uint64_t hashval,
									  void *value);

/* Return the next unused space in the cache list.
 * Should only be invoked on non-full lists. */
CacheListNode *CacheList_GetUnused(CacheList *list);

// Free the list and all its elements.
void CacheList_Free(CacheList *list);

