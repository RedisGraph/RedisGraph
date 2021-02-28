//------------------------------------------------------------------------------
// GB_add_phase0: find vectors of C to compute for C=A+B or C<M>=A+B
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// The eWise add of two matrices, C=A+B, C<M>=A+B, or C<!M>=A+B starts with
// this phase, which determines which vectors of C need to be computed.
// This phase is also used for GB_masker.

// On input, A and B are the two matrices being added, and M is the optional
// mask matrix (not complemented).  The complemented mask is handed in GB_mask,
// not here.

// The A matrix can be sparse, hypersparse, slice, or hyperslice.  The B matrix
// can only be sparse or hypersparse.  See GB_wait, which can pass in A as any
// of the four formats.  In this case, no mask is present.

// On output, an integer (Cnvec) a boolean (Ch_to_Mh) and up to 3 arrays are
// returned, either NULL or of size Cnvec.  Let n = A->vdim be the vector
// dimension of A, B, M and C.

//      Ch:  the list of vectors to compute.  If not NULL, Ch [k] = j is the
//      kth vector in C to compute, which will become the hyperlist C->h of C.
//      Note that some of these vectors may turn out to be empty, because of
//      the mask, or because the vector j appeared in A or B, but is empty.
//      It is pruned at the end of GB_add_phase2.  If Ch is NULL then it is an
//      implicit list of size n, and Ch [k] == k for all k = 0:n-1.  In this
//      case, C will be a standard matrix, not hypersparse.  Thus, the kth
//      vector is j = (Ch == NULL) ? k : Ch [k].

//      Ch is freed by GB_add if phase1 fails.  phase2 either frees it or
//      transplants it into C.

//      Ch_is_Mh:  true if the mask M is present, hypersparse, and not
//      complemented, false otherwise.  In this case Ch is a deep copy of Mh.
//      Only GB_add uses this option; it is not used by GB_masker (Ch_is_Mh
//      is always false for GB_masker).  This is determined by passing in
//      p_Ch_is_Mh as a NULL or non-NULL pointer.

//      C_to_A:  if A is hypersparse, then C_to_A [k] = kA if the kth vector, j
//      = (Ch == NULL) ? k : Ch [k] appears in A, as j = Ah [kA].  If j does
//      not appear in A, then C_to_A [k] = -1.  If A is not hypersparse, then
//      C_to_A is returned as NULL.

//      C_to_B:  if B is hypersparse, then C_to_B [k] = kB if the kth vector, j
//      = (Ch == NULL) ? k : Ch [k] appears in B, as j = Bh [kB].  If j does
//      not appear in B, then C_to_B [k] = -1.  If B is not hypersparse, then
//      C_to_B is returned as NULL.

//      C_to_M:  if M is hypersparse, and Ch_is_Mh is false, then C_to_M [k] =
//      kM if the kth vector, j = (Ch == NULL) ? k : Ch [k] appears in M, as j
//      = Mh [kM].  If j does not appear in M, then C_to_M [k] = -1.  If M is
//      not hypersparse, then C_to_M is returned as NULL.

#include "GB_add.h"

#define GB_FREE_WORK                                            \
{                                                               \
    GB_FREE_MEMORY (kA_start, ntasks+1, sizeof (int64_t)) ;     \
    GB_FREE_MEMORY (kB_start, ntasks+1, sizeof (int64_t)) ;     \
    GB_FREE_MEMORY (kC_start, ntasks+1, sizeof (int64_t)) ;     \
}

//------------------------------------------------------------------------------
// GB_allocate_result
//------------------------------------------------------------------------------

