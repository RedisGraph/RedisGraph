/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "gtest.h"

#include <ctime>
#include <cstdlib>

#ifdef __cplusplus
extern "C" {
#endif

#include "../../deps/GraphBLAS/Include/GraphBLAS.h"
#include "../../src/util/rmalloc.h"

#ifdef __cplusplus
}
#endif

#define _ISO true
#define _NOT_ISO false
bool _iso;

class TuplesTest: public ::testing::TestWithParam<std::tuple<int, bool>> {
  protected:
	void SetUp() override {
		// Seed the random number generator with the current time
		srand(time(0));

		// Use the malloc family for allocations
		Alloc_Reset();

		GrB_init(GrB_NONBLOCKING);
		GxB_Global_Option_set(GxB_FORMAT, GxB_BY_ROW); // all matrices in CSR format
		_iso = std::get<1>(GetParam());
	}

	void TearDown() override {
		GrB_finalize();
	}

	// creates a random n by m matrix
	GrB_Matrix CreateRandomMatrix(GrB_Index n, GrB_Index m) {
		GrB_Matrix A = CreateSquareNByNEmptyMatrix(n);

		for(GrB_Index i = 0; i < n; i++) {
			for(GrB_Index j = 0; j < m; j++) {
				int r = rand() % 100;
				if(r >= 50) {
					int v = rand() % 100;
					GrB_Matrix_setElement_UINT64(A, v, i, j);
				}
			}
		}

		return A;
	}

	GrB_Matrix CreateSquareNByNDiagonalMatrix(GrB_Index n) {
		GrB_Matrix A = CreateSquareNByNEmptyMatrix(n);

		uint64_t  *X = (uint64_t*)  malloc(sizeof(uint64_t)  * n);
		GrB_Index *I = (GrB_Index*) malloc(sizeof(GrB_Index) * n);
		GrB_Index *J = (GrB_Index*) malloc(sizeof(GrB_Index) * n);

		// initialize
		for(int i = 0; i < n; i++) {
			I[i] = i;
			J[i] = i;
			X[i] = _iso ? 1 : i;
		}

		GrB_Matrix_build_UINT64(A, I, J, X, n, GrB_FIRST_UINT64);
		GrB_Matrix_wait(A, GrB_MATERIALIZE);

		free(X);
		free(I);
		free(J);

		bool iso;
		GxB_Matrix_iso(&iso, A); // returns iso status of a matrix
		if(iso != _iso) throw;

		return A;
	}

	GrB_Matrix CreateSquareNByNEmptyMatrix(GrB_Index n) {
		GrB_Matrix A;
		GrB_Matrix_new(&A, GrB_UINT64, n, n);
		// matrix iterator requires matrix format to be sparse
		// to avoid future conversion from HYPER-SPARSE, BITMAP, FULL to SPARSE
		// we set matrix format at creation time
		GxB_Matrix_Option_set(A, GxB_SPARSITY_CONTROL, std::get<0>(GetParam()));
		GrB_Matrix_wait(A, GrB_MATERIALIZE);

		bool iso;
		GxB_Matrix_iso(&iso, A); // returns iso status of a matrix
		if(iso) throw;

		return A;
	}
};

