/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "RG.h"
#include "datablock.h"
#include "datablock_iterator.h"
#include "../arr.h"
#include "../rmalloc.h"
#include <math.h>
#include <stdbool.h>

static void _DataBlock_AddBlocks
(
	DataBlock *dataBlock,
	uint blockCount
) {
	ASSERT(dataBlock);
	ASSERT(blockCount > 0);

	uint prevBlockCount = dataBlock->blockCount;
	dataBlock->blockCount += blockCount;
	if(!dataBlock->blocks)
		dataBlock->blocks = rm_malloc(sizeof(Block *) * dataBlock->blockCount);
	else
		dataBlock->blocks = rm_realloc(dataBlock->blocks, sizeof(Block *) * dataBlock->blockCount);

	uint i;
	for(i = prevBlockCount; i < dataBlock->blockCount; i++) {
		dataBlock->blocks[i] = Block_New(dataBlock->itemSize, dataBlock->blockCap);
		if(i > 0) dataBlock->blocks[i - 1]->next = dataBlock->blocks[i];
	}
	dataBlock->blocks[i - 1]->next = NULL;

	dataBlock->itemCap = dataBlock->blockCount * dataBlock->blockCap;
}

// Checks to see if idx is within global array bounds
// array bounds are between 0 and itemCount + #deleted indices
// e.g. [3, 7, 2, D, 1, D, 5] where itemCount = 5 and #deleted indices is 2
// and so it is valid to query the array with idx 6.
static inline bool _DataBlock_IndexOutOfBounds
(
	const DataBlock *dataBlock,
	uint64_t idx
) {
	return (idx >= (dataBlock->itemCount + array_len(dataBlock->deletedIdx)));
}

DataBlockItemHeader *DataBlock_GetItemHeader
(
	const DataBlock *dataBlock,
	uint64_t idx
) {
	Block *block = GET_ITEM_BLOCK(dataBlock, idx);
	idx = ITEM_POSITION_WITHIN_BLOCK(idx, dataBlock->blockCap);
	return (DataBlockItemHeader *)block->data + (idx * block->itemSize);
}

//------------------------------------------------------------------------------
// DataBlock API implementation
//------------------------------------------------------------------------------

DataBlock *DataBlock_New
(
	uint64_t blockCap,
	uint64_t itemCap,
	uint itemSize,
	fpDestructor fp
) {
	DataBlock *dataBlock = rm_malloc(sizeof(DataBlock));

	dataBlock->blocks     = NULL;
	dataBlock->itemSize   = itemSize + ITEM_HEADER_SIZE;
	dataBlock->itemCount  = 0;
	dataBlock->blockCount = 0;
	dataBlock->blockCap   = blockCap;
	dataBlock->deletedIdx = array_new(uint64_t, 128);
	dataBlock->destructor = fp;

	_DataBlock_AddBlocks(dataBlock,
			ITEM_COUNT_TO_BLOCK_COUNT(itemCap, dataBlock->blockCap));

	return dataBlock;
}

// returns number of items stored
uint64_t DataBlock_ItemCount
(
	const DataBlock *dataBlock
) {
	ASSERT(dataBlock != NULL);

	return dataBlock->itemCount;
}

// returns number of blocks
uint64_t DataBlock_BlockCount
(
	const DataBlock *dataBlock
) {
	ASSERT(dataBlock != NULL);

	return dataBlock->blockCount;
}

// Returns an iterator which scans entire datablock
DataBlockIterator *DataBlock_Scan
(
	const DataBlock *dataBlock
) {
	// start with first item
	int64_t s = 0;

	// end with last item
	int64_t e = dataBlock->itemCount + array_len(dataBlock->deletedIdx);

	return DataBlockIterator_New(dataBlock->blocks, dataBlock->blockCap, s, e);
}

// Returns an iterator which scans entire datablock backwards
DataBlockIterator *DataBlock_ScanDesc
(
	const DataBlock *dataBlock
) {
	// start with last item
	int64_t s = dataBlock->itemCount + array_len(dataBlock->deletedIdx) -1;

	// end with first item
	int64_t e = -1;

	return DataBlockIterator_New(dataBlock->blocks, dataBlock->blockCap, s, e);
}

DataBlockIterator *DataBlock_FullScan
(
	const DataBlock *dataBlock
) {
	ASSERT(dataBlock != NULL);

	// start with first item
	int64_t s = 0;

	// end with last item
	int64_t e = dataBlock->blockCount * dataBlock->blockCap;

	return DataBlockIterator_New(dataBlock->blocks, dataBlock->blockCap, s, e);
}

