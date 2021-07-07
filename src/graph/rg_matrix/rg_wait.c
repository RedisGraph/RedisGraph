/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "RG.h"
#include "rg_matrix.h"
#include "../../util/rmalloc.h"

#define DELTA_MAX_PENDING_CHANGES 2

static inline void _SetUndirty
(
	RG_Matrix C
) {
	ASSERT(C);
	C->dirty = false;
}

static GrB_Info RG_Matrix_sync
(
	RG_Matrix C
) {
	ASSERT(C != NULL);

	GrB_Matrix      m     =  RG_MATRIX_MATRIX(C);
	GrB_Descriptor  desc  =  GrB_NULL;
	GrB_Matrix      mask  =  GrB_NULL;
	GrB_Matrix      dp    =  RG_MATRIX_DELTA_PLUS(C);
	GrB_Matrix      dm    =  RG_MATRIX_DELTA_MINUS(C);

	GrB_Info info;
	GrB_Index dp_nvals;
	GrB_Index dm_nvals;
	GrB_Matrix_nvals(&dp_nvals, dp);
	GrB_Matrix_nvals(&dm_nvals, dm);

	bool  additions  =  dp_nvals  >  0;
	bool  deletions  =  dm_nvals  >  0;

	//--------------------------------------------------------------------------
	// perform deletions
	//--------------------------------------------------------------------------

	if(deletions) {
		info = GrB_transpose(m, dm, GrB_NULL, m, GrB_DESC_RSCT0);
		ASSERT(info == GrB_SUCCESS);

		// clear delta minus
		info = GrB_Matrix_clear(dm);
		ASSERT(info == GrB_SUCCESS);
	}

	//--------------------------------------------------------------------------
	// perform additions
	//--------------------------------------------------------------------------

	if(additions) {
		GrB_Type t;
		GrB_BinaryOp op;
		info = GxB_Matrix_type(&t, m);
		ASSERT(info == GrB_SUCCESS);

		// use SECOND to overwrite entries that exists in both:
		// 'm' and 'delta-plus' with values from 'delta-plus'
		op = (t == GrB_BOOL) ? GrB_SECOND_BOOL : GrB_SECOND_UINT64;
		info = GrB_Matrix_eWiseAdd_BinaryOp(m, GrB_NULL, GrB_NULL, op, m,
				dp, GrB_NULL);
		ASSERT(info == GrB_SUCCESS);

		// clear delta plus
		info = GrB_Matrix_clear(dp);
		ASSERT(info == GrB_SUCCESS);
	}

	//--------------------------------------------------------------------------
	// validate that both delta-plus and delta-minus are cleared
	//--------------------------------------------------------------------------
	GrB_Index nvals;
	GrB_Matrix_nvals(&nvals, dp);
	ASSERT(nvals == 0);
	GrB_Matrix_nvals(&nvals, dm);
	ASSERT(nvals == 0);

	info = GrB_wait(&m);
	ASSERT(info == GrB_SUCCESS);

	return info;
}

GrB_Info RG_Matrix_wait
(
	RG_Matrix A,
	bool force_sync
) {
	ASSERT(A != NULL);
	if(A->maintain_transpose) RG_Matrix_wait(A->transposed, force_sync);
	
	GrB_Info    info         =  GrB_SUCCESS;
	GrB_Matrix  m            =  RG_MATRIX_MATRIX(A);
	GrB_Matrix  delta_plus   =  RG_MATRIX_DELTA_PLUS(A);
	GrB_Matrix  delta_minus  =  RG_MATRIX_DELTA_MINUS(A);

	info = GrB_wait(&delta_plus);
	ASSERT(info == GrB_SUCCESS);

	info = GrB_wait(&delta_minus);
	ASSERT(info == GrB_SUCCESS);

	// check if merge is required
	
	GrB_Index delta_plus_nvals;
	GrB_Index delta_minus_nvals;
	GrB_Matrix_nvals(&delta_plus_nvals, delta_plus);
	GrB_Matrix_nvals(&delta_minus_nvals, delta_minus);
	if(force_sync ||
	   delta_plus_nvals + delta_minus_nvals >= DELTA_MAX_PENDING_CHANGES) {
		info = RG_Matrix_sync(A);
		 
	}

	_SetUndirty(A);

	return info;
}

