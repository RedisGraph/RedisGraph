//------------------------------------------------------------------------------
// GB_type:  hard-coded functions for each built-in type
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// If this file is in the Generated/ folder, do not edit it (auto-generated).

#include "GB.h"
#ifndef GBCOMPACT
#include "GB_control.h"
#include "GB_ek_slice.h"
#include "GB_type__include.h"

// The operation is defined by the following types and operators:

// C<M>=x (C is dense): GB_Cdense_05d__fp64
// C<A>=A (C is dense): GB_Cdense_06d__fp64

// C type:   double

#define GB_CTYPE \
    double

#define GB_CX(p) Cx [p]

// Cx [p] = scalar
#define GB_COPY_SCALAR_TO_C(p,x) Cx [p] = x

// Cx [p] = Ax [pA]
#define GB_COPY_A_TO_C(Cx,p,Ax,pA) Cx [p] = Ax [pA]

// test the mask condition with Ax [pA]
#define GB_AX_MASK(Ax,pA,asize) (Ax [pA] != 0)

// hard-coded loops can be vectorized
#define GB_PRAGMA_VECTORIZE GB_PRAGMA_SIMD

// disable this operator and use the generic case if these conditions hold
#define GB_DISABLE \
    (GxB_NO_FP64)

//------------------------------------------------------------------------------
// C<M>=x, when C is dense
//------------------------------------------------------------------------------

GrB_Info GB_Cdense_05d__fp64
(
    GrB_Matrix C,
    const GrB_Matrix M,
    const bool Mask_struct,
    const GB_void *p_cwork,
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
    double cwork = (*((double *) p_cwork)) ;
    #include "GB_dense_subassign_05d_template.c"
    return (GrB_SUCCESS) ;
    #endif
}

//------------------------------------------------------------------------------
// C<A>=A, when C is dense
//------------------------------------------------------------------------------

GrB_Info GB_Cdense_06d__fp64
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
    #include "GB_dense_subassign_06d_template.c"
    return (GrB_SUCCESS) ;
    #endif
}

//------------------------------------------------------------------------------
// C<M>=A, when C is empty and A is dense
//------------------------------------------------------------------------------

GrB_Info GB_Cdense_25__fp64
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
    #include "GB_dense_subassign_25_template.c"
    return (GrB_SUCCESS) ;
    #endif
}

#endif

