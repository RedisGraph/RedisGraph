//------------------------------------------------------------------------------
// GB_subassign_08: C(I,J)<M> += A ; no S
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Method 08: C(I,J)<M> += A ; no S

// M:           present
// Mask_comp:   false
// C_replace:   false
// accum:       present
// A:           matrix
// S:           none

#define GB_FREE_WORK GB_FREE_EMULT_SLICE

#include "GB_subassign_methods.h"

//------------------------------------------------------------------------------
// GB_PHASE1_ACTION
//------------------------------------------------------------------------------

// action to take for phase 1 when A(i,j) exists and M(i,j)=1
#define GB_PHASE1_ACTION                                                    \
{                                                                           \
    if (cjdense)                                                            \
    {                                                                       \
        /* direct lookup of C(iC,jC) */                                     \
        GB_iC_DENSE_LOOKUP ;                                                \
        /* ----[C A 1] or [X A 1]------------------------------- */         \
        /* [C A 1]: action: ( =C+A ): apply accum                */         \
        /* [X A 1]: action: ( undelete ): zombie lives           */         \
        GB_withaccum_C_A_1_matrix ;                                         \
    }                                                                       \
    else                                                                    \
    {                                                                       \
        /* binary search for C(iC,jC) in C(:,jC) */                         \
        GB_iC_BINARY_SEARCH ;                                               \
        if (cij_found)                                                      \
        {                                                                   \
            /* ----[C A 1] or [X A 1]--------------------------- */         \
            /* [C A 1]: action: ( =C+A ): apply accum            */         \
            /* [X A 1]: action: ( undelete ): zombie lives       */         \
            GB_withaccum_C_A_1_matrix ;                                     \
        }                                                                   \
        else                                                                \
        {                                                                   \
            /* ----[. A 1]-------------------------------------- */         \
            /* [. A 1]: action: ( insert )                       */         \
            task_pending++ ;                                                \
        }                                                                   \
    }                                                                       \
}

//------------------------------------------------------------------------------
// GB_PHASE2_ACTION
//------------------------------------------------------------------------------

// action to take for phase 2 when A(i,j) exists and M(i,j)=1
#define GB_PHASE2_ACTION                                                    \
{                                                                           \
    ASSERT (!cjdense) ;                                                     \
    {                                                                       \
        /* binary search for C(iC,jC) in C(:,jC) */                         \
        GB_iC_BINARY_SEARCH ;                                               \
        if (!cij_found)                                                     \
        {                                                                   \
            /* ----[. A 1]-------------------------------------- */         \
            /* [. A 1]: action: ( insert )                       */         \
            GB_PENDING_INSERT (Ax +(pA*asize)) ;                            \
        }                                                                   \
    }                                                                       \
}

//------------------------------------------------------------------------------
// GB_subassign_08: C(I,J)<M> += A ; no S
//------------------------------------------------------------------------------

