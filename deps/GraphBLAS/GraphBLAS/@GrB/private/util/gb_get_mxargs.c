//------------------------------------------------------------------------------
// gb_get_mxargs: get input arguments to a GraphBLAS mexFunction 
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// gb_get_mxargs collects all the input arguments for the 12 foundational
// GraphBLAS operations.  The user-level view is described below.  For
// the private mexFunctions, the descriptor always appears as the last
// argument.  The matrix arguments are either MATLAB sparse or full matrices,
// GraphBLAS matrices.  To call the mexFunction, the opaque content of the
// GraphBLAS matrices has been extracted, so they appear here as GraphBLAS
// structs.

#include "gb_matlab.h"

void gb_get_mxargs
(
    // input:
    int nargin,                 // # input arguments for mexFunction
    const mxArray *pargin [ ],  // input arguments for mexFunction
    const char *usage,          // usage to print, if too many args appear

    // output:
    const mxArray *Matrix [4],  // matrix arguments
    int *nmatrices,             // # of matrix arguments
    const mxArray *String [2],  // string arguments
    int *nstrings,              // # of string arguments
    const mxArray *Cell [2],    // cell array arguments
    int *ncells,                // # of cell array arguments
    GrB_Descriptor *desc,       // last argument is always the descriptor
    base_enum_t *base,          // desc.base
    kind_enum_t *kind,          // desc.kind
    GxB_Format_Value *fmt       // desc.format
)
{

    //--------------------------------------------------------------------------
    // get the descriptor
    //--------------------------------------------------------------------------

    (*desc) = gb_mxarray_to_descriptor (pargin [nargin-1], kind, fmt, base) ;

    //--------------------------------------------------------------------------
    // find the remaining arguments
    //--------------------------------------------------------------------------

    (*nmatrices) = 0 ;
    (*nstrings) = 0 ;
    (*ncells) = 0 ;

    for (int k = 0 ; k < (nargin-1) ; k++)
    {
        if (mxIsCell (pargin [k]))
        {
            // I or J index arguments
            if ((*ncells) >= 2)
            { 
                ERROR ("only 2D indexing is supported") ;
            }
            Cell [(*ncells)++] = pargin [k] ;
        }
        else if (mxIsChar (pargin [k]))
        {
            // accum operator, unary op, binary op, monoid, or semiring
            if ((*nstrings) >= 2)
            { 
                ERROR (usage) ;
            }
            String [(*nstrings)++] = pargin [k] ;
        }
        else
        {
            // a matrix argument is C, M, A, or B
            if ((*nmatrices) >= 4)
            { 
                // at most 4 matrix inputs are allowed
                ERROR (usage) ;
            }
            Matrix [(*nmatrices)++] = pargin [k] ;
        }
    }
}

