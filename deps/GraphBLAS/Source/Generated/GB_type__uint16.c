//------------------------------------------------------------------------------
// GB_type:  hard-coded functions for each built-in type
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// If this file is in the Generated/ folder, do not edit it (auto-generated).

#include "GB.h"
#ifndef GBCOMPACT
#include "GB_control.h"
#include "GB_ek_slice.h"
#include "GB_type__include.h"

// The operation is defined by the following types and operators:

// C<M>=x (C is dense): GB_Cdense_05d__uint16
// C<A>=A (C is dense): GB_Cdense_06d__uint16
// C<M>=A (C is empty, A dense): GB_Cdense_25__uint16

// C type:   uint16_t

#define GB_CTYPE \
    uint16_t

// C must have the same type as A or the scalar x
#define GB_ATYPE GB_CTYPE

#define GB_CX(p) Cx [p]

// Cx [p] = scalar
#define GB_COPY_SCALAR_TO_C(p,x) Cx [p] = x

// Cx [p] = Ax [pA]
#define GB_COPY_A_TO_C(Cx,p,Ax,pA) Cx [p] = Ax [pA]

// test the mask condition with Ax [pA]
#define GB_AX_MASK(Ax,pA,asize) \
    (Ax [pA] != 0)

// hard-coded loops can be vectorized
#define GB_PRAGMA_SIMD_VECTORIZE GB_PRAGMA_SIMD

// disable this operator and use the generic case if these conditions hold
#define GB_DISABLE \
    (GxB_NO_UINT16)

//------------------------------------------------------------------------------
// C<M>=x, when C is dense
//------------------------------------------------------------------------------

GrB_Info GB_Cdense_05d__uint16
(
    GrB_Matrix C,
    const GrB_Matrix M,
    const bool Mask_struct,
    const GB_void *p_cwork,     // scalar of type C->type
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const int ntasks,
    const int nthreads
)
{ 
    #if GB_DISABLE
    return (GrB_NO_VALUE) ;
    #else
    uint16_t cwork = (*((uint16_t *) p_cwork)) ;
    #include "GB_dense_subassign_05d_template.c"
    return (GrB_SUCCESS) ;
    #endif
}

//------------------------------------------------------------------------------
// C<A>=A, when C is dense
//------------------------------------------------------------------------------

GrB_Info GB_Cdense_06d__uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    const bool Mask_struct,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const int ntasks,
    const int nthreads
)
{ 
    #if GB_DISABLE
    return (GrB_NO_VALUE) ;
    #else
    ASSERT (C->type == A->type) ;
    #include "GB_dense_subassign_06d_template.c"
    return (GrB_SUCCESS) ;
    #endif
}

//------------------------------------------------------------------------------
// C<M>=A, when C is empty and A is dense
//------------------------------------------------------------------------------

GrB_Info GB_Cdense_25__uint16
(
    GrB_Matrix C,
    const GrB_Matrix M,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const int ntasks,
    const int nthreads
)
{ 
    #if GB_DISABLE
    return (GrB_NO_VALUE) ;
    #else
    ASSERT (C->type == A->type) ;
    #include "GB_dense_subassign_25_template.c"
    return (GrB_SUCCESS) ;
    #endif
}

//------------------------------------------------------------------------------
// convert sparse to bitmap
//------------------------------------------------------------------------------

GrB_Info GB_convert_s2b__uint16
(
    GrB_Matrix A,
    GB_void *GB_RESTRICT Ax_new_void,
    int8_t  *GB_RESTRICT Ab,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const int ntasks,
    const int nthreads
)
{ 
    #if GB_DISABLE
    return (GrB_NO_VALUE) ;
    #else
    uint16_t *GB_RESTRICT Ax_new = (uint16_t *) Ax_new_void ;
    #include "GB_convert_sparse_to_bitmap_template.c"
    return (GrB_SUCCESS) ;
    #endif
}

#endif

