//------------------------------------------------------------------------------
// GB_AxB_saxpy3_slice_quick: construct a single task for GB_AxB_saxpy3
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// create a single task for C=A*B, for a single thread.

#include "GB_AxB_saxpy3.h"

GrB_Info GB_AxB_saxpy3_slice_quick
(
    // inputs
    GrB_Matrix C,                   // output matrix
    const GrB_Matrix A,             // input matrix A
    const GrB_Matrix B,             // input matrix B
    // outputs
    GB_saxpy3task_struct **SaxpyTasks_handle,
    size_t *SaxpyTasks_size_handle,
    int *ntasks,                    // # of tasks created (coarse and fine)
    int *nfine,                     // # of fine tasks created
    int *nthreads,                  // # of threads to use
    GB_Context Context
)
{ 

    //--------------------------------------------------------------------------
    // get inputs
    //--------------------------------------------------------------------------

    (*ntasks) = 1 ;
    (*nfine) = 0 ;
    (*nthreads) = 1 ;

    const int64_t bnvec = B->nvec ;
    const int64_t cvlen = A->vlen ;

    //--------------------------------------------------------------------------
    // allocate the task
    //--------------------------------------------------------------------------

    size_t SaxpyTasks_size = 0 ;
    GB_saxpy3task_struct *SaxpyTasks = GB_MALLOC_WERK (1, GB_saxpy3task_struct,
        &SaxpyTasks_size) ;
    if (SaxpyTasks == NULL)
    { 
        // out of memory
        return (GrB_OUT_OF_MEMORY) ;
    }

    // clear SaxpyTasks
    memset (SaxpyTasks, 0, SaxpyTasks_size) ;

    //--------------------------------------------------------------------------
    // create a single Gustavson task
    //--------------------------------------------------------------------------

    SaxpyTasks [0].start   = 0 ;
    SaxpyTasks [0].end     = bnvec-1 ;
    SaxpyTasks [0].vector  = -1 ;
    SaxpyTasks [0].hsize   = cvlen ;
    SaxpyTasks [0].Hi      = NULL ;      // assigned later
    SaxpyTasks [0].Hf      = NULL ;      // assigned later
    SaxpyTasks [0].Hx      = NULL ;      // assigned later
    SaxpyTasks [0].my_cjnz = 0 ;         // unused
    SaxpyTasks [0].leader  = 0 ;
    SaxpyTasks [0].team_size = 1 ;

    if (bnvec == 1)
    { 
        // convert the single coarse task into a single fine task
        SaxpyTasks [0].start  = 0 ;                   // first entry in B(:,0)
        SaxpyTasks [0].end = GB_nnz_held (B) - 1 ;    // last entry in B(:,0)
        SaxpyTasks [0].vector = 0 ;
        (*nfine) = 1 ;
    }

    //--------------------------------------------------------------------------
    // return result
    //--------------------------------------------------------------------------

    (*SaxpyTasks_handle) = SaxpyTasks ;
    (*SaxpyTasks_size_handle) = SaxpyTasks_size ;
    return (GrB_SUCCESS) ;
}