TEST_P(TuplesTest, RandomVectorTest) {
	//--------------------------------------------------------------------------
	// Build a random vector
	//--------------------------------------------------------------------------

	GrB_Vector A;
	GrB_Index nvals = 0;
	GrB_Index nrows = 1024;
	GrB_Index *I = (GrB_Index *)malloc(sizeof(GrB_Index) * nrows);
	GrB_Info info;
	bool *X = (bool *)malloc(sizeof(bool) * nrows);

	GrB_Vector_new(&A, GrB_BOOL, nrows);

	// matrix iterator requires matrix format to be sparse
	// to avoid future conversion from HYPER-SPARSE, BITMAP, FULL to SPARSE
	// we set matrix format at creation time
	info = GxB_Vector_Option_set(A, GxB_SPARSITY_CONTROL, GxB_SPARSE);
	ASSERT_EQ(info, GrB_SUCCESS);

	double mid_point = RAND_MAX / 2;
	for(int i = 0; i < nrows; i++) {
		if(rand() > mid_point) {
			I[nvals] = i;
			X[nvals] = true;
			nvals++;
		}
	}

	GrB_Vector_build_BOOL(A, I, X, nvals, GrB_FIRST_BOOL);

	GrB_Index I_expected[nvals];
	GrB_Vector_extractTuples_BOOL(I_expected, NULL, &nvals, A);
	GrB_Vector_wait(A, GrB_MATERIALIZE);

	//--------------------------------------------------------------------------
	// Get an iterator over all nonzero elements.
	//--------------------------------------------------------------------------

	GxB_MatrixTupleIter *iter;
	GxB_MatrixTupleIter_new(&iter, (GrB_Matrix)A);
	GrB_Index col;

	//--------------------------------------------------------------------------
	// Verify iterator returned values.
	//--------------------------------------------------------------------------
	bool depleted = false;
	for(int i = 0; i < nvals; i++) {
		GxB_MatrixTupleIter_next(iter, NULL, &col, NULL, &depleted);
		ASSERT_FALSE(depleted);
		ASSERT_EQ(col, I_expected[i]);
	}
	GxB_MatrixTupleIter_next(iter, NULL, &col, NULL, &depleted);
	ASSERT_TRUE(depleted);

	//--------------------------------------------------------------------------
	// Clean up.
	//--------------------------------------------------------------------------
	free(I);
	free(X);
	GxB_MatrixTupleIter_free(&iter);
	GrB_Vector_free(&A);
}

TEST_P(TuplesTest, VectorIteratorTest) {
	//--------------------------------------------------------------------------
	// Build a vector
	//--------------------------------------------------------------------------

	GrB_Vector A;
	GrB_Vector_new(&A, GrB_BOOL, 4);

	// matrix iterator requires matrix format to be sparse
	// to avoid future conversion from HYPER-SPARSE, BITMAP, FULL to SPARSE
	// we set matrix format at creation time
	GrB_Info info;
	info = GxB_Vector_Option_set(A, GxB_SPARSITY_CONTROL, GxB_SPARSE);
	ASSERT_EQ(info, GrB_SUCCESS);

	GrB_Index nvals = 2;
	GrB_Index I[2]  = {1, 3};
	bool      X[2]  = {true, _iso ? true : false};

	bool      X_expected[nvals];
	GrB_Index I_expected[nvals];

	GrB_Vector_build_BOOL(A, I, X, nvals, GrB_FIRST_BOOL);
	GrB_Vector_extractTuples_BOOL(I_expected, X_expected, &nvals, A);

	//--------------------------------------------------------------------------
	// Get an iterator over all vector nonzero elements.
	//--------------------------------------------------------------------------

	bool                 val;
	GrB_Index            col;
	GxB_MatrixTupleIter  *iter;
	GxB_MatrixTupleIter_new(&iter, (GrB_Matrix)A);

	//--------------------------------------------------------------------------
	// Verify iterator returned values.
	//--------------------------------------------------------------------------
	bool depleted = false;
	for(int i = 0; i < nvals; i++) {
		GxB_MatrixTupleIter_next(iter, NULL, &col, &val, &depleted);
		ASSERT_FALSE(depleted);
		ASSERT_EQ(col, I_expected[i]);
		ASSERT_EQ(val, X_expected[i]);
	}
	GxB_MatrixTupleIter_next(iter, NULL, &col, &val, &depleted);
	ASSERT_TRUE(depleted);

	//--------------------------------------------------------------------------
	// Reset iterator and re-verify.
	//--------------------------------------------------------------------------

	GxB_MatrixTupleIter_reset(iter);
	for(int i = 0; i < nvals; i++) {
		GxB_MatrixTupleIter_next(iter, NULL, &col, &val, &depleted);
		ASSERT_FALSE(depleted);
		ASSERT_EQ(col, I_expected[i]);
		ASSERT_EQ(val, X_expected[i]);
	}
	GxB_MatrixTupleIter_next(iter, NULL, &col, &val, &depleted);
	ASSERT_TRUE(depleted);

	//--------------------------------------------------------------------------
	// Clean up.
	//--------------------------------------------------------------------------
	GxB_MatrixTupleIter_free(&iter);
	GrB_Vector_free(&A);
}

