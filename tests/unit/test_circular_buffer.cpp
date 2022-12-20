/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "gtest.h"

#include <vector>

#include <sys/param.h>

#ifdef __cplusplus
extern "C" {
#endif
#include "../../src/util/circular_buffer.h"
#ifdef __cplusplus
}
#endif

TEST(CircularBuffer, TestWrapsWrites) {
    CircularBuffer cb = CircularBuffer_New(sizeof(uint8_t), 2);
    ASSERT_TRUE(cb);
    ASSERT_EQ(CircularBuffer_GetCapacity(cb), 2);
    ASSERT_TRUE(CircularBuffer_IsEmpty(cb));
    ASSERT_EQ(CircularBuffer_ItemCount(cb), 0);
    ASSERT_FALSE(CircularBuffer_ViewCurrent(cb));

    uint8_t value = 1;
    CircularBuffer_Add(cb, &value);
    ASSERT_FALSE(CircularBuffer_IsEmpty(cb));
    ASSERT_EQ(CircularBuffer_GetCapacity(cb), 2);
    ASSERT_EQ(CircularBuffer_ItemCount(cb), 1);
    uint8_t viewed = *(uint8_t *)CircularBuffer_ViewCurrent(cb);
    ASSERT_EQ(viewed, 1);
    ASSERT_EQ(CircularBuffer_ItemCount(cb), 1);

    value = 2;
    CircularBuffer_Add(cb, &value);
    ASSERT_EQ(CircularBuffer_GetCapacity(cb), 2);
    viewed = *(uint8_t*)CircularBuffer_ViewCurrent(cb);
    ASSERT_EQ(viewed, 1);
    ASSERT_EQ(CircularBuffer_ItemCount(cb), 2);

    value = 3;
    CircularBuffer_Add(cb, &value);
    ASSERT_EQ(CircularBuffer_GetCapacity(cb), 2);
    // The read pointer still points to the least recently read entry,
    // after overwrite. Here, the "1" was overwritten by "3", while "2"
    // was left untouched, hence the "current" points to the first unread
    // entry, which is "2" in this case.
    viewed = *(uint8_t*)CircularBuffer_ViewCurrent(cb);
    ASSERT_EQ(viewed, 2);
    ASSERT_EQ(CircularBuffer_ItemCount(cb), 2);

    value = 4;
    CircularBuffer_Add(cb, &value);
    ASSERT_EQ(CircularBuffer_GetCapacity(cb), 2);
    // The read pointer has never been advanced by reading, and so now
    // should just point to the earliest possible entry. In this case, it
    // is located at the zeroeth offset, with the value of "3".
    viewed = *(uint8_t*)CircularBuffer_ViewCurrent(cb);
    ASSERT_EQ(viewed, 3);

    CircularBuffer_Free(cb);
}

TEST(CircularBuffer, TestWrapsReads) {
    CircularBuffer cb = CircularBuffer_New(sizeof(uint8_t), 2);
    uint8_t value = 1;
    CircularBuffer_Add(cb, &value);
    uint8_t removed = 0;
    CircularBuffer_Read(cb, &removed);
    ASSERT_EQ(removed, 1);
    value = 2;
    CircularBuffer_Add(cb, &value);
    CircularBuffer_Read(cb, &removed);
    ASSERT_EQ(removed, 2);
    value = 3;
    CircularBuffer_Add(cb, &value);
    CircularBuffer_Read(cb, &removed);
    ASSERT_EQ(removed, 3);

    value = 4;
    CircularBuffer_Add(cb, &value);
    value = 5;
    CircularBuffer_Add(cb, &value);
    CircularBuffer_Read(cb, &removed);
    ASSERT_EQ(removed, 4);
    CircularBuffer_Read(cb, &removed);
    ASSERT_EQ(removed, 5);
    value = 6;
    CircularBuffer_Add(cb, &value);
    value = 7;
    CircularBuffer_Add(cb, &value);
    value = 8;
    CircularBuffer_Add(cb, &value);
    CircularBuffer_Read(cb, &removed);
    ASSERT_EQ(removed, 7);
    CircularBuffer_Read(cb, &removed);
    ASSERT_EQ(removed, 8);
    removed = 0;
    CircularBuffer_Read(cb, &removed);
    // When nothing to remove, the pointer remains unchanged.
    ASSERT_EQ(removed, 0);

    CircularBuffer_Free(cb);
}

TEST(CircularBuffer, TestResetCapacitySingleStageNoReallocation) {
    CircularBuffer cb = CircularBuffer_New(sizeof(uint8_t), 5);
    uint8_t value = 1;
    CircularBuffer_Add(cb, &value);
    value = 2;
    CircularBuffer_Add(cb, &value);
    value = 3;
    CircularBuffer_Add(cb, &value);
    value = 4;
    CircularBuffer_Add(cb, &value);
    value = 5;
    CircularBuffer_Add(cb, &value);

    CircularBuffer old_address = cb;
    // Returns true as if the capacity setting went okay.
    ASSERT_TRUE(CircularBuffer_SetCapacity(&cb, 5));
    // The address hasn't changed, meaning there has been no reallocation.
    ASSERT_EQ(old_address, cb);

    CircularBuffer_Free(cb);
}

