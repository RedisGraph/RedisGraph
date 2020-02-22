//------------------------------------------------------------------------------
// gbextracttuples: extract all entries from a GraphBLAS matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Usage:

// [I J X] = GrB.extracttuples (A, desc)

// desc.base = 'zero-based':    I and J are returned as 0-based int64 indices
// desc.base = 'one-based int': I and J are returned as 1-based int64 indices
// desc.base = 'one-based':     I and J are returned as 1-based double indices
// desc.base = 'default':       'one-based', unless max(size(A)) > flintmax,
//                              in which case 'one-based int' is used.

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

    gb_usage (nargin >= 1 && nargin <= 2 && nargout <= 3,
        "usage: [I,J,X] = GrB.extracttuples (A, desc)") ;

    //--------------------------------------------------------------------------
    // get the optional descriptor
    //--------------------------------------------------------------------------

    base_enum_t base = BASE_DEFAULT ;
    kind_enum_t kind = KIND_GRB ;
    GxB_Format_Value fmt = GxB_NO_FORMAT ;
    GrB_Descriptor desc = NULL ;
    if (nargin == 2)
    { 
        desc = gb_mxarray_to_descriptor (pargin [1], &kind, &fmt, &base) ;
    }
    OK (GrB_Descriptor_free (&desc)) ;

    //--------------------------------------------------------------------------
    // get the matrix
    //--------------------------------------------------------------------------

    GrB_Matrix A = gb_get_shallow (pargin [0]) ;
    GrB_Index nvals ;
    OK (GrB_Matrix_nvals (&nvals, A)) ;
    GrB_Type xtype ;
    OK (GxB_Matrix_type (&xtype, A)) ;

    //--------------------------------------------------------------------------
    // determine what to extract
    //--------------------------------------------------------------------------

    bool extract_I = true ;
    bool extract_J = (nargout > 1) ;
    bool extract_X = (nargout > 2) ;

    //--------------------------------------------------------------------------
    // allocate I and J
    //--------------------------------------------------------------------------

    GrB_Index s = MAX (nvals, 1) ;
    GrB_Index *I = extract_I ? mxMalloc (s * sizeof (GrB_Index)) : NULL ;
    GrB_Index *J = extract_J ? mxMalloc (s * sizeof (GrB_Index)) : NULL ;

    //--------------------------------------------------------------------------
    // extract the tuples and export X
    //--------------------------------------------------------------------------

    if (xtype == GrB_BOOL)
    {
        bool *X = extract_X ? mxMalloc (s * sizeof (bool)) : NULL ;
        OK (GrB_Matrix_extractTuples_BOOL (I, J, X, &nvals, A)) ;
        if (extract_X)
        { 
            pargout [2] = gb_export_to_mxfull (&X, nvals, 1, GrB_BOOL) ;
        }
    }
    else if (xtype == GrB_INT8)
    {
        int8_t *X = extract_X ? mxMalloc (s * sizeof (int8_t)) : NULL ;
        OK (GrB_Matrix_extractTuples_INT8 (I, J, X, &nvals, A)) ;
        if (extract_X)
        { 
            pargout [2] = gb_export_to_mxfull (&X, nvals, 1, GrB_INT8) ;
        }
    }
    else if (xtype == GrB_INT16)
    {
        int16_t *X = extract_X ? mxMalloc (s * sizeof (int16_t)) : NULL ;
        OK (GrB_Matrix_extractTuples_INT16 (I, J, X, &nvals, A)) ;
        if (extract_X)
        { 
            pargout [2] = gb_export_to_mxfull (&X, nvals, 1, GrB_INT16) ;
        }
    }
    else if (xtype == GrB_INT32)
    {
        int32_t *X = extract_X ? mxMalloc (s * sizeof (int32_t)) : NULL ;
        OK (GrB_Matrix_extractTuples_INT32 (I, J, X, &nvals, A)) ;
        if (extract_X)
        { 
            pargout [2] = gb_export_to_mxfull (&X, nvals, 1, GrB_INT32) ;
        }
    }
    else if (xtype == GrB_INT64)
    {
        int64_t *X = extract_X ? mxMalloc (s * sizeof (int64_t)) : NULL ;
        OK (GrB_Matrix_extractTuples_INT64 (I, J, X, &nvals, A)) ;
        if (extract_X)
        { 
            pargout [2] = gb_export_to_mxfull (&X, nvals, 1, GrB_INT64) ;
        }
    }
    else if (xtype == GrB_UINT8)
    {
        uint8_t *X = extract_X ? mxMalloc (s * sizeof (uint8_t)) : NULL ;
        OK (GrB_Matrix_extractTuples_UINT8 (I, J, X, &nvals, A)) ;
        if (extract_X)
        { 
            pargout [2] = gb_export_to_mxfull (&X, nvals, 1, GrB_UINT8) ;
        }
    }
    else if (xtype == GrB_UINT16)
    {
        uint16_t *X = extract_X ? mxMalloc (s * sizeof (uint16_t)) : NULL ;
        OK (GrB_Matrix_extractTuples_UINT16 (I, J, X, &nvals, A)) ;
        if (extract_X)
        { 
            pargout [2] = gb_export_to_mxfull (&X, nvals, 1, GrB_UINT16) ;
        }
    }
    else if (xtype == GrB_UINT32)
    {
        uint32_t *X = extract_X ? mxMalloc (s * sizeof (uint32_t)) : NULL ;
        OK (GrB_Matrix_extractTuples_UINT32 (I, J, X, &nvals, A)) ;
        if (extract_X)
        { 
            pargout [2] = gb_export_to_mxfull (&X, nvals, 1, GrB_UINT32) ;
        }
    }
    else if (xtype == GrB_UINT64)
    {
        uint64_t *X = extract_X ? mxMalloc (s * sizeof (uint64_t)) : NULL ;
        OK (GrB_Matrix_extractTuples_UINT64 (I, J, X, &nvals, A)) ;
        if (extract_X)
        { 
            pargout [2] = gb_export_to_mxfull (&X, nvals, 1, GrB_UINT64) ;
        }
    }
    else if (xtype == GrB_FP32)
    {
        float *X = extract_X ? mxMalloc (s * sizeof (float)) : NULL ;
        OK (GrB_Matrix_extractTuples_FP32 (I, J, X, &nvals, A)) ;
        if (extract_X)
        { 
            pargout [2] = gb_export_to_mxfull (&X, nvals, 1, GrB_FP32) ;
        }
    }
    else if (xtype == GrB_FP64)
    {
        double *X = extract_X ? mxMalloc (s * sizeof (double)) : NULL ;
        OK (GrB_Matrix_extractTuples_FP64 (I, J, X, &nvals, A)) ;
        if (extract_X)
        { 
            pargout [2] = gb_export_to_mxfull (&X, nvals, 1, GrB_FP64) ;
        }
    }
    #ifdef GB_COMPLEX_TYPE
    else if (xtype == gb_complex_type)
    {
        double *X = extract_X ? mxMalloc (s * sizeof (double complex)) : NULL ;
        OK (GrB_Matrix_extractTuples_UDT (I, J, X, &nvals, A)) ;
        if (extract_X)
        {
            pargout [2] = gb_export_to_mxfull (&X, nvals, 1, gb_complex_type) ;
        }
    }
    #endif
    else
    {
        ERROR ("unsupported type") ;
    }

    //--------------------------------------------------------------------------
    // determine if zero-based or one-based
    //--------------------------------------------------------------------------

    if (base == BASE_DEFAULT)
    {
        GrB_Index nrows, ncols ;
        OK (GrB_Matrix_nrows (&nrows, A)) ;
        OK (GrB_Matrix_ncols (&ncols, A)) ;
        if (MAX (nrows, ncols) > FLINTMAX)
        {
            // the matrix is too large for I and J to be returned as double
            base = BASE_1_INT64 ;
        }
        else
        {
            // this is the typical case
            base = BASE_1_DOUBLE ;
        }
    }

    //--------------------------------------------------------------------------
    // free workspace
    //--------------------------------------------------------------------------

    OK (GrB_Matrix_free (&A)) ;

    //--------------------------------------------------------------------------
    // export I and J
    //--------------------------------------------------------------------------

    if (base == BASE_0_INT64)
    { 

        //----------------------------------------------------------------------
        // export I and J in their native zero-based integer format
        //----------------------------------------------------------------------

        if (extract_I)
        { 
            pargout [0] = gb_export_to_mxfull (&I, nvals, 1, GrB_INT64) ;
        }

        if (extract_J)
        { 
            pargout [1] = gb_export_to_mxfull (&J, nvals, 1, GrB_INT64) ;
        }

    }
    else if (base == BASE_1_DOUBLE)
    { 

        //----------------------------------------------------------------------
        // export I and J as double one-based integers
        //----------------------------------------------------------------------

        if (extract_I)
        { 
            double *I_double = mxMalloc (s * sizeof (double)) ;
            GB_matlab_helper1 (I_double, I, (int64_t) nvals) ;
            gb_mxfree (&I) ;
            pargout [0] = gb_export_to_mxfull (&I_double, nvals, 1, GrB_FP64) ;
        }

        if (extract_J)
        { 
            double *J_double = mxMalloc (s * sizeof (double)) ;
            GB_matlab_helper1 (J_double, J, (int64_t) nvals) ;
            gb_mxfree (&J) ;
            pargout [1] = gb_export_to_mxfull (&J_double, nvals, 1, GrB_FP64) ;
        }

    }
    else if (base == BASE_1_INT64)
    { 

        //----------------------------------------------------------------------
        // export I and J as int64 one-based integers
        //----------------------------------------------------------------------

        if (extract_I)
        { 
            GB_matlab_helper1i ((int64_t *) I, (int64_t) nvals) ;
            pargout [0] = gb_export_to_mxfull (&I, nvals, 1, GrB_INT64) ;
        }

        if (extract_J)
        { 
            GB_matlab_helper1i ((int64_t *) J, (int64_t) nvals) ;
            pargout [1] = gb_export_to_mxfull (&J, nvals, 1, GrB_INT64) ;
        }
    }

    GB_WRAPUP ;
}

