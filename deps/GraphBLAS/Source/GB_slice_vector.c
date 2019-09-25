//------------------------------------------------------------------------------
// GB_slice_vector:  slice a vector for GB_add, GB_emult, and GB_mask
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// A(:,kA) and B(:,kB) are two long vectors that will be added with GB_add,
// GB_emult, or GB_mask, and the work to compute them needs to be split into
// multiple tasks.  They represent the same vector index j, for:

//      C(:,j) = A(:,j) +  B(:,j) in GB_add
//      C(:,j) = A(:,j) .* B(:,j) in GB_emult
//      C(:,j)<M(:,j)> = B(:,j) in GB_mask (A is passed in as the input C)
//      union (A->h, B->h) in GB_add_phase0.

// The vector index j is not needed here.  The vectors kA and kB are not
// required, either; just the positions where the vectors appear in A and B
// (pA_start, pA_end, pB_start, and pB_end).

// This method finds i so that nnz (A (i:end,kA)) + nnz (B (i:end,kB)) is
// roughly equal to target_work.  The entries in A(i:end,kA) start at position
// pA in Ai and Ax, and the entries in B(i:end,kB) start at position pB in Bi
// and Bx.  Once the work is split, pM is found for M(i:end,kM), if the mask M
// is present.

// The lists Ai and Bi can also be any sorted integer array.  This is used by
// GB_add_phase0 to construct the set union of A->h and B->h.  In this case,
// pA_start and pB_start are both zero, and pA_end and pB_end are A->nvec and
// B->nvec, respectively.  A can be a non-hypersparse slice, so that A->h is
// NULL.  In this case, Ai is NULL, and represents the implicit list
// A_hfirst:A_hfirst+pA_end-1, inclusive.

// This macro defines the kth entry in the Ai list, for k = 0 to pA_end-1:
#define GB_Ai(k) ((Ai != NULL) ? Ai [k] : (A_hfirst + (k)))

// If n = A->vlen = B->vlen, anz = nnz (A (:,kA)), and bnz = nnz (B (:,kB)),
// then the total time taken by this function is O(log(n)*(log(anz)+log(bnz))),
// or at most O((log(n)^2)).

#include "GB.h"

