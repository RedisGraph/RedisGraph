//------------------------------------------------------------------------------
// GB_mx_put_global: put the GraphBLAS status in MATLAB workspace
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
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
    // free the complex type and operators
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

    GrB_finalize ( ) ;

    //--------------------------------------------------------------------------
    // check nmalloc
    //--------------------------------------------------------------------------

    int64_t nmalloc = GB_Global_nmalloc_get ( ) ;
    if (nmalloc != 0)
    {
        int64_t inuse   = GB_Global_inuse_get ( ) ;
        int64_t maxused = GB_Global_maxused_get ( ) ;
        printf ("GraphBLAS nmalloc "GBd"! inuse "GBd" maxused "GBd"\n",
            nmalloc, inuse, maxused) ;
        mexErrMsgTxt ("memory leak!") ;
    }
}

