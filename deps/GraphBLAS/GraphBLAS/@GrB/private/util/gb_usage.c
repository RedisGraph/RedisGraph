//------------------------------------------------------------------------------
// gb_usage: check usage and make sure GrB.init has been called
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "gb_matlab.h"
#include "GB_printf.h"

void gb_usage       // check usage and make sure GrB.init has been called
(
    bool ok,                // if false, then usage is not correct
    const char *message     // error message if usage is not correct
)
{

    //--------------------------------------------------------------------------
    // make sure GrB.init has been called
    //--------------------------------------------------------------------------

    if (!GB_Global_GrB_init_called_get ( ))
    {

        //----------------------------------------------------------------------
        // initialize GraphBLAS
        //----------------------------------------------------------------------

        GB_printf_function = mexPrintf ;

        OK (GxB_init (GrB_NONBLOCKING, mxMalloc, mxCalloc, mxRealloc, mxFree,
            false)) ;

        // MATLAB matrices are stored by column
        OK (GxB_Global_Option_set (GxB_FORMAT, GxB_BY_COL)) ;

        // print short format by default
        GB_Global_print_format_set (1) ;

        // print 1-based indices
        GB_Global_print_one_based_set (true) ;
    }

    //--------------------------------------------------------------------------
    // check usage
    //--------------------------------------------------------------------------

    if (!ok)
    {
        ERROR (message) ;
    }

    //--------------------------------------------------------------------------
    // get test coverage
    //--------------------------------------------------------------------------

    #ifdef GBCOV
    gbcov_get ( ) ;
    #endif
}