// Make sure datablock can accommodate at least k items.
void DataBlock_Accommodate(DataBlock *dataBlock, int64_t k) {
	// Compute number of free slots.
	int64_t freeSlotsCount = dataBlock->itemCap - dataBlock->itemCount;
	int64_t additionalItems = k - freeSlotsCount;

	if(additionalItems > 0) {
		int64_t additionalBlocks =
			ITEM_COUNT_TO_BLOCK_COUNT(additionalItems, dataBlock->blockCap);
		_DataBlock_AddBlocks(dataBlock, additionalBlocks);
	}
}

void DataBlock_Ensure(DataBlock *dataBlock, uint64_t idx) {
	ASSERT(dataBlock != NULL);

	// datablock[idx] exists
	if(dataBlock->itemCap > idx) return;

	// make sure datablock cap > 'idx'
	int64_t additionalItems = (1 + idx) - dataBlock->itemCap;
	int64_t additionalBlocks =
		ITEM_COUNT_TO_BLOCK_COUNT(additionalItems, dataBlock->blockCap);
	_DataBlock_AddBlocks(dataBlock, additionalBlocks);

	ASSERT(dataBlock->itemCap > idx);
}

void *DataBlock_GetItem(const DataBlock *dataBlock, uint64_t idx) {
	ASSERT(dataBlock != NULL);

	// return NULL if idx is out of bounds
	if(_DataBlock_IndexOutOfBounds(dataBlock, idx)) return NULL;

	DataBlockItemHeader *item_header = DataBlock_GetItemHeader(dataBlock, idx);

	// Incase item is marked as deleted, return NULL.
	if(IS_ITEM_DELETED(item_header)) return NULL;

	return ITEM_DATA(item_header);
}

void *DataBlock_AllocateItem(DataBlock *dataBlock, uint64_t *idx) {
	// make sure we've got room for items
	if(dataBlock->itemCount >= dataBlock->itemCap) {
		// allocate an additional block
		_DataBlock_AddBlocks(dataBlock, 1);
	}
	ASSERT(dataBlock->itemCap > dataBlock->itemCount);

	// get index into which to store item,
	// prefer reusing free indicies
	uint pos = dataBlock->itemCount;
	if(array_len(dataBlock->deletedIdx) > 0) {
		pos = array_pop(dataBlock->deletedIdx);
	}
	dataBlock->itemCount++;

	if(idx) *idx = pos;

	DataBlockItemHeader *item_header = DataBlock_GetItemHeader(dataBlock, pos);
	MARK_HEADER_AS_NOT_DELETED(item_header);

	return ITEM_DATA(item_header);
}

void DataBlock_DeleteItem(DataBlock *dataBlock, uint64_t idx) {
	ASSERT(dataBlock != NULL);
	ASSERT(!_DataBlock_IndexOutOfBounds(dataBlock, idx));

	// Return if item already deleted.
	DataBlockItemHeader *item_header = DataBlock_GetItemHeader(dataBlock, idx);
	if(IS_ITEM_DELETED(item_header)) return;

	// Call item destructor.
	if(dataBlock->destructor) {
		unsigned char *item = ITEM_DATA(item_header);
		dataBlock->destructor(item);
	}

	MARK_HEADER_AS_DELETED(item_header);

	array_append(dataBlock->deletedIdx, idx);
	dataBlock->itemCount--;
}

uint32_t DataBlock_DeletedItemsCount(const DataBlock *dataBlock) {
	return array_len(dataBlock->deletedIdx);
}

inline bool DataBlock_ItemIsDeleted(void *item) {
	DataBlockItemHeader *header = GET_ITEM_HEADER(item);
	return IS_ITEM_DELETED(header);
}

