//------------------------------------------------------------------------------
// GB_AxB__any_div_uint64.c: matrix multiply for a single semiring
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// If this file is in the Generated1/ or Generated2/ folder, do not edit it
// (it is auto-generated from Generator/*).

#include "GB_dev.h"

#ifndef GBCUDA_DEV

#include "GB.h"
#include "GB_control.h"
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

// A'*B (dot2):        GB (_Adot2B__any_div_uint64)
// A'*B (dot3):        GB (_Adot3B__any_div_uint64)
// C+=A'*B (dot4):     GB (_Adot4B__(none))
// A*B (saxpy bitmap): GB (_AsaxbitB__any_div_uint64)
// A*B (saxpy3):       GB (_Asaxpy3B__any_div_uint64)
//     no mask:        GB (_Asaxpy3B_noM__any_div_uint64)
//     mask M:         GB (_Asaxpy3B_M__any_div_uint64)
//     mask !M:        GB (_Asaxpy3B_notM__any_div_uint64)
// A*B (saxpy4):       GB (_Asaxpy4B__(none))
// A*B (saxpy5):       GB (_Asaxpy5B__(none))

// C type:     uint64_t
// A type:     uint64_t
// A pattern?  0
// B type:     uint64_t
// B pattern?  0

// Multiply: z = GB_idiv_uint64 (x, y)
// Add:      cij = t
//    'any' monoid?  1
//    atomic?        1
//    OpenMP atomic? 0
//    identity:      0
//    terminal?      1
//    terminal condition: break ;
// MultAdd:  { uint64_t x_op_y = GB_idiv_uint64 (x, y) ; z = x_op_y ; }

#define GB_ATYPE \
    uint64_t

#define GB_BTYPE \
    uint64_t

#define GB_CTYPE \
    uint64_t

#define GB_ASIZE \
    sizeof (uint64_t)

#define GB_BSIZE \
    sizeof (uint64_t) 

#define GB_CSIZE \
    sizeof (uint64_t)

// # of bits in the type of C, for AVX2 and AVX512F
#define GB_CNBITS \
    64

// true for int64, uint64, float, double, float complex, and double complex 
#define GB_CTYPE_IGNORE_OVERFLOW \
    1

// aik = Ax [pA]
#define GB_GETA(aik,Ax,pA,A_iso) \
    uint64_t aik = GBX (Ax, pA, A_iso)

// true if values of A are not used
#define GB_A_IS_PATTERN \
    0 \

// bkj = Bx [pB]
#define GB_GETB(bkj,Bx,pB,B_iso) \
    uint64_t bkj = GBX (Bx, pB, B_iso)

// true if values of B are not used
#define GB_B_IS_PATTERN \
    0 \

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
    z = GB_idiv_uint64 (x, y)

// cast from a real scalar (or 2, if C is complex) to the type of C
#define GB_CTYPE_CAST(x,y) \
    ((uint64_t) x)

// cast from a real scalar (or 2, if A is complex) to the type of A
#define GB_ATYPE_CAST(x,y) \
    ((uint64_t) x)

// multiply-add
#define GB_MULTADD(z, x, y, i, k, j) \
    { uint64_t x_op_y = GB_idiv_uint64 (x, y) ; z = x_op_y ; }

// monoid identity value
#define GB_IDENTITY \
    0

// 1 if the identity value can be assigned via memset, with all bytes the same
#define GB_HAS_IDENTITY_BYTE \
    0

// identity byte, for memset
#define GB_IDENTITY_BYTE \
    (none)

// true if the monoid has a terminal value
#define GB_MONOID_IS_TERMINAL \
    1

// break if cij reaches the terminal value (dot product only)
#define GB_DOT_TERMINAL(cij) \
    break ;

// simd pragma for dot-product loop vectorization
#define GB_PRAGMA_SIMD_DOT(cij) \
    ;

// simd pragma for other loop vectorization
#define GB_PRAGMA_SIMD_VECTORIZE GB_PRAGMA_SIMD

