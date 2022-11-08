//------------------------------------------------------------------------------
// GB_binop:  hard-coded functions for each built-in binary operator
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// If this file is in the Generated2/ folder, do not edit it
// (it is auto-generated from Generator/*).

#include "GB.h"
#ifndef GBCUDA_DEV
#include "GB_emult.h"
#include "GB_control.h"
#include "GB_ek_slice.h"
#include "GB_dense.h"
#include "GB_atomics.h"
#include "GB_bitmap_assign_methods.h"
#include "GB_binop__include.h"

// C=binop(A,B) is defined by the following types and operators:

// A+B function (eWiseAdd):         GB (_AaddB)
// A.*B function (eWiseMult):       GB (_AemultB)
// A.*B function (eWiseMult):       GB (_AemultB_02)
// A.*B function (eWiseMult):       GB (_AemultB_04)
// A.*B function (eWiseMult):       GB (_AemultB_bitmap)
// A*D function (colscale):         GB (_AxD)
// D*A function (rowscale):         GB (_DxB)
// C+=B function (dense accum):     GB (_Cdense_accumB)
// C+=b function (dense accum):     GB (_Cdense_accumb)
// C+=A+B function (dense ewise3):  GB (_Cdense_ewise3_accum)
// C=A+B function (dense ewise3):   GB (_Cdense_ewise3_noaccum)
// C=scalar+B                       GB (_bind1st)
// C=scalar+B'                      GB (_bind1st_tran)
// C=A+scalar                       GB (_bind2nd)
// C=A'+scalar                      GB (_bind2nd_tran)

// C type:     GB_ctype
// A type:     GB_atype
// A pattern?  GB_a_is_pattern
// B type:     GB_btype
// B pattern?  GB_b_is_pattern

// BinaryOp:   GB_binaryop(cij,aij,bij,i,j)

#define GB_ATYPE \
    GB_atype

#define GB_BTYPE \
    GB_btype

#define GB_CTYPE \
    GB_ctype

// true if the types of A and B are identical
#define GB_ATYPE_IS_BTYPE \
    GB_atype_is_btype

// true if the types of C and A are identical
#define GB_CTYPE_IS_ATYPE \
    GB_ctype_is_atype

// true if the types of C and B are identical
#define GB_CTYPE_IS_BTYPE \
    GB_ctype_is_btype

// aij = Ax [pA]
#define GB_GETA(aij,Ax,pA,A_iso)  \
    GB_geta(aij,Ax,pA,A_iso)

// true if values of A are not used
#define GB_A_IS_PATTERN \
    GB_a_is_pattern \

// bij = Bx [pB]
#define GB_GETB(bij,Bx,pB,B_iso)  \
    GB_getb(bij,Bx,pB,B_iso)

// true if values of B are not used
#define GB_B_IS_PATTERN \
    GB_b_is_pattern \

// declare scalar of the same type as C
#define GB_CTYPE_SCALAR(t)  \
    GB_ctype t

// cij = Ax [pA]
#define GB_COPY_A_TO_C(cij,Ax,pA,A_iso) \
    GB_copy_a_to_c(cij,Ax,pA,A_iso)

// cij = Bx [pB]
#define GB_COPY_B_TO_C(cij,Bx,pB,B_iso) \
    GB_copy_b_to_c(cij,Bx,pB,B_iso)

#define GB_CX(p) Cx [p]

// binary operator
#define GB_BINOP(z,x,y,i,j) \
    GB_binaryop(z,x,y,i,j) ;

// true if the binop must be flipped
#define GB_BINOP_FLIP \
    GB_binaryop_flip

// op is second
#define GB_OP_IS_SECOND \
    GB_op_is_second

// do the numerical phases of GB_add and GB_emult
#define GB_PHASE_2_OF_2

// hard-coded loops can be vectorized
#define GB_PRAGMA_SIMD_VECTORIZE GB_PRAGMA_SIMD

// disable this operator and use the generic case if these conditions hold
#define GB_DISABLE \
    GB_disable

//------------------------------------------------------------------------------
// C += A+B, all 3 matrices dense
//------------------------------------------------------------------------------

if_is_binop_subset

// The op must be MIN, MAX, PLUS, MINUS, RMINUS, TIMES, DIV, or RDIV.

void GB (_Cdense_ewise3_accum)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    const GrB_Matrix B,
    const int nthreads
)
{ 
    #include "GB_dense_ewise3_accum_template.c"
}

