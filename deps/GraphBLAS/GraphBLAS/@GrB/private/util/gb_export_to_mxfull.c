//------------------------------------------------------------------------------
// gb_export_to_mxfull: export a full array to a MATLAB full matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// The input (void *) X is exported to a MATLAB full mxArray S.

// The input array must be deep, but this cannot be checked here.  The caller
// must ensure that the input X is deep.  The output is a standard MATLAB full
// matrix as an mxArray.  No typecasting is done.

#include "gb_matlab.h"

mxArray *gb_export_to_mxfull    // return exported MATLAB full matrix F
(
    void **X_handle,            // pointer to array to export
    const GrB_Index nrows,      // dimensions of F
    const GrB_Index ncols,
    GrB_Type type               // type of the array
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    CHECK_ERROR (X_handle == NULL, "internal error 3") ;
    CHECK_ERROR (type == NULL, "internal error 11") ;

    //--------------------------------------------------------------------------
    // allocate an empty full matrix of the right type, then set content
    //--------------------------------------------------------------------------

    mxArray *F ;
    void *X = (*X_handle) ;
    if (X == NULL)
    {
        // A GrB_Matrix C has a null C->x array, if C has no entries.  Since
        // C has already been expanded to a full matrix, C->x can be NULL
        // only if nrows or ncols is zero.
        CHECK_ERROR (nrows > 0 && ncols > 0, "internal error 12") ;
        X = mxMalloc (2 * sizeof (double)) ;
    }

    if (type == GrB_BOOL)
    { 
        F = mxCreateLogicalMatrix (0, 0) ;
        mxSetData (F, X) ;
    }
    else if (type == GrB_FP32)
    { 
        F = mxCreateNumericMatrix (0, 0, mxSINGLE_CLASS, mxREAL) ;
        mxSetSingles (F, X) ;
    }
    else if (type == GrB_FP64)
    { 
        F = mxCreateNumericMatrix (0, 0, mxDOUBLE_CLASS, mxREAL) ;
        mxSetDoubles (F, X) ;
    }
    else if (type == GrB_INT8)
    { 
        F = mxCreateNumericMatrix (0, 0, mxINT8_CLASS, mxREAL) ;
        mxSetInt8s (F, X) ;
    }
    else if (type == GrB_INT16)
    { 
        F = mxCreateNumericMatrix (0, 0, mxINT16_CLASS, mxREAL) ;
        mxSetInt16s (F, X) ;
    }
    else if (type == GrB_INT32)
    { 
        F = mxCreateNumericMatrix (0, 0, mxINT32_CLASS, mxREAL) ;
        mxSetInt32s (F, X) ;
    }
    else if (type == GrB_INT64)
    { 
        F = mxCreateNumericMatrix (0, 0, mxINT64_CLASS, mxREAL) ;
        mxSetInt64s (F, X) ;
    }
    else if (type == GrB_UINT8)
    { 
        F = mxCreateNumericMatrix (0, 0, mxUINT8_CLASS, mxREAL) ;
        mxSetUint8s (F, X) ;
    }
    else if (type == GrB_UINT16)
    { 
        F = mxCreateNumericMatrix (0, 0, mxUINT16_CLASS, mxREAL) ;
        mxSetUint16s (F, X) ;
    }
    else if (type == GrB_UINT32)
    { 
        F = mxCreateNumericMatrix (0, 0, mxUINT32_CLASS, mxREAL) ;
        mxSetUint32s (F, X) ;
    }
    else if (type == GrB_UINT64)
    { 
        F = mxCreateNumericMatrix (0, 0, mxUINT64_CLASS, mxREAL) ;
        mxSetUint64s (F, X) ;
    }
    else if (type == GxB_FC32)
    {
        F = mxCreateNumericMatrix (0, 0, mxSINGLE_CLASS, mxCOMPLEX) ;
        mxSetComplexSingles (F, X) ;
    }
    else if (type == GxB_FC64)
    {
        F = mxCreateNumericMatrix (0, 0, mxDOUBLE_CLASS, mxCOMPLEX) ;
        mxSetComplexDoubles (F, X) ;
    }
    else
    {
        ERROR ("unsupported type") ;
    }

    // set the size
    mxSetM (F, nrows) ;
    mxSetN (F, ncols) ;

    // tell the caller that X is exported
    (*X_handle) = NULL ;

    //--------------------------------------------------------------------------
    // return the new MATLAB full matrix
    //--------------------------------------------------------------------------

    return (F) ;
}

