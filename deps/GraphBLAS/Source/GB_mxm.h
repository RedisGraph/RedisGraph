//------------------------------------------------------------------------------
// GB_mxm.h: definitions for C=A*B
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#ifndef GB_MXM_H
#define GB_MXM_H
#include "GB_AxB_saxpy.h"

//------------------------------------------------------------------------------

GrB_Info GB_mxm                     // C<M> = A*B
(
    GrB_Matrix C,                   // input/output matrix for results
    const bool C_replace,           // if true, clear C before writing to it
    const GrB_Matrix M,             // optional mask for C, unused if NULL
    const bool Mask_comp,           // if true, use !M
    const bool Mask_struct,         // if true, use the only structure of M
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C,T)
    const GrB_Semiring semiring,    // defines '+' and '*' for C=A*B
    const GrB_Matrix A,             // input matrix
    const bool A_transpose,         // if true, use A' instead of A
    const GrB_Matrix B,             // input matrix
    const bool B_transpose,         // if true, use B' instead of B
    const bool flipxy,              // if true, do z=fmult(b,a) vs fmult(a,b)
    const GrB_Desc_Value AxB_method,// for auto vs user selection of methods
    const int do_sort,              // if nonzero, try to return C unjumbled
    GB_Context Context
) ;

GrB_Info GB_AxB_dot                 // dot product (multiple methods)
(
    GrB_Matrix *Chandle,            // output matrix, NULL on input
    GrB_Matrix C_in_place,          // input/output matrix, if done in-place
    GrB_Matrix M,                   // optional mask matrix
    const bool Mask_comp,           // if true, use !M
    const bool Mask_struct,         // if true, use the only structure of M
    const GrB_Matrix A,             // input matrix A
    const GrB_Matrix B,             // input matrix B
    const GrB_Semiring semiring,    // semiring that defines C=A*B
    const bool flipxy,              // if true, do z=fmult(b,a) vs fmult(a,b)
    bool *mask_applied,             // if true, mask was applied
    bool *done_in_place,            // if true, C_in_place was computed in-place
    GB_Context Context
) ;

GB_PUBLIC   // accessed by the MATLAB tests in GraphBLAS/Test only
GrB_Info GB_AxB_meta                // C<M>=A*B meta algorithm
(
    GrB_Matrix *Chandle,            // output matrix (if not done in-place)
    GrB_Matrix C_in_place,          // input/output matrix, if done in-place
    bool C_replace,                 // C matrix descriptor
    const bool C_is_csc,            // desired CSR/CSC format of C
    GrB_Matrix *MT_handle,          // return MT = M' to caller, if computed
    const GrB_Matrix M_in,          // mask for C<M> (not complemented)
    const bool Mask_comp,           // if true, use !M
    const bool Mask_struct,         // if true, use the only structure of M
    const GrB_BinaryOp accum,       // accum operator for C_input += A*B
    const GrB_Matrix A_in,          // input matrix
    const GrB_Matrix B_in,          // input matrix
    const GrB_Semiring semiring,    // semiring that defines C=A*B
    bool A_transpose,               // if true, use A', else A
    bool B_transpose,               // if true, use B', else B
    bool flipxy,                    // if true, do z=fmult(b,a) vs fmult(a,b)
    bool *mask_applied,             // if true, mask was applied
    bool *done_in_place,            // if true, C was computed in-place
    GrB_Desc_Value AxB_method,      // for auto vs user selection of methods
    const int do_sort,              // if nonzero, try to return C unjumbled
    GB_Context Context
) ;

GrB_Info GB_AxB_rowscale            // C = D*B, row scale with diagonal D
(
    GrB_Matrix *Chandle,            // output matrix
    const GrB_Matrix D,             // diagonal input matrix
    const GrB_Matrix B,             // input matrix
    const GrB_Semiring semiring,    // semiring that defines C=D*A
    const bool flipxy,              // if true, do z=fmult(b,a) vs fmult(a,b)
    GB_Context Context
) ;

GrB_Info GB_AxB_colscale            // C = A*D, column scale with diagonal D
(
    GrB_Matrix *Chandle,            // output matrix
    const GrB_Matrix A,             // input matrix
    const GrB_Matrix D,             // diagonal input matrix
    const GrB_Semiring semiring,    // semiring that defines C=A*D
    const bool flipxy,              // if true, do z=fmult(b,a) vs fmult(a,b)
    GB_Context Context
) ;


bool GB_AxB_semiring_builtin        // true if semiring is builtin
(
    // inputs:
    const GrB_Matrix A,
    const bool A_is_pattern,        // true if only the pattern of A is used
    const GrB_Matrix B,
    const bool B_is_pattern,        // true if only the pattern of B is used
    const GrB_Semiring semiring,    // semiring that defines C=A*B
    const bool flipxy,              // true if z=fmult(y,x), flipping x and y
    // outputs, unused by caller if this function returns false
    GB_Opcode *mult_opcode,         // multiply opcode
    GB_Opcode *add_opcode,          // add opcode
    GB_Type_code *xcode,            // type code for x input
    GB_Type_code *ycode,            // type code for y input
    GB_Type_code *zcode             // type code for z output
) ;

