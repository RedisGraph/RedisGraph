/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Apache License, Version 2.0,
 * modified with the Commons Clause restriction.
 */

#include "../../deps/googletest/include/gtest/gtest.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <string.h>
#include "../../src/util/datablock/datablock.h"

#ifdef __cplusplus
}
#endif

class DataBlockTest: public ::testing::Test {
};


TEST_F(DataBlockTest, NewDataBlock) {
    // Create a new data block, which can hold at least 1024 items
    // each item is an integer.
    size_t itemSize = sizeof(int);
    DataBlock *dataBlock = DataBlock_New(1024, itemSize);

    EXPECT_EQ(dataBlock->itemCount, 0);     // No items were added. 
    EXPECT_GE(dataBlock->itemCap, 1024);
    EXPECT_EQ(dataBlock->itemSize, itemSize);      
    EXPECT_GE(dataBlock->blockCount, 1024/BLOCK_CAP);

    for(int i = 0; i < dataBlock->blockCount; i++) {
        Block *block = dataBlock->blocks[i];
        EXPECT_EQ(block->itemSize, dataBlock->itemSize);
        EXPECT_TRUE(block->data != NULL);
        if(i > 0) {
            EXPECT_TRUE(dataBlock->blocks[i-1]->next == dataBlock->blocks[i]);
        }
    }

    DataBlock_Free(dataBlock);
}

TEST_F(DataBlockTest, AddItemToDataBlock) {
    DataBlock *dataBlock = DataBlock_New(1024, sizeof(int));
    size_t itemCount = 512;
    DataBlockIterator *it;
    DataBlock_AddItems(dataBlock, itemCount, &it);
    EXPECT_EQ(dataBlock->itemCount, itemCount);

    // Set items.
    for(int i = 0 ; i < itemCount; i++) {
        int *value = (int*)DataBlockIterator_Next(it);
        *value = i;        
    }

    // Read items.
    for(int i = 0 ; i < itemCount; i++) {
        int *value = (int*)DataBlock_GetItem(dataBlock, i);
        EXPECT_EQ(*value, i);
    }

    // Add enough item to cause datablock to re-allocate.
    size_t prevItemCount = dataBlock->itemCount;
    itemCount*=64;
    DataBlock_AddItems(dataBlock, itemCount, &it);
    EXPECT_EQ(dataBlock->itemCount, prevItemCount+itemCount);
    
    // Set items.
    for(int i = 0 ; i < itemCount; i++) {
        int *value = (int*)DataBlockIterator_Next(it);
        *value = prevItemCount + i;
    }

    // Read items.
    for(int i = 0 ; i < dataBlock->itemCount; i++) {
        int *value = (int*)DataBlock_GetItem(dataBlock, i);
        EXPECT_EQ(*value, i);
    }

    DataBlockIterator_Free(it);
    DataBlock_Free(dataBlock);
}

TEST_F(DataBlockTest, ScanDataBlock) {
    DataBlock *dataBlock = DataBlock_New(1024, sizeof(int));
    size_t itemCount = 2048;
    DataBlock_AddItems(dataBlock, itemCount, NULL);

    // Set items.
    for(int i = 0 ; i < itemCount; i++) {
        DataBlock_SetItem(dataBlock, &i, i);
    }

    // Scan through items.
    int count = 0;
    int *item = NULL;
    DataBlockIterator *it = DataBlock_Scan(dataBlock);
    while((item = (int*)DataBlockIterator_Next(it))) {
        EXPECT_EQ(*item, count);
        count++;
    }
    EXPECT_EQ(count, itemCount);

    item = (int*)DataBlockIterator_Next(it);
    // EXPECT_EQ(item, NULL);

    DataBlockIterator_Reset(it);

    // Re-scan through items.
    count = 0;
    item = NULL;
    while((item = (int*)DataBlockIterator_Next(it))) {
        EXPECT_EQ(*item, count);
        count++;
    }
    EXPECT_EQ(count, itemCount);

    item = (int*)DataBlockIterator_Next(it);
    // EXPECT_EQ(item, NULL);

    DataBlockIterator_Free(it);
}

TEST_F(DataBlockTest, GetItemBlock) {
    DataBlock *dataBlock = DataBlock_New(1024, sizeof(int));
    size_t itemCount = 2048;
    DataBlock_AddItems(dataBlock, itemCount, NULL);

    // Set items.
    for(int i = 0 ; i < itemCount; i++) {
        DataBlock_SetItem(dataBlock, &i, i);
    }

    for(int i = 0 ; i < itemCount; i++) {
        Block *block = DataBlock_GetItemBlock(dataBlock, i);
        EXPECT_EQ(block, dataBlock->blocks[i/BLOCK_CAP]);
    }
}

TEST_F(DataBlockTest, MigrateItem) {
    DataBlock *dataBlock = DataBlock_New(1024, sizeof(int));
    size_t itemCount = 2048;
    DataBlock_AddItems(dataBlock, itemCount, NULL);

    // Set items.
    for(int i = 0 ; i < itemCount; i++) {
        DataBlock_SetItem(dataBlock, &i, i);
    }

    for(int i = dataBlock->itemCount-1; i > 0; i--) {
        int *migratedItem = (int*)DataBlock_GetItem(dataBlock, i);
        DataBlock_CopyItem(dataBlock, i, 0);
        DataBlock_FreeTop(dataBlock, 1);
        int *item = (int*)DataBlock_GetItem(dataBlock, 0);
        EXPECT_EQ(*migratedItem, *item);
        EXPECT_EQ(dataBlock->itemCount, i);
    }
}
