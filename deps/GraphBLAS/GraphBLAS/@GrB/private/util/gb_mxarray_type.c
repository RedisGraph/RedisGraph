//------------------------------------------------------------------------------
// gb_mxarray_type: return the GraphBLAS type of a MATLAB matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "gb_matlab.h"

GrB_Type gb_mxarray_type        // return the GrB_Type of a MATLAB matrix
(
    const mxArray *X
)
{

    GrB_Type type ;

    switch (mxGetClassID (X))
    {
        case mxLOGICAL_CLASS  : type = GrB_BOOL     ; break ;
        case mxINT8_CLASS     : type = GrB_INT8     ; break ;
        case mxINT16_CLASS    : type = GrB_INT16    ; break ;
        case mxINT32_CLASS    : type = GrB_INT32    ; break ;
        case mxINT64_CLASS    : type = GrB_INT64    ; break ;
        case mxUINT8_CLASS    : type = GrB_UINT8    ; break ;
        case mxUINT16_CLASS   : type = GrB_UINT16   ; break ;
        case mxUINT32_CLASS   : type = GrB_UINT32   ; break ;
        case mxUINT64_CLASS   : type = GrB_UINT64   ; break ;
        case mxSINGLE_CLASS   : type = GrB_FP32     ; break ;
        case mxDOUBLE_CLASS   : type = GrB_FP64     ; break ;

        case mxVOID_CLASS     :
        case mxCHAR_CLASS     :
        case mxUNKNOWN_CLASS  :
        case mxCELL_CLASS     :
        case mxSTRUCT_CLASS   :
        case mxFUNCTION_CLASS :
        default               : ERROR ("invalid type") ;
    }

    if (mxIsComplex (X))
    { 
        #ifdef GB_COMPLEX_TYPE
        type = gb_complex_type ;
        #else
        ERROR ("complex not yet supported") ;
        #endif
    }

    return (type) ;
}

