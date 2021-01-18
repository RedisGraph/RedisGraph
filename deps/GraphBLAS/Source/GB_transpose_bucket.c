//------------------------------------------------------------------------------
// GB_transpose_bucket: transpose and optionally typecast and/or apply operator
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// C = A' or op(A').  Optionally typecasts from A->type to the new type ctype,
// and/or optionally applies a unary operator.

// If an operator z=op(x) is provided, the type of z must be the same as the
// type of C.  The type of A must be compatible with the type of of x (A is
// typecasted into the type of x).  These conditions must be checked in the
// caller.

// This function is agnostic for the CSR/CSC format of C and A.  C_is_csc is
// defined by the caller and assigned to C->is_csc, but otherwise unused.
// A->is_csc is ignored.

// The input can be hypersparse or non-hypersparse.  The output C is always
// non-hypersparse, and never shallow.

// If A is m-by-n in CSC format, with e nonzeros, the time and memory taken is
// O(m+n+e) if A is non-hypersparse, or O(m+e) if hypersparse.  This is fine if
// most rows and columns of A are non-empty, but can be very costly if A or A'
// is hypersparse.  In particular, if A is a non-hypersparse column vector with
// m >> e, the time and memory is O(m), which can be huge.  Thus, for
// hypersparse matrices, or for very sparse matrices, the qsort method should
// be used instead (see GB_transpose).

// This method is parallel, but not highly scalable.  At most O(e/m) threads
// are used.

#include "GB_transpose.h"

#define GB_FREE_WORK                                                    \
{                                                                       \
    if (Workspaces != NULL)                                             \
    {                                                                   \
        for (int tid = 0 ; tid < nworkspaces ; tid++)                   \
        {                                                               \
            GB_FREE (Workspaces [tid]) ;                                \
        }                                                               \
    }                                                                   \
    GB_FREE (Workspaces) ;                                              \
    GB_FREE (A_slice) ;                                                 \
}

#define GB_FREE_ALL                                                     \
{                                                                       \
    GB_Matrix_free (&C) ;                                               \
    GB_FREE_WORK ;                                                      \
}

