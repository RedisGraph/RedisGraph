//------------------------------------------------------------------------------
// gb_assign: assign entries into a GraphBLAS matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// With do_subassign false, gb_assign is an interface to GrB_Matrix_assign and
// GrB_Matrix_assign_[TYPE], computing the GraphBLAS expression:

//      C<#M,replace>(I,J) = accum (C(I,J), A) or accum(C(I,J), A')

// With do_subassign true, gb_assign is an interface to GxB_Matrix_subassign
// and GxB_Matrix_subassign_[TYPE], computing the GraphBLAS expression:

//      C(I,J)<#M,replace> = accum (C(I,J), A) or accum(C(I,J), A')

// A can be a matrix or a scalar.  If it is a scalar with nnz (A) == 0,
// then it is first expanded to an empty matrix of size length(I)-by-length(J),
// and G*B_Matrix_*assign is used (not GraphBLAS scalar assignment).

// MATLAB Usage:

//      Cout = GrB.assign    (Cin, M, accum, A, I, J, desc)
//      Cout = GrB.subassign (Cin, M, accum, A, I, J, desc)

// Cin, A, and desc are required.  See GrB.m for more details.

#include "gb_matlab.h"

void gb_assign                  // gbassign or gbsubassign mexFunctions
(
    int nargout,                // # output arguments for mexFunction
    mxArray *pargout [ ],       // output arguments for mexFunction
    int nargin,                 // # input arguments for mexFunction
    const mxArray *pargin [ ],  // input arguments for mexFunction
    bool do_subassign,          // true: do subassign, false: do assign
    const char *usage           // usage string to print if error
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    gb_usage (nargin >= 3 && nargin <= 7 && nargout <= 1, usage) ;

    //--------------------------------------------------------------------------
    // find the arguments
    //--------------------------------------------------------------------------

    mxArray *Matrix [4], *String [2], *Cell [2] ;
    base_enum_t base ;
    kind_enum_t kind ;
    GxB_Format_Value fmt ;
    int nmatrices, nstrings, ncells ;
    GrB_Descriptor desc ;
    gb_get_mxargs (nargin, pargin, usage, Matrix, &nmatrices, String, &nstrings,
        Cell, &ncells, &desc, &base, &kind, &fmt) ;

    CHECK_ERROR (nmatrices < 2 || nmatrices > 3 || nstrings > 1, usage) ;

    //--------------------------------------------------------------------------
    // get the matrices
    //--------------------------------------------------------------------------

    GrB_Type atype, ctype ;
    GrB_Matrix C, M = NULL, A ;

    if (nmatrices == 2)
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
    OK (GxB_Matrix_type (&ctype, C)) ;

    //--------------------------------------------------------------------------
    // get the operator
    //--------------------------------------------------------------------------

    GrB_BinaryOp accum = NULL ;

    if (nstrings == 1)
    { 
        accum = gb_mxstring_to_binop (String [0], ctype) ;
    }

    //--------------------------------------------------------------------------
    // get the size of Cin
    //--------------------------------------------------------------------------

    GrB_Index cnrows, cncols ;
    OK (GrB_Matrix_nrows (&cnrows, C)) ;
    OK (GrB_Matrix_ncols (&cncols, C)) ;

    //--------------------------------------------------------------------------
    // get I and J
    //--------------------------------------------------------------------------

    GrB_Index *I = (GrB_Index *) GrB_ALL ;
    GrB_Index *J = (GrB_Index *) GrB_ALL ;
    GrB_Index ni = cnrows, nj = cncols ;
    bool I_allocated = false, J_allocated = false ;

    if (cnrows == 1 && ncells == 1)
    { 
        // only J is present
        J = gb_mxcell_to_index (Cell [0], base, cncols, &J_allocated, &nj) ;
    }
    else if (ncells == 1)
    { 
        // only I is present
        I = gb_mxcell_to_index (Cell [0], base, cnrows, &I_allocated, &ni) ;
    }
    else if (ncells == 2)
    { 
        // both I and J are present
        I = gb_mxcell_to_index (Cell [0], base, cnrows, &I_allocated, &ni) ;
        J = gb_mxcell_to_index (Cell [1], base, cncols, &J_allocated, &nj) ;
    }

    //--------------------------------------------------------------------------
    // determine if A is a scalar (ignore the transpose descriptor)
    //--------------------------------------------------------------------------

    GrB_Index anrows, ancols, anvals ;
    OK (GrB_Matrix_nrows (&anrows, A)) ;
    OK (GrB_Matrix_ncols (&ancols, A)) ;
    OK (GrB_Matrix_nvals (&anvals, A)) ;
    bool scalar_assignment = (anrows == 1) && (ancols == 1) ;

    if (scalar_assignment && anvals == 0)
    { 
        // A is a sparse scalar.  Expand it to an ni-by-nj sparse matrix with
        // the same type as C, with no entries, and use matrix assignment.
        OK (GrB_Matrix_free (&A)) ;
        OK (GrB_Matrix_new (&A, ctype, ni, nj)) ;
        OK (GxB_Matrix_Option_get (C, GxB_FORMAT, &fmt)) ;
        OK (GxB_Matrix_Option_set (A, GxB_FORMAT, fmt)) ;
        scalar_assignment = false ;
    }

    //--------------------------------------------------------------------------
    // compute C(I,J)<M> += A or C<M>(I,J) += A
    //--------------------------------------------------------------------------

    if (scalar_assignment)
    { 
        gb_matrix_assign_scalar (C, M, accum, A, I, ni, J, nj, desc,
            do_subassign) ;
    }
    else
    {
        if (do_subassign)
        { 
            OK (GxB_Matrix_subassign (C, M, accum, A, I, ni, J, nj, desc)) ;
        }
        else
        { 
            OK (GrB_Matrix_assign (C, M, accum, A, I, ni, J, nj, desc)) ;
        }
    }

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