TEST_P(TuplesTest, RandomMatrixTest) {
	//--------------------------------------------------------------------------
	// Build a random matrix
	//--------------------------------------------------------------------------

	GrB_Matrix A;
	GrB_Index nvals = 0;
	GrB_Index nrows = 1024;
	GrB_Index ncols = 1024;

	uint64_t  *X = (uint64_t  *) malloc(sizeof(GrB_UINT64) * ncols * nrows);
	GrB_Index *I = (GrB_Index *) malloc(sizeof(GrB_Index)  * ncols * nrows);
	GrB_Index *J = (GrB_Index *) malloc(sizeof(GrB_Index)  * ncols * nrows);

	GrB_Matrix_new(&A, GrB_UINT64, nrows, ncols);

	// matrix iterator requires matrix format to be sparse
	// to avoid future conversion from HYPER-SPARSE, BITMAP, FULL to SPARSE
	// we set matrix format at creation time
	GrB_Info info;
	info = GxB_Matrix_Option_set(A, GxB_SPARSITY_CONTROL, std::get<0>(GetParam()));
	ASSERT_EQ(info, GrB_SUCCESS);

	double mid_point = RAND_MAX / 2;
	for(int i = 0; i < nrows; i++) {
		for(int j = 0; j < ncols; j++) {
			if(rand() > mid_point) {
				I[nvals] = i;
				J[nvals] = j;
				X[nvals] = rand();
				nvals++;
			}
		}
	}
	GrB_Matrix_build_UINT64(A, I, J, X, nvals, GrB_FIRST_UINT64);

	GrB_Index *I_expected = (GrB_Index *) malloc(sizeof(GrB_Index) * nvals);
	GrB_Index *J_expected = (GrB_Index *) malloc(sizeof(GrB_Index) * nvals);
	uint64_t  *X_expected = (uint64_t  *) malloc(sizeof(uint64_t)  * nvals);
	GrB_Matrix_extractTuples_UINT64(
			I_expected, J_expected, X_expected, &nvals, A);

	//--------------------------------------------------------------------------
	// Get an iterator over all matrix nonzero elements.
	//--------------------------------------------------------------------------

	uint64_t             val;
	GrB_Index            row;
	GrB_Index            col;
	GxB_MatrixTupleIter  *iter;
	GxB_MatrixTupleIter_new(&iter, A);

	//--------------------------------------------------------------------------
	// Verify iterator returned values.
	//--------------------------------------------------------------------------
	bool depleted = false;
	for(int i = 0; i < nvals; i++) {
		GxB_MatrixTupleIter_next(iter, &row, &col, &val, &depleted);
		ASSERT_FALSE(depleted);
		ASSERT_EQ(row, I_expected[i]);
		ASSERT_EQ(col, J_expected[i]);
		ASSERT_EQ(val, X_expected[i]);
	}
	GxB_MatrixTupleIter_next(iter, &row, &col, &val, &depleted);
	ASSERT_TRUE(depleted);

	//--------------------------------------------------------------------------
	// Clean up.
	//--------------------------------------------------------------------------
	free(I);
	free(J);
	free(X);
	free(I_expected);
	free(J_expected);
	free(X_expected);
	GxB_MatrixTupleIter_free(&iter);
	GrB_Matrix_free(&A);
}

