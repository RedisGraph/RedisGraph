//------------------------------------------------------------------------------
// GB_AxB_flopcount:  compute flops for C=A*B, C<M>=A*B, or C<!M>=A*B
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// On input, A, B, and M (optional) are matrices for C=A*B, C<M>=A*B, or
// C<!M>=A*B.  The flop count for each B(:,j) is computed, and returned as a
// cumulative sum.  This function is CSR/CSC agnostic, but for simplicity of
// this description, assume A and B are both CSC matrices, so that ncols(A) ==
// nrows(B).  For both CSR and CSC, A->vdim == B->vlen holds.  A and/or B may
// be hypersparse, in any combination.

// Bflops has size (B->nvec)+1, for both standard and hypersparse B.  Let
// n=B->vdim be the column dimension of B (that is, B is m-by-n).

// If B is a standard CSC matrix then Bflops has size n+1 == B->nvec+1, and on
// output, Bflops [j] is the # of flops required to compute C (:, 0:j-1).  B->h
// is NULL, and is implicitly the vector 0:(n-1).

// If B is hypersparse, then let Bh = B->h.  Its size is B->nvec, and j = Bh
// [kk] is the (kk)th column in the data structure for B.  C will also be
// hypersparse, and only C(:,Bh) will be computed (C may have fewer non-empty
// columns than B).  On output, Bflops [kk] is the number of needed flops to
// compute C (:, Bh [0:kk-1]).

// In both cases, Bflops [0] = 0, and Bflops [B->nvec] = total number of flops.
// The size of Bflops is B->nvec+1 so that it has the same size as B->p.  The
// first entry of B->p and Bflops are both zero.  This allows B to be sliced
// either by # of entries in B (by slicing B->p) or by the flop count required
// (by slicing Bflops).

// This algorithm does not look at the values of M, A, or B, just their
// patterns.  The flop count of C=A*B, C<M>=A*B, or C<!M>=A*B is computed for a
// saxpy-based method; the work for A'*B for the dot product method is not
// computed.

// The algorithm scans all nonzeros in B.  It only scans at most the min and
// max (first and last) row indices in A and M (if M is present).  If A and M
// are not hypersparse, the time taken is O(nnz(B)+n).  If all matrices are
// hypersparse, the time is O(nnz(B)*log(h)) where h = max # of vectors present
// in A and M.  In pseudo-MATLAB, and assuming B is in standard (not
// hypersparse) form:

/*
    [m n] = size (B) ;
    Bflops = zeros (1,n+1) ;        % (set to zero in the caller)
    Mwork = 0 ;
    for each column j in B:
        if (B (:,j) is empty) continue ;
        mjnz = nnz (M (:,j))
        if (M is present, not complemented, and M (:,j) is empty) continue ;
        im_first = min row index of nonzeros in M(:,j)
        im_last  = max row index of nonzeros in M(:,j)
        Bflops (j) = mjnz if M present, to scatter M(:,j) (M or !M case)
        Mwork += mjnz
        for each k where B (k,j) is nonzero:
            aknz = nnz (A (:,k))
            if (aknz == 0) continue ;
            alo = min row index of nonzeros in A(:,k)
            ahi = max row index of nonzeros in A(:,k)
            if (M is present and not complemented)
                if (intersection (alo:ahi, im_first:im_last) empty) continue
            end
            % numerical phase will compute: C(:,j)<#M(:,j)> += A(:,k)*B(k,j)
            % where #M is no mask, M, or !M.  This typically takes aknz flops,
            % or with a binary search if nnz(M(:,j)) << nnz(A(:,k)).
            Bflops (j) += aknz
        end
    end
*/ 

#include "GB_mxm.h"
#include "GB_ek_slice.h"
#include "GB_bracket.h"

#define GB_FREE_WORK                                                        \
{                                                                           \
    GB_ek_slice_free (&pstart_slice, &kfirst_slice, &klast_slice, ntasks) ; \
    GB_FREE_MEMORY (Wfirst, ntasks, sizeof (int64_t)) ;                     \
    GB_FREE_MEMORY (Wlast,  ntasks, sizeof (int64_t)) ;                     \
}

