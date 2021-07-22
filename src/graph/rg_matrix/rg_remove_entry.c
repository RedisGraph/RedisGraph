/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "RG.h"
#include "rg_utils.h"
#include "rg_matrix.h"
#include "../../util/arr.h"
#include "../../util/rmalloc.h"

static bool _removeEntryFromMultiValArr
(
	uint64_t **entries,  // multi-value array
	uint64_t entry       // element to remove output new value
) {
	ASSERT(*entries != NULL);

	uint  i  =  0;
	uint  n  =  array_len(*entries);

	// search for entry
	for(; i < n; i++) {
		if((*entries)[i] == entry) {
			break;
		}
	}

	ASSERT(i < n);

	// remove located entry
	// migrate last element and reduce array size
	array_del_fast(*entries, i);

	// incase we're left with a single entry revert back to scalar
	if(array_len(*entries) == 1) {
		entry = (*entries)[0];
		array_free(*entries);
		*entries = entry;
		return true;
	}

	return false;
}

static GrB_Info _removeElementMultiVal
(
    GrB_Matrix A,                    // matrix to remove entry from
    GrB_Index i,                    // row index
    GrB_Index j,                    // column index
	uint64_t  v                     // value to remove
) {
	ASSERT(A);

	uint64_t  x;
	GrB_Info  info;

	info = GrB_Matrix_extractElement(&x, A, i, j);
	ASSERT(info == GrB_SUCCESS);
	ASSERT((SINGLE_EDGE(x)) == false);

	// remove entry from multi-value
	x = CLEAR_MSB(x);
	if(_removeEntryFromMultiValArr(&x, v)) {
		// update entry
		info = GrB_Matrix_setElement(A, x, i, j);
	}

	return info;
}

GrB_Info RG_Matrix_removeEntry
(
    RG_Matrix C,                    // matrix to remove entry from
    GrB_Index i,                    // row index
    GrB_Index j,                    // column index
	uint64_t  v                     // value to remove
) {
	ASSERT(C);
	RG_Matrix_checkBounds(C, i, j);

	uint64_t    m_x;
	uint64_t    dm_x;
	uint64_t    dp_x;
	GrB_Info    info;
	bool        in_m        =  false;
	bool        in_dp       =  false;
	bool        in_dm       =  false;
	bool        multi_edge  =  RG_Matrix_getMultiEdge(C);
	GrB_Matrix  m           =  RG_MATRIX_M(C);
	GrB_Matrix  dp          =  RG_MATRIX_DELTA_PLUS(C);
	GrB_Matrix  dm          =  RG_MATRIX_DELTA_MINUS(C);

	if(C->maintain_transpose) {
		info = RG_Matrix_removeElement(C->transposed, j, i);
		if(info != GrB_SUCCESS) {
			return info;
		} 
	}

	ASSERT(multi_edge);

	// entry should exists in either delta-plus or main
	// locate entry
	info = GrB_Matrix_extractElement(&m_x, m, i, j);
	in_m = (info == GrB_SUCCESS);

	info = GrB_Matrix_extractElement(&dp_x, dp, i, j);
	in_dp = (info == GrB_SUCCESS);

	info = GrB_Matrix_extractElement(&dm_x, dm, i, j);
	in_dm = (info == GrB_SUCCESS);

	// mask 'in_m' incase it is marked for deletion
	in_m = in_m && !(in_dm);

	// entry missing from both 'm' and 'dp'
	if(!(in_m || in_dp)) {
		return GrB_NO_VALUE;
	}

	// entry can't exists in both 'm' and 'dp'
	ASSERT(in_m != in_dp);

	//--------------------------------------------------------------------------
	// entry exists in 'M'
	//--------------------------------------------------------------------------

	if(in_m) {
		if(SINGLE_EDGE(m_x)) {
			// mark deletion in delta minus
			info = GrB_Matrix_setElement(dm, true, i, j);
			ASSERT(info == GrB_SUCCESS);
			RG_Matrix_setDirty(C);
		} else {
			info = _removeElementMultiVal(m, i, j, v);
			ASSERT(info == GrB_SUCCESS);
		}
	}

	//--------------------------------------------------------------------------
	// entry exists in 'delta-plus'
	//--------------------------------------------------------------------------

	if(in_dp) {
		if(SINGLE_EDGE(dp_x)) {
			info = GrB_Matrix_removeElement(dp, i, j);
			ASSERT(info == GrB_SUCCESS);
			RG_Matrix_setDirty(C);
		} else {
			info = _removeElementMultiVal(dp, i, j, v);
			ASSERT(info == GrB_SUCCESS);
		}
	}

#ifdef RG_DEBUG
	RG_Matrix_validateState(C, i, j);
#endif

	return info;
}

