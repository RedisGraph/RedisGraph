//------------------------------------------------------------------------------
// GB_kroner: Kronecker product, C = kron (A,B)
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// C = kron(A,B) where op determines the binary multiplier to use.  The type of
// A and B are compatible with the x and y inputs of z=op(x,y), but can be
// different.  The type of C is the type of z.  C is hypersparse if either A
// or B are hypersparse.

// FUTURE: GB_kron would be faster with built-in types and operators.

// FUTURE: at most one thread is used for each vector of C=kron(A,B).  The
// matrix C is normally very large, but if both A and B are n-by-1, then C is
// n^2-by-1 and only a single thread is used.  A better method for this case
// would construct vectors of C in parallel.

// FUTURE: each vector C(:,k) takes O(nnz(C(:,k))) work, but this is not
// accounted for in the parallel load-balancing.

#include "GB_kron.h"

GrB_Info GB_kroner                  // C = kron (A,B)
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

    const int64_t *restrict Ap = A->p ;
    const int64_t *restrict Ah = A->h ;
    const int64_t *restrict Ai = A->i ;
    const GB_void *restrict Ax = A->x ;
    const int64_t asize = A->type->size ;
    const int64_t avlen = A->vlen ;
    const int64_t avdim = A->vdim ;
    int64_t anvec = A->nvec ;
    int64_t anz = GB_NNZ (A) ;

    const int64_t *restrict Bp = B->p ;
    const int64_t *restrict Bh = B->h ;
    const int64_t *restrict Bi = B->i ;
    const GB_void *restrict Bx = B->x ;
    const int64_t bsize = B->type->size ;
    const int64_t bvlen = B->vlen ;
    const int64_t bvdim = B->vdim ;
    int64_t bnvec = B->nvec ;
    int64_t bnz = GB_NNZ (B) ;

    //--------------------------------------------------------------------------
    // determine the number of threads to use
    //--------------------------------------------------------------------------

    double work = ((double) anz) * ((double) bnz)
                + (((double) anvec) * ((double) bnvec)) ;

    GB_GET_NTHREADS_MAX (nthreads_max, chunk, Context) ;
    int nthreads = GB_nthreads (work, chunk, nthreads_max) ;

    //--------------------------------------------------------------------------
    // allocate the output matrix C
    //--------------------------------------------------------------------------

    // C has the same type as z for the multiply operator, z=op(x,y)

    GrB_Index cvlen, cvdim, cnzmax, cnvec ;

    bool ok = GB_Index_multiply (&cvlen, avlen, bvlen) ;
    ok = ok & GB_Index_multiply (&cvdim, avdim, bvdim) ;
    ok = ok & GB_Index_multiply (&cnzmax, anz, bnz) ;
    ok = ok & GB_Index_multiply (&cnvec, anvec, bnvec) ;
    ASSERT (ok) ;

    // C is hypersparse if either A or B are hypersparse
    bool C_is_hyper = (cvdim > 1) && (A->is_hyper || B->is_hyper) ;

    GrB_Matrix C = NULL ;           // allocate a new header for C
    GB_CREATE (&C, op->ztype, (int64_t) cvlen, (int64_t) cvdim, GB_Ap_malloc,
        C_is_csc, GB_SAME_HYPER_AS (C_is_hyper), B->hyper_ratio, cnvec,
        cnzmax, true, Context) ;
    if (info != GrB_SUCCESS)
    { 
        // out of memory
        return (info) ;
    }

    //--------------------------------------------------------------------------
    // get C
    //--------------------------------------------------------------------------

    int64_t *restrict Cp = C->p ;
    int64_t *restrict Ch = C->h ;
    int64_t *restrict Ci = C->i ;
    GB_void *restrict Cx = C->x ;
    const int64_t csize = C->type->size ;

    GxB_binary_function fmult = op->function ;

    GB_cast_function
        cast_A = GB_cast_factory (op->xtype->code, A->type->code),
        cast_B = GB_cast_factory (op->ytype->code, B->type->code) ;

    //--------------------------------------------------------------------------
    // compute the column counts of C, and C->h if C is hypersparse
    //--------------------------------------------------------------------------

    #pragma omp parallel for num_threads(nthreads) schedule(guided) collapse(2)
    for (int64_t kA = 0 ; kA < anvec ; kA++)
    {
        for (int64_t kB = 0 ; kB < bnvec ; kB++)
        {
            // get A(:,jA), the (kA)th vector of A
            int64_t jA = (Ah == NULL) ? kA : Ah [kA] ;
            int64_t aknz = Ap [kA+1] - Ap [kA] ;
            // get B(:,jB), the (kB)th vector of B
            int64_t jB = (Bh == NULL) ? kB : Bh [kB] ;
            int64_t bknz = Bp [kB+1] - Bp [kB] ;
            // determine # entries in C(:,jC), the (kC)th vector of C
            int64_t kC = kA * bnvec + kB ;
            Cp [kC] = aknz * bknz ;
            if (C_is_hyper)
            { 
                Ch [kC] = jA * bvdim + jB ;
            }
        }
    }

    //--------------------------------------------------------------------------
    // replace Cp with its cumulative sum
    //--------------------------------------------------------------------------

    GB_cumsum (Cp, cnvec, &(C->nvec_nonempty), nthreads) ;
    if (C_is_hyper) C->nvec = cnvec ;
    C->magic = GB_MAGIC ;

    //--------------------------------------------------------------------------
    // C = kron (A,B)
    //--------------------------------------------------------------------------

    #pragma omp parallel for num_threads(nthreads) schedule(guided) collapse(2)
    for (int64_t kA = 0 ; kA < anvec ; kA++)
    {
        for (int64_t kB = 0 ; kB < bnvec ; kB++)
        {
            // get B(:,jB), the (kB)th vector of B
            int64_t pB_start = Bp [kB] ;
            int64_t pB_end   = Bp [kB+1] ;
            int64_t bknz = pB_start - pB_end ;
            if (bknz == 0) continue ;
            GB_void bwork [bsize] ;
            // get C(:,jC), the (kC)th vector of C
            int64_t kC = kA * bnvec + kB ;
            int64_t pC = Cp [kC] ;
            // get A(:,jA), the (kA)th vector of A
            int64_t pA_start = Ap [kA] ;
            int64_t pA_end   = Ap [kA+1] ;
            GB_void awork [asize] ;
            for (int64_t pA = pA_start ; pA < pA_end ; pA++)
            {
                // awork = A(iA,jA), typecasted to op->xtype
                int64_t iA = Ai [pA] ;
                int64_t iAblock = iA * bvlen ;
                cast_A (awork, Ax +(pA*asize), asize) ;
                for (int64_t pB = pB_start ; pB < pB_end ; pB++)
                { 
                    // bwork = B(iB,jB), typecasted to op->ytype
                    int64_t iB = Bi [pB] ;
                    cast_B (bwork, Bx +(pB*bsize), bsize) ;
                    // C(iC,jC) = A(iA,jA) * B(iB,jB)
                    int64_t iC = iAblock + iB ;
                    Ci [pC] = iC ;
                    fmult (Cx +(pC*csize), awork, bwork) ;
                    pC++ ;
                }
            }
        }
    }

    //--------------------------------------------------------------------------
    // remove empty vectors from C, if hypersparse
    //--------------------------------------------------------------------------

    if (C_is_hyper && C->nvec_nonempty < cnvec)
    {
        // create new Cp_new and Ch_new arrays, with no empty vectors
        int64_t *restrict Cp_new = NULL ;
        int64_t *restrict Ch_new = NULL ;
        int64_t nvec_new ;
        info = GB_hyper_prune (&Cp_new, &Ch_new, &nvec_new, C->p, C->h, cnvec,
            Context) ;
        if (info != GrB_SUCCESS)
        { 
            // out of memory
            GB_MATRIX_FREE (&C) ;
            return (info) ;
        }
        // transplant the new hyperlist into C
        GB_FREE_MEMORY (C->p, cnvec+1, sizeof (int64_t)) ;
        GB_FREE_MEMORY (C->h, cnvec,   sizeof (int64_t)) ;
        C->p = Cp_new ;
        C->h = Ch_new ;
        C->nvec = nvec_new ;
        C->plen = nvec_new ;
        ASSERT (C->nvec == C->nvec_nonempty) ;
    }

    ASSERT (C->nvec_nonempty == GB_nvec_nonempty (C, Context)) ;

    //--------------------------------------------------------------------------
    // return result
    //--------------------------------------------------------------------------

    ASSERT_OK (GB_check (C, "C=kron(A,B)", GB0)) ;
    (*Chandle) = C ;
    return (GrB_SUCCESS) ;
}

