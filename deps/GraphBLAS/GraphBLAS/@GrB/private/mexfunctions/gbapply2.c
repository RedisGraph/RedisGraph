//------------------------------------------------------------------------------
// gbapply2: apply a binary operator to a matrix, with scalar binding
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// gbapply2 is an interface to GxB_Matrix_apply_BinaryOp1st.
// and GxB_Matrix_apply_Binaryop2nd.

// Usage:

// C = gbapply2 (binop, A, B)
// C = gbapply2 (binop, A, B, desc)
// C = gbapply2 (Cin, accum, binop, A, B, desc)
// C = gbapply2 (Cin, M, binop, A, B, desc)
// C = gbapply2 (Cin, M, accum, binop, A, B, desc)

// Either A or B (or both) must be a scalar (1-by-1, with 0 or 1 entries).
// If the scalar has no entry, it is treated as the value zero.

// If Cin is not present then it is implicitly a matrix with no entries, of the
// right size (which depends on A, B, and the descriptor).

#include "gb_matlab.h"

#define USAGE "usage: C = GrB.apply2 (Cin, M, accum, binop, A, B, desc)"

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

    gb_usage (nargin >= 3 && nargin <= 7 && nargout <= 2, USAGE) ;

    //--------------------------------------------------------------------------
    // find the arguments
    //--------------------------------------------------------------------------

    mxArray *Matrix [4], *String [2], *Cell [2] ;
    base_enum_t base ;
    kind_enum_t kind ;
    GxB_Format_Value fmt ;
    int nmatrices, nstrings, ncells, sparsity ;
    GrB_Descriptor desc ;
    gb_get_mxargs (nargin, pargin, USAGE, Matrix, &nmatrices, String, &nstrings,
        Cell, &ncells, &desc, &base, &kind, &fmt, &sparsity) ;

    CHECK_ERROR (nmatrices < 2 || nstrings < 1 || ncells > 0, USAGE) ;

    //--------------------------------------------------------------------------
    // get the matrices
    //--------------------------------------------------------------------------

    GrB_Type atype, btype, ctype = NULL ;
    GrB_Matrix C = NULL, M = NULL, A, B ;

    if (nmatrices == 2)
    { 
        A = gb_get_shallow (Matrix [0]) ;
        B = gb_get_shallow (Matrix [1]) ;
    }
    else if (nmatrices == 3)
    { 
        C = gb_get_deep    (Matrix [0]) ;
        A = gb_get_shallow (Matrix [1]) ;
        B = gb_get_shallow (Matrix [2]) ;
    }
    else // if (nmatrices == 4)
    { 
        C = gb_get_deep    (Matrix [0]) ;
        M = gb_get_shallow (Matrix [1]) ;
        A = gb_get_shallow (Matrix [2]) ;
        B = gb_get_shallow (Matrix [3]) ;
    }

    OK (GxB_Matrix_type (&atype, A)) ;
    OK (GxB_Matrix_type (&btype, B)) ;
    if (C != NULL)
    { 
        OK (GxB_Matrix_type (&ctype, C)) ;
    }

    //--------------------------------------------------------------------------
    // determine which input is the scalar and which is the matrix
    //--------------------------------------------------------------------------

    GrB_Index anrows, ancols, bnrows, bncols ;

    // get the size of A and B
    OK (GrB_Matrix_nrows (&anrows, A)) ;
    OK (GrB_Matrix_ncols (&ancols, A)) ;
    OK (GrB_Matrix_nrows (&bnrows, B)) ;
    OK (GrB_Matrix_ncols (&bncols, B)) ;

    GxB_Scalar scalar = NULL, scalar0 = NULL ;
    bool binop_bind1st ;
    if (anrows == 1 && ancols == 1)
    {
        // A is the scalar and B is the matrix
        binop_bind1st = true ;
        scalar = (GxB_Scalar) A ;   // NOTE: this is not allowed by the spec
    }
    else if (bnrows == 1 && bncols == 1)
    {
        // A is the matrix and B is the scalar
        binop_bind1st = false ;
        scalar = (GxB_Scalar) B ;   // NOTE: this is not allowed by the spec
    }
    else
    {
        ERROR ("either A or B must be a scalar") ;
    }

    //--------------------------------------------------------------------------
    // make sure the scalar has one entry
    //--------------------------------------------------------------------------

    GrB_Index nvals ;
    OK (GxB_Scalar_nvals (&nvals, scalar)) ;
    if (nvals == 0)
    {
        // GxB_apply requires at least one entry.  Create a new scalar zero.
        OK (GxB_Scalar_dup (&scalar0, scalar)) ;
        // the scalar need not be int32; this will typecast as needed
        OK (GxB_Scalar_setElement_INT32 (scalar0, 0)) ;
        OK (GxB_Scalar_wait (&scalar0)) ;
        scalar = scalar0 ;
    }

    //--------------------------------------------------------------------------
    // get the operators
    //--------------------------------------------------------------------------

    GrB_BinaryOp accum = NULL, op = NULL ;

    if (nstrings == 1)
    { 
        op    = gb_mxstring_to_binop (String [0], atype, btype) ;
    }
    else 
    { 
        // if accum appears, then Cin must also appear
        CHECK_ERROR (C == NULL, USAGE) ;
        accum = gb_mxstring_to_binop (String [0], ctype, ctype) ;
        op    = gb_mxstring_to_binop (String [1], atype, btype) ;
    }

    //--------------------------------------------------------------------------
    // construct C if not present on input
    //--------------------------------------------------------------------------

    // If C is NULL, then it is not present on input.
    // Construct C of the right size and type.

    if (C == NULL)
    { 
        // get the descriptor to determine if the input matrix is transposed
        GrB_Index cnrows, cncols ;
        if (binop_bind1st)
        {
            // A is the scalar and B is the matrix
            GrB_Desc_Value in1 ;
            OK (GxB_Desc_get (desc, GrB_INP0, &in1)) ;
            bool B_transpose = (in1 == GrB_TRAN) ;
            // determine the size of C
            cnrows = (B_transpose) ? bncols : bnrows ;
            cncols = (B_transpose) ? bnrows : bncols ;
        }
        else
        {
            // A is the matrix and B is the scalar
            GrB_Desc_Value in0 ;
            OK (GxB_Desc_get (desc, GrB_INP0, &in0)) ;
            bool A_transpose = (in0 == GrB_TRAN) ;
            // determine the size of C
            cnrows = (A_transpose) ? ancols : anrows ;
            cncols = (A_transpose) ? anrows : ancols ;
        }

        // use the ztype of the op as the type of C
        OK (GxB_BinaryOp_ztype (&ctype, op)) ;

        // create the matrix C and set its format and sparsity
        fmt = gb_get_format (cnrows, cncols, A, B, fmt) ;
        sparsity = gb_get_sparsity (A, B, sparsity) ;
        C = gb_new (ctype, cnrows, cncols, fmt, sparsity) ;
    }

    //--------------------------------------------------------------------------
    // compute C<M> += op (A,B) where one input is a scalar
    //--------------------------------------------------------------------------

    if (binop_bind1st)
    {
        OK1 (C, GxB_Matrix_apply_BinaryOp1st (C, M, accum, op, scalar, B,
            desc)) ;
    }
    else
    {
        OK1 (C, GxB_Matrix_apply_BinaryOp2nd (C, M, accum, op, A, scalar,
            desc)) ;
    }

    //--------------------------------------------------------------------------
    // free shallow copies
    //--------------------------------------------------------------------------

    OK (GrB_Matrix_free (&M)) ;
    OK (GrB_Matrix_free (&A)) ;
    OK (GrB_Matrix_free (&B)) ;
    OK (GrB_Matrix_free (&scalar0)) ;
    OK (GrB_Descriptor_free (&desc)) ;

    //--------------------------------------------------------------------------
    // export the output matrix C back to MATLAB
    //--------------------------------------------------------------------------

    pargout [0] = gb_export (&C, kind) ;
    pargout [1] = mxCreateDoubleScalar (kind) ;
    GB_WRAPUP ;
}

