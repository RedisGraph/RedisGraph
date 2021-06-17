//------------------------------------------------------------------------------
// GxB_cuda_init: initialize GraphBLAS for use with CUDA
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// DRAFT: in progress

// GrB_init, GxB_init, or GxB_cuda_init must called before any other GraphBLAS
// operation.  GrB_finalize must be called as the last GraphBLAS operation.

// If CUDA was not available when GraphBLAS was compiled, then this function
// acks just like GrB_init.

#include "GB.h"

GrB_Info GxB_cuda_init      // start up GraphBLAS for use with CUDA
(
    GrB_Mode mode           // blocking or non-blocking mode
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GB_CONTEXT ("GxB_cuda_init (mode)") ;

    //--------------------------------------------------------------------------
    // initialize GraphBLAS
    //--------------------------------------------------------------------------

    return (GB_init
        (mode,                          // blocking or non-blocking mode
        NULL, NULL, NULL, NULL,         // use GxB_cuda_* memory managment
        true,                           // memory functions are thread-safe
        true,                           // use CUDA
        Context)) ;
}

