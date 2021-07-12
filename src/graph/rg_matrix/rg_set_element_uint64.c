/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "RG.h"
#include "rg_matrix.h"
#include "../../util/arr.h"

// sets A[i,j] = x
// dealing with multi-value entries
static GrB_Info setMultiEdgeEntry
(
    GrB_Matrix A,                       // matrix to modify
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
		v = entries;
		v = SET_MSB(v);
		info = GrB_Matrix_setElement_UINT64(A, v, i, j);
		// cheap sync, entry already exists
		GrB_wait(&A);
	}

	ASSERT(info == GrB_SUCCESS);

	return info;
}

static GrB_Info setDP                   // DP (i,j) = x
(
    RG_Matrix C,                        // matrix to modify
    uint64_t x,                         // scalar to assign to C(i,j)
    GrB_Index i,                        // row index
    GrB_Index j                         // column index
) {
	// add entry to delta-plus
	GrB_Info info;
	GrB_Matrix  dp          =  RG_MATRIX_DELTA_PLUS(C);
	bool        multi_edge  =  RG_Matrix_getMultiEdge(C);

	if(!multi_edge) {
		// no support for multi-edge simply set entry
		info = GrB_Matrix_setElement_UINT64(dp, x, i, j);
	} else {
		info = setMultiEdgeEntry(dp, x, i, j);
	}

	if(info == GrB_SUCCESS) RG_Matrix_setDirty(C);

	return info;
}

static GrB_Info setM                    // M (i,j) = x
(
    RG_Matrix C,                        // matrix to modify
    uint64_t x,                         // scalar to assign to C(i,j)
    GrB_Index i,                        // row index
    GrB_Index j                         // column index
) {
	// add entry to delta-plus
	GrB_Info info;
	GrB_Matrix  m           =  RG_MATRIX_M(C);
	bool        multi_edge  =  RG_Matrix_getMultiEdge(C);

	if(!multi_edge) {
		// no support for multi-edge simply set entry
		info = GrB_Matrix_setElement_UINT64(m, x, i, j);
	} else {
		info = setMultiEdgeEntry(m, x, i, j);
	}

	if(info == GrB_SUCCESS) RG_Matrix_setDirty(C);

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

	GrB_Info info;

	if(C->maintain_transpose) {
		info = RG_Matrix_setElement_UINT64(C->transposed, x, j, i);
		ASSERT(info == GrB_SUCCESS);
	}

	GrB_Matrix m  = RG_MATRIX_M(C);
	GrB_Matrix dp = RG_MATRIX_DELTA_PLUS(C);
	GrB_Matrix dm = RG_MATRIX_DELTA_MINUS(C);

	uint64_t v;
	bool  entry_exists     =  false;  //  M[i,j] exists
	bool  mark_as_deleted  =  false;  //  dm[i,j] exists

	//--------------------------------------------------------------------------
	// check deleted
	//--------------------------------------------------------------------------

	info = GrB_Matrix_extractElement(&v, dm, i, j);	
	mark_as_deleted = (info == GrB_SUCCESS);

	if(mark_as_deleted) {
		// can't use 'M', add entry to 'delta-plus'
		info = setDP(C, x, i, j);
	} else {
		// entry isn't marked for deletion
		// see if entry already exists in 'm'
		// we'll prefer setting entry in 'm' incase it already exists
		// otherwise we'll set the entry in 'delta-plus'
		info = GrB_Matrix_extractElement_UINT64(&v, m, i, j);
		entry_exists = (info == GrB_SUCCESS);

		if(entry_exists) {
			info = setM(C, x, i, j);
		} else {
			// add entry to 'delta-plus'
			info = setDP(C, x, i, j);
		}
	}

#ifdef RG_DEBUG
	RG_Matrix_validateState(C, i, j);
#endif

	return info;
}

