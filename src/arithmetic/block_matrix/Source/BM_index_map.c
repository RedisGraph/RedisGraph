/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "./Utility/utility.h"
#include "./BM_index_map.h"

#define LAGRAPH_FREE_ALL     \
{                            \
}

// Maps global entry position to block position.
GrB_Info BlockMatrix_mapEntryToBlock
(
	const BlockMatrix B,        // block matrix
	GrB_Index i,                // entry row index
	GrB_Index j,                // entry column index
	GrB_Index *block_i,         // block row index
	GrB_Index *block_j,         // block column index
	GrB_Index *block_i_offset,  // entry row offset within block
	GrB_Index *block_j_offset   // entry column offset within block
) {
	// TODO: Validate i and j.
	GrB_Info info;
	if(B == NULL || block_i == NULL || block_j == NULL || block_i_offset == NULL ||
	   block_j_offset == NULL) {
		LAGRAPH_ERROR("BlockMatrix_mapEntryToBlock: invalid inputs", GrB_NULL_POINTER);
	}

	*block_i = i / B->block_nrows;
	*block_j = j / B->block_ncols;
	*block_i_offset = i % B->block_nrows;
	*block_j_offset = j % B->block_ncols;

	return GrB_SUCCESS;
}
