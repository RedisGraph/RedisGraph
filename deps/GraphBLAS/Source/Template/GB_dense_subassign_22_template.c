//------------------------------------------------------------------------------
// GB_dense_subassign_22_template: C += b where C is dense and b is a scalar
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

{

    //--------------------------------------------------------------------------
    // get C
    //--------------------------------------------------------------------------

    GB_CTYPE *restrict Cx = (GB_CTYPE *) C->x ;
    const int64_t cnz = GB_nnz (C) ;
    ASSERT (!C->iso) ;

    //--------------------------------------------------------------------------
    // C += b where C is dense and b is a scalar
    //--------------------------------------------------------------------------

    int64_t pC ;
    #pragma omp parallel for num_threads(nthreads) schedule(static)
    for (pC = 0 ; pC < cnz ; pC++)
    { 
        GB_BINOP (GB_CX (pC), GB_CX (pC), bwork, 0, 0) ;
    }
}