GrB_Info GB_subassign_08
(
    GrB_Matrix C,
    // input:
    const GrB_Index *I,
    const int64_t nI,
    const int Ikind,
    const int64_t Icolon [3],
    const GrB_Index *J,
    const int64_t nJ,
    const int Jkind,
    const int64_t Jcolon [3],
    const GrB_Matrix M,
    const bool Mask_struct,
    const GrB_BinaryOp accum,
    const GrB_Matrix A,
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // get inputs
    //--------------------------------------------------------------------------

    GB_GET_C ;
    int64_t zorig = C->nzombies ;
    const bool C_is_hyper = C->is_hyper ;
    const int64_t Cnvec = C->nvec ;
    const int64_t cvlen = C->vlen ;
    const int64_t *GB_RESTRICT Ch = C->h ;
    const int64_t *GB_RESTRICT Cp = C->p ;
    GB_GET_MASK ;
    // const int64_t mvlen = M->vlen ;
    GB_GET_A ;
    const int64_t *GB_RESTRICT Ah = A->h ;
    GB_GET_ACCUM ;

    //--------------------------------------------------------------------------
    // Method 08: C(I,J)<M> += A ; no S
    //--------------------------------------------------------------------------

    // Time: Close to optimal. Omega (sum_j (min (nnz (A(:,j)), nnz (M(:,j)))),
    // since only the intersection of A.*M needs to be considered.  If either
    // M(:,j) or A(:,j) are very sparse compared to the other, then the shorter
    // is traversed with a linear-time scan and a binary search is used for the
    // other.  If the number of nonzeros is comparable, a linear-time scan is
    // used for both.  Once two entries M(i,j)=1 and A(i,j) are found with the
    // same index i, the entry A(i,j) is accumulated or inserted into C.

    // The algorithm is very much like the eWise multiplication of A.*M, so the
    // parallel scheduling relies on GB_emult_phase0(AA and GB_ewise_slice.

    //--------------------------------------------------------------------------
    // Parallel: slice the eWiseMult of A.*M (Method 08)
    //--------------------------------------------------------------------------

    GB_SUBASSIGN_EMULT_SLICE (A,M) ;

    //--------------------------------------------------------------------------
    // phase 1: create zombies, update entries, and count pending tuples
    //--------------------------------------------------------------------------

    int taskid ;
    #pragma omp parallel for num_threads(nthreads) schedule(dynamic,1) \
        reduction(+:nzombies)
    for (taskid = 0 ; taskid < ntasks ; taskid++)
    {

        //----------------------------------------------------------------------
        // get the task descriptor
        //----------------------------------------------------------------------

        GB_GET_TASK_DESCRIPTOR_PHASE1 ;

        //----------------------------------------------------------------------
        // compute all vectors in this task
        //----------------------------------------------------------------------

        for (int64_t k = kfirst ; k <= klast ; k++)
        {

            //------------------------------------------------------------------
            // get A(:,j) and M(:,j)
            //------------------------------------------------------------------

            int64_t j = (Zh == NULL) ? k : Zh [k] ;
            GB_GET_EMULT_VECTOR (pA, pA_end, pA, pA_end, Ap, Ah, j, k, Z_to_A) ;
            GB_GET_EMULT_VECTOR (pM, pM_end, pB, pB_end, Mp, Mh, j, k, Z_to_M) ;

            //------------------------------------------------------------------
            // quick checks for empty intersection of A(:,j) and M(:,j)
            //------------------------------------------------------------------

            int64_t ajnz = pA_end - pA ;
            int64_t mjnz = pM_end - pM ;
            if (ajnz == 0 || mjnz == 0) continue ;
            int64_t iA_first = Ai [pA] ;
            int64_t iA_last  = Ai [pA_end-1] ;
            int64_t iM_first = Mi [pM] ;
            int64_t iM_last  = Mi [pM_end-1] ;
            if (iA_last < iM_first || iM_last < iA_first) continue ;
            int64_t pM_start = pM ;

            //------------------------------------------------------------------
            // get jC, the corresponding vector of C
            //------------------------------------------------------------------

            GB_GET_jC ;
            bool cjdense = (pC_end - pC_start == cvlen) ;

            //------------------------------------------------------------------
            // C(I,jC)<M(:,j)> += A(:,j) ; no S
            //------------------------------------------------------------------

            if (ajnz > 32 * mjnz)
            {

                //--------------------------------------------------------------
                // A(:,j) is much denser than M(:,j)
                //--------------------------------------------------------------

                for ( ; pM < pM_end ; pM++)
                {
                    if (GB_mcast (Mx, pM, msize))
                    { 
                        int64_t iA = Mi [pM] ;
                        // find iA in A(:,j)
                        int64_t pright = pA_end - 1 ;
                        bool found ;
                        // FUTURE::: exploit dense A(:,j)
                        GB_BINARY_SEARCH (iA, Ai, pA, pright, found) ;
                        if (found) GB_PHASE1_ACTION ;
                    }
                }

            }
            else if (mjnz > 32 * ajnz)
            {

                //--------------------------------------------------------------
                // M(:,j) is much denser than A(:,j)
                //--------------------------------------------------------------

                // FUTURE::: exploit dense mask
                bool mjdense = false ;

                for ( ; pA < pA_end ; pA++)
                { 
                    int64_t iA = Ai [pA] ;
                    GB_MIJ_BINARY_SEARCH_OR_DENSE_LOOKUP (iA) ;
                    if (mij) GB_PHASE1_ACTION ;
                }

            }
            else
            {

                //----------------------------------------------------------
                // A(:,j) and M(:,j) have about the same # of entries
                //----------------------------------------------------------

                // linear-time scan of A(:,j) and M(:,j)

                while (pA < pA_end && pM < pM_end)
                {
                    int64_t iA = Ai [pA] ;
                    int64_t iM = Mi [pM] ;
                    if (iA < iM)
                    { 
                        // A(i,j) exists but not M(i,j)
                        GB_NEXT (A) ;
                    }
                    else if (iM < iA)
                    { 
                        // M(i,j) exists but not A(i,j)
                        GB_NEXT (M) ;
                    }
                    else
                    { 
                        // both A(i,j) and M(i,j) exist
                        if (GB_mcast (Mx, pM, msize)) GB_PHASE1_ACTION ;
                        GB_NEXT (A) ;
                        GB_NEXT (M) ;
                    }
                }
            }
        }

        GB_PHASE1_TASK_WRAPUP ;
    }

    //--------------------------------------------------------------------------
    // phase 2: insert pending tuples
    //--------------------------------------------------------------------------

    GB_PENDING_CUMSUM ;
    zorig = C->nzombies ;

    #pragma omp parallel for num_threads(nthreads) schedule(dynamic,1) \
        reduction(&&:pending_sorted)
    for (taskid = 0 ; taskid < ntasks ; taskid++)
    {

        //----------------------------------------------------------------------
        // get the task descriptor
        //----------------------------------------------------------------------

        GB_GET_TASK_DESCRIPTOR_PHASE2 ;

        //----------------------------------------------------------------------
        // compute all vectors in this task
        //----------------------------------------------------------------------

        for (int64_t k = kfirst ; k <= klast ; k++)
        {

            //------------------------------------------------------------------
            // get A(:,j) and M(:,j)
            //------------------------------------------------------------------

            int64_t j = (Zh == NULL) ? k : Zh [k] ;
            GB_GET_EMULT_VECTOR (pA, pA_end, pA, pA_end, Ap, Ah, j, k, Z_to_A) ;
            GB_GET_EMULT_VECTOR (pM, pM_end, pB, pB_end, Mp, Mh, j, k, Z_to_M) ;

            //------------------------------------------------------------------
            // quick checks for empty intersection of A(:,j) and M(:,j)
            //------------------------------------------------------------------

            int64_t ajnz = pA_end - pA ;
            int64_t mjnz = pM_end - pM ;
            if (ajnz == 0 || mjnz == 0) continue ;
            int64_t iA_first = Ai [pA] ;
            int64_t iA_last  = Ai [pA_end-1] ;
            int64_t iM_first = Mi [pM] ;
            int64_t iM_last  = Mi [pM_end-1] ;
            if (iA_last < iM_first || iM_last < iA_first) continue ;
            int64_t pM_start = pM ;

            //------------------------------------------------------------------
            // get jC, the corresponding vector of C
            //------------------------------------------------------------------

            GB_GET_jC ;
            bool cjdense = (pC_end - pC_start == cvlen) ;
            if (cjdense) continue ;

            //------------------------------------------------------------------
            // C(I,jC)<M(:,j)> += A(:,j) ; no S
            //------------------------------------------------------------------

            if (ajnz > 32 * mjnz)
            {

                //--------------------------------------------------------------
                // A(:,j) is much denser than M(:,j)
                //--------------------------------------------------------------

                for ( ; pM < pM_end ; pM++)
                {
                    if (GB_mcast (Mx, pM, msize))
                    { 
                        int64_t iA = Mi [pM] ;
                        // find iA in A(:,j)
                        int64_t pright = pA_end - 1 ;
                        bool found ;
                        // FUTURE::: exploit dense A(:,j)
                        GB_BINARY_SEARCH (iA, Ai, pA, pright, found) ;
                        if (found) GB_PHASE2_ACTION ;
                    }
                }

            }
            else if (mjnz > 32 * ajnz)
            {

                //--------------------------------------------------------------
                // M(:,j) is much denser than A(:,j)
                //--------------------------------------------------------------

                // FUTURE::: exploit dense mask
                bool mjdense = false ;

                for ( ; pA < pA_end ; pA++)
                { 
                    int64_t iA = Ai [pA] ;
                    GB_MIJ_BINARY_SEARCH_OR_DENSE_LOOKUP (iA) ;
                    if (mij) GB_PHASE2_ACTION ;
                }

            }
            else
            {

                //----------------------------------------------------------
                // A(:,j) and M(:,j) have about the same # of entries
                //----------------------------------------------------------

                // linear-time scan of A(:,j) and M(:,j)

                while (pA < pA_end && pM < pM_end)
                {
                    int64_t iA = Ai [pA] ;
                    int64_t iM = Mi [pM] ;
                    if (iA < iM)
                    { 
                        // A(i,j) exists but not M(i,j)
                        GB_NEXT (A) ;
                    }
                    else if (iM < iA)
                    { 
                        // M(i,j) exists but not A(i,j)
                        GB_NEXT (M) ;
                    }
                    else
                    { 
                        // both A(i,j) and M(i,j) exist
                        if (GB_mcast (Mx, pM, msize)) GB_PHASE2_ACTION ;
                        GB_NEXT (A) ;
                        GB_NEXT (M) ;
                    }
                }
            }
        }

        GB_PHASE2_TASK_WRAPUP ;
    }

    //--------------------------------------------------------------------------
    // finalize the matrix and return result
    //--------------------------------------------------------------------------

    GB_SUBASSIGN_WRAPUP ;
}

