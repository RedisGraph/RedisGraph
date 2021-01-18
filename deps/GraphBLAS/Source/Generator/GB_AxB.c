//------------------------------------------------------------------------------
// GB_AxB:  hard-coded functions for semiring: C<M>=A*B or A'*B
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// If this file is in the Generated/ folder, do not edit it (auto-generated).

#include "GB.h"
#ifndef GBCOMPACT
#include "GB_control.h"
#include "GB_ek_slice.h"
#include "GB_bracket.h"
#include "GB_sort.h"
#include "GB_atomics.h"
#include "GB_AxB_saxpy3.h"
#include "GB_AxB__include.h"
#include "GB_unused.h"
#include "GB_bitmap_assign_methods.h"

// The C=A*B semiring is defined by the following types and operators:

// A'*B function (dot2):     GB_Adot2B
// A'*B function (dot3):     GB_Adot3B
// C+=A'*B function (dot4):  GB_Adot4B
// A*B function (saxpy3):    GB_Asaxpy3B

// C type:   GB_ctype
// A type:   GB_atype
// B type:   GB_btype

// Multiply: GB_multiply(z,aik,bkj,i,k,j)
// Add:      GB_add_update(cij, z)
//           'any' monoid?  GB_is_any_monoid
//           atomic?        GB_has_atomic
//           OpenMP atomic? GB_has_omp_atomic
// MultAdd:  GB_multiply_add(cij,aik,bkj,i,k,j)
// Identity: GB_identity
// Terminal: GB_terminal

#define GB_ATYPE \
    GB_atype

#define GB_BTYPE \
    GB_btype

#define GB_CTYPE \
    GB_ctype

#define GB_ASIZE (sizeof (GB_BTYPE))
#define GB_BSIZE (sizeof (GB_BTYPE))
#define GB_CSIZE (sizeof (GB_CTYPE))

// true for int64, uint64, float, double, float complex, and double complex 
#define GB_CTYPE_IGNORE_OVERFLOW \
    GB_ctype_ignore_overflow

// aik = Ax [pA]
#define GB_GETA(aik,Ax,pA) \
    GB_geta(aik,Ax,pA)

// bkj = Bx [pB]
#define GB_GETB(bkj,Bx,pB) \
    GB_getb(bkj,Bx,pB)

// Gx [pG] = Ax [pA]
#define GB_LOADA(Gx,pG,Ax,pA) \
    Gx [pG] = Ax [pA]

// Gx [pG] = Bx [pB]
#define GB_LOADB(Gx,pG,Bx,pB) \
    GB_loadb(Gx,pG,Bx,pB)

#define GB_CX(p) Cx [p]

// multiply operator
#define GB_MULT(z, x, y, i, k, j) \
    GB_multiply(z, x, y, i, k, j)

// cast from a real scalar (or 2, if C is complex) to the type of C
#define GB_CTYPE_CAST(x,y) \
    GB_ctype_cast(x,y)

// cast from a real scalar (or 2, if A is complex) to the type of A
#define GB_ATYPE_CAST(x,y) \
    GB_atype_cast(x,y)

// multiply-add
#define GB_MULTADD(z, x, y, i, k, j) \
    GB_multiply_add(z, x, y, i, k, j)

// monoid identity value
#define GB_IDENTITY \
    GB_identity

// 1 if the identity value can be assigned via memset, with all bytes the same
#define GB_HAS_IDENTITY_BYTE \
    GB_has_identity_byte

// identity byte, for memset
#define GB_IDENTITY_BYTE \
    GB_identity_byte

// break if cij reaches the terminal value (dot product only)
#define GB_DOT_TERMINAL(cij) \
    GB_terminal

// simd pragma for dot-product loop vectorization
#define GB_PRAGMA_SIMD_DOT(cij) \
    GB_dot_simd_vectorize(cij)

// simd pragma for other loop vectorization
#define GB_PRAGMA_SIMD_VECTORIZE GB_PRAGMA_SIMD

// 1 for the PLUS_PAIR_(real) semirings, not for the complex case
#define GB_IS_PLUS_PAIR_REAL_SEMIRING \
    GB_is_plus_pair_real_semiring

