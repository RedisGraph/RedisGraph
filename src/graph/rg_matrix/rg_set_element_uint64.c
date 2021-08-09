/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "RG.h"
#include "rg_utils.h"
#include "rg_matrix.h"
#include "../../util/arr.h"

// dealing with multi-value entries
static GrB_Info setMultiEdgeEntry
(
    GrB_Matrix A,                       // matrix to modify
	GrB_Matrix TA,                      // transposed matrix to modify
    uint64_t x,                         // scalar to assign to A(i,j)
    GrB_Index i,                        // row index
    GrB_Index j                         // column index
) {
	uint64_t v;                // v = A[i,j]
	GrB_Info info;
	uint64_t *entries = NULL;  // array of values at A[i,j]

	// check if entry already exists
	info = GrB_Matrix_extractElement(&v, A, i, j);	
	bool exists = (info == GrB_SUCCESS);

	// new entry, simply set
	if(!exists) {
		// mark 'x' as a single entry
		v = x;
		info = GrB_Matrix_setElement_UINT64(A, v, i, j);
		info = GrB_Matrix_setElement_UINT64(TA, v, j, i);
	} else {
		// entry already exists
		if(SINGLE_EDGE(v)) {
			// swap from single entry to multi-entry
			entries = array_new(uint64_t, 2);
			array_append(entries, v);
			array_append(entries, x);
		} else {
			v = CLEAR_MSB(v);
			// append entry to array
			entries = (uint64_t *)v;
			array_append(entries, x);
		}
		v = (uint64_t)entries;
		v = SET_MSB(v);
		info = GrB_Matrix_setElement_UINT64(A, v, i, j);
		info = GrB_Matrix_setElement_UINT64(TA, v, j, i);
		// cheap sync, entry already exists
		GrB_wait(&A);
	}

	ASSERT(info == GrB_SUCCESS);

	return info;
}

GrB_Info RG_Matrix_setElement_UINT64    // C (i,j) = x
(
    RG_Matrix C,                        // matrix to modify
    uint64_t x,                         // scalar to assign to C(i,j)
    GrB_Index i,                        // row index
    GrB_Index j                         // column index
) {
	ASSERT(C != NULL);
	ASSERT(RG_MATRIX_MAINTAIN_TRANSPOSE(C));
	RG_Matrix_checkBounds(C, i, j);

	uint64_t  v;
	GrB_Info  info;
	bool      entry_exists       =  false;          //  M[i,j] exists
	bool      mark_for_deletion  =  false;          //  dm[i,j] exists

	GrB_Matrix m   = RG_MATRIX_M(C);
	GrB_Matrix dp  = RG_MATRIX_DELTA_PLUS(C);
	GrB_Matrix dm  = RG_MATRIX_DELTA_MINUS(C);
	GrB_Matrix tm  = RG_MATRIX_TM(C);
	GrB_Matrix tdp = RG_MATRIX_TDELTA_PLUS(C);
	GrB_Matrix tdm = RG_MATRIX_TDELTA_MINUS(C);

	#if RG_DEBUG

		//----------------------------------------------------------------------
		// validate type
		//----------------------------------------------------------------------

		GrB_Type t;
		info = GxB_Matrix_type(&t, m);
		ASSERT(info == GrB_SUCCESS);
		ASSERT(t == GrB_UINT64);

	#endif

	//--------------------------------------------------------------------------
	// check deleted
	//--------------------------------------------------------------------------

	info = GrB_Matrix_extractElement(&v, dm, i, j);	
	mark_for_deletion = (info == GrB_SUCCESS);

	if(mark_for_deletion) { // m contains single edge, simple replace
		// clear dm[i,j]
		info = GrB_Matrix_removeElement(dm, i, j);
		ASSERT(info == GrB_SUCCESS);
		info = GrB_Matrix_removeElement(tdm, j, i);
		ASSERT(info == GrB_SUCCESS);

		// overwrite m[i,j]
		info = GrB_Matrix_setElement(m, x, i, j);
		ASSERT(info == GrB_SUCCESS);
		info = GrB_Matrix_setElement(tm, x, j, i);
		ASSERT(info == GrB_SUCCESS);
	} else {
		// entry isn't marked for deletion
		// see if entry already exists in 'm'
		// we'll prefer setting entry in 'm' incase it already exists
		// otherwise we'll set the entry in 'delta-plus'
		info = GrB_Matrix_extractElement_UINT64(&v, m, i, j);
		entry_exists = (info == GrB_SUCCESS);

		if(entry_exists) {
			// update entry at m[i,j]
			info = setMultiEdgeEntry(m, tm, x, i, j);
		} else {
			// update entry at dp[i,j]
			info = setMultiEdgeEntry(dp, tdp, x, i, j);
		}
	}

#ifdef RG_DEBUG
	RG_Matrix_validateState(C, i, j);
#endif

	RG_Matrix_setDirty(C);
	return info;
}

