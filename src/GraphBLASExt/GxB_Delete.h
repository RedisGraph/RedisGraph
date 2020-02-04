/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include <stdint.h>
#include "../../deps/GraphBLAS/Include/GraphBLAS.h"

/* Clears entry at position M[row,col]
 * If M[row,col] isn't empty then M's NNZ is reduced by one. */
GrB_Info GxB_Matrix_Delete
(
	GrB_Matrix M,
	GrB_Index row,
	GrB_Index col
) ;