// 1 for performance-critical semirings, which get extra optimization
#define GB_IS_PERFORMANCE_CRITICAL_SEMIRING \
    GB_is_performance_critical_semiring

// declare the cij scalar
#if GB_IS_PLUS_PAIR_REAL_SEMIRING
    // also initialize cij to zero
    #define GB_CIJ_DECLARE(cij) \
        GB_ctype cij = 0
#else
    // all other semirings: just declare cij, do not initialize it
    #define GB_CIJ_DECLARE(cij) \
        GB_ctype cij
#endif

// cij = Cx [pC]
#define GB_GETC(cij,p) cij = Cx [p]

// Cx [pC] = cij
#define GB_PUTC(cij,p) Cx [p] = cij

// Cx [p] = t
#define GB_CIJ_WRITE(p,t) Cx [p] = t

// C(i,j) += t
#define GB_CIJ_UPDATE(p,t) \
    GB_add_update(Cx [p], t)

// x + y
#define GB_ADD_FUNCTION(x,y) \
    GB_add_function(x, y)

// bit pattern for bool, 8-bit, 16-bit, and 32-bit integers
#define GB_CTYPE_BITS \
    GB_ctype_bits

// 1 if monoid update can skipped entirely (the ANY monoid)
#define GB_IS_ANY_MONOID \
    GB_is_any_monoid

// 1 if monoid update is EQ
#define GB_IS_EQ_MONOID \
    GB_is_eq_monoid

// 1 if monoid update can be done atomically, 0 otherwise
#define GB_HAS_ATOMIC \
    GB_has_atomic

// 1 if monoid update can be done with an OpenMP atomic update, 0 otherwise
#if GB_MICROSOFT
    #define GB_HAS_OMP_ATOMIC \
        GB_microsoft_has_omp_atomic
#else
    #define GB_HAS_OMP_ATOMIC \
        GB_has_omp_atomic
#endif

// 1 for the ANY_PAIR semirings
#define GB_IS_ANY_PAIR_SEMIRING \
    GB_is_any_pair_semiring

// 1 if PAIR is the multiply operator 
#define GB_IS_PAIR_MULTIPLIER \
    GB_is_pair_multiplier

// 1 if monoid is PLUS_FC32
#define GB_IS_PLUS_FC32_MONOID \
    GB_is_plus_fc32_monoid

// 1 if monoid is PLUS_FC64
#define GB_IS_PLUS_FC64_MONOID \
    GB_is_plus_fc64_monoid

// 1 if monoid is ANY_FC32
#define GB_IS_ANY_FC32_MONOID \
    GB_is_any_fc32_monoid

// 1 if monoid is ANY_FC64
#define GB_IS_ANY_FC64_MONOID \
    GB_is_any_fc64_monoid

// 1 if monoid is MIN for signed or unsigned integers
#define GB_IS_IMIN_MONOID \
    GB_is_imin_monoid

// 1 if monoid is MAX for signed or unsigned integers
#define GB_IS_IMAX_MONOID \
    GB_is_imax_monoid

// 1 if monoid is MIN for float or double
#define GB_IS_FMIN_MONOID \
    GB_is_fmin_monoid

// 1 if monoid is MAX for float or double
#define GB_IS_FMAX_MONOID \
    GB_is_fmax_monoid

// 1 for the FIRSTI or FIRSTI1 multiply operator
#define GB_IS_FIRSTI_MULTIPLIER \
    GB_is_firsti_multiplier

// 1 for the FIRSTJ or FIRSTJ1 multiply operator
#define GB_IS_FIRSTJ_MULTIPLIER \
    GB_is_firstj_multiplier

// 1 for the SECONDJ or SECONDJ1 multiply operator
#define GB_IS_SECONDJ_MULTIPLIER \
    GB_is_secondj_multiplier

// atomic compare-exchange
#define GB_ATOMIC_COMPARE_EXCHANGE(target, expected, desired) \
    GB_atomic_compare_exchange (target, expected, desired)

#if GB_IS_ANY_PAIR_SEMIRING

    // result is purely symbolic; no numeric work to do.  Hx is not used.
    #define GB_HX_WRITE(i,t)
    #define GB_CIJ_GATHER(p,i)
    #define GB_CIJ_GATHER_UPDATE(p,i)
    #define GB_HX_UPDATE(i,t)
    #define GB_CIJ_MEMCPY(p,i,len)

