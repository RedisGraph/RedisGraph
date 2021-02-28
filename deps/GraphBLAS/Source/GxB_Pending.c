//------------------------------------------------------------------------------
// GxB_Matrix_Pending: checks to see if matrix has pending operations
//------------------------------------------------------------------------------

#include "GB.h"
#include "GB_Pending.h"

GrB_Info GxB_Matrix_Pending     // does matrix has pending operations
(
	GrB_Matrix A,           // matrix to query
	bool *pending           // are there any pending operations
) {
	GB_WHERE("GxB_Matrix_Pending (A)") ;
	//--------------------------------------------------------------------------
	// check inputs
	//--------------------------------------------------------------------------
	GB_RETURN_IF_NULL_OR_FAULTY(A) ;
	GB_RETURN_IF_NULL(pending) ;

	(*pending) = (A->nzombies > 0) || (GB_Pending_n(A) > 0) ;

	return (GrB_SUCCESS) ;
}
