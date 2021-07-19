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
#include "../../src/graph/rg_matrix/rg_matrix.h"
#include "../../src/graph/rg_matrix/rg_matrix_iter.h"

#ifdef __cplusplus
}
#endif


class RGMatrixTupleIterTest: public ::testing::Test {
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

// test RGMatrixTupleIter initialization
TEST_F(RGMatrixTupleIterTest, RGMatrixTupleiIter_new) {
	RG_Matrix          A                   =  NULL;
	GrB_Type           t                   =  GrB_UINT64;
	GrB_Info           info                =  GrB_SUCCESS;
    RG_MatrixTupleIter *iter               =  NULL;
	GrB_Index          nrows               =  100;
	GrB_Index          ncols               =  100;
	bool               multi_edge          =  true;
	bool               maintain_transpose  =  true;

	info = RG_Matrix_new(&A, t, nrows, ncols, multi_edge, maintain_transpose);
	ASSERT_EQ(info, GrB_SUCCESS);

    info = RG_MatrixTupleIter_new(&iter, A);
    ASSERT_TRUE(iter != NULL);

    RG_MatrixTupleIter_free(&iter);
    ASSERT_TRUE(iter == NULL);
	RG_Matrix_free(&A);
	ASSERT_TRUE(A == NULL);
}

// test RGMatrixTupleIter initialization
TEST_F(RGMatrixTupleIterTest, RGMatrixTupleiIter_next) {
	RG_Matrix          A                   =  NULL;
	GrB_Type           t                   =  GrB_BOOL;
	GrB_Info           info                =  GrB_SUCCESS;
    RG_MatrixTupleIter *iter               =  NULL;
    GrB_Index          i                   =  0;
	GrB_Index          j                   =  1;
    GrB_Index          row                 =  0;
	GrB_Index          col                 =  0;
	GrB_Index          nrows               =  100;
	GrB_Index          ncols               =  100;
    bool               sync                =  false;
    bool               depleted            =  false;
	bool               multi_edge          =  true;
	bool               maintain_transpose  =  true;

	info = RG_Matrix_new(&A, t, nrows, ncols, multi_edge, maintain_transpose);
	ASSERT_EQ(info, GrB_SUCCESS);

    // set element at position i,j
	info = RG_Matrix_setElement_BOOL(A, true, i, j);
	ASSERT_EQ(info, GrB_SUCCESS);

	//--------------------------------------------------------------------------
	// flush matrix, sync
	//--------------------------------------------------------------------------
	
	// wait, force sync
	sync = true;
	RG_Matrix_wait(A, sync);

    //--------------------------------------------------------------------------
	// set pending changes
	//--------------------------------------------------------------------------

	// remove element at position i,j
	info = RG_Matrix_removeElement(A, i, j);
	ASSERT_EQ(info, GrB_SUCCESS);

	// set element at position i+1,j+1
	info = RG_Matrix_setElement_BOOL(A, true, i+1, j+1);
	ASSERT_EQ(info, GrB_SUCCESS);

    info = RG_MatrixTupleIter_new(&iter, A);
    ASSERT_TRUE(iter != NULL);

    info = RG_MatrixTupleIter_next(iter, &row, &col, NULL, &depleted);
    ASSERT_TRUE(iter != NULL);
    
    ASSERT_EQ(row, i+1);
	ASSERT_EQ(col, j+1);

    info = RG_MatrixTupleIter_next(iter, &row, &col, NULL, &depleted);
    ASSERT_TRUE(iter != NULL);

    ASSERT_EQ(depleted, true);

    RG_MatrixTupleIter_free(&iter);
    ASSERT_TRUE(iter == NULL);
	RG_Matrix_free(&A);
	ASSERT_TRUE(A == NULL);
}