GrB_Info GB_AxB_flopcount
(
    int64_t *Mwork,             // amount of work to handle the mask M
    int64_t *Bflops,            // size B->nvec+1 and all zero
    const GrB_Matrix M,         // optional mask matrix
    const bool Mask_comp,       // if true, mask is complemented
    const GrB_Matrix A,
    const GrB_Matrix B,
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT_MATRIX_OK_OR_NULL (M, "M for flop count A*B", GB0) ;
    ASSERT_MATRIX_OK (A, "A for flop count A*B", GB0) ;
    ASSERT_MATRIX_OK (B, "B for flop count A*B", GB0) ;
    ASSERT (!GB_PENDING (M)) ; ASSERT (!GB_ZOMBIES (M)) ;
    ASSERT (!GB_PENDING (A)) ; ASSERT (!GB_ZOMBIES (A)) ;
    ASSERT (!GB_PENDING (B)) ; ASSERT (!GB_ZOMBIES (B)) ;
    ASSERT (A->vdim == B->vlen) ;
    ASSERT (Bflops != NULL) ;
    ASSERT (Mwork != NULL) ;

    //--------------------------------------------------------------------------
    // determine the number of threads to use
    //--------------------------------------------------------------------------

    int64_t bnz = GB_NNZ (B) ;
    int64_t bnvec = B->nvec ;

    GB_GET_NTHREADS_MAX (nthreads_max, chunk, Context) ;
    int nthreads = GB_nthreads (bnz + bnvec, chunk, nthreads_max) ;

    #ifdef GB_DEBUG
    // Bflops must be set to zero in the caller
    for (int64_t kk = 0 ; kk <= bnvec ; kk++)
    {
        ASSERT (Bflops [kk] == 0) ;
    }
    #endif

    //--------------------------------------------------------------------------
    // get the mask, if present
    //--------------------------------------------------------------------------

    bool mask_is_M = (M != NULL && !Mask_comp) ;
    const int64_t *GB_RESTRICT Mh = NULL ;
    const int64_t *GB_RESTRICT Mp = NULL ;
    const int64_t *GB_RESTRICT Mi = NULL ;
    int64_t mnvec = 0 ;
    bool M_is_hyper = GB_IS_HYPER (M) ;
    if (M != NULL)
    { 
        Mh = M->h ;
        Mp = M->p ;
        Mi = M->i ;
        mnvec = M->nvec ;
    }

    //--------------------------------------------------------------------------
    // get A and B
    //--------------------------------------------------------------------------

    const int64_t *GB_RESTRICT Ah = A->h ;
    const int64_t *GB_RESTRICT Ap = A->p ;
    const int64_t *GB_RESTRICT Ai = A->i ;
    int64_t anvec = A->nvec ;
    bool A_is_hyper = GB_IS_HYPER (A) ;

    const int64_t *GB_RESTRICT Bh = B->h ;
    const int64_t *GB_RESTRICT Bp = B->p ;
    const int64_t *GB_RESTRICT Bi = B->i ;
    bool B_is_hyper = GB_IS_HYPER (B) ;

    //--------------------------------------------------------------------------
    // construct the parallel tasks
    //--------------------------------------------------------------------------

    // taskid does entries pstart_slice [taskid] to pstart_slice [taskid+1]-1
    // and vectors kfirst_slice [taskid] to klast_slice [taskid].  The first
    // and last vectors may be shared with prior slices and subsequent slices.

    int64_t *GB_RESTRICT Wfirst = NULL ;       // size ntasks
    int64_t *GB_RESTRICT Wlast = NULL ;        // size ntasks

    int ntasks = (nthreads == 1) ? 1 : (64 * nthreads) ;
    ntasks = GB_IMIN (ntasks, bnz) ;
    ntasks = GB_IMAX (ntasks, 1) ;
    int64_t *pstart_slice, *kfirst_slice, *klast_slice ;
    if (!GB_ek_slice (&pstart_slice, &kfirst_slice, &klast_slice, B, ntasks))
    { 
        // out of memory
        GB_FREE_WORK ;
        return (GB_OUT_OF_MEMORY) ;
    }

    //--------------------------------------------------------------------------
    // allocate workspace
    //--------------------------------------------------------------------------

    GB_MALLOC_MEMORY (Wfirst, ntasks, sizeof (int64_t)) ;
    GB_MALLOC_MEMORY (Wlast,  ntasks, sizeof (int64_t)) ;
    if (Wfirst == NULL || Wlast == NULL)
    { 
        // out of memory
        GB_FREE_WORK ;
        return (GB_OUT_OF_MEMORY) ;
    }

    //--------------------------------------------------------------------------
    // compute flop counts for C=A*B, C<M>=A*B, or C<!M>=A*B
    //--------------------------------------------------------------------------

    int64_t total_Mwork = 0 ;
    int taskid ;
    #pragma omp parallel for num_threads(nthreads) schedule(dynamic,1) \
        reduction(+:total_Mwork)
    for (taskid = 0 ; taskid < ntasks ; taskid++)
    {

        //----------------------------------------------------------------------
        // get the task descriptor
        //----------------------------------------------------------------------

        int64_t kfirst = kfirst_slice [taskid] ;
        int64_t klast  = klast_slice  [taskid] ;
        Wfirst [taskid] = 0 ;
        Wlast  [taskid] = 0 ;
        int64_t mpleft = 0 ;     // for GB_lookup of the mask M
        int64_t task_Mwork = 0 ;

        //----------------------------------------------------------------------
        // count flops for vectors kfirst to klast of B
        //----------------------------------------------------------------------

        for (int64_t kk = kfirst ; kk <= klast ; kk++)
        {

            // nnz (B (:,j)), for all tasks
            int64_t bjnz = Bp [kk+1] - Bp [kk] ;
            // C(:,j) is empty if the entire vector B(:,j) is empty
            if (bjnz == 0) continue ;

            //------------------------------------------------------------------
            // find the part of B(:,j) to be computed by this task
            //------------------------------------------------------------------

            int64_t pB, pB_end ;
            GB_get_pA_and_pC (&pB, &pB_end, NULL,
                taskid, kk, kfirst, klast, pstart_slice, NULL, NULL, Bp) ;
            int64_t my_bjnz = pB_end - pB ;
            int64_t j = (B_is_hyper) ? Bh [kk] : kk ;

            //------------------------------------------------------------------
            // see if M(:,j) is present and non-empty
            //------------------------------------------------------------------

            int64_t bjflops = 0 ;
            int64_t im_first = -1, im_last = -1 ;
            int64_t mjnz = 0 ;
            if (M != NULL)
            {
                int64_t mpright = mnvec - 1 ;
                int64_t pM, pM_end ;
                GB_lookup (M_is_hyper, Mh, Mp, &mpleft, mpright, j,
                    &pM, &pM_end) ;
                mjnz = pM_end - pM ;
                // If M not complemented: C(:,j) is empty if M(:,j) is empty.
                if (mjnz == 0 && !Mask_comp) continue ;
                if (mjnz > 0)
                {
                    // M(:,j) not empty; get 1st and last index in M(:,j)
                    im_first = Mi [pM] ;
                    im_last  = Mi [pM_end-1] ;
                    if (pB == Bp [kk])
                    { 
                        // this task owns the top part of B(:,j), so it can
                        // account for the work to access M(:,j), without the
                        // work being duplicated by other tasks working on
                        // B(:,j)
                        bjflops = mjnz ;
                        // keep track of total work spent examining the mask.
                        // If any B(:,j) is empty, M(:,j) can be ignored.  So
                        // total_Mwork will be <= nnz (M).
                        task_Mwork += mjnz ;
                    }
                }
            }
            int64_t mjnz_much = 64 * mjnz ;

            //------------------------------------------------------------------
            // trim Ah on right
            //------------------------------------------------------------------

            // Ah [0..A->nvec-1] holds the set of non-empty vectors of A, but
            // only vectors k corresponding to nonzero entries B(k,j) are
            // accessed for this vector B(:,j).  If nnz (B(:,j)) > 2, prune the
            // search space on the right, so the remaining calls to GB_lookup
            // will only need to search Ah [pleft...pright-1].  pright does not
            // change.  pleft is advanced as B(:,j) is traversed, since the
            // indices in B(:,j) are sorted in ascending order.

            int64_t pleft = 0 ;
            int64_t pright = anvec-1 ;
            if (A_is_hyper && my_bjnz > 2)
            { 
                // trim Ah [0..pright] to remove any entries past last B(:,j)
                GB_bracket_right (Bi [pB_end-1], Ah, 0, &pright) ;
            }

            //------------------------------------------------------------------
            // count the flops to compute C(:,j)<#M(:,j)> = A*B(:,j)
            //------------------------------------------------------------------

            // where #M is either not present, M, or !M

            for ( ; pB < pB_end ; pB++)
            {
                // B(k,j) is nonzero
                int64_t k = Bi [pB] ;

                // find A(:,k), reusing pleft since Bi [...] is sorted
                int64_t pA, pA_end ;
                GB_lookup (A_is_hyper, Ah, Ap, &pleft, pright, k, &pA, &pA_end);

                // skip if A(:,k) empty
                int64_t aknz = pA_end - pA ;
                if (aknz == 0) continue ;

                double bkjflops ;

                // skip if intersection of A(:,k) and M(:,j) is empty
                // and mask is not complemented (C<M>=A*B)
                if (mask_is_M)
                {
                    // A(:,k) is non-empty; get first and last index of A(:,k)
                    int64_t alo = Ai [pA] ;
                    int64_t ahi = Ai [pA_end-1] ;
                    if (ahi < im_first || alo > im_last) continue ;
                    if (aknz > 256 && mjnz_much < aknz)
                    { 
                        // scan M(:j), and do binary search for A(i,j)
                        bkjflops = mjnz * (1 + 4 * log2 ((double) aknz)) ;
                    }
                    else
                    { 
                        // scan A(:k), and lookup M(i,j)
                        bkjflops = aknz ;
                    }
                }
                else
                { 
                    // A(:,k)*B(k,j) requires aknz flops
                    bkjflops = aknz ;
                }

                // increment by flops for the single entry B(k,j)
                // C(:,j)<#M(:,j)> += A(:,k)*B(k,j).
                bjflops += bkjflops ;
            }

            //------------------------------------------------------------------
            // log the flops for B(:,j)
            //------------------------------------------------------------------

            if (kk == kfirst)
            { 
                Wfirst [taskid] = bjflops ;
            }
            else if (kk == klast)
            { 
                Wlast [taskid] = bjflops ;
            }
            else
            { 
                Bflops [kk] = bjflops ;
            }
        }

        // compute the total work to access the mask, which is <= nnz (M)
        total_Mwork += task_Mwork ;
    }

    //--------------------------------------------------------------------------
    // reduce the first and last vector of each slice
    //--------------------------------------------------------------------------

    // See also Template/GB_reduce_each_vector.c

    int64_t kprior = -1 ;

    for (int taskid = 0 ; taskid < ntasks ; taskid++)
    {

        //----------------------------------------------------------------------
        // sum up the partial flops that taskid computed for kfirst
        //----------------------------------------------------------------------

        int64_t kfirst = kfirst_slice [taskid] ;
        int64_t klast  = klast_slice  [taskid] ;

        if (kfirst <= klast)
        {
            int64_t pB = pstart_slice [taskid] ;
            int64_t pB_end =
                GB_IMIN (Bp [kfirst+1], pstart_slice [taskid+1]) ;
            if (pB < pB_end)
            {
                if (kprior < kfirst)
                { 
                    // This task is the first one that did work on
                    // B(:,kfirst), so use it to start the reduction.
                    Bflops [kfirst] = Wfirst [taskid] ;
                }
                else
                { 
                    // subsequent task for B(:,kfirst)
                    Bflops [kfirst] += Wfirst [taskid] ;
                }
                kprior = kfirst ;
            }
        }

        //----------------------------------------------------------------------
        // sum up the partial flops that taskid computed for klast
        //----------------------------------------------------------------------

        if (kfirst < klast)
        {
            int64_t pB = Bp [klast] ;
            int64_t pB_end   = pstart_slice [taskid+1] ;
            if (pB < pB_end)
            {
                /* if */ ASSERT (kprior < klast) ;
                { 
                    // This task is the first one that did work on
                    // B(:,klast), so use it to start the reduction.
                    Bflops [klast] = Wlast [taskid] ;
                }
                /*
                else
                {
                    // If kfirst < klast and B(:,klast) is not empty,
                    // then this task is always the first one to do
                    // work on B(:,klast), so this case is never used.
                    ASSERT (GB_DEAD_CODE) ;
                    // subsequent task to work on B(:,klast)
                    Bflops [klast] += Wlast [taskid] ;
                }
                */
                kprior = klast ;
            }
        }
    }

    //--------------------------------------------------------------------------
    // cumulative sum of Bflops
    //--------------------------------------------------------------------------

    // Bflops = cumsum ([0 Bflops]) ;
    ASSERT (Bflops [bnvec] == 0) ;
    GB_cumsum (Bflops, bnvec, NULL, nthreads) ;
    // Bflops [bnvec] is now the total flop count, including the time to
    // compute A*B and to handle the mask.  total_Mwork is part of this total
    // flop count, but is also returned separtely.

    //--------------------------------------------------------------------------
    // free workspace and return result
    //--------------------------------------------------------------------------

    GB_FREE_WORK ;
    (*Mwork) = total_Mwork ;
    return (GrB_SUCCESS) ;
}

