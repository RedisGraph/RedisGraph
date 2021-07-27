//------------------------------------------------------------------------------
// GB_AxB.c: matrix multiply for a single semiring
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// If this file is in the Generated1/ or Generated2/ folder, do not edit it
// (it is auto-generated from Generator/*).

#include "GB_dev.h"

ifndef_compact

#include "GB.h"
#include "GB_control.h"
#include "GB_bracket.h"
#include "GB_sort.h"
#include "GB_atomics.h"
#include "GB_AxB_saxpy.h"
if_not_any_pair_semiring
#include "GB_AxB__include2.h"
#else
#include "GB_AxB__include1.h"
#endif
#include "GB_unused.h"
#include "GB_bitmap_assign_methods.h"
#include "GB_ek_slice_search.c"

// This C=A*B semiring is defined by the following types and operators:

// A'*B (dot2):        GB (_Adot2B)
// A'*B (dot3):        GB (_Adot3B)
// C+=A'*B (dot4):     GB (_Adot4B)
// A*B (saxpy bitmap): GB (_AsaxbitB)
// A*B (saxpy3):       GB (_Asaxpy3B)
//     no mask:        GB (_Asaxpy3B_noM)
//     mask M:         GB (_Asaxpy3B_M)
//     mask !M:        GB (_Asaxpy3B_notM)

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

#define GB_ASIZE \
    GB_asize

#define GB_BSIZE \
    GB_bsize 

#define GB_CSIZE \
    GB_csize

// true for int64, uint64, float, double, float complex, and double complex 
#define GB_CTYPE_IGNORE_OVERFLOW \
    GB_ctype_ignore_overflow

// aik = Ax [pA]
#define GB_GETA(aik,Ax,pA,A_iso) \
    GB_geta(aik,Ax,pA,A_iso)

// bkj = Bx [pB]
#define GB_GETB(bkj,Bx,pB,B_iso) \
    GB_getb(bkj,Bx,pB,B_iso)

// Gx [pG] = Ax [pA]
#define GB_LOADA(Gx,pG,Ax,pA,A_iso) \
    GB_loada(Gx,pG,Ax,pA,A_iso)

// Gx [pG] = Bx [pB]
#define GB_LOADB(Gx,pG,Bx,pB,B_iso) \
    GB_loadb(Gx,pG,Bx,pB,B_iso)

#define GB_CX(p) \
    GB_cx

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

// declare the cij scalar (initialize cij to zero for PLUS_PAIR)
#define GB_CIJ_DECLARE(cij) \
    GB_cij_declare

// cij = Cx [pC] for dot4 method only
#define GB_GET4C(cij,p) \
    GB_get4c

// Cx [pC] = cij
#define GB_PUTC(cij,p) \
    GB_putc

// Cx [p] = t
#define GB_CIJ_WRITE(p,t) \
    GB_cij_write

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

// 1 for the ANY_PAIR_ISO semiring
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
    GB_atomic_compare_exchange

// Hx [i] = t
#define GB_HX_WRITE(i,t) \
    GB_hx_write

// Cx [p] = Hx [i]
#define GB_CIJ_GATHER(p,i) \
    GB_cij_gather

// Cx [p] += Hx [i]
#define GB_CIJ_GATHER_UPDATE(p,i) \
    GB_add_update(Cx [p], Hx [i])

// Hx [i] += t
#define GB_HX_UPDATE(i,t) \
    GB_add_update(Hx [i], t)

// memcpy (&(Cx [p]), &(Hx [i]), len)
#define GB_CIJ_MEMCPY(p,i,len) \
    GB_cij_memcpy

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
// GB_Adot2B: C=A'*B, C<M>=A'*B, or C<!M>=A'*B: dot product method, C is bitmap
//------------------------------------------------------------------------------

GrB_Info GB (_Adot2B)
(
    GrB_Matrix C,
    const GrB_Matrix M, const bool Mask_comp, const bool Mask_struct,
    const GrB_Matrix A, bool A_is_pattern, int64_t *restrict A_slice,
    const GrB_Matrix B, bool B_is_pattern, int64_t *restrict B_slice,
    int nthreads, int naslice, int nbslice
)
{ 
    if_disabled
    return (GrB_NO_VALUE) ;
    #else
    #include "GB_AxB_dot2_meta.c"
    return (GrB_SUCCESS) ;
    #endif
}

