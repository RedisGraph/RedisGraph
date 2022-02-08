//------------------------------------------------------------------------------
// GB_bitmap_assign_methods.h: definitions for GB_bitmap_assign* methods
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#ifndef GB_BITMAP_ASSIGN_METHODS_H
#define GB_BITMAP_ASSIGN_METHODS_H
#include "GB_bitmap_assign.h"
#include "GB_ek_slice.h"
#include "GB_partition.h"
#include "GB_ij.h"
#include "GB_subassign_IxJ_slice.h"
#include "GB_unused.h"

//------------------------------------------------------------------------------
// burble
//------------------------------------------------------------------------------

    #define GBURBLE_BITMAP_ASSIGN(method,M,Mask_comp,accum,Ikind,Jkind,akind) \
        GBURBLE ("Method:" method " ") ;                                    \
        GB_burble_assign (C_replace, Ikind, Jkind, M, Mask_comp,            \
            Mask_struct, accum, A, akind) ;

//------------------------------------------------------------------------------
// GB_GET_C_BITMAP: get the C matrix (must be bitmap)
//------------------------------------------------------------------------------

// C must be a bitmap matrix

#define GB_GET_C_BITMAP                                                     \
    GrB_Info info ;                                                         \
    /* also get the max # of threads to use */                              \
    GB_GET_NTHREADS_MAX (nthreads_max, chunk, Context) ;                    \
    ASSERT_MATRIX_OK (C, "C for bitmap assign", GB0) ;                      \
    ASSERT (GB_IS_BITMAP (C)) ;                                             \
    int8_t  *Cb = C->b ;                                                    \
    const bool C_iso = C->iso ;                                             \
    GB_void *Cx = (C_iso) ? NULL : (GB_void *) C->x ;                       \
    const size_t csize = C->type->size ;                                    \
    const GB_Type_code ccode = C->type->code ;                              \
    const int64_t cvdim = C->vdim ;                                         \
    const int64_t cvlen = C->vlen ;                                         \
    const int64_t vlen = cvlen ;    /* for GB_bitmap_assign_IxJ_template */ \
    const int64_t cnzmax = cvlen * cvdim ;                                  \
    int64_t cnvals = C->nvals ;

//------------------------------------------------------------------------------
// GB_GET_M: get the mask matrix M and check for aliasing
//------------------------------------------------------------------------------

// ALIAS of C and M for bitmap methods: For the assign methods, C==M is always
// permitted, since with arbitrary (I,J) indexing (I,J), the mask entry M(i,j)
// always controls C(i,j).  For row/col assign, aliasing of C and M would be
// unusual, since M is a single row or column.  But if C was also a single
// row/column, then C and M can be safely aliased.  For subassign, C==M can
// only occur if (I,J) are (:,:).

#define GB_GET_M                                                            \
    ASSERT_MATRIX_OK (M, "M for bitmap assign", GB0) ;                      \
    const int64_t *Mp = M->p ;                                              \
    const int8_t  *Mb = M->b ;                                              \
    const int64_t *Mh = M->h ;                                              \
    const int64_t *Mi = M->i ;                                              \
    const GB_void *Mx = (GB_void *) (Mask_struct ? NULL : (M->x)) ;         \
    const size_t msize = M->type->size ;                                    \
    const size_t mvlen = M->vlen ;

//------------------------------------------------------------------------------
// GB_SLICE_M: slice the mask matrix M
//------------------------------------------------------------------------------

#define GB_SLICE_M                                                          \
    GB_GET_M                                                                \
    GB_WERK_DECLARE (M_ek_slicing, int64_t) ;                               \
    int M_ntasks, M_nthreads ;                                              \
    GB_SLICE_MATRIX (M, 8, chunk) ;

//------------------------------------------------------------------------------
// GB_GET_A: get the A matrix or the scalar
//------------------------------------------------------------------------------

