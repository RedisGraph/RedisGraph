//------------------------------------------------------------------------------
// gbvreduce: reduce a matrix to a vector
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// gbvreduce is an interface to GrB_Matrix_reduce.

// Usage:

//  Cout = GrB.vreduce (op, A, desc)
//  Cout = GrB.vreduce (Cin, M, op, A, desc)
//  Cout = GrB.vreduce (Cin, accum, op, A, desc)
//  Cout = GrB.vreduce (Cin, M, accum, op, A, desc)

// If Cin is not present then it is implicitly a matrix with no entries, of the
// right size (which depends on A and the descriptor).

#include "gb_matlab.h"

#define USAGE "usage: Cout = GrB.vreduce (Cin, M, accum, op, A, desc)"

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

    gb_usage ((nargin == 3 || nargin == 5 || nargin == 6) && nargout <= 1,
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

    CHECK_ERROR (nmatrices < 1 || nmatrices > 3 || nstrings < 1 || ncells > 0,
        USAGE) ;

    //--------------------------------------------------------------------------
    // get the matrices
    //--------------------------------------------------------------------------

    GrB_Type atype, ctype = NULL ;
    GrB_Matrix C = NULL, M = NULL, A ;

    if (nmatrices == 1)
    { 
        A = gb_get_shallow (Matrix [0]) ;
    }
    else if (nmatrices == 2)
    { 
        C = gb_get_deep    (Matrix [0]) ;
        A = gb_get_shallow (Matrix [1]) ;
    }
    else // if (nmatrices == 3)
    { 
        C = gb_get_deep    (Matrix [0]) ;
        M = gb_get_shallow (Matrix [1]) ;
        A = gb_get_shallow (Matrix [2]) ;
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
    GrB_Monoid monoid ;

    if (nstrings == 1)
    { 
        monoid = gb_mxstring_to_monoid (String [0], atype) ;
    }
    else 
    { 
        // if accum appears, then Cin must also appear
        CHECK_ERROR (C == NULL, USAGE) ;
        accum  = gb_mxstring_to_binop  (String [0], ctype) ;
        monoid = gb_mxstring_to_monoid (String [1], atype) ;
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

        // determine the size of the vector C
        GrB_Index cnrows = (A_transpose) ? ancols : anrows ;

        // use the ztype of the monoid as the type of C
        GrB_BinaryOp binop ;
        OK (GxB_Monoid_operator (&binop, monoid)) ;
        OK (GxB_BinaryOp_ztype (&ctype, binop)) ;

        OK (GrB_Matrix_new (&C, ctype, cnrows, 1)) ;
        fmt = gb_get_format (cnrows, 1, A, NULL, fmt) ;
        OK (GxB_Matrix_Option_set (C, GxB_FORMAT, fmt)) ;
    }

    //--------------------------------------------------------------------------
    // compute C<M> += reduce(A)
    //--------------------------------------------------------------------------

    OK (GrB_Matrix_reduce_Monoid (C, M, accum, monoid, A, desc)) ;

    //--------------------------------------------------------------------------
    // free shallow copies
    //--------------------------------------------------------------------------

    OK (GrB_Matrix_free (&M)) ;
    OK (GrB_Matrix_free (&A)) ;
    OK (GrB_Descriptor_free (&desc)) ;

    //--------------------------------------------------------------------------
    // export the output matrix C back to MATLAB
    //--------------------------------------------------------------------------

    pargout [0] = gb_export (&C, kind) ;
    GB_WRAPUP ;
}

