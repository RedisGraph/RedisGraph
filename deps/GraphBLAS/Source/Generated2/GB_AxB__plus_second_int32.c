//------------------------------------------------------------------------------
// GB_AxB__plus_second_int32.c: matrix multiply for a single semiring
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// If this file is in the Generated1/ or Generated2/ folder, do not edit it
// (it is auto-generated from Generator/*).

#include "GB_dev.h"

#ifndef GBCOMPACT

#include "GB.h"
#include "GB_control.h"
#include "GB_bracket.h"
#include "GB_sort.h"
#include "GB_atomics.h"
#include "GB_AxB_saxpy.h"
#if 1
#include "GB_AxB__include2.h"
#else
#include "GB_AxB__include1.h"
#endif
#include "GB_unused.h"
#include "GB_bitmap_assign_methods.h"
#include "GB_ek_slice_search.c"

// This C=A*B semiring is defined by the following types and operators:

// A'*B (dot2):        GB (_Adot2B__plus_second_int32)
// A'*B (dot3):        GB (_Adot3B__plus_second_int32)
// C+=A'*B (dot4):     GB (_Adot4B__plus_second_int32)
// A*B (saxpy bitmap): GB (_AsaxbitB__plus_second_int32)
// A*B (saxpy3):       GB (_Asaxpy3B__plus_second_int32)
//     no mask:        GB (_Asaxpy3B_noM__plus_second_int32)
//     mask M:         GB (_Asaxpy3B_M__plus_second_int32)
//     mask !M:        GB (_Asaxpy3B_notM__plus_second_int32)

// C type:   int32_t
// A type:   int32_t
// B type:   int32_t

// Multiply: z = bkj
// Add:      cij += z
//           'any' monoid?  0
//           atomic?        1
//           OpenMP atomic? 1
// MultAdd:  cij += bkj
// Identity: 0
// Terminal: ;

#define GB_ATYPE \
    int32_t

#define GB_BTYPE \
    int32_t

#define GB_CTYPE \
    int32_t

#define GB_ASIZE \
    sizeof (int32_t)

#define GB_BSIZE \
    sizeof (int32_t) 

#define GB_CSIZE \
    sizeof (int32_t)

// true for int64, uint64, float, double, float complex, and double complex 
#define GB_CTYPE_IGNORE_OVERFLOW \
    0

// aik = Ax [pA]
#define GB_GETA(aik,Ax,pA,A_iso) \
    ;

// bkj = Bx [pB]
#define GB_GETB(bkj,Bx,pB,B_iso) \
    int32_t bkj = GBX (Bx, pB, B_iso)

// Gx [pG] = Ax [pA]
#define GB_LOADA(Gx,pG,Ax,pA,A_iso) \
    Gx [pG] = GBX (Ax, pA, A_iso)

// Gx [pG] = Bx [pB]
#define GB_LOADB(Gx,pG,Bx,pB,B_iso) \
    Gx [pG] = GBX (Bx, pB, B_iso)

#define GB_CX(p) \
    Cx [p]

// multiply operator
#define GB_MULT(z, x, y, i, k, j) \
    z = y

// cast from a real scalar (or 2, if C is complex) to the type of C
#define GB_CTYPE_CAST(x,y) \
    ((int32_t) x)

// cast from a real scalar (or 2, if A is complex) to the type of A
#define GB_ATYPE_CAST(x,y) \
    ((int32_t) x)

// multiply-add
#define GB_MULTADD(z, x, y, i, k, j) \
    z += y

// monoid identity value
#define GB_IDENTITY \
    0

// 1 if the identity value can be assigned via memset, with all bytes the same
#define GB_HAS_IDENTITY_BYTE \
    1

// identity byte, for memset
#define GB_IDENTITY_BYTE \
    0

// break if cij reaches the terminal value (dot product only)
#define GB_DOT_TERMINAL(cij) \
    ;

// simd pragma for dot-product loop vectorization
#define GB_PRAGMA_SIMD_DOT(cij) \
    GB_PRAGMA_SIMD_REDUCTION (+,cij)

// simd pragma for other loop vectorization
#define GB_PRAGMA_SIMD_VECTORIZE GB_PRAGMA_SIMD

// 1 for the PLUS_PAIR_(real) semirings, not for the complex case
#define GB_IS_PLUS_PAIR_REAL_SEMIRING \
    0

// declare the cij scalar (initialize cij to zero for PLUS_PAIR)
#define GB_CIJ_DECLARE(cij) \
    int32_t cij

// cij = Cx [pC] for dot4 method only
#define GB_GET4C(cij,p) \
    cij = (C_in_iso) ? cinput : Cx [p]

// Cx [pC] = cij
#define GB_PUTC(cij,p) \
    Cx [p] = cij

// Cx [p] = t
#define GB_CIJ_WRITE(p,t) \
    Cx [p] = t