// ALIAS of C and A for bitmap methods: OK only for C(:,:)=A assignment.

#define GB_GET_A_AND_SCALAR                                                 \
    const int64_t *Ap = NULL ;                                              \
    const int64_t *Ah = NULL ;                                              \
    const int8_t  *Ab = NULL ;                                              \
    const int64_t *Ai = NULL ;                                              \
    const GB_void *Ax = NULL ;                                              \
    const bool A_iso = (A == NULL) ? false : A->iso ;                       \
    const size_t       asize = (A==NULL) ? scalar_type->size : A->type->size ; \
    const GB_Type_code acode = (A==NULL) ? scalar_type->code : A->type->code ; \
    if (A != NULL)                                                          \
    {                                                                       \
        ASSERT_MATRIX_OK (A, "A for bitmap assign/subassign", GB0) ;        \
        Ap = A->p ;                                                         \
        Ah = A->h ;                                                         \
        Ab = A->b ;                                                         \
        Ai = A->i ;                                                         \
        Ax = (C_iso) ? NULL : (GB_void *) A->x ;                            \
    }                                                                       \
    GB_cast_function cast_A_to_C = GB_cast_factory (ccode, acode) ;         \
    GB_void cwork [GB_VLA(csize)] ;                                         \
    if (A == NULL && !C_iso)                                                \
    {                                                                       \
        cast_A_to_C (cwork, scalar, asize) ;                                \
    }                                                                       \

//------------------------------------------------------------------------------
// GB_GET_ACCUM: get the accumulator op and its related typecasting functions
//------------------------------------------------------------------------------

#define GB_GET_ACCUM_FOR_BITMAP                                             \
    ASSERT_BINARYOP_OK (accum, "accum for bitmap assign", GB0) ;            \
    ASSERT (!GB_OP_IS_POSITIONAL (accum)) ;                                 \
    GxB_binary_function faccum = accum->binop_function ;                    \
    GB_cast_function cast_A_to_Y = GB_cast_factory (accum->ytype->code, acode);\
    GB_cast_function cast_C_to_X = GB_cast_factory (accum->xtype->code, ccode);\
    GB_cast_function cast_Z_to_C = GB_cast_factory (ccode, accum->ztype->code);\
    size_t xsize = accum->xtype->size ;                                     \
    size_t ysize = accum->ytype->size ;                                     \
    size_t zsize = accum->ztype->size ;                                     \
    GB_void ywork [GB_VLA(ysize)] ;                                         \
    if (A == NULL)                                                          \
    {                                                                       \
        cast_A_to_Y (ywork, scalar, asize) ;                                \
    }

//------------------------------------------------------------------------------
// GB_ASSIGN_SCALAR:  Cx [pC] = cwork, already typecasted
//------------------------------------------------------------------------------

#define GB_ASSIGN_SCALAR(pC)                                \
{                                                           \
    if (!C_iso)                                             \
    {                                                       \
        memcpy (Cx +(pC)*csize, cwork, csize) ;             \
    }                                                       \
}

//------------------------------------------------------------------------------
// GB_ASSIGN_AIJ:  Cx [pC] = Ax [pA], with typecasting as needed
//------------------------------------------------------------------------------

#define GB_ASSIGN_AIJ(pC,pA)                                \
{                                                           \
    if (!C_iso)                                             \
    {                                                       \
        cast_A_to_C (Cx +(pC)*csize, Ax +(A_iso ? 0:(pA)*asize), csize) ;   \
    }                                                       \
}

//------------------------------------------------------------------------------
// GB_ACCUM_SCALAR:  Cx [pC] += ywork
//------------------------------------------------------------------------------

