/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "./Utility/utility.h"
#include "../Include/block_matrix.h"

#define LAGRAPH_FREE_ALL            \
{                                   \
	if(C) {                         \
		BlockMatrix_clear(C);       \
	}                               \
}

//------------------------------------------------------------------------------
// matrix transpose
//------------------------------------------------------------------------------

// T = A' is computed by default, but A can also be transposed via the
// descriptor.  In this case A is not transposed at all, and T = A.  The result
// is then passed through the Mask and accum, like almost all other GraphBLAS
// operations.  This makes GrB_transpose a direct interface to the accum/mask
// operation, C<Mask> = accum (C,A), or C<Mask> = accum (C,A') by default.

GrB_Info BlockMatrix_transpose      // C<Mask> = accum (C, A')
(
	BlockMatrix C,                  // input/output matrix for results
	const GrB_Matrix Mask,          // optional mask for C, unused if NULL
	const GrB_BinaryOp accum,       // optional accum for Z=accum(C,T)
	const BlockMatrix A,            // first input:  matrix A
	const GrB_Descriptor desc       // descriptor for C, Mask, and A
) {
	GrB_Info info;
	if(C == NULL || A == NULL) {
		// input arguments invalid
		LAGRAPH_ERROR("BlockMatrix_transpose: invalid inputs", GrB_NULL_POINTER);
	}

	GrB_Index anrows;
	GrB_Index ancols;
	GrB_Index cnrows;
	GrB_Index cncols;
	LAGRAPH_OK(BlockMatrix_nrows(&anrows, A));
	LAGRAPH_OK(BlockMatrix_ncols(&ancols, A));
	LAGRAPH_OK(BlockMatrix_nrows(&cnrows, C));
	LAGRAPH_OK(BlockMatrix_ncols(&cncols, C));

	// Validate dimensions.
	if(anrows != cncols || ancols != cnrows) {
		LAGRAPH_ERROR("BlockMatrix_transpose: Dimensions not compatible", GrB_DIMENSION_MISMATCH);
	}

	if(C == A) {
		// In place transpose.
		LAGRAPH_ERROR("BlockMatrix_transpose: in place transpose not supported", GrB_PANIC);
	}

	// Clear output.
	LAGRAPH_OK(BlockMatrix_clear(C));

	// Relocate each block of A to its transposed position in C.
	GrB_Index nvals;
	LAGRAPH_OK(GrB_Matrix_nvals(&nvals, A->s));

	GrB_Index I[nvals];
	GrB_Index J[nvals];
	LAGRAPH_OK(GrB_Matrix_extractTuples_BOOL(I, J, NULL, &nvals, A->s));

	for(GrB_Index i = 0; i < nvals; i++) {
		GrB_Matrix block;
		GrB_Index row = I[i];
		GrB_Index col = J[i];
		LAGRAPH_OK(BlockMatrix_GetBlock(&block, A, row, col));

		// Make a copy of block.
		GrB_Matrix tblock;
		LAGRAPH_OK(GrB_Matrix_new(&tblock, C->t, C->block_nrows, C->block_ncols));
		LAGRAPH_OK(GrB_transpose(tblock, GrB_NULL, GrB_NULL, block, GrB_NULL));
		LAGRAPH_OK(BlockMatrix_SetBlock(C, tblock, col, row));
	}

	return GrB_SUCCESS;
}
