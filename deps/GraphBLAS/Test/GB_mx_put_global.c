//------------------------------------------------------------------------------
// GB_mx_put_global: put the GraphBLAS status in MATLAB workspace
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB_mex.h"

void GB_mx_put_global
(
    bool cover,
    GrB_Desc_Value AxB_method_used
)
{

    //--------------------------------------------------------------------------
    // check nmalloc
    //--------------------------------------------------------------------------

    Complex_finalize ( ) ;

    //--------------------------------------------------------------------------
    // return the time to MATLAB, if it was computed
    //--------------------------------------------------------------------------

    GB_mx_put_time (AxB_method_used) ;

    //--------------------------------------------------------------------------
    // log the statement coverage
    //--------------------------------------------------------------------------

    #ifdef GBCOVER
    if (cover) GB_cover_put ( ) ;
    #endif

    //--------------------------------------------------------------------------
    // finalize GraphBLAS
    //--------------------------------------------------------------------------

    if (GB_Global.nmalloc != 0)
    {
        printf ("GraphBLAS nmalloc "GBd"! inuse "GBd" maxused "GBd"\n",
            GB_Global.nmalloc, GB_Global.inuse, GB_Global.maxused) ;
        mexErrMsgTxt ("memory leak!") ;
    }
}

