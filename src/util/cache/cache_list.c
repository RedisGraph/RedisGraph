/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "cache_list.h"
#include "../rmalloc.h"
#include <assert.h>

CacheList *CacheList_New(uint size, listValueFreeFunc freeCB) {
	CacheList *list = rm_malloc(sizeof(CacheList));
	list->buffer = rm_calloc(size, sizeof(CacheListNode));
	list->head = NULL;
	list->tail = NULL;
	list->buffer_len = 0;
	list->buffer_cap = size;
	list->ValueFree = freeCB;
	return list;
}

inline bool CacheList_IsFull(CacheList *list) {
	return list->buffer_len == list->buffer_cap;
}

void CacheList_Promote(CacheList *list, CacheListNode *node) {
	// Node was already head, do nothing.
	if(list->head == node) return;

	/* Detach this node and connect its previous and next pointers. */
	CacheListNode *prev = node->prev;
	// Current node was not the tail, connect prev to the next element.
	if(prev != NULL) prev->next = node->next;
	node->next->prev = prev;

	/* Move this node to the head. */
	node->next = list->head;
	node->prev = NULL; // The head node has no previous element.
	list->head->next = node;
	list->head = node;
}

CacheListNode *CacheList_RemoveTail(CacheList *list) {
	// We can only get here on a filled list.
	CacheListNode *tail = list->tail;

	// Update the tail to point to the new last element.
	list->tail = tail->prev;
	tail->prev->next = NULL; // The tail node has no next element.

	/* TODO consider replacing this with a rax callback that frees node->value.
	 * This may not be possible while remaining data-agnostic,
	 * as we need the callback to look like ExecutionPlan_Free(node->value)
	 * OTOH, maybe possible if values in rax are actual plans, but then lookups are less valuable.
	 */
	list->ValueFree(tail->value);
	return tail;
}

CacheListNode *CacheList_NewNode(CacheList *list, CacheListNode *node, uint64_t hashval,
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