// C(i,j) += t
#define GB_CIJ_UPDATE(p,t) \
    Cx [p] += t

// x + y
#define GB_ADD_FUNCTION(x,y) \
    x + y

// bit pattern for bool, 8-bit, 16-bit, and 32-bit integers
#define GB_CTYPE_BITS \
    0xffffffffL

// 1 if monoid update can skipped entirely (the ANY monoid)
#define GB_IS_ANY_MONOID \
    0

// 1 if monoid update is EQ
#define GB_IS_EQ_MONOID \
    0

// 1 if monoid update can be done atomically, 0 otherwise
#define GB_HAS_ATOMIC \
    1

// 1 if monoid update can be done with an OpenMP atomic update, 0 otherwise
#if GB_MICROSOFT
    #define GB_HAS_OMP_ATOMIC \
        1
#else
    #define GB_HAS_OMP_ATOMIC \
        1
#endif

// 1 for the ANY_PAIR_ISO semiring
#define GB_IS_ANY_PAIR_SEMIRING \
    0

// 1 if PAIR is the multiply operator 
#define GB_IS_PAIR_MULTIPLIER \
    0

// 1 if monoid is PLUS_FC32
#define GB_IS_PLUS_FC32_MONOID \
    0

// 1 if monoid is PLUS_FC64
#define GB_IS_PLUS_FC64_MONOID \
    0

// 1 if monoid is ANY_FC32
#define GB_IS_ANY_FC32_MONOID \
    0

// 1 if monoid is ANY_FC64
#define GB_IS_ANY_FC64_MONOID \
    0

// 1 if monoid is MIN for signed or unsigned integers
#define GB_IS_IMIN_MONOID \
    0

// 1 if monoid is MAX for signed or unsigned integers
#define GB_IS_IMAX_MONOID \
    0

// 1 if monoid is MIN for float or double
#define GB_IS_FMIN_MONOID \
    0

// 1 if monoid is MAX for float or double
#define GB_IS_FMAX_MONOID \
    0

// 1 for the FIRSTI or FIRSTI1 multiply operator
#define GB_IS_FIRSTI_MULTIPLIER \
    0

// 1 for the FIRSTJ or FIRSTJ1 multiply operator
#define GB_IS_FIRSTJ_MULTIPLIER \
    0

// 1 for the SECONDJ or SECONDJ1 multiply operator
#define GB_IS_SECONDJ_MULTIPLIER \
    0

// atomic compare-exchange
#define GB_ATOMIC_COMPARE_EXCHANGE(target, expected, desired) \
    GB_ATOMIC_COMPARE_EXCHANGE_32 (target, expected, desired)

// Hx [i] = t
#define GB_HX_WRITE(i,t) \
    Hx [i] = t

// Cx [p] = Hx [i]
#define GB_CIJ_GATHER(p,i) \
    Cx [p] = Hx [i]

// Cx [p] += Hx [i]
#define GB_CIJ_GATHER_UPDATE(p,i) \
    Cx [p] += Hx [i]

// Hx [i] += t
#define GB_HX_UPDATE(i,t) \
    Hx [i] += t

// memcpy (&(Cx [p]), &(Hx [i]), len)
#define GB_CIJ_MEMCPY(p,i,len) \
    memcpy (Cx +(p), Hx +(i), (len) * sizeof(int32_t));

// 1 if the semiring has a concise bitmap multiply-add
#define GB_HAS_BITMAP_MULTADD \
    1

// concise statement(s) for the bitmap case:
//  if (exists)
//      if (cb == 0)
//          cx = ax * bx
//          cb = 1
//      else
//          cx += ax * bx
#define GB_BITMAP_MULTADD(cb,cx,exists,ax,bx) \
    cx += (bx) * X [exists] ; cb |= exists

// define X for bitmap multiply-add
#define GB_XINIT \
    int32_t X [2] = {0,1}

// load X [1] = bkj for bitmap multiply-add
#define GB_XLOAD(bkj) \
    ;

// disable this semiring and use the generic case if these conditions hold
#define GB_DISABLE \
    (GxB_NO_PLUS || GxB_NO_SECOND || GxB_NO_INT32 || GxB_NO_PLUS_INT32 || GxB_NO_SECOND_INT32 || GxB_NO_PLUS_SECOND_INT32)

//------------------------------------------------------------------------------
// GB_Adot2B: C=A'*B, C<M>=A'*B, or C<!M>=A'*B: dot product method, C is bitmap
//------------------------------------------------------------------------------

