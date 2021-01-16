//------------------------------------------------------------------------------
// GB_mkl.h: definitions for using the Intel MKL and/or CBLAS
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#include "GB.h"

#ifndef GB_MKL_H
#define GB_MKL_H

// disable MKL and the CBLAS: work in progress
#undef  GB_HAS_CBLAS
#define GB_HAS_CBLAS 0

#undef  GB_HAS_MKL_GRAPH
#define GB_HAS_MKL_GRAPH 0

//==============================================================================
// determine if MKL and/or CBLAS is available
//==============================================================================

#define GB_INTEL_MKL_VERSION 0

#if !defined ( GBCOMPACT )

    #ifdef MKL_ILP64

        // use the Intel MKL ILP64 parallel CBLAS
        #include "mkl.h"
        #define GB_CBLAS_INT MKL_INT
        #define GB_CBLAS_INT_MAX INT64_MAX

        // INTEL_MKL_VERSION is broken in 2021.1.beta6 (it is 202101
        // but should be 20210001).  See
        // https://software.intel.com/content/www/us/en/develop/documentation/mkl-windows-developer-guide/top/coding-tips/using-predefined-preprocessor-symbols-for-intel-mkl-version-dependent-compilation.html
        #undef  GB_INTEL_MKL_VERSION
        // This definition follows that web page:
        #define GB_INTEL_MKL_VERSION ((__INTEL_MKL__*100+__INTEL_MKL_MINOR__)*100+__INTEL_MKL_UPDATE__)

        #if ( GB_INTEL_MKL_VERSION >= 20200001 )
            // use the Intel MKL_graph library
            #include "mkl_graph.h"
            #include "i_malloc.h"
            #undef  GB_HAS_MKL_GRAPH
            #define GB_HAS_MKL_GRAPH 1
        #endif

    #elif defined ( GB_HAS_CBLAS )

        // FUTURE: other CBLAS packages here
        #include "cblas.h"
        #define GB_CBLAS_INT int
        #define GB_CBLAS_INT_MAX INT32_MAX
        // etc ...

    #endif

#endif

//==============================================================================
// MKL_graph definitions
//==============================================================================

#if GB_HAS_MKL_GRAPH

//------------------------------------------------------------------------------
// semirings
//------------------------------------------------------------------------------

#if ( GB_INTEL_MKL_VERSION == 20200001 )

    // 2020.1: does not have PLUS_SECOND
    #define GB_MKL_GRAPH_SEMIRING_PLUS_SECOND_FP32 (-1)

#else

    // 2021.1.beta6: added PLUS_SECOND
    #define GB_MKL_GRAPH_SEMIRING_PLUS_SECOND_FP32 \
        MKL_GRAPH_SEMIRING_PLUS_SECOND_FP32

#endif

//------------------------------------------------------------------------------
// descriptors
//------------------------------------------------------------------------------

#if ( GB_INTEL_MKL_VERSION == 20200001 )

    #define GB_MKL_GRAPH_FIELD_OUTPUT        MKL_GRAPH_MODIFIER_OUTPUT
    #define GB_MKL_GRAPH_FIELD_FIRST_INPUT   MKL_GRAPH_MODIFIER_FIRST_INPUT
    #define GB_MKL_GRAPH_FIELD_SECOND_INPUT  MKL_GRAPH_MODIFIER_SECOND_INPUT
    #define GB_MKL_GRAPH_FIELD_MASK          MKL_GRAPH_MODIFIER_MASK

    #define GB_MKL_GRAPH_MOD_NONE                   MKL_GRAPH_NO_MODIFIER
    #define GB_MKL_GRAPH_MOD_STRUCTURE_COMPLEMENT   MKL_GRAPH_STRUCTURE_COMPLEMENT
    #define GB_MKL_GRAPH_MOD_TRANSPOSE              MKL_GRAPH_TRANSPOSE
    #define GB_MKL_GRAPH_MOD_REPLACE                MKL_GRAPH_REPLACE
    #define GB_MKL_GRAPH_MOD_ONLY_STRUCTURE         MKL_GRAPH_ONLY_STRUCTURE
    #define GB_MKL_GRAPH_MOD_KEEP_MASK_STRUCTURE    MKL_GRAPH_KEEP_MASK_STRUCTURE

