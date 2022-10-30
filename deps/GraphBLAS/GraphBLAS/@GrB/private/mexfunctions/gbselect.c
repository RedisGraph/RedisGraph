//------------------------------------------------------------------------------
// gbselect: select entries from a GraphBLAS matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// gbselect is an interface to GrB_Matrix_select and GxB_Matrix_select.

// Usage:

// C = gbselect (op, A)
// C = gbselect (op, A, desc)
// C = gbselect (op, A, b, desc)

// C = gbselect (Cin, accum, op, A, desc)
// C = gbselect (Cin, accum, op, A, b, desc)

// C = gbselect (Cin, M, op, A, desc)
// C = gbselect (Cin, M, op, A, b, desc)

// C = gbselect (Cin, M, accum, op, A, desc)
// C = gbselect (Cin, M, accum, op, A, b, desc)

// If Cin is not present then it is implicitly a matrix with no entries, of the
// right size (which depends on A, and the descriptor).  The type of Cin, if
// not present, is determined by the ztype of the accum, if present, or
// otherwise it has the same time as A.

// If op is '==' or '~=' and b is a NaN, and A has type GrB_FP32, GrB_FP64,
// GxB_FC32, or GxB_FC64, then a user-defined operator is used instead of
// GxB_EQ_THUNK, GxB_NE_THUNK, GrB_VALUEEQ* or GrB_VALUENE*.

// The 'tril', 'triu', 'diag', 'offdiag', and 2-input operators all require
// the b scalar.  The b scalar must not appear for the '*0' operators.

#include "gb_interface.h"

#define USAGE "usage: C = GrB.select (Cin, M, accum, op, A, b, desc)"

//------------------------------------------------------------------------------
// nan functions for GrB_IndexUnaryOp operators
//------------------------------------------------------------------------------

void gb_isnan32 (bool *z, const float *aij,
                 int64_t i, int64_t j, const void *thunk)
{ 
    (*z) = (isnan (*aij)) ;
}

void gb_isnan64 (bool *z, const double *aij,
                 int64_t i, int64_t j, const void *thunk)
{ 
    (*z) = (isnan (*aij)) ;
}

void gb_isnotnan32 (bool *z, const float *aij,
                    int64_t i, int64_t j, const void *thunk)
{ 
    (*z) = (!isnan (*aij)) ;
}

void gb_isnotnan64 (bool *z, const double *aij,
                    int64_t i, int64_t j, const void *thunk)
{ 
    (*z) = (!isnan (*aij)) ;
}

void gb_isnanfc32 (bool *z, const GxB_FC32_t *aij,
                   int64_t i, int64_t j, const void *thunk)
{ 
    (*z) = GB_cisnanf (*aij) ;
}

void gb_isnanfc64 (bool *z, const GxB_FC64_t *aij,
                   int64_t i, int64_t j, const void *thunk)
{ 
    (*z) = GB_cisnan (*aij) ;
}

void gb_isnotnanfc32 (bool *z, const GxB_FC32_t *aij,
                      int64_t i, int64_t j, const void *thunk)
{ 
    (*z) = !GB_cisnanf (*aij) ;
}

void gb_isnotnanfc64 (bool *z, const GxB_FC64_t *aij,
                      int64_t i, int64_t j, const void *thunk)
{ 
    (*z) = !GB_cisnan (*aij) ;
}

//------------------------------------------------------------------------------
// gbselect mexFunction
//------------------------------------------------------------------------------

