//------------------------------------------------------------------------------
// GB_mex_cast: cast a MATLAB array using C-style casting rules
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Usage: C = GB_mex_cast (X, classname) casts the dense array X to the MATLAB
// class given by classname, using C-style typecasting rules instead of
// MATLAB's rules.

#include "GB_mex.h"

#define USAGE "C = GB_mex_cast (X, classname, cover)"

void mexFunction
(
    int nargout,
    mxArray *pargout [ ],
    int nargin,
    const mxArray *pargin [ ]
)
{

    // do not get coverage counts unless the 3rd arg is present
    bool do_cover = (nargin == 3) ;
    bool malloc_debug = GB_mx_get_global (do_cover) ;

    // check inputs
    GB_WHERE (USAGE) ;
    if (nargout > 2 || nargin < 1 || nargin > 3)
    {
        mexErrMsgTxt ("Usage: " USAGE) ;
    }

    if (mxIsSparse (pargin [0]))
    {
        mexErrMsgTxt ("X must be dense") ;
    }

    // get X
    void *X ;
    int64_t nrows, ncols ;
    mxClassID xclass ;
    GrB_Type xtype ;
    GB_mx_mxArray_to_array (pargin [0], &X, &nrows, &ncols, &xclass, &xtype) ;
    if (xtype == NULL)
    {
        mexErrMsgTxt ("X must be numeric") ;
    }

    // get the type for C, default is same as X
    mxClassID cclass = GB_mx_string_to_classID (xclass, PARGIN (1)) ;
    GrB_Type ctype = GB_mx_classID_to_Type (cclass) ;
    if (ctype == NULL)
    {
        mexErrMsgTxt ("C must be numeric") ;
    }

    // create C
    if (xtype == Complex)
    {
        // ignore cclass, just copy the Complex X to the mxArray output
        pargout [0] = mxCreateNumericMatrix (nrows, ncols, mxDOUBLE_CLASS,
            mxCOMPLEX) ;
        GB_mx_complex_split (nrows*ncols, X, pargout [0]) ;
        // X is a deep copy that must be freed
        GB_FREE_MEMORY (X, nrows*ncols, sizeof (double complex)) ;
    }
    else
    {
        // typecast the shallow MATLAB X into the output C
        pargout [0] = mxCreateNumericMatrix (nrows, ncols, cclass, mxREAL) ;
        void *C = mxGetData (pargout [0]) ;

        // cast the data from X to C
        GB_cast_array (C, ctype->code, X, xtype->code, nrows*ncols) ;

        // X is a shallow copy that must not be freed
    }

    GB_mx_put_global (do_cover, 0) ;
}