#define GB_ACCUM_SCALAR(pC)                                     \
{                                                               \
    if (!C_iso)                                                 \
    {                                                           \
        GB_void xwork [GB_VLA(xsize)] ;                         \
        cast_C_to_X (xwork, Cx +((pC)*csize), csize) ;          \
        GB_void zwork [GB_VLA(zsize)] ;                         \
        faccum (zwork, xwork, ywork) ;                          \
        cast_Z_to_C (Cx +((pC)*csize), zwork, csize) ;          \
    }                                                           \
}

//------------------------------------------------------------------------------
// GB_ACCUM_AIJ:  Cx [pC] += Ax [pA]
//------------------------------------------------------------------------------

#define GB_ACCUM_AIJ(pC, pA)                                        \
{                                                                   \
    if (!C_iso)                                                     \
    {                                                               \
        /* ywork = Ax [pA], with typecasting as needed */           \
        GB_void ywork [GB_VLA(ysize)] ;                             \
        cast_A_to_Y (ywork, Ax +(A_iso ? 0:(pA)*asize), asize) ;    \
        /* Cx [pC] += ywork */                                      \
        GB_ACCUM_SCALAR (pC) ;                                      \
    }                                                               \
}

//------------------------------------------------------------------------------
// prototypes
//------------------------------------------------------------------------------

GrB_Info GB_bitmap_assign_fullM_accum
(
    // input/output:
    GrB_Matrix C,               // input/output matrix in bitmap format
    // inputs:
    const bool C_replace,       // descriptor for C
    const GrB_Index *I,         // I index list
    const int64_t nI,
    const int Ikind,
    const int64_t Icolon [3],
    const GrB_Index *J,         // J index list
    const int64_t nJ,
    const int Jkind,
    const int64_t Jcolon [3],
    const GrB_Matrix M,         // mask matrix, which is not NULL here
    const bool Mask_comp,       // true for !M, false for M
    const bool Mask_struct,     // true if M is structural, false if valued
    const GrB_BinaryOp accum,   // present here
    const GrB_Matrix A,         // input matrix, not transposed
    const void *scalar,         // input scalar
    const GrB_Type scalar_type, // type of input scalar
    const int assign_kind,      // row assign, col assign, assign, or subassign
    GB_Context Context
) ;

GrB_Info GB_bitmap_assign_fullM_accum_whole
(
    // input/output:
    GrB_Matrix C,               // input/output matrix in bitmap format
    // inputs:
    const bool C_replace,       // descriptor for C
    const GrB_Matrix M,         // mask matrix, which is present here
    const bool Mask_comp,       // true for !M, false for M
    const bool Mask_struct,     // true if M is structural, false if valued
    const GrB_BinaryOp accum,   // present here
    const GrB_Matrix A,         // input matrix, not transposed
    const void *scalar,         // input scalar
    const GrB_Type scalar_type, // type of input scalar
    GB_Context Context
) ;

GrB_Info GB_bitmap_assign_fullM_noaccum
(
    // input/output:
    GrB_Matrix C,               // input/output matrix in bitmap format
    const bool C_replace,       // descriptor for C
    // inputs:
    const GrB_Index *I,         // I index list
    const int64_t nI,
    const int Ikind,
    const int64_t Icolon [3],
    const GrB_Index *J,         // J index list
    const int64_t nJ,
    const int Jkind,
    const int64_t Jcolon [3],
    const GrB_Matrix M,         // mask matrix, which is present here
    const bool Mask_comp,       // true for !M, false for M
    const bool Mask_struct,     // true if M is structural, false if valued
//  const GrB_BinaryOp accum,   // not present
    const GrB_Matrix A,         // input matrix, not transposed
    const void *scalar,         // input scalar
    const GrB_Type scalar_type, // type of input scalar
    const int assign_kind,      // row assign, col assign, assign, or subassign
    GB_Context Context
) ;

