/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "./Utility/utility.h"
#include "../Include/block_matrix.h"

#define LAGRAPH_FREE_ALL                                            \
{                                                                   \
  GrB_free(&ARow);                                                  \
  GrB_free(&BCol);                                                  \
  GrB_free(&pattern);                                               \
  GrB_free(&descriptor);                                            \
  LAGRAPH_FREE(Ap);                                                 \
  LAGRAPH_FREE(Aj);                                                 \
  LAGRAPH_FREE(Ax);                                                 \
  if(intermidates) {                                                \
    for(GrB_Index i = 1; i < nintermidates; i++) {                  \
      GrB_free(&intermidates[i]);                                   \
    }                                                               \
    LAGRAPH_FREE(intermidates);                                     \
  }                                                                 \
}

/* Performs dot product, multiples row i of A by column j of B
 * C = A[i,*] * B[*,j] */
GrB_Info BlockMatrix_DotProduct
(
	GrB_Matrix *C,                  // input/output matrix
	const GrB_Semiring semiring,    // defines '+' and '*' for A*B
	const BlockMatrix A,            // first input: block matrix A
	const BlockMatrix B,            // second input: block matrix B
	GrB_Index row,                  // row index of A
	GrB_Index col                   // column index of B
) {
	GrB_Info info;
	int64_t nonempty;
	GrB_Index nvals = 0;
	GrB_Index *Ap = NULL;
	GrB_Index *Aj = NULL;
	void      *Ax = NULL;
	GrB_Vector ARow = GrB_NULL;
	GrB_Vector BCol = GrB_NULL;
	GrB_Vector pattern = GrB_NULL;
	GrB_Index  nintermidates = 0;
	GrB_Matrix *intermidates = NULL;
	GrB_Descriptor descriptor = GrB_NULL;

	if(C == NULL || semiring == NULL || A == NULL || B == NULL) {
		// input arguments invalid
		LAGRAPH_ERROR("BlockMatrix_DotProduct: invalid inputs", GrB_NULL_POINTER);
	}

	*C = GrB_NULL;	// Default.

	// C = A[i:*] * B[*:j]

	// Perform DOT product between row I of A and column J of B
	// start by finding out the intersection between A[i:*] and B[*:j]
	// this will determine which elements are needed to be multiplied
	// assuming length of vectors is n, it might be that only even indecies
	// are set, as such we need to compute Ai0 * B0j + Ai2 * B2j + ...
	// This is further break down, in the first stage we compute the multiplications:
	// Aik * Bkj
	// second stage would sum up all of the intermidate multiplications:
	// AikBkj + AimBmj

	GrB_Index nrows;
	GrB_Index ncols;
	LAGRAPH_OK(BlockMatrix_BlocksPerRow(&nrows, A));
	LAGRAPH_OK(BlockMatrix_BlocksPerColumn(&ncols, B));

	if(nrows != ncols) {
		LAGRAPH_ERROR("BlockMatrix_DotProduct: Dimensions not compatible", GrB_DIMENSION_MISMATCH);
	}
	if(A->block_nrows != B->block_nrows || A->block_ncols != B->block_ncols) {
		LAGRAPH_ERROR("BlockMatrix_DotProduct: Dimensions not compatible", GrB_DIMENSION_MISMATCH);
	}
	if(row >= nrows || col >= ncols) {
		LAGRAPH_ERROR("BlockMatrix_DotProduct: out of range", GrB_INDEX_OUT_OF_BOUNDS);
	}

	GrB_Index block_nrows = A->block_nrows;
	GrB_Index block_ncols = B->block_ncols;

	// Compute intersection between Ai and Bj.
	LAGRAPH_OK(GrB_Descriptor_new(&descriptor));
	LAGRAPH_OK(GrB_Vector_new(&ARow, GrB_BOOL, nrows));
	LAGRAPH_OK(GrB_Vector_new(&BCol, GrB_BOOL, ncols));
	LAGRAPH_OK(GrB_Vector_new(&pattern, GrB_BOOL, ncols));
	LAGRAPH_OK(GrB_Descriptor_set(descriptor, GrB_INP0, GrB_TRAN));
	LAGRAPH_OK(GrB_Col_extract(ARow, GrB_NULL, GrB_NULL, A->s, GrB_ALL, ncols, row, descriptor));
	LAGRAPH_OK(GrB_Col_extract(BCol, GrB_NULL, GrB_NULL, B->s, GrB_ALL, nrows, col, GrB_NULL));
	LAGRAPH_OK(GrB_eWiseMult_Vector_Semiring(pattern, GrB_NULL, GrB_NULL, GxB_ANY_PAIR_BOOL, ARow, BCol,
											 GrB_NULL));

	// Extract indicies which are set in both vectors.
	LAGRAPH_OK(GrB_Vector_nvals(&nvals, pattern));

	if(nvals == 0) {
		// No intersecting entries, return an empty matrix.
		LAGRAPH_OK(GrB_Matrix_new(C, GrB_BOOL, block_nrows, block_ncols));
		LAGRAPH_FREE_ALL;
		return GrB_SUCCESS;
	}

	GrB_Index I[nvals];
	intermidates = calloc(sizeof(GrB_Matrix), nvals);  // Allocate intermidate matrices.
	if(intermidates == NULL) {
		LAGRAPH_ERROR("out of memory", GrB_OUT_OF_MEMORY) ;
	}

	LAGRAPH_OK(GrB_Vector_extractTuples_BOOL(I, GrB_NULL, &nvals, pattern));

	// First stage, perform multiplications.
	#pragma omp parallel
	{
		#pragma omp single
		{
			for(int i = 0; i < nvals; i++) {
				GrB_Index idx = I[i];
				GrB_Matrix Ai;
				GrB_Matrix Bi;
				// TODO: Handle failure.
				BlockMatrix_GetBlock(&Ai, A, row, idx);
				BlockMatrix_GetBlock(&Bi, B, idx, col);

				#pragma omp task
				{
					// TODO: handle error in this parallel context.
					GrB_Matrix_new(intermidates + i, GrB_BOOL, block_nrows, block_ncols);
					info = GrB_mxm(intermidates[i], GrB_NULL, GrB_NULL, semiring, Ai, Bi, GrB_NULL);
				}
			}
		}	// End of single
	}	// End of parallel region

	// Second stage, perform summation.
	nintermidates = nvals;
	while(nintermidates > 1) {
		int i = 0;
		#pragma omp parallel
		{
			#pragma omp single
			{
				for(; i < nintermidates / 2; i++) {
					#pragma omp task
					{
						int left = i;
						int right = nintermidates - i - 1;
						// TODO: Handle failure.
						info = GrB_eWiseAdd_Matrix_Semiring(intermidates[left], GrB_NULL, GrB_NULL, GxB_ANY_PAIR_BOOL, intermidates[left], intermidates[right], GrB_NULL);
					}
				}
			}	// End of single
		}	// End of parallel region
		nintermidates -= i;
	}

	*C = intermidates[0];
	LAGRAPH_FREE_ALL;
	return GrB_SUCCESS;
}