GrB_Info GB (_Adot2B__plus_second_int32)
(
    GrB_Matrix C,
    const GrB_Matrix M, const bool Mask_comp, const bool Mask_struct,
    const GrB_Matrix A, bool A_is_pattern, int64_t *restrict A_slice,
    const GrB_Matrix B, bool B_is_pattern, int64_t *restrict B_slice,
    int nthreads, int naslice, int nbslice
)
{ 
    #if GB_DISABLE
    return (GrB_NO_VALUE) ;
    #else
    #include "GB_AxB_dot2_meta.c"
    return (GrB_SUCCESS) ;
    #endif
}

//------------------------------------------------------------------------------
// GB_Adot3B: C<M>=A'*B: masked dot product, C is sparse or hyper
//------------------------------------------------------------------------------

GrB_Info GB (_Adot3B__plus_second_int32)
(
    GrB_Matrix C,
    const GrB_Matrix M, const bool Mask_struct,
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
    #include "GB_AxB_dot3_meta.c"
    return (GrB_SUCCESS) ;
    #endif
}

//------------------------------------------------------------------------------
// GB_Adot4B:  C+=A'*B: dense dot product (not used for ANY_PAIR_ISO)
//------------------------------------------------------------------------------

#if 1

    GrB_Info GB (_Adot4B__plus_second_int32)
    (
        GrB_Matrix C, const bool C_in_iso, const GB_void *cinput_void,
        const GrB_Matrix A, bool A_is_pattern,
        int64_t *restrict A_slice, int naslice,
        const GrB_Matrix B, bool B_is_pattern,
        int64_t *restrict B_slice, int nbslice,
        const int nthreads
    )
    { 
        #if GB_DISABLE
        return (GrB_NO_VALUE) ;
        #else
        const int32_t cinput = (*((const int32_t *) cinput_void)) ;
        #include "GB_AxB_dot4_meta.c"
        return (GrB_SUCCESS) ;
        #endif
    }

#endif

//------------------------------------------------------------------------------
// GB_AsaxbitB: C=A*B, C<M>=A*B, C<!M>=A*B: saxpy method, C is bitmap/full
//------------------------------------------------------------------------------

#include "GB_AxB_saxpy3_template.h"

GrB_Info GB (_AsaxbitB__plus_second_int32)
(
    GrB_Matrix C,   // bitmap or full
    const GrB_Matrix M, const bool Mask_comp, const bool Mask_struct,
    const GrB_Matrix A, bool A_is_pattern,
    const GrB_Matrix B, bool B_is_pattern,
    GB_Context Context
)
{ 
    #if GB_DISABLE
    return (GrB_NO_VALUE) ;
    #else
    #include "GB_bitmap_AxB_saxpy_template.c"
    return (GrB_SUCCESS) ;
    #endif
}

//------------------------------------------------------------------------------
// GB_Asaxpy3B: C=A*B, C<M>=A*B, C<!M>=A*B: saxpy method (Gustavson + Hash)
//------------------------------------------------------------------------------

GrB_Info GB (_Asaxpy3B__plus_second_int32)
(
    GrB_Matrix C,   // C<any M>=A*B, C sparse or hypersparse
    const GrB_Matrix M, const bool Mask_comp, const bool Mask_struct,
    const bool M_in_place,
    const GrB_Matrix A, bool A_is_pattern,
    const GrB_Matrix B, bool B_is_pattern,
    GB_saxpy3task_struct *restrict SaxpyTasks,
    const int ntasks, const int nfine, const int nthreads, const int do_sort,
    GB_Context Context
)
{ 
    #if GB_DISABLE
    return (GrB_NO_VALUE) ;
    #else
    ASSERT (GB_IS_SPARSE (C) || GB_IS_HYPERSPARSE (C)) ;
    if (M == NULL)
    {
        // C = A*B, no mask
        return (GB (_Asaxpy3B_noM__plus_second_int32) (C,
            A, A_is_pattern, B, B_is_pattern,
            SaxpyTasks, ntasks, nfine, nthreads, do_sort, Context)) ;
    }
    else if (!Mask_comp)
    {
        // C<M> = A*B
        return (GB (_Asaxpy3B_M__plus_second_int32) (C,
            M, Mask_struct, M_in_place,
            A, A_is_pattern, B, B_is_pattern,
            SaxpyTasks, ntasks, nfine, nthreads, do_sort, Context)) ;
    }
    else
    {
        // C<!M> = A*B
        return (GB (_Asaxpy3B_notM__plus_second_int32) (C,
            M, Mask_struct, M_in_place,
            A, A_is_pattern, B, B_is_pattern,
            SaxpyTasks, ntasks, nfine, nthreads, do_sort, Context)) ;
    }
    #endif
}

//------------------------------------------------------------------------------
// GB_Asaxpy3B_M: C<M>=A*Bi: saxpy method (Gustavson + Hash)
//------------------------------------------------------------------------------

