/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "linked_list.h"
#include "rmalloc.h"

inline void LinkedList_Init(LinkedList *list) {
	list->head = NULL;
	list->tail = NULL;
}

inline void LinkedList_AddNode(LinkedList *list, LinkedListNode *node) {
	node->prev = list->tail;
	// If list is empty, both head and tail are NULL. Set them to the new node.
	if(list->head == NULL && list->tail == NULL) list->head = list->tail = node;
	else list->tail->next = node; 	// There is a tail
	// In any case, the new node is now the tail.
	list->tail = node;
	node->list = list;
}

inline void LinkedList_RemoveNode(LinkedList *list, LinkedListNode *node) {
	if(node->list != list) return;
	if(node->prev) node->prev->next = node->next;
	if(node->next) node->next->prev = node->prev;
	if(node == list->tail) list->tail = node->prev;
	if(node == list->head) list->head = node->next;
	// This node is disconnected.
	node->next = NULL;
	node->prev = NULL;
	node->list = NULL;
}

void LinkedList_MoveForward(LinkedList *list, LinkedListNode *node) {
	// The node is not belonged to the list.
	if(node->list != list) return;
	if(node != list->head) {
		// Pull node out from its place (link its prev and next).
		node->prev->next = node->next;
		// Check if node is tail (next == NULL)
		if(node->next) node->next->prev = node->prev;
		else list->tail = node->prev; // In case node is tail, move tail to point at its prev.

		node->next = node->prev;
		// Originally node->prev->prev
		node->prev = node->next->prev;
		node->next->prev = node;
		if(node->prev) node->prev->next = node;
		else list->head = node;
	}
}

void LinkedList_MoveToHead(LinkedList *list, LinkedListNode *node) {
	// The node is not belonged to the list.
	if(node->list != list) return;
	if(node != list->head) {
		LinkedList_RemoveNode(list, node);
		// Put node in the head and link with previous head.
		node->next = list->head;
		node->prev = NULL;
		node->next->prev = node;
		list->head = node;
		node->list = list;
	}
}

void LinkedList_MoveBack(LinkedList *list, LinkedListNode *node) {
	// The node is not belonged to the list.
	if(node->list != list) return;
	if(node != list->tail) {
		// Pull node out from its place (link its prev and next).
		node->next->prev = node->prev;
		// Check if node is head (prev == NULL)
		if(node->prev) node->prev->next = node->next;
		else list->head = node->next; // In case node is head, move head to point at its next.

		node->prev = node->next;
		// Originally node->next->next
		node->next = node->prev->next;
		node->prev->next = node;
		if(node->next) node->next->prev = node;
		else list->tail = node;
	}
}

void LinkedList_MoveToTail(LinkedList *list, LinkedListNode *node) {
	// The node is not belonged to the list.
	if(node->list != list) return;
	if(node != list->tail) {
		LinkedList_RemoveNode(list, node);
		// Put node in the tail and link with previous tail.
		node->prev = list->tail;
		node->next = NULL;
		node->prev->next = node;
		list->tail = node;
		node->list = list;
	}
}