endif_is_binop_subset

//------------------------------------------------------------------------------
// C = A+B, all 3 matrices dense
//------------------------------------------------------------------------------

void GB (_Cdense_ewise3_noaccum)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    const GrB_Matrix B,
    const int nthreads
)
{ 
    #include "GB_dense_ewise3_noaccum_template.c"
}

//------------------------------------------------------------------------------
// C += B, accumulate a sparse matrix into a dense matrix
//------------------------------------------------------------------------------

GrB_Info GB (_Cdense_accumB)
(
    GrB_Matrix C,
    const GrB_Matrix B,
    const int64_t *B_ek_slicing, const int B_ntasks, const int B_nthreads
)
{
    #if GB_DISABLE
    return (GrB_NO_VALUE) ;
    #else
    if_C_dense_update
    { 
        #include "GB_dense_subassign_23_template.c"
    }
    endif_C_dense_update
    return (GrB_SUCCESS) ;
    #endif
}

//------------------------------------------------------------------------------
// C += b, accumulate a scalar into a dense matrix
//------------------------------------------------------------------------------

GrB_Info GB (_Cdense_accumb)
(
    GrB_Matrix C,
    const GB_void *p_bwork,
    const int nthreads
)
{
    #if GB_DISABLE
    return (GrB_NO_VALUE) ;
    #else
    if_C_dense_update
    { 
        // get the scalar b for C += b, of type GB_btype
        GB_btype bwork = (*((GB_btype *) p_bwork)) ;
        #include "GB_dense_subassign_22_template.c"
        return (GrB_SUCCESS) ;
    }
    endif_C_dense_update
    return (GrB_SUCCESS) ;
    #endif
}

//------------------------------------------------------------------------------
// C = A*D, column scale with diagonal D matrix
//------------------------------------------------------------------------------

if_binop_is_semiring_multiplier

GrB_Info GB (_AxD)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    const GrB_Matrix D,
    const int64_t *A_ek_slicing, const int A_ntasks, const int A_nthreads
)
{ 
    #if GB_DISABLE
    return (GrB_NO_VALUE) ;
    #else
    GB_ctype *restrict Cx = (GB_ctype *) C->x ;
    #include "GB_AxB_colscale_template.c"
    return (GrB_SUCCESS) ;
    #endif
}

endif_binop_is_semiring_multiplier

//------------------------------------------------------------------------------
// C = D*B, row scale with diagonal D matrix
//------------------------------------------------------------------------------

if_binop_is_semiring_multiplier

GrB_Info GB (_DxB)
(
    GrB_Matrix C,
    const GrB_Matrix D,
    const GrB_Matrix B,
    int nthreads
)
{ 
    #if GB_DISABLE
    return (GrB_NO_VALUE) ;
    #else
    GB_ctype *restrict Cx = (GB_ctype *) C->x ;
    #include "GB_AxB_rowscale_template.c"
    return (GrB_SUCCESS) ;
    #endif
}

endif_binop_is_semiring_multiplier

//------------------------------------------------------------------------------
// eWiseAdd: C=A+B, C<M>=A+B, C<!M>=A+B
//------------------------------------------------------------------------------

GrB_Info GB (_AaddB)
(
    GrB_Matrix C,
    const int C_sparsity,
    const GrB_Matrix M,
    const bool Mask_struct,
    const bool Mask_comp,
    const GrB_Matrix A,
    const GrB_Matrix B,
    const bool is_eWiseUnion,
    const GB_void *alpha_scalar_in,
    const GB_void *beta_scalar_in,
    const bool Ch_is_Mh,
    const int64_t *restrict C_to_M,
    const int64_t *restrict C_to_A,
    const int64_t *restrict C_to_B,
    const GB_task_struct *restrict TaskList,
    const int C_ntasks,
    const int C_nthreads,
    GB_Context Context
)
{ 
    #if GB_DISABLE
    return (GrB_NO_VALUE) ;
    #else
    GB_WERK_DECLARE (M_ek_slicing, int64_t) ;
    GB_WERK_DECLARE (A_ek_slicing, int64_t) ;
    GB_WERK_DECLARE (B_ek_slicing, int64_t) ;
    GB_atype alpha_scalar ;
    GB_btype beta_scalar ;
    if (is_eWiseUnion)
    {
        alpha_scalar = (*((GB_atype *) alpha_scalar_in)) ;
        beta_scalar  = (*((GB_btype *) beta_scalar_in )) ;
    }
    #include "GB_add_template.c"
    GB_FREE_WORKSPACE ;
    return (GrB_SUCCESS) ;
    #endif
}

