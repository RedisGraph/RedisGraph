//------------------------------------------------------------------------------
// GB_AxB_dot2_meta: C=A'*B or C<!M>=A'*B via dot productes
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

{

    //--------------------------------------------------------------------------
    // get B and C
    //--------------------------------------------------------------------------

    #if defined ( GB_PHASE_2_OF_2)
    int64_t  *restrict Cp = C->p ;
    int64_t  *restrict Ci = C->i ;
    GB_CTYPE *restrict Cx = C->x ;
    const GB_BTYPE *restrict Bx = B_is_pattern ? NULL : B->x ;
    #endif

    const int64_t *restrict Bi = B->i ;
    int64_t bvlen = B->vlen ;

    // create the iterator for B.  Since the iterator is a read-only object
    // after initialization with GBI1_init, it can be shared by all threads.
    GBI_single_iterator Iter ;
    int64_t B_slice [nbslice+1] ;
    GB_pslice (B_slice, /* B */ B->p, B->nvec, nbslice) ;
    GBI1_init (&Iter, B) ;

    //--------------------------------------------------------------------------
    // C=A'*B or C<!M>=A'*B via dot products
    //--------------------------------------------------------------------------

    if (M == NULL)
    { 

        // C = A'*B via dot products
        #include "GB_AxB_dot2_nomask.c"

    }
    else
    {

        //----------------------------------------------------------------------
        // get M
        //----------------------------------------------------------------------

        const int64_t *restrict Mp = M->p ;
        const int64_t *restrict Mh = M->h ;
        const int64_t *restrict Mi = M->i ;
        const GB_void *restrict Mx = M->x ;
        GB_cast_function cast_M = GB_cast_factory (GB_BOOL_code, M->type->code);
        size_t msize = M->type->size ;
        const int64_t mnvec = M->nvec ;
        bool M_is_hyper = GB_IS_HYPER (M) ;

        // C<!M> = A'*B via dot products
        #include "GB_AxB_dot2_compmask.c"
    }
}

