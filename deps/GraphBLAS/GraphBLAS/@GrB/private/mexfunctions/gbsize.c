//------------------------------------------------------------------------------
// gbsize: dimension and type of a GraphBLAS or MATLAB matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// The input may be either a GraphBLAS matrix struct or a standard MATLAB
// matrix.  Note that the output may be int64, to accomodate huge hypersparse
// matrices.  Also returns the type of the matrix.

// Usage:

// [m, n, type] = gbsize (X)

#include "gb_matlab.h"

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

    gb_usage (nargin == 1 && nargout <= 4, "usage: [m n type] = gbsize (X)") ;

    //--------------------------------------------------------------------------
    // get the # of rows and columns of a GraphBLAS or MATLAB matrix
    //--------------------------------------------------------------------------

    GrB_Index nrows, ncols ;
    int typecode = -1 ;

    if (mxIsStruct (pargin [0]))
    { 

        //----------------------------------------------------------------------
        // get the size of a GraphBLAS matrix
        //----------------------------------------------------------------------

        // get the type
        mxArray *mx_type = mxGetField (pargin [0], 0, "GraphBLASv4") ;
        CHECK_ERROR (mx_type == NULL, "invalid GraphBLASv4 struct") ;

        // get the scalar info
        mxArray *opaque = mxGetField (pargin [0], 0, "s") ;
        CHECK_ERROR (opaque == NULL, "invalid GraphBLASv4 struct") ;
        int64_t *s = mxGetInt64s (opaque) ;
        int64_t vlen = s [1] ;
        int64_t vdim = s [2] ;
        bool is_csc = (bool) (s [6]) ;

        nrows = (is_csc) ? vlen : vdim ;
        ncols = (is_csc) ? vdim : vlen ;

        //----------------------------------------------------------------------
        // return type of a GraphBLAS matrix, if requested
        //----------------------------------------------------------------------

        if (nargout > 2)
        { 
            // return the type
            pargout [2] = mxDuplicateArray (mx_type) ;
        }

    }
    else
    {

        //----------------------------------------------------------------------
        // get the size of a MATLAB matrix
        //----------------------------------------------------------------------

        nrows = (GrB_Index) mxGetM (pargin [0]) ;
        ncols = (GrB_Index) mxGetN (pargin [0]) ;

        //----------------------------------------------------------------------
        // get the type of a MATLAB matrix, if requested
        //----------------------------------------------------------------------

        if (nargout > 2)
        { 
            mxClassID class = mxGetClassID (pargin [0]) ;
            bool is_complex = mxIsComplex (pargin [0]) ;
            pargout [2] = gb_mxclass_to_mxstring (class, is_complex) ;
        }
    }

    //--------------------------------------------------------------------------
    // return the size as int64 or double
    //--------------------------------------------------------------------------

    if (nrows > FLINTMAX || ncols > FLINTMAX)
    { 
        // output is int64 to avoid flint overflow
        int64_t *p ;
        pargout [0] = mxCreateNumericMatrix (1, 1, mxINT64_CLASS, mxREAL) ;
        p = mxGetInt64s (pargout [0]) ;
        p [0] = (int64_t) nrows ;
        pargout [1] = mxCreateNumericMatrix (1, 1, mxINT64_CLASS, mxREAL) ;
        p = mxGetInt64s (pargout [1]) ;
        p [0] = (int64_t) ncols ;
    }
    else
    { 
        // output is double
        pargout [0] = mxCreateDoubleScalar ((double) nrows) ;
        pargout [1] = mxCreateDoubleScalar ((double) ncols) ;
    }

    GB_WRAPUP ;
}