GrB_Info GB_bitmap_assign_fullM_noaccum_whole
(
    // input/output:
    GrB_Matrix C,               // input/output matrix in bitmap format
    const bool C_replace,       // descriptor for C
    // inputs:
    const GrB_Matrix M,         // mask matrix, which is present here
    const bool Mask_comp,       // true for !M, false for M
    const bool Mask_struct,     // true if M is structural, false if valued
//  const GrB_BinaryOp accum,   // not present
    const GrB_Matrix A,         // input matrix, not transposed
    const void *scalar,         // input scalar
    const GrB_Type scalar_type, // type of input scalar
    GB_Context Context
) ;

GrB_Info GB_bitmap_assign_M_accum
(
    // input/output:
    GrB_Matrix C,               // input/output matrix in bitmap format
    // inputs:
    const bool C_replace,       // descriptor for C
    const GrB_Index *I,         // I index list
    const int64_t nI,
    const int Ikind,
    const int64_t Icolon [3],
    const GrB_Index *J,         // J index list
    const int64_t nJ,
    const int Jkind,
    const int64_t Jcolon [3],
    const GrB_Matrix M,         // mask matrix, which is not NULL here
//  const bool Mask_comp,       // false here
    const bool Mask_struct,     // true if M is structural, false if valued
    const GrB_BinaryOp accum,   // present here
    const GrB_Matrix A,         // input matrix, not transposed
    const void *scalar,         // input scalar
    const GrB_Type scalar_type, // type of input scalar
    const int assign_kind,      // row assign, col assign, assign, or subassign
    GB_Context Context
) ;

GrB_Info GB_bitmap_assign_M_accum_whole
(
    // input/output:
    GrB_Matrix C,               // input/output matrix in bitmap format
    // inputs:
    const bool C_replace,       // descriptor for C
    const GrB_Matrix M,         // mask matrix, which is not NULL here
//  const bool Mask_comp,       // false here
    const bool Mask_struct,     // true if M is structural, false if valued
    const GrB_BinaryOp accum,   // present here
    const GrB_Matrix A,         // input matrix, not transposed
    const void *scalar,         // input scalar
    const GrB_Type scalar_type, // type of input scalar
    GB_Context Context
) ;

GrB_Info GB_bitmap_assign_M_noaccum
(
    // input/output:
    GrB_Matrix C,               // input/output matrix in bitmap format
    // inputs:
    const bool C_replace,       // descriptor for C
    const GrB_Index *I,         // I index list
    const int64_t nI,
    const int Ikind,
    const int64_t Icolon [3],
    const GrB_Index *J,         // J index list
    const int64_t nJ,
    const int Jkind,
    const int64_t Jcolon [3],
    const GrB_Matrix M,         // mask matrix, which is not NULL here
//  const bool Mask_comp,       // false here
    const bool Mask_struct,     // true if M is structural, false if valued
//  const GrB_BinaryOp accum,   // not present
    const GrB_Matrix A,         // input matrix, not transposed
    const void *scalar,         // input scalar
    const GrB_Type scalar_type, // type of input scalar
    const int assign_kind,      // row assign, col assign, assign, or subassign
    GB_Context Context
) ;

GrB_Info GB_bitmap_assign_M_noaccum_whole
(
    // input/output:
    GrB_Matrix C,               // input/output matrix in bitmap format
    // inputs:
    const bool C_replace,       // descriptor for C
    const GrB_Matrix M,         // mask matrix, which is not NULL here
//  const bool Mask_comp,       // false here
    const bool Mask_struct,     // true if M is structural, false if valued
//  const GrB_BinaryOp accum,   // not present
    const GrB_Matrix A,         // input matrix, not transposed
    const void *scalar,         // input scalar
    const GrB_Type scalar_type, // type of input scalar
    GB_Context Context
) ;

