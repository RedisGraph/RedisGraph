//------------------------------------------------------------------------------
// GB_Pending_n.h: return the # of pending tuples in a matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#ifndef GB_PENDING_N_H
#define GB_PENDING_N_H

#ifdef GB_CUDA_KERNEL

    // create the static inline device function for the GPU
    #include "GB_Pending_n_template.c"

#else

    // declare the regular function for the CPU
    int64_t GB_Pending_n        // return # of pending tuples in A
    (
        GrB_Matrix A
    ) ;

#endif
#endif
