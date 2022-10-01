/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "gtest.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "../../src/util/rmalloc.h"
#include "../../src/configuration/config.h"
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

		// set delta matrix flush threshold
		Config_Option_set(Config_DELTA_MAX_PENDING_CHANGES, "10000", NULL);
	}

	static void TearDownTestCase() {
		GrB_finalize();
	}
};

// test RGMatrixTupleIter initialization
TEST_F(RGMatrixTupleIterTest, RGMatrixTupleiIter_attach) {
	RG_Matrix          A                   =  NULL;
	GrB_Type           t                   =  GrB_UINT64;
	GrB_Info           info                =  GrB_SUCCESS;
	GrB_Index          nrows               =  100;
	GrB_Index          ncols               =  100;
	RG_MatrixTupleIter iter;
	memset(&iter, 0, sizeof(RG_MatrixTupleIter));

	info = RG_Matrix_new(&A, t, nrows, ncols);
	ASSERT_EQ(info, GrB_SUCCESS);

	info = RG_MatrixTupleIter_attach(&iter, A, RG_ITER_MIN_ROW, RG_ITER_MAX_ROW);
	ASSERT_TRUE(RG_MatrixTupleIter_is_attached(&iter, A));

	RG_MatrixTupleIter_detach(&iter);
	ASSERT_TRUE(iter.A == NULL);

	RG_Matrix_free(&A);
	ASSERT_TRUE(A == NULL);
}

// test RGMatrixTupleIter iteration
TEST_F(RGMatrixTupleIterTest, RGMatrixTupleiIter_next) {
	RG_Matrix          A                   =  NULL;
	GrB_Type           t                   =  GrB_UINT64;
	GrB_Info           info                =  GrB_SUCCESS;
	GrB_Index          i                   =  1;
	GrB_Index          j                   =  2;
	GrB_Index          row                 =  0;
	GrB_Index          col                 =  0;
	GrB_Index          nrows               =  100;
	GrB_Index          ncols               =  100;
	uint64_t           val                 =  0;
	bool               sync                =  false;
	RG_MatrixTupleIter iter;
	memset(&iter, 0, sizeof(RG_MatrixTupleIter));

	info = RG_Matrix_new(&A, t, nrows, ncols);
	ASSERT_EQ(info, GrB_SUCCESS);

	// set element at position i,j
	info = RG_Matrix_setElement_UINT64(A, 0, i, j);
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
	info = RG_Matrix_removeElement_UINT64(A, i, j);
	ASSERT_EQ(info, GrB_SUCCESS);

	// set element at position i+1,j+1
	info = RG_Matrix_setElement_UINT64(A, 1, i+1, j+1);
	ASSERT_EQ(info, GrB_SUCCESS);

	info = RG_MatrixTupleIter_attach(&iter, A, RG_ITER_MIN_ROW, RG_ITER_MAX_ROW);
	ASSERT_TRUE(RG_MatrixTupleIter_is_attached(&iter, A));

	info = RG_MatrixTupleIter_next_UINT64(&iter, &row, &col, &val);
	ASSERT_EQ(info, GrB_SUCCESS);
	
	ASSERT_EQ(row, i+1);
	ASSERT_EQ(col, j+1);
	ASSERT_EQ(val, 1);

	info = RG_MatrixTupleIter_next_UINT64(&iter, &row, &col, &val);

	ASSERT_EQ(info, GxB_EXHAUSTED);

	RG_Matrix_free(&A);
	ASSERT_TRUE(A == NULL);
	RG_MatrixTupleIter_detach(&iter);
	ASSERT_TRUE(iter.A == NULL);
}

