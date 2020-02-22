//------------------------------------------------------------------------------
// GB_cblas_saxpy: Y += alpha*X where X and Y are dense float arrays
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Y += alpha*X where are X and Y are dense arrays of stride 1, of type float.

// X and Y can have any size, and will often be larger than 2^31.

#include "GB_dense.h"
#include "GB_cblas.h"

void GB_cblas_saxpy         // Y += alpha*X
(
    const int64_t n,        // length of X and Y (note the int64_t type)
    const float alpha,      // scale factor
    const float *X,         // the array X, always stride 1
    float *Y,               // the array Y, always stride 1
    int nthreads            // maximum # of threads to use
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    // The GB_cblas_* gateway functions always exist in the GraphBLAS library,
    // but if GB_HAS_CBLAS is false at compile time, they become stubs that do
    // nothing at all, and they are never called.

    ASSERT (Y != NULL) ;
    ASSERT (X != NULL) ;
    ASSERT (nthreads >= 1) ;

    #if GB_HAS_CBLAS

    //--------------------------------------------------------------------------
    // determine the number of threads to use
    //--------------------------------------------------------------------------

    // Use no more than nthreads to do the saxpy.  Fewer threads may be used at
    // this function's discretion, but no more than nthreads can be used.  This
    // is a strict requirement.

    // Note that *other* GraphBLAS threads may be calling this function at the
    // same time, so the CBLAS thread setting must be done in a thread-safe
    // manner.  Multiple user threads could be calling GrB_* operations in
    // parallel, and each GraphBLAS call may have its own unique thread
    // maximum.  So the solution cannot assume that this function is only being
    // called in parallel from a single GrB_* operation.

    // Set the # of threads to use, in a thread-safe manner.  Do so in a
    // portable manner, for any BLAS library.  #ifdef's may be used to handle
    // this, depending on which BLAS library is being used, as determined by
    // the CMake build system.

    // Even if this function is called inside a parallel region, nthreads
    // could be larger than one.  In that case, nested parallelism has been
    // requested.

    #ifdef MKL_ILP64
    int save_nthreads = mkl_set_num_threads_local (nthreads) ;
    #endif

    //--------------------------------------------------------------------------
    // Y += alpha*X
    //--------------------------------------------------------------------------

    GB_CBLAS_INT stride1 = (GB_CBLAS_INT) 1 ;

    if (sizeof (GB_CBLAS_INT) == sizeof (int64_t))
    {
        // call *axpy in a single chunk
        cblas_saxpy     // y += alpha*x
        (
            n,          // length of x and y
            alpha,      // scale factor (typically 1.0)
            X,
            stride1,    // x is stride 1
            Y,
            stride1     // y is stride 1
        ) ;
    }
    else
    {
        // call *axpy in chunks of size GB_CBLAS_INT_MAX
        for (int64_t p = 0 ; p < n ; p += GB_CBLAS_INT_MAX)
        {
            GB_CBLAS_INT chunk =
                (GB_CBLAS_INT) GB_IMIN (n - p, GB_CBLAS_INT_MAX) ;
            cblas_saxpy     // y += alpha*x
            (
                chunk,      // length of x and y (this chunk)
                alpha,      // scale factor (typically 1.0)
                X + p,      // this chunk of x
                stride1,    // x is stride 1
                Y + p,      // this chunk of y
                stride1     // y is stride 1
            ) ;
        }
    }

    //--------------------------------------------------------------------------
    // restore the # of threads for the BLAS
    //--------------------------------------------------------------------------

    #ifdef MKL_ILP64
    mkl_set_num_threads_local (save_nthreads) ;
    #endif

    #endif
}

