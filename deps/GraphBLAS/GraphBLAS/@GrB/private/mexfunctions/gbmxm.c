//------------------------------------------------------------------------------
// gbmxm: sparse matrix-matrix multiplication
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// gbmxm is an interface to GrB_mxm.

// Usage:

// Cout = GrB.mxm (semiring, A, B, desc)
// Cout = GrB.mxm (Cin, accum, semiring, A, B, desc)
// Cout = GrB.mxm (Cin, M, semiring, A, B, desc)
// Cout = GrB.mxm (Cin, M, accum, semiring, A, B, desc)

// If Cin is not present then it is implicitly a matrix with no entries, of the
// right size (which depends on A, B, and the descriptor).

#include "gb_matlab.h"

#define USAGE "usage: Cout = GrB.mxm (Cin, M, accum, semiring, A, B, desc)"

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

    gb_usage ((nargin == 4 || nargin == 6 || nargin == 7) && nargout <= 1,
        USAGE) ;

    //--------------------------------------------------------------------------
    // find the arguments
    //--------------------------------------------------------------------------

    mxArray *Matrix [4], *String [2], *Cell [2] ;
    base_enum_t base ;
    kind_enum_t kind ;
    GxB_Format_Value fmt ;
    int nmatrices, nstrings, ncells ;
    GrB_Descriptor desc ;
    gb_get_mxargs (nargin, pargin, USAGE, Matrix, &nmatrices, String, &nstrings,
        Cell, &ncells, &desc, &base, &kind, &fmt) ;

    CHECK_ERROR (nmatrices < 2 || nstrings < 1 || ncells > 0, USAGE) ;

    //--------------------------------------------------------------------------
    // get the matrices
    //--------------------------------------------------------------------------

    GrB_Type atype, ctype = NULL ;
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
    if (C != NULL)
    { 
        OK (GxB_Matrix_type (&ctype, C)) ;
    }

    //--------------------------------------------------------------------------
    // get the operators
    //--------------------------------------------------------------------------

    GrB_BinaryOp accum = NULL ;
    GrB_Semiring semiring ;

    if (nstrings == 1)
    { 
        semiring = gb_mxstring_to_semiring (String [0], atype) ;
    }
    else 
    { 
        // if accum appears, then Cin must also appear
        CHECK_ERROR (C == NULL, USAGE) ;
        accum    = gb_mxstring_to_binop    (String [0], ctype) ;
        semiring = gb_mxstring_to_semiring (String [1], atype) ;
    }

    //--------------------------------------------------------------------------
    // construct C if not present on input
    //--------------------------------------------------------------------------

    // If C is NULL, then it is not present on input.
    // Construct C of the right size and type.

    if (C == NULL)
    { 
        // get the descriptor contents to determine if A and B are transposed
        GrB_Desc_Value in0, in1 ;
        OK (GxB_Desc_get (desc, GrB_INP0, &in0)) ;
        OK (GxB_Desc_get (desc, GrB_INP1, &in1)) ;
        bool A_transpose = (in0 == GrB_TRAN) ;
        bool B_transpose = (in1 == GrB_TRAN) ;

        // get the size of A and B
        GrB_Index anrows, ancols, bnrows, bncols ;
        OK (GrB_Matrix_nrows (&anrows, A)) ;
        OK (GrB_Matrix_ncols (&ancols, A)) ;
        OK (GrB_Matrix_nrows (&bnrows, B)) ;
        OK (GrB_Matrix_ncols (&bncols, B)) ;

        // determine the size of C
        GrB_Index cnrows = (A_transpose) ? ancols : anrows ;
        GrB_Index cncols = (B_transpose) ? bnrows : bncols ;

        // use the semiring's additive monoid as the type of C
        GrB_Monoid add_monoid ;
        GrB_BinaryOp add ;
        OK (GxB_Semiring_add (&add_monoid, semiring)) ;
        OK (GxB_Monoid_operator (&add, add_monoid)) ;
        OK (GxB_BinaryOp_ztype (&ctype, add)) ;

        OK (GrB_Matrix_new (&C, ctype, cnrows, cncols)) ;
        fmt = gb_get_format (cnrows, cncols, A, B, fmt) ;
        OK (GxB_Matrix_Option_set (C, GxB_FORMAT, fmt)) ;
    }

    //--------------------------------------------------------------------------
    // compute C<M> += A*B
    //--------------------------------------------------------------------------

    OK (GrB_mxm (C, M, accum, semiring, A, B, desc)) ;

    //--------------------------------------------------------------------------
    // free shallow copies
    //--------------------------------------------------------------------------

    OK (GrB_Matrix_free (&M)) ;
    OK (GrB_Matrix_free (&A)) ;
    OK (GrB_Matrix_free (&B)) ;
    OK (GrB_Descriptor_free (&desc)) ;

    //--------------------------------------------------------------------------
    // export the output matrix C back to MATLAB
    //--------------------------------------------------------------------------

    pargout [0] = gb_export (&C, kind) ;
    GB_WRAPUP ;
}

