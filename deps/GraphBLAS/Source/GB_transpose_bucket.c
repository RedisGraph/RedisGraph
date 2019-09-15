//------------------------------------------------------------------------------
// GB_transpose_bucket: transpose and optionally typecast and/or apply operator
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// C = A' or op(A').  Optionally typecasts from A->type to the new type ctype,
// and/or optionally applies a unary operator.

// If an operator z=op(x) is provided, the type of z must be the same as the
// type of C.  The type of A must be compatible with the type of of x (A is
// typecasted into the type of x).  These conditions must be checked in the
// caller.

// The input matrix A may have jumbled row indices; this is OK.
// The output matrix C will always have sorted row indices.

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
    for (int taskid = 0 ; taskid < naslice ; taskid++)                  \
    {                                                                   \
        GB_FREE_MEMORY (Rowcounts [taskid], vlen+1, sizeof (int64_t)) ; \
    }                                                                   \
}

#define GB_FREE_ALL                                                     \
{                                                                       \
    GB_MATRIX_FREE (&C) ;                                               \
    GB_FREE_WORK ;                                                      \
}

GrB_Info GB_transpose_bucket    // bucket transpose; typecast and apply op
(
    GrB_Matrix *Chandle,        // output matrix (unallocated on input)
    const GrB_Type ctype,       // type of output matrix C
    const bool C_is_csc,        // format of output matrix C
    const GrB_Matrix A,         // input matrix
    const GrB_UnaryOp op,       // operator to apply, NULL if no operator
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (Chandle != NULL) ;
    (*Chandle) = NULL ;
    ASSERT_OK (GB_check (ctype, "ctype for transpose", GB0)) ;
    // OK if the matrix A is jumbled; this function is intended to sort it.
    ASSERT_OK_OR_JUMBLED (GB_check (A, "A input for transpose_bucket", GB0)) ;
    ASSERT (!GB_PENDING (A)) ; ASSERT (!GB_ZOMBIES (A)) ;

    if (op != NULL)
    { 
        ASSERT_OK (GB_check (op, "op for transpose", GB0)) ;
        ASSERT (ctype == op->ztype) ;
        ASSERT (GB_Type_compatible (A->type, op->xtype)) ;
    }

    //--------------------------------------------------------------------------
    // get A
    //--------------------------------------------------------------------------

    int64_t anz = GB_NNZ (A) ;
    int64_t vlen = A->vlen ;

    //--------------------------------------------------------------------------
    // determine the number of threads to use
    //--------------------------------------------------------------------------

    GB_GET_NTHREADS_MAX (nthreads_max, chunk, Context) ;

    // # of threads to use in the O(vlen) loops below
    int nthreads = GB_nthreads (vlen, chunk, nthreads_max) ;

    // A is sliced into naslice parts, so that each part has at least vlen
    // entries.  The workspace required is naslice*vlen, so this ensures
    // the workspace is no more than the size of A.

    // naslice < floor (anz / vlen) < anz / vlen
    // thus naslice*vlen < anz

    // also, naslice < nthreads_max, since each part will be about the same size

    int naslice = GB_nthreads (anz, GB_IMAX (vlen, chunk), nthreads_max) ;

    //--------------------------------------------------------------------------
    // initialze the row count arrays
    //--------------------------------------------------------------------------

    int64_t *Rowcounts [naslice] ;
    for (int taskid = 0 ; taskid < naslice ; taskid++)
    {
        Rowcounts [taskid] = NULL ;
    }

    //--------------------------------------------------------------------------
    // allocate C: always non-hypersparse
    //--------------------------------------------------------------------------

    // The bucket transpose only works when C is not hypersparse.
    // A can be hypersparse.

    // [ C->p is allocated but not initialized.  It is NON-hypersparse.
    GrB_Info info ;
    GrB_Matrix C = NULL ;
    GB_CREATE (&C, ctype, A->vdim, vlen, GB_Ap_malloc, C_is_csc,
        GB_FORCE_NONHYPER, A->hyper_ratio, vlen, anz, true, Context) ;
    GB_OK (info) ;

    int64_t *restrict Cp = C->p ;

    //--------------------------------------------------------------------------
    // allocate workspace
    //--------------------------------------------------------------------------

    for (int taskid = 0 ; taskid < naslice ; taskid++)
    {
        int64_t *rowcount = NULL ;
        GB_CALLOC_MEMORY (rowcount, vlen + 1, sizeof (int64_t)) ;
        if (rowcount == NULL)
        { 
            // out of memory
            GB_FREE_ALL ;
            return (GB_OUT_OF_MEMORY) ;
        }
        Rowcounts [taskid] = rowcount ;
    }

    //--------------------------------------------------------------------------
    // phase1: symbolic analysis
    //--------------------------------------------------------------------------

    // create the iterator for A
    GBI_single_iterator Iter ;
    int64_t A_slice [naslice+1] ;
    GB_pslice (A_slice, /* A */ A->p, A->nvec, naslice) ;
    GBI1_init (&Iter, A) ;

    // sum up the row counts and find C->p
    if (naslice == 1)
    {

        //----------------------------------------------------------------------
        // A is not sliced
        //----------------------------------------------------------------------

        // compute the row counts of A.  No need to scan the A->p pointers
        int64_t *restrict rowcount = Rowcounts [0] ;
        const int64_t *restrict Ai = A->i ;
        for (int64_t p = 0 ; p < anz ; p++)
        { 
            rowcount [Ai [p]]++ ;
        }

        // cumulative sum of the rowcount, and copy back into C->p
        GB_cumsum (rowcount, vlen, (&C->nvec_nonempty), nthreads) ;
        GB_memcpy (Cp, rowcount, (vlen+1) * sizeof (int64_t), nthreads) ;

    }
    else
    {

        //----------------------------------------------------------------------
        // A is sliced
        //----------------------------------------------------------------------

        // compute the row counts of A for each slice
        #define GB_PHASE_1_OF_2
        #include "GB_unaryop_transpose.c"

        // cumulative sum of the rowcounts across the slices
        #pragma omp parallel for num_threads(nthreads) schedule(static)
        for (int64_t i = 0 ; i < vlen ; i++)
        {
            int64_t s = 0 ;
            for (int taskid = 0 ; taskid < naslice ; taskid++)
            { 
                int64_t *restrict rowcount = Rowcounts [taskid] ;
                int64_t c = rowcount [i] ;
                rowcount [i] = s ;
                s += c ;
            }
            Cp [i] = s ;
        }
        Cp [vlen] = 0 ;

        // compute the vector pointers for C; also compute C->nvec_nonempty
        GB_cumsum (Cp, vlen, &(C->nvec_nonempty), nthreads) ;

        // add Cp back to all Rowcounts
        #pragma omp parallel for num_threads(nthreads) schedule(static)
        for (int64_t i = 0 ; i < vlen ; i++)
        {
            int64_t s = Cp [i] ;
            int64_t *restrict rowcount = Rowcounts [0] ;
            rowcount [i] = s ;
            for (int taskid = 1 ; taskid < naslice ; taskid++)
            { 
                int64_t *restrict rowcount = Rowcounts [taskid] ;
                rowcount [i] += s ;
            }
        }
    }

    C->magic = GB_MAGIC ;      // C is now initialized ]

    //--------------------------------------------------------------------------
    // phase2: transpose A into C
    //--------------------------------------------------------------------------

    // transpose both the pattern and the values
    if (op == NULL)
    { 
        // do not apply an operator; optional typecast to ctype
        GB_transpose_ix (C, A, Rowcounts, Iter, A_slice, naslice) ;
    }
    else
    { 
        // apply an operator, C has type op->ztype
        GB_transpose_op (C, op, A, Rowcounts, Iter, A_slice, naslice) ;
    }

    //--------------------------------------------------------------------------
    // free workspace and return result
    //--------------------------------------------------------------------------

    GB_FREE_WORK ;
    ASSERT_OK (GB_check (C, "C transpose of A", GB0)) ;
    ASSERT (!C->is_hyper) ;
    (*Chandle) = C ;
    return (GrB_SUCCESS) ;
}

