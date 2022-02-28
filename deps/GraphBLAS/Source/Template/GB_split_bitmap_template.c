//------------------------------------------------------------------------------
// GB_split_bitmap_template: split a bitmap matrix into a bitmap tile
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

{

    //--------------------------------------------------------------------------
    // get C and the tile A
    //--------------------------------------------------------------------------

    #ifndef GB_ISO_SPLIT
    const GB_CTYPE *restrict Ax = (GB_CTYPE *) A->x ;
    GB_CTYPE *restrict Cx = (GB_CTYPE *) C->x ;
    #endif

    int64_t pC ;
    #pragma omp parallel for num_threads(C_nthreads) schedule(static) \
        reduction(+:cnz)
    for (pC = 0 ; pC < cnzmax ; pC++)
    {
        int64_t i = pC % cvlen ;
        int64_t j = pC / cvlen ;
        int64_t iA = aistart + i ;
        int64_t jA = avstart + j ;
        int64_t pA = iA + jA * avlen ;
        Cb [pC] = Ab [pA] ;
        if (Ab [pA])
        { 
            // Cx [pC] = Ax [pA] ;
            GB_COPY (pC, pA) ;
            cnz++ ;
        }
    }

    done = true ;
}

#undef GB_CTYPE
#undef GB_ISO_SPLIT

