//------------------------------------------------------------------------------
// gbdisp: display a GraphBLAS matrix struct
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

    gb_usage (nargin == 3 && nargout == 0, "usage: gbdisp (C,cnz,level)") ;

    //--------------------------------------------------------------------------
    // get cnz and level
    //--------------------------------------------------------------------------

    int64_t cnz = (int64_t) mxGetScalar (pargin [1]) ;
    int level = (int) mxGetScalar (pargin [2]) ;

    #define LEN 256
    char s [LEN+1] ;
    if (cnz == 0)
    {
        sprintf (s, "no nonzeros") ;
    }
    else if (cnz == 1)
    {
        sprintf (s, "1 nonzero") ;
    }
    else
    {
        sprintf (s, GBd " nonzeros", cnz) ;
    }

    //--------------------------------------------------------------------------
    // print the GraphBLAS matrix
    //--------------------------------------------------------------------------

    GrB_Matrix C = gb_get_shallow (pargin [0]) ;
    OK (GxB_Matrix_fprint (C, s, level, NULL)) ;
    printf ("\n") ;
    OK (GrB_Matrix_free (&C)) ;
    GB_WRAPUP ;
}

