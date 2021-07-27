
//------------------------------------------------------------------------------
// GB_reduce_to_scalar_cuda.cu: reduce on the GPU with semiring 
//------------------------------------------------------------------------------

// SPDX-License-Identifier: Apache-2.0
// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB_cuda.h"

#include "templates/reduceWarp.cu.jit"
#include "templates/reduceNonZombiesWarp.cu.jit"
#include "test/semiringFactory.hpp"

#include "GB_jit_launcher.h"
#include "GB_callback.hpp"


const std::vector<std::string> header_names ={};

GrB_Info GB_reduce_to_scalar_cuda
(
    GB_void *s,
    const GrB_Monoid reduce,
    const GrB_Matrix A,
    GB_Context Context
)
{ 

    printf ("Hi I am %s :-)\n", __FILE__) ;

    // result = sum (Anz [0..anz-1]) using the GPU,
    // with a kernel that has ntasks = grid.x and blocksize = blockDim.x
    // nthreads = # of GPUs to use, but 1 for now
    // We have a workspace W of size ntasks.

    thread_local static jitify::JitCache kernel_cache;
    std::string reduce_kernel_name = "reduceNonZombiesWarp";

    // stringified kernel specified above
    jitify::Program program= kernel_cache.program( templates_reduceNonZombiesWarp_cu, 0, 0,
        file_callback_plus);
    //{"--use_fast_math", "-I/usr/local/cuda/include"});

    int nnz = GB_NNZ( A ) ;
    GrB_Type ctype = reduce->op->ztype ;

    int blocksize = 1024 ;
    int ntasks = ( nnz + blocksize -1) / blocksize ;

    int32_t *block_sum;
    //cudaMallocManaged ((void**) &block_sum, (num_reduce_blocks)*sizeof(int32_t)) ;
    block_sum = (int32_t*)GB_cuda_malloc( (ntasks)*sizeof(int32_t)) ;

    dim3 red_grid(ntasks);
    dim3 red_block(blocksize);

    GBURBLE ("(GPU reduce launch nblocks,blocksize= %d,%d )\n", ntasks, blocksize) ;
    jit::launcher( reduce_kernel_name + "_" + reduce->op->name,
                   templates_reduceNonZombiesWarp_cu,
                   header_names,
                   compiler_flags,
                   callback_wrapper)
                   .set_kernel_inst( reduce_kernel_name , { ctype->name })
                   .configure(red_grid, red_block) //if commented, use implicit 1D configure in launch
                   .launch(
                            A->i,   // index vector, only sum up values >= 0
                            A->x,   // input pointer to vector to reduce, with zombies
                            block_sum,             // Block sums on return 
                            (unsigned int)nnz      // length of vector to reduce to scalar

                        );

    cudaDeviceSynchronize();


    for (int i = 0 ; i < ntasks ; i++)
    {
        *s += (block_sum [i]) ; 
    }


    return (GrB_SUCCESS) ;
}

