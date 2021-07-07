/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "RG.h"
#include "rg_matrix.h"
#include "../../util/arr.h"
#include "../../util/rmalloc.h"

GrB_Info RG_Matrix_removeElement
(
    RG_Matrix C,                    // matrix to remove entry from
    GrB_Index i,                    // row index
    GrB_Index j                     // column index
) {
	ASSERT(C);

	uint64_t    x;
	GrB_Info    info;
	bool        in_m        =  false;
	bool        in_dp       =  false;
	bool        in_dm       =  false;
	bool        multi_edge  =  false;
	GrB_Matrix  m           =  RG_MATRIX_MATRIX(C);
	GrB_Matrix  dp          =  RG_MATRIX_DELTA_PLUS(C);
	GrB_Matrix  dm          =  RG_MATRIX_DELTA_MINUS(C);

#if RG_DEBUG
	// check bounds
	GrB_Index nrows;
	GrB_Index ncols;
	GrB_Matrix_nrows(&nrows, m);
	GrB_Matrix_ncols(&ncols, m);
	ASSERT(i < nrows);
	ASSERT(j < ncols);
#endif

	if(C->maintain_transpose) {
		info = RG_Matrix_removeElement(C->transposed, j, i);
		ASSERT(info == GrB_SUCCESS);
	}

	info = GrB_Matrix_extractElement(&x, m, i, j);
	in_m = (info == GrB_SUCCESS);

	info = GrB_Matrix_extractElement(&x, dp, i, j);
	in_dp = (info == GrB_SUCCESS);

	info = GrB_Matrix_extractElement(&x, dm, i, j);
	in_dm = (info == GrB_SUCCESS);

	// mask 'in_m' incase it is marked for deletion
	in_m = in_m && !(in_dm);

	// entry missing from both 'm' and 'dp'
	if(!(in_m || in_dp)) {
		return GrB_SUCCESS;
	}

	// entry exists in 'M'
	if(in_m) {
		// mark deletion in delta minus
		info = GrB_Matrix_setElement(dm, true, i, j);
		ASSERT(info == GrB_SUCCESS);
	}

	// entry exists in 'delta-plus'
	if(in_dp) {
		// remove entry from 'dp'
		multi_edge = RG_Matrix_getMultiEdge(C);
		if(multi_edge) {
			GrB_Matrix_extractElement(&x, dp, i, j);
			if((SINGLE_EDGE(x)) == false) {
				array_free((uint64_t*)x);
			}
		}

		info = GrB_Matrix_removeElement(dp, i, j);
		ASSERT(info == GrB_SUCCESS);
	}

	RG_Matrix_setDirty(C);
	return info;
}

