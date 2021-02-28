//------------------------------------------------------------------------------
// GB_binop:  hard-coded functions for each built-in binary operator
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// If this file is in the Generated/ folder, do not edit it (auto-generated).

#include "GB.h"
#ifndef GBCOMPACT
#include "GB_control.h"
#include "GB_ek_slice.h"
#include "GB_dense.h"
#include "GB_binop__include.h"

// C=binop(A,B) is defined by the following types and operators:

// A+B function (eWiseAdd):         GB_AaddB__land_int8
// A.*B function (eWiseMult):       GB_AemultB__land_int8
// A*D function (colscale):         GB_AxD__land_int8
// D*A function (rowscale):         GB_DxB__land_int8
// C+=A function (dense accum):     GB_Cdense_accumA__land_int8
// C+=x function (dense accum):     GB_Cdense_accumX__land_int8
// C+=A+B function (dense ewise3):  (none)
// C=A+B function (dense ewise3):   GB_Cdense_ewise3_noaccum__land_int8

// C type:   int8_t
// A type:   int8_t
// B type:   int8_t
// BinaryOp: cij = ((aij != 0) && (bij != 0))

#define GB_ATYPE \
    int8_t

#define GB_BTYPE \
    int8_t

#define GB_CTYPE \
    int8_t

// aij = Ax [pA]
#define GB_GETA(aij,Ax,pA)  \
    int8_t aij = Ax [pA]

// bij = Bx [pB]
#define GB_GETB(bij,Bx,pB)  \
    int8_t bij = Bx [pB]

// declare scalar of the same type as C
#define GB_CTYPE_SCALAR(t)  \
    int8_t t

// cij = Ax [pA]
#define GB_COPY_A_TO_C(cij,Ax,pA) cij = Ax [pA] ;

// cij = Bx [pB]
#define GB_COPY_B_TO_C(cij,Bx,pB) cij = Bx [pB] ;

#define GB_CX(p) Cx [p]

// binary operator
#define GB_BINOP(z, x, y)   \
    z = ((x != 0) && (y != 0)) ;

// op is second
#define GB_OP_IS_SECOND \
    0

// op is plus_fp32 or plus_fp64
#define GB_OP_IS_PLUS_REAL \
    0

// op is minus_fp32 or minus_fp64
#define GB_OP_IS_MINUS_REAL \
    0

// GB_cblas_*axpy gateway routine, if it exists for this operator and type:
#define GB_CBLAS_AXPY \
    (none)

// do the numerical phases of GB_add and GB_emult
#define GB_PHASE_2_OF_2

// hard-coded loops can be vectorized
#define GB_PRAGMA_VECTORIZE GB_PRAGMA_SIMD

// disable this operator and use the generic case if these conditions hold
#define GB_DISABLE \
    (GxB_NO_LAND || GxB_NO_INT8 || GxB_NO_LAND_INT8)

//------------------------------------------------------------------------------
// C += A+B, all 3 matrices dense
//------------------------------------------------------------------------------

#if 0

// The op must be MIN, MAX, PLUS, MINUS, RMINUS, TIMES, DIV, or RDIV.

void (none)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    const GrB_Matrix B,
    const int nthreads
)
{ 
    #include "GB_dense_ewise3_accum_template.c"
}

#endif

//------------------------------------------------------------------------------
// C = A+B, all 3 matrices dense
//------------------------------------------------------------------------------

GrB_Info GB_Cdense_ewise3_noaccum__land_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    const GrB_Matrix B,
    const int nthreads
)
{ 
    #if GB_DISABLE
    return (GrB_NO_VALUE) ;
    #else
    #include "GB_dense_ewise3_noaccum_template.c"
    return (GrB_SUCCESS) ;
    #endif
}

//------------------------------------------------------------------------------
// C += A, accumulate a sparse matrix into a dense matrix
//------------------------------------------------------------------------------

GrB_Info GB_Cdense_accumA__land_int8
(
    GrB_Matrix C,
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
    
    { 
        #include "GB_dense_subassign_23_template.c"
    }
    
    return (GrB_SUCCESS) ;
    #endif
}

//------------------------------------------------------------------------------
// C += x, accumulate a scalar into a dense matrix
//------------------------------------------------------------------------------

GrB_Info GB_Cdense_accumX__land_int8
(
    GrB_Matrix C,
    const GB_void *p_ywork,
    const int nthreads
)
{
    #if GB_DISABLE
    return (GrB_NO_VALUE) ;
    #else
    
    { 
        int8_t ywork = (*((int8_t *) p_ywork)) ;
        #include "GB_dense_subassign_22_template.c"
        return (GrB_SUCCESS) ;
    }
    
    return (GrB_SUCCESS) ;
    #endif
}

//------------------------------------------------------------------------------
// C = A*D, column scale with diagonal D matrix
//------------------------------------------------------------------------------

GrB_Info GB_AxD__land_int8
(
    GrB_Matrix C,
    const GrB_Matrix A, bool A_is_pattern,
    const GrB_Matrix D, bool D_is_pattern,
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
    int8_t *GB_RESTRICT Cx = C->x ;
    #include "GB_AxB_colscale_meta.c"
    return (GrB_SUCCESS) ;
    #endif
}

//------------------------------------------------------------------------------
// C = D*B, row scale with diagonal D matrix
//------------------------------------------------------------------------------

GrB_Info GB_DxB__land_int8
(
    GrB_Matrix C,
    const GrB_Matrix D, bool D_is_pattern,
    const GrB_Matrix B, bool B_is_pattern,
    int nthreads
)
{ 
    #if GB_DISABLE
    return (GrB_NO_VALUE) ;
    #else
    int8_t *GB_RESTRICT Cx = C->x ;
    #include "GB_AxB_rowscale_meta.c"
    return (GrB_SUCCESS) ;
    #endif
}

//------------------------------------------------------------------------------
// eWiseAdd: C = A+B or C<M> = A+B
//------------------------------------------------------------------------------

GrB_Info GB_AaddB__land_int8
(
    GrB_Matrix C,
    const GrB_Matrix M,
    const bool Mask_struct,
    const GrB_Matrix A,
    const GrB_Matrix B,
    const bool Ch_is_Mh,
    const int64_t *GB_RESTRICT C_to_M,
    const int64_t *GB_RESTRICT C_to_A,
    const int64_t *GB_RESTRICT C_to_B,
    const GB_task_struct *GB_RESTRICT TaskList,
    const int ntasks,
    const int nthreads
)
{ 
    #if GB_DISABLE
    return (GrB_NO_VALUE) ;
    #else
    #include "GB_add_template.c"
    return (GrB_SUCCESS) ;
    #endif
}

//------------------------------------------------------------------------------
// eWiseMult: C = A.*B or C<M> = A.*B
//------------------------------------------------------------------------------

GrB_Info GB_AemultB__land_int8
(
    GrB_Matrix C,
    const GrB_Matrix M,
    const bool Mask_struct,
    const GrB_Matrix A,
    const GrB_Matrix B,
    const int64_t *GB_RESTRICT C_to_M,
    const int64_t *GB_RESTRICT C_to_A,
    const int64_t *GB_RESTRICT C_to_B,
    const GB_task_struct *GB_RESTRICT TaskList,
    const int ntasks,
    const int nthreads
)
{ 
    #if GB_DISABLE
    return (GrB_NO_VALUE) ;
    #else
    #include "GB_emult_template.c"
    return (GrB_SUCCESS) ;
    #endif
}

#endif

