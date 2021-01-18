//------------------------------------------------------------------------------
// GB_subassign.h: definitions for GB_subassign
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#ifndef GB_SUBASSIGN_H
#define GB_SUBASSIGN_H
#include "GB_ij.h"
#include "GB_add.h"

GrB_Info GB_subassign               // C(Rows,Cols)<M> += A or A'
(
    GrB_Matrix C_in,                // input/output matrix for results
    bool C_replace,                 // descriptor for C
    const GrB_Matrix M_in,          // optional mask for C(Rows,Cols)
    const bool Mask_comp,           // true if mask is complemented
    const bool Mask_struct,         // if true, use the only structure of M
    const bool M_transpose,         // true if the mask should be transposed
    const GrB_BinaryOp accum,       // optional accum for accum(C,T)
    const GrB_Matrix A_in,          // input matrix
    const bool A_transpose,         // true if A is transposed
    const GrB_Index *Rows,          // row indices
    const GrB_Index nRows_in,       // number of row indices
    const GrB_Index *Cols,          // column indices
    const GrB_Index nCols_in,       // number of column indices
    const bool scalar_expansion,    // if true, expand scalar to A
    const void *scalar,             // scalar to be expanded
    const GB_Type_code scalar_code, // type code of scalar to expand
    GB_Context Context
) ;

GrB_Info GB_subassign_scalar        // C(Rows,Cols)<M> += x
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Matrix M,             // mask for C(Rows,Cols), unused if NULL
    const GrB_BinaryOp accum,       // accum for Z=accum(C(Rows,Cols),T)
    const void *scalar,             // scalar to assign to C(Rows,Cols)
    const GB_Type_code scalar_code, // type code of scalar to assign
    const GrB_Index *Rows,          // row indices
    const GrB_Index nRows,          // number of row indices
    const GrB_Index *Cols,          // column indices
    const GrB_Index nCols,          // number of column indices
    const GrB_Descriptor desc,      // descriptor for C(Rows,Cols) and M
    GB_Context Context
) ;

int GB_subassigner_method           // return method to use in GB_subassigner
(
    // inputs: no matrix is modified
    const GrB_Matrix C,             // input/output matrix for results
    const bool C_replace,           // C matrix descriptor
    const GrB_Matrix M,             // optional mask for C(I,J), unused if NULL
    const bool Mask_comp,           // mask descriptor
    const bool Mask_struct,         // if true, use the only structure of M
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C(I,J),A)
    const GrB_Matrix A,             // input matrix (NULL for scalar expansion)
    const int Ikind,
    const int Jkind,
    const bool scalar_expansion     // if true, expand scalar to A
) ;

GrB_Info GB_subassigner             // C(I,J)<#M> = A or accum (C (I,J), A)
(
    // input/output
    GrB_Matrix C,                   // input/output matrix for results
    // input
    const int subassign_method,
    const bool C_replace,           // C matrix descriptor
    const GrB_Matrix M,             // optional mask for C(I,J), unused if NULL
    const bool Mask_comp,           // mask descriptor
    const bool Mask_struct,         // if true, use the only structure of M
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C(I,J),A)
    const GrB_Matrix A,             // input matrix (NULL for scalar expansion)
    const GrB_Index *I,             // list of indices
    const int64_t   ni,             // number of indices
    const int64_t nI,
    const int Ikind,
    const int64_t Icolon [3],
    const GrB_Index *J,             // list of vector indices
    const int64_t   nj,             // number of column indices
    const int64_t nJ,
    const int Jkind,
    const int64_t Jcolon [3],
    const bool scalar_expansion,    // if true, expand scalar to A
    const void *scalar,             // scalar to be expanded
    const GrB_Type atype,           // type code of scalar to expand
    GB_Context Context
) ;