TEST_P(TuplesTest, MatrixIteratorTest) {
	//--------------------------------------------------------------------------
	// Build a 4X4 matrix
	//--------------------------------------------------------------------------

	GrB_Index nvals = 4;
	GrB_Matrix A = CreateSquareNByNDiagonalMatrix(nvals);
	GrB_Index I_expected[nvals];
	GrB_Index J_expected[nvals];
	uint64_t  X_expected[nvals];
	GrB_Matrix_extractTuples_UINT64(
			I_expected, J_expected, X_expected, &nvals, A);

	//--------------------------------------------------------------------------
	// Get an iterator over all matrix nonzero elements.
	//--------------------------------------------------------------------------

	uint64_t             val;
	GrB_Index            row;
	GrB_Index            col;
	GxB_MatrixTupleIter  *iter;
	GxB_MatrixTupleIter_new(&iter, A);

	//--------------------------------------------------------------------------
	// Verify iterator returned values.
	//--------------------------------------------------------------------------

	bool depleted = false;
	for(int i = 0; i < nvals; i++) {
		GxB_MatrixTupleIter_next(iter, &row, &col, &val, &depleted);
		ASSERT_FALSE(depleted);
		ASSERT_EQ(row, I_expected[i]);
		ASSERT_EQ(col, J_expected[i]);
		ASSERT_EQ(val, X_expected[i]);
	}
	GxB_MatrixTupleIter_next(iter, &row, &col, &val, &depleted);
	ASSERT_TRUE(depleted);

	//--------------------------------------------------------------------------
	// Reset iterator an re-verify.
	//--------------------------------------------------------------------------

	GxB_MatrixTupleIter_reset(iter);
	for(int i = 0; i < nvals; i++) {
		GxB_MatrixTupleIter_next(iter, &row, &col, &val, &depleted);
		ASSERT_FALSE(depleted);
		ASSERT_EQ(row, I_expected[i]);
		ASSERT_EQ(col, J_expected[i]);
		ASSERT_EQ(val, X_expected[i]);
	}
	GxB_MatrixTupleIter_next(iter, &row, &col, &val, &depleted);
	ASSERT_TRUE(depleted);

	//--------------------------------------------------------------------------
	// Clean up.
	//--------------------------------------------------------------------------

	GxB_MatrixTupleIter_free(&iter);
	GrB_Matrix_free(&A);
}

TEST_P(TuplesTest, RowIteratorTest) {
	//--------------------------------------------------------------------------
	// Build a 4X4 matrix
	//--------------------------------------------------------------------------

	uint64_t v;
	GrB_Index row;
	GrB_Index col;
	GrB_Info info;
	bool depleted = false;
	GrB_Index nrows = 4;
	GrB_Matrix A = CreateSquareNByNDiagonalMatrix(nrows);

	GxB_MatrixTupleIter *iter;
	GxB_MatrixTupleIter_new(&iter, A);

	for(int i = 0; i < nrows; i++) {
		//----------------------------------------------------------------------
		// test iterating over each row twice
		//----------------------------------------------------------------------

		int reuse = 2;
		for(int j = 0; j < reuse; j++) {
			//------------------------------------------------------------------
			// get an iterator over the current column
			//------------------------------------------------------------------

			GxB_MatrixTupleIter_iterate_row(iter, i);

			//------------------------------------------------------------------
			// verify iterator returned values
			//------------------------------------------------------------------

			depleted = false;

			GxB_MatrixTupleIter_next(iter, &row, &col, NULL, &depleted);
			ASSERT_FALSE(depleted);
			ASSERT_EQ(row, i);
			ASSERT_EQ(col, i);
			
			GxB_MatrixTupleIter_next(iter, &row, &col, NULL, &depleted);
			ASSERT_TRUE(depleted);
		}
	}

	// clean up
	GxB_MatrixTupleIter_free(&iter);
	GrB_Matrix_free(&A);

	// test iterating both empty and none empty rows

	//--------------------------------------------------------------------------
	// Build a 4X4 matrix
	//--------------------------------------------------------------------------

	// A[0:] none empty
	// A[1:] empty
	// A[2:] none empty
	// A[3:] empty

	A = CreateSquareNByNEmptyMatrix(4);

	info = GrB_Matrix_setElement_UINT64(A, -1, 0, 0);
	ASSERT_EQ(info, GrB_SUCCESS);

	info = GrB_Matrix_setElement_UINT64(A, -2, 2, 1);
	ASSERT_EQ(info, GrB_SUCCESS);

	info = GrB_Matrix_setElement_UINT64(A, -3, 2, 3);
	ASSERT_EQ(info, GrB_SUCCESS);

	// attach iterator
	GxB_MatrixTupleIter_new(&iter, A);

	//--------------------------------------------------------------------------
	// iterate row 0
	//--------------------------------------------------------------------------

	GxB_MatrixTupleIter_iterate_row(iter, 0);

	depleted = false;
	GxB_MatrixTupleIter_next(iter, &row, &col, &v, &depleted);
	ASSERT_FALSE(depleted);
	ASSERT_EQ(row, 0);
	ASSERT_EQ(col, 0);
	ASSERT_EQ(v,  -1);

	GxB_MatrixTupleIter_next(iter, &row, &col, &v, &depleted);
	ASSERT_TRUE(depleted);

	//--------------------------------------------------------------------------
	// iterate row 1
	//--------------------------------------------------------------------------

	GxB_MatrixTupleIter_iterate_row(iter, 1);

	depleted = false;
	GxB_MatrixTupleIter_next(iter, &row, &col, &v, &depleted);
	ASSERT_TRUE(depleted);

	//--------------------------------------------------------------------------
	// iterate row 2
	//--------------------------------------------------------------------------

	GxB_MatrixTupleIter_iterate_row(iter, 2);

	depleted = false;
	GxB_MatrixTupleIter_next(iter, &row, &col, &v, &depleted);
	ASSERT_FALSE(depleted);
	ASSERT_EQ(row, 2);
	ASSERT_EQ(col, 1);
	ASSERT_EQ(v,  -2);

	GxB_MatrixTupleIter_next(iter, &row, &col, &v, &depleted);
	ASSERT_FALSE(depleted);
	ASSERT_EQ(row, 2);
	ASSERT_EQ(col, 3);
	ASSERT_EQ(v,  -3);

	GxB_MatrixTupleIter_next(iter, &row, &col, &v, &depleted);
	ASSERT_TRUE(depleted);

	//--------------------------------------------------------------------------
	// iterate row 3
	//--------------------------------------------------------------------------

	GxB_MatrixTupleIter_iterate_row(iter, 3);

	depleted = false;
	GxB_MatrixTupleIter_next(iter, &row, &col, &v, &depleted);
	ASSERT_TRUE(depleted);

	// clean up
	GxB_MatrixTupleIter_free(&iter);
	GrB_Matrix_free(&A);
}

