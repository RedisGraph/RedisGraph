/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "linked_list.h"
#include "rmalloc.h"

inline void LinkedList_Init(LinkedList *list) {
	list->front = NULL;
	list->end = NULL;
}

inline void LinkedList_AddNode(LinkedList *list, LinkedListNode *node) {
	node->prev = list->end;
	// If list is empty, both front and end are NULL. Set them to the new node.
	if(list->front == NULL && list->end == NULL) list->front = list->end = node;
	else list->end->next = node; 	// There is a end
	// In any case, the new node is now the end.
	list->end = node;
	node->list = list;
}

inline void LinkedList_RemoveNode(LinkedList *list, LinkedListNode *node) {
	if(node->list != list) return;
	if(node->prev) node->prev->next = node->next;
	if(node->next)node->next->prev = node->prev;
	if(node == list->end) list->end = node->prev;
	if(node == list->front) list->front = node->next;
	// This node is disconnected.
	node->next = NULL;
	node->prev = NULL;
	node->list = NULL;
}

void LinkedList_MoveForward(LinkedList *list, LinkedListNode *node) {
	// The node is not belonged to the list.
	if(node->list != list) return;
	if(node != list->front) {
		// Pull node out from its place (link its prev and next).
		node->prev->next = node->next;
		// Check if node is end (next == NULL)
		if(node->next) node->next->prev = node->prev;
		else list->end = node->prev; // In case node is end, move end to point at its prev.

		node->next = node->prev;
		// Originally node->prev->prev
		node->prev = node->next->prev;
		node->next->prev = node;
		if(node->prev) node->prev->next = node;
		else list->front = node;
	}
}

void LinkedList_MoveToFront(LinkedList *list, LinkedListNode *node) {
	// The node is not belonged to the list.
	if(node->list != list) return;
	if(node != list->front) {
		// Pull node out from its place (link its prev and next).
		node->prev->next = node->next;
		// Check if node is end (next == NULL)
		if(node->next) node->next->prev = node->prev;
		// In case node is end,  move end to point at its prev and nullify end's next.
		if(node == list->end) {
			list->end = node->prev;
			list->end->next = NULL;
		}
		// Put node in the front and link with previous front.
		node->next = list->front;
		node->prev = NULL;
		node->next->prev = node;
		list->front = node;
	}
}

void LinkedList_MoveBack(LinkedList *list, LinkedListNode *node) {
	// The node is not belonged to the list.
	if(node->list != list) return;
	if(node != list->end) {
		// Pull node out from its place (link its prev and next).
		node->next->prev = node->prev;
		// Check if node is front (prev == NULL)
		if(node->prev) node->prev->next = node->next;
		else list->front = node->next; // In case node is front, move front to point at its next.

		node->prev = node->next;
		// Originally node->next->next
		node->next = node->prev->next;
		node->prev->next = node;
		if(node->next) node->next->prev = node;
		else list->end = node;
	}
}

void LinkedList_MoveToEnd(LinkedList *list, LinkedListNode *node) {
	// The node is not belonged to the list.
	if(node->list != list) return;
	if(node != list->end) {
		// Pull node out from its place (link its prev and next).
		node->next->prev = node->prev;
		// Check if node is front (prev == NULL)
		if(node->prev) node->prev->next = node->next;
		else list->front = node->next; // In case node is front, move front to point at its next.
		// Put node in the end and link with previous end.
		node->prev = list->end;
		node->next = NULL;
		node->prev->next = node;
		list->end = node;
	}
}
