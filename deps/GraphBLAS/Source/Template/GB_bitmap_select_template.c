//------------------------------------------------------------------------------
// GB_bitmap_select_template: C=select(A,thunk) if A is bitmap or full
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// Ab and Cb can be aliased, if A is bitmap and the selection is done in-place.
// Ax and Cx are not aliased.

// TODO: If done in-place, Cx can be passed as NULL.  Then if A is not bitmap,
// C->b needs to be allocated, but not C->x.

// the following macro is awkward but currently needed for the user_select op:
#undef  GBI
#define GBI(Ai,p,avlen) i

{
    int8_t *Ab = A->b ;
    GB_ATYPE *GB_RESTRICT Ax = A->x ;
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
            int8_t cb = GBB (Ab, pA) && GB_TEST_VALUE_OF_ENTRY (pA) ;
        #else
            // test the existence and position of A(i,j) 
            #if defined ( GB_TRIL_SELECTOR )
            int8_t cb = GBB (Ab, pA) && (j-i <= ithunk) ;
            #elif defined ( GB_TRIU_SELECTOR )
            int8_t cb = GBB (Ab, pA) && (j-i >= ithunk) ;
            #elif defined ( GB_DIAG_SELECTOR )
            int8_t cb = GBB (Ab, pA) && (j-i == ithunk) ;
            #elif defined ( GB_OFFDIAG_SELECTOR )
            int8_t cb = GBB (Ab, pA) && (j-i != ithunk) ;
            #else
            ASSERT (GB_DEAD_CODE) ;
            #endif
        #endif
        Cb [pA] = cb ;
        cnvals += cb ;
        // if (Cx != NULL)
        { 
            // Cx [pA] = Ax [pA]
            GB_SELECT_ENTRY (Cx, pA, Ax, pA) ;
        }
    }
    (*cnvals_handle)= cnvals ;
}

