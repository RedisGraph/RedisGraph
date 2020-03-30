/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "./Utility/utility.h"
#include "../Include/block_matrix.h"

#define LAGRAPH_FREE_ALL            \
{                                   \
	GrB_free(&s);                   \
	LAGraph_free(blocks);           \
    LAGraph_free(&block_matrix);    \
}

GrB_Info BlockMatrix_new    // create a new matrix with no entries
(
	BlockMatrix *B,         // handle of matrix to create
	GrB_Type type,          // type of matrix to create
	GrB_Index nrows,        // matrix dimension is nrows-by-ncols
	GrB_Index ncols,
	int nblocks
) {

	GrB_Info    info;
	GrB_Matrix  s = GrB_NULL;
	GrB_Matrix  *blocks = NULL;
	BlockMatrix block_matrix = NULL;

	if(B == NULL) {
		// input arguments invalid
		LAGRAPH_ERROR("BlockMatrix_new: invalid inputs", GrB_NULL_POINTER);
	}

	// Set defaults.
	*B = NULL;

	// Determine block size.

	// Number of blocks should be a squared number.
	int blocks_per_row = sqrt(nblocks);
	int blocks_per_col = blocks_per_row;
	if((sqrt(nblocks) - blocks_per_row) > 0) {
		LAGRAPH_ERROR("BlockMatrix_new: number of blocks should be a sqrt number.", GrB_PANIC);
	}

	GrB_Index block_nrows = ceil((float)nrows / blocks_per_row);
	GrB_Index block_ncols = ceil((float)ncols / blocks_per_col);
	printf("Creating a %dX%d block matrix, block size: %lldX%lld\n", blocks_per_col, blocks_per_row,
		   block_nrows, block_ncols);

	// Create structure matrix.
	LAGRAPH_OK(GrB_Matrix_new(&s, GrB_BOOL, blocks_per_row, blocks_per_col));

	// Create block matrix.
	block_matrix = malloc(sizeof(GB_BlockMatrix_opaque));
	// Create blocks array.
	blocks = calloc(sizeof(GrB_Matrix), nblocks);

	if(block_matrix == NULL || blocks == NULL) {
		LAGRAPH_ERROR("out of memory", GrB_OUT_OF_MEMORY) ;
	}

	// Initialize block matrix.
	block_matrix->s = s;
	block_matrix->t = type;
	block_matrix->blocks = blocks;
	block_matrix->block_nrows = block_nrows;
	block_matrix->block_ncols = block_ncols;

	*B = block_matrix;
	return GrB_SUCCESS;
}
