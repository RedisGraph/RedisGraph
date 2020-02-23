//------------------------------------------------------------------------------
// GB_subassign_03: C(I,J) += scalar ; using S
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Method 03: C(I,J) += scalar ; using S

// M:           NULL
// Mask_comp:   false
// C_replace:   false
// accum:       present
// A:           scalar
// S:           constructed

#include "GB_subassign_methods.h"

GrB_Info GB_subassign_03
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
    const void *scalar,
    const GrB_Type atype,
    const GrB_Matrix S,
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // get inputs
    //--------------------------------------------------------------------------

    GB_GET_C ;
    const bool C_is_hyper = C->is_hyper ;
    const int64_t *GB_RESTRICT Ch = C->h ;
    const int64_t *GB_RESTRICT Cp = C->p ;
    const int64_t Cnvec = C->nvec ;
    GB_GET_S ;
    const int64_t *GB_RESTRICT Sh = S->h ;
    const int64_t Snvec = S->nvec ;
    const bool S_is_hyper = S->is_hyper ;
    GB_GET_ACCUM_SCALAR ;

    //--------------------------------------------------------------------------
    // Method 03: C(I,J) += scalar ; using S
    //--------------------------------------------------------------------------

    // Time: Optimal; must visit all IxJ, so Omega(|I|*|J|) is required.

    // Entries in S are found and the corresponding entry in C replaced with
    // the scalar.

    // Method 01 and Method 03 are very similar.

    //--------------------------------------------------------------------------
    // Parallel: all IxJ (Methods 01, 03, 13, 15, 17, 19)
    //--------------------------------------------------------------------------

    GB_SUBASSIGN_IXJ_SLICE ;

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

        GB_GET_IXJ_TASK_DESCRIPTOR_PHASE1 ;

        //----------------------------------------------------------------------
        // compute all vectors in this task
        //----------------------------------------------------------------------

        for (int64_t j = kfirst ; j <= klast ; j++)
        {

            //------------------------------------------------------------------
            // get jC, the corresponding vector of C
            //------------------------------------------------------------------

            GB_GET_jC ;

            //------------------------------------------------------------------
            // get S(iA_start:end,j)
            //------------------------------------------------------------------

            GB_GET_VECTOR_FOR_IXJ (S) ;

            //------------------------------------------------------------------
            // C(I(iA_start,iA_end-1),jC) += scalar
            //------------------------------------------------------------------

            for (int64_t iA = iA_start ; iA < iA_end ; iA++)
            {
                bool found = (pS < pS_end) && (Si [pS] == iA) ;
                if (!found)
                { 
                    // ----[. A 1]----------------------------------------------
                    // S (i,j) is not present, the scalar is present
                    // [. A 1]: action: ( insert )
                    task_pending++ ;
                }
                else
                { 
                    // ----[C A 1] or [X A 1]-----------------------------------
                    // both S (i,j) and A (i,j) present
                    // [C A 1]: action: ( =C+A ): apply accum
                    // [X A 1]: action: ( undelete ): zombie lives
                    GB_C_S_LOOKUP ;
                    GB_withaccum_C_A_1_scalar ;
                    GB_NEXT (S) ;
                }
            }
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

        GB_GET_IXJ_TASK_DESCRIPTOR_PHASE2 ;

        //----------------------------------------------------------------------
        // compute all vectors in this task
        //----------------------------------------------------------------------

        for (int64_t j = kfirst ; j <= klast ; j++)
        {

            //------------------------------------------------------------------
            // get jC, the corresponding vector of C
            //------------------------------------------------------------------

            GB_GET_jC ;

            //------------------------------------------------------------------
            // get S(iA_start:end,j)
            //------------------------------------------------------------------

            GB_GET_VECTOR_FOR_IXJ (S) ;

            //------------------------------------------------------------------
            // C(I(iA_start,iA_end-1),jC) += scalar
            //------------------------------------------------------------------

            for (int64_t iA = iA_start ; iA < iA_end ; iA++)
            {
                bool found = (pS < pS_end) && (Si [pS] == iA) ;
                if (!found)
                { 
                    // ----[. A 1]----------------------------------------------
                    // S (i,j) is not present, the scalar is present
                    // [. A 1]: action: ( insert )
                    int64_t iC = GB_ijlist (I, iA, Ikind, Icolon) ;
                    GB_PENDING_INSERT (scalar) ;
                }
                else
                { 
                    // both S (i,j) and A (i,j) present
                    GB_NEXT (S) ;
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

