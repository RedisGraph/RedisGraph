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
#include "../../src/util/circular_buffer_nrg.h"
#ifdef __cplusplus
}
#endif

TEST(CircularBufferNRG, TestWrapsWrites) {
    CircularBufferNRG cb = CircularBufferNRG_New(sizeof(uint8_t), 2);
    ASSERT_TRUE(cb);
    ASSERT_EQ(CircularBufferNRG_GetCapacity(cb), 2);
    ASSERT_TRUE(CircularBufferNRG_IsEmpty(cb));
    ASSERT_EQ(CircularBufferNRG_ItemCount(cb), 0);
    ASSERT_FALSE(CircularBufferNRG_ViewCurrent(cb));

    uint8_t value = 1;
    CircularBufferNRG_Add(cb, &value);
    ASSERT_FALSE(CircularBufferNRG_IsEmpty(cb));
    ASSERT_EQ(CircularBufferNRG_GetCapacity(cb), 2);
    ASSERT_EQ(CircularBufferNRG_ItemCount(cb), 1);
    uint8_t viewed = *(uint8_t *)CircularBufferNRG_ViewCurrent(cb);
    ASSERT_EQ(viewed, 1);
    ASSERT_EQ(CircularBufferNRG_ItemCount(cb), 1);

    value = 2;
    CircularBufferNRG_Add(cb, &value);
    ASSERT_EQ(CircularBufferNRG_GetCapacity(cb), 2);
    viewed = *(uint8_t*)CircularBufferNRG_ViewCurrent(cb);
    ASSERT_EQ(viewed, 1);
    ASSERT_EQ(CircularBufferNRG_ItemCount(cb), 2);

    value = 3;
    CircularBufferNRG_Add(cb, &value);
    ASSERT_EQ(CircularBufferNRG_GetCapacity(cb), 2);
    // The read pointer still points to the least recently read entry,
    // after overwrite. Here, the "1" was overwritten by "3", while "2"
    // was left untouched, hence the "current" points to the first unread
    // entry, which is "2" in this case.
    viewed = *(uint8_t*)CircularBufferNRG_ViewCurrent(cb);
    ASSERT_EQ(viewed, 2);
    ASSERT_EQ(CircularBufferNRG_ItemCount(cb), 2);

    value = 4;
    CircularBufferNRG_Add(cb, &value);
    ASSERT_EQ(CircularBufferNRG_GetCapacity(cb), 2);
    // The read pointer has never been advanced by reading, and so now
    // should just point to the earliest possible entry. In this case, it
    // is located at the zeroeth offset, with the value of "3".
    viewed = *(uint8_t*)CircularBufferNRG_ViewCurrent(cb);
    ASSERT_EQ(viewed, 3);

    CircularBufferNRG_Free(cb);
}

TEST(CircularBufferNRG, TestWrapsReads) {
    CircularBufferNRG cb = CircularBufferNRG_New(sizeof(uint8_t), 2);
    uint8_t value = 1;
    CircularBufferNRG_Add(cb, &value);
    uint8_t removed = 0;
    CircularBufferNRG_Read(cb, &removed);
    ASSERT_EQ(removed, 1);
    value = 2;
    CircularBufferNRG_Add(cb, &value);
    CircularBufferNRG_Read(cb, &removed);
    ASSERT_EQ(removed, 2);
    value = 3;
    CircularBufferNRG_Add(cb, &value);
    CircularBufferNRG_Read(cb, &removed);
    ASSERT_EQ(removed, 3);

    value = 4;
    CircularBufferNRG_Add(cb, &value);
    value = 5;
    CircularBufferNRG_Add(cb, &value);
    CircularBufferNRG_Read(cb, &removed);
    ASSERT_EQ(removed, 4);
    CircularBufferNRG_Read(cb, &removed);
    ASSERT_EQ(removed, 5);
    value = 6;
    CircularBufferNRG_Add(cb, &value);
    value = 7;
    CircularBufferNRG_Add(cb, &value);
    value = 8;
    CircularBufferNRG_Add(cb, &value);
    CircularBufferNRG_Read(cb, &removed);
    ASSERT_EQ(removed, 7);
    CircularBufferNRG_Read(cb, &removed);
    ASSERT_EQ(removed, 8);
    removed = 0;
    CircularBufferNRG_Read(cb, &removed);
    // When nothing to remove, the pointer remains unchanged.
    ASSERT_EQ(removed, 0);

    CircularBufferNRG_Free(cb);
}

TEST(CircularBufferNRG, TestResetCapacitySingleStageNoReallocation) {
    CircularBufferNRG cb = CircularBufferNRG_New(sizeof(uint8_t), 5);
    uint8_t value = 1;
    CircularBufferNRG_Add(cb, &value);
    value = 2;
    CircularBufferNRG_Add(cb, &value);
    value = 3;
    CircularBufferNRG_Add(cb, &value);
    value = 4;
    CircularBufferNRG_Add(cb, &value);
    value = 5;
    CircularBufferNRG_Add(cb, &value);

    CircularBufferNRG old_address = cb;
    // Returns true as if the capacity setting went okay.
    ASSERT_TRUE(CircularBufferNRG_SetCapacity(&cb, 5));
    // The address hasn't changed, meaning there has been no reallocation.
    ASSERT_EQ(old_address, cb);

    CircularBufferNRG_Free(cb);
}

