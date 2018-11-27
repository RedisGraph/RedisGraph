//------------------------------------------------------------------------------
// GB_status_code: return an error string describing the last error
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB.h"

// GB_status_code: convert GrB_Info enum into a string

const char *GB_status_code (GrB_Info info)
{
    switch (info)
    {
        case GrB_SUCCESS              : return ("GrB_SUCCESS") ;
        case GrB_NO_VALUE             : return ("GrB_NO_VALUE") ;
        case GrB_UNINITIALIZED_OBJECT : return ("GrB_UNINITIALIZED_OBJECT") ;
        case GrB_INVALID_OBJECT       : return ("GrB_INVALID_OBJECT") ;
        case GrB_NULL_POINTER         : return ("GrB_NULL_POINTER") ;
        case GrB_INVALID_VALUE        : return ("GrB_INVALID_VALUE") ;
        case GrB_INVALID_INDEX        : return ("GrB_INVALID_INDEX") ;
        case GrB_DOMAIN_MISMATCH      : return ("GrB_DOMAIN_MISMATCH") ;
        case GrB_DIMENSION_MISMATCH   : return ("GrB_DIMENSION_MISMATCH") ;
        case GrB_OUTPUT_NOT_EMPTY     : return ("GrB_OUTPUT_NOT_EMPTY") ;
        case GrB_OUT_OF_MEMORY        : return ("GrB_OUT_OF_MEMORY") ;
        case GrB_INSUFFICIENT_SPACE   : return ("GrB_INSUFFICIENT_SPACE") ;
        case GrB_INDEX_OUT_OF_BOUNDS  : return ("GrB_INDEX_OUT_OF_BOUNDS") ;
        case GrB_PANIC                : return ("GrB_PANIC") ;
        default                       : return ("unknown!") ;
    }
}

