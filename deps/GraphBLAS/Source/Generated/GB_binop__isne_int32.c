//------------------------------------------------------------------------------
// GB_binop:  hard-coded functions for each built-in binary operator
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// If this file is in the Generated/ folder, do not edit it (auto-generated).

#include "GB.h"
#ifndef GBCOMPACT
#include "GB_control.h"
#include "GB_ek_slice.h"
#include "GB_dense.h"
#include "GB_atomics.h"
#include "GB_bitmap_assign_methods.h"
#include "GB_binop__include.h"

// C=binop(A,B) is defined by the following types and operators:

// A+B function (eWiseAdd):         GB_AaddB__isne_int32
// A.*B function (eWiseMult):       GB_AemultB__isne_int32
// A*D function (colscale):         GB_AxD__isne_int32
// D*A function (rowscale):         GB_DxB__isne_int32
// C+=B function (dense accum):     GB_Cdense_accumB__isne_int32
// C+=b function (dense accum):     GB_Cdense_accumb__isne_int32
// C+=A+B function (dense ewise3):  (none)
// C=A+B function (dense ewise3):   GB_Cdense_ewise3_noaccum__isne_int32
// C=scalar+B                       GB_bind1st__isne_int32
// C=scalar+B'                      GB_bind1st_tran__isne_int32
// C=A+scalar                       GB_bind2nd__isne_int32
// C=A'+scalar                      GB_bind2nd_tran__isne_int32

// C type:   int32_t
// A type:   int32_t
// B,b type: int32_t
// BinaryOp: cij = (aij != bij)

#define GB_ATYPE \
    int32_t

#define GB_BTYPE \
    int32_t

#define GB_CTYPE \
    int32_t

// true if the types of A and B are identical
#define GB_ATYPE_IS_BTYPE \
    1

// true if the types of C and A are identical
#define GB_CTYPE_IS_ATYPE \
    1

// true if the types of C and B are identical
#define GB_CTYPE_IS_BTYPE \
    1

// aij = Ax [pA]
#define GB_GETA(aij,Ax,pA)  \
    int32_t aij = Ax [pA]

// bij = Bx [pB]
#define GB_GETB(bij,Bx,pB)  \
    int32_t bij = Bx [pB]

// declare scalar of the same type as C
#define GB_CTYPE_SCALAR(t)  \
    int32_t t

// cij = Ax [pA]
#define GB_COPY_A_TO_C(cij,Ax,pA) \
    cij = Ax [pA]

// cij = Bx [pB]
#define GB_COPY_B_TO_C(cij,Bx,pB) \
    cij = Bx [pB]

#define GB_CX(p) Cx [p]

// binary operator
#define GB_BINOP(z, x, y, i, j) \
    z = (x != y) ;

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
#define GB_PRAGMA_SIMD_VECTORIZE GB_PRAGMA_SIMD

// disable this operator and use the generic case if these conditions hold
#define GB_DISABLE \
    (GxB_NO_ISNE || GxB_NO_INT32 || GxB_NO_ISNE_INT32)

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

GrB_Info GB_Cdense_ewise3_noaccum__isne_int32
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
// C += B, accumulate a sparse matrix into a dense matrix
//------------------------------------------------------------------------------

GrB_Info GB_Cdense_accumB__isne_int32
(
    GrB_Matrix C,
    const GrB_Matrix B,
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
// C += b, accumulate a scalar into a dense matrix
//------------------------------------------------------------------------------

GrB_Info GB_Cdense_accumb__isne_int32
(
    GrB_Matrix C,
    const GB_void *p_bwork,
    const int nthreads
)
{
    #if GB_DISABLE
    return (GrB_NO_VALUE) ;
    #else
    
    { 
        // get the scalar b for C += b, of type int32_t
        int32_t bwork = (*((int32_t *) p_bwork)) ;
        #include "GB_dense_subassign_22_template.c"
        return (GrB_SUCCESS) ;
    }
    
    return (GrB_SUCCESS) ;
    #endif
}

//------------------------------------------------------------------------------
// C = A*D, column scale with diagonal D matrix
//------------------------------------------------------------------------------



GrB_Info GB_AxD__isne_int32
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
    int32_t *GB_RESTRICT Cx = (int32_t *) C->x ;
    #include "GB_AxB_colscale_meta.c"
    return (GrB_SUCCESS) ;
    #endif
}