void mexFunction
(
    int nargout,
    mxArray *pargout [ ],
    int nargin,
    const mxArray *pargin [ ]
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    gb_usage (nargin >= 2 && nargin <= 7 && nargout <= 2, USAGE) ;

    //--------------------------------------------------------------------------
    // find the arguments
    //--------------------------------------------------------------------------

    mxArray *Matrix [6], *String [2], *Cell [2] ;
    base_enum_t base ;
    kind_enum_t kind ;
    GxB_Format_Value fmt ;
    int nmatrices, nstrings, ncells, sparsity ;
    GrB_Descriptor desc ;
    gb_get_mxargs (nargin, pargin, USAGE, Matrix, &nmatrices, String, &nstrings,
        Cell, &ncells, &desc, &base, &kind, &fmt, &sparsity) ;

    CHECK_ERROR (nmatrices < 1 || nmatrices > 4 || nstrings < 1 || ncells > 0,
        USAGE) ;

    //--------------------------------------------------------------------------
    // get the select operator; determine the type and ithunk later
    //--------------------------------------------------------------------------

    int64_t ithunk = 0 ;
    GxB_SelectOp selop = NULL ;
    GrB_IndexUnaryOp idxunop = NULL ;
    bool thunk_required = false ; 
    bool op_is_positional = false ;

    gb_mxstring_to_selectop (&idxunop, &selop, &thunk_required,
        &op_is_positional, &ithunk, String [nstrings-1], NULL) ;

    //--------------------------------------------------------------------------
    // get the matrices
    //--------------------------------------------------------------------------

    GrB_Type atype, ctype = NULL ;
    GrB_Matrix C = NULL, M = NULL, A, b = NULL ;

    if (thunk_required)
    {
        if (nmatrices == 1)
        { 
            ERROR ("select operator input is missing") ;
        }
        else if (nmatrices == 2)
        { 
            A = gb_get_shallow (Matrix [0]) ;
            b = gb_get_shallow (Matrix [1]) ;
        }
        else if (nmatrices == 3)
        { 
            C = gb_get_deep    (Matrix [0]) ;
            A = gb_get_shallow (Matrix [1]) ;
            b = gb_get_shallow (Matrix [2]) ;
        }
        else // if (nmatrices == 4)
        { 
            C = gb_get_deep    (Matrix [0]) ;
            M = gb_get_shallow (Matrix [1]) ;
            A = gb_get_shallow (Matrix [2]) ;
            b = gb_get_shallow (Matrix [3]) ;
        }
    }
    else
    {
        if (nmatrices == 1)
        { 
            A = gb_get_shallow (Matrix [0]) ;
        }
        else if (nmatrices == 2)
        { 
            C = gb_get_deep    (Matrix [0]) ;
            A = gb_get_shallow (Matrix [1]) ;
        }
        else if (nmatrices == 3)
        { 
            C = gb_get_deep    (Matrix [0]) ;
            M = gb_get_shallow (Matrix [1]) ;
            A = gb_get_shallow (Matrix [2]) ;
        }
        else // if (nmatrices == 4)
        { 
            ERROR (USAGE) ;
        }
    }

    OK (GxB_Matrix_type (&atype, A)) ;
    if (C != NULL)
    { 
        OK (GxB_Matrix_type (&ctype, C)) ;
    }

    //--------------------------------------------------------------------------
    // finalize the select operator and ithunk
    //--------------------------------------------------------------------------

    ithunk = 0 ;
    GrB_Type btype = NULL ;
    if (b != NULL)
    {
        OK (GxB_Matrix_type (&btype, b)) ;
        if (op_is_positional)
        { 
            // get ithunk from the b scalar
            OK0 (GrB_Matrix_extractElement_INT64 (&ithunk, b, 0, 0)) ;
        }
    }

    gb_mxstring_to_selectop (&idxunop, &selop, &thunk_required,
        &op_is_positional, &ithunk, String [nstrings-1], atype) ;

    //--------------------------------------------------------------------------
    // get the accum operator
    //--------------------------------------------------------------------------

    GrB_BinaryOp accum = NULL ;
    if (nstrings > 1)
    { 
        // if accum appears, then Cin must also appear
        CHECK_ERROR (C == NULL, USAGE) ;
        accum = gb_mxstring_to_binop (String [0], ctype, ctype) ;
    }

    //--------------------------------------------------------------------------
    // construct C if not present on input
    //--------------------------------------------------------------------------

    // If C is NULL, then it is not present on input.
    // Construct C of the right size and type.

    if (C == NULL)
    { 
        // get the descriptor contents to determine if A is transposed
        GrB_Desc_Value in0 ;
        OK (GxB_Desc_get (desc, GrB_INP0, &in0)) ;
        bool A_transpose = (in0 == GrB_TRAN) ;

        // get the size of A
        GrB_Index anrows, ancols ;
        OK (GrB_Matrix_nrows (&anrows, A)) ;
        OK (GrB_Matrix_ncols (&ancols, A)) ;

        // determine the size of C
        GrB_Index cnrows = (A_transpose) ? ancols : anrows ;
        GrB_Index cncols = (A_transpose) ? anrows : ancols ;

        // C has the same type as A
        OK (GxB_Matrix_type (&ctype, A)) ;

        // create the matrix C and set its format and sparsity
        fmt = gb_get_format (cnrows, cncols, A, NULL, fmt) ;
        sparsity = gb_get_sparsity (A, NULL, sparsity) ;
        C = gb_new (ctype, cnrows, cncols, fmt, sparsity) ;
    }

    //--------------------------------------------------------------------------
    // handle the NaN case
    //--------------------------------------------------------------------------

    GrB_IndexUnaryOp nan_test = NULL ;
    GrB_Matrix b2 = b ;
    GrB_Matrix b3 = NULL, b4 = NULL ;

    if (op_is_positional)
    { 
        // construct a new int64 thunk scalar for positional ops
        OK (GrB_Matrix_new (&b3, GrB_INT64, 1, 1)) ;
        OK (GrB_Matrix_setElement_INT64 (b3, ithunk, 0, 0)) ;
        b2 = b3 ;
    }
    else if (b != NULL)
    {
        // check if b is NaN
        bool b_is_nan = false ;
        if (btype == GrB_FP32)
        { 
            float b_value = 0 ;
            OK0 (GrB_Matrix_extractElement_FP32 (&b_value, b, 0, 0)) ;
            b_is_nan = isnan (b_value) ;
        }
        else if (btype == GrB_FP64)
        { 
            double b_value = 0 ;
            OK0 (GrB_Matrix_extractElement_FP64 (&b_value, b, 0, 0)) ;
            b_is_nan = isnan (b_value) ;
        }
        else if (btype == GxB_FC32)
        { 
            GxB_FC32_t b_value = GxB_CMPLXF (0, 0) ;
            OK0 (GxB_Matrix_extractElement_FC32 (&b_value, b, 0, 0)) ;
            b_is_nan = GB_cisnanf (b_value) ;
        }
        else if (btype == GxB_FC64)
        { 
            GxB_FC64_t b_value = GxB_CMPLX (0, 0) ;
            OK0 (GxB_Matrix_extractElement_FC64 (&b_value, b, 0, 0)) ;
            b_is_nan = GB_cisnan (b_value) ;
        }

        if (b_is_nan)
        {
            // b is NaN; create a new nan_test operator if it should be used
            // instead of the built-in GxB_EQ_THUNK, GxB_NE_THUNK, GrB_VALUEEQ*
            // or GrB_VALUENE* operators.

            if (idxunop == GrB_VALUEEQ_FP32 ||
                selop == GxB_EQ_THUNK && atype == GrB_FP32)
            { 
                OK (GrB_IndexUnaryOp_new (&nan_test,
                    (GxB_index_unary_function) gb_isnan32,
                    GrB_BOOL, GrB_FP32, GrB_FP32)) ;
            }
            else if (idxunop == GrB_VALUEEQ_FP64 ||
                     selop == GxB_EQ_THUNK && atype == GrB_FP64)
            { 
                OK (GrB_IndexUnaryOp_new (&nan_test,
                    (GxB_index_unary_function) gb_isnan64,
                    GrB_BOOL, GrB_FP64, GrB_FP64)) ;
            }
            else if (idxunop == GxB_VALUEEQ_FC32 ||
                     selop == GxB_EQ_THUNK && atype == GxB_FC32)
            { 
                OK (GrB_IndexUnaryOp_new (&nan_test,
                    (GxB_index_unary_function) gb_isnanfc32,
                    GrB_BOOL, GxB_FC32, GxB_FC32)) ;
            }
            else if (idxunop == GxB_VALUEEQ_FC64 ||
                     selop == GxB_EQ_THUNK && atype == GxB_FC64)
            { 
                OK (GrB_IndexUnaryOp_new (&nan_test,
                    (GxB_index_unary_function) gb_isnanfc64,
                    GrB_BOOL, GxB_FC64, GxB_FC64)) ;
            }
            else if (idxunop == GrB_VALUENE_FP32 ||
                     selop == GxB_NE_THUNK && atype == GrB_FP32)
            { 
                OK (GrB_IndexUnaryOp_new (&nan_test,
                    (GxB_index_unary_function) gb_isnotnan32,
                    GrB_BOOL, GrB_FP32, GrB_FP32)) ;
            }
            else if (idxunop == GrB_VALUENE_FP64 ||
                     selop == GxB_NE_THUNK && atype == GrB_FP64)
            { 
                OK (GrB_IndexUnaryOp_new (&nan_test,
                    (GxB_index_unary_function) gb_isnotnan64,
                    GrB_BOOL, GrB_FP64, GrB_FP64)) ;
            }
            else if (idxunop == GxB_VALUENE_FC32 ||
                     selop == GxB_NE_THUNK && atype == GxB_FC32)
            { 
                OK (GrB_IndexUnaryOp_new (&nan_test,
                    (GxB_index_unary_function) gb_isnotnanfc32,
                    GrB_BOOL, GxB_FC32, GxB_FC32)) ;
            }
            else if (idxunop == GxB_VALUENE_FC64 ||
                     selop == GxB_NE_THUNK && atype == GxB_FC64)
            { 
                OK (GrB_IndexUnaryOp_new (&nan_test,
                    (GxB_index_unary_function) gb_isnotnanfc64,
                    GrB_BOOL, GxB_FC64, GxB_FC64)) ;
            }
        }

        if (nan_test != NULL)
        { 
            // use the new operator instead of the built-in one
            selop = NULL ;
            idxunop = nan_test ;
        }
    }

    //--------------------------------------------------------------------------
    // compute C<M> += select (A, b2)
    //--------------------------------------------------------------------------

    if (selop != NULL)
    { 
        OK1 (C, GxB_Matrix_select (C, M, accum, selop, A,
            (GrB_Scalar) b2, desc)) ;
    }
    else
    { 
        // typecast the b2 scalar to the idxunop->ytype
        GrB_Type ytype ;
        char ytype_name [GxB_MAX_NAME_LEN] ;
        OK (GxB_IndexUnaryOp_ytype_name (ytype_name, idxunop)) ;
        OK (GxB_Type_from_name (&ytype, ytype_name)) ;
        OK (GrB_Matrix_new (&b4, ytype, 1, 1)) ;
        OK (GrB_Matrix_assign (b4, NULL, NULL, b2, GrB_ALL, 1, GrB_ALL, 1,
            NULL)) ;
        OK1 (C, GrB_Matrix_select_Scalar (C, M, accum, idxunop, A,
            (GrB_Scalar) b4, desc)) ;
    }

    //--------------------------------------------------------------------------
    // free shallow copies
    //--------------------------------------------------------------------------

    OK (GrB_Matrix_free (&M)) ;
    OK (GrB_Matrix_free (&A)) ;
    OK (GrB_Matrix_free (&b)) ;
    OK (GrB_Matrix_free (&b3)) ;
    OK (GrB_Matrix_free (&b4)) ;
    OK (GrB_Descriptor_free (&desc)) ;
    OK (GrB_IndexUnaryOp_free (&nan_test)) ;

    //--------------------------------------------------------------------------
    // export the output matrix C
    //--------------------------------------------------------------------------

    pargout [0] = gb_export (&C, kind) ;
    pargout [1] = mxCreateDoubleScalar (kind) ;
    GB_WRAPUP ;
}

