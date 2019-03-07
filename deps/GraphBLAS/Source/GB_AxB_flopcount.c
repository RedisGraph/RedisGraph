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
    Bflops = zeros (1,n+1) ;
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
        end
    end
*/ 

// If Bflops is NULL, then this function is being called by a single thread.
// Bflops is not computed.  Instead, the total_flops are computed, and the
// function returns just the result of the test (total_flops <= floplimit).
// total_flops is not returned, just the true/false result of the test.  This
// allows the function to return early, once the total_flops exceeds the
// threshold.

// PARALLEL: easy, one simple for-all loop, no dependencies

#include "GB.h"

bool GB_AxB_flopcount           // compute flops for C<M>=A*B or C=A*B
(
    int64_t *Bflops,            // size B->nvec+1, if present
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

    GB_GET_NTHREADS (nthreads, Context) ;
    if (Bflops == NULL)
    {
        // a single thread is testing if (total_flops <= floplimit)
        ASSERT (Context == NULL) ;
        ASSERT (nthreads == 1) ;
    }

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
    int64_t mpleft = 0 ;            // mpleft is modified below
    int64_t mpright = mnvec - 1 ;

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
    int64_t bnvec = B->nvec ;
    bool B_is_hyper = GB_IS_HYPER (B) ;

    //--------------------------------------------------------------------------
    // compute flop counts for C<M> = A*B
    //--------------------------------------------------------------------------

    // for each column of B:
    // PARALLEL: no dependencies in this loop
    // mpleft is threadprivate

    // total_flops is only needed if Bflops is NULL, and in that case,
    // nthreads is 1.
    int64_t total_flops = 0 ;

    for (int64_t kk = 0 ; kk < bnvec ; kk++)
    {

        // The (kk)th iteration of this loop computes Bflops [kk], if not NULL.
        // All iterations are completely independent.

        //----------------------------------------------------------------------
        // get B(:,j)
        //----------------------------------------------------------------------

        int64_t j = (B_is_hyper) ? Bh [kk] : kk ;
        int64_t pB     = Bp [kk] ;
        int64_t pB_end = Bp [kk+1] ;

        // C(:,j) is empty if B(:,j) is empty
        int64_t bjnz = pB_end - pB ;
        if (bjnz == 0)
        {
            if (Bflops != NULL)
            { 
                Bflops [kk] = 0 ;
            }
            continue ;
        }

        //----------------------------------------------------------------------
        // see if M(:,j) is present and non-empty
        //----------------------------------------------------------------------

        int64_t im_first = -1, im_last = -1 ;
        if (M != NULL)
        {
            // reuse mpleft from the last binary search of M->h, to speed up
            // the search.  This is just a heuristic, and resetting mpleft to
            // zero here would be work too (just more of M->h would be
            // searched; the results would be the same), as in:
            // int64_t mpleft = 0 ;     // this works too
            // To reuse mpleft from its prior iteration, each thread needs its
            // own private mpleft.
            int64_t pM, pM_end ;
            GB_lookup (M_is_hyper, Mh, Mp, &mpleft, mpright, j, &pM, &pM_end) ;
            int64_t mjnz = pM_end - pM ;
            if (mjnz == 0)
            {
                // C(:,j) is empty if M(:,j) is empty
                if (Bflops != NULL)
                { 
                    Bflops [kk] = 0 ;
                }
                continue ;
            }
            // M(:,j) has at least one entry; get 1st and last index in M(:,j)
            im_first = Mi [pM] ;
            im_last  = Mi [pM_end-1] ;
        }

        //----------------------------------------------------------------------
        // trim Ah on right
        //----------------------------------------------------------------------

        // Ah [0..A->nvec-1] holds the set of non-empty vectors of A, but only
        // vectors k corresponding to nonzero entries B(k,j) are accessed for
        // this vector B(:,j).  If nnz (B(:,j)) > 2, prune the search space on
        // the right, so the remaining calls to GB_lookup will only need to
        // search Ah [pleft...pright-1].  pright does not change.  pleft is
        // advanced as B(:,j) is traversed, since the indices in B(:,j) are
        // sorted in ascending order.

        int64_t pleft = 0 ;
        int64_t pright = anvec-1 ;
        if (A_is_hyper && bjnz > 2)
        { 
            // trim Ah [0..pright] to remove any entries past the last B(:,j)
            GB_bracket_right (Bi [pB_end-1], Ah, 0, &pright) ;
        }

        //----------------------------------------------------------------------
        // count the flops to compute C(:,j)<M(:,j)> = A*B(:,j)
        //----------------------------------------------------------------------

        int64_t bjflops = 0 ;

        for ( ; pB < pB_end ; pB++)
        {
            // B(k,j) is nonzero
            int64_t k = Bi [pB] ;

            // find A(:,k), reusing pleft since Bi [...] is sorted
            int64_t pA, pA_end ;
            GB_lookup (A_is_hyper, Ah, Ap, &pleft, pright, k, &pA, &pA_end) ;

            // skip if A(:,k) empty
            int64_t aknz = pA_end - pA ;
            if (aknz == 0) continue ;

            // skip if intersection of A(:,k) and M(:,j) is empty
            if (M != NULL)
            { 
                // A(:,k) is non-empty; get the first and last index of A(:,k)
                int64_t alo = Ai [pA] ;
                int64_t ahi = Ai [pA_end-1] ;
                if (ahi < im_first || alo > im_last) continue ;
            }

            // increment by flops to compute the saxpy operation
            // C(:,j)<M(:,j)> += A(:,k)*B(k,j).
            bjflops += aknz ;

            // check for a quick return
            if (Bflops == NULL)
            {
                // the work is being done by a single thread
                total_flops += aknz ;
                if (total_flops > floplimit)
                { 
                    // quick return:  (total_flops <= floplimit) is false.
                    // total_flops is not returned since it is only partially
                    // computed.  However, it does not exceed the floplimit
                    // threshold, so the result is false.
                    return (false) ;
                }
            }
        }

        if (Bflops != NULL)
        { 
            Bflops [kk] = bjflops ;
        }
    }

    //--------------------------------------------------------------------------
    // return result
    //--------------------------------------------------------------------------

    if (Bflops == NULL)
    { 
        // total_flops has been fullly computed, but Bflops has not.  Just
        // return the result of the test with the floplimit.
        return (total_flops <= floplimit) ;
    }
    else
    {
        // Bflops = cumsum ([0 Bflops]) ;
        Bflops [bnvec] = 0 ;
        GB_cumsum (Bflops, bnvec, NULL, Context) ;
        // Bflops [bnvec] is now the total flop count
        return (Bflops [bnvec] <= floplimit) ;
    }
}

