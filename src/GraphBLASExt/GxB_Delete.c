/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "./GxB_Delete.h"

GrB_Info GxB_Matrix_Delete
(
	GrB_Matrix M,
	GrB_Index row,
	GrB_Index col
) {
	GrB_Matrix Z ;  // 1X1 empty matrix.
	GrB_Matrix_new(&Z, GrB_BOOL, 1, 1) ;

	GrB_Info info = GxB_Matrix_subassign(M,
										 GrB_NULL,
										 GrB_NULL,
										 Z,
										 &row,
										 1,
										 &col,
										 1,
										 GrB_NULL) ;

	GrB_Matrix_free(&Z);
	return info;
}
