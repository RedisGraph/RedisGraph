/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "cache_list.h"
#include "../rmalloc.h"
#include <assert.h>

CacheList *CacheList_New(uint size, CacheItemFreeFunc freeCB) {
	CacheList *list = rm_malloc(sizeof(CacheList));
	list->buffer = rm_calloc(size, sizeof(CacheListNode));
	list->head = NULL;
	list->tail = NULL;
	list->buffer_len = 0;
	list->buffer_cap = size;
	list->ValueFree = freeCB;
	return list;
}

inline bool CacheList_IsFull(const CacheList *list) {
	return list->buffer_len == list->buffer_cap;
}

void CacheList_Promote(CacheList *list, CacheListNode *node) {
	// Node was already head, do nothing.
	if(list->head == node) return;

	// Check for tail
	if(list->tail == node) {
		// Update tail.
		list->tail = node->prev;
	} else {
		// Node is not the tail, it has a next.
		node->next->prev = node->prev;
	}
	// Node is not head, it will always has a prev.
	node->prev->next = node->next;

	/* Move this node to the head. */
	node->next = list->head;
	node->prev = NULL; // The head node has no previous element.
	list->head->prev = node;
	list->head = node;
}

CacheListNode *CacheList_RemoveTail(CacheList *list) {
	// We can only get here on a filled list.
	assert(CacheList_IsFull(list) && "CacheList_RemoveTail: list should be full");
	CacheListNode *tail = list->tail;

	// Update the tail to point to the new last element.
	list->tail = tail->prev;
	list->tail->next = NULL; // The tail node has no next element.

	list->ValueFree(tail->value);
	return tail;
}

CacheListNode *CacheList_PopulateNode(CacheList *list, CacheListNode *node, uint64_t hashval,
									  void *value) {
	// Assign data members to the new node.
	node->hashval = hashval;
	node->value = value;

	if(list->head == NULL) {
		list->head = list->tail = node;
		return node;
	}
	// Insert the new element as the head.
	CacheListNode *old_head = list->head;
	list->head = node;
	old_head->prev = node;
	node->prev = NULL; // The head node has no previous element.
	node->next = old_head;

	return node;
}

CacheListNode *CacheList_GetUnused(CacheList *list) {
	assert(list->buffer_len < list->buffer_cap);
	// Return the address of the next unused element and increment the buffer length.
	return &list->buffer[list->buffer_len++];
}

void CacheList_Free(CacheList *list) {
	// Call the free routine for every cache member.
	for(uint i = 0; i < list->buffer_len; i ++) list->ValueFree(list->buffer[i].value);
	rm_free(list->buffer);
	rm_free(list);
}

