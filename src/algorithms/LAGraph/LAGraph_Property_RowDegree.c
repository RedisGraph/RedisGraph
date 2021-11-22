//------------------------------------------------------------------------------
// LAGraph_Property_RowDegree: determine G->rowdegree
//------------------------------------------------------------------------------

// LAGraph, (c) 2021 by The LAGraph Contributors, All Rights Reserved.
// SPDX-License-Identifier: BSD-2-Clause
// Contributed by Tim Davis, Texas A&M University.

//------------------------------------------------------------------------------

#include "RG.h"
#include "GraphBLAS.h"

#define LAGraph_FREE_WORK       \
{                               \
	GrB_free (&S) ;             \
	GrB_free (&x) ;             \
}

#define LAGraph_FREE_ALL        \
{                               \
	LAGraph_FREE_WORK ;         \
	GrB_free (&rowdegree) ;     \
}

#define GrB_TRY(res)              \
{                                 \
	info = res ;                  \
	ASSERT(info == GrB_SUCCESS) ; \
}

GrB_Semiring LAGraph_plus_one_int64  = NULL ;

int LAGraph_Property_RowDegree  // 0 if successful, -1 if failure
(
	GrB_Matrix    A,
	GrB_Vector *rowdegree
)
{

	//--------------------------------------------------------------------------
	// clear msg and check G
	//--------------------------------------------------------------------------

	GrB_Info info ;
	GrB_Matrix S = NULL ;
	GrB_Vector x = NULL ;

	UNUSED(info);

	//--------------------------------------------------------------------------
	// determine the size of A
	//--------------------------------------------------------------------------

	GrB_Index nrows, ncols ;
	GrB_TRY (GrB_Matrix_nrows (&nrows, A)) ;
	GrB_TRY (GrB_Matrix_ncols (&ncols, A)) ;

	//--------------------------------------------------------------------------
	// compute the rowdegree
	//--------------------------------------------------------------------------

	GrB_TRY (GrB_Vector_new (rowdegree, GrB_INT64, nrows)) ;
	// x = zeros (ncols,1)
	GrB_TRY (GrB_Vector_new (&x, GrB_INT64, ncols)) ;
	GrB_TRY (GrB_assign (x, NULL, NULL, 0, GrB_ALL, ncols, NULL)) ;

	if(!LAGraph_plus_one_int64) {
		GrB_TRY (GrB_Semiring_new (&LAGraph_plus_one_int64,
			GrB_PLUS_MONOID_INT64 , GrB_ONEB_INT64 )) ;
	}
		
	GrB_TRY (GrB_mxv (*rowdegree, NULL, NULL, LAGraph_plus_one_int64,
		A, x, NULL)) ;

	LAGraph_FREE_WORK ;
	return (0) ;
}