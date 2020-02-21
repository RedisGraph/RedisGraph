//------------------------------------------------------------------------------
// GB_dense_subassign_22_template: C += x where C is dense and x is a scalar
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// All entries in C+=A are computed fully in parallel, using the same kind of
// parallelism as Template/GB_AxB_colscale.c.

{

    //--------------------------------------------------------------------------
    // get C
    //--------------------------------------------------------------------------

    GB_CTYPE *GB_RESTRICT Cx = C->x ;
    const int64_t cnz = GB_NNZ (C) ;

    //--------------------------------------------------------------------------
    // C += x where C is dense and x is a scalar
    //--------------------------------------------------------------------------

    int64_t pC ;
    #pragma omp parallel for num_threads(nthreads) schedule(static)
    for (pC = 0 ; pC < cnz ; pC++)
    { 
        GB_BINOP (GB_CX (pC), GB_CX (pC), ywork) ;
    }
}