//------------------------------------------------------------------------------
// eWiseMult: C=A.*B, C<M>=A.*B, or C<M!>=A.*B where C is sparse/hyper
//------------------------------------------------------------------------------

if_binop_emult_is_enabled

GrB_Info GB (_AemultB)
(
    GrB_Matrix C,
    const int C_sparsity,
    const int ewise_method,
    const GrB_Matrix M,
    const bool Mask_struct,
    const bool Mask_comp,
    const GrB_Matrix A,
    const GrB_Matrix B,
    const int64_t *restrict C_to_M,
    const int64_t *restrict C_to_A,
    const int64_t *restrict C_to_B,
    const GB_task_struct *restrict TaskList,
    const int C_ntasks,
    const int C_nthreads,
    GB_Context Context
)
{ 
    #if GB_DISABLE
    return (GrB_NO_VALUE) ;
    #else
    #include "GB_emult_meta.c"
    return (GrB_SUCCESS) ;
    #endif
}

endif_binop_emult_is_enabled

//------------------------------------------------------------------------------
// eWiseMult: C<#> = A.*B when A is sparse/hyper and B is bitmap/full
//------------------------------------------------------------------------------

if_binop_emult_is_enabled

GrB_Info GB (_AemultB_02)
(
    GrB_Matrix C,
    const GrB_Matrix M,
    const bool Mask_struct,
    const bool Mask_comp,
    const GrB_Matrix A,
    const GrB_Matrix B,
    const bool flipxy,
    const int64_t *restrict Cp_kfirst,
    const int64_t *A_ek_slicing, const int A_ntasks, const int A_nthreads
)
{ 
    #if GB_DISABLE
    return (GrB_NO_VALUE) ;
    #else
    #if GB_BINOP_FLIP
        // The operator is not commutative, and does not have a flipped
        // variant.  For example z=atan2(y,x).
        if (flipxy)
        {
            // use fmult(y,x)
            #undef  GB_FLIPPED
            #define GB_FLIPPED 1
            #include "GB_emult_02_template.c"
        }
        else
        {
            // use fmult(x,y)
            #undef  GB_FLIPPED
            #define GB_FLIPPED 0
            #include "GB_emult_02_template.c"
        }
    #else
        // No need to handle the flip: the operator is either commutative, or
        // has been handled by changing z=div(y,x) to z=rdiv(x,y) for example.
        #undef  GB_FLIPPED
        #define GB_FLIPPED 0
        #include "GB_emult_02_template.c"
    #endif
    return (GrB_SUCCESS) ;
    #endif
}

endif_binop_emult_is_enabled

//------------------------------------------------------------------------------
// eWiseMult: C<M> = A.*B, M sparse/hyper, A and B bitmap/full
//------------------------------------------------------------------------------

if_binop_emult_is_enabled

GrB_Info GB (_AemultB_04)
(
    GrB_Matrix C,
    const GrB_Matrix M,
    const bool Mask_struct,
    const GrB_Matrix A,
    const GrB_Matrix B,
    const int64_t *restrict Cp_kfirst,
    const int64_t *M_ek_slicing, const int M_ntasks, const int M_nthreads
)
{ 
    #if GB_DISABLE
    return (GrB_NO_VALUE) ;
    #else
    #include "GB_emult_04_template.c"
    return (GrB_SUCCESS) ;
    #endif
}

endif_binop_emult_is_enabled

//------------------------------------------------------------------------------
// eWiseMult: C=A.*B, C<M>=A.*B, C<!M>=A.*B where C is bitmap
//------------------------------------------------------------------------------

if_binop_emult_is_enabled

GrB_Info GB (_AemultB_bitmap)
(
    GrB_Matrix C,
    const int ewise_method,
    const GrB_Matrix M,
    const bool Mask_struct,
    const bool Mask_comp,
    const GrB_Matrix A,
    const GrB_Matrix B,
    const int64_t *M_ek_slicing, const int M_ntasks, const int M_nthreads,
    const int C_nthreads,
    GB_Context Context
)
{ 
    #if GB_DISABLE
    return (GrB_NO_VALUE) ;
    #else
    #include "GB_bitmap_emult_template.c"
    return (GrB_SUCCESS) ;
    #endif
}

endif_binop_emult_is_enabled

