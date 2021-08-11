/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "RG.h"
#include "rg_matrix.h"
#include "../../util/rmalloc.h"

void RG_Matrix_setDirty
(
	RG_Matrix C
) {
	ASSERT(C);
	C->dirty = true;
	if(RG_MATRIX_MAINTAIN_TRANSPOSE(C)) C->transposed->dirty = true;
}

RG_Matrix RG_Matrix_getTranspose
(
	const RG_Matrix C
) {
	ASSERT(C != NULL);
	return C->transposed;
}

bool RG_Matrix_isDirty
(
	const RG_Matrix C
) {
	ASSERT(C);
	return C->dirty;
}

// locks the matrix
void RG_Matrix_Lock
(
	RG_Matrix C
) {
	ASSERT(C);
	pthread_mutex_lock(&C->mutex);
}

// unlocks the matrix
void RG_Matrix_Unlock
(
	RG_Matrix C
) {
	ASSERT(C);
	pthread_mutex_unlock(&C->mutex);
}

GrB_Info RG_Matrix_nrows
(
	GrB_Index *nrows,
	const RG_Matrix C
) {
	ASSERT(C);
	ASSERT(nrows);

	GrB_Matrix m = RG_MATRIX_M(C);
	return GrB_Matrix_nrows(nrows, m);
}

GrB_Info RG_Matrix_ncols
(
	GrB_Index *ncols,
	const RG_Matrix C
) {
	ASSERT(C);
	ASSERT(ncols);

	GrB_Matrix m = RG_MATRIX_M(C);
	return GrB_Matrix_ncols(ncols, m);
}

GrB_Info RG_Matrix_nvals    // get the number of entries in a matrix
(
    GrB_Index *nvals,       // matrix has nvals entries
    const RG_Matrix A       // matrix to query
) {
	ASSERT(A      !=  NULL);
	ASSERT(nvals  !=  NULL);

	GrB_Matrix  m;
	GrB_Matrix  dp;
	GrB_Matrix  dm;
	GrB_Info    info;

	GrB_Index  m_nvals   =  0;
	GrB_Index  dp_nvals  =  0;
	GrB_Index  dm_nvals  =  0;

	// nvals = nvals(M) + nvals(DP) - nvals(DM)

	m   =  RG_MATRIX_M(A);
	dp  =  RG_MATRIX_DELTA_PLUS(A);
	dm  =  RG_MATRIX_DELTA_MINUS(A);

	info = GrB_Matrix_nvals(&m_nvals, m);
	ASSERT(info == GrB_SUCCESS);
	info = GrB_Matrix_nvals(&dp_nvals, dp);
	ASSERT(info == GrB_SUCCESS);
	info = GrB_Matrix_nvals(&dm_nvals, dm);
	ASSERT(info == GrB_SUCCESS);

	*nvals = m_nvals + dp_nvals - dm_nvals;
	return info;
}

GrB_Info RG_Matrix_clear
(
    RG_Matrix A
) {	
	GrB_Matrix  m            =  RG_MATRIX_M(A);
	GrB_Info    info         =  GrB_SUCCESS;
	GrB_Matrix  delta_plus   =  RG_MATRIX_DELTA_PLUS(A);
	GrB_Matrix  delta_minus  =  RG_MATRIX_DELTA_MINUS(A);

	info = GrB_Matrix_clear(m);
	ASSERT(info == GrB_SUCCESS);

	info = GrB_Matrix_clear(m);
	ASSERT(info == GrB_SUCCESS);

	info = GrB_Matrix_clear(m);
	ASSERT(info == GrB_SUCCESS);

	A->dirty = false;
	if(RG_MATRIX_MAINTAIN_TRANSPOSE(A)) A->transposed->dirty = false;

	return info;
}
