/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */


#include <sys/param.h>

#include "src/util/circular_buffer_nrg.h"
#include "src/util/rmalloc.h"
#include "src/util/arr.h"

void setup() {
	Alloc_Reset();
}

#define TEST_INIT setup();
#include "acutest.h"

// GTest to AcuTest redefinitions
#define ASSERT_EQ(lhs, rhs) TEST_ASSERT(lhs == rhs)
#define ASSERT_NE(lhs, rhs) TEST_ASSERT(lhs != rhs)
#define ASSERT_NOT_NULL(lhs) ASSERT_NE(lhs, NULL)

void test_CircularBufferNRG_TestWrapsWrites(void) {
    CircularBufferNRG cb = CircularBufferNRG_New(sizeof(uint8_t), 2);
    ASSERT_NOT_NULL(cb);
    ASSERT_EQ(CircularBufferNRG_GetCapacity(cb), 2);
    TEST_ASSERT(CircularBufferNRG_IsEmpty(cb));
    ASSERT_EQ(CircularBufferNRG_ItemCount(cb), 0);
    TEST_ASSERT(!CircularBufferNRG_ViewCurrent(cb));

    uint8_t value = 1;
    CircularBufferNRG_Add(cb, &value);
    TEST_ASSERT(!CircularBufferNRG_IsEmpty(cb));
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

void test_CircularBufferNRG_TestWrapsReads(void) {
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

void test_CircularBufferNRG_TestResetCapacitySingleStageNoReallocation(void) {
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
    TEST_ASSERT(CircularBufferNRG_SetCapacity(&cb, 5));
    // The address hasn't changed, meaning there has been no reallocation.
    ASSERT_EQ(old_address, cb);

    CircularBufferNRG_Free(cb);
}

bool view_element(void *user_data, const void *item) {
    uint8_t *viewed = (uint8_t*)(user_data);
    const uint8_t element = *(const uint8_t *)item;
    array_append(viewed, element);

    return false;
}

void test_CircularBufferNRG_TestReadAllFullCollection(void) {
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
    uint8_t *viewed = array_new(uint8_t, 10);
    CircularBufferNRG_ReadAll(cb, view_element, (void*)viewed);
    static const uint8_t EXPECTED[] = { 1, 2, 3, 4, 5 };
    static const size_t LENGTH = sizeof(EXPECTED) / sizeof(EXPECTED[0]);
    ASSERT_EQ(array_len(viewed), LENGTH);
    for (size_t i = 0; i < LENGTH; ++i) {
        ASSERT_EQ(viewed[i], EXPECTED[i]);
    }
    const uint8_t current = *(uint8_t*)CircularBufferNRG_ViewCurrent(cb);
    ASSERT_EQ(current, old_current);

    array_free(viewed);
    CircularBufferNRG_Free(cb);
}

typedef struct ViewElementAndEndData {
    uint8_t *viewed;
    uint64_t max_view_count;
} ViewElementAndEndData;

bool view_element_and_end(void *user_data, const void *item) {
    ViewElementAndEndData *data = (ViewElementAndEndData*)user_data;

    const uint8_t element = *(const uint8_t *)item;
    array_append(data->viewed, element);

    if (array_len(data->viewed) >= data->max_view_count) {
        return true;
    }

    return false;
}

void test_CircularBufferNRG_TestReadAllAndEndPrematurely(void) {
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

    ViewElementAndEndData data = {};
    data.viewed = array_new(uint8_t, 10);
    data.max_view_count = 3;
    CircularBufferNRG_ReadAll(cb, view_element_and_end, (void *)&data);
    static const uint8_t EXPECTED[] = { 1, 2, 3 };
    static const size_t LENGTH = sizeof(EXPECTED) / sizeof(EXPECTED[0]);
    ASSERT_EQ(array_len(data.viewed), LENGTH);
    for (size_t i = 0; i < LENGTH; ++i) {
        ASSERT_EQ(data.viewed[i], EXPECTED[i]);
    }
    const uint8_t current = *(uint8_t*)CircularBufferNRG_ViewCurrent(cb);
    // The read offset has moved because of the "read", so we should not
    // expect to be able to read the same elements again. As we stopped
    // reading right after the element "3", the next read or a view are
    // supposed to look at the element after, which is "4".
    ASSERT_EQ(current, 4);

    array_free(data.viewed);
    CircularBufferNRG_Free(cb);
}

void test_CircularBufferNRG_TestViewAll(void) {
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
    uint8_t *viewed = array_new(uint8_t, 10);
    CircularBufferNRG_ViewAll(cb, view_element, (void *)viewed);
    static const uint8_t EXPECTED[] = { 1, 2, 3, 4, 5 };
    static const size_t LENGTH = sizeof(EXPECTED) / sizeof(EXPECTED[0]);
    ASSERT_EQ(array_len(viewed), LENGTH);
    for (size_t i = 0; i < LENGTH; ++i) {
        ASSERT_EQ(viewed[i], EXPECTED[i]);
    }
    const uint8_t current = *(uint8_t*)CircularBufferNRG_ViewCurrent(cb);
    // The read offset hasn't moved because of the "view", so we should
    // expect to be able to read the same elements again, as there has been
    // no read in between the calls.
    ASSERT_EQ(current, old_current);

    array_free(viewed);
    CircularBufferNRG_Free(cb);
}

void test_CircularBufferNRG_TestViewAllWithCircle(void) {
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
        uint8_t *viewed = array_new(uint8_t, 10);
        CircularBufferNRG_ViewAll(cb, view_element, (void*)viewed);
        const uint8_t EXPECTED[] = { 3, 4, 5 };
        const size_t LENGTH = sizeof(EXPECTED) / sizeof(EXPECTED[0]);
        ASSERT_EQ(array_len(viewed), LENGTH);
        for (size_t i = 0; i < LENGTH; ++i) {
            ASSERT_EQ(viewed[i], EXPECTED[i]);
        }
        const uint8_t current = *(uint8_t*)CircularBufferNRG_ViewCurrent(cb);
        // The read offset hasn't moved because of the "view", so we should
        // expect to be able to read the same elements again, as there has been
        // no read in between the calls.
        ASSERT_EQ(current, old_current);
        array_free(viewed);
    }

    value = 6;
    CircularBufferNRG_Add(cb, &value);
    value = 7;
    CircularBufferNRG_Add(cb, &value);

    {
        const uint8_t old_current = *(uint8_t*)CircularBufferNRG_ViewCurrent(cb);
        uint8_t *viewed = array_new(uint8_t, 10);
        CircularBufferNRG_ViewAll(cb, view_element, (void *)viewed);
        const uint8_t EXPECTED[] = { 5, 6, 7 };
        const size_t LENGTH = sizeof(EXPECTED) / sizeof(EXPECTED[0]);
        ASSERT_EQ(array_len(viewed), LENGTH);
        for (size_t i = 0; i < LENGTH; ++i) {
            ASSERT_EQ(viewed[i], EXPECTED[i]);
        }
        const uint8_t current = *(uint8_t*)CircularBufferNRG_ViewCurrent(cb);
        // The read offset hasn't moved because of the "view", so we should
        // expect to be able to read the same elements again, as there has been
        // no read in between the calls.
        ASSERT_EQ(current, old_current);
        array_free(viewed);
    }


    CircularBufferNRG_Free(cb);
}

void delete_element(void *user_data, void *item) {
    uint64_t *deleted_counter = (uint64_t*)user_data;
    char** element = (char **)item;
    free(*element);
    ++(*deleted_counter);
}

void delete_count(void *user_data, void *item) {
    uint64_t *deleted_counter = (uint64_t*)user_data;
    ++(*deleted_counter);
}

void test_CircularBufferNRG_TestCustomDeleterIsUsedProperly(void) {
    CircularBufferNRG cb = CircularBufferNRG_New(sizeof(char *), 2);
    uint64_t deleted_counter = 0;
    CircularBufferNRG_SetDeleter(
        cb,
        delete_element,
        (void *)&deleted_counter
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
    CircularBufferNRG_Read(cb, (void *)&r);
    TEST_ASSERT(!strcmp(r, "22"));
    ASSERT_EQ(deleted_counter, 1);

    CircularBufferNRG_Free(cb);
    ASSERT_EQ(deleted_counter, 3);
}

void clone_func(const void *source, void *destination, void *user_data) {
    uint64_t *copied_counter = (uint64_t*)user_data;
    *(uint8_t*)destination = *(uint8_t*)source;
    ++(*copied_counter);
}

typedef struct Reallocation {
    uint64_t initialCapacity;
    uint8_t initial_elements_start;
    uint8_t initial_elements_count;
    uint64_t newCapacity;
    bool expectedReallocation;
    bool expectedEmpty;
    uint8_t expected_elements_start;
    uint8_t expected_elements_count;
} Reallocation;

void test_CircularBufferNRG_TestResetCapacitySingleStageWithReallocation(void) {
    const Reallocation reallocations[] = {
        {
            // We simply reduce the capacity and reallocate.
            6, 1, 4, 5, true, false, 1, 4
        },
        {
            // We simply reduce the capacity and reallocate.
            6, 1, 5, 5, true, false, 1, 5
        },
        {
            // We reduce the capacity, reallocate and copy only the
            // most recently added elements.
            6, 1, 9, 5, true, false, 5, 5
        }
    };

    const size_t reallocations_count = sizeof(reallocations) / sizeof(reallocations[0]);
    for (size_t i = 0; i < reallocations_count; ++i) {
        const Reallocation param = reallocations[i];

        CircularBufferNRG cb = CircularBufferNRG_New(sizeof(uint8_t), param.initialCapacity);
        for (uint8_t j = 0; j < param.initial_elements_count; ++j) {
            const uint8_t element = param.initial_elements_start + j;
            CircularBufferNRG_Add(cb, &element);
        }
        const uint64_t original_item_count = CircularBufferNRG_ItemCount(cb);
        ASSERT_EQ(original_item_count, MIN(param.initial_elements_count, param.initialCapacity));

        uint64_t deleted_counter = 0;
        CircularBufferNRG_SetDeleter(
            cb,
            delete_count,
            (void *)&deleted_counter
        );
        uint64_t copied_counter = 0;
        CircularBufferNRG_SetItemClone(
            cb,
            clone_func,
            (void*)&copied_counter
        );
        CircularBufferNRG old_address = cb;
        TEST_ASSERT(CircularBufferNRG_SetCapacity(&cb, param.newCapacity));
        if (param.expectedReallocation) {
            // The address has changed, meaning there has been a reallocation.
            ASSERT_NE(old_address, cb);
            ASSERT_EQ(deleted_counter, original_item_count);
            ASSERT_EQ(copied_counter, MIN(param.newCapacity, original_item_count));
        } else {
            // The address hasn't changed, meaning there has been no reallocation.
            ASSERT_EQ(old_address, cb);
        }

        ASSERT_EQ(CircularBufferNRG_GetCapacity(cb), param.newCapacity);
        ASSERT_EQ(CircularBufferNRG_IsEmpty(cb), param.expectedEmpty);
        ASSERT_EQ(CircularBufferNRG_ItemCount(cb), param.expected_elements_count);

        // Confirm the new buffer has only the elements expected.
        for (uint8_t j = 0; j < param.expected_elements_count; ++j) {
            const uint8_t element = param.expected_elements_start + j;
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
}

TEST_LIST = {
    { "test_CircularBufferNRG_TestResetCapacitySingleStageWithReallocation", test_CircularBufferNRG_TestResetCapacitySingleStageWithReallocation },
    { "test_CircularBufferNRG_TestCustomDeleterIsUsedProperly", test_CircularBufferNRG_TestCustomDeleterIsUsedProperly },
    { "test_CircularBufferNRG_TestViewAllWithCircle", test_CircularBufferNRG_TestViewAllWithCircle },
    { "test_CircularBufferNRG_TestViewAll", test_CircularBufferNRG_TestViewAll },
    { "test_CircularBufferNRG_TestReadAllAndEndPrematurely", test_CircularBufferNRG_TestReadAllAndEndPrematurely },
    { "test_CircularBufferNRG_TestReadAllFullCollection", test_CircularBufferNRG_TestReadAllFullCollection },
    { "test_CircularBufferNRG_TestResetCapacitySingleStageNoReallocation", test_CircularBufferNRG_TestResetCapacitySingleStageNoReallocation },
    { "test_CircularBufferNRG_TestWrapsReads", test_CircularBufferNRG_TestWrapsReads },
    { "test_CircularBufferNRG_TestWrapsWrites", test_CircularBufferNRG_TestWrapsWrites },
    { NULL, NULL }
};
