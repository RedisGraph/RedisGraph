//------------------------------------------------------------------------------
// GB_AxB_saxpy3_cumsum: finalize nnz(C(:,j)) and find cumulative sum of Cp
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// phase3: fine tasks finalize their computation nnz(C(:,j))
// phase4: cumulative sum of C->p

#include "GB_AxB_saxpy3.h"

void GB_AxB_saxpy3_cumsum
(
    GrB_Matrix C,               // finalize C->p
    GB_saxpy3task_struct *SaxpyTasks, // list of tasks, and workspace
    int nfine,                  // number of fine tasks
    double chunk,               // chunk size
    int nthreads,               // number of threads
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // get C
    //--------------------------------------------------------------------------

    ASSERT (!GB_IS_BITMAP (C)) ;
    ASSERT (!GB_IS_FULL (C)) ;
    int64_t *restrict Cp = C->p ;
    const int64_t cvlen = C->vlen ;
    const int64_t cnvec = C->nvec ;

    //==========================================================================
    // phase3: count nnz(C(:,j)) for fine tasks
    //==========================================================================

    int taskid ;
    #pragma omp parallel for num_threads(nthreads) schedule(dynamic,1)
    for (taskid = 0 ; taskid < nfine ; taskid++)
    {

        //----------------------------------------------------------------------
        // get the task descriptor
        //----------------------------------------------------------------------

        // int64_t kk = SaxpyTasks [taskid].vector ;
        int64_t hash_size = SaxpyTasks [taskid].hsize ;
        bool use_Gustavson = (hash_size == cvlen) ;
        int team_size = SaxpyTasks [taskid].team_size ;
        int leader    = SaxpyTasks [taskid].leader ;
        int my_teamid = taskid - leader ;
        int64_t my_cjnz = 0 ;

        if (use_Gustavson)
        {

            //------------------------------------------------------------------
            // phase3: fine Gustavson task, C=A*B, C<M>=A*B, or C<!M>=A*B
            //------------------------------------------------------------------

            // Hf [i] == 2 if C(i,j) is an entry in C(:,j)

            int8_t *restrict Hf ;
            Hf = (int8_t *restrict) SaxpyTasks [taskid].Hf ;
            int64_t istart, iend ;
            GB_PARTITION (istart, iend, cvlen, my_teamid, team_size) ;
            for (int64_t i = istart ; i < iend ; i++)
            {
                if (Hf [i] == 2)
                { 
                    my_cjnz++ ;
                }
            }

        }
        else
        {

            //------------------------------------------------------------------
            // phase3: fine hash task, C=A*B, C<M>=A*B, or C<!M>=A*B
            //------------------------------------------------------------------

            // (Hf [hash] & 3) == 2 if C(i,j) is an entry in C(:,j),
            // and the index i of the entry is (Hf [hash] >> 2) - 1.

            int64_t *restrict Hf = (int64_t *restrict) SaxpyTasks [taskid].Hf ;
            int64_t mystart, myend ;
            GB_PARTITION (mystart, myend, hash_size, my_teamid, team_size) ;
            for (int64_t hash = mystart ; hash < myend ; hash++)
            {
                if ((Hf [hash] & 3) == 2)
                { 
                    my_cjnz++ ;
                }
            }
        }

        SaxpyTasks [taskid].my_cjnz = my_cjnz ;   // count my nnz(C(:,j))
    }

    //==========================================================================
    // phase4: compute Cp with cumulative sum
    //==========================================================================

    //--------------------------------------------------------------------------
    // sum nnz (C (:,j)) for fine tasks
    //--------------------------------------------------------------------------

    // SaxpyTasks [taskid].my_cjnz is the # of unique entries found in C(:,j) by
    // that task.  Sum these terms to compute total # of entries in C(:,j).

    for (taskid = 0 ; taskid < nfine ; taskid++)
    { 
        int64_t kk = SaxpyTasks [taskid].vector ;
        Cp [kk] = 0 ;
    }

    for (taskid = 0 ; taskid < nfine ; taskid++)
    { 
        int64_t kk = SaxpyTasks [taskid].vector ;
        int64_t my_cjnz = SaxpyTasks [taskid].my_cjnz ;
        Cp [kk] += my_cjnz ;
        ASSERT (my_cjnz <= cvlen) ;
    }

    //--------------------------------------------------------------------------
    // cumulative sum for Cp (fine and coarse tasks)
    //--------------------------------------------------------------------------

    // Cp [kk] is now nnz (C (:,j)), for all vectors j, whether computed by
    // fine tasks or coarse tasks, and where j == GBH (Bh, kk) 

    int nth = GB_nthreads (cnvec, chunk, nthreads) ;
    GB_cumsum (Cp, cnvec, &(C->nvec_nonempty), nth, Context) ;

    //--------------------------------------------------------------------------
    // cumulative sum of nnz (C (:,j)) for each team of fine tasks
    //--------------------------------------------------------------------------

    int64_t cjnz_sum = 0 ;
    for (taskid = 0 ; taskid < nfine ; taskid++)
    {
        if (taskid == SaxpyTasks [taskid].leader)
        {
            cjnz_sum = 0 ;
            // also find the max (C (:,j)) for any fine hash tasks
            int64_t hash_size = SaxpyTasks [taskid].hsize ;
            bool use_Gustavson = (hash_size == cvlen) ;
            if (!use_Gustavson)
            { 
                int64_t kk = SaxpyTasks [taskid].vector ;
                int64_t cjnz = Cp [kk+1] - Cp [kk] ;
            }
        }
        int64_t my_cjnz = SaxpyTasks [taskid].my_cjnz ;
        SaxpyTasks [taskid].my_cjnz = cjnz_sum ;
        cjnz_sum += my_cjnz ;
    }
}