//------------------------------------------------------------------------------
// C = D*B, row scale with diagonal D matrix
//------------------------------------------------------------------------------



GrB_Info GB_DxB__isne_int32
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
    int32_t *GB_RESTRICT Cx = (int32_t *) C->x ;
    #include "GB_AxB_rowscale_meta.c"
    return (GrB_SUCCESS) ;
    #endif
}



//------------------------------------------------------------------------------
// eWiseAdd: C = A+B or C<M> = A+B
//------------------------------------------------------------------------------

#undef  GB_FREE_ALL
#define GB_FREE_ALL                                                     \
{                                                                       \
    GB_ek_slice_free (&pstart_Mslice, &kfirst_Mslice, &klast_Mslice) ;  \
    GB_ek_slice_free (&pstart_Aslice, &kfirst_Aslice, &klast_Aslice) ;  \
    GB_ek_slice_free (&pstart_Bslice, &kfirst_Bslice, &klast_Bslice) ;  \
}

GrB_Info GB_AaddB__isne_int32
(
    GrB_Matrix C,
    const int C_sparsity,
    const GrB_Matrix M,
    const bool Mask_struct,
    const bool Mask_comp,
    const GrB_Matrix A,
    const GrB_Matrix B,
    const bool Ch_is_Mh,
    const int64_t *GB_RESTRICT C_to_M,
    const int64_t *GB_RESTRICT C_to_A,
    const int64_t *GB_RESTRICT C_to_B,
    const GB_task_struct *GB_RESTRICT TaskList,
    const int C_ntasks,
    const int C_nthreads,
    GB_Context Context
)
{ 
    #if GB_DISABLE
    return (GrB_NO_VALUE) ;
    #else
    int64_t *pstart_Mslice = NULL, *kfirst_Mslice = NULL, *klast_Mslice = NULL ;
    int64_t *pstart_Aslice = NULL, *kfirst_Aslice = NULL, *klast_Aslice = NULL ;
    int64_t *pstart_Bslice = NULL, *kfirst_Bslice = NULL, *klast_Bslice = NULL ;
    #include "GB_add_template.c"
    GB_FREE_ALL ;
    return (GrB_SUCCESS) ;
    #endif
}

//------------------------------------------------------------------------------
// eWiseMult: C = A.*B or C<M> = A.*B
//------------------------------------------------------------------------------

GrB_Info GB_AemultB__isne_int32
(
    GrB_Matrix C,
    const int C_sparsity,
    const GrB_Matrix M,
    const bool Mask_struct,
    const bool Mask_comp,
    const GrB_Matrix A,
    const GrB_Matrix B,
    const int64_t *GB_RESTRICT C_to_M,
    const int64_t *GB_RESTRICT C_to_A,
    const int64_t *GB_RESTRICT C_to_B,
    const GB_task_struct *GB_RESTRICT TaskList,
    const int C_ntasks,
    const int C_nthreads,
    GB_Context Context
)
{ 
    #if GB_DISABLE
    return (GrB_NO_VALUE) ;
    #else
    int64_t *pstart_Mslice = NULL, *kfirst_Mslice = NULL, *klast_Mslice = NULL ;
    int64_t *pstart_Aslice = NULL, *kfirst_Aslice = NULL, *klast_Aslice = NULL ;
    int64_t *pstart_Bslice = NULL, *kfirst_Bslice = NULL, *klast_Bslice = NULL ;
    #include "GB_emult_template.c"
    GB_FREE_ALL ;
    return (GrB_SUCCESS) ;
    #endif
}

//------------------------------------------------------------------------------
// Cx = op (x,Bx):  apply a binary operator to a matrix with scalar bind1st
//------------------------------------------------------------------------------