bool view_element(void *user_data, const void *item) {
    std::vector<uint8_t> *viewed = reinterpret_cast<std::vector<uint8_t>*>(user_data);
    const uint8_t element = *reinterpret_cast<const uint8_t *>(item);
    viewed->push_back(element);

    return false;
}

TEST(CircularBufferNRG, TestReadAllFullCollection) {
    CircularBufferNRG cb = CircularBufferNRG_New(sizeof(uint8_t), 5);
    uint8_t value = 1;
    CircularBufferNRG_Add(cb, &value);
    value = 2;
    CircularBufferNRG_Add(cb, &value);
    value = 3;
    CircularBufferNRG_Add(cb, &value);
    value = 4;
    CircularBufferNRG_Add(cb, &value);
    value = 5;
    CircularBufferNRG_Add(cb, &value);

    const uint8_t old_current = *(uint8_t*)CircularBufferNRG_ViewCurrent(cb);
    std::vector<uint8_t> viewed;
    CircularBufferNRG_ReadAll(cb, view_element, reinterpret_cast<void *>(&viewed));
    const std::vector<uint8_t> expected = { 1, 2, 3, 4, 5 };
    ASSERT_EQ(viewed, expected);
    const uint8_t current = *(uint8_t*)CircularBufferNRG_ViewCurrent(cb);
    ASSERT_EQ(current, old_current);

    CircularBufferNRG_Free(cb);
}

struct ViewElementAndEndData {
    std::vector<uint8_t> viewed;
    uint64_t max_view_count;
};

bool view_element_and_end(void *user_data, const void *item) {
    ViewElementAndEndData *data = reinterpret_cast<ViewElementAndEndData*>(user_data);

    const uint8_t element = *reinterpret_cast<const uint8_t *>(item);
    data->viewed.push_back(element);

    if (data->viewed.size() >= data->max_view_count) {
        return true;
    }

    return false;
}

TEST(CircularBufferNRG, TestReadAllAndEndPrematurely) {
    CircularBufferNRG cb = CircularBufferNRG_New(sizeof(uint8_t), 5);
    uint8_t value = 1;
    CircularBufferNRG_Add(cb, &value);
    value = 2;
    CircularBufferNRG_Add(cb, &value);
    value = 3;
    CircularBufferNRG_Add(cb, &value);
    value = 4;
    CircularBufferNRG_Add(cb, &value);
    value = 5;
    CircularBufferNRG_Add(cb, &value);

    ViewElementAndEndData data {};
    data.max_view_count = 3;
    CircularBufferNRG_ReadAll(cb, view_element_and_end, reinterpret_cast<void *>(&data));
    const std::vector<uint8_t> expected = { 1, 2, 3 };
    ASSERT_EQ(data.viewed, expected);
    const uint8_t current = *(uint8_t*)CircularBufferNRG_ViewCurrent(cb);
    // The read offset has moved because of the "read", so we should not
    // expect to be able to read the same elements again. As we stopped
    // reading right after the element "3", the next read or a view are
    // supposed to look at the element after, which is "4".
    ASSERT_EQ(current, 4);

    CircularBufferNRG_Free(cb);
}

TEST(CircularBufferNRG, TestViewAll) {
    CircularBufferNRG cb = CircularBufferNRG_New(sizeof(uint8_t), 5);
    uint8_t value = 1;
    CircularBufferNRG_Add(cb, &value);
    value = 2;
    CircularBufferNRG_Add(cb, &value);
    value = 3;
    CircularBufferNRG_Add(cb, &value);
    value = 4;
    CircularBufferNRG_Add(cb, &value);
    value = 5;
    CircularBufferNRG_Add(cb, &value);

    const uint8_t old_current = *(uint8_t*)CircularBufferNRG_ViewCurrent(cb);
    std::vector<uint8_t> viewed;
    CircularBufferNRG_ViewAll(cb, view_element, reinterpret_cast<void *>(&viewed));
    const std::vector<uint8_t> expected = { 1, 2, 3, 4, 5 };
    ASSERT_EQ(viewed, expected);
    const uint8_t current = *(uint8_t*)CircularBufferNRG_ViewCurrent(cb);
    // The read offset hasn't moved because of the "view", so we should
    // expect to be able to read the same elements again, as there has been
    // no read in between the calls.
    ASSERT_EQ(current, old_current);

    CircularBufferNRG_Free(cb);
}

