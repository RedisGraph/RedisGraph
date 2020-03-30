/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "./Utility/utility.h"
#include "../Include/block_matrix.h"
#include "BM_dot_product.h"

#define LAGRAPH_FREE_ALL                        \
{                                               \
	if(blocks) {                                \
		for(GrB_Index i = 0; i < nnz; i++) {    \
			GrB_free(&blocks[i]);               \
		}                                       \
		free(blocks);                           \
	}                                           \
}

GrB_Info BlockMatrix_mxm            // C<Mask> = accum (C, A*B)
(
	BlockMatrix C,                  // input/output matrix for results
	const GrB_Matrix Mask,          // optional mask for C, unused if NULL
	const GrB_BinaryOp accum,       // optional accum for Z=accum(C,T)
	const GrB_Semiring semiring,    // defines '+' and '*' for A*B
	const BlockMatrix A,            // first input:  matrix A
	const BlockMatrix B,            // second input: matrix B
	const GrB_Descriptor desc       // descriptor for C, Mask, A, and B
) {
	GrB_Info info;
	GrB_Index anrows;
	GrB_Index ancols;
	GrB_Index bnrows;
	GrB_Index bncols;
	GrB_Index cnrows;
	GrB_Index cncols;
	GrB_Index nnz = 0;
	GrB_Matrix *blocks = NULL;
	GrB_Matrix pattern = GrB_NULL;

	if(C == NULL || A == NULL || B == NULL) {
		// input arguments invalid
		LAGRAPH_ERROR("BlockMatrix_mxm: invalid inputs", GrB_NULL_POINTER);
	}

	LAGRAPH_OK(BlockMatrix_nrows(&anrows, A));
	LAGRAPH_OK(BlockMatrix_ncols(&ancols, A));
	LAGRAPH_OK(BlockMatrix_nrows(&bnrows, B));
	LAGRAPH_OK(BlockMatrix_ncols(&bncols, B));
	LAGRAPH_OK(BlockMatrix_nrows(&cnrows, C));
	LAGRAPH_OK(BlockMatrix_ncols(&cncols, C));

	if(ancols != bnrows || cnrows != anrows || cncols != bncols) {
		LAGRAPH_ERROR("BlockMatrix_mxm: Dimensions not compatible", GrB_DIMENSION_MISMATCH);
	}

	// Determine which blocks in the result matrix will be set.
	LAGRAPH_OK(GrB_mxm(C->s, GrB_NULL, GrB_NULL, GxB_ANY_PAIR_BOOL, A->s, B->s, GrB_NULL));

	LAGRAPH_OK(BlockMatrix_nblocks(&nnz, C));
	GrB_Index I[nnz];
	GrB_Index J[nnz];
	LAGRAPH_OK(GrB_Matrix_extractTuples_BOOL(I, J, NULL, &nnz, C->s));

	blocks = calloc(sizeof(GrB_Matrix), nnz);
	if(blocks == NULL) {
		LAGRAPH_ERROR("out of memory", GrB_OUT_OF_MEMORY);
	}

	int nthread = MIN(nnz, 4);
	#pragma omp parallel for num_threads(nthread)
	for(GrB_Index i = 0; i < nnz; i++) {
		GrB_Index row = I[i];
		GrB_Index col = J[i];
		// Perform DOT product between row I of A and col J of B.
		// TODO: Handle failure.
		BlockMatrix_DotProduct(blocks + i, GxB_ANY_PAIR_BOOL, A, B, row, col);
	}
	// End of parallel region.

	// Set blocks into C. Note `_SetBlock` isn't thread safe.
	for(GrB_Index i = 0; i < nnz; i++) {
		GrB_Index row = I[i];
		GrB_Index col = J[i];
		LAGRAPH_OK(BlockMatrix_SetBlock(C, blocks[i], row, col));
	}

	free(blocks);
	return GrB_SUCCESS;
}
