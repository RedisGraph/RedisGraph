/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "datablock.h"

/*
 * Note!
 * This API is to be used only during out of order re-construction of a data block.
 * DO NOT USE THE BELOW METHODS COMBINED WITH THE NORMAL DATA BLACK API.
 */

/* Return a pointer to allocated item in a given index.
 * If needed, allocates new space.
 * Elements in the range [max_allocated_item.. idx] are neither considered deleted nor allocated,
 * Retrieving such an element will return garbage. */
void *DataBlock_AllocateItemOutOfOrder(DataBlock *dataBlock, uint64_t idx);

/* Marks item at position index as deleted.
 * Allocates additional space if need.
 * The function updates the data block's free list and set the element as deleted.
 * Note:
 * 1. Item distructor is not invoked in this call.
 * 2. This call does not decrease the number of items in the data block. */
void DataBlock_MarkAsDeletedOutOfOrder(DataBlock *dataBlock, uint64_t idx);
