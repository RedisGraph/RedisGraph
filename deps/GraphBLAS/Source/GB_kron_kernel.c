//------------------------------------------------------------------------------
// GB_kron_kernel: Kronecker product, C = kron (A,B)
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// C = kron(A,B) where op determines the binary multiplier to use.  The type of
// A and B are compatible with the x and y inputs of z=op(x,y), but can be
// different.  The type of C always matches the type of z.  The caller
// (GB_kron) has been already allocated C with the right size and nzmax, so
// this function cannot fail.

#include "GB.h"

void GB_kron_kernel                 // C = kron (A,B)
(
    GrB_Matrix C,                   // output matrix
    const GrB_BinaryOp op,          // multiply operator
    const GrB_Matrix A,             // input matrix
    const GrB_Matrix B              // input matrix
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT_OK (GB_check (A, "A for kron (A,B)", 0)) ;
    ASSERT_OK (GB_check (B, "B for kron (A,B)", 0)) ;
    ASSERT_OK (GB_check (C, "C for kron (A,B)", 0)) ;
    ASSERT (!PENDING (A)) ; ASSERT (!ZOMBIES (A)) ;
    ASSERT (!PENDING (B)) ; ASSERT (!ZOMBIES (B)) ;
    ASSERT (!PENDING (C)) ; ASSERT (!ZOMBIES (C)) ;

    //--------------------------------------------------------------------------
    // get inputs
    //--------------------------------------------------------------------------

    const int64_t *restrict Ap = A->p ;
    const int64_t *restrict Ai = A->i ;
    const void    *restrict Ax = A->x ;
    const int64_t asize = A->type->size ;
    const int64_t anrows = A->nrows ;
    const int64_t ancols = A->ncols ;

    const int64_t *restrict Bp = B->p ;
    const int64_t *restrict Bi = B->i ;
    const void    *restrict Bx = B->x ;
    const int64_t bsize = B->type->size ;
    const int64_t bnrows = B->nrows ;
    const int64_t bncols = B->ncols ;

    int64_t *restrict Cp = C->p ;
    int64_t *restrict Ci = C->i ;
    void    *restrict Cx = C->x ;
    const int64_t csize = C->type->size ;
    GrB_Index cncols, cnz ;

    #ifndef NDEBUG
    // check the dimensions, type, and nnz of C
    GrB_Index cnrows ;
    bool ok = GB_Index_multiply (&cnrows, anrows,  bnrows) ;
    ok = ok & GB_Index_multiply (&cncols, ancols,  bncols) ;
    ok = ok & GB_Index_multiply (&cnz,    NNZ (A), NNZ (B)) ;
    ASSERT (ok) ;
    ASSERT (cnrows == C->nrows) ;
    ASSERT (cncols == C->ncols) ;
    ASSERT (C->nzmax >= cnz) ;
    ASSERT (C->type == op->ztype) ;
    #endif

    cncols = C->ncols ;

    GB_binary_function fmult = op->function ;

    char awork [asize] ;
    char bwork [bsize] ;

    cnz = 0 ;

    //--------------------------------------------------------------------------
    // C = kron (A,B)
    //--------------------------------------------------------------------------

    GB_cast_function
        cast_A = GB_cast_factory (op->xtype->code, A->type->code),
        cast_B = GB_cast_factory (op->ytype->code, B->type->code) ;

    // FUTURE: this could be done faster with built-in types and operators

    for (int64_t cj = 0 ; cj < cncols ; cj++)
    {
        // log the start of column C(:,cj)
        Cp [cj] = cnz ;

        int64_t aj = cj / bncols ;
        int64_t bj = cj % bncols ;

        for (int64_t pa = Ap [aj] ; pa < Ap [aj+1] ; pa++)
        {
            // awork = A(ai,aj), typecasted to op->xtype
            int64_t ai = Ai [pa] ;
            int64_t aiblock = ai * bnrows ;
            cast_A (awork, Ax + (pa*asize), asize) ;

            for (int64_t pb = Bp [bj] ; pb < Bp [bj+1] ; pb++)
            {
                // bwork = B(bi,bj), typecasted to op->ytype
                int64_t bi = Bi [pb] ;
                cast_B (bwork, Bx + (pb*bsize), bsize) ;

                // C(ci,cj) = A(ai,aj) * B(bi,bj)
                int64_t ci = aiblock + bi ;
                Ci [cnz] = ci ;
                fmult (Cx + (cnz*csize), awork, bwork) ;
                cnz++ ;
            }
        }
    }

    Cp [cncols] = cnz ;
    C->magic = MAGIC ;

    ASSERT (cnz == NNZ (A) * NNZ (B)) ;
    ASSERT_OK (GB_check (C, "C=kron(A,B)", 0)) ;
}

