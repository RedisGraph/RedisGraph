//------------------------------------------------------------------------------
// gb_type_to_mxstring: create a MATLAB string from a GraphBLAS type
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "gb_matlab.h"

GrB_Type gb_type_to_mxstring    // return the MATLAB string from a GrB_Type
(
    const GrB_Type type
)
{ 

    if      (type == GrB_BOOL)   return (mxCreateString ("logical")) ;
    else if (type == GrB_INT8)   return (mxCreateString ("int8")) ;
    else if (type == GrB_INT16)  return (mxCreateString ("int16")) ;
    else if (type == GrB_INT32)  return (mxCreateString ("int32")) ;
    else if (type == GrB_INT64)  return (mxCreateString ("int64")) ;
    else if (type == GrB_UINT8)  return (mxCreateString ("uint8")) ;
    else if (type == GrB_UINT16) return (mxCreateString ("uint16")) ;
    else if (type == GrB_UINT32) return (mxCreateString ("uint32")) ;
    else if (type == GrB_UINT64) return (mxCreateString ("uint64")) ;
    else if (type == GrB_FP32)   return (mxCreateString ("single")) ;
    else if (type == GrB_FP64)   return (mxCreateString ("double")) ;
    #ifdef GB_COMPLEX_TYPE
    else if (type == gb_complex_type)
    {
        return (mxCreateString ("complex")) ;
    }
    #endif
    else
    {
        ERROR ("unsupported type") ;
    }

    return (NULL) ;
}