// test RGMatrixTupleIter iteration for sparse matrix
TEST_F(RGMatrixTupleIterTest, RGMatrixTupleiIter_next_sparse) {
	RG_Matrix          A                   =  NULL;
	GrB_Type           t                   =  GrB_UINT64;
	GrB_Info           info                =  GrB_SUCCESS;
	GrB_Index          row                 =  0;
	GrB_Index          col                 =  0;
	GrB_Index          nrows               =  100;
	GrB_Index          ncols               =  100;
	uint64_t           val                 =  0;
	bool               sync                =  false;
	RG_MatrixTupleIter iter;
	memset(&iter, 0, sizeof(RG_MatrixTupleIter));

	info = RG_Matrix_new(&A, t, nrows, ncols);
	ASSERT_EQ(info, GrB_SUCCESS);

	for (GrB_Index i = 25; i < 100; i++) {
		for (GrB_Index j = 25; j < 100; j++) {
			// set element at position i,j
			info = RG_Matrix_setElement_UINT64(A, 0, i, j);
			ASSERT_EQ(info, GrB_SUCCESS);
		}
	}

	//--------------------------------------------------------------------------
	// flush matrix, sync
	//--------------------------------------------------------------------------
	
	// wait, force sync
	sync = true;
	RG_Matrix_wait(A, sync);

	//--------------------------------------------------------------------------
	// check M is sparse
	//--------------------------------------------------------------------------

	GrB_Matrix M = RG_MATRIX_M(A);
	
	int sparsity;
	GxB_Matrix_Option_get(M, GxB_SPARSITY_STATUS, &sparsity);
	ASSERT_TRUE(sparsity == GxB_SPARSE);

	//--------------------------------------------------------------------------
	// check iter start from correct row
	//--------------------------------------------------------------------------

	info = RG_MatrixTupleIter_attach(&iter, A, RG_ITER_MIN_ROW, RG_ITER_MAX_ROW);
	ASSERT_TRUE(RG_MatrixTupleIter_is_attached(&iter, A));

	info = RG_MatrixTupleIter_next_UINT64(&iter, &row, &col, &val);
	ASSERT_EQ(info, GrB_SUCCESS);
	
	ASSERT_EQ(row, 25);
	ASSERT_EQ(col, 25);
	ASSERT_EQ(val, 0);

	RG_Matrix_free(&A);
	ASSERT_TRUE(A == NULL);
	RG_MatrixTupleIter_detach(&iter);
	ASSERT_TRUE(iter.A == NULL);
}

// test RGMatrixTupleIter iteration
TEST_F(RGMatrixTupleIterTest, RGMatrixTupleiIter_reuse) {
	RG_Matrix          A                   =  NULL;
	RG_Matrix          B                   =  NULL;
	GrB_Type           t                   =  GrB_UINT64;
	GrB_Info           info                =  GrB_SUCCESS;
	GrB_Index          i                   =  1;
	GrB_Index          j                   =  2;
	GrB_Index          row                 =  0;
	GrB_Index          col                 =  0;
	GrB_Index          nrows               =  100;
	GrB_Index          ncols               =  100;
	uint64_t           val                 =  0;
	bool               sync                =  false;
	RG_MatrixTupleIter iter;
	memset(&iter, 0, sizeof(RG_MatrixTupleIter));

	info = RG_Matrix_new(&A, t, nrows, ncols);
	ASSERT_EQ(info, GrB_SUCCESS);

	info = RG_Matrix_new(&B, t, nrows, ncols);
	ASSERT_EQ(info, GrB_SUCCESS);

	// set element at position i,j
	info = RG_Matrix_setElement_UINT64(A, 0, i, j);
	ASSERT_EQ(info, GrB_SUCCESS);

	//--------------------------------------------------------------------------
	// flush matrix, sync
	//--------------------------------------------------------------------------
	
	// wait, force sync
	sync = true;
	RG_Matrix_wait(A, sync);

	info = RG_MatrixTupleIter_attach(&iter, B, RG_ITER_MIN_ROW, RG_ITER_MAX_ROW);
	ASSERT_TRUE(RG_MatrixTupleIter_is_attached(&iter, B));

	info = RG_MatrixTupleIter_attach(&iter, A, RG_ITER_MIN_ROW, RG_ITER_MAX_ROW);
	ASSERT_TRUE(RG_MatrixTupleIter_is_attached(&iter, A));

	info = RG_MatrixTupleIter_next_UINT64(&iter, &row, &col, &val);

	ASSERT_EQ(info, GrB_SUCCESS);
	ASSERT_EQ(row, i);
	ASSERT_EQ(col, j);
	ASSERT_EQ(val, 0);

	info = RG_MatrixTupleIter_next_UINT64(&iter, &row, &col, &val);

	ASSERT_EQ(info, GxB_EXHAUSTED);

	RG_Matrix_free(&A);
	ASSERT_TRUE(A == NULL);
	RG_Matrix_free(&B);
	ASSERT_TRUE(A == NULL);
	RG_MatrixTupleIter_detach(&iter);
	ASSERT_TRUE(iter.A == NULL);
}