GrB_Info GB_bitmap_assign_noM_accum
(
    // input/output:
    GrB_Matrix C,               // input/output matrix in bitmap format
    // inputs:
    const bool C_replace,       // descriptor for C
    const GrB_Index *I,         // I index list
    const int64_t nI,
    const int Ikind,
    const int64_t Icolon [3],
    const GrB_Index *J,         // J index list
    const int64_t nJ,
    const int Jkind,
    const int64_t Jcolon [3],
//  const GrB_Matrix M,         // mask matrix, not present here
    const bool Mask_comp,       // true for !M, false for M
    const bool Mask_struct,     // true if M is structural, false if valued
    const GrB_BinaryOp accum,   // present
    const GrB_Matrix A,         // input matrix, not transposed
    const void *scalar,         // input scalar
    const GrB_Type scalar_type, // type of input scalar
    const int assign_kind,      // row assign, col assign, assign, or subassign
    GB_Context Context
) ;

GrB_Info GB_bitmap_assign_noM_accum_whole
(
    // input/output:
    GrB_Matrix C,               // input/output matrix in bitmap format
    // inputs:
    const bool C_replace,       // descriptor for C
//  const GrB_Matrix M,         // mask matrix, not present here
    const bool Mask_comp,       // true for !M, false for M
    const bool Mask_struct,     // true if M is structural, false if valued
    const GrB_BinaryOp accum,   // present
    const GrB_Matrix A,         // input matrix, not transposed
    const void *scalar,         // input scalar
    const GrB_Type scalar_type, // type of input scalar
    GB_Context Context
) ;

GB_PUBLIC
GrB_Info GB_bitmap_assign_noM_noaccum
(
    // input/output:
    GrB_Matrix C,               // input/output matrix in bitmap format
    // inputs:
    const bool C_replace,       // descriptor for C
    const GrB_Index *I,         // I index list
    const int64_t nI,
    const int Ikind,
    const int64_t Icolon [3],
    const GrB_Index *J,         // J index list
    const int64_t nJ,
    const int Jkind,
    const int64_t Jcolon [3],
//  const GrB_Matrix M,         // mask matrix, not present here
    const bool Mask_comp,       // true for !M, false for M
    const bool Mask_struct,     // true if M is structural, false if valued
//  const GrB_BinaryOp accum,   // not present
    const GrB_Matrix A,         // input matrix, not transposed
    const void *scalar,         // input scalar
    const GrB_Type scalar_type, // type of input scalar
    const int assign_kind,      // row assign, col assign, assign, or subassign
    GB_Context Context
) ;

GrB_Info GB_bitmap_assign_noM_noaccum_whole
(
    // input/output:
    GrB_Matrix C,               // input/output matrix in bitmap format
    // inputs:
    const bool C_replace,       // descriptor for C
//  const GrB_Matrix M,         // mask matrix, not present here
    const bool Mask_comp,       // true for !M, false for M
    const bool Mask_struct,     // true if M is structural, false if valued
//  const GrB_BinaryOp accum,   // not present
    const GrB_Matrix A,         // input matrix, not transposed
    const void *scalar,         // input scalar
    const GrB_Type scalar_type, // type of input scalar
    GB_Context Context
) ;

GrB_Info GB_bitmap_assign_notM_accum
(
    // input/output:
    GrB_Matrix C,               // input/output matrix in bitmap format
    // inputs:
    const bool C_replace,       // descriptor for C
    const GrB_Index *I,         // I index list
    const int64_t nI,
    const int Ikind,
    const int64_t Icolon [3],
    const GrB_Index *J,         // J index list
    const int64_t nJ,
    const int Jkind,
    const int64_t Jcolon [3],
    const GrB_Matrix M,         // mask matrix
//  const bool Mask_comp,       // true here, for !M only
    const bool Mask_struct,     // true if M is structural, false if valued
    const GrB_BinaryOp accum,   // present
    const GrB_Matrix A,         // input matrix, not transposed
    const void *scalar,         // input scalar
    const GrB_Type scalar_type, // type of input scalar
    const int assign_kind,      // row assign, col assign, assign, or subassign
    GB_Context Context
) ;

