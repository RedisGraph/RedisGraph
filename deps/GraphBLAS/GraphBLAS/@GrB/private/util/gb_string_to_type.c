//------------------------------------------------------------------------------
// gb_string_to_type: return the GraphBLAS type from a string
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "gb_matlab.h"

GrB_Type gb_string_to_type      // return the GrB_Type from a string
(
    const char *classname
)
{ 

    if (MATCH (classname, "logical" )) return (GrB_BOOL) ;
    if (MATCH (classname, "int8"    )) return (GrB_INT8) ;
    if (MATCH (classname, "int16"   )) return (GrB_INT16) ;
    if (MATCH (classname, "int32"   )) return (GrB_INT32) ;
    if (MATCH (classname, "int64"   )) return (GrB_INT64) ;
    if (MATCH (classname, "uint8"   )) return (GrB_UINT8) ;
    if (MATCH (classname, "uint16"  )) return (GrB_UINT16) ;
    if (MATCH (classname, "uint32"  )) return (GrB_UINT32) ;
    if (MATCH (classname, "uint64"  )) return (GrB_UINT64) ;
    if (MATCH (classname, "single"  )) return (GrB_FP32) ;
    if (MATCH (classname, "double"  )) return (GrB_FP64) ;
    #ifdef GB_COMPLEX_TYPE
    if (MATCH (classname, "complex" )) return (gb_complex_type) ;
    #endif

    // The string is not a type, but this is not an error here.  For example,
    // G = GrB (m,n,'double','by row') queries both its string input arguments
    // with gb_mxstring_to_type and gb_mxstring_to_format, to parse its inputs.
    return (NULL) ;
}

