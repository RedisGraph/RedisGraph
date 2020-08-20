//------------------------------------------------------------------------------
// GB_AxB_dot2_meta: C=A'*B or C<!M>=A'*B via dot productes
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

{

    //--------------------------------------------------------------------------
    // get B and C
    //--------------------------------------------------------------------------

    #if defined ( GB_PHASE_2_OF_2)
    int64_t  *GB_RESTRICT Cp = C->p ;
    int64_t  *GB_RESTRICT Ci = C->i ;
    GB_CTYPE *GB_RESTRICT Cx = C->x ;
    const GB_BTYPE *GB_RESTRICT Bx = B_is_pattern ? NULL : B->x ;
    #endif

    const int64_t *GB_RESTRICT Bi = B->i ;
    int64_t bvlen = B->vlen ;

    // create the iterator for B.  Since the iterator is a read-only object
    // after initialization with GBI1_init, it can be shared by all threads.
    GBI_single_iterator Iter ;
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

        const int64_t *GB_RESTRICT Mp = M->p ;
        const int64_t *GB_RESTRICT Mh = M->h ;
        const int64_t *GB_RESTRICT Mi = M->i ;
        const GB_void *GB_RESTRICT Mx = (Mask_struct ? NULL : (M->x)) ;
        size_t msize = M->type->size ;
        const int64_t mnvec = M->nvec ;
        bool M_is_hyper = GB_IS_HYPER (M) ;

        // C<!M> = A'*B via dot products
        #include "GB_AxB_dot2_compmask.c"
    }

}

