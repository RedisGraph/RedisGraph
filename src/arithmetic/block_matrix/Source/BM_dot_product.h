/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "../Include/block_matrix.h"

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
);
