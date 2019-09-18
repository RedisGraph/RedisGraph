//------------------------------------------------------------------------------
// GB_AxB_flopcount:  compute flops for C<M>=A*B or C=A*B
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// On input, A and B are two matrices for C<M>=A*B or C=A*B.  The flop count
// for each B(:,j) is computed, and returned as a cumulative sum.  This
// function is CSR/CSC agnostic, but for simplicity of this description, assume
// A and B are both CSC matrices, so that ncols(A) == nrows(B).  For both CSR
// and CSC, A->vdim == B->vlen holds.  A and/or B may be hypersparse, in any
// combination.

// The complemented mask is not handled, so the flops for C<!M>=A*B is not
// computed.

// If present, Bflops has size (B->nvec)+1, for both standard and hypersparse
// B.  Let n = B->vdim be the column dimension of B (that is, B is m-by-n).

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
// patterns.  If the mask is present, it is assumed to not be complemented.
// The flop count of C=A*B or C<M>=A*B is computed for a saxpy-based method;
// the work for A'*B for the dot product method is not computed.

// The algorithm scans all nonzeros in B.  It only scans at most the min and
// max (first and last) row indices in A and M (if M is present).  If A and M
// are not hypersparse, the time taken is O(nnz(B)+n).  If all matrices are
// hypersparse, the time is O(nnz(B)*log(h)) where h = max # of vectors present
// in A and M.  In pseudo-MATLAB, and assuming B is in standard (not
// hypersparse) form:

/*
    [m n] = size (B) ;
    Bflops = zeros (1,n+1) ;        % (set to zero in the caller)
    for each column j in B:
        if (B (:,j) is empty) continue ;
        if (M is present and M (:,j) is empty) continue ;
        im_first = min row index of nonzeros in M(:,j)
        im_last  = max row index of nonzeros in M(:,j)
        for each k where B (k,j) is nonzero:
            aknz = nnz (A (:,k))
            if (aknz == 0) continue ;
            alo = min row index of nonzeros in A(:,k)
            ahi = max row index of nonzeros in A(:,k)
            if (M is present)
                if (intersection (alo:ahi, im_first:im_last) empty) continue
            end
            % numerical phase will compute: C(:,j)<M(:,j)> += A(:,k)*B(k,j),
            % which takes aknz flops, so:
            Bflops (j) += aknz
            Bflops_per_entry (k,j) = aknz
        end
    end
*/ 

// If Bflops and Bflops_per_entry are both NULL, then only the true/false
// result of the test (total_flops <= floplimit) is returned.  This allows the
// function to return early, once the total_flops exceeds the threshold.

#include "GB_mxm.h"
#include "GB_ek_slice.h"
#include "GB_bracket.h"

