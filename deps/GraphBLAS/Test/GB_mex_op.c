//------------------------------------------------------------------------------
// GB_mex_op: apply a built-in GraphBLAS operator to MATLAB arrays
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Usage:

// Z = GB_mex_op (op, X, Y)
// Z = GB_mex_op (op, X)

// Apply a built-in GraphBLAS operator or a user-defined Complex operator to
// one or two arrays X and Y of any MATLAB logical or numeric class.  X and Y
// are first typecasted into the x and y operand types of the op.  The output Z
// has the same class as the z type of the op.

#include "GB_mex.h"

#define USAGE "Z = GB_mex_op (opname, X, Y, cover)"

#define FREE_ALL                        \
{                                       \
    if (op_ztype == Complex) GB_FREE_MEMORY (Z, nx+1, sizeof (double complex));\
    if (X_type   == Complex) GB_FREE_MEMORY (X, nx+1, sizeof (double complex));\
    if (Y_type   == Complex) GB_FREE_MEMORY (Y, ny+1, sizeof (double complex));\
    GB_mx_put_global (do_cover, 0) ;    \
}

void mexFunction
(
    int nargout,
    mxArray *pargout [ ],
    int nargin,
    const mxArray *pargin [ ]
)
{

    void *X = NULL, *Y = NULL, *Z = NULL ;
    GrB_Type X_type = NULL, Y_type = NULL ;
    int64_t nrows = 0, ncols = 0, nx = 0, ny = 0, nrows2 = 0, ncols2 = 0 ;
    size_t Y_size = 1 ;

    bool do_cover = (nargin == 4) ;
    bool malloc_debug = GB_mx_get_global (do_cover) ;

    // if Y is char and cover present, treat as if nargin == 2
    if (do_cover)
    {
        if (mxIsChar (pargin [2]))
        {
            nargin = 2 ;
        }
    }

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GB_WHERE (USAGE) ;
    if (nargout > 1 || nargin < 2 || nargin > 4)
    {
        mexErrMsgTxt ("Usage: " USAGE) ;
    }

    //--------------------------------------------------------------------------
    // get op; default class is class(X)
    //--------------------------------------------------------------------------

    GrB_UnaryOp  op1 = NULL ;
    GrB_BinaryOp op2 = NULL ;
    GrB_Type op_ztype = NULL, op_xtype, op_ytype ;
    size_t   op_zsize, op_xsize, op_ysize ;

    // check for complex case
    bool XisComplex = mxIsComplex (pargin [1]) ;
    bool YisComplex = (nargin > 2) ? mxIsComplex (pargin [2]) : false ;

    if (nargin > 2)
    {
        // get a binary op
        if (!GB_mx_mxArray_to_BinaryOp (&op2, pargin [0], "GB_mex_op",
            GB_NOP_opcode, mxGetClassID (pargin [1]),
            XisComplex, YisComplex) || op2 == NULL)
        {
            FREE_ALL ;
            mexErrMsgTxt ("binary op missing") ;
        }
        op_ztype = op2->ztype ; op_zsize = op_ztype->size ;
        op_xtype = op2->xtype ; op_xsize = op_xtype->size ;
        op_ytype = op2->ytype ; op_ysize = op_ytype->size ;
        ASSERT_OK (GB_check (op2, "binary op", GB0)) ;
    }
    else
    {
        // get a unary op
        if (!GB_mx_mxArray_to_UnaryOp (&op1, pargin [0], "GB_mex_op",
            GB_NOP_opcode, mxGetClassID (pargin [1]),
            XisComplex) || op1 == NULL)
        {
            FREE_ALL ;
            mexErrMsgTxt ("unary op missing") ;
        }
        op_ztype = op1->ztype ; op_zsize = op_ztype->size ;
        op_xtype = op1->xtype ; op_xsize = op_xtype->size ;
        op_ytype = NULL       ; op_ysize = 1 ;
        ASSERT_OK (GB_check (op1, "unary op", GB0)) ;
    }

    ASSERT_OK (GB_check (op_ztype, "Z type", GB0)) ;

    //--------------------------------------------------------------------------
    // get X
    //--------------------------------------------------------------------------

    mxClassID xclass ;
    GB_mx_mxArray_to_array (pargin [1], &X, &nrows, &ncols, &xclass, &X_type) ;
    nx = nrows * ncols ;
    if (X_type == NULL)
    {
        FREE_ALL ;
        mexErrMsgTxt ("X must be numeric") ;
    }
    ASSERT_OK (GB_check (X_type, "X type", GB0)) ;
    size_t X_size = X_type->size ;

    if (!GB_Type_compatible (op_xtype, X_type))
    {
        FREE_ALL ;
        mexErrMsgTxt ("op xtype not compatible with X") ;
    }

    //--------------------------------------------------------------------------
    // get Y
    //--------------------------------------------------------------------------

    if (nargin > 2)
    {
        mxClassID yclass ;
        GB_mx_mxArray_to_array (pargin [2], &Y, &nrows2, &ncols2, &yclass,
            &Y_type) ;
        ny = nrows2 * ncols2 ;
        if (nrows2 != nrows || ncols2 != ncols)
        {
            FREE_ALL ;
            mexErrMsgTxt ("X and Y must be the same size") ;
        }
        if (Y_type == NULL)
        {
            FREE_ALL ;
            mexErrMsgTxt ("Y must be numeric") ;
        }
        ASSERT_OK (GB_check (Y_type, "Y type", GB0)) ;
        Y_size = Y_type->size ;

        if (!GB_Type_compatible (op_ytype, Y_type))
        {
            FREE_ALL ;
            mexErrMsgTxt ("op ytype not compatible with Y") ;
        }
    }

    //--------------------------------------------------------------------------
    // create Z of the same class as op_ztype
    //--------------------------------------------------------------------------

    if (op_ztype->code == GB_BOOL_code)
    {
        // Z is boolean, use a shallow copy of the MATLAB Z
        pargout [0] = mxCreateLogicalMatrix (nrows, ncols) ;
        Z = mxGetData (pargout [0]) ;
    }
    else if (op_ztype == Complex)
    {
        // Z is complex, create a temporary array
        GB_MALLOC_MEMORY (Z, nx + 1, sizeof (double complex)) ;
        // Z must be copied into the MATLAB pargout [0] when done, then freed
    }
    else
    {
        // Z is any other built-in type, use a shallow copy of the MATLAB Z
        mxClassID zclass = GB_mx_Type_to_classID (op_ztype) ;
        pargout [0] = mxCreateNumericMatrix (nrows, ncols, zclass, mxREAL) ;
        Z = mxGetData (pargout [0]) ;
    }

    //--------------------------------------------------------------------------
    // get scalar workspace
    //--------------------------------------------------------------------------

    char xwork [op_xsize] ;
    char ywork [op_ysize] ;

    GB_cast_function cast_X = GB_cast_factory (op_xtype->code, X_type->code) ;

    //--------------------------------------------------------------------------
    // do the op
    //--------------------------------------------------------------------------

    if (nargin > 2)
    {
        // Z = f (X,Y)
        GxB_binary_function f_binary = op2->function ;

        GB_cast_function cast_Y = GB_cast_factory (op_ytype->code,Y_type->code);
        for (int64_t k = 0 ; k < nx ; k++)
        {
            cast_X (xwork, X +(k*X_size), X_size) ;
            cast_Y (ywork, Y +(k*Y_size), Y_size) ;
            f_binary (Z +(k*op_zsize), xwork, ywork) ;
        }

    }
    else
    {
        // Z = f (X)
        GxB_unary_function f_unary = op1->function ;
        for (int64_t k = 0 ; k < nx ; k++)
        {
            cast_X (xwork, X +(k*X_size), X_size) ;
            f_unary (Z +(k*op_zsize), xwork) ;
        }
    }

    //--------------------------------------------------------------------------
    // copy the complex Z back to MATLAB
    //--------------------------------------------------------------------------

    if (op_ztype == Complex)
    {
        pargout [0] = mxCreateNumericMatrix (nrows, ncols,
            mxDOUBLE_CLASS, mxCOMPLEX) ;
        GB_mx_complex_split (nx, Z, pargout [0]) ;
    }

    //--------------------------------------------------------------------------
    // free workspace and return to MATLAB
    //--------------------------------------------------------------------------

    FREE_ALL ;
}