// test RGMatrixTupleIter_iterate_row
TEST_F(RGMatrixTupleIterTest, RGMatrixTupleiIter_iterate_row) {
	RG_Matrix          A                   =  NULL;
	GrB_Type           t                   =  GrB_UINT64;
	GrB_Info           info                =  GrB_SUCCESS;
	GrB_Index          i                   =  1;
	GrB_Index          j                   =  2;
	GrB_Index          row                 =  0;
	GrB_Index          col                 =  0;
	GrB_Index          nrows               =  100;
	GrB_Index          ncols               =  100;
	uint64_t           val                 =  0;
	bool               sync                =  false;
	RG_MatrixTupleIter iter;
	memset(&iter, 0, sizeof(RG_MatrixTupleIter));

	info = RG_Matrix_new(&A, t, nrows, ncols);
	ASSERT_EQ(info, GrB_SUCCESS);

	// set element at position i,j
	info = RG_Matrix_setElement_UINT64(A, 1, i, j);
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
	info = RG_Matrix_removeElement_UINT64(A, i, j);
	ASSERT_EQ(info, GrB_SUCCESS);

	// wait, DM can't have pendding changes
	sync = false;
	RG_Matrix_wait(A, sync);

	// set element at position i+1,j+1
	info = RG_Matrix_setElement_UINT64(A, 2, i+1, j+1);
	ASSERT_EQ(info, GrB_SUCCESS);

	info = RG_MatrixTupleIter_attach(&iter, A, RG_ITER_MIN_ROW, RG_ITER_MAX_ROW);
	ASSERT_EQ(iter.A , A);

	info = RG_MatrixTupleIter_iterate_row(&iter, i);
	ASSERT_EQ(info, GrB_SUCCESS);

	info = RG_MatrixTupleIter_next_UINT64(&iter, &row, &col, &val);
	ASSERT_EQ(info, GxB_EXHAUSTED);

	info = RG_MatrixTupleIter_reset(&iter);
	ASSERT_EQ(info, GrB_SUCCESS);

	info = RG_MatrixTupleIter_iterate_row(&iter, i+1);
	ASSERT_EQ(info, GrB_SUCCESS);

	info = RG_MatrixTupleIter_next_UINT64(&iter, &row, &col, &val);

	ASSERT_EQ(info, GrB_SUCCESS);
	ASSERT_EQ(row, i+1);
	ASSERT_EQ(col, j+1);
	ASSERT_EQ(val, 2);

	info = RG_MatrixTupleIter_next_UINT64(&iter, &row, &col, &val);
	ASSERT_EQ(info, GxB_EXHAUSTED);

	RG_Matrix_free(&A);
	ASSERT_TRUE(A == NULL);
	RG_MatrixTupleIter_detach(&iter);
	ASSERT_TRUE(iter.A == NULL);
}