void GB_slice_vector
(
    // output: return i, pA, and pB
    int64_t *p_i,                   // work starts at A(i,kA) and B(i,kB)
    int64_t *p_pM,                  // M(i:end,kM) starts at pM
    int64_t *p_pA,                  // A(i:end,kA) starts at pA
    int64_t *p_pB,                  // B(i:end,kB) starts at pB
    // input:
    const int64_t pM_start,         // M(:,kM) starts at pM_start in Mi,Mx
    const int64_t pM_end,           // M(:,kM) ends at pM_end-1 in Mi,Mx
    const int64_t *restrict Mi,     // indices of M (or NULL)
    const int64_t pA_start,         // A(:,kA) starts at pA_start in Ai,Ax
    const int64_t pA_end,           // A(:,kA) ends at pA_end-1 in Ai,Ax
    const int64_t *restrict Ai,     // indices of A
    const int64_t A_hfirst,         // if Ai is an implicit hyperlist
    const int64_t pB_start,         // B(:,kB) starts at pB_start in Bi,Bx
    const int64_t pB_end,           // B(:,kB) ends at pB_end-1 in Bi,Bx
    const int64_t *restrict Bi,     // indices of B
    const int64_t vlen,             // A->vlen and B->vlen
    const double target_work        // target work
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (p_pA != NULL && p_pB != NULL) ;

    //--------------------------------------------------------------------------
    // find i, pA, and pB for the start of this task
    //--------------------------------------------------------------------------

    // search for index i in the range ileft:iright, inclusive
    int64_t ileft  = 0 ;
    int64_t iright = vlen-1 ;
    int64_t i = 0 ;

    int64_t aknz = pA_end - pA_start ;
    int64_t bknz = pB_end - pB_start ;
    int64_t mknz = pM_end - pM_start ;      // zero if M not present

    bool a_empty = (aknz == 0) ;
    bool b_empty = (bknz == 0) ;
    bool m_empty = (mknz == 0) ;
    // printf ("empty a %d b %d m %d\n", a_empty, b_empty, m_empty) ;

    int64_t pM = (m_empty) ? -1 : pM_start ;
    int64_t pA = (a_empty) ? -1 : pA_start ;
    int64_t pB = (b_empty) ? -1 : pB_start ;

    ASSERT (GB_IMPLIES (!b_empty, Bi != NULL)) ;
    ASSERT (GB_IMPLIES (!m_empty, Mi != NULL)) ;

    while (ileft < iright)
    {

        //----------------------------------------------------------------------
        // find the index i in the middle of ileft:iright
        //----------------------------------------------------------------------

        i = (ileft + iright) / 2 ;
        // printf ("   slice vector i "GBd" in ["GBd" to "GBd"]\n", i, ileft,
        //     iright) ;

        //----------------------------------------------------------------------
        // find where i appears in A(:,kA)
        //----------------------------------------------------------------------

        if (a_empty)
        { 
            // Ai is empty so i does not appear
            pA = -1 ;
        }
        else if (Ai == NULL)
        { 
            // Ai is an implicit hyperlist: A_hfirst, A_first+1, ... to
            // A_hfirst + pA_end - 1, inclusive.  No need for a binary search.
            ASSERT (pA_start == 0) ;
            if (i < A_hfirst)
            { 
                // i comes before the first entry, so it does not appear
                pA = 0 ;
            }
            else if (A_hfirst + pA_end - 1 < i)
            { 
                // i comes after the last entry, so it does not appear
                pA = pA_end ;
            }
            else // (A_hfirst <= i && i <= A_hfirst + pA_end - 1)
            { 
                // i is in the implicit hyperlist
                pA = i - A_hfirst ;
                ASSERT (GB_Ai (pA) == i) ;
            }
        }
        else if (aknz == vlen)
        { 
            // A(:,kA) is dense; no need for a binary search
            pA = pA_start + i ;
            ASSERT (GB_Ai (pA) == i) ;
        }
        else
        { 
            // Ai is an explicit integer list, Ai [pA_start:pA_end-1]
            ASSERT (aknz > 0) ;
            pA = pA_start ;
            bool afound ;
            int64_t apright = pA_end - 1 ;
            GB_BINARY_SPLIT_SEARCH (i, Ai, pA, apright, afound) ;
            ASSERT (GB_IMPLIES (afound, GB_Ai (pA) == i)) ;
            ASSERT (pA_start <= pA && pA <= pA_end) ;
        }

        ASSERT (GB_IMPLIES (pA >  pA_start && pA < pA_end, (GB_Ai (pA-1) < i)));
        ASSERT (GB_IMPLIES (pA >= pA_start && pA < pA_end, (GB_Ai (pA) >= i )));

        // Ai has been split.  If afound is false:
        //      Ai [pA_start : pA-1] < i
        //      Ai [pA : pA_end-1]   > i
        // If afound is true:
        //      Ai [pA_start : pA-1] < i
        //      Ai [pA : pA_end-1]  >= i
        //
        // in both cases, if i is chosen as the breakpoint, then the
        // subtask starts at index i, and position pA in Ai,Ax.

        // if A(:,kA) is empty, then pA is -1

        //----------------------------------------------------------------------
        // find where i appears in B(:,kB)
        //----------------------------------------------------------------------

        if (b_empty)
        { 
            // B(:,kB) is empty so i does not appear
            pB = -1 ;
        }
        else if (bknz == vlen)
        { 
            // B(:,kB) is dense; no need for a binary search
            pB = pB_start + i ;
            ASSERT (Bi [pB] == i) ;
        }
        else
        { 
            // B(:,kB) is sparse, and not empty
            // printf ("i is "GBd" bknz "GBd"\n", i, bknz) ;
            ASSERT (bknz > 0) ;
            pB = pB_start ;
            bool bfound ;
            int64_t bpright = pB_end - 1 ;
            GB_BINARY_SPLIT_SEARCH (i, Bi, pB, bpright, bfound) ;
            ASSERT (pB_start <= pB && pB <= pB_end) ;
            // printf ("pB "GBd" bfound %d\n", pB, bfound) ;
        }
        ASSERT (GB_IMPLIES (pB >  pB_start && pB < pB_end, (Bi [pB-1] < i))) ;
        ASSERT (GB_IMPLIES (pB >= pB_start && pB < pB_end, (Bi [pB] >= i ))) ;

        // Bi has been split.  If bfound is false:
        //      Bi [pB_start : pB-1] < i
        //      Bi [pB : pB_end-1]   > i
        // If bfound is true:
        //      Bi [pB_start : pB-1] < i
        //      Bi [pB : pB_end-1]  >= i
        //
        // in both cases, if i is chosen as the breakpoint, then the
        // subtask starts at index i, and position pB in Bi,Bx.

        // if B(:,kB) is empty, then pB is -1

        //----------------------------------------------------------------------
        // determine if the subtask is near the target task size
        //----------------------------------------------------------------------

        double work = (a_empty ? 0 : (pA_end - pA))
                    + (b_empty ? 0 : (pB_end - pB)) ;
        // printf ("    work %g target %g\n", work, target_work) ;

        if (work < 0.9999 * target_work)
        { 

            //------------------------------------------------------------------
            // work is too low
            //------------------------------------------------------------------

            // work is too low, so i is too high.
            // Keep searching in the range (ileft:i), inclusive.

            iright = i ;

        }
        else if (work > 1.0001 * target_work)
        { 

            //------------------------------------------------------------------
            // work is too high
            //------------------------------------------------------------------

            // work is too high, so i is too low.
            // Keep searching in the range (i+1):iright, inclusive.

            ileft = i + 1 ;

        }
        else
        { 

            //------------------------------------------------------------------
            // work is about right; use this result.
            //------------------------------------------------------------------

            // return i, pA, and pB as the start of this task.
            ASSERT (0 <= i && i <= vlen) ;
            ASSERT (pA == -1 || (pA_start <= pA && pA <= pA_end)) ;
            ASSERT (pB == -1 || (pB_start <= pB && pB <= pB_end)) ;
            break ;
        }
    }

    //--------------------------------------------------------------------------
    // find where i appears in M(:,kM)
    //--------------------------------------------------------------------------

    // printf ("sliced at i "GBd" pA "GBd" pB "GBd"\n", i, pA, pB) ;

    if (m_empty)
    { 
        pM = -1 ;
    }
    else if (mknz == vlen)
    { 
        // M(:,kM) is dense; no need for a binary search
        pM = pM_start + i ;
        ASSERT (Mi [pM] == i) ;
    }
    else
    { 
        // M(:,kM) is sparse, and not empty
        ASSERT (mknz > 0) ;
        pM = pM_start ;
        bool mfound ;
        int64_t mpright = pM_end - 1 ;
        GB_BINARY_SPLIT_SEARCH (i, Mi, pM, mpright, mfound) ;
    }
    // printf ("pM "GBd"\n", pM) ;

    //--------------------------------------------------------------------------
    // return result
    //--------------------------------------------------------------------------

    // pM, pA, and pB partition the three vectors M(:,j), A(:,j), and B(:,j),
    // or if any vector is empty, their p* pointer is -1.

    ASSERT (GB_IMPLIES ((pM >  pM_start && pM < pM_end), Mi [pM-1] <  i)) ;
    ASSERT (GB_IMPLIES ((pM >= pM_start && pM < pM_end), Mi [pM  ] >= i)) ;
    ASSERT (GB_IMPLIES ((pA >  pA_start && pA < pA_end), GB_Ai (pA-1) <  i)) ;
    ASSERT (GB_IMPLIES ((pA >= pA_start && pA < pA_end), GB_Ai (pA  ) >= i)) ;
    ASSERT (GB_IMPLIES ((pB >  pB_start && pB < pB_end), Bi [pB-1] <  i)) ;
    ASSERT (GB_IMPLIES ((pB >= pB_start && pB < pB_end), Bi [pB  ] >= i)) ;

    // printf ("sliced vector: i "GBd" pA "GBd" pB "GBd" pM "GBd"\n",
    //     i, pA, pM, pB) ;

    if (p_i != NULL)
    { 
        (*p_i)  = i ;
    }
    if (p_pM != NULL)
    { 
        (*p_pM) = pM ;
    }
    (*p_pA) = pA ;
    (*p_pB) = pB ;
}

