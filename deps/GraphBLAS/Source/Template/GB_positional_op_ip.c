//------------------------------------------------------------------------------
// GB_positional_op_ip: C = positional_op (A), depending only on i
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// A can be jumbled.  If A is jumbled, so is C.

{

    //--------------------------------------------------------------------------
    // Cx = positional_op (A)
    //--------------------------------------------------------------------------

    int64_t p ;
    #pragma omp parallel for num_threads(nthreads) schedule(static)
    for (p = 0 ; p < anz ; p++)
    { 
        // GB_POSITION is either i or i+1
        int64_t i = GBI (Ai, p, avlen) ;
        Cx_int [p] = GB_POSITION ;
    }
}

#undef GB_POSITION