TEST_P(TuplesTest, RowIteratorEmptyMatrixTest) {
	//--------------------------------------------------------------------------
	// Build a 4X4 empty matrix
	//--------------------------------------------------------------------------

	GrB_Index nrows = 4;
	GrB_Matrix A = CreateSquareNByNEmptyMatrix(nrows);

	GrB_Index row;
	GrB_Index col;
	GxB_MatrixTupleIter *iter;
	GxB_MatrixTupleIter_new(&iter, A);

	for(int i = 0; i < nrows; i++) {
		//----------------------------------------------------------------------
		// Get an iterator over the current row.
		//----------------------------------------------------------------------

		GxB_MatrixTupleIter_iterate_row(iter, i);

		//----------------------------------------------------------------------
		// Verify iterator returned values.
		//----------------------------------------------------------------------

		bool depleted = false;
		GxB_MatrixTupleIter_next(iter, &row, &col, NULL, &depleted);
		ASSERT_TRUE(depleted);
	}

	GxB_MatrixTupleIter_free(&iter);
	GrB_Matrix_free(&A);
}

TEST_P(TuplesTest, RowIteratorInvalidRows) {
	//--------------------------------------------------------------------------
	// Build a random 4X4 matrix
	//--------------------------------------------------------------------------

	GrB_Index nrows = 4;
	GrB_Matrix A = CreateRandomMatrix(nrows, nrows);

	GrB_Info info;
	GrB_Index row;
	GrB_Index col;
	GxB_MatrixTupleIter *iter;
	GxB_MatrixTupleIter_new(&iter, A);

	//----------------------------------------------------------------------
	// Get an iterator over a none existing row
	//----------------------------------------------------------------------

	info = GxB_MatrixTupleIter_iterate_row(iter, nrows);
	ASSERT_EQ(info, GrB_INVALID_INDEX);

	//----------------------------------------------------------------------
	// try to iterate over a none existing row
	//----------------------------------------------------------------------

	bool depleted = false;
	GxB_MatrixTupleIter_next(iter, &row, &col, NULL, &depleted);
	ASSERT_TRUE(depleted);

	// clean up
	GxB_MatrixTupleIter_free(&iter);
	GrB_Matrix_free(&A);
}

