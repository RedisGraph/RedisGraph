/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "../Include/block_matrix.h"

// Maps global entry position to block position.
GrB_Info BlockMatrix_mapEntryToBlock
(
  const BlockMatrix B,          // block matrix
	GrB_Index i,                // entry row index
	GrB_Index j,                // entry column index
	GrB_Index *block_i,         // block row index
	GrB_Index *block_j,         // block column index
	GrB_Index *block_i_offset,  // entry row offset within block
	GrB_Index *block_j_offset   // entry column offset within block
);
