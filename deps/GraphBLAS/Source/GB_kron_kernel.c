//------------------------------------------------------------------------------
// GB_kron_kernel: Kronecker product, C = kron (A,B)
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// C = kron(A,B) where op determines the binary multiplier to use.  The type of
// A and B are compatible with the x and y inputs of z=op(x,y), but can be
// different.  The type of C is the type of z.  C is hypersparse if either A
// or B are hypersparse.

#include "GB.h"

GrB_Info GB_kron_kernel             // C = kron (A,B)
(
    GrB_Matrix *Chandle,            // output matrix
    const bool C_is_csc,            // desired format of C
    const GrB_BinaryOp op,          // multiply operator
    const GrB_Matrix A,             // input matrix
    const GrB_Matrix B,             // input matrix
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (Chandle != NULL) ;
    ASSERT_OK (GB_check (A, "A for kron (A,B)", GB0)) ;
    ASSERT_OK (GB_check (B, "B for kron (A,B)", GB0)) ;
    ASSERT_OK (GB_check (op, "op for kron (A,B)", GB0)) ;
    ASSERT (!GB_PENDING (A)) ; ASSERT (!GB_ZOMBIES (A)) ;
    ASSERT (!GB_PENDING (B)) ; ASSERT (!GB_ZOMBIES (B)) ;

    //--------------------------------------------------------------------------
    // get inputs
    //--------------------------------------------------------------------------

    GrB_Info info ;

    (*Chandle) = NULL ;

    const int64_t *restrict Ai = A->i ;
    const GB_void *restrict Ax = A->x ;
    const int64_t asize = A->type->size ;

    const int64_t *restrict Bi = B->i ;
    const GB_void *restrict Bx = B->x ;
    const int64_t bsize = B->type->size ;
    const int64_t bvlen = B->vlen ;
    const int64_t bvdim = B->vdim ;

    //--------------------------------------------------------------------------
    // allocate the output matrix C
    //--------------------------------------------------------------------------

    // C has the same type as z for the multiply operator, z=op(x,y)

    GrB_Index cvlen, cvdim, cnzmax ;

    bool ok = GB_Index_multiply (&cvlen, A->vlen, bvlen) ;
    ok = ok & GB_Index_multiply (&cvdim, A->vdim, bvdim) ;
    ok = ok & GB_Index_multiply (&cnzmax, GB_NNZ (A), GB_NNZ (B)) ;
    ASSERT (ok) ;

    // C is hypersparse if either A or B are hypersparse
    bool C_is_hyper = (cvdim > 1) && (A->is_hyper || B->is_hyper) ;

    GrB_Matrix C = NULL ;           // allocate a new header for C
    GB_CREATE (&C, op->ztype, (int64_t) cvlen, (int64_t) cvdim, GB_Ap_calloc,
        C_is_csc, GB_SAME_HYPER_AS (C_is_hyper), B->hyper_ratio,
        A->nvec_nonempty * B->nvec_nonempty, cnzmax, true) ;
    if (info != GrB_SUCCESS)
    { 
        // out of memory
        return (info) ;
    }

    //--------------------------------------------------------------------------
    // get C and workspace
    //--------------------------------------------------------------------------

    int64_t *restrict Ci = C->i ;
    GB_void *restrict Cx = C->x ;
    const int64_t csize = C->type->size ;

    char awork [asize] ;
    char bwork [bsize] ;

    GxB_binary_function fmult = op->function ;

    GB_cast_function
        cast_A = GB_cast_factory (op->xtype->code, A->type->code),
        cast_B = GB_cast_factory (op->ytype->code, B->type->code) ;

    // FUTURE: this could be done faster with built-in types and operators

    //--------------------------------------------------------------------------
    // C = kron (A,B)
    //--------------------------------------------------------------------------

    int64_t cnz, cnz_last, cj_last ;
    GB_jstartup (C, &cj_last, &cnz, &cnz_last) ;

    GBI_iterator A_iter ;
    for (GB_each_vector (A_iter, A))
    {

        int64_t GBI1_initj (A_iter, aj, pA_start, pA_end) ;
        int64_t ajblock = aj * bvdim ;

        GBI_iterator B_iter ;
        for (GB_each_vector (B_iter, B))
        {

            int64_t GBI1_initj (B_iter, bj, pB_start, pB_end) ;
            int64_t cj = ajblock + bj ;

            for (int64_t pa = pA_start ; pa < pA_end ; pa++)
            {
                // awork = A(ai,aj), typecasted to op->xtype
                int64_t ai = Ai [pa] ;
                int64_t aiblock = ai * bvlen ;
                cast_A (awork, Ax +(pa*asize), asize) ;

                for (int64_t pb = pB_start ; pb < pB_end ; pb++)
                { 
                    // bwork = B(bi,bj), typecasted to op->ytype
                    int64_t bi = Bi [pb] ;
                    cast_B (bwork, Bx +(pb*bsize), bsize) ;

                    // C(ci,cj) = A(ai,aj) * B(bi,bj)
                    int64_t ci = aiblock + bi ;
                    Ci [cnz] = ci ;
                    fmult (Cx +(cnz*csize), awork, bwork) ;
                    cnz++ ;
                }
            }

            // cannot fail since C->plen is the upper bound: the product of
            // number of non empty vectors of A and B
            GrB_Info info = GB_jappend (C, cj, &cj_last, cnz, &cnz_last,
                Context) ;
            ASSERT (info == GrB_SUCCESS) ;
            #if 0
            // if it could fail, do this:
            if (info != GrB_SUCCESS) { GB_MATRIX_FREE (&C) ; return (info) ; }
            #endif
        }
    }

    GB_jwrapup (C, cj_last, cnz) ;

    //--------------------------------------------------------------------------
    // return result
    //--------------------------------------------------------------------------

    ASSERT (cnz == GB_NNZ (A) * GB_NNZ (B)) ;
    ASSERT_OK (GB_check (C, "C=kron(A,B)", GB0)) ;
    (*Chandle) = C ;
    return (GrB_SUCCESS) ;
}

