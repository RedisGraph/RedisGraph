/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "RG.h"
#include "rg_matrix.h"
#include "../../util/rmalloc.h"
#include "configuration/config.h"

static inline void _SetUndirty
(
	RG_Matrix C
) {
	ASSERT(C);

	C->dirty = false;

	if(RG_MATRIX_MAINTAIN_TRANSPOSE(C)) {
		C->transposed->dirty = false;
	}
}

static GrB_Info RG_Matrix_sync
(
	RG_Matrix C
) {
	ASSERT(C != NULL);

	GrB_Matrix      m     =  RG_MATRIX_M(C);
	GrB_Matrix      dp    =  RG_MATRIX_DELTA_PLUS(C);
	GrB_Matrix      dm    =  RG_MATRIX_DELTA_MINUS(C);
	GrB_Descriptor  desc  =  GrB_NULL;
	GrB_Matrix      mask  =  GrB_NULL;

	GrB_Info info;
	GrB_Index dp_nvals;
	GrB_Index dm_nvals;

	//--------------------------------------------------------------------------
	// determin change set
	//--------------------------------------------------------------------------

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
		GrB_Semiring s;
		info = GxB_Matrix_type(&t, m);
		ASSERT(info == GrB_SUCCESS);

		s = (t == GrB_BOOL) ? GxB_ANY_PAIR_BOOL : GxB_ANY_PAIR_UINT64;
		info = GrB_Matrix_eWiseAdd_Semiring(m, NULL, NULL, s, m, dp, NULL);
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

	// wait on all 3 matrices
	info = GrB_wait(m,  GrB_MATERIALIZE);
	ASSERT(info == GrB_SUCCESS);

	info = GrB_wait(dm, GrB_MATERIALIZE);
	ASSERT(info == GrB_SUCCESS);

	info = GrB_wait(dp, GrB_MATERIALIZE);
	ASSERT(info == GrB_SUCCESS);

	return info;
}

GrB_Info RG_Matrix_wait
(
	RG_Matrix A,
	bool force_sync
) {
	ASSERT(A != NULL);
	if(RG_MATRIX_MAINTAIN_TRANSPOSE(A)) {
		RG_Matrix_wait(A->transposed, force_sync);
	}
	
	GrB_Info   info        = GrB_SUCCESS;
	GrB_Matrix m           = RG_MATRIX_M(A);
	GrB_Matrix delta_plus  = RG_MATRIX_DELTA_PLUS(A);
	GrB_Matrix delta_minus = RG_MATRIX_DELTA_MINUS(A);

	// check if merge is required
	GrB_Index delta_plus_nvals;
	GrB_Index delta_minus_nvals;
	GrB_Matrix_nvals(&delta_plus_nvals, delta_plus);
	GrB_Matrix_nvals(&delta_minus_nvals, delta_minus);

	uint64_t delta_max_pending_changes;
	Config_Option_get(Config_DELTA_MAX_PENDING_CHANGES,
			&delta_max_pending_changes);

	if(force_sync ||
	   delta_plus_nvals + delta_minus_nvals >= delta_max_pending_changes) {
		info = RG_Matrix_sync(A);
	} else {
		// wait on 'm', in most cases 'm' won't contain any pending work
		// but it might need to build its internal hyper-hash
		info = GrB_wait(m, GrB_MATERIALIZE);
		ASSERT(info == GrB_SUCCESS);

		info = GrB_wait(delta_plus, GrB_MATERIALIZE);
		ASSERT(info == GrB_SUCCESS);

		info = GrB_wait(delta_minus, GrB_MATERIALIZE);
		ASSERT(info == GrB_SUCCESS);
	}

	_SetUndirty(A);

	return info;
}

