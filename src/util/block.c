/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "block.h"
#include "rmalloc.h"
#include <assert.h>

Block *Block_New(uint itemSize, uint capacity) {
	assert(itemSize > 0);
	Block *block = rm_calloc(1, sizeof(Block) + (capacity * itemSize));
	block->itemSize = itemSize;
	return block;
}

void Block_Free(Block *block) {
	assert(block);
	rm_free(block);
}

