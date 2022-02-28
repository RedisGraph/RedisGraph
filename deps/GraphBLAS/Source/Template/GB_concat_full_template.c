//------------------------------------------------------------------------------
// GB_concat_full_template: concatenate an full tile into a full matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// If C and A are iso, this method is not used.

{

    //--------------------------------------------------------------------------
    // get C and the tile A
    //--------------------------------------------------------------------------

    const bool A_iso = A->iso ;
    const GB_CTYPE *restrict Ax = (GB_CTYPE *) A->x ;
          GB_CTYPE *restrict Cx = (GB_CTYPE *) C->x ;

    int64_t pA ;
    #pragma omp parallel for num_threads(A_nthreads) schedule(static)
    for (pA = 0 ; pA < anz ; pA++)
    { 
        int64_t i = pA % avlen ;
        int64_t j = pA / avlen ;
        int64_t iC = cistart + i ;
        int64_t jC = cvstart + j ;
        int64_t pC = iC + jC * cvlen ;
        // Cx [pC] = Ax [pA] ;
        GB_COPY (pC, pA, A_iso) ;
    }

    done = true ;
}

#undef GB_CTYPE