// 1 for the PLUS_PAIR_(real) semirings, not for the complex case
#define GB_IS_PLUS_PAIR_REAL_SEMIRING \
    0

// 1 if the semiring is accelerated with AVX2 or AVX512f
#define GB_SEMIRING_HAS_AVX_IMPLEMENTATION \
    0

// declare the cij scalar (initialize cij to zero for PLUS_PAIR)
#define GB_CIJ_DECLARE(cij) \
    uint64_t cij

// Cx [pC] = cij
#define GB_PUTC(cij,p) \
    Cx [p] = cij

// Cx [p] = t
#define GB_CIJ_WRITE(p,t) \
    Cx [p] = t

// C(i,j) += t
#define GB_CIJ_UPDATE(p,t) \
    Cx [p] = t

// x + y
#define GB_ADD_FUNCTION(x,y) \
    y

// bit pattern for bool, 8-bit, 16-bit, and 32-bit integers
#define GB_CTYPE_BITS \
    0

// 1 if monoid update can skipped entirely (the ANY monoid)
#define GB_IS_ANY_MONOID \
    1

// 1 if monoid update is EQ
#define GB_IS_EQ_MONOID \
    0

// 1 if monoid update can be done atomically, 0 otherwise
#define GB_HAS_ATOMIC \
    1

// 1 if monoid update can be done with an OpenMP atomic update, 0 otherwise
#if GB_COMPILER_MSC
    /* MS Visual Studio only has OpenMP 2.0, with fewer atomics */
    #define GB_HAS_OMP_ATOMIC \
        0
#else
    #define GB_HAS_OMP_ATOMIC \
        0
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

// 1 for the FIRSTI1, FIRSTJ1, SECONDI1, or SECONDJ1 multiply operators
#define GB_OFFSET \
    0

// atomic compare-exchange
#define GB_ATOMIC_COMPARE_EXCHANGE(target, expected, desired) \
    GB_ATOMIC_COMPARE_EXCHANGE_64 (target, expected, desired)

// Hx [i] = t
#define GB_HX_WRITE(i,t) \
    Hx [i] = t

// Cx [p] = Hx [i]
#define GB_CIJ_GATHER(p,i) \
    Cx [p] = Hx [i]

// Cx [p] += Hx [i]
#define GB_CIJ_GATHER_UPDATE(p,i) \
    Cx [p] = Hx [i]

// Hx [i] += t
#define GB_HX_UPDATE(i,t) \
    Hx [i] = t

// memcpy (&(Cx [p]), &(Hx [i]), len)
#define GB_CIJ_MEMCPY(p,i,len) \
    memcpy (Cx +(p), Hx +(i), (len) * sizeof(uint64_t));

// disable this semiring and use the generic case if these conditions hold
#define GB_DISABLE \
    (GxB_NO_ANY || GxB_NO_DIV || GxB_NO_UINT64 || GxB_NO_ANY_UINT64 || GxB_NO_DIV_UINT64 || GxB_NO_ANY_DIV_UINT64)

//------------------------------------------------------------------------------
// GB_Adot2B: C=A'*B, C<M>=A'*B, or C<!M>=A'*B: dot product method, C is bitmap
//------------------------------------------------------------------------------

// if A_not_transposed is true, then C=A*B is computed where A is bitmap or full

