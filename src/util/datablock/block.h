/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#ifndef _BLOCK_H_
#define _BLOCK_H_

#include <stdlib.h>

// Number of items in a block. Should always be a power of 2.
#define BLOCK_CAP 16384


/* Data block is a type agnostic continuous block of memory
 * used to hold items of the same type, each block has a next
 * pointer to another block or NULL if this is the last block. */

typedef struct Block {
	size_t itemSize;        // Size of a single Item in bytes.
	struct Block *next;     // Pointer to next block.
	unsigned char data[];   // Item array. MUST BE LAST MEMBER OF THE STRUCT!
} Block;

#endif

