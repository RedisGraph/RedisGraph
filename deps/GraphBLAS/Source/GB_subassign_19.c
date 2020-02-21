//------------------------------------------------------------------------------
// GB_subassign_19: C(I,J)<!M,repl> += scalar ; using S
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Method 19: C(I,J)<!M,repl> += scalar ; using S

// M:           present
// Mask_comp:   true
// C_replace:   true
// accum:       present
// A:           scalar
// S:           constructed

#include "GB_subassign_methods.h"

GrB_Info GB_subassign_19
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
    const int64_t Cnvec = C->nvec ;
    const int64_t *GB_RESTRICT Ch = C->h ;
    const int64_t *GB_RESTRICT Cp = C->p ;
    GB_GET_MASK ;
    const bool M_is_hyper = M->is_hyper ;
    const int64_t Mnvec = M->nvec ;
    GB_GET_S ;
    const int64_t *GB_RESTRICT Sh = S->h ;
    const int64_t Snvec = S->nvec ;
    const bool S_is_hyper = S->is_hyper ;
    GB_GET_ACCUM_SCALAR ;

    //--------------------------------------------------------------------------
    // Method 19: C(I,J)<!M,repl> += scalar ; using S
    //--------------------------------------------------------------------------

    // Time: Close to optimal; must visit all IxJ, so Omega(|I|*|J|) is
    // required.  The sparsity of !M cannot be exploited.

    // Methods 13, 15, 17, and 19 are very similar.

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
            // get S(iA_start:end,j) and M(iA_start:end,j)
            //------------------------------------------------------------------

            GB_GET_VECTOR_FOR_IXJ (S) ;
            GB_GET_VECTOR_FOR_IXJ (M) ;

            //------------------------------------------------------------------
            // C(I(iA_start,iA_end-1),jC)<!M,repl> += scalar
            //------------------------------------------------------------------

            for (int64_t iA = iA_start ; iA < iA_end ; iA++)
            {

                //--------------------------------------------------------------
                // Get the indices at the top of each list.
                //--------------------------------------------------------------

                int64_t iS = (pS < pS_end) ? Si [pS] : INT64_MAX ;
                int64_t iM = (pM < pM_end) ? Mi [pM] : INT64_MAX ;

                //--------------------------------------------------------------
                // find the smallest index of [iS iA iM] (always iA)
                //--------------------------------------------------------------

                int64_t i = iA ;

                //--------------------------------------------------------------
                // get M(i,j)
                //--------------------------------------------------------------

                bool mij ;
                if (i == iM)
                { 
                    // mij = (bool) M [pM]
                    mij = GB_mcast (Mx, pM, msize) ;
                    GB_NEXT (M) ;
                }
                else
                { 
                    // mij not present, implicitly false
                    ASSERT (i < iM) ;
                    mij = false ;
                }

                // complement the mask entry mij since Mask_comp is true
                mij = !mij ;

                //--------------------------------------------------------------
                // accumulate the entry
                //--------------------------------------------------------------

                if (i == iS)
                {
                    ASSERT (i == iA) ;
                    {
                        // both S (i,j) and A (i,j) present
                        GB_C_S_LOOKUP ;
                        if (mij)
                        { 
                            // ----[C A 1] or [X A 1]---------------------------
                            // [C A 1]: action: ( =C+A ): apply accum
                            // [X A 1]: action: ( undelete ): zombie lives
                            GB_withaccum_C_A_1_scalar ;
                        }
                        else
                        { 
                            // ----[C A 0] or [X A 0]---------------------------
                            // [X A 0]: action: ( X ): still a zombie
                            // [C A 0]: C_repl: action: ( delete ): zombie
                            GB_DELETE_ENTRY ;
                        }
                        GB_NEXT (S) ;
                    }
                }
                else
                {
                    ASSERT (i == iA) ;
                    {
                        // S (i,j) is not present, A (i,j) is present
                        if (mij)
                        { 
                            // ----[. A 1]--------------------------------------
                            // [. A 1]: action: ( insert )
                            task_pending++ ;
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
            // get S(iA_start:end,j) and M(iA_start:end,j)
            //------------------------------------------------------------------

            GB_GET_VECTOR_FOR_IXJ (S) ;
            GB_GET_VECTOR_FOR_IXJ (M) ;

            //------------------------------------------------------------------
            // C(I(iA_start,iA_end-1),jC)<!M,repl> += scalar
            //------------------------------------------------------------------

            for (int64_t iA = iA_start ; iA < iA_end ; iA++)
            {

                //--------------------------------------------------------------
                // Get the indices at the top of each list.
                //--------------------------------------------------------------

                int64_t iS = (pS < pS_end) ? Si [pS] : INT64_MAX ;
                int64_t iM = (pM < pM_end) ? Mi [pM] : INT64_MAX ;

                //--------------------------------------------------------------
                // find the smallest index of [iS iA iM] (always iA)
                //--------------------------------------------------------------

                int64_t i = iA ;

                //--------------------------------------------------------------
                // get M(i,j)
                //--------------------------------------------------------------

                bool mij ;
                if (i == iM)
                { 
                    // mij = (bool) M [pM]
                    mij = GB_mcast (Mx, pM, msize) ;
                    GB_NEXT (M) ;
                }
                else
                { 
                    // mij not present, implicitly false
                    ASSERT (i < iM) ;
                    mij = false ;
                }

                // complement the mask entry mij since Mask_comp is true
                mij = !mij ;

                //--------------------------------------------------------------
                // accumulate the entry
                //--------------------------------------------------------------

                if (i == iS)
                {
                    ASSERT (i == iA) ;
                    { 
                        GB_NEXT (S) ;
                    }
                }
                else
                {
                    ASSERT (i == iA) ;
                    {
                        // S (i,j) is not present, A (i,j) is present
                        if (mij)
                        { 
                            // ----[. A 1]--------------------------------------
                            // [. A 1]: action: ( insert )
                            int64_t iC = GB_ijlist (I, iA, Ikind, Icolon) ;
                            GB_PENDING_INSERT (scalar) ;
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

