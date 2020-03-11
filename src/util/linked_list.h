/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

typedef struct LinkedListNode {
	struct LinkedListNode *prev; // Previous Node in the list.
	struct LinkedListNode *next; // Next node in the list.
} LinkedListNode;


typedef struct LinkedList {
	LinkedListNode *head;
	LinkedListNode *tail;
} LinkedList;

void LinkedList_Init(LinkedList *list);

void LinkedList_MoveForward(LinkedList *list, LinkedListNode *node);

void LinkedList_MoveToHead(LinkedList *list, LinkedListNode *node);

void LinkedList_MoveBack(LinkedList *list, LinkedListNode *node);

void LinkedList_MoveToTail(LinkedList *list, LinkedListNode *node);

void LinkedList_AddNode(LinkedList *list, LinkedListNode *node);

void LinkedList_RemoveNode(LinkedList *list, LinkedListNode *node);