TEST_P(TuplesTest, IteratorRange) {
	// matrix is 6X6 and will be populated with the following indices
	GrB_Index indices[6][3] = {
		{0, 2, _iso ? (GrB_Index)7 : (GrB_Index)2},
		{2, 1, _iso ? (GrB_Index)7 : (GrB_Index)2},
		{2, 3, _iso ? (GrB_Index)7 : (GrB_Index)3},
		{3, 0, _iso ? (GrB_Index)7 : (GrB_Index)3},
		{3, 4, _iso ? (GrB_Index)7 : (GrB_Index)4},
		{5, 5, _iso ? (GrB_Index)7 : (GrB_Index)5}
	};

	GrB_Info   info;
	GrB_Index  row;
	GrB_Index  col;
	uint64_t   val;
	bool       depleted;

	// create and populate the matrix
	GrB_Index n = 6;
	GrB_Matrix A = CreateSquareNByNEmptyMatrix(n);
	for(int i = 0; i < 6; i ++) {
		row = indices[i][0];
		col = indices[i][1];
		val = indices[i][2];
		GrB_Matrix_setElement_UINT64(A, val, row, col);
	}

	bool iso;
	GxB_Matrix_iso(&iso, A); // returns iso status of a matrix
	ASSERT_EQ(iso, _iso);

	// create iterator
	GxB_MatrixTupleIter *iter;
	GxB_MatrixTupleIter_new(&iter, A);

	// check for invalid index exception for range iteration
	info = GxB_MatrixTupleIter_iterate_range(iter, -1, n - 1);
	ASSERT_EQ(GrB_INVALID_INDEX, info);
	info = GxB_MatrixTupleIter_iterate_range(iter, n - 1, 0);
	ASSERT_EQ(GrB_INVALID_INDEX, info);

	// check for invalid index exception on out-of-bounds iterator
	info = GxB_MatrixTupleIter_iterate_range(iter, n + 5, n + 5);
	ASSERT_EQ(GrB_INVALID_INDEX, info);

	// iterate single row
	info = GxB_MatrixTupleIter_iterate_range(iter, 2, 2);
	ASSERT_EQ(GrB_SUCCESS, info);

	// check that the right indices are retrived
	for(int i = 1; i <= 2; i++) {
		info = GxB_MatrixTupleIter_next(iter, &row, &col, &val, &depleted);
		ASSERT_EQ(GrB_SUCCESS, info);
		ASSERT_EQ(indices[i][0], row);
		ASSERT_EQ(indices[i][1], col);
		ASSERT_EQ(indices[i][2], val);
		ASSERT_FALSE(depleted);
	}
	GxB_MatrixTupleIter_next(iter, &row, &col, &val, &depleted);
	ASSERT_TRUE(depleted);

	// check for legal range setting
	info = GxB_MatrixTupleIter_iterate_range(iter, 2, 3);
	ASSERT_EQ(GrB_SUCCESS, info);

	// check that the right indices are retrived
	for(int i = 1; i <= 4; i++) {
		info = GxB_MatrixTupleIter_next(iter, &row, &col, &val, &depleted);
		ASSERT_EQ(GrB_SUCCESS, info);
		ASSERT_EQ(indices[i][0], row);
		ASSERT_EQ(indices[i][1], col);
		ASSERT_EQ(indices[i][2], val);
		ASSERT_FALSE(depleted);
	}
	GxB_MatrixTupleIter_next(iter, &row, &col, &val, &depleted);
	ASSERT_TRUE(depleted);

	// set the entire rows as range,
	// check that iterator is depleted only when it is done iterating the matrix
	info = GxB_MatrixTupleIter_iterate_range(iter, 0, n * 2) ;
	ASSERT_EQ(GrB_SUCCESS, info);

	for(int i = 0; i < 6; i ++) {
		info = GxB_MatrixTupleIter_next(iter, &row, &col, &val, &depleted);
		ASSERT_EQ(GrB_SUCCESS, info);
		ASSERT_EQ(indices[i][0], row);
		ASSERT_EQ(indices[i][1], col);
		ASSERT_EQ(indices[i][2], val);
		ASSERT_FALSE(depleted);
	}
	GxB_MatrixTupleIter_next(iter, &row, &col, &val, &depleted);
	ASSERT_TRUE(depleted);
}

INSTANTIATE_TEST_SUITE_P(TestParameters, TuplesTest,
		::testing::Combine(
			::testing::Values(GxB_SPARSE, GxB_HYPERSPARSE),
			::testing::Values(_ISO, _NOT_ISO)));

