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

#ifdef __cplusplus
}
#endif

class RGMatrixTest: public ::testing::Test {
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
TEST_F(RGMatrixTest, RGMatrix_new) {
	RG_Matrix   A                   =  NULL;
	GrB_Matrix  M                   =  NULL;
	GrB_Matrix  DP                  =  NULL;
	GrB_Matrix  DM                  =  NULL;
	GrB_Type    t                   =  GrB_UINT64;
	GrB_Info    info                =  GrB_SUCCESS;
	GrB_Index   nvals               =  0;
	GrB_Index   nrows               =  100;
	GrB_Index   ncols               =  100;
	int         scontrol            =  GxB_ANY_SPARSITY;
	bool        multi_edge          =  true;
	bool        maintain_transpose  =  true;

	info = RG_Matrix_new(&A, t, nrows, ncols, multi_edge, maintain_transpose);
	ASSERT_EQ(info, GrB_SUCCESS);

	// get internal matrices
	M   =  RG_MATRIX_MATRIX(A);
	DP  =  RG_MATRIX_DELTA_PLUS(A);
	DM  =  RG_MATRIX_DELTA_MINUS(A);

	// verify sparsity control
	GxB_Matrix_Option_get(M, GxB_SPARSITY_CONTROL, &scontrol);
	ASSERT_EQ(scontrol, GxB_SPARSE);
	GxB_Matrix_Option_get(DP, GxB_SPARSITY_CONTROL, &scontrol);
	ASSERT_EQ(scontrol, GxB_HYPERSPARSE);
	GxB_Matrix_Option_get(DM, GxB_SPARSITY_CONTROL, &scontrol);
	ASSERT_EQ(scontrol, GxB_HYPERSPARSE);

	// matrix isn't dirty
	ASSERT_FALSE(RG_Matrix_isDirty(A));

	// matrix multi-edge set accordingly
	ASSERT_EQ(RG_Matrix_getMultiEdge(A), multi_edge);

	// matrix maintain transpose set accordingly
	ASSERT_EQ(A->maintain_transpose, maintain_transpose);

	// matrix should be empty
	RG_Matrix_nvals(&nvals, A);
	ASSERT_EQ(nvals, 0);

	RG_Matrix_free(&A);
	ASSERT_TRUE(A == NULL);
}

// setting an empty entry
TEST_F(RGMatrixTest, RGMatrix_simple_set) {
	GrB_Type    t                   =  GrB_BOOL;
	RG_Matrix   A                   =  NULL;
	GrB_Matrix  M                   =  NULL;
	GrB_Matrix  DP                  =  NULL;
	GrB_Matrix  DM                  =  NULL;
	GrB_Info    info                =  GrB_SUCCESS;
	GrB_Index   nvals               =  0;
	GrB_Index   nrows               =  100;
	GrB_Index   ncols               =  100;
	GrB_Index   i                   =  0;
	GrB_Index   j                   =  1;
	bool        x                   =  false;
	bool        multi_edge          =  false;
	bool        maintain_transpose  =  false;

	info = RG_Matrix_new(&A, t, nrows, ncols, multi_edge, maintain_transpose);
	ASSERT_EQ(info, GrB_SUCCESS);

	// make sure element at position i,j doesn't exists
	info = RG_Matrix_extractElement_BOOL(&x, A, i, j);
	ASSERT_EQ(info, GrB_NO_VALUE);

	//--------------------------------------------------------------------------
	// set element at position i,j
	//--------------------------------------------------------------------------

	info = RG_Matrix_setElement_BOOL(A, true, i, j);
	ASSERT_EQ(info, GrB_SUCCESS);

	// make sure element at position i,j exists
	info = RG_Matrix_extractElement_BOOL(&x, A, i, j);
	ASSERT_EQ(info, GrB_SUCCESS);
	ASSERT_EQ(x, true);
	
	// matrix should contain a single element
	RG_Matrix_nvals(&nvals, A);
	ASSERT_EQ(nvals, 1);

	// matrix should be mark as dirty
	ASSERT_TRUE(RG_Matrix_isDirty(A));

	// get internal matrices
	M   =  RG_MATRIX_MATRIX(A);
	DP  =  RG_MATRIX_DELTA_PLUS(A);
	DM  =  RG_MATRIX_DELTA_MINUS(A);

	//--------------------------------------------------------------------------
	// validations
	//--------------------------------------------------------------------------

	// M should be empty
	GrB_Matrix_nvals(&nvals, M);
	ASSERT_EQ(nvals, 0);

	// DM should be empty
	GrB_Matrix_nvals(&nvals, DM);
	ASSERT_EQ(nvals, 0);

	// DP should contain a single element
	GrB_Matrix_nvals(&nvals, DP);
	ASSERT_EQ(nvals, 1);

	//--------------------------------------------------------------------------
	// set already set entry
	//--------------------------------------------------------------------------

	// flush matrix
	RG_Matrix_wait(A, false);

	// introduce existing entry
	info = RG_Matrix_setElement_BOOL(A, true, i, j);
	ASSERT_EQ(info, GrB_SUCCESS);

	// expecting no change
	ASSERT_FALSE(RG_Matrix_isDirty(A));

	//--------------------------------------------------------------------------
	// test transposed matrix
	//--------------------------------------------------------------------------

	//RG_Matrix T = RG_Matrix_getTranspose(A);
	//ASSERT_TRUE(T != NULL);

	//// make sure element at position j,i exists
	//info = RG_Matrix_extractElement_BOOL(&x, A, j, i);
	//ASSERT_EQ(info, GrB_SUCCESS);
	//ASSERT_EQ(x, true);

	//// matrix should contain a single element
	//RG_Matrix_nvals(&nvals, T);
	//ASSERT_EQ(nvals, 1);

	//// get internal matrices
	//M   =  RG_MATRIX_MATRIX(T);
	//DP  =  RG_MATRIX_DELTA_PLUS(T);
	//DM  =  RG_MATRIX_DELTA_MINUS(T);

	//// M should be empty
	//GrB_Matrix_nvals(&nvals, M);
	//ASSERT_EQ(nvals, 0);

	//// DM should be empty
	//GrB_Matrix_nvals(&nvals, DM);
	//ASSERT_EQ(nvals, 0);

	//// DP should contain a single element
	//GrB_Matrix_nvals(&nvals, DP);
	//ASSERT_EQ(nvals, 1);

	// clean up
	RG_Matrix_free(&A);
	ASSERT_TRUE(A == NULL);
}

// remove single entry
TEST_F(RGMatrixTest, RGMatrix_simple_del) {
	GrB_Type    t                   =  GrB_BOOL;
	RG_Matrix   A                   =  NULL;
	GrB_Matrix  M                   =  NULL;
	GrB_Matrix  DP                  =  NULL;
	GrB_Matrix  DM                  =  NULL;
	GrB_Info    info                =  GrB_SUCCESS;
	GrB_Index   nvals               =  0;
	GrB_Index   nrows               =  100;
	GrB_Index   ncols               =  100;
	GrB_Index   i                   =  0;
	GrB_Index   j                   =  1;
	bool        multi_edge          =  false;
	bool        maintain_transpose  =  false;

	info = RG_Matrix_new(&A, t, nrows, ncols, multi_edge, maintain_transpose);
	ASSERT_EQ(info, GrB_SUCCESS);

	// get internal matrices
	M   =  RG_MATRIX_MATRIX(A);
	DP  =  RG_MATRIX_DELTA_PLUS(A);
	DM  =  RG_MATRIX_DELTA_MINUS(A);

	//--------------------------------------------------------------------------
	// remove none existing entry
	//--------------------------------------------------------------------------

	info = RG_Matrix_removeElement(A, i, j);
	ASSERT_EQ(info, GrB_SUCCESS);

	//--------------------------------------------------------------------------
	// remove none flushed addition
	//--------------------------------------------------------------------------

	// set element at position i,j
	info = RG_Matrix_setElement_BOOL(A, true, i, j);
	ASSERT_EQ(info, GrB_SUCCESS);

	// remove element at position i,j
	info = RG_Matrix_removeElement(A, i, j);
	ASSERT_EQ(info, GrB_SUCCESS);

	// matrix should be mark as dirty
	ASSERT_TRUE(RG_Matrix_isDirty(A));

	//--------------------------------------------------------------------------
	// validations
	//--------------------------------------------------------------------------

	// A should be empty
	RG_Matrix_nvals(&nvals, A);
	ASSERT_EQ(nvals, 0);

	// M should be empty
	GrB_Matrix_nvals(&nvals, M);
	ASSERT_EQ(nvals, 0);

	// DM should be empty
	GrB_Matrix_nvals(&nvals, DM);
	ASSERT_EQ(nvals, 0);

	// DP should be empty
	GrB_Matrix_nvals(&nvals, DP);
	ASSERT_EQ(nvals, 0);

	//--------------------------------------------------------------------------
	// remove flushed addition
	//--------------------------------------------------------------------------

	// set element at position i,j
	info = RG_Matrix_setElement_BOOL(A, true, i, j);
	ASSERT_EQ(info, GrB_SUCCESS);

	// force sync
	info = RG_Matrix_wait(A, true);

	// entry should migrated from 'delta-plus' to 'M'
	// remove element at position i,j
	info = RG_Matrix_removeElement(A, i, j);
	ASSERT_EQ(info, GrB_SUCCESS);

	//--------------------------------------------------------------------------
	// validations
	//--------------------------------------------------------------------------

	// A should be empty
	RG_Matrix_nvals(&nvals, A);
	ASSERT_EQ(nvals, 0);

	// M should contain a single element
	GrB_Matrix_nvals(&nvals, M);
	ASSERT_EQ(nvals, 1);

	// DM should contain a single element
	GrB_Matrix_nvals(&nvals, DM);
	ASSERT_EQ(nvals, 1);

	// DP should be empty
	GrB_Matrix_nvals(&nvals, DP);
	ASSERT_EQ(nvals, 0);

	//--------------------------------------------------------------------------
	// flush deletion
	//--------------------------------------------------------------------------

	// entry should be removed from both 'delta-minus' and 'M'
	info = RG_Matrix_wait(A, true);
	ASSERT_EQ(info, GrB_SUCCESS);

	//--------------------------------------------------------------------------
	// validations
	//--------------------------------------------------------------------------

	// A should be empty
	RG_Matrix_nvals(&nvals, A);
	ASSERT_EQ(nvals, 0);

	// M should be empty
	GrB_Matrix_nvals(&nvals, M);
	ASSERT_EQ(nvals, 0);

	// DM should be empty
	GrB_Matrix_nvals(&nvals, DM);
	ASSERT_EQ(nvals, 0);

	// DP should be empty
	GrB_Matrix_nvals(&nvals, DP);
	ASSERT_EQ(nvals, 0);

	//--------------------------------------------------------------------------
	// clean up
	//--------------------------------------------------------------------------

	RG_Matrix_free(&A);
	ASSERT_TRUE(A == NULL);
}

// flush simple addition
TEST_F(RGMatrixTest, RGMatrix_flush) {
	GrB_Type    t                   =  GrB_BOOL;
	RG_Matrix   A                   =  NULL;
	GrB_Matrix  M                   =  NULL;
	GrB_Matrix  DP                  =  NULL;
	GrB_Matrix  DM                  =  NULL;
	GrB_Info    info                =  GrB_SUCCESS;
	GrB_Index   nvals               =  0;
	GrB_Index   nrows               =  100;
	GrB_Index   ncols               =  100;
	GrB_Index   i                   =  0;
	GrB_Index   j                   =  1;
	bool        sync                =  false;
	bool        multi_edge          =  false;
	bool        maintain_transpose  =  false;

	info = RG_Matrix_new(&A, t, nrows, ncols, multi_edge, maintain_transpose);
	ASSERT_EQ(info, GrB_SUCCESS);

	// set element at position i,j
	info = RG_Matrix_setElement_BOOL(A, true, i, j);
	ASSERT_EQ(info, GrB_SUCCESS);

	// get internal matrices
	M   =  RG_MATRIX_MATRIX(A);
	DP  =  RG_MATRIX_DELTA_PLUS(A);
	DM  =  RG_MATRIX_DELTA_MINUS(A);

	//--------------------------------------------------------------------------
	// flush matrix, no sync
	//--------------------------------------------------------------------------
	
	// wait, don't force sync
	sync = false;
	RG_Matrix_wait(A, sync);

	// M should be empty
	GrB_Matrix_nvals(&nvals, M);
	ASSERT_EQ(nvals, 0);

	// DM should be empty
	GrB_Matrix_nvals(&nvals, DM);
	ASSERT_EQ(nvals, 0);

	// DP should contain a single element
	GrB_Matrix_nvals(&nvals, DP);
	ASSERT_EQ(nvals, 1);

	//--------------------------------------------------------------------------
	// flush matrix, sync
	//--------------------------------------------------------------------------
	
	// wait, force sync
	sync = true;
	RG_Matrix_wait(A, sync);

	RG_Matrix_nvals(&nvals, A);
	ASSERT_EQ(nvals, 1);

	// M should be empty
	GrB_Matrix_nvals(&nvals, M);
	ASSERT_EQ(nvals, 1);

	// DM should be empty
	GrB_Matrix_nvals(&nvals, DM);
	ASSERT_EQ(nvals, 0);

	// DP should contain a single element
	GrB_Matrix_nvals(&nvals, DP);
	ASSERT_EQ(nvals, 0);

	// clean up
	RG_Matrix_free(&A);
	ASSERT_TRUE(A == NULL);
}

//------------------------------------------------------------------------------
// transpose test
//------------------------------------------------------------------------------

// M[i,j] = x, M[i,j] = y
TEST_F(RGMatrixTest, RGMatrix_managed_transposed) {
	GrB_Type    t                   =  GrB_UINT64;
	RG_Matrix   A                   =  NULL;
	RG_Matrix   T                   =  NULL;  // A transposed
	GrB_Matrix  TM                  =  NULL;  // primary internal matrix
	GrB_Matrix  TDP                 =  NULL;  // delta plus
	GrB_Matrix  TDM                 =  NULL;  // delta minus
	GrB_Info    info                =  GrB_SUCCESS;
	GrB_Index   nvals               =  0;
	GrB_Index   nrows               =  100;
	GrB_Index   ncols               =  100;
	GrB_Index   i                   =  0;
	GrB_Index   j                   =  1;
	uint64_t    x                   =  0;  // M[i,j] = x
	uint64_t    v                   =  0;  // v = M[i,j]
	bool        multi_edge          =  false;
	bool        maintain_transpose  =  true;

	//--------------------------------------------------------------------------
	// create RGMatrix
	//--------------------------------------------------------------------------

	info = RG_Matrix_new(&A, t, nrows, ncols, multi_edge, maintain_transpose);
	ASSERT_EQ(info, GrB_SUCCESS);

	// make sure transposed was created
	T = RG_Matrix_getTranspose(A);
	ASSERT_TRUE(T != A);
	ASSERT_TRUE(T != NULL);

	// get internal matrices
	TM   =  RG_MATRIX_MATRIX(T);
	TDP  =  RG_MATRIX_DELTA_PLUS(T);
	TDM  =  RG_MATRIX_DELTA_MINUS(T);

	//--------------------------------------------------------------------------
	// set element at position i,j
	//--------------------------------------------------------------------------

	info = RG_Matrix_setElement_UINT64(A, x, i, j);
	ASSERT_EQ(info, GrB_SUCCESS);

	// make sure element at position j,i exists
	info = RG_Matrix_extractElement_UINT64(&v, T, j, i);
	ASSERT_EQ(info, GrB_SUCCESS);
	ASSERT_EQ(x, v);
	
	// matrix should contain a single element
	RG_Matrix_nvals(&nvals, T);
	ASSERT_EQ(nvals, 1);

	// matrix should be mark as dirty
	ASSERT_TRUE(RG_Matrix_isDirty(T));

	//--------------------------------------------------------------------------
	// validations
	//--------------------------------------------------------------------------

	// TM should be empty
	GrB_Matrix_nvals(&nvals, TM);
	ASSERT_EQ(nvals, 0);

	// TDM should be empty
	GrB_Matrix_nvals(&nvals, TDM);
	ASSERT_EQ(nvals, 0);

	// TDP should contain a single element
	GrB_Matrix_nvals(&nvals, TDP);
	ASSERT_EQ(nvals, 1);

	//--------------------------------------------------------------------------
	// flush matrix
	//--------------------------------------------------------------------------

	RG_Matrix_wait(A, true);

	// flushing 'A' should flush 'T' aswell

	// TM should contain a single element
	GrB_Matrix_nvals(&nvals, TM);
	ASSERT_EQ(nvals, 1);

	// TDM should be empty
	GrB_Matrix_nvals(&nvals, TDM);
	ASSERT_EQ(nvals, 0);

	// TDP should be empty
	GrB_Matrix_nvals(&nvals, TDP);
	ASSERT_EQ(nvals, 0);

	//--------------------------------------------------------------------------
	// delete element at position i,j
	//--------------------------------------------------------------------------
	
	info = RG_Matrix_removeElement(A, i, j);
	ASSERT_EQ(info, GrB_SUCCESS);

	// matrix should be mark as dirty
	ASSERT_TRUE(RG_Matrix_isDirty(T));

	//--------------------------------------------------------------------------
	// validations
	//--------------------------------------------------------------------------

	// TM should contain a single element
	GrB_Matrix_nvals(&nvals, TM);
	ASSERT_EQ(nvals, 1);

	// TDM should contain a single element
	GrB_Matrix_nvals(&nvals, TDM);
	ASSERT_EQ(nvals, 1);

	// TDP should be empty
	GrB_Matrix_nvals(&nvals, TDP);
	ASSERT_EQ(nvals, 0);

	//--------------------------------------------------------------------------
	// flush matrix
	//--------------------------------------------------------------------------

	// flushing 'A' should flush 'T' aswell

	RG_Matrix_wait(A, true);

	// TM should be empty
	GrB_Matrix_nvals(&nvals, TM);
	ASSERT_EQ(nvals, 0);

	// TDM should be empty
	GrB_Matrix_nvals(&nvals, TDM);
	ASSERT_EQ(nvals, 0);

	// TDP should be empty
	GrB_Matrix_nvals(&nvals, TDP);
	ASSERT_EQ(nvals, 0);

	// clean up
	RG_Matrix_free(&A);
	ASSERT_TRUE(A == NULL);
}