GB_PUBLIC   // accessed by the MATLAB tests in GraphBLAS/Test only
GrB_Info GB_AxB_dot2                // C=A'*B or C<!M>=A'*B, dot product method
(
    GrB_Matrix *Chandle,            // output matrix
    const GrB_Matrix M,             // mask matrix for C<!M>=A'*B
    const bool Mask_comp,           // if true, use !M
    const bool Mask_struct,         // if true, use the only structure of M
    const GrB_Matrix A,             // input matrix
    const GrB_Matrix B,             // input matrix
    const GrB_Semiring semiring,    // semiring that defines C=A*B
    const bool flipxy,              // if true, do z=fmult(b,a) vs fmult(a,b)
    GB_Context Context
) ;

bool GB_is_diagonal             // true if A is diagonal
(
    const GrB_Matrix A,         // input matrix to examine
    GB_Context Context
) ;

GB_PUBLIC   // accessed by the MATLAB tests in GraphBLAS/Test only
GrB_Info GB_AxB_dot3                // C<M> = A'*B using dot product method
(
    GrB_Matrix *Chandle,            // output matrix
    const GrB_Matrix M,             // mask matrix for C<M>=A'*B or C<!M>=A'*B
    const bool Mask_struct,         // if true, use the only structure of M
    const GrB_Matrix A,             // input matrix
    const GrB_Matrix B,             // input matrix
    const GrB_Semiring semiring,    // semiring that defines C=A*B
    const bool flipxy,              // if true, do z=fmult(b,a) vs fmult(a,b)
    GB_Context Context
) ;

GrB_Info GB_AxB_dot3_slice
(
    // output:
    GB_task_struct **p_TaskList,    // array of structs, of size max_ntasks
    int *p_max_ntasks,              // size of TaskList
    int *p_ntasks,                  // # of tasks constructed
    int *p_nthreads,                // # of threads to use
    // input:
    const GrB_Matrix C,             // matrix to slice
    GB_Context Context
) ;

GrB_Info GB_AxB_dot3_one_slice
(
    // output:
    GB_task_struct **p_TaskList,    // array of structs, of size max_ntasks
    int *p_max_ntasks,              // size of TaskList
    int *p_ntasks,                  // # of tasks constructed
    int *p_nthreads,                // # of threads to use
    // input:
    const GrB_Matrix M,             // matrix to slice
    GB_Context Context
) ;

GrB_Info GB_AxB_dot4                // C+=A'*B, dot product method
(
    GrB_Matrix C,                   // input/output matrix, must be dense
    const GrB_Matrix A,             // input matrix
    const GrB_Matrix B,             // input matrix
    const GrB_Semiring semiring,    // semiring that defines C+=A*B
    const bool flipxy,              // if true, do z=fmult(b,a) vs fmult(a,b)
    GB_Context Context
) ;

void GB_AxB_pattern
(
    // outputs:
    bool *A_is_pattern,     // true if A is pattern-only, because of the mult
    bool *B_is_pattern,     // true if B is pattern-only, because of the mult
    // inputs:
    const bool flipxy,      // if true,  z = mult (b,a) will be computed
                            // if false, z = mult (a,b) will be computed
    const GB_Opcode mult_opcode // opcode of multiply operator
) ;

//------------------------------------------------------------------------------
// GB_AxB_dot4_control: determine if the dot4 method should be used
//------------------------------------------------------------------------------

// C += A'*B where C is modified in-place

static inline bool GB_AxB_dot4_control
(
    const GrB_Matrix C_in,      // NULL if C cannot be modified in-place
    const GrB_Matrix M,
    const bool Mask_comp
)
{
    return (C_in != NULL && M == NULL && !Mask_comp && !GB_IS_BITMAP (C_in)) ;
}

//------------------------------------------------------------------------------
// GB_AxB_dot3_control: determine if the dot3 method should be used
//------------------------------------------------------------------------------

// C<M>=A'*B where M is sparse or hypersparse, and not complemented

static inline bool GB_AxB_dot3_control
(
    const GrB_Matrix M,
    const bool Mask_comp
)
{
    return (M != NULL && !Mask_comp &&
        (GB_IS_SPARSE (M) || GB_IS_HYPERSPARSE (M))) ;
}

//------------------------------------------------------------------------------
// GB_AxB_dot2_control: determine if the dot2 method should be used
//------------------------------------------------------------------------------

// C=A'*B, C<M>=A'*B, or C<!M>=A'*B where C is constructed in bitmap format.
// C must be small and likely very dense.

bool GB_AxB_dot2_control  // true: use dot2, false: use saxpy
(
    const GrB_Matrix A,
    const GrB_Matrix B,
    GB_Context Context
) ;

#endif

