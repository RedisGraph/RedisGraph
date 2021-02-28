//------------------------------------------------------------------------------
// GB_subassign_04: C(I,J) += A ; using S
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Method 04: C(I,J) += A ; using S

// M:           NULL
// Mask_comp:   false
// C_replace:   false
// accum:       present
// A:           matrix
// S:           constructed

#define GB_FREE_WORK GB_FREE_TWO_SLICE

#include "GB_subassign_methods.h"

GrB_Info GB_subassign_04
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
    const GrB_BinaryOp accum,
    const GrB_Matrix A,
    const GrB_Matrix S,
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // get inputs
    //--------------------------------------------------------------------------

    GB_GET_C ;
    GB_GET_A ;
    GB_GET_S ;
    GB_GET_ACCUM ;

    //--------------------------------------------------------------------------
    // Method 04: C(I,J) += A ; using S
    //--------------------------------------------------------------------------

    // Time: Close to Optimal.  Every entry in A must be visited, and the
    // corresponding entry in S must then be found.  Time for this phase is
    // Omega(nnz(A)), but S has already been constructed, in Omega(nnz(S))
    // time.  This method simply traverses all of A+S (like GB_add for
    // computing A+S), the same as Method 02.  Time taken is O(nnz(A)+nnz(S)).
    // The only difference is that the traversal of A+S can terminate if A is
    // exhausted.  Entries in S but not A do not actually require any work
    // (unlike Method 02, which must visit all entries in A+S).

    // Method 02 and Method 04 are somewhat similar.  They differ on how C is
    // modified when the entry is present in S but not A.

    // Compare with Method 16, which computes C(I,J)<!M> += A, using S.

    //--------------------------------------------------------------------------
    // Parallel: Z=A+S (Methods 02, 04, 09, 10, 11, 12, 14, 16, 18, 20)
    //--------------------------------------------------------------------------

    GB_SUBASSIGN_TWO_SLICE (A, S) ;

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
            // get A(:,j) and S(:,j)
            //------------------------------------------------------------------

            int64_t j = (Zh == NULL) ? k : Zh [k] ;
            GB_GET_MAPPED_VECTOR (pA, pA_end, pA, pA_end, Ap, j, k, Z_to_X) ;
            GB_GET_MAPPED_VECTOR (pS, pS_end, pB, pB_end, Sp, j, k, Z_to_S) ;

            //------------------------------------------------------------------
            // do a 2-way merge of S(:,j) and A(:,j)
            //------------------------------------------------------------------

            // jC = J [j] ; or J is a colon expression
            // int64_t jC = GB_ijlist (J, j, Jkind, Jcolon) ;

            // while both list S (:,j) and A (:,j) have entries
            while (pS < pS_end && pA < pA_end)
            {
                int64_t iS = Si [pS] ;
                int64_t iA = Ai [pA] ;

                if (iS < iA)
                { 
                    // ----[C . 1] or [X . 1]-----------------------------------
                    // S (i,j) is present but A (i,j) is not
                    // [C . 1]: action: ( C ): no change, with accum
                    // [X . 1]: action: ( X ): still a zombie
                    GB_NEXT (S) ;

                }
                else if (iA < iS)
                { 
                    // ----[. A 1]----------------------------------------------
                    // S (i,j) is not present, A (i,j) is present
                    // [. A 1]: action: ( insert )
                    task_pending++ ;
                    GB_NEXT (A) ;
                }
                else
                { 
                    // ----[C A 1] or [X A 1]-----------------------------------
                    // both S (i,j) and A (i,j) present
                    // [C A 1]: action: ( =C+A ): apply accum
                    // [X A 1]: action: ( undelete ): zombie lives
                    GB_C_S_LOOKUP ;
                    GB_withaccum_C_A_1_matrix ;
                    GB_NEXT (S) ;
                    GB_NEXT (A) ;
                }
            }

            // ignore the remainder of S (:,j)

            // List A (:,j) has entries.  List S (:,j) exhausted.
            task_pending += (pA_end - pA) ;
        }

        GB_PHASE1_TASK_WRAPUP ;
    }

    //--------------------------------------------------------------------------
    // phase 2: insert pending tuples
    //--------------------------------------------------------------------------

    GB_PENDING_CUMSUM ;

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
            // get A(:,j) and S(:,j)
            //------------------------------------------------------------------

            int64_t j = (Zh == NULL) ? k : Zh [k] ;
            GB_GET_MAPPED_VECTOR (pA, pA_end, pA, pA_end, Ap, j, k, Z_to_X) ;
            GB_GET_MAPPED_VECTOR (pS, pS_end, pB, pB_end, Sp, j, k, Z_to_S) ;

            //------------------------------------------------------------------
            // do a 2-way merge of S(:,j) and A(:,j)
            //------------------------------------------------------------------

            // jC = J [j] ; or J is a colon expression
            int64_t jC = GB_ijlist (J, j, Jkind, Jcolon) ;

            // while both list S (:,j) and A (:,j) have entries
            while (pS < pS_end && pA < pA_end)
            {
                int64_t iS = Si [pS] ;
                int64_t iA = Ai [pA] ;

                if (iS < iA)
                { 
                    GB_NEXT (S) ;

                }
                else if (iA < iS)
                { 
                    // ----[. A 1]----------------------------------------------
                    // S (i,j) is not present, A (i,j) is present
                    // [. A 1]: action: ( insert )
                    int64_t iC = GB_ijlist (I, iA, Ikind, Icolon) ;
                    GB_PENDING_INSERT (Ax +(pA*asize)) ;
                    GB_NEXT (A) ;
                }
                else
                { 
                    GB_NEXT (S) ;
                    GB_NEXT (A) ;
                }
            }

            // ignore the remainder of S (:,j)

            // while list A (:,j) has entries.  List S (:,j) exhausted.
            while (pA < pA_end)
            { 
                // ----[. A 1]--------------------------------------------------
                // S (i,j) is not present, A (i,j) is present
                // [. A 1]: action: ( insert )
                int64_t iA = Ai [pA] ;
                int64_t iC = GB_ijlist (I, iA, Ikind, Icolon) ;
                GB_PENDING_INSERT (Ax +(pA*asize)) ;
                GB_NEXT (A) ;
            }
        }
        GB_PHASE2_TASK_WRAPUP ;
    }

    //--------------------------------------------------------------------------
    // finalize the matrix and return result
    //--------------------------------------------------------------------------

    GB_SUBASSIGN_WRAPUP ;
}