void view_element(void *user_data, const void *item) {
    std::vector<uint8_t> *viewed = reinterpret_cast<std::vector<uint8_t>*>(user_data);
    const uint8_t element = *reinterpret_cast<const uint8_t *>(item);
    viewed->push_back(element);
}

TEST(CircularBuffer, TestViewAll) {
    CircularBuffer cb = CircularBuffer_New(sizeof(uint8_t), 5);
    uint8_t value = 1;
    CircularBuffer_Add(cb, &value);
    value = 2;
    CircularBuffer_Add(cb, &value);
    value = 3;
    CircularBuffer_Add(cb, &value);
    value = 4;
    CircularBuffer_Add(cb, &value);
    value = 5;
    CircularBuffer_Add(cb, &value);

    std::vector<uint8_t> viewed;
    CircularBuffer_ViewAll(cb, view_element, reinterpret_cast<void *>(&viewed));
    const std::vector<uint8_t> expected = { 1, 2, 3, 4, 5 };
    ASSERT_EQ(viewed, expected);

    CircularBuffer_Free(cb);
}

void delete_element(void *user_data, void *item) {
    uint64_t *deleted_counter = reinterpret_cast<uint64_t*>(user_data);
    char** element = reinterpret_cast<char **>(item);
    free(*element);
    ++(*deleted_counter);
}

TEST(CircularBuffer, TestCustomDeleterIsUsedProperly) {
    CircularBuffer cb = CircularBuffer_New(sizeof(char *), 2);
    uint64_t deleted_counter = 0;
    CircularBuffer_SetDeleter(
        cb,
        delete_element,
        reinterpret_cast<void *>(&deleted_counter)
    );
    char *s = strdup("11");
    CircularBuffer_Add(cb, &s);
    s = strdup("22");
    CircularBuffer_Add(cb, &s);
    s = strdup("33");
    CircularBuffer_Add(cb, &s);

    // We deleted "11".
    ASSERT_EQ(deleted_counter, 1);

    // Reads don't delete.
    char *r = NULL;
    CircularBuffer_Read(cb, reinterpret_cast<void *>(&r));
    ASSERT_TRUE(!strcmp(r, "22"));
    ASSERT_EQ(deleted_counter, 1);

    CircularBuffer_Free(cb);
    ASSERT_EQ(deleted_counter, 3);
}

struct Reallocation {
    uint64_t initialCapacity;
    std::vector<uint8_t> initialElements;
    uint64_t newCapacity;
    bool expectedReallocation;
    bool expectedEmpty;
    std::vector<uint8_t> expectedElements;
};

class Reallocation1Fixture: public ::testing::TestWithParam<Reallocation> {
};

TEST_P(Reallocation1Fixture, TestResetCapacitySingleStageWithReallocation1Parametrised) {
    const Reallocation param = GetParam();

    CircularBuffer cb = CircularBuffer_New(sizeof(uint8_t), param.initialCapacity);
    for (const auto element : param.initialElements) {
        CircularBuffer_Add(cb, &element);
    }
    const uint64_t original_item_count = CircularBuffer_ItemCount(cb);
    ASSERT_EQ(original_item_count, MIN(param.initialElements.size(), param.initialCapacity));

    CircularBuffer old_address = cb;
    ASSERT_TRUE(CircularBuffer_SetCapacity(&cb, param.newCapacity));
    if (param.expectedReallocation) {
        // The address has changed, meaning there has been a reallocation.
        ASSERT_NE(old_address, cb);
    } else {
        // The address hasn't changed, meaning there has been no reallocation.
        ASSERT_EQ(old_address, cb);
    }

    ASSERT_EQ(CircularBuffer_GetCapacity(cb), param.newCapacity);
    ASSERT_EQ(CircularBuffer_IsEmpty(cb), param.expectedEmpty);
    ASSERT_EQ(CircularBuffer_ItemCount(cb), param.expectedElements.size());

    // Confirm the new buffer has only the elements expected.
    for (const auto element : param.expectedElements) {
        uint8_t removed = 0;
        CircularBuffer_Read(cb, &removed);
        ASSERT_EQ(removed, element);
    }

    // After all the expected elements have been read, we shouldn't be able
    // to read more.
    uint8_t removed = 127;
    CircularBuffer_Read(cb, &removed);
    ASSERT_EQ(removed, 127);

    CircularBuffer_Free(cb);
}

INSTANTIATE_TEST_SUITE_P(
        CircularBufferReallocation1Tests,
        Reallocation1Fixture,
        ::testing::Values<Reallocation>(
            Reallocation {
                // We simply reduce the capacity and reallocate.
                6, { 1, 2, 3, 4 }, 5, true, false, { 1, 2, 3, 4 }
            },
            Reallocation {
                // We simply reduce the capacity and reallocate.
                6, { 1, 2, 3, 4, 5 }, 5, true, false, { 1, 2, 3, 4, 5 }
            },
            Reallocation {
                // We reduce the capacity, reallocate and copy only the
                // most recently added elements.
                6, { 1, 2, 3, 4, 5, 6, 7, 8, 9 }, 5, true, false, { 5, 6, 7, 8, 9 }
            }
        ));
