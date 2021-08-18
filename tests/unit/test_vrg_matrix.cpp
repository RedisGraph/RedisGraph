/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "gtest.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "../../src/util/rmalloc.h"
#include "../../src/graph/vrg_matrix/vrg_matrix.h"

#ifdef __cplusplus
}
#endif

class VRGMatrixTest: public ::testing::Test {
	protected:
	static void SetUpTestCase() {
		// use the malloc family for allocations
		Alloc_Reset();

		// initialize GraphBLAS
		GrB_init(GrB_NONBLOCKING);

		// all matrices in CSR format
		GxB_Global_Option_set(GxB_FORMAT, GxB_BY_ROW);
	}

	static void TearDownTestCase() {
		GrB_finalize();
	}
};

// test RGMatrix initialization
TEST_F(VRGMatrixTest, VRGMatrix) {
	RG_Matrix   M0       =  NULL;  // first version of M
	RG_Matrix   M1       =  NULL;  // second version of M
	VRG_Matrix  VM       =  NULL;  // versioned matrix
	RG_Matrix   A        =  NULL;  // retrieved version from VM

	// create both M0 and M1
	RG_Matrix_new(&M0, GrB_BOOL, 10, 10);
	RG_Matrix_new(&M1, GrB_BOOL, 10, 10);

	// create a new versioned matrix
	VM = VRG_Matrix_new(M0, 1);

	// get existing version
	A = VRG_Matrix_getVersion(VM, 1);
	ASSERT_EQ(A, M0);

	// get none existing version
	A = VRG_Matrix_getVersion(VM, 0);
	ASSERT_TRUE(A == NULL);

	// introduce a new version
	VRG_Matrix_addVersion(VM, M1, 2);
	A = VRG_Matrix_getVersion(VM, 2);
	ASSERT_EQ(A, M1);

	// delete existing version
	VRG_Matrix_delVersion(VM, 1);
	A = VRG_Matrix_getVersion(VM, 1);
	ASSERT_TRUE(A == NULL);

	// delete none existing version
	VRG_Matrix_delVersion(VM, 3);

	// get existing version
	A = VRG_Matrix_getVersion(VM, 2);
	ASSERT_EQ(A, M1);

	// ask for a version which is greater than latest
	A = VRG_Matrix_getVersion(VM, 6);
	ASSERT_EQ(A, M1);

	// clean up
	VRG_Matrix_free(&VM);
	ASSERT_TRUE(VM == NULL);
}