#else

    // Hx [i] = t
    #define GB_HX_WRITE(i,t) Hx [i] = t

    // Cx [p] = Hx [i]
    #define GB_CIJ_GATHER(p,i) Cx [p] = Hx [i]

    // Cx [p] += Hx [i]
    #define GB_CIJ_GATHER_UPDATE(p,i) \
        GB_add_update(Cx [p], Hx [i])

    // Hx [i] += t
    #define GB_HX_UPDATE(i,t) \
        GB_add_update(Hx [i], t)

    // memcpy (&(Cx [p]), &(Hx [i]), len)
    #define GB_CIJ_MEMCPY(p,i,len) \
        memcpy (Cx +(p), Hx +(i), (len) * sizeof(GB_ctype))

#endif

// 1 if the semiring has a concise bitmap multiply-add
#define GB_HAS_BITMAP_MULTADD \
    GB_has_bitmap_multadd

// concise statement(s) for the bitmap case:
//  if (exists)
//      if (cb == 0)
//          cx = ax * bx
//          cb = 1
//      else
//          cx += ax * bx
#define GB_BITMAP_MULTADD(cb,cx,exists,ax,bx) \
    GB_bitmap_multadd(cb,cx,exists,ax,bx)

// define X for bitmap multiply-add
#define GB_XINIT \
    GB_xinit

// load X [1] = bkj for bitmap multiply-add
#define GB_XLOAD(bkj) \
    GB_xload(bkj)

// disable this semiring and use the generic case if these conditions hold
#define GB_DISABLE \
    GB_disable

//------------------------------------------------------------------------------
// C=A'*B, C<M>=A'*B, or C<!M>=A'*B: dot product method where C is bitmap
//------------------------------------------------------------------------------

GrB_Info GB_Adot2B
(
    GrB_Matrix C,
    const GrB_Matrix M, const bool Mask_comp, const bool Mask_struct,
    const GrB_Matrix A, bool A_is_pattern, int64_t *GB_RESTRICT A_slice,
    const GrB_Matrix B, bool B_is_pattern, int64_t *GB_RESTRICT B_slice,
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
// C<M>=A'*B: masked dot product method (phase 2) where C is sparse or hyper
//------------------------------------------------------------------------------

GrB_Info GB_Adot3B
(
    GrB_Matrix C,
    const GrB_Matrix M, const bool Mask_struct,
    const GrB_Matrix A, bool A_is_pattern,
    const GrB_Matrix B, bool B_is_pattern,
    const GB_task_struct *GB_RESTRICT TaskList,
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
// C+=A'*B: dense dot product
//------------------------------------------------------------------------------

GrB_Info GB_Adot4B
(
    GrB_Matrix C,
    const GrB_Matrix A, bool A_is_pattern,
    int64_t *GB_RESTRICT A_slice, int naslice,
    const GrB_Matrix B, bool B_is_pattern,
    int64_t *GB_RESTRICT B_slice, int nbslice,
    const int nthreads
)
{ 
    #if GB_DISABLE
    return (GrB_NO_VALUE) ;
    #else
    #include "GB_AxB_dot4_meta.c"
    return (GrB_SUCCESS) ;
    #endif
}

//------------------------------------------------------------------------------
// C=A*B, C<M>=A*B, C<!M>=A*B: saxpy3 method (Gustavson + Hash)
//------------------------------------------------------------------------------

#include "GB_AxB_saxpy3_template.h"

GrB_Info GB_Asaxpy3B
(
    GrB_Matrix C,
    const GrB_Matrix M, const bool Mask_comp, const bool Mask_struct,
    const bool M_dense_in_place,
    const GrB_Matrix A, bool A_is_pattern,
    const GrB_Matrix B, bool B_is_pattern,
    GB_saxpy3task_struct *GB_RESTRICT TaskList,
    int ntasks,
    int nfine,
    int nthreads,
    const int do_sort,
    GB_Context Context
)
{ 
    #if GB_DISABLE
    return (GrB_NO_VALUE) ;
    #else
    #include "GB_AxB_saxpy_template.c"
    return (GrB_SUCCESS) ;
    #endif
}

#endif