GrB_Info GB_bind1st__isne_int32
(
    GB_void *Cx_output,         // Cx and Bx may be aliased
    const GB_void *x_input,
    const GB_void *Bx_input,
    const int8_t *GB_RESTRICT Bb,
    int64_t anz,
    int nthreads
)
{ 
    #if GB_DISABLE
    return (GrB_NO_VALUE) ;
    #else
    int32_t *Cx = (int32_t *) Cx_output ;
    int32_t   x = (*((int32_t *) x_input)) ;
    int32_t *Bx = (int32_t *) Bx_input ;
    int64_t p ;
    #pragma omp parallel for num_threads(nthreads) schedule(static)
    for (p = 0 ; p < anz ; p++)
    {
        if (!GBB (Bb, p)) continue ;
        int32_t bij = Bx [p] ;
        Cx [p] = (x != bij) ;
    }
    return (GrB_SUCCESS) ;
    #endif
}



//------------------------------------------------------------------------------
// Cx = op (Ax,y):  apply a binary operator to a matrix with scalar bind2nd
//------------------------------------------------------------------------------



GrB_Info GB_bind2nd__isne_int32
(
    GB_void *Cx_output,         // Cx and Ax may be aliased
    const GB_void *Ax_input,
    const GB_void *y_input,
    const int8_t *GB_RESTRICT Ab,
    int64_t anz,
    int nthreads
)
{ 
    #if GB_DISABLE
    return (GrB_NO_VALUE) ;
    #else
    int64_t p ;
    int32_t *Cx = (int32_t *) Cx_output ;
    int32_t *Ax = (int32_t *) Ax_input ;
    int32_t   y = (*((int32_t *) y_input)) ;
    #pragma omp parallel for num_threads(nthreads) schedule(static)
    for (p = 0 ; p < anz ; p++)
    {
        if (!GBB (Ab, p)) continue ;
        int32_t aij = Ax [p] ;
        Cx [p] = (aij != y) ;
    }
    return (GrB_SUCCESS) ;
    #endif
}



//------------------------------------------------------------------------------
// C = op (x, A'): transpose and apply a binary operator
//------------------------------------------------------------------------------



// cij = op (x, aij), no typecasting (in spite of the macro name)
#undef  GB_CAST_OP
#define GB_CAST_OP(pC,pA)                       \
{                                               \
    int32_t aij = Ax [pA] ;                      \
    Cx [pC] = (x != aij) ;        \
}

GrB_Info GB_bind1st_tran__isne_int32
(
    GrB_Matrix C,
    const GB_void *x_input,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Workspaces,
    const int64_t *GB_RESTRICT A_slice,
    int nworkspaces,
    int nthreads
)
{ 
    // GB_unop_transpose.c uses GB_ATYPE, but A is
    // the 2nd input to binary operator z=f(x,y).
    #undef  GB_ATYPE
    #define GB_ATYPE \
    int32_t
    #if GB_DISABLE
    return (GrB_NO_VALUE) ;
    #else
    int32_t x = (*((const int32_t *) x_input)) ;
    #include "GB_unop_transpose.c"
    return (GrB_SUCCESS) ;
    #endif
    #undef  GB_ATYPE
    #define GB_ATYPE \
    int32_t
}



//------------------------------------------------------------------------------
// C = op (A', y): transpose and apply a binary operator
//------------------------------------------------------------------------------



// cij = op (aij, y), no typecasting (in spite of the macro name)
#undef  GB_CAST_OP
#define GB_CAST_OP(pC,pA)                       \
{                                               \
    int32_t aij = Ax [pA] ;                      \
    Cx [pC] = (aij != y) ;        \
}

GrB_Info GB_bind2nd_tran__isne_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    const GB_void *y_input,
    int64_t *GB_RESTRICT *Workspaces,
    const int64_t *GB_RESTRICT A_slice,
    int nworkspaces,
    int nthreads
)
{ 
    #if GB_DISABLE
    return (GrB_NO_VALUE) ;
    #else
    int32_t y = (*((const int32_t *) y_input)) ;
    #include "GB_unop_transpose.c"
    return (GrB_SUCCESS) ;
    #endif
}



#endif

