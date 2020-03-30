/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "../Include/block_matrix.h"

GrB_Info BlockMatrix_ClearBlock // Clears a single block in a matrix
(
  BlockMatrix B,                // matrix to update
  GrB_Index row,                // block row index
  GrB_Index col                 // block column index
);
