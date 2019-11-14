//------------------------------------------------------------------------------
// gb_at_exit: function to call if GraphBLAS is cleared
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Each mexFunction in the MATLAB interface to GraphBLAS registers this
// function via mexAtExit, so that if the mexFunction is cleared, GraphBLAS is
// finalized.  The global flag that keeps track of whether or not GrB_init has
// been called is reset to false, to all further use of GraphBLAS in the same
// MATLAB session.

#include "gb_matlab.h"

void gb_at_exit (void)
{

    //--------------------------------------------------------------------------
    // finalize GraphBLAS, unless it's already been done
    //--------------------------------------------------------------------------

    if (GB_Global_GrB_init_called_get ( ))
    { 

        //----------------------------------------------------------------------
        // finish GraphBLAS
        //----------------------------------------------------------------------

        GrB_finalize ( ) ;

    }

    //--------------------------------------------------------------------------
    // allow GraphBLAS to be called again
    //--------------------------------------------------------------------------

    GB_Global_GrB_init_called_set (false) ;
}