TEST(CircularBufferNRG, TestViewAllWithCircle) {
    CircularBufferNRG cb = CircularBufferNRG_New(sizeof(uint8_t), 3);
    uint8_t value = 1;
    CircularBufferNRG_Add(cb, &value);
    value = 2;
    CircularBufferNRG_Add(cb, &value);
    value = 3;
    CircularBufferNRG_Add(cb, &value);
    value = 4;
    CircularBufferNRG_Add(cb, &value);
    value = 5;
    CircularBufferNRG_Add(cb, &value);

    {
        const uint8_t old_current = *(uint8_t*)CircularBufferNRG_ViewCurrent(cb);
        std::vector<uint8_t> viewed;
        CircularBufferNRG_ViewAll(cb, view_element, reinterpret_cast<void *>(&viewed));
        const std::vector<uint8_t> expected = { 3, 4, 5 };
        ASSERT_EQ(viewed, expected);
        const uint8_t current = *(uint8_t*)CircularBufferNRG_ViewCurrent(cb);
        // The read offset hasn't moved because of the "view", so we should
        // expect to be able to read the same elements again, as there has been
        // no read in between the calls.
        ASSERT_EQ(current, old_current);
    }
    value = 6;
    CircularBufferNRG_Add(cb, &value);
    value = 7;
    CircularBufferNRG_Add(cb, &value);

    {
        const uint8_t old_current = *(uint8_t*)CircularBufferNRG_ViewCurrent(cb);
        std::vector<uint8_t> viewed;
        CircularBufferNRG_ViewAll(cb, view_element, reinterpret_cast<void *>(&viewed));
        const std::vector<uint8_t> expected = { 5, 6, 7 };
        ASSERT_EQ(viewed, expected);
        const uint8_t current = *(uint8_t*)CircularBufferNRG_ViewCurrent(cb);
        // The read offset hasn't moved because of the "view", so we should
        // expect to be able to read the same elements again, as there has been
        // no read in between the calls.
        ASSERT_EQ(current, old_current);
    }


    CircularBufferNRG_Free(cb);
}

void delete_element(void *user_data, void *item) {
    uint64_t *deleted_counter = reinterpret_cast<uint64_t*>(user_data);
    char** element = reinterpret_cast<char **>(item);
    free(*element);
    ++(*deleted_counter);
}

TEST(CircularBufferNRG, TestCustomDeleterIsUsedProperly) {
    CircularBufferNRG cb = CircularBufferNRG_New(sizeof(char *), 2);
    uint64_t deleted_counter = 0;
    CircularBufferNRG_SetDeleter(
        cb,
        delete_element,
        reinterpret_cast<void *>(&deleted_counter)
    );
    char *s = strdup("11");
    CircularBufferNRG_Add(cb, &s);
    s = strdup("22");
    CircularBufferNRG_Add(cb, &s);
    s = strdup("33");
    CircularBufferNRG_Add(cb, &s);

    // We deleted "11".
    ASSERT_EQ(deleted_counter, 1);

    // Reads don't delete.
    char *r = NULL;
    CircularBufferNRG_Read(cb, reinterpret_cast<void *>(&r));
    ASSERT_TRUE(!strcmp(r, "22"));
    ASSERT_EQ(deleted_counter, 1);

    CircularBufferNRG_Free(cb);
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

    CircularBufferNRG cb = CircularBufferNRG_New(sizeof(uint8_t), param.initialCapacity);
    for (const auto element : param.initialElements) {
        CircularBufferNRG_Add(cb, &element);
    }
    const uint64_t original_item_count = CircularBufferNRG_ItemCount(cb);
    ASSERT_EQ(original_item_count, MIN(param.initialElements.size(), param.initialCapacity));

    CircularBufferNRG old_address = cb;
    ASSERT_TRUE(CircularBufferNRG_SetCapacity(&cb, param.newCapacity));
    if (param.expectedReallocation) {
        // The address has changed, meaning there has been a reallocation.
        ASSERT_NE(old_address, cb);
    } else {
        // The address hasn't changed, meaning there has been no reallocation.
        ASSERT_EQ(old_address, cb);
    }

    ASSERT_EQ(CircularBufferNRG_GetCapacity(cb), param.newCapacity);
    ASSERT_EQ(CircularBufferNRG_IsEmpty(cb), param.expectedEmpty);
    ASSERT_EQ(CircularBufferNRG_ItemCount(cb), param.expectedElements.size());

    // Confirm the new buffer has only the elements expected.
    for (const auto element : param.expectedElements) {
        uint8_t removed = 0;
        CircularBufferNRG_Read(cb, &removed);
        ASSERT_EQ(removed, element);
    }

    // After all the expected elements have been read, we shouldn't be able
    // to read more.
    uint8_t removed = 127;
    CircularBufferNRG_Read(cb, &removed);
    ASSERT_EQ(removed, 127);

    CircularBufferNRG_Free(cb);
}

INSTANTIATE_TEST_SUITE_P(
        CircularBufferNRGReallocation1Tests,
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
