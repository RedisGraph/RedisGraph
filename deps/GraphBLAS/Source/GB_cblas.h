//------------------------------------------------------------------------------
// GB_cblas.h: definitions to use the CBLAS
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#ifndef GB_CBLAS_H
#define GB_CBLAS_H

#ifdef GB_HAS_CBLAS

    #ifdef MKL_ILP64

    // use the Intel MKL ILP64 parallel BLAS
    #include "mkl.h"
    #define GB_CBLAS_INT MKL_INT
    #define GB_CBLAS_INT_MAX INT64_MAX

    #else

    // FUTURE: other BLAS packages here
    #include "cblas.h"
    #define GB_CBLAS_INT int
    #define GB_CBLAS_INT_MAX INT32_MAX
    // etc ...

    #endif

#endif
#endif

