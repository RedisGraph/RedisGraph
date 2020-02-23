//------------------------------------------------------------------------------
// GB_subassign_11: C(I,J)<M,repl> += scalar ; using S
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Method 11: C(I,J)<M,repl> += scalar ; using S

// M:           present
// Mask_comp:   false
// C_replace:   true
// accum:       present
// A:           scalar
// S:           constructed

#define GB_FREE_WORK GB_FREE_TWO_SLICE

#include "GB_subassign_methods.h"

GrB_Info GB_subassign_11
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
    // GB_GET_MASK ;
    const int64_t *GB_RESTRICT Mp = M->p ;
//  const int64_t *GB_RESTRICT Mh = M->h ;
    const int64_t *GB_RESTRICT Mi = M->i ;
    const GB_void *GB_RESTRICT Mx = (Mask_struct ? NULL : (M->x)) ;
    const size_t msize = M->type->size ;
    GB_GET_ACCUM_SCALAR ;
    GB_GET_S ;

    //--------------------------------------------------------------------------
    // Method 11: C(I,J)<M,repl> += scalar ; using S
    //--------------------------------------------------------------------------

    // Time: Optimal.  All entries in M+S must be examined.  All entries in S
    // are modified:  if M(i,j)=1 then S(i,j) is used to write to the
    // corresponding entry in C.  If M(i,j) is not present, or zero, then the
    // entry in C is cleared (because of C_replace).  If S(i,j) is not present,
    // and M(i,j)=1, then the scalar is inserted into C.  The only case that
    // can be skipped is if neither S nor M is present.  As a result, this
    // method need not traverse all of IxJ.  It can limit its traversal to the
    // pattern of M+S.

    // Method 09 and Method 11 are very similar.

    //--------------------------------------------------------------------------
    // Parallel: Z=M+S (Methods 02, 04, 09, 10, 11, 12, 14, 16, 18, 20)
    //--------------------------------------------------------------------------

    GB_SUBASSIGN_TWO_SLICE (M, S) ;

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
            // get S(:,j) and M(:,j)
            //------------------------------------------------------------------

            int64_t j = (Zh == NULL) ? k : Zh [k] ;
            GB_GET_MAPPED_VECTOR (pM, pM_end, pA, pA_end, Mp, j, k, Z_to_X) ;
            GB_GET_MAPPED_VECTOR (pS, pS_end, pB, pB_end, Sp, j, k, Z_to_S) ;

            //------------------------------------------------------------------
            // do a 2-way merge of S(:,j) and M(:,j)
            //------------------------------------------------------------------

            // jC = J [j] ; or J is a colon expression
            // int64_t jC = GB_ijlist (J, j, Jkind, Jcolon) ;

            // while both list S (:,j) and M (:,j) have entries
            while (pS < pS_end && pM < pM_end)
            {
                int64_t iS = Si [pS] ;
                int64_t iM = Mi [pM] ;

                if (iS < iM)
                { 
                    // S (i,j) is present but M (i,j) is not
                    // ----[C A 0] or [X A 0]-----------------------------------
                    // [X A 0]: action: ( X ): still a zombie
                    // [C A 0]: C_repl: action: ( delete ): becomes zombie
                    GB_C_S_LOOKUP ;
                    GB_DELETE_ENTRY ;
                    GB_NEXT (S) ;
                }
                else if (iM < iS)
                {
                    // S (i,j) is not present, M (i,j) is present
                    if (GB_mcast (Mx, pM, msize))
                    { 
                        // ----[. A 1]------------------------------------------
                        // [. A 1]: action: ( insert )
                        task_pending++ ;
                    }
                    GB_NEXT (M) ;
                }
                else
                {
                    // both S (i,j) and M (i,j) present
                    GB_C_S_LOOKUP ;
                    if (GB_mcast (Mx, pM, msize))
                    { 
                        // ----[C A 1] or [X A 1]-------------------------------
                        // [C A 1]: action: ( =C+A ): apply accum
                        // [X A 1]: action: ( undelete ): zombie lives
                        GB_withaccum_C_A_1_scalar ;
                    }
                    else
                    { 
                        // ----[C A 0] or [X A 0]-------------------------------
                        // [X A 0]: action: ( X ): still a zombie
                        // [C A 0]: C_repl: action: ( delete ): becomes zombie
                        GB_DELETE_ENTRY ;
                    }
                    GB_NEXT (S) ;
                    GB_NEXT (M) ;
                }
            }

            // while list S (:,j) has entries.  List M (:,j) exhausted
            while (pS < pS_end)
            { 
                // S (i,j) is present but M (i,j) is not
                // ----[C A 0] or [X A 0]-----------------------------------
                // [X A 0]: action: ( X ): still a zombie
                // [C A 0]: C_repl: action: ( delete ): becomes zombie
                GB_C_S_LOOKUP ;
                GB_DELETE_ENTRY ;
                GB_NEXT (S) ;
            }

            // while list M (:,j) has entries.  List S (:,j) exhausted
            while (pM < pM_end)
            {
                // S (i,j) is not present, M (i,j) is present
                if (GB_mcast (Mx, pM, msize))
                { 
                    // ----[. A 1]------------------------------------------
                    // [. A 1]: action: ( insert )
                    task_pending++ ;
                }
                GB_NEXT (M) ;
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

        GB_GET_TASK_DESCRIPTOR_PHASE2 ;

        //----------------------------------------------------------------------
        // compute all vectors in this task
        //----------------------------------------------------------------------

        for (int64_t k = kfirst ; k <= klast ; k++)
        {

            //------------------------------------------------------------------
            // get S(:,j) and M(:,j)
            //------------------------------------------------------------------

            int64_t j = (Zh == NULL) ? k : Zh [k] ;
            GB_GET_MAPPED_VECTOR (pM, pM_end, pA, pA_end, Mp, j, k, Z_to_X) ;
            GB_GET_MAPPED_VECTOR (pS, pS_end, pB, pB_end, Sp, j, k, Z_to_S) ;

            //------------------------------------------------------------------
            // do a 2-way merge of S(:,j) and M(:,j)
            //------------------------------------------------------------------

            // jC = J [j] ; or J is a colon expression
            int64_t jC = GB_ijlist (J, j, Jkind, Jcolon) ;

            // while both list S (:,j) and M (:,j) have entries
            while (pS < pS_end && pM < pM_end)
            {
                int64_t iS = Si [pS] ;
                int64_t iM = Mi [pM] ;

                if (iS < iM)
                { 
                    // S (i,j) is present but M (i,j) is not
                    GB_NEXT (S) ;
                }
                else if (iM < iS)
                {
                    // S (i,j) is not present, M (i,j) is present
                    if (GB_mcast (Mx, pM, msize))
                    { 
                        // ----[. A 1]------------------------------------------
                        // [. A 1]: action: ( insert )
                        int64_t iC = GB_ijlist (I, iM, Ikind, Icolon) ;
                        GB_PENDING_INSERT (scalar) ;
                    }
                    GB_NEXT (M) ;
                }
                else
                { 
                    // both S (i,j) and M (i,j) present
                    GB_NEXT (S) ;
                    GB_NEXT (M) ;
                }
            }

            // while list M (:,j) has entries.  List S (:,j) exhausted
            while (pM < pM_end)
            {
                // S (i,j) is not present, M (i,j) is present
                if (GB_mcast (Mx, pM, msize))
                { 
                    // ----[. A 1]------------------------------------------
                    // [. A 1]: action: ( insert )
                    int64_t iM = Mi [pM] ;
                    int64_t iC = GB_ijlist (I, iM, Ikind, Icolon) ;
                    GB_PENDING_INSERT (scalar) ;
                }
                GB_NEXT (M) ;
            }
        }

        GB_PHASE2_TASK_WRAPUP ;
    }

    //--------------------------------------------------------------------------
    // finalize the matrix and return result
    //--------------------------------------------------------------------------

    GB_SUBASSIGN_WRAPUP ;
}

