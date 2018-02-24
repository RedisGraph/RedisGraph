//------------------------------------------------------------------------------
// GrB_finalize: finalize GraphBLAS
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// GrB_finalize must be called as the last GraphBLAS function.

// In this version of SuiteSparse:GraphBLAS, GrB_finalize frees the workspace
// held internally in thread-local storage.  It can be called at any time and
// can be followed by GraphBLAS function.

#include "GB.h"

GrB_Info GrB_finalize ( )
{

    // free all workspace
    GB_Mark_free ( ) ;
    GB_Work_free ( ) ;
    GB_Flag_free ( ) ;

    return (GrB_SUCCESS) ;      // method always succeeds
}

