
//------------------------------------------------------------------------------
// GB_reduce_to_scalar_cuda.cu: reduce on the GPU with semiring 
//------------------------------------------------------------------------------

// SPDX-License-Identifier: Apache-2.0
// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

extern "C"
{
#include "GB_reduce.h"
}

#include "GB_cuda.h"
#include "GB_jit_cache.h"
#include "GB_cuda_common_jitFactory.hpp"
#include "GB_cuda_reduce_jitFactory.hpp"
#include "GB_cuda_type_wrap.hpp"

GrB_Info GB_reduce_to_scalar_cuda
(
    GB_void *s,
    const GrB_Monoid reduce,
    const GrB_Matrix A,
    GB_Context Context
)
{

    cudaStream_t stream;
    CHECK_CUDA(cudaStreamCreate(&stream));

    //----------------------------------------------------------------------
    // reduce C to a scalar, just for testing:
    //----------------------------------------------------------------------

    GBURBLE ("(get nnz) ") ;
    int64_t nz = GB_nnz(A);
    GBURBLE ("(got nnz) ") ;

    GB_cuda_reduce_factory myreducefactory;
    myreducefactory.reduce_factory(reduce, A);

    GB_cuda_reduce( myreducefactory, A, s, reduce, stream);
    GBURBLE ("(did reduce) ") ;

    CHECK_CUDA(cudaStreamSynchronize(stream));
    CHECK_CUDA(cudaStreamDestroy(stream));
    return GrB_SUCCESS ;
}

