//------------------------------------------------------------------------------
// gbtype: type of a GraphBLAS matrix struct, or any MATLAB variable
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// The input may be any MATLAB variable.  If it is a GraphBLAS G.opaque struct,
// then its internal type is returned.

#include "gb_matlab.h"

void mexFunction
(
    int nargout,
    mxArray *pargout [ ],
    int nargin,
    const mxArray *pargin [ ]
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    gb_usage (nargin == 1 && nargout <= 1, "usage: type = GrB.type (X)") ;

    //--------------------------------------------------------------------------
    // get the type of the matrix
    //--------------------------------------------------------------------------

    mxArray *c = NULL ;
    mxClassID class = mxGetClassID (pargin [0]) ;

    if (class == mxSTRUCT_CLASS)
    {
        mxArray *mx_type = mxGetField (pargin [0], 0, "GraphBLAS") ;
        if (mx_type != NULL)
        { 
            // X is a GraphBLAS G.opaque struct; get its type
            GrB_Type type = gb_mxstring_to_type (mx_type) ;
                 if (type == GrB_BOOL  ) c = mxCreateString ("logical") ;
            else if (type == GrB_INT8  ) c = mxCreateString ("int8") ;
            else if (type == GrB_INT16 ) c = mxCreateString ("int16") ;
            else if (type == GrB_INT32 ) c = mxCreateString ("int32") ;
            else if (type == GrB_INT64 ) c = mxCreateString ("int64") ;
            else if (type == GrB_UINT8 ) c = mxCreateString ("uint8") ;
            else if (type == GrB_UINT16) c = mxCreateString ("uint16") ;
            else if (type == GrB_UINT32) c = mxCreateString ("uint32") ;
            else if (type == GrB_UINT64) c = mxCreateString ("uint64") ;
            else if (type == GrB_FP32  ) c = mxCreateString ("single") ;
            else if (type == GrB_FP64  ) c = mxCreateString ("double") ;
            #ifdef GB_COMPLEX_TYPE
            else if (type == gb_complex_type) c = mxCreateString ("complex") ;
            #endif
            else ERROR ("unknown GraphBLAS type") ;
        }
    }

    if (c == NULL)
    {
        // if c is still NULL, then it is not a GraphBLAS opaque struct
        switch (class)
        {
            // a MATLAB sparse or dense matrix, valid for G = GrB (X), or
            // for inputs to any GrB.method.
            case mxLOGICAL_CLASS  : c = mxCreateString ("logical") ;  break ;
            case mxINT8_CLASS     : c = mxCreateString ("int8") ;     break ;
            case mxINT16_CLASS    : c = mxCreateString ("int16") ;    break ;
            case mxINT32_CLASS    : c = mxCreateString ("int32") ;    break ;
            case mxINT64_CLASS    : c = mxCreateString ("int64") ;    break ;
            case mxUINT8_CLASS    : c = mxCreateString ("uint8") ;    break ;
            case mxUINT16_CLASS   : c = mxCreateString ("uint16") ;   break ;
            case mxUINT32_CLASS   : c = mxCreateString ("uint32") ;   break ;
            case mxUINT64_CLASS   : c = mxCreateString ("uint64") ;   break ;
            case mxSINGLE_CLASS   : c = mxCreateString ("single") ;   break ;
            case mxDOUBLE_CLASS   : c = mxCreateString ("double") ;   break ;

            // a MATLAB struct, cell, char, void, function, or unknown
            case mxSTRUCT_CLASS   : c = mxCreateString ("struct") ;   break ;
            case mxCELL_CLASS     : c = mxCreateString ("cell") ;     break ;
            case mxCHAR_CLASS     : c = mxCreateString ("char") ;     break ;
            case mxVOID_CLASS     : c = mxCreateString ("void") ;     break ;
            case mxFUNCTION_CLASS : c = mxCreateString ("function_handle") ;
                                   break ;
            case mxUNKNOWN_CLASS  :
            default               : c = mxCreateString ("unknown") ;  break ;
        }
    }

    //--------------------------------------------------------------------------
    // return the result
    //--------------------------------------------------------------------------

    pargout [0] = c ;
    GB_WRAPUP ;
}

