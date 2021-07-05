/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "RG.h"
#include "rg_matrix.h"

GrB_Info RG_mxm                     // C = A * B
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Semiring semiring,    // defines '+' and '*' for A*B
    const GrB_Matrix A,             // first input:  matrix A
    const RG_Matrix B               // second input: matrix B
) {
	ASSERT(C != NULL);
	ASSERT(A != NULL);
	ASSERT(B != NULL);

	// multiply GrB_Matrix by RG_Matrix
	// A * B
	// where B is represented by:
	// 1. a primary matrix 'M'
	// 2. addition matrix 'delta-plus'
	// 3. deletion matrix 'delta-minus'
	//
	// it is possible for either 'delta-plus' or 'delta-minus' to be empty
	// this operation performs: A * B by computing:
	// (A * (M + 'delta-plus'))<!'delta-minus'>

	GrB_Info info;
	GrB_Index nrows;     // number of rows in result matrix
	GrB_Index ncols;     // number of columns in result matrix 
	GrB_Index dp_nvals;  // number of entries in A * 'dp'
	GrB_Index dm_nvals;  // number of entries in A * 'dm'

	GrB_Matrix  M      =  RG_MATRIX_MATRIX(B);
	GrB_Matrix  dp     =  RG_MATRIX_DELTA_PLUS(B);
	GrB_Matrix  dm     =  RG_MATRIX_DELTA_MINUS(B);
	GrB_Matrix  mask   =  NULL;  // entities removed
	GrB_Matrix  accum  =  NULL;  // entities added

	GrB_Matrix_nrows(&nrows, C);
	GrB_Matrix_ncols(&ncols, C);
	GrB_Matrix_nvals(&dp_nvals, dp);
	GrB_Matrix_nvals(&dm_nvals, dm);

	if(dm_nvals > 0) {
		// compute A * 'delta-minus'
		info = GrB_Matrix_new(&mask, GrB_BOOL, nrows, ncols);
		ASSERT(info == GrB_SUCCESS);

		info = GrB_mxm(mask, NULL, NULL, GxB_ANY_PAIR_BOOL, A, dm, NULL);
		ASSERT(info == GrB_SUCCESS);

		info = GrB_Matrix_nvals(&dm_nvals, mask);
		ASSERT(info == GrB_SUCCESS);
	}

	if(dp_nvals > 0) {
		// compute A * 'delta-plus'
		info = GrB_Matrix_new(&accum, GrB_BOOL, nrows, ncols);
		ASSERT(info == GrB_SUCCESS);

		info = GrB_mxm(accum, NULL, NULL, semiring, A, dp, NULL);
		ASSERT(info == GrB_SUCCESS);

		info = GrB_Matrix_nvals(&dp_nvals, accum);
		ASSERT(info == GrB_SUCCESS);
	}

	GrB_Descriptor  desc       =  NULL;
	bool            additions  =  dp_nvals  >  0;
	bool            deletions  =  dm_nvals  >  0;

	if (deletions) {
		desc = GrB_DESC_RSC;
	} else {
		GrB_free(&mask);
		mask = NULL;
	}

	// compute (A * M)<!mask>
	info = GrB_mxm(C, mask, NULL, semiring, A, M, desc);
	ASSERT(info == GrB_SUCCESS);

	if(additions) {
		info = GrB_eWiseAdd(C, NULL, NULL, GxB_ANY_PAIR_BOOL, C, accum, NULL);
		ASSERT(info == GrB_SUCCESS);
	}

	// clean up
	if(mask)  GrB_free(&mask);
	if(accum) GrB_free(&accum);

	return info;
}

