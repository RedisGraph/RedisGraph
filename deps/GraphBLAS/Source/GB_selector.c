//------------------------------------------------------------------------------
// GB_selector:  select entries from a matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB_select.h"
#include "GB_ek_slice.h"
#include "GB_sel__include.h"

#define GB_FREE_ALL                         \
{                                           \
    GB_MATRIX_FREE (&C) ;                   \
    GB_FREE_WORK ;                          \
}

#define GB_FREE_WORK                                    \
{                                                       \
    GB_ek_slice_free (&pstart_slice, &kfirst_slice, &klast_slice, ntasks) ; \
    GB_FREE_MEMORY (Wfirst, ntasks, sizeof (int64_t)) ;             \
    GB_FREE_MEMORY (Wlast, ntasks, sizeof (int64_t)) ;              \
    GB_FREE_MEMORY (C_pstart_slice, ntasks, sizeof (int64_t)) ;     \
    GB_FREE_MEMORY (Zp, aplen,   sizeof (int64_t)) ;                \
    GB_FREE_MEMORY (Cp, aplen+1, sizeof (int64_t)) ;                \
    GB_FREE_MEMORY (Ch, aplen,   sizeof (int64_t)) ;                \
    GB_FREE_MEMORY (Ci, cnz,     sizeof (int64_t)) ;                \
    GB_FREE_MEMORY (Cx, cnz,     asize) ;                           \
}

//------------------------------------------------------------------------------
// GB_selector
//------------------------------------------------------------------------------

