//------------------------------------------------------------------------------
// GB_mex_about9: still more basic tests (not for Windows)
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// Windows is limited to user-defined types of size 128 or less.

#include "GB_mex.h"
#include "GB_mex_errors.h"

#define USAGE "GB_mex_about9"
#define FREE_ALL ;
#define GET_DEEP_COPY ;
#define FREE_DEEP_COPY ;

typedef struct
{
    double stuff [32] ;
}
bigtype ;

#define F1                                                              \
"void f1 (void *z, const void *x) "                                     \
"{ (*((double *)z)) = 2*(*(double *)x) ; } "
 void f1 (void *z, const void *x)
 { (*((double *)z)) = 2*(*(double *)x) ; }

#define F2                                                              \
"void f2 (void *z, const void *x, const void *y) "                      \
"{ (*((double *)z)) = 2*(*(double *)x) + 1 ; }   "
 void f2 (void *z, const void *x, const void *y)
 { (*((double *)z)) = 2*(*(double *)x) + 1 ; }

#define I1                                                          \
"void i1 (void *z, const void *x, GrB_Index i, GrB_Index j, "       \
" const void *thunk) "                                              \
"{ (*((bool *)z)) = (i == j) ; }"
 void i1 (void *z, const void *x, GrB_Index i, GrB_Index j,
  const void *thunk)
 { (*((bool *)z)) = (i == j) ; }

#define GET_DEEP_COPY ;
#define FREE_DEEP_COPY ;

void mexFunction
(
    int nargout,
    mxArray *pargout [ ],
    int nargin,
    const mxArray *pargin [ ]
)
{

    GrB_Info info ;

    //--------------------------------------------------------------------------
    // startup GraphBLAS
    //--------------------------------------------------------------------------

    bool malloc_debug = GB_mx_get_global (true) ;

    //--------------------------------------------------------------------------
    // user-defined type of 256 bytes
    //--------------------------------------------------------------------------

    GrB_Type BigType ;
    METHOD (GxB_Type_new (&BigType, sizeof (bigtype), "bigtype",
        "typedef struct { double stuff [32] ; } bigtype")) ;
    OK (GxB_Type_fprint (BigType, "(256-byte big type)", GxB_COMPLETE,
        stdout)) ;
    OK (GrB_Type_free (&BigType)) ;

    //--------------------------------------------------------------------------
    // user-defined operators
    //--------------------------------------------------------------------------

    GrB_UnaryOp op1 = NULL ;
    METHOD (GxB_UnaryOp_new (&op1, &f1, GrB_FP64, GrB_FP64, "f1", F1)) ;
    OK (GxB_UnaryOp_fprint (op1, "f1", GxB_COMPLETE, stdout)) ;
    OK (GrB_UnaryOp_free (&op1)) ;

    GrB_BinaryOp op2 = NULL ;
    METHOD (GxB_BinaryOp_new (&op2, &f2, GrB_FP64, GrB_FP64, GrB_FP64,
        "f2", F2)) ;
    OK (GxB_BinaryOp_fprint (op2, "f2", GxB_COMPLETE, stdout)) ;
    OK (GrB_BinaryOp_free (&op2)) ;

    GrB_IndexUnaryOp opi = NULL ;
    METHOD (GxB_IndexUnaryOp_new (&opi, &i1, GrB_FP64, GrB_FP64, GrB_FP64,
        "i1", I1)) ;
    OK (GxB_IndexUnaryOp_fprint (opi, "i1", GxB_COMPLETE, stdout)) ;
    OK (GrB_IndexUnaryOp_free (&opi)) ;

    //--------------------------------------------------------------------------
    // wrapup
    //--------------------------------------------------------------------------

    GB_mx_put_global (true) ;
    printf ("\nGB_mex_about9: all tests passed\n\n") ;
}

