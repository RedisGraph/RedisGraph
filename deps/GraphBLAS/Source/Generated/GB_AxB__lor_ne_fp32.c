


//------------------------------------------------------------------------------
// GB_AxB:  hard-coded C=A*B and C<M>=A*B
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

// If this filename has a double underscore in its name ("__") then it has been
// automatically constructed from Generator/GB_AxB.c, via the Source/axb*.m
// scripts, and should not be editted.  Edit the original source file instead.

//------------------------------------------------------------------------------

#include "GB.h"
#ifndef GBCOMPACT
#include "GB_heap.h"
#include "GB_AxB__semirings.h"

// The C=A*B semiring is defined by the following types and operators:

// A*B function (Gustavon):  GB_AgusB__lor_ne_fp32
// A'*B function (dot):      GB_AdotB__lor_ne_fp32
// A*B function (heap):      GB_AheapB__lor_ne_fp32
// Z type:   bool (the type of C)
// X type:   float (the type of x for z=mult(x,y))
// Y type:   float (the type of y for z=mult(x,y))
// handle flipxy: 0 (0 if mult(x,y) is commutative, 1 otherwise)
// Identity: false (where cij = (cij || identity) does not change cij)
// Multiply: z = x != y
// Add:      cij = (cij || z)

#define GB_XTYPE \
    float
#define GB_YTYPE \
    float
#define GB_HANDLE_FLIPXY \
    0

#define GB_MULTOP(z,x,y) \
    z = x != y

//------------------------------------------------------------------------------
// C<M>=A*B and C=A*B: gather/scatter saxpy-based method (Gustavson)
//------------------------------------------------------------------------------

#define GB_IDENTITY \
    false

// x [i] = y
#define GB_COPY_SCALAR_TO_ARRAY(x,i,y,s)                \
    x [i] = y ;

// x = y [i]
#define GB_COPY_ARRAY_TO_SCALAR(x,y,i,s)                \
    GB_btype x = y [i] ;

// x [i] = y [i]
#define GB_COPY_ARRAY_TO_ARRAY(x,i,y,j,s)               \
    x [i] = y [j] ;

// mult-add operation (no mask)
#define GB_MULTADD_NOMASK                               \
{                                                       \
    /* Sauna_Work [i] += A(i,k) * B(k,j) */             \
    GB_atype aik = Ax [pA] ;                            \
    bool t ;                                        \
    GB_MULTIPLY (t, aik, bkj) ;                         \
    Sauna_Work [i] = (Sauna_Work [i] || t) ;                         \
}

// mult-add operation (with mask)
#define GB_MULTADD_WITH_MASK                            \
{                                                       \
    /* Sauna_Work [i] += A(i,k) * B(k,j) */             \
    GB_atype aik = Ax [pA] ;                            \
    bool t ;                                        \
    GB_MULTIPLY (t, aik, bkj) ;                         \
    if (mark == hiwater)                                \
    {                                                   \
        /* first time C(i,j) seen */                    \
        Sauna_Mark [i] = hiwater + 1 ;                  \
        Sauna_Work [i] = t ;                            \
    }                                                   \
    else                                                \
    {                                                   \
        /* C(i,j) seen before, update it */             \
        Sauna_Work [i] = (Sauna_Work [i] || t) ;                     \
    }                                                   \
}

GrB_Info GB_AgusB__lor_ne_fp32
(
    GrB_Matrix C,
    const GrB_Matrix M,
    const GrB_Matrix A,
    const GrB_Matrix B,
    bool flipxy,                // if true, A and B have been swapped
    GB_Sauna Sauna,             // sparse accumulator
    GB_Context Context
)
{ 

    bool *restrict Sauna_Work = Sauna->Sauna_Work ;  // size C->vlen*zsize
    bool *restrict Cx = C->x ;
    GrB_Info info = GrB_SUCCESS ;

    #include "GB_AxB_Gustavson_flipxy.c"

    return (info) ;
}

//------------------------------------------------------------------------------
// C<M>=A'*B or C=A'*B: dot product
//------------------------------------------------------------------------------

// get A(k,i)
#define GB_DOT_GETA(pA)                                 \
    GB_atype aki = Ax [pA] ;

// get B(k,j)
#define GB_DOT_GETB(pB)                                 \
    GB_btype bkj = Bx [pB] ;

// t = aki*bkj
#define GB_DOT_MULT(bkj)                                \
    bool t ;                                        \
    GB_MULTIPLY (t, aki, bkj) ;

// cij += t
#define GB_DOT_ADD                                      \
    cij = (cij || t) ;

