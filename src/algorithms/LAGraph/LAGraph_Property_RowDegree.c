//------------------------------------------------------------------------------
// LAGraph_Property_RowDegree: determine G->rowdegree
//------------------------------------------------------------------------------

// LAGraph, (c) 2021 by The LAGraph Contributors, All Rights Reserved.
// SPDX-License-Identifier: BSD-2-Clause
// Contributed by Tim Davis, Texas A&M University.

//------------------------------------------------------------------------------

#include "RG.h"
#include "GraphBLAS.h"

GrB_Semiring LAGraph_plus_one_int64  = NULL ;

int LAGraph_Property_RowDegree  // 0 if successful, -1 if failure
(
	GrB_Matrix    A,
	GrB_Vector *rowdegree
)
{
	GrB_Vector x = NULL ;

	//--------------------------------------------------------------------------
	// determine the size of A
	//--------------------------------------------------------------------------

	GrB_Index nrows, ncols ;
	GrB_OK (GrB_Matrix_nrows (&nrows, A)) ;
	GrB_OK (GrB_Matrix_ncols (&ncols, A)) ;

	//--------------------------------------------------------------------------
	// compute the rowdegree
	//--------------------------------------------------------------------------

	GrB_OK (GrB_Vector_new (rowdegree, GrB_INT64, nrows)) ;
	// x = zeros (ncols,1)
	GrB_OK (GrB_Vector_new (&x, GrB_INT64, ncols)) ;
	GrB_OK (GrB_assign (x, NULL, NULL, 0, GrB_ALL, ncols, NULL)) ;

	if(!LAGraph_plus_one_int64) {
		GrB_OK (GrB_Semiring_new (&LAGraph_plus_one_int64,
			GrB_PLUS_MONOID_INT64 , GrB_ONEB_INT64 )) ;
	}
		
	GrB_OK (GrB_mxv (*rowdegree, NULL, NULL, LAGraph_plus_one_int64,
		A, x, NULL)) ;

	GrB_free (&x) ;
	return (0) ;
}