// migrates an item
bool DataBlock_MigrateItem
(
	DataBlock *dataBlock,  // datablock
	uint64_t src,          // item original position
	uint64_t *dest         // item new position
) {
	// validations
	// src must be within bounds
	// migrated item must not be NULL
	ASSERT(dataBlock != NULL);
	ASSERT(!_DataBlock_IndexOutOfBounds(dataBlock, src));
	ASSERT(DataBlock_GetItem(dataBlock, src) != NULL);

	// see if there's a deleted slot to migrate to
	// only migrate item from a higher index to a lower index
	uint32_t n = array_len(dataBlock->deletedIdx);
	if(n == 0) {
		// no available slots
		return false;
	}

	uint64_t to = dataBlock->deletedIdx[n-1];

	if(to > src) {
		// can't migrate forward
		return false;
	}

	// mark 'to' as none deleted
	void *to_ptr = DataBlock_GetItemHeader(dataBlock, to);

	// migrate item
	DataBlockItemHeader *src_ptr = DataBlock_GetItemHeader(dataBlock, src);
	memmove(to_ptr, (void*)src_ptr, dataBlock->itemSize);

	// we can either mark src as deleted or we can zero out its memory
	// marking as deleted seems cheaper
	MARK_HEADER_AS_DELETED(src_ptr);

	// remove used index from free list
	*dest = array_pop(dataBlock->deletedIdx);

	return true;
}

//------------------------------------------------------------------------------
// Out of order functionality
//------------------------------------------------------------------------------

void *DataBlock_AllocateItemOutOfOrder
(
	DataBlock *dataBlock,
	uint64_t idx
) {
	// Check if idx<=data block's current capacity. If needed, allocate additional blocks.
	DataBlock_Ensure(dataBlock, idx);
	DataBlockItemHeader *item_header = DataBlock_GetItemHeader(dataBlock, idx);
	MARK_HEADER_AS_NOT_DELETED(item_header);
	dataBlock->itemCount++;
	return ITEM_DATA(item_header);
}

void DataBlock_MarkAsDeletedOutOfOrder
(
	DataBlock *dataBlock,
	uint64_t idx
) {
	// Check if idx<=data block's current capacity. If needed, allocate additional blocks.
	DataBlock_Ensure(dataBlock, idx);
	DataBlockItemHeader *item_header = DataBlock_GetItemHeader(dataBlock, idx);
	// Delete
	MARK_HEADER_AS_DELETED(item_header);
	array_append(dataBlock->deletedIdx, idx);
}

// remove empty blocks
// returns number of blocks removed
int DataBlock_Trim
(
	DataBlock *dataBlock  // datablock to trim
) {
	ASSERT(dataBlock != NULL);

	// DataBlock_Trim should be called as part of the GRAPH.VACUUM process
	// the datablock is expected to have the following structure
	//
	// |------------------------------------------------|
	// |*,*,*,|*,*,D,|D,D,D,|D,D,D,|_,_,_,|_,_,_,|_,_,_,|
	//
	// occupied entries at the begining, followed by deleted entries
	// and possibly empty blocks
	//
	// Trim will locate the last none empty block and remove all following
	// blocks, trimming the datablock above will result in a compact datablock
	//
	// |-------------|
	// |*,*,*,|*,*,_,|
	//

	// determine last none empty block index
	uint64_t max_idx = dataBlock->itemCount;
	int last_block = ITEM_IDX_TO_BLOCK_IDX(max_idx, dataBlock->blockCap);
	int blocks_removed = dataBlock->blockCount - 1 - last_block;

	// quickly return if there are no blocks to remove
	if(blocks_removed == 0) {
		return blocks_removed;
	}

	//--------------------------------------------------------------------------
	// discard empty blocks
	//--------------------------------------------------------------------------
	for(int i = last_block + 1; i < dataBlock->blockCount; i++) {
		Block_Free(dataBlock->blocks[i]);
	}

	//--------------------------------------------------------------------------
	// update datablock
	//--------------------------------------------------------------------------

	// updata datablock blockCount
	dataBlock->blockCount = last_block + 1;

	// update datablock blocks array
	dataBlock->blocks = rm_realloc(dataBlock->blocks,
			sizeof(Block*) * dataBlock->blockCount);

	// update datablock capacity
	dataBlock->itemCap =
		(dataBlock->blockCount * dataBlock->blockCap);

	// update last block next pointer
	dataBlock->blocks[last_block]->next = NULL;

	// clear datablock's free list
	array_free(dataBlock->deletedIdx);
	dataBlock->deletedIdx = array_new(uint64_t, 128);

	return blocks_removed;
}

void DataBlock_Free(DataBlock *dataBlock) {
	for(uint i = 0; i < dataBlock->blockCount; i++) {
		Block_Free(dataBlock->blocks[i]);
	}

	rm_free(dataBlock->blocks);
	array_free(dataBlock->deletedIdx);
	rm_free(dataBlock);
}

