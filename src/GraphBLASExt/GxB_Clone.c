/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "GxB_Clone.h"
#include <assert.h>

GrB_Info GxB_MatrixClone
(
	const GrB_Matrix A,
	GrB_Matrix *C
) {
	assert(A && C);
	GrB_Info res;
	GrB_Type type;
	GrB_Index nrows;
	GrB_Index ncols;
	GrB_Descriptor desc;

	res = GxB_Matrix_type(&type, A);
	assert(res == GrB_SUCCESS);
	res = GrB_Matrix_nrows(&nrows, A);
	assert(res == GrB_SUCCESS);
	res = GrB_Matrix_ncols(&ncols, A);
	assert(res == GrB_SUCCESS);
	res = GrB_Matrix_new(C, type, nrows, ncols);
	assert(res == GrB_SUCCESS);

	res = GrB_Descriptor_new(&desc);
	assert(res == GrB_SUCCESS);
	res = GrB_Descriptor_set(desc, GrB_INP0, GrB_TRAN);
	assert(res == GrB_SUCCESS);

	res = GrB_transpose(*C, GrB_NULL, GrB_NULL, A, desc);
	assert(res == GrB_SUCCESS);

	res = GrB_free(&desc);
	assert(res == GrB_SUCCESS);

	return res;
}