GrB_Info GB (_Adot2B__any_div_uint64)
(
    GrB_Matrix C,
    const GrB_Matrix M, const bool Mask_comp, const bool Mask_struct,
    const bool A_not_transposed,
    const GrB_Matrix A, int64_t *restrict A_slice,
    const GrB_Matrix B, int64_t *restrict B_slice,
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

GrB_Info GB (_Adot3B__any_div_uint64)
(
    GrB_Matrix C,
    const GrB_Matrix M, const bool Mask_struct,
    const GrB_Matrix A,
    const GrB_Matrix B,
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

#if 0

    GrB_Info GB (_Adot4B__(none))
    (
        GrB_Matrix C,
        const GrB_Matrix A, int64_t *restrict A_slice, int naslice,
        const GrB_Matrix B, int64_t *restrict B_slice, int nbslice,
        const int nthreads,
        GB_Context Context
    )
    { 
        #if GB_DISABLE
        return (GrB_NO_VALUE) ;
        #else
        #include "GB_AxB_dot4_meta.c"
        return (GrB_SUCCESS) ;
        #endif
    }

#endif

//------------------------------------------------------------------------------
// GB_AsaxbitB: C=A*B, C<M>=A*B, C<!M>=A*B: saxpy method, C is bitmap/full
//------------------------------------------------------------------------------

#include "GB_AxB_saxpy3_template.h"

GrB_Info GB (_AsaxbitB__any_div_uint64)
(
    GrB_Matrix C,   // bitmap or full
    const GrB_Matrix M, const bool Mask_comp, const bool Mask_struct,
    const GrB_Matrix A,
    const GrB_Matrix B,
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
// GB_Asaxpy4B: C += A*B when C is full
//------------------------------------------------------------------------------

#if 0

    GrB_Info GB (_Asaxpy4B__(none))
    (
        GrB_Matrix C,
        const GrB_Matrix A,
        const GrB_Matrix B,
        const int ntasks,
        const int nthreads,
        const int nfine_tasks_per_vector,
        const bool use_coarse_tasks,
        const bool use_atomics,
        const int64_t *A_slice,
        GB_Context Context
    )
    { 
        #if GB_DISABLE
        return (GrB_NO_VALUE) ;
        #else
        #include "GB_AxB_saxpy4_template.c"
        return (GrB_SUCCESS) ;
        #endif
    }

#endif

//------------------------------------------------------------------------------
// GB_Asaxpy5B: C += A*B when C is full, A is bitmap/full, B is sparse/hyper
//------------------------------------------------------------------------------

#if 0

    #if GB_DISABLE
    #elif ( !GB_A_IS_PATTERN )

        //----------------------------------------------------------------------
        // saxpy5 method with vectors of length 8 for double, 16 for single
        //----------------------------------------------------------------------

        // AVX512F: vector registers are 512 bits, or 64 bytes, which can hold
        // 16 floats or 8 doubles.

        #define GB_V16_512 (16 * GB_CNBITS <= 512)
        #define GB_V8_512  ( 8 * GB_CNBITS <= 512)
        #define GB_V4_512  ( 4 * GB_CNBITS <= 512)

        #define GB_V16 GB_V16_512
        #define GB_V8  GB_V8_512
        #define GB_V4  GB_V4_512

        #if GB_SEMIRING_HAS_AVX_IMPLEMENTATION && GB_COMPILER_SUPPORTS_AVX512F \
            && GB_V4_512

            GB_TARGET_AVX512F static inline void GB_AxB_saxpy5_unrolled_avx512f
            (
                GrB_Matrix C,
                const GrB_Matrix A,
                const GrB_Matrix B,
                const int ntasks,
                const int nthreads,
                const int64_t *B_slice,
                GB_Context Context
            )
            {
                #include "GB_AxB_saxpy5_unrolled.c"
            }

        #endif

        //----------------------------------------------------------------------
        // saxpy5 method with vectors of length 4 for double, 8 for single
        //----------------------------------------------------------------------

        // AVX2: vector registers are 256 bits, or 32 bytes, which can hold
        // 8 floats or 4 doubles.

        #define GB_V16_256 (16 * GB_CNBITS <= 256)
        #define GB_V8_256  ( 8 * GB_CNBITS <= 256)
        #define GB_V4_256  ( 4 * GB_CNBITS <= 256)

        #undef  GB_V16
        #undef  GB_V8
        #undef  GB_V4

        #define GB_V16 GB_V16_256
        #define GB_V8  GB_V8_256
        #define GB_V4  GB_V4_256

        #if GB_SEMIRING_HAS_AVX_IMPLEMENTATION && GB_COMPILER_SUPPORTS_AVX2 \
            && GB_V4_256

            GB_TARGET_AVX2 static inline void GB_AxB_saxpy5_unrolled_avx2
            (
                GrB_Matrix C,
                const GrB_Matrix A,
                const GrB_Matrix B,
                const int ntasks,
                const int nthreads,
                const int64_t *B_slice,
                GB_Context Context
            )
            {
                #include "GB_AxB_saxpy5_unrolled.c"
            }

        #endif

        //----------------------------------------------------------------------
        // saxpy5 method unrolled, with no vectors
        //----------------------------------------------------------------------

        #undef  GB_V16
        #undef  GB_V8
        #undef  GB_V4

        #define GB_V16 0
        #define GB_V8  0
        #define GB_V4  0

        static inline void GB_AxB_saxpy5_unrolled_vanilla
        (
            GrB_Matrix C,
            const GrB_Matrix A,
            const GrB_Matrix B,
            const int ntasks,
            const int nthreads,
            const int64_t *B_slice,
            GB_Context Context
        )
        {
            #include "GB_AxB_saxpy5_unrolled.c"
        }

    #endif

    GrB_Info GB (_Asaxpy5B__(none))
    (
        GrB_Matrix C,
        const GrB_Matrix A,
        const GrB_Matrix B,
        const int ntasks,
        const int nthreads,
        const int64_t *B_slice,
        GB_Context Context
    )
    { 
        #if GB_DISABLE
        return (GrB_NO_VALUE) ;
        #else
        #include "GB_AxB_saxpy5_meta.c"
        return (GrB_SUCCESS) ;
        #endif
    }

#endif

//------------------------------------------------------------------------------
// GB_Asaxpy3B: C=A*B, C<M>=A*B, C<!M>=A*B: saxpy method (Gustavson + Hash)
//------------------------------------------------------------------------------

GrB_Info GB (_Asaxpy3B__any_div_uint64)
(
    GrB_Matrix C,   // C<any M>=A*B, C sparse or hypersparse
    const GrB_Matrix M, const bool Mask_comp, const bool Mask_struct,
    const bool M_in_place,
    const GrB_Matrix A,
    const GrB_Matrix B,
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
        return (GB (_Asaxpy3B_noM__any_div_uint64) (C, A, B,
            SaxpyTasks, ntasks, nfine, nthreads, do_sort, Context)) ;
    }
    else if (!Mask_comp)
    {
        // C<M> = A*B
        return (GB (_Asaxpy3B_M__any_div_uint64) (C,
            M, Mask_struct, M_in_place, A, B,
            SaxpyTasks, ntasks, nfine, nthreads, do_sort, Context)) ;
    }
    else
    {
        // C<!M> = A*B
        return (GB (_Asaxpy3B_notM__any_div_uint64) (C,
            M, Mask_struct, M_in_place, A, B,
            SaxpyTasks, ntasks, nfine, nthreads, do_sort, Context)) ;
    }
    #endif
}

//------------------------------------------------------------------------------
// GB_Asaxpy3B_M: C<M>=A*B: saxpy method (Gustavson + Hash)
//------------------------------------------------------------------------------

#if ( !GB_DISABLE )

    GrB_Info GB (_Asaxpy3B_M__any_div_uint64)
    (
        GrB_Matrix C,   // C<M>=A*B, C sparse or hypersparse
        const GrB_Matrix M, const bool Mask_struct,
        const bool M_in_place,
        const GrB_Matrix A,
        const GrB_Matrix B,
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

    GrB_Info GB (_Asaxpy3B_noM__any_div_uint64)
    (
        GrB_Matrix C,   // C=A*B, C sparse or hypersparse
        const GrB_Matrix A,
        const GrB_Matrix B,
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

    GrB_Info GB (_Asaxpy3B_notM__any_div_uint64)
    (
        GrB_Matrix C,   // C<!M>=A*B, C sparse or hypersparse
        const GrB_Matrix M, const bool Mask_struct,
        const bool M_in_place,
        const GrB_Matrix A,
        const GrB_Matrix B,
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

