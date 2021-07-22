/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "RG.h"
#include "rg_matrix.h"
#include "../../util/rmalloc.h"

// creates a new matrix
GrB_Info RG_Matrix_new
(
	RG_Matrix *A,
	GrB_Type type,
	GrB_Index nrows,
	GrB_Index ncols,
	bool multi_edge,
	bool maintain_transpose
) {
	GrB_Info info;
	RG_Matrix matrix = rm_calloc(1, sizeof(_RG_Matrix));

	//--------------------------------------------------------------------------
	// input validations
	//--------------------------------------------------------------------------

	// supported types: boolean and uint64
	ASSERT(type == GrB_BOOL || type == GrB_UINT64);

	matrix->dirty               =  false;
	matrix->multi_edge          =  multi_edge & (type == GrB_UINT64);
	matrix->maintain_transpose  =  maintain_transpose;

	//--------------------------------------------------------------------------
	// create m, delta-plus and delta-minus
	//--------------------------------------------------------------------------

	info = GrB_Matrix_new(&matrix->matrix, type, nrows, ncols);
	ASSERT(info == GrB_SUCCESS);
	//info = GxB_set(matrix->matrix, GxB_SPARSITY_CONTROL, GxB_SPARSE);
	//ASSERT(info == GrB_SUCCESS);

	info = GrB_Matrix_new(&matrix->delta_plus, type, nrows, ncols);
	ASSERT(info == GrB_SUCCESS);
	//info = GxB_set(matrix->delta_plus, GxB_SPARSITY_CONTROL, GxB_HYPERSPARSE);
	//ASSERT(info == GrB_SUCCESS);

	info = GrB_Matrix_new(&matrix->delta_minus, GrB_BOOL, nrows, ncols);
	ASSERT(info == GrB_SUCCESS);
	//info = GxB_set(matrix->delta_minus, GxB_SPARSITY_CONTROL, GxB_HYPERSPARSE);
	//ASSERT(info == GrB_SUCCESS);

	//----------------------------------------------------------------------------
	// create transpose matrix if required
	//----------------------------------------------------------------------------

	if(maintain_transpose) {
		info = RG_Matrix_new(&matrix->transposed, type, nrows, ncols,
				multi_edge, false);
		ASSERT(info == GrB_SUCCESS);
	}

	int mutex_res = pthread_mutex_init(&matrix->mutex, NULL);
	ASSERT(mutex_res == 0);

	*A = matrix;
	return info;
}

