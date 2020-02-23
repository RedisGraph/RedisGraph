//------------------------------------------------------------------------------
// gbformat: get/set the matrix format to use in GraphBLAS
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

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

    gb_usage (nargin <= 1 && nargout <= 1,
        "usage: f = GrB.format or GrB.format (f)") ;

    //--------------------------------------------------------------------------
    // get/set the format
    //--------------------------------------------------------------------------

    GxB_Format_Value fmt ;

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

            // set the global format
            fmt = gb_mxstring_to_format (pargin [0]) ;
            OK (GxB_Global_Option_set (GxB_FORMAT, fmt)) ;

        }
        else
        { 

            //------------------------------------------------------------------
            // GrB.format (G)
            //------------------------------------------------------------------

            // get the format of the input matrix G
            GrB_Matrix G = gb_get_shallow (pargin [0]) ;
            OK (GxB_Matrix_Option_get (G, GxB_FORMAT, &fmt)) ;
            OK (GrB_Matrix_free (&G)) ;
        }
    }

    //--------------------------------------------------------------------------
    // return result
    //--------------------------------------------------------------------------

    if (fmt == GxB_BY_ROW)
    { 
        pargout [0] = mxCreateString ("by row") ;
    }
    else
    { 
        pargout [0] = mxCreateString ("by col") ;
    }
    GB_WRAPUP ;
}