GrB_Info GB_selector
(
    GrB_Matrix *Chandle,        // output matrix, NULL to modify A in-place
    GB_Select_Opcode opcode,    // selector opcode
    const GxB_SelectOp op,      // user operator
    const bool flipij,          // if true, flip i and j for user operator
    GrB_Matrix A,               // input matrix
    int64_t ithunk,             // (int64_t) Thunk, if Thunk is NULL
    const GxB_Scalar Thunk,     // optional input for select operator
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    // If the opcode is NONZOMBIE, then GB_wait has removed A from the queue.
    // A will have zombies and pending tuples, but it is not in the queue.
    ASSERT_MATRIX_OK (A, "A input for GB_selector", GB_FLIP (GB0)) ;
    ASSERT_SELECTOP_OK_OR_NULL (op, "selectop for GB_selector", GB0) ;
    ASSERT_SCALAR_OK_OR_NULL (Thunk, "Thunk for GB_selector", GB0) ;
    ASSERT (opcode >= 0 && opcode <= GB_USER_SELECT_opcode) ;

    GrB_Info info ;
    if (Chandle != NULL)
    { 
        (*Chandle) = NULL ;
    }

    int64_t *GB_RESTRICT Zp = NULL ;
    int64_t *GB_RESTRICT Wfirst = NULL ;
    int64_t *GB_RESTRICT Wlast = NULL ;
    int64_t *GB_RESTRICT C_pstart_slice = NULL ;

    //--------------------------------------------------------------------------
    // determine the number of threads and tasks to use
    //--------------------------------------------------------------------------

    int64_t anz = GB_NNZ (A) ;
    int64_t anvec = A->nvec ;
    double work = 8*anvec + ((opcode == GB_DIAG_opcode) ? 0 : anz) ;

    GB_GET_NTHREADS_MAX (nthreads_max, chunk, Context) ;
    int nthreads = GB_nthreads (work, chunk, nthreads_max) ;

    int ntasks = (nthreads == 1) ? 1 : (8 * nthreads) ;
    ntasks = GB_IMIN (ntasks, anz) ;
    ntasks = GB_IMAX (ntasks, 1) ;

    //--------------------------------------------------------------------------
    // get A
    //--------------------------------------------------------------------------

    int64_t *GB_RESTRICT Ah = A->h ;
    int64_t *GB_RESTRICT Ap = A->p ;
    int64_t *GB_RESTRICT Ai = A->i ;
    GB_void *GB_RESTRICT Ax = A->x ;
    int64_t asize = A->type->size ;
    int64_t aplen = A->plen ;
    int64_t avlen = A->vlen ;
    int64_t avdim = A->vdim ;
    GB_Type_code typecode = A->type->code ;

    //--------------------------------------------------------------------------
    // get Thunk
    //--------------------------------------------------------------------------

    // The scalar value of Thunk(0) is typecasted to an integer (int64_t
    // ithunk) for built-in operators (tril, triu, diag, offdiag, and resize).
    // It is also typecast to the same type as A (to the scalar athunk).  This
    // is used for gt, ge, lt, le, ne, eq to Thunk, for built-in types.

    // If Thunk is NULL, or has no entry, it is treated as a scalar value
    // of zero.

    GB_void athunk [GB_VLA(asize)] ;
    memset (athunk, 0, asize) ;
    GB_void *GB_RESTRICT xthunk = athunk ;

    if (Thunk != NULL && GB_NNZ (Thunk) > 0)
    {
        // xthunk points to Thunk->x for user-defined select operators
        xthunk = Thunk->x ;
        GB_Type_code tcode = Thunk->type->code ;
        ithunk = 0 ;
        if (tcode <= GB_FP64_code && opcode < GB_USER_SELECT_opcode)
        { 
            // ithunk = (int64_t) Thunk (0)
            GB_cast_array ((GB_void *GB_RESTRICT) &ithunk,
                                   GB_INT64_code, Thunk->x, tcode, 1, NULL) ;
            // athunk = (atype) Thunk (0)
            GB_cast_array (athunk, A->type->code, Thunk->x, tcode, 1, NULL) ;
            // xthunk now points to the typecasted (atype) Thunk (0)
            xthunk = athunk ;
        }
    }

    //--------------------------------------------------------------------------
    // get the user-defined operator
    //--------------------------------------------------------------------------

    GxB_select_function user_select = NULL ;
    if (op != NULL && opcode >= GB_USER_SELECT_opcode)
    { 
        GB_BURBLE_MATRIX (A, "generic ") ;
        user_select = (GxB_select_function) (op->function) ;
    }

    //--------------------------------------------------------------------------
    // allocate the new vector pointers of C
    //--------------------------------------------------------------------------

    GrB_Matrix C = NULL ;
    int64_t *GB_RESTRICT Cp = NULL ;
    int64_t *GB_RESTRICT Ch = NULL ;
    int64_t *GB_RESTRICT Ci = NULL ;
    GB_void *GB_RESTRICT Cx = NULL ;
    GB_CALLOC_MEMORY (Cp, aplen+1, sizeof (int64_t)) ;
    int64_t cnz = 0 ;
    if (Cp == NULL)
    { 
        // out of memory
        return (GB_OUT_OF_MEMORY) ;
    }
    Cp [anvec] = 0 ;

    //--------------------------------------------------------------------------
    // slice the entries for each task
    //--------------------------------------------------------------------------

    // Task tid does entries pstart_slice [tid] to pstart_slice [tid+1]-1 and
    // vectors kfirst_slice [tid] to klast_slice [tid].  The first and last
    // vectors may be shared with prior slices and subsequent slices.

    int64_t *pstart_slice = NULL, *kfirst_slice = NULL, *klast_slice = NULL ;
    if (!GB_ek_slice (&pstart_slice, &kfirst_slice, &klast_slice, A, ntasks))
    { 
        // out of memory
        GB_FREE_ALL ;
        return (GB_OUT_OF_MEMORY) ;
    }

    //--------------------------------------------------------------------------
    // allocate workspace for each task
    //--------------------------------------------------------------------------

    GB_CALLOC_MEMORY (Wfirst, ntasks, sizeof (int64_t)) ;
    GB_CALLOC_MEMORY (Wlast, ntasks, sizeof (int64_t)) ;
    GB_CALLOC_MEMORY (C_pstart_slice, ntasks, sizeof (int64_t)) ;

    if (Wfirst == NULL || Wlast  == NULL || C_pstart_slice == NULL)
    { 
        // out of memory
        GB_FREE_ALL ;
        return (GB_OUT_OF_MEMORY) ;
    }

    //--------------------------------------------------------------------------
    // count the live entries in each vector
    //--------------------------------------------------------------------------

    // Use the GB_reduce_each_vector template to count the number of live
    // entries in each vector of A.  The result is computed in Cp, where Cp [k]
    // is the number of live entries in the kth vector of A.

    if (opcode <= GB_RESIZE_opcode)
    {
        // allocate Zp
        GB_MALLOC_MEMORY (Zp, aplen, sizeof (int64_t)) ;
        if (Zp == NULL)
        { 
            // out of memory
            GB_FREE_ALL ;
            return (GB_OUT_OF_MEMORY) ;
        }
    }

    //--------------------------------------------------------------------------
    // phase1: launch the switch factory to count the entries
    //--------------------------------------------------------------------------

    #define GB_SELECT_PHASE1
    #define GB_sel1(opname,aname) GB_sel_phase1_ ## opname ## aname
    #define GB_SEL_WORKER(opname,aname,atype)                           \
    {                                                                   \
        GB_sel1 (opname, aname) (Zp, Cp,                                \
            (GB_void *) Wfirst, (GB_void *) Wlast,                      \
            A, kfirst_slice, klast_slice, pstart_slice, flipij, ithunk, \
            (atype *) xthunk, user_select, ntasks, nthreads) ;          \
    }                                                                   \
    break ;

    #include "GB_select_factory.c"

    #undef  GB_SELECT_PHASE1
    #undef  GB_SEL_WORKER

    //--------------------------------------------------------------------------
    // compute the new vector pointers
    //--------------------------------------------------------------------------

    // Cp = cumsum (Cp)
    int64_t C_nvec_nonempty ;
    GB_cumsum (Cp, anvec, &C_nvec_nonempty, nthreads) ;
    cnz = Cp [anvec] ;

    //--------------------------------------------------------------------------
    // determine the slice boundaries in the new C matrix
    //--------------------------------------------------------------------------

    int64_t kprior = -1 ;
    int64_t pC = 0 ;

    for (int taskid = 0 ; taskid < ntasks ; taskid++)
    {
        int64_t k = kfirst_slice [taskid] ;

        if (kprior < k)
        { 
            // Task taskid is the first one to do work on C(:,k), so it starts
            // at Cp [k], and it contributes Wfirst [taskid] entries to C(:,k)
            pC = Cp [k] ;
            kprior = k ;
        }

        // Task taskid contributes Wfirst [taskid] entries to C(:,k)
        C_pstart_slice [taskid] = pC ;
        pC += Wfirst [taskid] ;

        int64_t klast = klast_slice [taskid] ;
        if (k < klast)
        { 
            // Task taskid is the last to contribute to C(:,k).
            ASSERT (pC == Cp [k+1]) ;
            // Task taskid contributes the first Wlast [taskid] entries
            // to C(:,klast), so the next task taskid+1 starts at this
            // location, if its first vector is klast of this task.
            pC = Cp [klast] + Wlast [taskid] ;
            kprior = klast ;
        }
    }

    //--------------------------------------------------------------------------
    // allocate new space for the compacted Ci and Cx
    //--------------------------------------------------------------------------

    GB_MALLOC_MEMORY (Ci, cnz, sizeof (int64_t)) ;

    if (opcode == GB_EQ_ZERO_opcode)
    { 
        // since Cx [0..cnz-1] is all zero, phase2 only needs to construct
        // the pattern in Ci
        GB_CALLOC_MEMORY (Cx, cnz, asize) ;
    }
    else
    { 
        GB_MALLOC_MEMORY (Cx, cnz, asize) ;
    }

    if (Ci == NULL || Cx == NULL)
    { 
        // out of memory
        GB_FREE_ALL ;
        return (GB_OUT_OF_MEMORY) ;
    }

    //--------------------------------------------------------------------------
    // phase2: launch the switch factory to select the entries
    //--------------------------------------------------------------------------

    #define GB_SELECT_PHASE2
    #define GB_sel2(opname,aname) GB_sel_phase2_ ## opname ## aname
    #define GB_SEL_WORKER(opname,aname,atype)                           \
    {                                                                   \
        GB_sel2 (opname, aname) (Ci, (atype *) Cx,                      \
            Zp, Cp, C_pstart_slice,                                     \
            A, kfirst_slice, klast_slice, pstart_slice, flipij, ithunk, \
            (atype *) xthunk, user_select, ntasks, nthreads) ;          \
    }                                                                   \
    break ;

    #include "GB_select_factory.c"

    //--------------------------------------------------------------------------
    // create the result
    //--------------------------------------------------------------------------

    if (Chandle == NULL)
    {

        //----------------------------------------------------------------------
        // transplant C back into A
        //----------------------------------------------------------------------

        if (A->is_hyper && C_nvec_nonempty < anvec)
        {
            // prune empty vectors from Ah and Ap
            int64_t cnvec = 0 ;
            for (int64_t k = 0 ; k < anvec ; k++)
            {
                if (Cp [k] < Cp [k+1])
                { 
                    Ah [cnvec] = Ah [k] ;
                    Ap [cnvec] = Cp [k] ;
                    cnvec++ ;
                }
            }
            Ap [cnvec] = Cp [anvec] ;
            A->nvec = cnvec ;
            ASSERT (A->nvec == C_nvec_nonempty) ;
            GB_FREE_MEMORY (Cp, aplen+1, sizeof (int64_t)) ;
        }
        else
        { 
            GB_FREE_MEMORY (Ap, aplen+1, sizeof (int64_t)) ;
            A->p = Cp ; Cp = NULL ;
        }

        ASSERT (Cp == NULL) ;

        GB_FREE_MEMORY (Ai, A->nzmax, sizeof (int64_t)) ;
        GB_FREE_MEMORY (Ax, A->nzmax, asize) ;
        A->i = Ci ; Ci = NULL ;
        A->x = Cx ; Cx = NULL ;
        A->nzmax = cnz ;
        A->nvec_nonempty = C_nvec_nonempty ;

        if (A->nzmax == 0)
        { 
            GB_FREE_MEMORY (A->i, A->nzmax, sizeof (int64_t)) ;
            GB_FREE_MEMORY (A->x, A->nzmax, asize) ;
        }

        // the NONZOMBIES opcode may have removed all zombies, but A->nzombie
        // is still nonzero.  It set to zero in GB_wait.
        ASSERT_MATRIX_OK (A, "A output for GB_selector", GB_FLIP (GB0)) ;

    }
    else
    {

        //----------------------------------------------------------------------
        // create C and transplant Cp, Ch, Ci, Cx into C
        //----------------------------------------------------------------------

        GB_NEW (&C, A->type, avlen, avdim, GB_Ap_null, true,
            GB_SAME_HYPER_AS (A->is_hyper), A->hyper_ratio, aplen, Context) ;
        GB_OK (info) ;

        if (A->is_hyper)
        {
            GB_MALLOC_MEMORY (Ch, aplen, sizeof (int64_t)) ;
            if (Ch == NULL)
            { 
                // out of memory
                GB_FREE_ALL ;
                return (GB_OUT_OF_MEMORY) ;
            }

            // copy non-empty vectors from Ah to Ch
            int64_t cnvec = 0 ;
            for (int64_t k = 0 ; k < anvec ; k++)
            {
                if (Cp [k] < Cp [k+1])
                { 
                    Ch [cnvec] = Ah [k] ;
                    Cp [cnvec] = Cp [k] ;
                    cnvec++ ;
                }
            }
            Cp [cnvec] = Cp [anvec] ;
            C->nvec = cnvec ;
            ASSERT (C->nvec == C_nvec_nonempty) ;
        }

        C->p = Cp ; Cp = NULL ;
        C->h = Ch ; Ch = NULL ;
        C->i = Ci ; Ci = NULL ;
        C->x = Cx ; Cx = NULL ;
        C->nzmax = cnz ;
        C->magic = GB_MAGIC ;
        C->nvec_nonempty = C_nvec_nonempty ;

        if (C->nzmax == 0)
        { 
            GB_FREE_MEMORY (C->i, C->nzmax, sizeof (int64_t)) ;
            GB_FREE_MEMORY (C->x, C->nzmax, asize) ;
        }

        (*Chandle) = C ;
        ASSERT_MATRIX_OK (C, "C output for GB_selector", GB0) ;
    }

    //--------------------------------------------------------------------------
    // free workspace and return result
    //--------------------------------------------------------------------------

    GB_FREE_WORK ;
    return (GrB_SUCCESS) ;
}