GrB_Info GB_bitmap_assign_notM_accum_whole
(
    // input/output:
    GrB_Matrix C,               // input/output matrix in bitmap format
    // inputs:
    const bool C_replace,       // descriptor for C
    const GrB_Matrix M,         // mask matrix
//  const bool Mask_comp,       // true here, for !M only
    const bool Mask_struct,     // true if M is structural, false if valued
    const GrB_BinaryOp accum,   // present
    const GrB_Matrix A,         // input matrix, not transposed
    const void *scalar,         // input scalar
    const GrB_Type scalar_type, // type of input scalar
    GB_Context Context
) ;

GrB_Info GB_bitmap_assign_notM_noaccum
(
    // input/output:
    GrB_Matrix C,               // input/output matrix in bitmap format
    // inputs:
    const bool C_replace,       // descriptor for C
    const GrB_Index *I,         // I index list
    const int64_t nI,
    const int Ikind,
    const int64_t Icolon [3],
    const GrB_Index *J,         // J index list
    const int64_t nJ,
    const int Jkind,
    const int64_t Jcolon [3],
    const GrB_Matrix M,         // mask matrix
//  const bool Mask_comp,       // true here, for !M only
    const bool Mask_struct,     // true if M is structural, false if valued
//  const GrB_BinaryOp accum,   // not present
    const GrB_Matrix A,         // input matrix, not transposed
    const void *scalar,         // input scalar
    const GrB_Type scalar_type, // type of input scalar
    const int assign_kind,      // row assign, col assign, assign, or subassign
    GB_Context Context
) ;

GrB_Info GB_bitmap_assign_notM_noaccum_whole
(
    // input/output:
    GrB_Matrix C,               // input/output matrix in bitmap format
    // inputs:
    const bool C_replace,       // descriptor for C
    const GrB_Matrix M,         // mask matrix
//  const bool Mask_comp,       // true here, for !M only
    const bool Mask_struct,     // true if M is structural, false if valued
//  const GrB_BinaryOp accum,   // not present
    const GrB_Matrix A,         // input matrix, not transposed
    const void *scalar,         // input scalar
    const GrB_Type scalar_type, // type of input scalar
    GB_Context Context
) ;

#define GB_BITMAP_M_SCATTER_PLUS_2  0
#define GB_BITMAP_M_SCATTER_MINUS_2 1
#define GB_BITMAP_M_SCATTER_SET_2   2
#define GB_BITMAP_M_SCATTER_MOD_2   3

void GB_bitmap_M_scatter        // scatter M into the C bitmap
(
    // input/output:
    GrB_Matrix C,
    // inputs:
    const GrB_Index *I,         // I index list
    const int64_t nI,
    const int Ikind,
    const int64_t Icolon [3],
    const GrB_Index *J,         // J index list
    const int64_t nJ,
    const int Jkind,
    const int64_t Jcolon [3],
    const GrB_Matrix M,         // mask to scatter into the C bitmap
    const bool Mask_struct,     // true if M is structural, false if valued
    const int assign_kind,      // row assign, col assign, assign, or subassign
    const int operation,        // +=2, -=2, or %=2
    const int64_t *M_ek_slicing,    // size M_ntasks+1
    const int M_ntasks,
    const int M_nthreads,
    GB_Context Context
) ;

void GB_bitmap_M_scatter_whole  // scatter M into the C bitmap
(
    // input/output:
    GrB_Matrix C,
    // inputs:
    const GrB_Matrix M,         // mask to scatter into the C bitmap
    const bool Mask_struct,     // true if M is structural, false if valued
    const int operation,        // +=2, -=2, or %=2
    const int64_t *M_ek_slicing,    // size M_ntasks+1
    const int M_ntasks,
    const int M_nthreads,
    GB_Context Context
) ;

void GB_bitmap_assign_to_full   // set all C->b to 1, or free it and make C full
(
    GrB_Matrix C,
    int nthreads_max
) ;

#endif

