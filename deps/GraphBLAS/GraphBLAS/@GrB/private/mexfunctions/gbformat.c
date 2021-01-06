//------------------------------------------------------------------------------
// gbformat: get/set the matrix format to use in GraphBLAS
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// Usage

// fmt = gbformat ;                 get the global default format (row/col)
// fmt = gbformat (fmt) ;           set the global default format
// [f,sparsity] = gbformat (G) ;    get the format and sparsity of a matrix
//                                  (either GraphBLAS or MATLAB)

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

    gb_usage (nargin <= 1 && nargout <= 2,
        "usage: [f,s] = GrB.format, GrB.format (f), GrB.format (G)") ;

    //--------------------------------------------------------------------------
    // get/set the format
    //--------------------------------------------------------------------------

    GxB_Format_Value fmt = GxB_BY_COL ;
    int sparsity = GxB_AUTO_SPARSITY ;

    if (nargin == 0)
    { 

        //----------------------------------------------------------------------
        // format = GrB.format
        //----------------------------------------------------------------------

        // get the global format
        OK (GxB_Global_Option_get (GxB_FORMAT, &fmt)) ;

    }
    else // if (nargin == 1)
    {

        if (mxIsChar (pargin [0]))
        { 

            //------------------------------------------------------------------
            // GrB.format (format)
            //------------------------------------------------------------------

            // parse the format string
            int ignore ;
            bool ok = gb_mxstring_to_format (pargin [0], &fmt, &ignore) ;
            CHECK_ERROR (!ok, "invalid format") ;
            // set the global format
            OK (GxB_Global_Option_set (GxB_FORMAT, fmt)) ;

        }
        else if (mxIsStruct (pargin [0]))
        { 

            //------------------------------------------------------------------
            // GrB.format (G) for a GraphBLAS matrix G
            //------------------------------------------------------------------

            // get the type
            mxArray *mx_type = mxGetField (pargin [0], 0, "GraphBLASv4") ;
            CHECK_ERROR (mx_type == NULL, "invalid GraphBLASv4 struct") ;

            // get the row/column format of the input matrix G
            mxArray *opaque = mxGetField (pargin [0], 0, "s") ;
            CHECK_ERROR (opaque == NULL, "invalid GraphBLASv4 struct") ;
            int64_t *s = mxGetInt64s (opaque) ;
            bool is_csc = (bool) (s [6]) ;
            fmt = (is_csc) ? GxB_BY_COL : GxB_BY_ROW ;

            // get the current sparsity status of the input matrix G
            switch (mxGetNumberOfFields (pargin [0]))
            {
                case 3 : sparsity = GxB_FULL ;        break ;
                case 4 : sparsity = GxB_BITMAP ;      break ;
                case 5 : sparsity = GxB_SPARSE ;      break ;
                case 6 : sparsity = GxB_HYPERSPARSE ; break ;
                default: ERROR ("invalid GraphBLASv4 struct") ;
            }

        }
        else
        { 

            //------------------------------------------------------------------
            // GrB.format (A) for a MATLAB matrix A
            //------------------------------------------------------------------

            // MATLAB matrices are always stored by column
            fmt = GxB_BY_COL ;
            // MATLAB matrices are sparse or full, never hypersparse or bitmap
            sparsity = mxIsSparse (pargin [0]) ? GxB_SPARSE : GxB_FULL ;
        }
    }

    //--------------------------------------------------------------------------
    // return result
    //--------------------------------------------------------------------------

    pargout [0] = mxCreateString ((fmt == GxB_BY_ROW) ? "by row" : "by col") ;
    if (nargout > 1)
    { 
        char *s ;
        switch (sparsity)
        {
            case GxB_HYPERSPARSE : s = "hypersparse" ; break ;
            case GxB_SPARSE :      s = "sparse"      ; break ;
            case GxB_BITMAP :      s = "bitmap"      ; break ;
            case GxB_FULL :        s = "full"        ; break ;
            default :              s = ""            ; break ;
        }
        pargout [1] = mxCreateString (s) ;
    }

    GB_WRAPUP ;
}

