/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

// Forword declaration.
typedef struct LinkedList LinkedList;

typedef struct LinkedListNode {
	struct LinkedListNode *prev; // Previous Node in the list.
	struct LinkedListNode *next; // Next node in the list.
	struct LinkedList *list;     // The contianing list.
} LinkedListNode;

/**
 * @brief  This linked list struct is managing its nodes from front to end with double linked list nodes.
 * @note   The traverse direction from front to end is with the "next" pointer. The traverse direction from end to front is with the "prev" pointer.
 */
typedef struct LinkedList {
	LinkedListNode *front;
	LinkedListNode *end;
} LinkedList;

/**
 * @brief  Initializes a linked list struct to an empty list.
 * @param  *list: Linked list struct.
 */
void LinkedList_Init(LinkedList *list);

/**
 * @brief  Moves a linked list node towrds the front of the list.
 * @note   If the node is not part of the list, nothing will be done.
 * @param  *list: Linked list struct.
 * @param  *node: Node in the list.
 */
void LinkedList_MoveForward(LinkedList *list, LinkedListNode *node);

/**
 * @brief  Set a linkes list node to become the front of the list.
 * @note   If the node is not part of the list, nothing will be done.
 * @param  *list: Linked list struct.
 * @param  *node: Node in the list.
 */
void LinkedList_MoveToFront(LinkedList *list, LinkedListNode *node);

/**
 * @brief  Moves a linked list node towrds the end of the list.
 * @note   If the node is not part of the list, nothing will be done.
 * @param  *list: Linked list struct.
 * @param  *node: Node in the list.
 */
void LinkedList_MoveBack(LinkedList *list, LinkedListNode *node);

/**
 * @brief  Set a linkes list node to become the end of the list.
 * @note   If the node is not part of the list, nothing will be done.
 * @param  *list: Linked list struct.
 * @param  *node: Node in the list.
 */
void LinkedList_MoveToEnd(LinkedList *list, LinkedListNode *node);

/**
 * @brief  Adds a new node to a linked list.
 * @note   The new node will be set as the new "end". if x was the previous "end" and "y" is the new node:
 *         x.next = y
 *         y.prev = x
 * @param  *list: Linked list struct.
 * @param  *node: Node in the list.
 */
void LinkedList_AddNode(LinkedList *list, LinkedListNode *node);

/**
 * @brief  Detach a node from the list.
 * @note   If the node is not part of the list, nothing will be done.
 * @param  *list: Linked list struct.
 * @param  *node: Node in the list.
 */
void LinkedList_RemoveNode(LinkedList *list, LinkedListNode *node);
