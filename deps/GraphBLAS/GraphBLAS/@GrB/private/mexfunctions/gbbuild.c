//------------------------------------------------------------------------------
// gbbuild: build a GraphBLAS matrix or a MATLAB sparse matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// Usage:

// A = gbbuild (I, J, X)
// A = gbbuild (I, J, X, desc)
// A = gbbuild (I, J, X, m, desc)
// A = gbbuild (I, J, X, m, n, desc)
// A = gbbuild (I, J, X, m, n, dup, desc) ;
// A = gbbuild (I, J, X, m, n, dup, type, desc) ;

// m and n default to the largest index in I and J, respectively.

// dup is a string that defaults to 'plus.xtype' where xtype is the type of X.
// If dup is given by without a type,  type of dup defaults to the type of X.

// type is a string that defines is the type of A, which defaults to the type
// of X.

// The descriptor is optional; if present, it must be the last input parameter.
// desc.kind is the only part used from the descriptor, and it defaults to
// desc.kind = 'GrB'.

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

    gb_usage (nargin >= 3 && nargin <= 8 && nargout <= 2,
        "usage: A = GrB.build (I, J, X, m, n, dup, type, desc)") ;

    //--------------------------------------------------------------------------
    // get the descriptor
    //--------------------------------------------------------------------------

    base_enum_t base ;
    kind_enum_t kind ;
    GxB_Format_Value fmt ;
    int sparsity ;
    GrB_Descriptor desc = NULL ;
    desc = gb_mxarray_to_descriptor (pargin [nargin-1], &kind, &fmt,
        &sparsity, &base) ;

    // if present, remove the descriptor from consideration
    if (desc != NULL) nargin-- ;

    OK (GrB_Descriptor_free (&desc)) ;

    //--------------------------------------------------------------------------
    // get I and J
    //--------------------------------------------------------------------------

    GrB_Index ni, nj ;
    bool I_allocated, J_allocated ;
    int64_t Imax = -1, Jmax = -1 ;

    GrB_Index *I = (GrB_Index *) gb_mxarray_to_list (pargin [0], base,
        &I_allocated, (int64_t *) &ni, &Imax) ;

    GrB_Index *J = (GrB_Index *) gb_mxarray_to_list (pargin [1], base,
        &J_allocated, (int64_t *) &nj, &Jmax) ;

    //--------------------------------------------------------------------------
    // get X
    //--------------------------------------------------------------------------

    GrB_Type xtype = gb_mxarray_type (pargin [2]) ;
    GrB_Index nx = mxGetNumberOfElements (pargin [2]) ;

    //--------------------------------------------------------------------------
    // check the sizes of I, J, and X, and the type of X
    //--------------------------------------------------------------------------

    GrB_Index nvals = MAX (ni, nj) ;
    nvals = MAX (nvals, nx) ;

    if (!(ni == 1 || ni == nvals) ||
        !(nj == 1 || nj == nvals) ||
        !(nx == 1 || nx == nvals))
    { 
        ERROR ("I, J, and X must have the same length") ;
    }

    CHECK_ERROR (!(mxIsNumeric (pargin [2]) || mxIsLogical (pargin [2])),
        "X must be a numeric or logical array") ;
    CHECK_ERROR (mxIsSparse (pargin [2]), "X cannot be sparse") ;

    //--------------------------------------------------------------------------
    // expand any scalars
    //--------------------------------------------------------------------------

    if (ni == 1 && ni < nvals)
    { 
        GrB_Index *I2 = (GrB_Index *) mxMalloc (nvals * sizeof (GrB_Index)) ;
        GB_matlab_helper8 ((GB_void *) I2, (GB_void *) I, nvals,
            sizeof (GrB_Index)) ;
        if (I_allocated) gb_mxfree (&I) ;
        I_allocated = true ;
        I = I2 ;
    }

    if (nj == 1 && nj < nvals)
    { 
        GrB_Index *J2 = (GrB_Index *) mxMalloc (nvals * sizeof (GrB_Index)) ;
        GB_matlab_helper8 ((GB_void *) J2, (GB_void *) J, nvals,
            sizeof (GrB_Index)) ;
        if (J_allocated) gb_mxfree (&J) ;
        J_allocated = true ;
        J = J2 ;
    }

    //--------------------------------------------------------------------------
    // get m and n if present
    //--------------------------------------------------------------------------

    GrB_Index nrows = 0, ncols = 0 ;

    if (nargin < 4)
    {
        // nrows = max entry in I + 1
        if (Imax > -1)
        { 
            // Imax already computed
            nrows = Imax ;
        }
        else
        { 
            // nrows = max entry in I+1
            bool ok = GB_matlab_helper4 (I, ni, &nrows) ;
            CHECK_ERROR (!ok, "out of memory") ;
        }
    }
    else
    { 
        // m is provided on input
        CHECK_ERROR (!gb_mxarray_is_scalar (pargin [3]), "m must be a scalar") ;
        nrows = (GrB_Index) mxGetScalar (pargin [3]) ;
    }

    if (nargin < 5)
    {
        if (Jmax > -1)
        { 
            // Jmax already computed
            ncols = Jmax ;
        }
        else
        { 
            // ncols = max entry in J+1
            bool ok = GB_matlab_helper4 (J, nj, &ncols) ;
            CHECK_ERROR (!ok, "out of memory") ;
        }
    }
    else
    { 
        // n is provided on input
        CHECK_ERROR (!gb_mxarray_is_scalar (pargin [4]), "n must be a scalar") ;
        ncols = (GrB_Index) mxGetScalar (pargin [4]) ;
    }

    //--------------------------------------------------------------------------
    // get the dup operator
    //--------------------------------------------------------------------------

    GrB_BinaryOp dup = NULL ;
    if (nargin > 5)
    { 
        dup = gb_mxstring_to_binop (pargin [5], xtype, xtype) ;
    }

    // if dup is NULL, defaults to plus.xtype, below.

    //--------------------------------------------------------------------------
    // get the output matrix type
    //--------------------------------------------------------------------------

    GrB_Type type = NULL ;
    if (nargin > 6)
    { 
        type = gb_mxstring_to_type (pargin [6]) ;
        CHECK_ERROR (type == NULL, "unknown type") ;
    }
    else
    { 
        type = xtype ;
    }

    //--------------------------------------------------------------------------
    // build the matrix
    //--------------------------------------------------------------------------

    fmt = gb_get_format (nrows, ncols, NULL, NULL, fmt) ;
    sparsity = gb_get_sparsity (NULL, NULL, sparsity) ;
    GrB_Matrix A = gb_new (type, nrows, ncols, fmt, sparsity) ;

    // expandx is true if X must be expanded from a scalar to a vector
    void *X2 = NULL ;
    bool expandx = (nx == 1 && nx < nvals) ;

    if (xtype == GrB_BOOL)
    { 
        bool empty = 0 ;
        bool *X = (nvals == 0) ? &empty : mxGetData (pargin [2]) ;
        if (dup == NULL) dup = GrB_LOR ;
        if (expandx)
        { 
            X2 = mxMalloc (nvals * sizeof (bool)) ;
            GB_matlab_helper8 ((GB_void *) X2, (GB_void *) X, nvals,
                sizeof (bool)) ;
            X = (bool *) X2 ;
        }
        OK1 (A, GrB_Matrix_build_BOOL (A, I, J, X, nvals, dup)) ;
    }
    else if (xtype == GrB_INT8)
    { 
        int8_t empty = 0 ;
        int8_t *X = (nvals == 0) ? &empty : mxGetInt8s (pargin [2]) ;
        if (dup == NULL) dup = GrB_PLUS_INT8 ;
        if (expandx)
        { 
            X2 = mxMalloc (nvals * sizeof (int8_t)) ;
            GB_matlab_helper8 ((GB_void *) X2, (GB_void *) X, nvals,
                sizeof (int8_t)) ;
            X = (int8_t *) X2 ;
        }
        OK1 (A, GrB_Matrix_build_INT8 (A, I, J, X, nvals, dup)) ;
    }
    else if (xtype == GrB_INT16)
    { 
        int16_t empty = 0 ;
        int16_t *X = (nvals == 0) ? &empty : mxGetInt16s (pargin [2]) ;
        if (dup == NULL) dup = GrB_PLUS_INT16 ;
        if (expandx)
        { 
            X2 = mxMalloc (nvals * sizeof (int16_t)) ;
            GB_matlab_helper8 ((GB_void *) X2, (GB_void *) X, nvals,
                sizeof (int16_t)) ;
            X = (int16_t *) X2 ;
        }
        OK1 (A, GrB_Matrix_build_INT16 (A, I, J, X, nvals, dup)) ;
    }
    else if (xtype == GrB_INT32)
    { 
        int32_t empty = 0 ;
        int32_t *X = (nvals == 0) ? &empty : mxGetInt32s (pargin [2]) ;
        if (dup == NULL) dup = GrB_PLUS_INT32 ;
        if (expandx)
        { 
            X2 = mxMalloc (nvals * sizeof (int32_t)) ;
            GB_matlab_helper8 ((GB_void *) X2, (GB_void *) X, nvals,
                sizeof (int32_t)) ;
            X = (int32_t *) X2 ;
        }
        OK1 (A, GrB_Matrix_build_INT32 (A, I, J, X, nvals, dup)) ;
    }
    else if (xtype == GrB_INT64)
    { 
        int64_t empty = 0 ;
        int64_t *X = (nvals == 0) ? &empty : mxGetInt64s (pargin [2]) ;
        if (dup == NULL) dup = GrB_PLUS_INT64 ;
        if (expandx)
        { 
            X2 = mxMalloc (nvals * sizeof (int64_t)) ;
            GB_matlab_helper8 ((GB_void *) X2, (GB_void *) X, nvals,
                sizeof (int64_t)) ;
            X = (int64_t *) X2 ;
        }
        OK1 (A, GrB_Matrix_build_INT64 (A, I, J, X, nvals, dup)) ;
    }
    else if (xtype == GrB_UINT8)
    { 
        uint8_t empty = 0 ;
        uint8_t *X = (nvals == 0) ? &empty : mxGetUint8s (pargin [2]) ;
        if (dup == NULL) dup = GrB_PLUS_UINT8 ;
        if (expandx)
        { 
            X2 = mxMalloc (nvals * sizeof (uint8_t)) ;
            GB_matlab_helper8 ((GB_void *) X2, (GB_void *) X, nvals,
                sizeof (uint8_t)) ;
            X = (uint8_t *) X2 ;
        }
        OK1 (A, GrB_Matrix_build_UINT8 (A, I, J, X, nvals, dup)) ;
    }
    else if (xtype == GrB_UINT16)
    { 
        uint16_t empty = 0 ;
        uint16_t *X = (nvals == 0) ? &empty : mxGetUint16s (pargin [2]) ;
        if (dup == NULL) dup = GrB_PLUS_UINT16 ;
        if (expandx)
        { 
            X2 = mxMalloc (nvals * sizeof (uint16_t)) ;
            GB_matlab_helper8 ((GB_void *) X2, (GB_void *) X, nvals,
                sizeof (uint16_t)) ;
            X = (uint16_t *) X2 ;
        }
        OK1 (A, GrB_Matrix_build_UINT16 (A, I, J, X, nvals, dup)) ;
    }
    else if (xtype == GrB_UINT32)
    { 
        uint32_t empty = 0 ;
        uint32_t *X = (nvals == 0) ? &empty : mxGetUint32s (pargin [2]) ;
        if (dup == NULL) dup = GrB_PLUS_UINT32 ;
        if (expandx)
        { 
            X2 = mxMalloc (nvals * sizeof (uint32_t)) ;
            GB_matlab_helper8 ((GB_void *) X2, (GB_void *) X, nvals,
                sizeof (uint32_t)) ;
            X = (uint32_t *) X2 ;
        }
        OK1 (A, GrB_Matrix_build_UINT32 (A, I, J, X, nvals, dup)) ;
    }
    else if (xtype == GrB_UINT64)
    { 
        uint64_t empty = 0 ;
        uint64_t *X = (nvals == 0) ? &empty : mxGetUint64s (pargin [2]) ;
        if (dup == NULL) dup = GrB_PLUS_UINT64 ;
        if (expandx)
        { 
            X2 = mxMalloc (nvals * sizeof (uint64_t)) ;
            GB_matlab_helper8 ((GB_void *) X2, (GB_void *) X, nvals,
                sizeof (uint64_t)) ;
            X = (uint64_t *) X2 ;
        }
        OK1 (A, GrB_Matrix_build_UINT64 (A, I, J, X, nvals, dup)) ;
    }
    else if (xtype == GrB_FP32)
    { 
        float empty = 0 ;
        float *X = (nvals == 0) ? &empty : mxGetSingles (pargin [2]) ;
        if (dup == NULL) dup = GrB_PLUS_FP32 ;
        if (expandx)
        { 
            X2 = mxMalloc (nvals * sizeof (float)) ;
            GB_matlab_helper8 ((GB_void *) X2, (GB_void *) X, nvals,
                sizeof (float)) ;
            X = (float *) X2 ;
        }
        OK1 (A, GrB_Matrix_build_FP32 (A, I, J, X, nvals, dup)) ;
    }
    else if (xtype == GrB_FP64)
    { 
        double empty = 0 ;
        double *X = (nvals == 0) ? &empty : mxGetDoubles (pargin [2]) ;
        if (dup == NULL) dup = GrB_PLUS_FP64 ;
        if (expandx)
        { 
            X2 = mxMalloc (nvals * sizeof (double)) ;
            GB_matlab_helper8 ((GB_void *) X2, (GB_void *) X, nvals,
                sizeof (double)) ;
            X = (double *) X2 ;
        }
        OK1 (A, GrB_Matrix_build_FP64 (A, I, J, X, nvals, dup)) ;
    }
    else if (xtype == GxB_FC32)
    { 
        GxB_FC32_t empty = GxB_CMPLXF (0,0) ;
        GxB_FC32_t *X = &empty ;
        if (nvals > 0) X = (GxB_FC32_t *) mxGetComplexSingles (pargin [2]) ;
        if (dup == NULL) dup = GxB_PLUS_FC32 ;
        if (expandx)
        { 
            X2 = mxMalloc (nvals * sizeof (GxB_FC32_t)) ;
            GB_matlab_helper8 ((GB_void *) X2, (GB_void *) X, nvals,
                sizeof (GxB_FC32_t)) ;
            X = (GxB_FC32_t *) X2 ;
        }
        OK1 (A, GxB_Matrix_build_FC32 (A, I, J, X, nvals, dup)) ;
    }
    else if (xtype == GxB_FC64)
    { 
        GxB_FC64_t empty = GxB_CMPLX (0,0) ;
        GxB_FC64_t *X = &empty ;
        if (nvals > 0) X = (GxB_FC64_t *) mxGetComplexDoubles (pargin [2]) ;
        if (dup == NULL) dup = GxB_PLUS_FC64 ;
        if (expandx)
        { 
            X2 = mxMalloc (nvals * sizeof (GxB_FC64_t)) ;
            GB_matlab_helper8 ((GB_void *) X2, (GB_void *) X, nvals,
                sizeof (GxB_FC64_t)) ;
            X = (GxB_FC64_t *) X2 ;
        }
        OK1 (A, GxB_Matrix_build_FC64 (A, I, J, X, nvals, dup)) ;
    }
    else
    {
        ERROR ("unsupported type") ;
    }

    //--------------------------------------------------------------------------
    // free workspace
    //--------------------------------------------------------------------------

    if (X2 != NULL ) gb_mxfree (&X2) ;
    if (I_allocated) gb_mxfree (&I) ;
    if (J_allocated) gb_mxfree (&J) ;

    //--------------------------------------------------------------------------
    // export the output matrix A back to MATLAB
    //--------------------------------------------------------------------------

    pargout [0] = gb_export (&A, kind) ;
    pargout [1] = mxCreateDoubleScalar (kind) ;
    GB_WRAPUP ;
}