// test RGMatrixTupleiIter_jump_to_row
TEST_F(RGMatrixTupleIterTest, RGMatrixTupleiIter_jump_to_row) {
	RG_Matrix          A                   =  NULL;
	GrB_Type           t                   =  GrB_UINT64;
	GrB_Info           info                =  GrB_SUCCESS;
	GrB_Index          i                   =  1;
	GrB_Index          j                   =  2;
	GrB_Index          row                 =  0;
	GrB_Index          col                 =  0;
	GrB_Index          nrows               =  100;
	GrB_Index          ncols               =  100;
	uint64_t           val                 =  0;
	bool               sync                =  false;
	RG_MatrixTupleIter iter;
	memset(&iter, 0, sizeof(RG_MatrixTupleIter));

	info = RG_Matrix_new(&A, t, nrows, ncols);
	ASSERT_EQ(info, GrB_SUCCESS);

	// set element at position i,j
	info = RG_Matrix_setElement_UINT64(A, 0, i, j);
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
	info = RG_Matrix_removeElement_UINT64(A, i, j);
	ASSERT_EQ(info, GrB_SUCCESS);

	// set element at position i+1,j+1
	info = RG_Matrix_setElement_UINT64(A, 1, i+1, j+1);
	ASSERT_EQ(info, GrB_SUCCESS);

	info = RG_MatrixTupleIter_attach(&iter, A, RG_ITER_MIN_ROW, RG_ITER_MAX_ROW);
	ASSERT_TRUE(RG_MatrixTupleIter_is_attached(&iter, A));

	info = RG_MatrixTupleIter_jump_to_row(&iter, i+1);
	ASSERT_EQ(info, GrB_SUCCESS);

	info = RG_MatrixTupleIter_next_UINT64(&iter, &row, &col, &val);

	ASSERT_EQ(info, GrB_SUCCESS);
	ASSERT_EQ(row, i+1);
	ASSERT_EQ(col, j+1);
	ASSERT_EQ(val, 1);

	info = RG_MatrixTupleIter_next_UINT64(&iter, &row, &col, &val);
	ASSERT_EQ(info, GxB_EXHAUSTED);

	RG_Matrix_free(&A);
	ASSERT_TRUE(A == NULL);
	RG_MatrixTupleIter_detach(&iter);
	ASSERT_TRUE(iter.A == NULL);
}

// test RGMatrixTupleiIter_iterate_range
TEST_F(RGMatrixTupleIterTest, RGMatrixTupleiIter_iterate_range) {
	RG_Matrix          A                   =  NULL;
	GrB_Type           t                   =  GrB_UINT64;
	GrB_Info           info                =  GrB_SUCCESS;
	GrB_Index          i                   =  1;
	GrB_Index          j                   =  2;
	GrB_Index          row                 =  0;
	GrB_Index          col                 =  0;
	GrB_Index          nrows               =  100;
	GrB_Index          ncols               =  100;
	uint64_t           val                 =  0;
	bool               sync                =  false;
	RG_MatrixTupleIter iter;
	memset(&iter, 0, sizeof(RG_MatrixTupleIter));

	info = RG_Matrix_new(&A, t, nrows, ncols);
	ASSERT_EQ(info, GrB_SUCCESS);

	// set element at position i,j
	info = RG_Matrix_setElement_UINT64(A, 0, i, j);
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
	info = RG_Matrix_removeElement_UINT64(A, i, j);
	ASSERT_EQ(info, GrB_SUCCESS);

	// set element at position i+1,j+1
	info = RG_Matrix_setElement_UINT64(A, 1, i+1, j+1);
	ASSERT_EQ(info, GrB_SUCCESS);

	info = RG_MatrixTupleIter_attach(&iter, A, RG_ITER_MIN_ROW, RG_ITER_MAX_ROW);
	ASSERT_TRUE(RG_MatrixTupleIter_is_attached(&iter, A));

	info = RG_MatrixTupleIter_iterate_range(&iter, i+1, i+1);
	ASSERT_EQ(info, GrB_SUCCESS);

	info = RG_MatrixTupleIter_next_UINT64(&iter, &row, &col, &val);

	ASSERT_EQ(info, GrB_SUCCESS);
	ASSERT_EQ(row, i+1);
	ASSERT_EQ(col, j+1);
	ASSERT_EQ(val, 1);

	info = RG_MatrixTupleIter_next_UINT64(&iter, &row, &col, &val);
	ASSERT_EQ(info, GxB_EXHAUSTED);

	RG_Matrix_free(&A);
	ASSERT_TRUE(A == NULL);
	RG_MatrixTupleIter_detach(&iter);
	ASSERT_TRUE(iter.A == NULL);
}
