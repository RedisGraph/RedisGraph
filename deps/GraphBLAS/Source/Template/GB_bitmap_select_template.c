//------------------------------------------------------------------------------
// GB_bitmap_select_template: C=select(A,thunk) if A is bitmap or full
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// Ab and Cb can be aliased, if A is bitmap and the selection is done in-place.
// Ax and Cx are not aliased.

// TODO: If done in-place, Cx can be passed as NULL.  Then if A is not bitmap,
// C->b needs to be allocated, but not C->x.

// TODO: use a single GB_memcpy for the values, regardless of selectop,
// if no typecasting is being done.

{
    int8_t *Ab = A->b ;
    GB_ATYPE *restrict Ax = (GB_ATYPE *) A->x ;
    const int64_t avlen = A->vlen ;
    const int64_t avdim = A->vdim ;
    const size_t asize = A->type->size ;
    const int64_t anz = avlen * avdim ;
    int64_t pA, cnvals = 0 ;
    #pragma omp parallel for num_threads(nthreads) schedule(static) \
        reduction(+:cnvals)
    for (pA = 0 ; pA < anz ; pA++)
    { 
        int64_t i = pA % avlen ;
        int64_t j = pA / avlen ;
        #if defined ( GB_ENTRY_SELECTOR )
            // test the existence and value of A(i,j) 
            GB_TEST_VALUE_OF_ENTRY (keep, pA) ;
        #endif
        int8_t cb = GBB (Ab, pA) &&
        #if defined ( GB_ENTRY_SELECTOR )
            keep ;
        #elif defined ( GB_TRIL_SELECTOR )
            (j-i <= ithunk) ;
        #elif defined ( GB_TRIU_SELECTOR )
            (j-i >= ithunk) ;
        #elif defined ( GB_DIAG_SELECTOR )
            (j-i == ithunk) ;
        #elif defined ( GB_OFFDIAG_SELECTOR )
            (j-i != ithunk) ;
        #elif defined ( GB_ROWINDEX_SELECTOR )
            (i+ithunk != 0) ;
        #elif defined ( GB_COLINDEX_SELECTOR )
            (j+ithunk != 0) ;
        #elif defined ( GB_COLLE_SELECTOR )
            (j <= ithunk) ;
        #elif defined ( GB_COLGT_SELECTOR )
            (j > ithunk) ;
        #elif defined ( GB_ROWLE_SELECTOR )
            (i <= ithunk) ;
        #elif defined ( GB_ROWGT_SELECTOR )
            (i > ithunk) ;
        #endif
        Cb [pA] = cb ;
        cnvals += cb ;
        { 
            // Cx [pA] = Ax [pA]
            GB_SELECT_ENTRY (Cx, pA, Ax, pA) ;
        }
    }
    (*cnvals_handle) = cnvals ;
}

