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

typedef void (*listValueFreeFunc)(void *);

typedef struct CacheListNode_t {
	void *value;
	uint64_t hashval;
	struct CacheListNode_t *prev;
	struct CacheListNode_t *next;
} CacheListNode;

typedef struct {
	CacheListNode *buffer;
	CacheListNode *head;
	CacheListNode *tail;
	uint buffer_len;
	uint buffer_cap;
	listValueFreeFunc ValueFree;
} CacheList;

CacheList *CacheList_New(uint size, listValueFreeFunc freeCB);

// Return true if the cache list is at capacity.
bool CacheList_IsFull(CacheList *list);

// Promote the given node to the head of the cache.
void CacheList_Promote(CacheList *list, CacheListNode *node);

// Delete the list's tail and free its contents.
CacheListNode *CacheList_RemoveTail(CacheList *list);

// Populate a new node and add it as the head of the list.
CacheListNode *CacheList_NewNode(CacheList *list, CacheListNode *node, uint64_t hashval,
								 void *value);

/* Return the next unused space in the cache list.
 * Should only be invoked on non-full lists. */
CacheListNode *CacheList_GetUnused(CacheList *list);

// Free the list and all its elements.
void CacheList_Free(CacheList *list);

