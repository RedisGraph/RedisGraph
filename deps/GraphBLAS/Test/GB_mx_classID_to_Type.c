//------------------------------------------------------------------------------
// GB_mx_classID_to_Type: get GraphBLAS type of the corresponding MATLAB class
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Given a MATLAB class ID, return the equivalent GraphBLAS type

#include "GB_mex.h"

GrB_Type GB_mx_classID_to_Type         // returns a GraphBLAS type
(
    const mxClassID xclass              // MATLAB class ID to convert
)
{

    GrB_Type xtype ;
    switch (xclass)
    {
        // all GraphBLAS built-in types are supported
        case mxLOGICAL_CLASS  : xtype = GrB_BOOL   ; break ;
        case mxINT8_CLASS     : xtype = GrB_INT8   ; break ;
        case mxUINT8_CLASS    : xtype = GrB_UINT8  ; break ;
        case mxINT16_CLASS    : xtype = GrB_INT16  ; break ;
        case mxUINT16_CLASS   : xtype = GrB_UINT16 ; break ;
        case mxINT32_CLASS    : xtype = GrB_INT32  ; break ;
        case mxUINT32_CLASS   : xtype = GrB_UINT32 ; break ;
        case mxINT64_CLASS    : xtype = GrB_INT64  ; break ;
        case mxUINT64_CLASS   : xtype = GrB_UINT64 ; break ;
        case mxSINGLE_CLASS   : xtype = GrB_FP32   ; break ;
        case mxDOUBLE_CLASS   : xtype = GrB_FP64   ; break ;
        case mxCELL_CLASS     :
        case mxCHAR_CLASS     :
        case mxUNKNOWN_CLASS  :
        case mxFUNCTION_CLASS :
        case mxSTRUCT_CLASS   :
        default               : xtype = NULL ;
    }
    return (xtype) ;
}