GrB_Info GB_transpose_bucket    // bucket transpose; typecast and apply op
(
    GrB_Matrix *Chandle,        // output matrix (unallocated on input)
    const GrB_Type ctype,       // type of output matrix C
    const bool C_is_csc,        // format of output matrix C
    const GrB_Matrix A,         // input matrix
        // no operator is applied if both op1 and op2 are NULL
        const GrB_UnaryOp op1,          // unary operator to apply
        const GrB_BinaryOp op2,         // binary operator to apply
        const GxB_Scalar scalar,        // scalar to bind to binary operator
        bool binop_bind1st,             // if true, binop(x,A) else binop(A,y)
    const int nworkspaces,      // # of workspaces to use
    const int nthreads,         // # of threads to use
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (Chandle != NULL) ;
    (*Chandle) = NULL ;
    ASSERT_TYPE_OK (ctype, "ctype for transpose", GB0) ;
    ASSERT_MATRIX_OK (A, "A input for transpose_bucket", GB0) ;
    ASSERT (!GB_PENDING (A)) ;
    ASSERT (!GB_ZOMBIES (A)) ;
    ASSERT (GB_JUMBLED_OK (A)) ;

    // if op1 and op2 are NULL, then no operator is applied

    // This method is only be used when A is sparse or hypersparse.
    // The full and bitmap cases are handled in GB_transpose.
    ASSERT (!GB_IS_FULL (A)) ;
    ASSERT (!GB_IS_BITMAP (A)) ;
    ASSERT (GB_IS_SPARSE (A) || GB_IS_HYPERSPARSE (A)) ;

    int64_t *GB_RESTRICT A_slice = NULL ;          // size nthreads+1
    int64_t *GB_RESTRICT *Workspaces = NULL ;      // size nworkspaces

    //--------------------------------------------------------------------------
    // get A
    //--------------------------------------------------------------------------

    int64_t anz = GB_NNZ (A) ;
    int64_t vlen = A->vlen ;

    //--------------------------------------------------------------------------
    // determine the number of threads to use
    //--------------------------------------------------------------------------

    // # of threads to use in the O(vlen) loops below
    GB_GET_NTHREADS_MAX (nthreads_max, chunk, Context) ;
    int nth = GB_nthreads (vlen, chunk, nthreads_max) ;

    //--------------------------------------------------------------------------
    // allocate C: always sparse
    //--------------------------------------------------------------------------

    // The bucket transpose only works when C is sparse.
    // A can be sparse or hypersparse.

    // C->p is allocated but not initialized.
    GrB_Info info ;
    GrB_Matrix C = NULL ;
    GB_OK (GB_new_bix (&C, // sparse, new header
        ctype, A->vdim, vlen, GB_Ap_malloc, C_is_csc,
        GxB_SPARSE, true, A->hyper_switch, vlen, anz, true, Context)) ;

    int64_t *GB_RESTRICT Cp = C->p ;

    //--------------------------------------------------------------------------
    // allocate workspace
    //--------------------------------------------------------------------------

    Workspaces = GB_CALLOC (nworkspaces, int64_t *) ;
    if (Workspaces == NULL)
    { 
        // out of memory
        GB_FREE_ALL ;
        return (GrB_OUT_OF_MEMORY) ;
    }

    for (int tid = 0 ; tid < nworkspaces ; tid++)
    {
        int64_t *workspace = GB_MALLOC (vlen + 1, int64_t) ;
        if (workspace == NULL)
        { 
            // out of memory
            GB_FREE_ALL ;
            return (GrB_OUT_OF_MEMORY) ;
        }
        Workspaces [tid] = workspace ;
    }

    //==========================================================================
    // phase1: symbolic analysis
    //==========================================================================

    // slice the A matrix, perfectly balanced for one task per thread
    if (!GB_pslice (&A_slice, A->p, A->nvec, nthreads, true))
    { 
        // out of memory
        GB_FREE_ALL ;
        return (GrB_OUT_OF_MEMORY) ;
    }

    // sum up the row counts and find C->p
    if (nthreads == 1)
    {

        //----------------------------------------------------------------------
        // sequential method: A is not sliced
        //----------------------------------------------------------------------

        // Only requires a single int64 workspace of size vlen for a single
        // thread.  The resulting C matrix is not jumbled.

        // compute the row counts of A.  No need to scan the A->p pointers
        ASSERT (nworkspaces == 1) ;
        int64_t *GB_RESTRICT workspace = Workspaces [0] ;
        memset (workspace, 0, (vlen + 1) * sizeof (int64_t)) ;
        const int64_t *GB_RESTRICT Ai = A->i ;
        for (int64_t p = 0 ; p < anz ; p++)
        { 
            int64_t i = Ai [p] ;
            workspace [i]++ ;
        }

        // cumulative sum of the workspace, and copy back into C->p
        GB_cumsum (workspace, vlen, (&C->nvec_nonempty), 1) ;
        memcpy (Cp, workspace, (vlen + 1) * sizeof (int64_t)) ;

    }
    else if (nworkspaces == 1)
    {

        //----------------------------------------------------------------------
        // atomic method: A is sliced but workspace is shared
        //----------------------------------------------------------------------

        // Only requires a single int64 workspace of size vlen, shared by all
        // threads.  Scales well, but requires atomics.  If the # of rows is
        // very small and the average row degree is high, this can be very slow
        // because of contention on the atomic workspace.  Otherwise, it is
        // typically faster than the non-atomic method.  The resulting C matrix
        // is jumbled.

        // compute the row counts of A.  No need to scan the A->p pointers
        int64_t *GB_RESTRICT workspace = Workspaces [0] ;
        GB_memset (workspace, 0, (vlen + 1) * sizeof (int64_t), nth) ;
        const int64_t *GB_RESTRICT Ai = A->i ;
        int64_t p ;
        #pragma omp parallel for num_threads(nthreads) schedule(static)
        for (p = 0 ; p < anz ; p++)
        { 
            int64_t i = Ai [p] ;
            // update workspace [i]++ automically:
            GB_ATOMIC_UPDATE
            workspace [i]++ ;
        }

        C->jumbled = true ; // atomic transpose leaves C jumbled

        // cumulative sum of the workspace, and copy back into C->p
        GB_cumsum (workspace, vlen, (&C->nvec_nonempty), nth) ;
        GB_memcpy (Cp, workspace, (vlen+ 1) * sizeof (int64_t), nth) ;

    }
    else
    {

        //----------------------------------------------------------------------
        // non-atomic method
        //----------------------------------------------------------------------

        // compute the row counts of A for each slice, one per thread; This
        // method is parallel, but not highly scalable.  Each thread requires
        // int64 workspace of size vlen, but no atomics are required.  The
        // resulting C matrix is not jumbled, so this can save work if C needs
        // to be unjumbled later.

        ASSERT (nworkspaces == nthreads) ;
        const int64_t *GB_RESTRICT Ap = A->p ;
        const int64_t *GB_RESTRICT Ah = A->h ;
        const int64_t *GB_RESTRICT Ai = A->i ;

        int tid ;
        #pragma omp parallel for num_threads(nthreads) schedule(static)
        for (tid = 0 ; tid < nthreads ; tid++)
        {
            // get the row counts for this slice, of size A->vlen
            int64_t *GB_RESTRICT workspace = Workspaces [tid] ;
            memset (workspace, 0, (vlen + 1) * sizeof (int64_t)) ;
            for (int64_t k = A_slice [tid] ; k < A_slice [tid+1] ; k++)
            {
                // iterate over the entries in A(:,j)
                int64_t j = GBH (Ah, k) ;
                int64_t pA_start = Ap [k] ;
                int64_t pA_end = Ap [k+1] ;
                for (int64_t pA = pA_start ; pA < pA_end ; pA++)
                { 
                    // count one more entry in C(i,:) for this slice
                    int64_t i = Ai [pA] ;
                    workspace [i]++ ;
                }
            }
        }

        // cumulative sum of the workspaces across the slices
        int64_t i ;
        #pragma omp parallel for num_threads(nth) schedule(static)
        for (i = 0 ; i < vlen ; i++)
        {
            int64_t s = 0 ;
            for (int tid = 0 ; tid < nthreads ; tid++)
            { 
                int64_t *GB_RESTRICT workspace = Workspaces [tid] ;
                int64_t c = workspace [i] ;
                workspace [i] = s ;
                s += c ;
            }
            Cp [i] = s ;
        }
        Cp [vlen] = 0 ;

        // compute the vector pointers for C
        GB_cumsum (Cp, vlen, &(C->nvec_nonempty), nth) ;

        // add Cp back to all Workspaces
        #pragma omp parallel for num_threads(nth) schedule(static)
        for (i = 0 ; i < vlen ; i++)
        {
            int64_t s = Cp [i] ;
            int64_t *GB_RESTRICT workspace = Workspaces [0] ;
            workspace [i] = s ;
            for (int tid = 1 ; tid < nthreads ; tid++)
            { 
                int64_t *GB_RESTRICT workspace = Workspaces [tid] ;
                workspace [i] += s ;
            }
        }
    }

    C->magic = GB_MAGIC ;

    //==========================================================================
    // phase2: transpose A into C
    //==========================================================================

    // transpose both the pattern and the values
    if (op1 == NULL && op2 == NULL)
    { 
        // do not apply an operator; optional typecast to C->type
        GB_transpose_ix (C, A, Workspaces, A_slice, nworkspaces, nthreads) ;
    }
    else
    { 
        // apply an operator, C has type op->ztype
        GB_transpose_op (C, op1, op2, scalar, binop_bind1st, A,
            Workspaces, A_slice, nworkspaces, nthreads) ;
    }

    //--------------------------------------------------------------------------
    // free workspace and return result
    //--------------------------------------------------------------------------

    GB_FREE_WORK ;
    ASSERT_MATRIX_OK (C, "C transpose of A", GB0) ;
    ASSERT (C->h == NULL) ;
    (*Chandle) = C ;
    return (GrB_SUCCESS) ;
}

