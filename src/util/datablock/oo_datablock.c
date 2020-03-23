/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "oo_datablock.h"
#include "../arr.h"

inline void *DataBlock_AllocateItemOutOfOrder(DataBlock *dataBlock, uint64_t idx) {
	// Check if idx<=data block's current capacity. If needed, allocate additional blocks.
	DataBlock_Accommodate(dataBlock, idx);
	DataBlockItemHeader *item_header = DataBlock_GetItemHeader(dataBlock, idx);
	MARK_HEADER_AS_NOT_DELETED(item_header);
	dataBlock->itemCount++;
	return ITEM_DATA(item_header);
}

inline void DataBlock_MarkAsDeletedOutOfOrder(DataBlock *dataBlock, uint64_t idx) {
	// Check if idx<=data block's current capacity. If needed, allocate additional blocks.
	DataBlock_Accommodate(dataBlock, idx);
	DataBlockItemHeader *item_header = DataBlock_GetItemHeader(dataBlock, idx);
	// Delete
	MARK_HEADER_AS_DELETED(item_header);
	dataBlock->deletedIdx = array_append(dataBlock->deletedIdx, idx);
}