static inline bool GB_allocate_result
(
    int64_t Cnvec,
    int64_t *GB_RESTRICT *Ch_handle,
    int64_t *GB_RESTRICT *C_to_M_handle,
    int64_t *GB_RESTRICT *C_to_A_handle,
    int64_t *GB_RESTRICT *C_to_B_handle
)
{
    bool ok = true ;
    if (Ch_handle != NULL)
    { 
        GB_MALLOC_MEMORY (*Ch_handle, Cnvec, sizeof (int64_t)) ;
        ok = (*Ch_handle != NULL) ;
    }
    if (C_to_M_handle != NULL)
    { 
        GB_MALLOC_MEMORY (*C_to_M_handle, Cnvec, sizeof (int64_t)) ;
        ok = ok && (*C_to_M_handle != NULL) ;
    }
    if (C_to_A_handle != NULL)
    { 
        GB_MALLOC_MEMORY (*C_to_A_handle, Cnvec, sizeof (int64_t)) ;
        ok = ok && (*C_to_A_handle != NULL) ;
    }
    if (C_to_B_handle != NULL)
    { 
        GB_MALLOC_MEMORY (*C_to_B_handle, Cnvec, sizeof (int64_t)) ;
        ok = ok && (*C_to_B_handle != NULL) ;
    }

    if (!ok)
    {
        // out of memory
        if (Ch_handle != NULL)
        { 
            GB_FREE_MEMORY (*Ch_handle,     Cnvec, sizeof (int64_t)) ;
        }
        if (C_to_M_handle != NULL)
        { 
            GB_FREE_MEMORY (*C_to_M_handle, Cnvec, sizeof (int64_t)) ;
        }
        if (C_to_A_handle != NULL)
        { 
            GB_FREE_MEMORY (*C_to_A_handle, Cnvec, sizeof (int64_t)) ;
        }
        if (C_to_B_handle != NULL)
        { 
            GB_FREE_MEMORY (*C_to_B_handle, Cnvec, sizeof (int64_t)) ;
        }
    }
    return (ok) ;
}

//------------------------------------------------------------------------------
// GB_add_phase0:  find the vectors of C for C<M>=A+B
//------------------------------------------------------------------------------