#else

    #define GB_MKL_GRAPH_FIELD_OUTPUT        MKL_GRAPH_FIELD_OUTPUT
    #define GB_MKL_GRAPH_FIELD_FIRST_INPUT   MKL_GRAPH_FIELD_FIRST_INPUT
    #define GB_MKL_GRAPH_FIELD_SECOND_INPUT  MKL_GRAPH_FIELD_SECOND_INPUT
    #define GB_MKL_GRAPH_FIELD_MASK          MKL_GRAPH_FIELD_MASK

    #define GB_MKL_GRAPH_MOD_NONE                   MKL_GRAPH_MOD_NONE
    #define GB_MKL_GRAPH_MOD_STRUCTURE_COMPLEMENT   MKL_GRAPH_MOD_STRUCTURE_COMPLEMENT
    #define GB_MKL_GRAPH_MOD_TRANSPOSE              MKL_GRAPH_MOD_TRANSPOSE
    #define GB_MKL_GRAPH_MOD_REPLACE                MKL_GRAPH_MOD_REPLACE
    #define GB_MKL_GRAPH_MOD_ONLY_STRUCTURE         MKL_GRAPH_MOD_ONLY_STRUCTURE
    #define GB_MKL_GRAPH_MOD_KEEP_MASK_STRUCTURE    MKL_GRAPH_MOD_KEEP_MASK_STRUCTURE

#endif


//------------------------------------------------------------------------------
// GB_info_mkl: map an Intel MKL status to a GraphBLAS GrB_Info
//------------------------------------------------------------------------------

static inline GrB_Info GB_info_mkl      // equivalent GrB_Info
(
    mkl_graph_status_t status           // MKL return status
)
{
    switch (status)
    {
        case MKL_GRAPH_STATUS_SUCCESS:          return (GrB_SUCCESS) ;
        case MKL_GRAPH_STATUS_NOT_INITIALIZED:  return (GrB_UNINITIALIZED_OBJECT) ;
        case MKL_GRAPH_STATUS_ALLOC_FAILED:     return (GrB_OUT_OF_MEMORY) ;
        case MKL_GRAPH_STATUS_INVALID_VALUE:    return (GrB_INVALID_VALUE) ;
        case MKL_GRAPH_STATUS_INTERNAL_ERROR:   return (GrB_PANIC) ;
        case MKL_GRAPH_STATUS_NOT_SUPPORTED:    return (GrB_NO_VALUE) ;
        default:                                return (GrB_PANIC) ;
    }
}

//------------------------------------------------------------------------------
// GB_MKL_OK    call an MKL_graph method and check its result
//------------------------------------------------------------------------------

#define GB_MKL_OK(mkl_method)                                               \
{                                                                           \
    info = GB_info_mkl (mkl_method) ;                                       \
    switch (info)                                                           \
    {                                                                       \
        case GrB_SUCCESS:                                                   \
            break ;                                                         \
        case GrB_OUT_OF_MEMORY:                                             \
        case GrB_UNINITIALIZED_OBJECT:                                      \
        case GrB_INVALID_VALUE:                                             \
        case GrB_PANIC:                                                     \
        case GrB_NO_VALUE:                                                  \
            GB_MKL_FREE_ALL ;                                               \
            return (info) ;                                                 \
        default:                                                            \
            GB_MKL_FREE_ALL ;                                               \
            return (GrB_PANIC) ;                                            \
    }                                                                       \
}

//------------------------------------------------------------------------------
// GB_MKL_GRAPH_*_DESTROY: free an MKL_graph matrix, vector, or descriptor
//------------------------------------------------------------------------------

#if ( GB_INTEL_MKL_VERSION == 20200001 )

    // 2020.1: the arguments are pointers to the opaque structs
    #define GB_MKL_GRAPH_MATRIX_DESTROY(A_mkl)                          \
    {                                                                   \
        if (A_mkl != NULL) mkl_graph_matrix_destroy (A_mkl) ;           \
        A_mkl = NULL ;                                                  \
    }

    #define GB_MKL_GRAPH_VECTOR_DESTROY(x_mkl)                          \
    {                                                                   \
        if (x_mkl != NULL) mkl_graph_vector_destroy (x_mkl) ;           \
        x_mkl = NULL ;                                                  \
    }

    #define GB_MKL_GRAPH_DESCRIPTOR_DESTROY(d_mkl)                      \
    {                                                                   \
        if (d_mkl != NULL) mkl_graph_descriptor_destroy (d_mkl) ;       \
        d_mkl = NULL ;                                                  \
    }

#else

    // 2021.1.beta6:  API has changed: now passing in a handle
    #define GB_MKL_GRAPH_MATRIX_DESTROY(A_mkl)                          \
    {                                                                   \
        if (A_mkl != NULL) mkl_graph_matrix_destroy (&(A_mkl)) ;        \
        A_mkl = NULL ;                                                  \
    }

    #define GB_MKL_GRAPH_VECTOR_DESTROY(x_mkl)                          \
    {                                                                   \
        if (x_mkl != NULL) mkl_graph_vector_destroy (&(x_mkl)) ;        \
        x_mkl = NULL ;                                                  \
    }

    #define GB_MKL_GRAPH_DESCRIPTOR_DESTROY(d_mkl)                      \
    {                                                                   \
        if (d_mkl != NULL) mkl_graph_descriptor_destroy (&(d_mkl)) ;    \
        d_mkl = NULL ;                                                  \
    }

