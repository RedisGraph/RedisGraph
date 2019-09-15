

//------------------------------------------------------------------------------
// GB_AxB:  hard-coded functions for semiring: C<M>=A*B or A'*B
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// If this file is in the Generated/ folder, do not edit it (auto-generated).

#include "GB.h"
#ifndef GBCOMPACT
#include "GB_control.h"
#include "GB_ek_slice.h"
#include "GB_Sauna.h"
#include "GB_jappend.h"
#include "GB_bracket.h"
#include "GB_iterator.h"
#include "GB_AxB__include.h"

// The C=A*B semiring is defined by the following types and operators:

// A*B function (Gustavon):  GB_AgusB__plus_plus_int64
// A'*B function (dot2):     GB_Adot2B__plus_plus_int64
// A'*B function (dot3):     GB_Adot3B__plus_plus_int64
// A*B function (heap):      GB_AheapB__plus_plus_int64

// C type:   int64_t
// A type:   int64_t
// B type:   int64_t

// Multiply: z = (aik + bkj)
// Add:      cij += x_op_y
// MultAdd:  int64_t x_op_y = (aik + bkj) ; cij += x_op_y
// Identity: 0
// Terminal: ;

#define GB_ATYPE \
    int64_t

#define GB_BTYPE \
    int64_t

#define GB_CTYPE \
    int64_t

// aik = Ax [pA]
#define GB_GETA(aik,Ax,pA) \
    int64_t aik = Ax [pA]

// bkj = Bx [pB]
#define GB_GETB(bkj,Bx,pB) \
    int64_t bkj = Bx [pB]

#define GB_CX(p) Cx [p]

// multiply operator
#define GB_MULT(z, x, y)        \
    z = (x + y) ;

// multiply-add
#define GB_MULTADD(z, x, y)     \
    int64_t x_op_y = (x + y) ; z += x_op_y ;

// copy scalar
#define GB_COPY_C(z,x) z = x ;

// monoid identity value (Gustavson's method only, with no mask)
#define GB_IDENTITY \
    0

// break if cij reaches the terminal value (dot product only)
#define GB_DOT_TERMINAL(cij) \
    ;

// simd pragma for dot product
#define GB_DOT_SIMD \
    GB_PRAGMA_SIMD

// cij is not a pointer but a scalar; nothing to do
#define GB_CIJ_REACQUIRE(cij,cnz) ;

// declare the cij scalar
#define GB_CIJ_DECLARE(cij) ; \
    int64_t cij ;

// save the value of C(i,j)
#define GB_CIJ_SAVE(cij,p) Cx [p] = cij ;

#define GB_SAUNA_WORK(i) Sauna_Work [i]

// disable this semiring and use the generic case if these conditions hold
#define GB_DISABLE \
    (GxB_NO_PLUS || GxB_NO_INT64 || GxB_NO_PLUS_INT64 || GxB_NO_PLUS_PLUS_INT64)

//------------------------------------------------------------------------------
// C<M>=A*B and C=A*B: gather/scatter saxpy-based method (Gustavson)
//------------------------------------------------------------------------------

GrB_Info GB_AgusB__plus_plus_int64
(
    GrB_Matrix C,
    const GrB_Matrix M,
    const GrB_Matrix A, bool A_is_pattern,
    const GrB_Matrix B, bool B_is_pattern,
    GB_Sauna Sauna
)
{ 
    #if GB_DISABLE
    return (GrB_NO_VALUE) ;
    #else
    int64_t *restrict Sauna_Work = Sauna->Sauna_Work ;
    int64_t *restrict Cx = C->x ;
    GrB_Info info = GrB_SUCCESS ;
    #include "GB_AxB_Gustavson_meta.c"
    return (info) ;
    #endif
}

//------------------------------------------------------------------------------
// C=A'*B or C<!M>=A'*B: dot product (phase 2)
//------------------------------------------------------------------------------

GrB_Info GB_Adot2B__plus_plus_int64
(
    GrB_Matrix C,
    const GrB_Matrix M,
    const GrB_Matrix *Aslice, bool A_is_pattern,
    const GrB_Matrix B, bool B_is_pattern,
    int64_t *restrict *C_counts,
    int nthreads, int naslice, int nbslice
)
{ 
    // C<M>=A'*B now uses dot3
    #if GB_DISABLE
    return (GrB_NO_VALUE) ;
    #else
    #define GB_PHASE_2_OF_2
    #include "GB_AxB_dot2_meta.c"
    #undef GB_PHASE_2_OF_2
    return (GrB_SUCCESS) ;
    #endif
}

//------------------------------------------------------------------------------
// C<M>=A'*B: masked dot product method (phase 2)
//------------------------------------------------------------------------------

GrB_Info GB_Adot3B__plus_plus_int64
(
    GrB_Matrix C,
    const GrB_Matrix M,
    const GrB_Matrix A, bool A_is_pattern,
    const GrB_Matrix B, bool B_is_pattern,
    const GB_task_struct *restrict TaskList,
    const int ntasks,
    const int nthreads
)
{ 
    #if GB_DISABLE
    return (GrB_NO_VALUE) ;
    #else
    #include "GB_AxB_dot3_template.c"
    return (GrB_SUCCESS) ;
    #endif
}

//------------------------------------------------------------------------------
// C<M>=A*B and C=A*B: heap saxpy-based method
//------------------------------------------------------------------------------

#include "GB_heap.h"

GrB_Info GB_AheapB__plus_plus_int64
(
    GrB_Matrix *Chandle,
    const GrB_Matrix M,
    const GrB_Matrix A, bool A_is_pattern,
    const GrB_Matrix B, bool B_is_pattern,
    int64_t *restrict List,
    GB_pointer_pair *restrict pA_pair,
    GB_Element *restrict Heap,
    const int64_t bjnz_max
)
{ 
    #if GB_DISABLE
    return (GrB_NO_VALUE) ;
    #else
    GrB_Matrix C = (*Chandle) ;
    int64_t *restrict Cx = C->x ;
    int64_t cij ;
    int64_t cvlen = C->vlen ;
    GrB_Info info = GrB_SUCCESS ;
    #include "GB_AxB_heap_meta.c"
    return (info) ;
    #endif
}

#endif