GrB_Info GB_assign_prep
(
    // output:
    GrB_Matrix *Chandle,            // C_in, or C2 if C is aliased to M or A
    GrB_Matrix *Mhandle,            // M_in, or a modified version M2
    GrB_Matrix *Ahandle,            // A_in, or a modified version A2

    // modified versions of the matrices C, M, and A:
    GrB_Matrix *C2_handle,          // NULL, or a copy of C
    GrB_Matrix *M2_handle,          // NULL, or a temporary matrix
    GrB_Matrix *A2_handle,          // NULL, or a temporary matrix

    // modified versions of the Rows/Cols lists, and their analysis:
    const GrB_Index **I_handle,     // Rows, Cols, or a modified copy I2
    GrB_Index **I2_handle,          // NULL, or sorted/pruned Rows or Cols
    int64_t *ni_handle,
    int64_t *nI_handle,
    int *Ikind_handle,
    int64_t Icolon [3],

    const GrB_Index **J_handle,     // Rows, Cols, or a modified copy J2
    GrB_Index **J2_handle,          // NULL, or sorted/pruned Rows or Cols
    int64_t *nj_handle,
    int64_t *nJ_handle,
    int *Jkind_handle,
    int64_t Jcolon [3],

    bool *done,                     // true if the prep has finished all work
    GrB_Type *atype_handle,         // type of A or the scalar

    // input/output
    GrB_Matrix C_in,                // input/output matrix for results
    bool *C_replace,                // descriptor for C
    int *assign_kind,               // row assign, col assign, assign, or
                                    // subassign

    // input
    const GrB_Matrix M_in,          // optional mask for C
    const bool Mask_comp,           // true if mask is complemented
    const bool Mask_struct,         // if true, use the only structure of M
    bool M_transpose,               // true if the mask should be transposed
    const GrB_BinaryOp accum,       // optional accum for accum(C,T)
    const GrB_Matrix A_in,          // input matrix
    bool A_transpose,               // true if A is transposed
    const GrB_Index *Rows,          // row indices
    const GrB_Index nRows_in,       // number of row indices
    const GrB_Index *Cols,          // column indices
    const GrB_Index nCols_in,       // number of column indices
    const bool scalar_expansion,    // if true, expand scalar to A
    const void *scalar,             // scalar to be expanded
    const GB_Type_code scalar_code, // type code of scalar to expand
    GB_Context Context
) ;

#define GB_SUBASSIGN_METHOD_01   1     // C(I,J) = scalar
#define GB_SUBASSIGN_METHOD_02   2     // C(I,J) = A
#define GB_SUBASSIGN_METHOD_03   3     // C(I,J) += scalar
#define GB_SUBASSIGN_METHOD_04   4     // C(I,J) += A
#define GB_SUBASSIGN_METHOD_05   5     // C(I,J)<M> = scalar
#define GB_SUBASSIGN_METHOD_05d 51     // C(:,:)<M> = scalar ; C is dense
#define GB_SUBASSIGN_METHOD_05e 52     // C(:,:)<M,struct> = scalar
#define GB_SUBASSIGN_METHOD_06d 61     // C(:,:)<A> = A ; C is dense/bitmap
#define GB_SUBASSIGN_METHOD_06n 62     // C(I,J)<M> = A ; no S
#define GB_SUBASSIGN_METHOD_06s 63     // C(I,J)<M> = A ; using S
#define GB_SUBASSIGN_METHOD_07   7     // C(I,J)<M> += scalar
#define GB_SUBASSIGN_METHOD_08n 80     // C(I,J)<M> += A, no S
#define GB_SUBASSIGN_METHOD_08s 81     // C(I,J)<M> += A, with S
#define GB_SUBASSIGN_METHOD_09   9     // C(I,J)<M,replace> = scalar
#define GB_SUBASSIGN_METHOD_10  10     // C(I,J)<M,replace> = A
#define GB_SUBASSIGN_METHOD_11  11     // C(I,J)<M,replace> += scalar
#define GB_SUBASSIGN_METHOD_12  12     // C(I,J)<M,replace> += A
#define GB_SUBASSIGN_METHOD_13  13     // C(I,J)<!M> = scalar
#define GB_SUBASSIGN_METHOD_14  14     // C(I,J)<!M> = A
#define GB_SUBASSIGN_METHOD_15  15     // C(I,J)<!M> += scalar
#define GB_SUBASSIGN_METHOD_16  16     // C(I,J)<!M> += A
#define GB_SUBASSIGN_METHOD_17  17     // C(I,J)<!M,replace> = scalar
#define GB_SUBASSIGN_METHOD_18  18     // C(I,J)<!M,replace> = A
#define GB_SUBASSIGN_METHOD_19  19     // C(I,J)<!M,replace> = scalar
#define GB_SUBASSIGN_METHOD_20  20     // C(I,J)<!M,replace> += A
#define GB_SUBASSIGN_METHOD_21  21     // C(:,:) = scalar ; C becomes full
#define GB_SUBASSIGN_METHOD_22  22     // C += scalar ; C is dense
#define GB_SUBASSIGN_METHOD_23  23     // C += A ; C is dense
#define GB_SUBASSIGN_METHOD_24  24     // C = A
#define GB_SUBASSIGN_METHOD_25  25     // C(:,:)<M,struct> = A ; C empty
#define GB_SUBASSIGN_METHOD_BITMAP 999 // bitmap assignment

#endif

