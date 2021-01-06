//------------------------------------------------------------------------------
// GB_AxB_dot4_meta:  C+=A'*B via dot products, where C is full
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// C+=A'*B where C is a dense matrix and computed in-place.  The monoid of the
// semiring matches the accum operator, and the type of C matches the ztype of
// accum.  That is, no typecasting can be done with C.

#define GB_DOT4

// cij += A(k,i) * B(k,j)
#undef  GB_DOT
#define GB_DOT(k,pA,pB)                                             \
{                                                                   \
    if (!cij_updated)                                               \
    {                                                               \
        cij_updated = true ;                                        \
        GB_GETC (cij, pC) ;                 /* cij = Cx [pC] */     \
    }                                                               \
    GB_GETA (aki, Ax, pA) ;                 /* aki = A(k,i) */      \
    GB_GETB (bkj, Bx, pB) ;                 /* bkj = B(k,j) */      \
    GB_MULTADD (cij, aki, bkj, i, k, j) ;   /* cij += aki * bkj */  \
    GB_DOT_TERMINAL (cij) ;         /* break if cij == terminal */  \
}

// C(i,j) = cij
#undef  GB_DOT_ALWAYS_SAVE_CIJ
#define GB_DOT_ALWAYS_SAVE_CIJ  \
{                               \
    GB_PUTC (cij, pC) ;         \
}

// save C(i,j) if it has been updated
#undef  GB_DOT_SAVE_CIJ
#define GB_DOT_SAVE_CIJ         \
{                               \
    if (cij_updated)            \
    {                           \
        GB_PUTC (cij, pC) ;     \
    }                           \
}

{ 

    //--------------------------------------------------------------------------
    // get A, B, and C
    //--------------------------------------------------------------------------

    GB_CTYPE *GB_RESTRICT Cx = (GB_CTYPE *) C->x ;
    const int64_t cvlen = C->vlen ;

    const int64_t  *GB_RESTRICT Bp = B->p ;
    const int8_t   *GB_RESTRICT Bb = B->b ;
    const int64_t  *GB_RESTRICT Bh = B->h ;
    const int64_t  *GB_RESTRICT Bi = B->i ;
    const GB_BTYPE *GB_RESTRICT Bx = (GB_BTYPE *) (B_is_pattern ? NULL : B->x) ;
    const int64_t vlen = B->vlen ;
    const bool B_is_hyper = GB_IS_HYPERSPARSE (B) ;
    const bool B_is_bitmap = GB_IS_BITMAP (B) ;
    const bool B_is_sparse = GB_IS_SPARSE (B) ;

    const int64_t  *GB_RESTRICT Ap = A->p ;
    const int8_t   *GB_RESTRICT Ab = A->b ;
    const int64_t  *GB_RESTRICT Ah = A->h ;
    const int64_t  *GB_RESTRICT Ai = A->i ;
    const GB_ATYPE *GB_RESTRICT Ax = (GB_ATYPE *) (A_is_pattern ? NULL : A->x) ;
    ASSERT (A->vlen == B->vlen) ;
    const bool A_is_hyper = GB_IS_HYPERSPARSE (A) ;
    const bool A_is_bitmap = GB_IS_BITMAP (A) ;
    const bool A_is_sparse = GB_IS_SPARSE (A) ;

    int ntasks = naslice * nbslice ;

    //--------------------------------------------------------------------------
    // C += A'*B
    //--------------------------------------------------------------------------

    #include "GB_meta16_factory.c"
}

#undef GB_DOT_ALWAYS_SAVE_CIJ
#undef GB_DOT_SAVE_CIJ

#undef GB_DOT4