GrB_Info GB_add_phase0          // find vectors in C for C=A+B or C<M>=A+B
(
    int64_t *p_Cnvec,           // # of vectors to compute in C
    int64_t *GB_RESTRICT *Ch_handle,        // Ch: size Cnvec, or NULL
    int64_t *GB_RESTRICT *C_to_M_handle,    // C_to_M: size Cnvec, or NULL
    int64_t *GB_RESTRICT *C_to_A_handle,    // C_to_A: size Cnvec, or NULL
    int64_t *GB_RESTRICT *C_to_B_handle,    // C_to_B: of size Cnvec, or NULL
    bool *p_Ch_is_Mh,           // if true, then Ch == Mh
    const GrB_Matrix M,         // optional mask, may be NULL; not complemented
    const GrB_Matrix A,         // standard, hypersparse, slice, or hyperslice
    const GrB_Matrix B,         // standard or hypersparse; never a slice
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (p_Cnvec != NULL) ;
    ASSERT (Ch_handle != NULL) ;
    ASSERT (C_to_A_handle != NULL) ;
    ASSERT (C_to_B_handle != NULL) ;
    ASSERT_MATRIX_OK (A, "A for add phase0", GB0) ;
    ASSERT_MATRIX_OK (B, "B for add phase0", GB0) ;
    ASSERT_MATRIX_OK_OR_NULL (M, "M for add phase0", GB0) ;
    ASSERT (A->vdim == B->vdim) ;
    ASSERT (GB_IMPLIES (M != NULL, A->vdim == M->vdim)) ;

    //--------------------------------------------------------------------------
    // initializations
    //--------------------------------------------------------------------------

    int64_t *GB_RESTRICT Ch = NULL ;
    int64_t *GB_RESTRICT C_to_M = NULL ;
    int64_t *GB_RESTRICT C_to_A = NULL ;
    int64_t *GB_RESTRICT C_to_B = NULL ;

    (*Ch_handle) = NULL ;
    (*C_to_A_handle) = NULL ;
    (*C_to_B_handle) = NULL ;
    if (C_to_M_handle != NULL)
    { 
        (*C_to_M_handle) = NULL ;
    }

    int64_t *GB_RESTRICT kA_start = NULL ;
    int64_t *GB_RESTRICT kB_start = NULL ;
    int64_t *GB_RESTRICT kC_start = NULL ;
    int ntasks = 0 ;

    //--------------------------------------------------------------------------
    // determine the number of threads to use
    //--------------------------------------------------------------------------

    GB_GET_NTHREADS_MAX (nthreads_max, chunk, Context) ;
    int nthreads = 1 ;      // nthreads depends on Cnvec, computed below

    //--------------------------------------------------------------------------
    // get content of M, A, and B
    //--------------------------------------------------------------------------

    int64_t Cnvec ;

    int64_t n = A->vdim ;
    int64_t Anvec = A->nvec ;
    bool A_is_hyper = A->is_hyper ;
    bool A_is_slice = A->is_slice ;
    const int64_t *GB_RESTRICT Ap = A->p ;
    const int64_t *GB_RESTRICT Ah = (A_is_hyper) ? A->h : NULL ;
    const int64_t A_hfirst = A->hfirst ;
    #define GB_Ah(k) (A_is_hyper ? Ah [k] : (A_hfirst + (k)))

    int64_t Bnvec = B->nvec ;
    const int64_t *GB_RESTRICT Bp = B->p ;
    const int64_t *GB_RESTRICT Bh = B->h ;
    bool B_is_hyper = B->is_hyper ;
    ASSERT (!B->is_slice) ;

    int64_t Mnvec = 0 ;
    const int64_t *GB_RESTRICT Mp = NULL ;
    const int64_t *GB_RESTRICT Mh = NULL ;
    bool M_is_hyper = false ;
    if (M != NULL)
    { 
        Mnvec = M->nvec ;
        Mp = M->p ;
        Mh = M->h ;
        M_is_hyper = M->is_hyper ;
        ASSERT (!M->is_slice) ;
    }

    // For GB_add, if M is present, hypersparse, and not complemented, then C
    // will be hypersparse, and it will have set of vectors as M (Ch == Mh).
    // For GB_masker, Ch is never equal to Mh.
    bool Ch_is_Mh = (p_Ch_is_Mh != NULL) && (M != NULL && M_is_hyper) ;

    //--------------------------------------------------------------------------
    // find the set union of the non-empty vectors of A and B
    //--------------------------------------------------------------------------

    if (Ch_is_Mh)
    {

        //----------------------------------------------------------------------
        // C is hypersparse, with the same vectors as the hypersparse M
        //----------------------------------------------------------------------

        // This step is done for GB_add only, not GB_masker.
        // GB_wait is the only place where A may be a slice, and it does not
        // use a mask.  So this phase can ignore the case where A is a slice.

        Cnvec = Mnvec ;
        nthreads = GB_nthreads (Cnvec, chunk, nthreads_max) ;

        ASSERT (!A_is_slice) ;

        if (!GB_allocate_result (Cnvec, &Ch, NULL,
            (A_is_hyper) ? (&C_to_A) : NULL, (B_is_hyper) ? (&C_to_B) : NULL))
        { 
            // out of memory
            GB_FREE_WORK ;
            return (GB_OUT_OF_MEMORY) ;
        }

        // copy Mh into Ch.  Ch is Mh so C_to_M is not needed.
        GB_memcpy (Ch, Mh, Mnvec * sizeof (int64_t), nthreads) ;

        // construct the mapping from C to A and B, if they are hypersparse
        if (A_is_hyper || B_is_hyper)
        {
            int64_t k ;
            #pragma omp parallel for num_threads(nthreads) schedule(static)
            for (k = 0 ; k < Cnvec ; k++)
            {
                int64_t j = Ch [k] ;
                if (A_is_hyper)
                { 
                    // C_to_A [k] = kA if Ah [kA] == j and A(:,j) is non-empty
                    int64_t kA = 0, pA, pA_end ;
                    GB_lookup (true, Ah, Ap, &kA, Anvec-1, j, &pA, &pA_end) ;
                    C_to_A [k] = (pA < pA_end) ? kA : -1 ;
                }
                if (B_is_hyper)
                { 
                    // C_to_B [k] = kB if Bh [kB] == j and B(:,j) is non-empty
                    int64_t kB = 0, pB, pB_end ;
                    GB_lookup (true, Bh, Bp, &kB, Bnvec-1, j, &pB, &pB_end) ;
                    C_to_B [k] = (pB < pB_end) ? kB : -1 ;
                }
            }
        }

    }
    else if ((A_is_hyper || A_is_slice) && B_is_hyper)
    {

        //----------------------------------------------------------------------
        // A is hypersparse or a hyperslice, and B is hypersparse
        //----------------------------------------------------------------------

        // Ch is the set union of Ah and Bh.  This is handled with a parallel
        // merge, since Ah and Bh are both sorted lists.

        //----------------------------------------------------------------------
        // phase 0: create the tasks
        //----------------------------------------------------------------------

        double work = GB_IMIN (Anvec + Bnvec, n) ;
        nthreads = GB_nthreads (work, chunk, nthreads_max) ;

        ntasks = (nthreads == 1) ? 1 : (64 * nthreads) ;
        ntasks = GB_IMIN (ntasks, work) ;

        // allocate workspace
        GB_MALLOC_MEMORY (kA_start, ntasks+1, sizeof (int64_t)) ;
        GB_MALLOC_MEMORY (kB_start, ntasks+1, sizeof (int64_t)) ;
        GB_MALLOC_MEMORY (kC_start, ntasks+1, sizeof (int64_t)) ;
        if (kA_start == NULL || kB_start == NULL || kC_start == NULL)
        {
            // out of memory
            GB_FREE_WORK ;
            return (GB_OUT_OF_MEMORY) ;
        }

        kA_start [0] = (Anvec == 0) ? -1 : 0 ;
        kB_start [0] = (Bnvec == 0) ? -1 : 0 ;
        kA_start [ntasks] = (Anvec == 0) ? -1 : Anvec ;
        kB_start [ntasks] = (Bnvec == 0) ? -1 : Bnvec ;

        for (int taskid = 1 ; taskid < ntasks ; taskid++)
        { 
            // create tasks: A and B are both hyper
            double target_work = ((ntasks-taskid) * work) / ntasks ;
            GB_slice_vector (NULL, NULL,
                &(kA_start [taskid]), &(kB_start [taskid]),
                0, 0, NULL,                 // Mi not present
                0, Anvec, Ah, A_hfirst,     // Ah, explicit or implicit list
                0, Bnvec, Bh,               // Bh, explicit list
                n,                          // Ah and Bh have dimension n
                target_work) ;
        }

        //----------------------------------------------------------------------
        // phase 1: count the entries in the result of each task
        //----------------------------------------------------------------------

        int taskid ;
        #pragma omp parallel for num_threads(nthreads) schedule (dynamic,1)
        for (taskid = 0 ; taskid < ntasks ; taskid++)
        {
            // merge Ah and Bh into Ch
            int64_t kA = kA_start [taskid] ;
            int64_t kB = kB_start [taskid] ;
            int64_t kA_end = kA_start [taskid+1] ;
            int64_t kB_end = kB_start [taskid+1] ;
            int64_t kC = 0 ;
            for ( ; kA < kA_end && kB < kB_end ; kC++)
            {
                int64_t jA = GB_Ah (kA) ;
                int64_t jB = Bh [kB] ;
                if (jA < jB)
                { 
                    // jA appears in A but not B
                    kA++ ;
                }
                else if (jB < jA)
                { 
                    // jB appears in B but not A
                    kB++ ;
                }
                else
                { 
                    // j = jA = jB appears in both A and B
                    kA++ ;
                    kB++ ;
                }
            }
            kC_start [taskid] = kC + (kA_end - kA) + (kB_end - kB) ;
        }

        //----------------------------------------------------------------------
        // phase 1b: cumulative sum of entries for each task
        //----------------------------------------------------------------------

        GB_cumsum (kC_start, ntasks, NULL, 1) ;
        Cnvec = kC_start [ntasks] ;

        //----------------------------------------------------------------------
        // allocate the result
        //----------------------------------------------------------------------

        // C will be hypersparse, so Ch is allocated.  The mask M is ignored
        // for computing Ch.  Ch is the set union of Ah and Bh.

        if (!GB_allocate_result (Cnvec, &Ch,
            (M_is_hyper) ? (&C_to_M) : NULL, &C_to_A, &C_to_B))
        { 
            // out of memory
            GB_FREE_WORK ;
            return (GB_OUT_OF_MEMORY) ;
        }

        //----------------------------------------------------------------------
        // phase 2: compute the result 
        //----------------------------------------------------------------------

        #pragma omp parallel for num_threads(nthreads) schedule (dynamic,1)
        for (taskid = 0 ; taskid < ntasks ; taskid++)
        {
            // merge Ah and Bh into Ch
            int64_t kA = kA_start [taskid] ;
            int64_t kB = kB_start [taskid] ;
            int64_t kC = kC_start [taskid] ;
            int64_t kA_end = kA_start [taskid+1] ;
            int64_t kB_end = kB_start [taskid+1] ;

            // merge Ah and Bh into Ch
            for ( ; kA < kA_end && kB < kB_end ; kC++)
            {
                int64_t jA = GB_Ah (kA) ;
                int64_t jB = Bh [kB] ;
                if (jA < jB)
                { 
                    // append jA to Ch
                    Ch     [kC] = jA ;
                    C_to_A [kC] = kA++ ;
                    C_to_B [kC] = -1 ;       // jA does not appear in B
                }
                else if (jB < jA)
                { 
                    // append jB to Ch
                    Ch     [kC] = jB ;
                    C_to_A [kC] = -1 ;       // jB does not appear in A
                    C_to_B [kC] = kB++ ;
                }
                else
                { 
                    // j appears in both A and B; append it to Ch
                    Ch     [kC] = jA ;
                    C_to_A [kC] = kA++ ;
                    C_to_B [kC] = kB++ ;
                }
            }
            if (kA < kA_end)
            {
                // B is exhausted but A is not
                for ( ; kA < kA_end ; kA++, kC++)
                { 
                    // append jA to Ch
                    int64_t jA = GB_Ah (kA) ;
                    Ch     [kC] = jA ;
                    C_to_A [kC] = kA ;
                    C_to_B [kC] = -1 ;
                }
            }
            else if (kB < kB_end)
            {
                // A is exhausted but B is not
                for ( ; kB < kB_end ; kB++, kC++)
                { 
                    // append jB to Ch
                    int64_t jB = Bh [kB] ;
                    Ch     [kC] = jB ;
                    C_to_A [kC] = -1 ;
                    C_to_B [kC] = kB ;
                }
            }
            ASSERT (kC == kC_start [taskid+1]) ;
        }

        //----------------------------------------------------------------------
        // check result via a sequential merge
        //----------------------------------------------------------------------

        #ifdef GB_DEBUG
        // merge Ah and Bh into Ch
        int64_t kA = 0 ;
        int64_t kB = 0 ;
        int64_t kC = 0 ;
        for ( ; kA < Anvec && kB < Bnvec ; kC++)
        {
            int64_t jA = GB_Ah (kA) ;
            int64_t jB = Bh [kB] ;
            if (jA < jB)
            {
                // append jA to Ch
                ASSERT (Ch     [kC] == jA) ;
                ASSERT (C_to_A [kC] == kA) ; kA++ ;
                ASSERT (C_to_B [kC] == -1) ;      // jA does not appear in B
            }
            else if (jB < jA)
            {
                // append jB to Ch
                ASSERT (Ch     [kC] == jB) ;
                ASSERT (C_to_A [kC] == -1) ;       // jB does not appear in A
                ASSERT (C_to_B [kC] == kB) ; kB++ ;
            }
            else
            {
                // j appears in both A and B; append it to Ch
                ASSERT (Ch     [kC] == jA) ;
                ASSERT (C_to_A [kC] == kA) ; kA++ ;
                ASSERT (C_to_B [kC] == kB) ; kB++ ;
            }
        }
        if (kA < Anvec)
        {
            // B is exhausted but A is not
            for ( ; kA < Anvec ; kA++, kC++)
            {
                // append jA to Ch
                int64_t jA = GB_Ah (kA) ;
                ASSERT (Ch     [kC] == jA) ;
                ASSERT (C_to_A [kC] == kA) ;
                ASSERT (C_to_B [kC] == -1) ;
            }
        }
        else if (kB < Bnvec)
        {
            // A is exhausted but B is not
            for ( ; kB < Bnvec ; kB++, kC++)
            {
                // append jB to Ch
                int64_t jB = Bh [kB] ;
                ASSERT (Ch     [kC] == jB) ;
                ASSERT (C_to_A [kC] == -1) ;
                ASSERT (C_to_B [kC] == kB) ;
            }
        }
        ASSERT (kC == Cnvec) ;
        #endif

    }
    else if ((A_is_hyper || A_is_slice) && !B_is_hyper)
    {

        //----------------------------------------------------------------------
        // A is hypersparse, B is standard
        //----------------------------------------------------------------------

        // C will be standard.  Construct the C_to_A mapping.

        Cnvec = n ;
        nthreads = GB_nthreads (Cnvec, chunk, nthreads_max) ;

        if (!GB_allocate_result (Cnvec, NULL,
            (M_is_hyper) ? (&C_to_M) : NULL, &C_to_A, NULL))
        { 
            // out of memory
            GB_FREE_WORK ;
            return (GB_OUT_OF_MEMORY) ;
        }

        int64_t j ;
        #pragma omp parallel for num_threads(nthreads) schedule(static)
        for (j = 0 ; j < n ; j++)
        { 
            C_to_A [j] = -1 ;
        }

        // scatter Ah into C_to_A
        int64_t kA ;
        #pragma omp parallel for num_threads(nthreads) schedule(static)
        for (kA = 0 ; kA < Anvec ; kA++)
        { 
            int64_t jA = GB_Ah (kA) ;
            C_to_A [jA] = kA ;
        }

    }
    else if (!(A_is_hyper || A_is_slice) && B_is_hyper)
    {

        //----------------------------------------------------------------------
        // A is standard, B is hypersparse
        //----------------------------------------------------------------------

        // C will be standard.  Construct the C_to_B mapping.

        Cnvec = n ;
        nthreads = GB_nthreads (Cnvec, chunk, nthreads_max) ;

        if (!GB_allocate_result (Cnvec, NULL,
            (M_is_hyper) ? (&C_to_M) : NULL, NULL, &C_to_B))
        { 
            // out of memory
            GB_FREE_WORK ;
            return (GB_OUT_OF_MEMORY) ;
        }

        int64_t j ;
        #pragma omp parallel for num_threads(nthreads) schedule(static)
        for (j = 0 ; j < n ; j++)
        { 
            C_to_B [j] = -1 ;
        }

        // scatter Bh into C_to_B
        int64_t kB ;
        #pragma omp parallel for num_threads(nthreads) schedule(static)
        for (kB = 0 ; kB < Bnvec ; kB++)
        { 
            int64_t jB = Bh [kB] ;
            C_to_B [jB] = kB ;
        }

    }
    else
    {

        //----------------------------------------------------------------------
        // A and B are both standard
        //----------------------------------------------------------------------

        // C will be standard

        Cnvec = n ;
        nthreads = GB_nthreads (Cnvec, chunk, nthreads_max) ;

        if (!GB_allocate_result (Cnvec, NULL,
            (M_is_hyper) ? (&C_to_M) : NULL, NULL, NULL))
        { 
            // out of memory
            GB_FREE_WORK ;
            return (GB_OUT_OF_MEMORY) ;
        }

    }

    //--------------------------------------------------------------------------
    // construct C_to_M if needed
    //--------------------------------------------------------------------------

    if (C_to_M != NULL)
    {
        if (Ch != NULL)
        {
            // C is hypersparse
            int64_t k ;
            #pragma omp parallel for num_threads(nthreads) schedule(static)
            for (k = 0 ; k < Cnvec ; k++)
            { 
                int64_t j = Ch [k] ;
                // C_to_M [k] = kM if Mh [kM] == j and M(:,j) is non-empty
                int64_t kM = 0, pM, pM_end ;
                GB_lookup (true, Mh, Mp, &kM, Mnvec-1, j, &pM, &pM_end) ;
                C_to_M [k] = (pM < pM_end) ? kM : -1 ;
            }
        }
        else
        {
            // C is standard
            int64_t j ;
            #pragma omp parallel for num_threads(nthreads) schedule(static)
            for (j = 0 ; j < n ; j++)
            { 
                C_to_M [j] = -1 ;
            }
            // scatter Mh into C_to_M
            int64_t kM ;
            #pragma omp parallel for num_threads(nthreads) schedule(static)
            for (kM = 0 ; kM < Mnvec ; kM++)
            { 
                int64_t jM = Mh [kM] ;
                C_to_M [jM] = kM ;
            }
        }
    }

    //--------------------------------------------------------------------------
    // return result
    //--------------------------------------------------------------------------

    (*p_Cnvec      ) = Cnvec ;
    if (p_Ch_is_Mh != NULL)
    { 
        // return Ch_is_Mh to GB_add.  For GB_masker, Ch is never Mh.
        (*p_Ch_is_Mh) = Ch_is_Mh ;
    }
    (*Ch_handle    ) = Ch ;
    (*C_to_A_handle) = C_to_A ;
    (*C_to_B_handle) = C_to_B ;
    if (C_to_M_handle != NULL)
    { 
        (*C_to_M_handle) = C_to_M ;
    }

    //--------------------------------------------------------------------------
    // The code below describes what the output contains:
    //--------------------------------------------------------------------------

    #ifdef GB_DEBUG
    ASSERT (A != NULL) ;        // A and B are always present
    ASSERT (B != NULL) ;
    int64_t jlast = -1 ;
    for (int64_t k = 0 ; k < Cnvec ; k++)
    {

        // C(:,j) is in the list, as the kth vector
        int64_t j ;
        if (Ch == NULL)
        {
            // C will be constructed as standard sparse
            j = k ;
        }
        else
        {
            // C will be constructed as hypersparse
            j = Ch [k] ;
        }

        // vectors j in Ch are sorted, and in the range 0:n-1
        ASSERT (j >= 0 && j < n) ;
        ASSERT (j > jlast) ;
        jlast = j ;

        // see if A (:,j) exists
        if (C_to_A != NULL)
        {
            // A is hypersparse, or a slice
            ASSERT (A->is_hyper || A->is_slice) ;
            int64_t kA = C_to_A [k] ;
            ASSERT (kA >= -1 && kA < A->nvec) ;
            if (kA >= 0)
            {
                int64_t jA = GB_Ah (kA) ;
                ASSERT (j == jA) ;
            }
        }
        else
        {
            // A is in standard sparse form
            // C_to_A exists only if A is hypersparse
            ASSERT (!(A->is_hyper || A->is_slice)) ;
        }

        // see if B (:,j) exists
        if (C_to_B != NULL)
        {
            // B is hypersparse
            ASSERT (B->is_hyper) ;
            int64_t kB = C_to_B [k] ;
            ASSERT (kB >= -1 && kB < B->nvec) ;
            if (kB >= 0)
            {
                int64_t jB = B->h [kB] ;
                ASSERT (j == jB) ;
            }
        }
        else
        {
            // B is in standard sparse form
            // C_to_B exists only if B is hypersparse
            ASSERT (!B->is_hyper) ;
        }

        // see if M (:,j) exists
        if (Ch_is_Mh)
        {
            // Ch is the same as Mh
            ASSERT (M != NULL) ;
            ASSERT (M->is_hyper) ;
            ASSERT (Ch != NULL && M->h != NULL && Ch [k] == M->h [k]) ;
            ASSERT (C_to_M == NULL) ;
        }
        else if (C_to_M != NULL)
        {
            // M is present and hypersparse
            ASSERT (M != NULL) ;
            ASSERT (M->is_hyper) ;
            int64_t kM = C_to_M [k] ;
            ASSERT (kM >= -1 && kM < M->nvec) ;
            if (kM >= 0)
            {
                int64_t jM = M->h [kM] ;
                ASSERT (j == jM) ;
            }
        }
        else
        {
            // M is not present, or in standard form
            ASSERT (M == NULL || !(M->is_hyper)) ;
        }
    }
    #endif

    //--------------------------------------------------------------------------
    // free workspace and return result
    //--------------------------------------------------------------------------

    GB_FREE_WORK ;
    return (GrB_SUCCESS) ;
}

