//------------------------------------------------------------------------------
// GB_AxB_saxpy3_generic: compute C=A*B, C<M>=A*B, or C<!M>=A*B in parallel
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// GB_AxB_saxpy3_generic computes C=A*B, C<M>=A*B, or C<!M>=A*B in parallel,
// with arbitrary types and operators.

//------------------------------------------------------------------------------

#include "GB_mxm.h"
#include "GB_AxB_saxpy3.h"
#include "GB_bracket.h"
#include "GB_sort.h"
#include "GB_atomics.h"

GrB_Info GB_AxB_saxpy3_generic
(
    GrB_Matrix C,
    const GrB_Matrix M, bool Mask_comp, const bool Mask_struct,
    const GrB_Matrix A, bool A_is_pattern,
    const GrB_Matrix B, bool B_is_pattern,
    const GrB_Semiring semiring,    // semiring that defines C=A*B
    const bool flipxy,              // if true, do z=fmult(b,a) vs fmult(a,b)
    GB_saxpy3task_struct *GB_RESTRICT TaskList,
    const int ntasks,
    const int nfine,
    const int nthreads,
    GB_Context Context
)
{

    //----------------------------------------------------------------------
    // get operators, functions, workspace, contents of A, B, and C
    //----------------------------------------------------------------------

    GrB_BinaryOp mult = semiring->multiply ;
    GrB_Monoid add = semiring->add ;
    ASSERT (mult->ztype == add->op->ztype) ;

    GxB_binary_function fmult = mult->function ;
    GxB_binary_function fadd  = add->op->function ;

    size_t csize = C->type->size ;
    size_t asize = A_is_pattern ? 0 : A->type->size ;
    size_t bsize = B_is_pattern ? 0 : B->type->size ;

    size_t xsize = mult->xtype->size ;
    size_t ysize = mult->ytype->size ;

    // scalar workspace: because of typecasting, the x/y types need not
    // be the same as the size of the A and B types.
    // flipxy false: aik = (xtype) A(i,k) and bkj = (ytype) B(k,j)
    // flipxy true:  aik = (ytype) A(i,k) and bkj = (xtype) B(k,j)
    size_t aik_size = flipxy ? ysize : xsize ;
    size_t bkj_size = flipxy ? xsize : ysize ;

    GB_void *GB_RESTRICT terminal = add->terminal ;
    GB_void *GB_RESTRICT identity = add->identity ;

    GB_cast_function cast_A, cast_B ;
    if (flipxy)
    { 
        // A is typecasted to y, and B is typecasted to x
        cast_A = A_is_pattern ? NULL : 
                 GB_cast_factory (mult->ytype->code, A->type->code) ;
        cast_B = B_is_pattern ? NULL : 
                 GB_cast_factory (mult->xtype->code, B->type->code) ;
    }
    else
    { 
        // A is typecasted to x, and B is typecasted to y
        cast_A = A_is_pattern ? NULL :
                 GB_cast_factory (mult->xtype->code, A->type->code) ;
        cast_B = B_is_pattern ? NULL :
                 GB_cast_factory (mult->ytype->code, B->type->code) ;
    }

    //----------------------------------------------------------------------
    // C = A*B via saxpy3 method, function pointers, and typecasting
    //----------------------------------------------------------------------

    #define GB_IDENTITY identity

    // aik = A(i,k), located in Ax [pA]
    #define GB_GETA(aik,Ax,pA)                                          \
        GB_void aik [GB_VLA(aik_size)] ;                                \
        if (!A_is_pattern) cast_A (aik, Ax +((pA)*asize), asize)

    // bkj = B(k,j), located in Bx [pB]
    #define GB_GETB(bkj,Bx,pB)                                          \
        GB_void bkj [GB_VLA(bkj_size)] ;                                \
        if (!B_is_pattern) cast_B (bkj, Bx +((pB)*bsize), bsize)

    // t = A(i,k) * B(k,j)
    #define GB_MULT(t, aik, bkj)                                        \
        GB_MULTIPLY (t, aik, bkj)

    // define t for each task
    #define GB_CIJ_DECLARE(t)                                           \
        GB_void t [GB_VLA(csize)]

    // address of Cx [p]
    #define GB_CX(p) (Cx +((p)*csize))

    // Cx [p] = t
    #define GB_CIJ_WRITE(p,t)                                           \
        memcpy (GB_CX (p), t, csize)

    // Cx [p] += t
    #define GB_CIJ_UPDATE(p,t)                                          \
        fadd (GB_CX (p), GB_CX (p), t)

    // address of Hx [i]
    #define GB_HX(i) (Hx +((i)*csize))

    // atomic update not available for function pointers
    #define GB_HAS_ATOMIC 0

    // normal Hx [i] += t
    #define GB_HX_UPDATE(i, t)                                          \
        fadd (GB_HX (i), GB_HX (i), t)

    // normal Hx [i] = t
    #define GB_HX_WRITE(i, t)                                           \
        memcpy (GB_HX (i), t, csize)

    // Cx [p] = Hx [i]
    #define GB_CIJ_GATHER(p,i)                                          \
        memcpy (GB_CX (p), GB_HX(i), csize)

    // memcpy (&(Cx [pC]), &(Hx [i]), len)
    #define GB_CIJ_MEMCPY(pC,i,len) \
        memcpy (Cx +((pC)*csize), Hx +((i)*csize), (len) * csize)

    // 1 if monoid update can skipped entirely (the ANY monoid)
    #define GB_IS_ANY_MONOID 0

    // user-defined monoid update cannot be done with an OpenMP atomic
    #define GB_HAS_OMP_ATOMIC 0

    // not an ANY_PAIR semiring
    #define GB_IS_ANY_PAIR_SEMIRING 0

    // not a PAIR multiply operator 
    #define GB_IS_PAIR_MULTIPLIER 0

    #define GB_ATYPE GB_void
    #define GB_BTYPE GB_void
    #define GB_CTYPE GB_void

    // no vectorization
    #define GB_PRAGMA_VECTORIZE
    #define GB_PRAGMA_VECTORIZE_DOT

    // definitions for GB_AxB_saxpy3_template.c
    #include "GB_AxB_saxpy3_template.h"

    if (flipxy)
    { 
        #define GB_MULTIPLY(z,x,y) fmult (z,y,x)
        #include "GB_AxB_saxpy3_template.c"
        #undef GB_MULTIPLY
    }
    else
    { 
        #define GB_MULTIPLY(z,x,y) fmult (z,x,y)
        #include "GB_AxB_saxpy3_template.c"
        #undef GB_MULTIPLY
    }

    return (GrB_SUCCESS) ;
}