bool GB_AxB_flopcount           // compute flops for C<M>=A*B or C=A*B
(
    int64_t *Bflops,            // size B->nvec+1 and all zero, if present
    int64_t *Bflops_per_entry,  // size nnz(B)+1 and all zero, if present
    const GrB_Matrix M,         // optional mask matrix
    const GrB_Matrix A,
    const GrB_Matrix B,
    int64_t floplimit,          // maximum flops to compute if Bflops NULL
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT_OK_OR_NULL (GB_check (M, "M for flop count A*B", GB0)) ;
    ASSERT_OK (GB_check (A, "A for flop count A*B", GB0)) ;
    ASSERT_OK (GB_check (B, "B for flop count A*B", GB0)) ;
    ASSERT (!GB_PENDING (M)) ; ASSERT (!GB_ZOMBIES (M)) ;
    ASSERT (!GB_PENDING (A)) ; ASSERT (!GB_ZOMBIES (A)) ;
    ASSERT (!GB_PENDING (B)) ; ASSERT (!GB_ZOMBIES (B)) ;
    ASSERT (A->vdim == B->vlen) ;

    //--------------------------------------------------------------------------
    // determine the number of threads to use
    //--------------------------------------------------------------------------

    int64_t bnz = GB_NNZ (B) ;
    int64_t bnvec = B->nvec ;

    GB_GET_NTHREADS_MAX (nthreads_max, chunk, Context) ;
    int nthreads = GB_nthreads (bnz + bnvec, chunk, nthreads_max) ;

    //--------------------------------------------------------------------------
    // determine the kind of result to return
    //--------------------------------------------------------------------------

    bool check_quick_return = (Bflops == NULL) && (Bflops_per_entry == NULL) ;

    #ifdef GB_DEBUG
    if (Bflops != NULL)
    {
        // Bflops is set to zero in the calller
        for (int64_t kk = 0 ; kk <= bnvec ; kk++)
        {
            ASSERT (Bflops [kk] == 0) ;
        }
    }
    if (Bflops_per_entry != NULL)
    {
        // Bflops_per_entry is set to zero in the calller
        for (int64_t pB = 0 ; pB <= bnz ; pB++)
        {
            ASSERT (Bflops_per_entry [pB] == 0) ;
        }
    }
    #endif

    //--------------------------------------------------------------------------
    // get the mask, if present
    //--------------------------------------------------------------------------

    const int64_t *restrict Mh = NULL ;
    const int64_t *restrict Mp = NULL ;
    const int64_t *restrict Mi = NULL ;
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

    const int64_t *restrict Ah = A->h ;
    const int64_t *restrict Ap = A->p ;
    const int64_t *restrict Ai = A->i ;
    int64_t anvec = A->nvec ;
    bool A_is_hyper = GB_IS_HYPER (A) ;

    const int64_t *restrict Bh = B->h ;
    const int64_t *restrict Bp = B->p ;
    const int64_t *restrict Bi = B->i ;
    bool B_is_hyper = GB_IS_HYPER (B) ;

    //--------------------------------------------------------------------------
    // construct the parallel tasks
    //--------------------------------------------------------------------------

    // Task tid does entries pstart_slice [tid] to pstart_slice [tid+1]-1
    // and vectors kfirst_slice [tid] to klast_slice [tid].  The first and
    // last vectors may be shared with prior slices and subsequent slices.

    int ntasks = (nthreads == 1) ? 1 : (64 * nthreads) ;
    ntasks = GB_IMIN (ntasks, bnz) ;
    ntasks = GB_IMAX (ntasks, 1) ;
    int64_t pstart_slice [ntasks+1] ;
    int64_t kfirst_slice [ntasks] ;
    int64_t klast_slice  [ntasks] ;

    GB_ek_slice (pstart_slice, kfirst_slice, klast_slice, B, ntasks) ;

    //--------------------------------------------------------------------------
    // compute flop counts for C<M> = A*B
    //--------------------------------------------------------------------------

    int64_t Wfirst [ntasks], Wlast [ntasks], Flops [ntasks+1] ;
    int64_t total_flops = 0 ;

    #pragma omp parallel for num_threads(nthreads) schedule(dynamic,1)
    for (int tid = 0 ; tid < ntasks ; tid++)
    {

        //----------------------------------------------------------------------
        // skip this task if limit already reached
        //----------------------------------------------------------------------

        bool quick_return = false ;
        int64_t flops_so_far = 0 ;
        if (check_quick_return)
        {
            { 
                #pragma omp atomic read
                flops_so_far = total_flops ;
            }
            if (flops_so_far > floplimit) continue ;
        }

        //----------------------------------------------------------------------
        // get the task descriptor
        //----------------------------------------------------------------------

        int64_t kfirst = kfirst_slice [tid] ;
        int64_t klast  = klast_slice  [tid] ;
        int64_t task_flops = 0 ;
        Wfirst [tid] = 0 ;
        Wlast  [tid] = 0 ;
        int64_t mpleft = 0 ;     // for GB_lookup of the mask M

        //----------------------------------------------------------------------
        // count flops for vectors kfirst to klast of B
        //----------------------------------------------------------------------

        for (int64_t kk = kfirst ; !quick_return && (kk <= klast) ; kk++)
        {

            //------------------------------------------------------------------
            // find the part of B(:,j) to be computed by this task
            //------------------------------------------------------------------

            int64_t pB, pB_end ;
            GB_get_pA_and_pC (&pB, &pB_end, NULL,
                tid, kk, kfirst, klast, pstart_slice, NULL, NULL, Bp) ;

            int64_t j = (B_is_hyper) ? Bh [kk] : kk ;

            // C(:,j) is empty if B(:,j) is empty
            int64_t bjnz = pB_end - pB ;
            if (bjnz == 0) continue ;

            //------------------------------------------------------------------
            // see if M(:,j) is present and non-empty
            //------------------------------------------------------------------

            int64_t im_first = -1, im_last = -1 ;
            if (M != NULL)
            { 
                int64_t mpright = mnvec - 1 ;
                int64_t pM, pM_end ;
                GB_lookup (M_is_hyper, Mh, Mp, &mpleft, mpright, j,
                    &pM, &pM_end) ;
                int64_t mjnz = pM_end - pM ;
                // C(:,j) is empty if M(:,j) is empty
                if (mjnz == 0) continue ;
                // M(:,j) has at least 1 entry; get 1st and last index in M(:,j)
                im_first = Mi [pM] ;
                im_last  = Mi [pM_end-1] ;
            }

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
            if (A_is_hyper && bjnz > 2)
            { 
                // trim Ah [0..pright] to remove any entries past last B(:,j)
                GB_bracket_right (Bi [pB_end-1], Ah, 0, &pright) ;
            }

            //------------------------------------------------------------------
            // count the flops to compute C(:,j)<M(:,j)> = A*B(:,j)
            //------------------------------------------------------------------

            int64_t bjflops = 0 ;

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

                // skip if intersection of A(:,k) and M(:,j) is empty
                if (M != NULL)
                { 
                    // A(:,k) is non-empty; get first and last index of A(:,k)
                    int64_t alo = Ai [pA] ;
                    int64_t ahi = Ai [pA_end-1] ;
                    if (ahi < im_first || alo > im_last) continue ;
                }

                // increment by flops for the single entry B(k,j)
                // C(:,j)<M(:,j)> += A(:,k)*B(k,j).
                bjflops += aknz ;

                if (Bflops_per_entry != NULL)
                { 
                    // flops for the single entry, B(k,j)
                    Bflops_per_entry [pB] = aknz ;
                }

                // check for a quick return
                if (check_quick_return)
                {
                    flops_so_far += aknz ;
                    if (flops_so_far > floplimit)
                    { 
                        // flop limit has been reached; terminate this and all
                        // other tasks
                        quick_return = true ;
                        break ;
                    }
                }
            }

            //------------------------------------------------------------------
            // sum up the flops for this task
            //------------------------------------------------------------------

            task_flops += bjflops ;

            //------------------------------------------------------------------
            // log the flops for B(:,j)
            //------------------------------------------------------------------

            if (Bflops != NULL)
            { 
                if (kk == kfirst)
                { 
                    Wfirst [tid] = bjflops ;
                }
                else if (kk == klast)
                { 
                    Wlast [tid] = bjflops ;
                }
                else
                { 
                    Bflops [kk] = bjflops ;
                }
            }
        }

        //----------------------------------------------------------------------
        // log the flops for this task
        //----------------------------------------------------------------------

        Flops [tid] = task_flops ;
        if (check_quick_return)
        { 
            #pragma omp atomic update
            total_flops += task_flops ;
        }
    }

    //--------------------------------------------------------------------------
    // finalize the results
    //--------------------------------------------------------------------------

    bool result ;

    if (check_quick_return)
    { 

        // The only output of this function is the result of this test:
        result = (total_flops <= floplimit) ;

    }
    else
    {

        //----------------------------------------------------------------------
        // cumulative sum of Bflops and Bflops_per_entry
        //----------------------------------------------------------------------

        GB_cumsum (Flops, ntasks, NULL, 1) ;
        int64_t total_flops = Flops [ntasks] ;
        result = (total_flops <= floplimit) ;

        if (Bflops != NULL)
        {

            //------------------------------------------------------------------
            // reduce the first and last vector of each slice
            //------------------------------------------------------------------

            // See also Template/GB_reduce_each_vector.c

            int64_t kprior = -1 ;

            for (int tid = 0 ; tid < ntasks ; tid++)
            {

                //--------------------------------------------------------------
                // sum up the partial flops that task tid computed for kfirst
                //--------------------------------------------------------------

                int64_t kfirst = kfirst_slice [tid] ;
                int64_t klast  = klast_slice  [tid] ;

                if (kfirst <= klast)
                {
                    int64_t pB = pstart_slice [tid] ;
                    int64_t pB_end =
                        GB_IMIN (Bp [kfirst+1], pstart_slice [tid+1]) ;
                    if (pB < pB_end)
                    {
                        if (kprior < kfirst)
                        { 
                            // This task is the first one that did work on
                            // B(:,kfirst), so use it to start the reduction.
                            Bflops [kfirst] = Wfirst [tid] ;
                        }
                        else
                        { 
                            // subsequent task for B(:,kfirst)
                            Bflops [kfirst] += Wfirst [tid] ;
                        }
                        kprior = kfirst ;
                    }
                }

                //--------------------------------------------------------------
                // sum up the partial flops that task tid computed for klast
                //--------------------------------------------------------------

                if (kfirst < klast)
                {
                    int64_t pB = Bp [klast] ;
                    int64_t pB_end   = pstart_slice [tid+1] ;
                    if (pB < pB_end)
                    {
                        /* if */ ASSERT (kprior < klast) ;
                        { 
                            // This task is the first one that did work on
                            // B(:,klast), so use it to start the reduction.
                            Bflops [klast] = Wlast [tid] ;
                        }
                        /*
                        else
                        {
                            // If kfirst < klast and B(:,klast) is not empty,
                            // then this task is always the first one to do
                            // work on B(:,klast), so this case is never used.
                            ASSERT (GB_DEAD_CODE) ;
                            // subsequent task to work on B(:,klast)
                            Bflops [klast] += Wlast [tid] ;
                        }
                        */
                        kprior = klast ;
                    }
                }
            }

            //------------------------------------------------------------------
            // cumulative sum of Bflops
            //------------------------------------------------------------------

            // Bflops = cumsum ([0 Bflops]) ;
            ASSERT (Bflops [bnvec] == 0) ;
            GB_cumsum (Bflops, bnvec, NULL, nthreads) ;
            // Bflops [bnvec] is now the total flop count
            ASSERT (total_flops == Bflops [bnvec]) ;
        }

        if (Bflops_per_entry != NULL)
        { 
            // Bflops_per_entry = cumsum ([0 Bflops_per_entry]) ;
            ASSERT (Bflops_per_entry [bnz] == 0) ;
            GB_cumsum (Bflops_per_entry, bnz, NULL, nthreads) ;
            // Bflops_per_entry [bnz] is now the total flop count
            ASSERT (total_flops == Bflops_per_entry [bnz]) ;
        }
    }

    //--------------------------------------------------------------------------
    // return result
    //--------------------------------------------------------------------------

    return (result) ;
}

