#include "gtest.h"

#ifdef __cplusplus
extern "C" {
#endif
#include "../../src/util/rmalloc.h"
#include "../../src/util/arr.h"
#ifdef __cplusplus
}
#endif

class ArrTest: public ::testing::Test {
  protected:
	static void SetUpTestCase() {// Use the malloc family for allocations
		Alloc_Reset();
	}

	inline int int_identity(int x) {
		return x;
	}

};

TEST_F(ArrTest, TestArrCloneWithCB) {
	int *arr = array_new(int, 10);
	for(int i = 0; i < 10; i++) arr = array_append(arr, i);
	int *arr_clone;
	array_clone_with_cb(arr_clone, arr, int_identity);
	ASSERT_EQ(array_len(arr), array_len(arr_clone));
	for(int i = 0; i < 10; i++) ASSERT_EQ(arr[i], arr_clone[i]);
	array_free(arr);
	array_free(arr_clone);
}