//------------------------------------------------------------------------------
// Cx = op (x,Bx):  apply a binary operator to a matrix with scalar bind1st
//------------------------------------------------------------------------------

if_binop_bind_is_enabled

GrB_Info GB (_bind1st)
(
    GB_void *Cx_output,         // Cx and Bx may be aliased
    const GB_void *x_input,
    const GB_void *Bx_input,
    const int8_t *restrict Bb,
    int64_t bnz,
    int nthreads
)
{ 
    #if GB_DISABLE
    return (GrB_NO_VALUE) ;
    #else
    GB_ctype *Cx = (GB_ctype *) Cx_output ;
    GB_atype   x = (*((GB_atype *) x_input)) ;
    GB_btype *Bx = (GB_btype *) Bx_input ;
    int64_t p ;
    #pragma omp parallel for num_threads(nthreads) schedule(static)
    for (p = 0 ; p < bnz ; p++)
    {
        if (!GBB (Bb, p)) continue ;
        GB_getb(bij, Bx, p, false) ;
        GB_binaryop(Cx [p], x, bij, 0, 0) ;
    }
    return (GrB_SUCCESS) ;
    #endif
}

endif_binop_bind_is_enabled

//------------------------------------------------------------------------------
// Cx = op (Ax,y):  apply a binary operator to a matrix with scalar bind2nd
//------------------------------------------------------------------------------

if_binop_bind_is_enabled

GrB_Info GB (_bind2nd)
(
    GB_void *Cx_output,         // Cx and Ax may be aliased
    const GB_void *Ax_input,
    const GB_void *y_input,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
)
{ 
    #if GB_DISABLE
    return (GrB_NO_VALUE) ;
    #else
    int64_t p ;
    GB_ctype *Cx = (GB_ctype *) Cx_output ;
    GB_atype *Ax = (GB_atype *) Ax_input ;
    GB_btype   y = (*((GB_btype *) y_input)) ;
    #pragma omp parallel for num_threads(nthreads) schedule(static)
    for (p = 0 ; p < anz ; p++)
    {
        if (!GBB (Ab, p)) continue ;
        GB_geta(aij, Ax, p, false) ;
        GB_binaryop(Cx [p], aij, y, 0, 0) ;
    }
    return (GrB_SUCCESS) ;
    #endif
}

endif_binop_bind_is_enabled

//------------------------------------------------------------------------------
// C = op (x, A'): transpose and apply a binary operator
//------------------------------------------------------------------------------

if_binop_bind_is_enabled

// cij = op (x, aij), no typecasting (in spite of the macro name)
#undef  GB_CAST_OP
#define GB_CAST_OP(pC,pA)                       \
{                                               \
    GB_getb(aij, Ax, pA, false) ;               \
    GB_binaryop(Cx [pC], x, aij, 0, 0) ;        \
}

GrB_Info GB (_bind1st_tran)
(
    GrB_Matrix C,
    const GB_void *x_input,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
)
{ 
    // GB_unop_transpose.c uses GB_ATYPE, but A is
    // the 2nd input to binary operator z=f(x,y).
    #undef  GB_ATYPE
    #define GB_ATYPE \
    GB_btype
    #if GB_DISABLE
    return (GrB_NO_VALUE) ;
    #else
    GB_atype x = (*((const GB_atype *) x_input)) ;
    #include "GB_unop_transpose.c"
    return (GrB_SUCCESS) ;
    #endif
    #undef  GB_ATYPE
    #define GB_ATYPE \
    GB_atype
}

endif_binop_bind_is_enabled

//------------------------------------------------------------------------------
// C = op (A', y): transpose and apply a binary operator
//------------------------------------------------------------------------------

if_binop_bind_is_enabled

// cij = op (aij, y), no typecasting (in spite of the macro name)
#undef  GB_CAST_OP
#define GB_CAST_OP(pC,pA)                       \
{                                               \
    GB_geta(aij, Ax, pA, false) ;               \
    GB_binaryop(Cx [pC], aij, y, 0, 0) ;        \
}

GrB_Info GB (_bind2nd_tran)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    const GB_void *y_input,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
)
{ 
    #if GB_DISABLE
    return (GrB_NO_VALUE) ;
    #else
    GB_btype y = (*((const GB_btype *) y_input)) ;
    #include "GB_unop_transpose.c"
    return (GrB_SUCCESS) ;
    #endif
}

endif_binop_bind_is_enabled

#endif

