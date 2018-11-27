//------------------------------------------------------------------------------
// GB_mx_mxArray_to_array: get a dense numerical MATLAB array
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB_mex.h"

void GB_mx_mxArray_to_array    // convert mxArray to array
(
    const mxArray *Xmatlab,     // input MATLAB array
    // output:
    void **X,                   // pointer to numerical values
    int64_t *nrows,             // number of rows of X
    int64_t *ncols,             // number of columns of X
    mxClassID *xclass,          // MATLAB class of X
    GrB_Type *xtype             // GraphBLAS type of X, NULL if error
)
{

    *X = NULL ;
    *nrows = 0 ;
    *ncols = 0 ;
    *xclass = mxUNKNOWN_CLASS ;
    *xtype = NULL ;

    if (!(mxIsNumeric (Xmatlab) || mxIsLogical (Xmatlab)))
    {
        mexWarnMsgIdAndTxt ("GB:warn","input must be numeric or logical array");
    }
    if (mxIsSparse (Xmatlab))
    {
        mexWarnMsgIdAndTxt ("GB:warn","input cannot be sparse") ;
    }

    if (mxIsComplex (Xmatlab))
    {
        // user-defined Complex type
        // make a deep copy of the MATLAB complex dense matrix
        int64_t nel = mxGetNumberOfElements (Xmatlab) ;
        GB_MALLOC_MEMORY (double *XX, nel+1, sizeof (double complex)) ;
        GB_mx_complex_merge (nel, XX, Xmatlab) ;
        *X = XX ;
        *xclass = mxDOUBLE_CLASS ;
        *xtype = Complex ;
    }
    else
    {
        // shallow copy of the MATLAB dense matrix
        *X = mxGetData (Xmatlab) ;
        *xclass = mxGetClassID (Xmatlab) ;
        *xtype = GB_mx_classID_to_Type (*xclass) ;
    }
    *nrows = mxGetM (Xmatlab) ;
    *ncols = mxGetN (Xmatlab) ;
}

