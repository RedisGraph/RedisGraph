//------------------------------------------------------------------------------
// gbextract: extract entries into a GraphBLAS matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// gbextract is an interface to GrB_Matrix_extract and GrB_Matrix_extract_[TYPE],
// computing the GraphBLAS expression:

//      C<#M,replace> = accum (C, A (I,J)) or
//      C<#M,replace> = accum (C, AT (I,J))

// Usage:

//      Cout = gbextract (Cin, M, accum, A, I, J, desc)

// A and desc are required.  See GrB.m for more details.
// If accum or M is used, then Cin must appear.

#include "gb_matlab.h"
#include "GB_ij.h"

#define USAGE "usage: Cout = GrB.extract (Cin, M, accum, A, I, J, desc)"

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

    gb_usage (nargin >= 2 && nargin <= 7 && nargout <= 1, USAGE) ;

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

    CHECK_ERROR (nmatrices < 1 || nmatrices > 3 || nstrings > 1, USAGE) ;

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
    // get the operator
    //--------------------------------------------------------------------------

    GrB_BinaryOp accum = NULL ;

    if (nstrings == 1)
    { 
        // if accum appears, then Cin must also appear
        CHECK_ERROR (C == NULL, USAGE) ;
        accum  = gb_mxstring_to_binop  (String [0], ctype) ;
    }

    //--------------------------------------------------------------------------
    // get the size of A
    //--------------------------------------------------------------------------

    GrB_Desc_Value in0 ;
    OK (GxB_Desc_get (desc, GrB_INP0, &in0)) ;
    GrB_Index anrows, ancols ;
    bool A_transpose = (in0 == GrB_TRAN) ;
    if (A_transpose)
    { 
        // T = AT (I,J) is to be extracted where AT = A'
        OK (GrB_Matrix_nrows (&ancols, A)) ;
        OK (GrB_Matrix_ncols (&anrows, A)) ;
    }
    else
    { 
        // T = A (I,J) is to be extracted
        OK (GrB_Matrix_nrows (&anrows, A)) ;
        OK (GrB_Matrix_ncols (&ancols, A)) ;
    }

    //--------------------------------------------------------------------------
    // get I and J
    //--------------------------------------------------------------------------

    GrB_Index *I = (GrB_Index *) GrB_ALL ;
    GrB_Index *J = (GrB_Index *) GrB_ALL ;
    GrB_Index ni = anrows, nj = ancols ;
    bool I_allocated = false, J_allocated = false ;

    if (anrows == 1 && ncells == 1)
    { 
        // only J is present
        J = gb_mxcell_to_index (Cell [0], base, ancols, &J_allocated, &nj) ;
    }
    else if (ncells == 1)
    { 
        // only I is present
        I = gb_mxcell_to_index (Cell [0], base, anrows, &I_allocated, &ni) ;
    }
    else if (ncells == 2)
    { 
        // both I and J are present
        I = gb_mxcell_to_index (Cell [0], base, anrows, &I_allocated, &ni) ;
        J = gb_mxcell_to_index (Cell [1], base, ancols, &J_allocated, &nj) ;
    }

    //--------------------------------------------------------------------------
    // construct C if not present on input
    //--------------------------------------------------------------------------

    if (C == NULL)
    { 
        // Cin is not present: determine its size, same type as A.
        // T = A(I,J) or AT(I,J) will be extracted.
        // accum must be null
        int I_kind, J_kind ;
        int64_t I_colon [3], J_colon [3] ;
        GrB_Index cnrows, cncols ;
        GB_ijlength (I, ni, anrows, &cnrows, &I_kind, I_colon) ;
        GB_ijlength (J, nj, ancols, &cncols, &J_kind, J_colon) ;
        ctype = atype ;

        OK (GrB_Matrix_new (&C, ctype, cnrows, cncols)) ;
        fmt = gb_get_format (cnrows, cncols, A, NULL, fmt) ;
        OK (GxB_Matrix_Option_set (C, GxB_FORMAT, fmt)) ;
    }

    //--------------------------------------------------------------------------
    // compute C<M> += A(I,J) or AT(I,J)
    //--------------------------------------------------------------------------

    OK (GrB_Matrix_extract (C, M, accum, A, I, ni, J, nj, desc)) ;

    //--------------------------------------------------------------------------
    // free shallow copies
    //--------------------------------------------------------------------------

    OK (GrB_Matrix_free (&M)) ;
    OK (GrB_Matrix_free (&A)) ;
    OK (GrB_Descriptor_free (&desc)) ;
    if (I_allocated) gb_mxfree (&I) ;
    if (J_allocated) gb_mxfree (&J) ;

    //--------------------------------------------------------------------------
    // export the output matrix C back to MATLAB
    //--------------------------------------------------------------------------

    pargout [0] = gb_export (&C, kind) ;
    GB_WRAPUP ;
}

