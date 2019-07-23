/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#ifndef __GXB_DELETE_H__
#define __GXB_DELETE_H__

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

#endif
