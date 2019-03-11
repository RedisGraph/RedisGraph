//------------------------------------------------------------------------------
// GB_AxB:  hard-coded C=A*B and C<M>=A*B
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Unless this file is Generator/GB_AxB.c, do not edit it (auto-generated)

#include "GB.h"
#ifndef GBCOMPACT
#include "GB_AxB__semirings.h"

// The C=A*B semiring is defined by the following types and operators:

// A*B function (Gustavon):  GB_AgusB__min_iseq_uint8
// A'*B function (dot):      GB_AdotB__min_iseq_uint8
// A*B function (heap):      GB_AheapB__min_iseq_uint8
// Z type:   uint8_t (the type of C)
// X type:   uint8_t (the type of x for z=mult(x,y))
// Y type:   uint8_t (the type of y for z=mult(x,y))
// handle flipxy: 0 (0 if mult(x,y) is commutative, 1 otherwise)
// Identity: UINT8_MAX (where cij = GB_IMIN (cij,identity) does not change cij)
// Multiply: z = x == y
// Add:      cij = GB_IMIN (cij,z)
// Terminal: if (cij == 0) break ;

#define GB_XTYPE \
    uint8_t
#define GB_YTYPE \
    uint8_t
#define GB_HANDLE_FLIPXY \
    0

#define GB_DOT_TERMINAL(cij) \
    if (cij == 0) break ;

#define GB_MULTOP(z,x,y) \
    z = x == y

//------------------------------------------------------------------------------
// C<M>=A*B and C=A*B: gather/scatter saxpy-based method (Gustavson)
//------------------------------------------------------------------------------

#define GB_IDENTITY \
    UINT8_MAX

// x [i] = y
#define GB_COPY_SCALAR_TO_ARRAY(x,i,y,s)    \
    x [i] = y ;

// x = y [i]
#define GB_COPY_ARRAY_TO_SCALAR(x,y,i,s)    \
    GB_btype x = y [i] ;

// x [i] = y [i]
#define GB_COPY_ARRAY_TO_ARRAY(x,i,y,j,s)   \
    x [i] = y [j] ;

// mult-add operation (no mask)
#define GB_MULTADD_NOMASK                   \
{                                           \
    /* Sauna_Work [i] += A(i,k) * B(k,j) */ \
    GB_atype aik = Ax [pA] ;                \
    uint8_t t ;                            \
    GB_MULTIPLY (t, aik, bkj) ;             \
    Sauna_Work [i] = GB_IMIN (Sauna_Work [i],t) ;             \
}

// mult-add operation (with mask)
#define GB_MULTADD_WITH_MASK                \
{                                           \
    /* Sauna_Work [i] += A(i,k) * B(k,j) */ \
    GB_atype aik = Ax [pA] ;                \
    uint8_t t ;                            \
    GB_MULTIPLY (t, aik, bkj) ;             \
    if (mark == hiwater)                    \
    {                                       \
        /* first time C(i,j) seen */        \
        Sauna_Mark [i] = hiwater + 1 ;      \
        Sauna_Work [i] = t ;                \
    }                                       \
    else                                    \
    {                                       \
        /* C(i,j) seen before, update it */ \
        Sauna_Work [i] = GB_IMIN (Sauna_Work [i],t) ;         \
    }                                       \
}

GrB_Info GB_AgusB__min_iseq_uint8
(
    GrB_Matrix C,
    const GrB_Matrix M,
    const GrB_Matrix A,
    const GrB_Matrix B,
    bool flipxy,
    GB_Sauna Sauna
)
{ 
    uint8_t *restrict Sauna_Work = Sauna->Sauna_Work ;
    uint8_t *restrict Cx = C->x ;
    GrB_Info info = GrB_SUCCESS ;
    #include "GB_AxB_Gustavson_flipxy.c"
    return (info) ;
}

//------------------------------------------------------------------------------
// C<M>=A'*B, C<!M>=A'*B or C=A'*B: dot product
//------------------------------------------------------------------------------

// get A(k,i)
#define GB_DOT_GETA(pA)        \
    GB_atype aki = Ax [pA] ;

// get B(k,j)
#define GB_DOT_GETB(pB)        \
    GB_btype bkj = Bx [pB] ;

// t = aki*bkj
#define GB_DOT_MULT(bkj)       \
    uint8_t t ;               \
    GB_MULTIPLY (t, aki, bkj) ;

// cij += t
#define GB_DOT_ADD             \
    cij = GB_IMIN (cij,t) ;

// cij = t
#define GB_DOT_COPY            \
    cij = t ;

// cij is not a pointer but a scalar; nothing to do
#define GB_DOT_REACQUIRE ;

// clear cij
#define GB_DOT_CLEAR           \
    cij = UINT8_MAX ;

// save the value of C(i,j)
#define GB_DOT_SAVE            \
    Cx [cnz] = cij ;

GrB_Info GB_AdotB__min_iseq_uint8
(
    GrB_Matrix *Chandle,
    const GrB_Matrix M,
    const bool Mask_comp,
    const GrB_Matrix A,
    const GrB_Matrix B,
    bool flipxy
)
{ 
    GrB_Matrix C = (*Chandle) ;
    uint8_t *restrict Cx = C->x ;
    uint8_t cij ;
    size_t bkj_size = B->type->size ;
    GrB_Info info = GrB_SUCCESS ;
    #include "GB_AxB_dot_flipxy.c"
    return (info) ;
}

//------------------------------------------------------------------------------
// C<M>=A*B and C=A*B: heap saxpy-based method
//------------------------------------------------------------------------------

#include "GB_heap.h"

#define GB_CIJ_GETB(pB) \
    GB_btype bkj = Bx [pB] ;

// C(i,j) = A(i,k) * bkj
#define GB_CIJ_MULT(pA)            \
{                                  \
    GB_atype aik = Ax [pA] ;       \
    GB_MULTIPLY (cij, aik, bkj) ;  \
}

// C(i,j) += A(i,k) * B(k,j)
#define GB_CIJ_MULTADD(pA,pB)      \
{                                  \
    GB_atype aik = Ax [pA] ;       \
    GB_btype bkj = Bx [pB] ;       \
    uint8_t t ;                   \
    GB_MULTIPLY (t, aik, bkj) ;    \
    cij = GB_IMIN (cij,t) ;               \
}

// cij is not a pointer but a scalar; nothing to do
#define GB_CIJ_REACQUIRE ;

// cij = identity
#define GB_CIJ_CLEAR \
    cij = UINT8_MAX ;

// save the value of C(i,j)
#define GB_CIJ_SAVE \
    Cx [cnz] = cij ;

GrB_Info GB_AheapB__min_iseq_uint8
(
    GrB_Matrix *Chandle,
    const GrB_Matrix M,
    const GrB_Matrix A,
    const GrB_Matrix B,
    bool flipxy,
    int64_t *restrict List,
    GB_pointer_pair *restrict pA_pair,
    GB_Element *restrict Heap,
    const int64_t bjnz_max
)
{ 
    GrB_Matrix C = (*Chandle) ;
    uint8_t *restrict Cx = C->x ;
    uint8_t cij ;
    int64_t cvlen = C->vlen ;
    GB_CIJ_CLEAR ;
    GrB_Info info = GrB_SUCCESS ;
    #include "GB_AxB_heap_flipxy.c"
    return (info) ;
}

#endif