//------------------------------------------------------------------------------
// GB_Adot3B: C<M>=A'*B: masked dot product, C is sparse or hyper
//------------------------------------------------------------------------------

GrB_Info GB (_Adot3B)
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
    if_disabled
    return (GrB_NO_VALUE) ;
    #else
    #include "GB_AxB_dot3_meta.c"
    return (GrB_SUCCESS) ;
    #endif
}

//------------------------------------------------------------------------------
// GB_Adot4B:  C+=A'*B: dense dot product (not used for ANY_PAIR_ISO)
//------------------------------------------------------------------------------

if_not_any_pair_semiring

    GrB_Info GB (_Adot4B)
    (
        GrB_Matrix C, const bool C_in_iso, const GB_void *cinput_void,
        const GrB_Matrix A, bool A_is_pattern,
        int64_t *restrict A_slice, int naslice,
        const GrB_Matrix B, bool B_is_pattern,
        int64_t *restrict B_slice, int nbslice,
        const int nthreads
    )
    { 
        if_disabled
        return (GrB_NO_VALUE) ;
        #else
        const GB_ctype cinput = (*((const GB_ctype *) cinput_void)) ;
        #include "GB_AxB_dot4_meta.c"
        return (GrB_SUCCESS) ;
        #endif
    }

#endif

//------------------------------------------------------------------------------
// GB_AsaxbitB: C=A*B, C<M>=A*B, C<!M>=A*B: saxpy method, C is bitmap/full
//------------------------------------------------------------------------------

#include "GB_AxB_saxpy3_template.h"

GrB_Info GB (_AsaxbitB)
(
    GrB_Matrix C,   // bitmap or full
    const GrB_Matrix M, const bool Mask_comp, const bool Mask_struct,
    const GrB_Matrix A, bool A_is_pattern,
    const GrB_Matrix B, bool B_is_pattern,
    GB_Context Context
)
{ 
    if_disabled
    return (GrB_NO_VALUE) ;
    #else
    #include "GB_bitmap_AxB_saxpy_template.c"
    return (GrB_SUCCESS) ;
    #endif
}

//------------------------------------------------------------------------------
// GB_Asaxpy3B: C=A*B, C<M>=A*B, C<!M>=A*B: saxpy method (Gustavson + Hash)
//------------------------------------------------------------------------------

GrB_Info GB (_Asaxpy3B)
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
    if_disabled
    return (GrB_NO_VALUE) ;
    #else
    ASSERT (GB_IS_SPARSE (C) || GB_IS_HYPERSPARSE (C)) ;
    if (M == NULL)
    {
        // C = A*B, no mask
        return (GB (_Asaxpy3B_noM) (C,
            A, A_is_pattern, B, B_is_pattern,
            SaxpyTasks, ntasks, nfine, nthreads, do_sort, Context)) ;
    }
    else if (!Mask_comp)
    {
        // C<M> = A*B
        return (GB (_Asaxpy3B_M) (C,
            M, Mask_struct, M_in_place,
            A, A_is_pattern, B, B_is_pattern,
            SaxpyTasks, ntasks, nfine, nthreads, do_sort, Context)) ;
    }
    else
    {
        // C<!M> = A*B
        return (GB (_Asaxpy3B_notM) (C,
            M, Mask_struct, M_in_place,
            A, A_is_pattern, B, B_is_pattern,
            SaxpyTasks, ntasks, nfine, nthreads, do_sort, Context)) ;
    }
    #endif
}

//------------------------------------------------------------------------------
// GB_Asaxpy3B_M: C<M>=A*Bi: saxpy method (Gustavson + Hash)
//------------------------------------------------------------------------------

if_not_disabled

    GrB_Info GB (_Asaxpy3B_M)
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

if_not_disabled

    GrB_Info GB (_Asaxpy3B_noM)
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

if_not_disabled

    GrB_Info GB (_Asaxpy3B_notM)
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