#endif

//------------------------------------------------------------------------------
// GB_AxB_saxpy3_mkl: C=A*B, C<M>=A*B, or C<!M>=A*B using Intel MKL_graph
//------------------------------------------------------------------------------

GrB_Info GB_AxB_saxpy3_mkl          // C = A*B using MKL
(
    GrB_Matrix *Chandle,            // output matrix
    const GrB_Matrix M,             // optional mask matrix
    const bool Mask_comp,           // if true, use !M
    const bool Mask_struct,         // if true, use the only structure of M
    const GrB_Matrix A,             // input matrix A
    const GrB_Matrix B,             // input matrix B
    const GrB_Semiring semiring,    // semiring that defines C=A*B
    const bool flipxy,              // if true, do z=fmult(b,a) vs fmult(a,b)
    bool *mask_applied,             // if true, then mask was applied
    GB_Context Context
) ;

//------------------------------------------------------------------------------
// GB_AxB_dot4_mkl: C+=A'*B where C and B are dense vectors
//------------------------------------------------------------------------------

GrB_Info GB_AxB_dot4_mkl            // c += A*b using MKL
(
    GrB_Vector c,                   // input/output vector (dense)
    const GrB_Matrix A,             // input matrix A
    const GrB_Vector b,             // input vector b (dense)
    const GrB_Semiring semiring,    // semiring that defines C=A'*B
    GB_Context Context
) ;

//------------------------------------------------------------------------------
// GB_AxB_semiring_mkl: map a GraphBLAS semiring to an Intel MKL semiring
//------------------------------------------------------------------------------

int GB_AxB_semiring_mkl         // return the MKL semiring, or -1 if none.
(
    GB_Opcode add_opcode,       // additive monoid
    GB_Opcode mult_opcode,      // multiply operator
    GB_Opcode xycode            // type of x for z = mult (x,y), except for
                                // z = SECOND(x,y) = y, where xycode is the
                                // type of y
) ;

//------------------------------------------------------------------------------
// GB_type_mkl: map a GraphBLAS type to an Intel MKL type
//------------------------------------------------------------------------------

static inline int GB_type_mkl   // return the MKL type, or -1 if none
(
    GB_Type_code type_code      // GraphBLAS type code
)
{
    switch (type_code)
    {
        case GB_BOOL_code:    return (MKL_GRAPH_TYPE_BOOL) ;
        case GB_INT8_code:    return (-1) ;
        case GB_INT16_code:   return (-1) ;
        case GB_INT32_code:   return (MKL_GRAPH_TYPE_INT32) ;
        case GB_INT64_code:   return (MKL_GRAPH_TYPE_INT64) ;
        case GB_UINT8_code:   return (-1) ;
        case GB_UINT16_code:  return (-1) ;
        case GB_UINT32_code:  return (-1) ;
        case GB_UINT64_code:  return (-1) ;
        case GB_FP32_code:    return (MKL_GRAPH_TYPE_FP32) ;
        case GB_FP64_code:    return (MKL_GRAPH_TYPE_FP64) ;
        case GB_FC32_code:    return (-1) ;
        case GB_FC64_code:    return (-1) ;
        default:              return (-1) ;
    }
}

#endif

//==============================================================================
// CBLAS definitions
//==============================================================================

#if GB_HAS_CBLAS

//------------------------------------------------------------------------------
// GB_cblas_saxpy: Y += alpha*X where X and Y are dense float arrays
//------------------------------------------------------------------------------

void GB_cblas_saxpy         // Y += alpha*X
(
    const int64_t n,        // length of X and Y (note the int64_t type)
    const float alpha,      // scale factor
    const float *X,         // the array X, always stride 1
    float *Y,               // the array Y, always stride 1
    int nthreads            // maximum # of threads to use
) ;

//------------------------------------------------------------------------------
// GB_cblas_daxpy: Y += alpha*X where X and Y are dense double arrays
//------------------------------------------------------------------------------

void GB_cblas_daxpy         // Y += alpha*X
(
    const int64_t n,        // length of X and Y (note the int64_t type)
    const double alpha,     // scale factor
    const double *X,        // the array X, always stride 1
    double *Y,              // the array Y, always stride 1
    int nthreads            // maximum # of threads to use
) ;

#endif
#endif

