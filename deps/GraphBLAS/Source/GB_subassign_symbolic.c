//------------------------------------------------------------------------------
// GB_subassign_symbolic: S = C(I,J)
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// TODO: if I == GrB_ALL and J == GrB_ALL (pass in Ikind and Jkind), then
// let S be a purely shallow copy of C.  Let S->x be NULL, which denotes
// S->x [p] == p.  But S->i needs to be a deep, zombie-less copy of C->i,
// because C->i is changing (zombie status).  S->p and S->h can be shallow
// copies of C->p and C->h.

#include "GB_subassign_methods.h"
#include "GB_subref.h"

#undef  GB_FREE_ALL
#define GB_FREE_ALL GB_Matrix_free (Shandle) ;

GrB_Info GB_subassign_symbolic
(
    // output
    GrB_Matrix *Shandle,        // S = C(I,J), extracting the pattern not values
    // inputs, not modified:
    const GrB_Matrix C,         // matrix to extract the pattern of
    const GrB_Index *I,         // index list for S = C(I,J), or GrB_ALL, etc.
    const int64_t ni,           // length of I, or special
    const GrB_Index *J,         // index list for S = C(I,J), or GrB_ALL, etc.
    const int64_t nj,           // length of J, or special
    const bool S_must_not_be_jumbled,
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GrB_Info info ;
    ASSERT (!GB_IS_BITMAP (C)) ;    // the caller cannot tolerate C bitmap

    //--------------------------------------------------------------------------
    // extract the pattern: S = C(I,J) for S_Extraction method, and quick mask
    //--------------------------------------------------------------------------

    // S is a sparse int64_t matrix.  Its "values" are not numerical, but
    // indices into C.  For example, suppose 100 = I [5] and 200 = J [7].  Then
    // S(5,7) is the entry C(I(5),J(7)), and the value of S(5,7) is the
    // position in C that holds that particular entry C(100,200):
    // pC = S->x [...] gives the location of the value C->x [pC] and row index
    // 100 = C->i [pC], and pC will be between C->p [200] ... C->p [200+1]-1
    // if C is non-hypersparse.  If C is hyperparse then pC will be still
    // reside inside the vector jC, in the range C->p [k] ... C->p [k+1]-1,
    // if jC is the kth non-empty vector in the hyperlist of C.

    //--------------------------------------------------------------------------
    // extract symbolic structure S=C(I,J)
    //--------------------------------------------------------------------------

    // FUTURE::: if whole_C_matrix is true, then C(:,:) = ... and S == C,
    // except that S is zombie-free, read-only; and C collects zombies.

    // FUTURE:: the properties of I and J are already known, and thus do
    // not need to be recomputed by GB_subref.

    // S and C have the same CSR/CSC format.  S can be jumbled.  It is in
    // in the same hypersparse form as C (unless S is empty, in which case
    // it is always returned as hypersparse). This also checks I and J.
    (*Shandle) = NULL ;
    GB_OK (GB_subref (Shandle, C->is_csc, C, I, ni, J, nj, true, Context)) ;
    GrB_Matrix S = (*Shandle) ;
    ASSERT (GB_JUMBLED_OK (S)) ;    // GB_subref can return S as unsorted

    //--------------------------------------------------------------------------
    // sort S if requested
    //--------------------------------------------------------------------------

    if (S_must_not_be_jumbled)
    { 
        GB_MATRIX_WAIT_IF_JUMBLED (S) ; // but the caller requires S sorted
        ASSERT (!GB_JUMBLED (S)) ;
    }

    //--------------------------------------------------------------------------
    // check the result of S=C(I,J)
    //--------------------------------------------------------------------------

    #ifdef GB_DEBUG
    ASSERT_MATRIX_OK (C, "C for subref extraction", GB0) ;
    ASSERT_MATRIX_OK (S, "S for subref extraction", GB0) ;

    // since C is not bitmap, neither is S
    ASSERT (!GB_IS_BITMAP (S)) ;

    // GB_subref sorts its input matrix, so C is no longer jumbled
    ASSERT (!GB_JUMBLED (C)) ;

    // this body of code explains what S contains.
    // S is nI-by-nJ where nI = length (I) and nJ = length (J)

    int64_t nI, Icolon [3], nJ, Jcolon [3] ;
    int Ikind, Jkind ;
    GB_ijlength (I, ni, C->vlen, &nI, &Ikind, Icolon) ;
    GB_ijlength (J, nj, C->vdim, &nJ, &Jkind, Jcolon) ;

    // get S
    const int64_t *GB_RESTRICT Sp = S->p ;
    const int64_t *GB_RESTRICT Sh = S->h ;
    const int64_t *GB_RESTRICT Si = S->i ;
    const int64_t *GB_RESTRICT Sx = (int64_t *) S->x ;
    // for each vector of S
    for (int64_t k = 0 ; k < S->nvec ; k++)
    {
        // prepare to iterate over the entries of vector S(:,jnew)
        int64_t jnew = GBH (Sh, k) ;
        int64_t pS_start = GBP (Sp, k, S->vlen) ;
        int64_t pS_end   = GBP (Sp, k+1, S->vlen) ;
        // S (inew,jnew) corresponds to C (iC, jC) ;
        // jC = J [j] ; or J is a colon expression
        int64_t jC = GB_ijlist (J, jnew, Jkind, Jcolon) ;
        for (int64_t pS = pS_start ; pS < pS_end ; pS++)
        {
            // S (inew,jnew) is a pointer back into C (I(inew), J(jnew))
            int64_t inew = GBI (Si, pS, S->vlen) ;
            ASSERT (inew >= 0 && inew < nI) ;
            // iC = I [iA] ; or I is a colon expression
            int64_t iC = GB_ijlist (I, inew, Ikind, Icolon) ;
            int64_t p = Sx [pS] ;
            ASSERT (p >= 0 && p < GB_NNZ (C)) ;
            int64_t pC_start, pC_end, pleft = 0, pright = C->nvec-1 ;
            bool found = GB_lookup (C->h != NULL, C->h, C->p, C->vlen,
                &pleft, pright, jC, &pC_start, &pC_end) ;
            ASSERT (found) ;
            // If iC == I [inew] and jC == J [jnew], (or the equivaleent
            // for GB_ALL, GB_RANGE, GB_STRIDE) then A(inew,jnew) will be
            // assigned to C(iC,jC), and p = S(inew,jnew) gives the pointer
            // into C to where the entry (C(iC,jC) appears in C:
            ASSERT (pC_start <= p && p < pC_end) ;
            ASSERT (iC == GB_UNFLIP (GBI (C->i, p, C->vlen))) ;
        }
    }
    #endif

    //--------------------------------------------------------------------------
    // return result
    //--------------------------------------------------------------------------

    return (GrB_SUCCESS) ;
}