#if ( !GB_DISABLE )

    GrB_Info GB (_Asaxpy3B_M__plus_second_int32)
    (
        GrB_Matrix C,   // C<M>=A*B, C sparse or hypersparse
        const GrB_Matrix M, const bool Mask_struct,
        const bool M_in_place,
        const GrB_Matrix A, bool A_is_pattern,
        const GrB_Matrix B, bool B_is_pattern,
        GB_saxpy3task_struct *restrict SaxpyTasks,
        const int ntasks, const int nfine, const int nthreads,
        const int do_sort,
        GB_Context Context
    )
    {
        if (GB_IS_SPARSE (A) && GB_IS_SPARSE (B))
        {
            // both A and B are sparse
            #define GB_META16
            #define GB_NO_MASK 0
            #define GB_MASK_COMP 0
            #define GB_A_IS_SPARSE 1
            #define GB_A_IS_HYPER  0
            #define GB_A_IS_BITMAP 0
            #define GB_A_IS_FULL   0
            #define GB_B_IS_SPARSE 1
            #define GB_B_IS_HYPER  0
            #define GB_B_IS_BITMAP 0
            #define GB_B_IS_FULL   0
            #include "GB_meta16_definitions.h"
            #include "GB_AxB_saxpy3_template.c"
        }
        else
        {
            // general case
            #undef GB_META16
            #define GB_NO_MASK 0
            #define GB_MASK_COMP 0
            #include "GB_meta16_definitions.h"
            #include "GB_AxB_saxpy3_template.c"
        }
        return (GrB_SUCCESS) ;
    }

#endif

//------------------------------------------------------------------------------
//GB_Asaxpy3B_noM: C=A*B: saxpy method (Gustavson + Hash)
//------------------------------------------------------------------------------

#if ( !GB_DISABLE )

    GrB_Info GB (_Asaxpy3B_noM__plus_second_int32)
    (
        GrB_Matrix C,   // C=A*B, C sparse or hypersparse
        const GrB_Matrix A, bool A_is_pattern,
        const GrB_Matrix B, bool B_is_pattern,
        GB_saxpy3task_struct *restrict SaxpyTasks,
        const int ntasks, const int nfine, const int nthreads,
        const int do_sort,
        GB_Context Context
    )
    {
        if (GB_IS_SPARSE (A) && GB_IS_SPARSE (B))
        {
            // both A and B are sparse
            #define GB_META16
            #define GB_NO_MASK 1
            #define GB_MASK_COMP 0
            #define GB_A_IS_SPARSE 1
            #define GB_A_IS_HYPER  0
            #define GB_A_IS_BITMAP 0
            #define GB_A_IS_FULL   0
            #define GB_B_IS_SPARSE 1
            #define GB_B_IS_HYPER  0
            #define GB_B_IS_BITMAP 0
            #define GB_B_IS_FULL   0
            #include "GB_meta16_definitions.h"
            #include "GB_AxB_saxpy3_template.c"
        }
        else
        {
            // general case
            #undef GB_META16
            #define GB_NO_MASK 1
            #define GB_MASK_COMP 0
            #include "GB_meta16_definitions.h"
            #include "GB_AxB_saxpy3_template.c"
        }
        return (GrB_SUCCESS) ;
    }

#endif

//------------------------------------------------------------------------------
//GB_Asaxpy3B_notM: C<!M>=A*B: saxpy method (Gustavson + Hash)
//------------------------------------------------------------------------------

#if ( !GB_DISABLE )

    GrB_Info GB (_Asaxpy3B_notM__plus_second_int32)
    (
        GrB_Matrix C,   // C<!M>=A*B, C sparse or hypersparse
        const GrB_Matrix M, const bool Mask_struct,
        const bool M_in_place,
        const GrB_Matrix A, bool A_is_pattern,
        const GrB_Matrix B, bool B_is_pattern,
        GB_saxpy3task_struct *restrict SaxpyTasks,
        const int ntasks, const int nfine, const int nthreads,
        const int do_sort,
        GB_Context Context
    )
    {
        if (GB_IS_SPARSE (A) && GB_IS_SPARSE (B))
        {
            // both A and B are sparse
            #define GB_META16
            #define GB_NO_MASK 0
            #define GB_MASK_COMP 1
            #define GB_A_IS_SPARSE 1
            #define GB_A_IS_HYPER  0
            #define GB_A_IS_BITMAP 0
            #define GB_A_IS_FULL   0
            #define GB_B_IS_SPARSE 1
            #define GB_B_IS_HYPER  0
            #define GB_B_IS_BITMAP 0
            #define GB_B_IS_FULL   0
            #include "GB_meta16_definitions.h"
            #include "GB_AxB_saxpy3_template.c"
        }
        else
        {
            // general case
            #undef GB_META16
            #define GB_NO_MASK 0
            #define GB_MASK_COMP 1
            #include "GB_meta16_definitions.h"
            #include "GB_AxB_saxpy3_template.c"
        }
        return (GrB_SUCCESS) ;
    }

#endif
#endif