// cij = t
#define GB_DOT_COPY                                     \
    cij = t ;

// cij is not a pointer but a scalar; nothing to do
#define GB_DOT_REACQUIRE ;

// clear cij
#define GB_DOT_CLEAR                                    \
    cij = false ;

// save the value of C(i,j)
#define GB_DOT_SAVE                                     \
    Cx [cnz] = cij ;

#define GB_DOT_WORK_TYPE \
    GB_btype

#define GB_DOT_WORK(k) Work [k]

// Work [k] = Bx [pB]
#define GB_DOT_SCATTER \
    Work [k] = Bx [pB] ;

GrB_Info GB_AdotB__lor_ne_fp32
(
    GrB_Matrix *Chandle,
    const GrB_Matrix M,
    const GrB_Matrix A,
    const GrB_Matrix B,
    bool flipxy,                  // if true, A and B have been swapped
    GB_Context Context
)
{ 

    GrB_Matrix C = (*Chandle) ;
    bool *restrict Cx = C->x ;
    bool cij ;
    GrB_Info info = GrB_SUCCESS ;
    size_t bkj_size = B->type->size ;       // no typecasting here

    #include "GB_AxB_dot_flipxy.c"

    return (info) ;
}

//------------------------------------------------------------------------------
// C<M>=A*B and C=A*B: heap saxpy-based method
//------------------------------------------------------------------------------

#define GB_CIJ_GETB(pB)                                \
    GB_btype bkj = Bx [pB] ;

// C(i,j) = A(i,k) * bkj
#define GB_CIJ_MULT(pA)                                \
{                                                      \
    GB_atype aik = Ax [pA] ;                           \
    GB_MULTIPLY (cij, aik, bkj) ;                      \
}

// C(i,j) += A(i,k) * B(k,j)
#define GB_CIJ_MULTADD(pA,pB)                          \
{                                                      \
    GB_atype aik = Ax [pA] ;                           \
    GB_btype bkj = Bx [pB] ;                           \
    bool t ;                                       \
    GB_MULTIPLY (t, aik, bkj) ;                        \
    cij = (cij || t) ;                                   \
}

// cij is not a pointer but a scalar; nothing to do
#define GB_CIJ_REACQUIRE ;

// cij = identity
#define GB_CIJ_CLEAR                                   \
    cij = false ;

// save the value of C(i,j)
#define GB_CIJ_SAVE                                    \
    Cx [cnz] = cij ;

GrB_Info GB_AheapB__lor_ne_fp32
(
    GrB_Matrix *Chandle,
    const GrB_Matrix M,
    const GrB_Matrix A,
    const GrB_Matrix B,
    bool flipxy,                  // if true, A and B have been swapped
    int64_t *restrict List,
    GB_pointer_pair *restrict pA_pair,
    GB_Element *restrict Heap,
    const int64_t bjnz_max,
    GB_Context Context
)
{ 

    GrB_Matrix C = (*Chandle) ;
    bool *restrict Cx = C->x ;
    bool cij ;
    int64_t cvlen = C->vlen ;
    GrB_Info info = GrB_SUCCESS ;
    GB_CIJ_CLEAR ;

    #include "GB_AxB_heap_flipxy.c"

    return (info) ;
}

//------------------------------------------------------------------------------
// clear macro definitions
//------------------------------------------------------------------------------

#undef GB_XTYPE
#undef GB_YTYPE
#undef GB_HANDLE_FLIPXY
#undef GB_MULTOP
#undef GB_IDENTITY
#undef GB_COPY_SCALAR_TO_ARRAY
#undef GB_COPY_ARRAY_TO_SCALAR
#undef GB_COPY_ARRAY_TO_ARRAY
#undef GB_MULTADD_NOMASK
#undef GB_MULTADD_WITH_MASK
#undef GB_DOT_GETA
#undef GB_DOT_GETB
#undef GB_DOT_MULT
#undef GB_DOT_ADD
#undef GB_DOT_COPY
#undef GB_DOT_REACQUIRE
#undef GB_DOT_CLEAR
#undef GB_DOT_SAVE
#undef GB_DOT_WORK_TYPE
#undef GB_DOT_WORK
#undef GB_DOT_SCATTER
#undef GB_CIJ_GETB
#undef GB_CIJ_MULT
#undef GB_CIJ_MULTADD
#undef GB_CIJ_REACQUIRE
#undef GB_CIJ_CLEAR
#undef GB_CIJ_SAVE
#undef GB_MULTIPLY

#endif

