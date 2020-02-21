//------------------------------------------------------------------------------
// GB_subassign_06n: C(I,J)<M> = A ; no S
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Method 06n: C(I,J)<M> = A ; no S

// M:           present
// Mask_comp:   false
// C_replace:   false
// accum:       NULL
// A:           matrix
// S:           none (see also GB_subassign_06s)

#include "GB_subassign_methods.h"

GrB_Info GB_subassign_06n
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
    GB_GET_A ;
    const int64_t *GB_RESTRICT Ah = A->h ;
    const int64_t Anvec = A->nvec ;
    const bool A_is_hyper = A->is_hyper ;
    const int64_t avlen = A->vlen ;
    GrB_BinaryOp accum = NULL ;

    //--------------------------------------------------------------------------
    // Method 06n: C(I,J)<M> = A ; no S
    //--------------------------------------------------------------------------

    // Time: O(nnz(M)*(log(a)+log(c)), where a and c are the # of entries in a
    // vector of A and C, respectively.  The entries in the intersection of M
    // (where the entries are true) and the matrix addition C(I,J)+A must be
    // examined.  This method scans M, and searches for entries in A and C(I,J)
    // using two binary searches.  If M is very dense, this method can be
    // slower than Method 06s.  This method is selected if nnz (A) >= nnz (M).

    // Compare with Methods 05 and 07, which use a similar algorithmic outline
    // and parallelization strategy.

    //--------------------------------------------------------------------------
    // Parallel: slice M into coarse/fine tasks (Method 05, 06n, 07)
    //--------------------------------------------------------------------------

    GB_SUBASSIGN_ONE_SLICE (M) ;

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
            // get j, the kth vector of M
            //------------------------------------------------------------------

            int64_t j = (Mh == NULL) ? k : Mh [k] ;
            GB_GET_VECTOR (pM, pM_end, pA, pA_end, Mp, k) ;
            int64_t mjnz = pM_end - pM ;
            if (mjnz == 0) continue ;

            //------------------------------------------------------------------
            // get A(:,j)
            //------------------------------------------------------------------

            int64_t pA, pA_end ;
            GB_VECTOR_LOOKUP (pA, pA_end, A, j) ;
            int64_t ajnz = pA_end - pA ;
            bool ajdense = (ajnz == avlen) ;
            int64_t pA_start = pA ;

            //------------------------------------------------------------------
            // get jC, the corresponding vector of C
            //------------------------------------------------------------------

            GB_GET_jC ;
            int64_t cjnz = pC_end - pC_start ;
            if (cjnz == 0 && ajnz == 0) continue ;
            bool cjdense = (cjnz == cvlen) ;

            //------------------------------------------------------------------
            // C(I,jC)<M(:,j)> = A(:,j) ; no S
            //------------------------------------------------------------------

            if (cjdense && ajdense)
            {

                //--------------------------------------------------------------
                // C(:,jC) and A(:,j) are both dense
                //--------------------------------------------------------------

                for ( ; pM < pM_end ; pM++)
                {

                    //----------------------------------------------------------
                    // update C(iC,jC), but only if M(iA,j) allows it
                    //----------------------------------------------------------

                    if (GB_mcast (Mx, pM, msize))
                    { 
                        int64_t iA = Mi [pM] ;
                        GB_iC_DENSE_LOOKUP ;

                        // find iA in A(:,j)
                        // A(:,j) is dense; no need for binary search
                        pA = pA_start + iA ;
                        ASSERT (Ai [pA] == iA) ;
                        // ----[C A 1] or [X A 1]-----------------------
                        // [C A 1]: action: ( =A ): copy A to C, no acc
                        // [X A 1]: action: ( undelete ): zombie lives
                        GB_noaccum_C_A_1_matrix ;
                    }
                }

            }
            else if (cjdense)
            {

                //--------------------------------------------------------------
                // C(:,jC) is dense, A(:,j) is sparse
                //--------------------------------------------------------------

                for ( ; pM < pM_end ; pM++)
                {

                    //----------------------------------------------------------
                    // update C(iC,jC), but only if M(iA,j) allows it
                    //----------------------------------------------------------

                    if (GB_mcast (Mx, pM, msize))
                    { 
                        int64_t iA = Mi [pM] ;
                        GB_iC_DENSE_LOOKUP ;

                        // find iA in A(:,j)
                        bool aij_found ;
                        int64_t apright = pA_end - 1 ;
                        GB_BINARY_SEARCH (iA, Ai, pA, apright, aij_found) ;

                        if (!aij_found)
                        { 
                            // C (iC,jC) is present but A (i,j) is not
                            // ----[C . 1] or [X . 1]---------------------------
                            // [C . 1]: action: ( delete ): becomes zombie
                            // [X . 1]: action: ( X ): still zombie
                            GB_DELETE_ENTRY ;
                        }
                        else
                        { 
                            // ----[C A 1] or [X A 1]---------------------------
                            // [C A 1]: action: ( =A ): copy A to C, no accum
                            // [X A 1]: action: ( undelete ): zombie lives
                            GB_noaccum_C_A_1_matrix ;
                        }
                    }
                }

            }
            else if (ajdense)
            {

                //--------------------------------------------------------------
                // C(:,jC) is sparse, A(:,j) is dense
                //--------------------------------------------------------------

                for ( ; pM < pM_end ; pM++)
                {

                    //----------------------------------------------------------
                    // update C(iC,jC), but only if M(iA,j) allows it
                    //----------------------------------------------------------

                    if (GB_mcast (Mx, pM, msize))
                    {
                        int64_t iA = Mi [pM] ;

                        // find C(iC,jC) in C(:,jC)
                        GB_iC_BINARY_SEARCH ;

                        // lookup iA in A(:,j)
                        pA = pA_start + iA ;
                        ASSERT (Ai [pA] == iA) ;

                        if (cij_found)
                        { 
                            // ----[C A 1] or [X A 1]---------------------------
                            // [C A 1]: action: ( =A ): copy A into C, no accum
                            // [X A 1]: action: ( undelete ): zombie lives
                            GB_noaccum_C_A_1_matrix ;
                        }
                        else
                        { 
                            // C (iC,jC) is not present, A (i,j) is present
                            // ----[. A 1]--------------------------------------
                            // [. A 1]: action: ( insert )
                            task_pending++ ;
                        }
                    }
                }

            }
            else
            {

                //--------------------------------------------------------------
                // C(:,jC) and A(:,j) are both sparse
                //--------------------------------------------------------------

                for ( ; pM < pM_end ; pM++)
                {

                    //----------------------------------------------------------
                    // update C(iC,jC), but only if M(iA,j) allows it
                    //----------------------------------------------------------

                    if (GB_mcast (Mx, pM, msize))
                    {
                        int64_t iA = Mi [pM] ;

                        // find C(iC,jC) in C(:,jC)
                        GB_iC_BINARY_SEARCH ;

                        // find iA in A(:,j)
                        bool aij_found ;
                        int64_t apright = pA_end - 1 ;
                        GB_BINARY_SEARCH (iA, Ai, pA, apright, aij_found) ;

                        if (cij_found && aij_found)
                        { 
                            // ----[C A 1] or [X A 1]---------------------------
                            // [C A 1]: action: ( =A ): copy A into C, no accum
                            // [X A 1]: action: ( undelete ): zombie lives
                            GB_noaccum_C_A_1_matrix ;
                        }
                        else if (!cij_found && aij_found)
                        { 
                            // C (iC,jC) is not present, A (i,j) is present
                            // ----[. A 1]--------------------------------------
                            // [. A 1]: action: ( insert )
                            task_pending++ ;
                        }
                        else if (cij_found && !aij_found)
                        { 
                            // C (iC,jC) is present but A (i,j) is not
                            // ----[C . 1] or [X . 1]---------------------------
                            // [C . 1]: action: ( delete ): becomes zombie
                            // [X . 1]: action: ( X ): still zombie
                            GB_DELETE_ENTRY ;
                        }
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
            // get j, the kth vector of M
            //------------------------------------------------------------------

            int64_t j = (Mh == NULL) ? k : Mh [k] ;
            GB_GET_VECTOR (pM, pM_end, pA, pA_end, Mp, k) ;
            int64_t mjnz = pM_end - pM ;
            if (mjnz == 0) continue ;

            //------------------------------------------------------------------
            // get A(:,j)
            //------------------------------------------------------------------

            int64_t pA, pA_end ;
            GB_VECTOR_LOOKUP (pA, pA_end, A, j) ;
            int64_t ajnz = pA_end - pA ;
            if (ajnz == 0) continue ;
            bool ajdense = (ajnz == avlen) ;
            int64_t pA_start = pA ;

            //------------------------------------------------------------------
            // get jC, the corresponding vector of C
            //------------------------------------------------------------------

            GB_GET_jC ;
            bool cjdense = ((pC_end - pC_start) == cvlen) ;

            //------------------------------------------------------------------
            // C(I,jC)<M(:,j)> = A(:,j)
            //------------------------------------------------------------------

            if (!cjdense)
            {

                //--------------------------------------------------------------
                // C(:,jC) is sparse; use binary search for C
                //--------------------------------------------------------------

                for ( ; pM < pM_end ; pM++)
                {

                    //----------------------------------------------------------
                    // update C(iC,jC), but only if M(iA,j) allows it
                    //----------------------------------------------------------

                    if (GB_mcast (Mx, pM, msize))
                    {
                        int64_t iA = Mi [pM] ;

                        // find iA in A(:,j)
                        if (ajdense)
                        {
                            // A(:,j) is dense; no need for binary search
                            pA = pA_start + iA ;
                            ASSERT (Ai [pA] == iA) ;
                        }
                        else
                        {
                            // A(:,j) is sparse; use binary search
                            int64_t apright = pA_end - 1 ;
                            bool aij_found ;
                            GB_BINARY_SEARCH (iA, Ai, pA, apright, aij_found) ;
                            if (!aij_found) continue ;
                        }

                        // find C(iC,jC) in C(:,jC)
                        GB_iC_BINARY_SEARCH ;
                        if (!cij_found)
                        { 
                            // C (iC,jC) is not present, A (i,j) is present
                            // ----[. A 1]--------------------------------------
                            // [. A 1]: action: ( insert )
                            GB_PENDING_INSERT (Ax +(pA*asize)) ;
                        }
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

