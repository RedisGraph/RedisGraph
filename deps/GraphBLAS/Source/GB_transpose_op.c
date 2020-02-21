//------------------------------------------------------------------------------
// GB_transpose_op: transpose, typecast, and apply an operator to a matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// C = op ((xtype) A')

// The values of A are typecasted to op->xtype and then passed to the unary
// operator.  The output is assigned to R, which must be of type op->ztype; no
// output typecasting done with the output of the operator.

// This method is parallel, but not highly scalable.  It uses only naslice =
// nnz(A)/(A->vlen) threads.

#include "GB_transpose.h"
#ifndef GBCOMPACT
#include "GB_unaryop__include.h"
#endif

void GB_transpose_op    // transpose, typecast, and apply operator to a matrix
(
    GrB_Matrix C,                       // output matrix
    const GrB_UnaryOp op,               // operator to apply
    const GrB_Matrix A,                 // input matrix
    int64_t *GB_RESTRICT *Rowcounts,       // Rowcounts [naslice]
    GBI_single_iterator Iter,           // iterator for the matrix A
    const int64_t *GB_RESTRICT A_slice,    // defines how A is sliced
    int naslice                         // # of slices of A
)
{ 

    //--------------------------------------------------------------------------
    // define the worker for the switch factory
    //--------------------------------------------------------------------------

    GrB_Info info ;
    GrB_Type Atype = A->type ;

    #define GB_tran(opname,zname,aname) GB_tran_ ## opname ## zname ## aname

    #define GB_WORKER(opname,zname,ztype,aname,atype)                        \
    {                                                                        \
        info = GB_tran (opname,zname,aname) (C, A, Rowcounts, Iter, A_slice, \
                naslice) ;                                                   \
        if (info == GrB_SUCCESS) return ;                                    \
    }                                                                        \
    break ;

    //--------------------------------------------------------------------------
    // launch the switch factory
    //--------------------------------------------------------------------------

    #ifndef GBCOMPACT
    #include "GB_unaryop_factory.c"
    #endif

    //--------------------------------------------------------------------------
    // generic worker: transpose, typecast, and apply an operator
    //--------------------------------------------------------------------------

    GB_BURBLE_MATRIX (A, "generic ") ;

    size_t asize = Atype->size ;
    size_t zsize = op->ztype->size ;
    size_t xsize = op->xtype->size ;
    GB_cast_function
        cast_A_to_X = GB_cast_factory (op->xtype->code, Atype->code) ;
    GxB_unary_function fop = op->function ;

    // Cx [pC] = op (cast (Ax [pA]))
    #define GB_CAST_OP(pC,pA)                                       \
    {                                                               \
        /* xwork = (xtype) Ax [pA] */                               \
        GB_void xwork [GB_VLA(xsize)] ;                             \
        cast_A_to_X (xwork, Ax +(pA*asize), asize) ;                \
        /* Cx [pC] = fop (xwork) ; Cx is of type op->ztype */       \
        fop (Cx +(pC*zsize), xwork) ;                               \
    }

    #define GB_ATYPE GB_void
    #define GB_CTYPE GB_void

    #define GB_PHASE_2_OF_2
    #include "GB_unaryop_transpose.c"
}

