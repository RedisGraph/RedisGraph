//------------------------------------------------------------------------------
// GB_mex_extractTuples: extract all tuples from a matrix or vector
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB_mex.h"

#define USAGE "[I,J,X] = GB_mex_extractTuples (A, xclass)"

#define FREE_ALL                        \
{                                       \
    GB_MATRIX_FREE (&A) ;               \
    GB_FREE_MEMORY (Xtemp, nvals, sizeof (double complex)) ; \
    GB_mx_put_global (true, 0) ;        \
}

void mexFunction
(
    int nargout,
    mxArray *pargout [ ],
    int nargin,
    const mxArray *pargin [ ]
)
{

    bool malloc_debug = GB_mx_get_global (true) ;
    GrB_Matrix A = NULL ;
    void *Y = NULL ;
    void *Xtemp = NULL ;
    void *X = NULL ;
    GrB_Index nvals = 0 ;

    // check inputs
    GB_WHERE (USAGE) ;
    if (nargout > 3 || nargin < 1 || nargin > 2)
    {
        mexErrMsgTxt ("Usage: " USAGE) ;
    }

    #define GET_DEEP_COPY ;
    #define FREE_DEEP_COPY ;

    // get A (shallow copy)
    A = GB_mx_mxArray_to_Matrix (pargin [0], "A input", false, true) ;
    if (A == NULL)
    {
        FREE_ALL ;
        mexErrMsgTxt ("A failed") ;
    }
    mxClassID aclass = GB_mx_Type_to_classID (A->type) ;

    // get the number of entries in A
    GrB_Matrix_nvals (&nvals, A) ;

    mxClassID xclass ;
    GrB_Type xtype ;

    if (A->type == Complex)
    {
        // input argument xclass is ignored
        xtype = Complex ;
        xclass = mxDOUBLE_CLASS ;
        // create Xtemp
        if (nargout > 2)
        {
            GB_MALLOC_MEMORY (Xtemp, nvals, sizeof (double complex)) ;
        }
    }
    else
    {
        // get xclass, default is class (A), and the corresponding xtype
        xclass = GB_mx_string_to_classID (aclass, PARGIN (1)) ;
        xtype = GB_mx_classID_to_Type (xclass) ;
        if (xtype == NULL)
        {
            FREE_ALL ;
            mexErrMsgTxt ("X must be numeric") ;
        }
        // create X
        if (nargout > 2)
        {
            pargout [2] = mxCreateNumericMatrix (nvals, 1, xclass, mxREAL) ;
            X = (void *) mxGetData (pargout [2]) ;
        }
    }

    // create I
    pargout [0] = mxCreateNumericMatrix (nvals, 1, mxUINT64_CLASS, mxREAL) ;
    GrB_Index *I = (GrB_Index *) mxGetData (pargout [0]) ;

    // create J
    GrB_Index *J = NULL ;
    if (nargout > 1)
    {
        pargout [1] = mxCreateNumericMatrix (nvals, 1, mxUINT64_CLASS, mxREAL) ;
        J = (GrB_Index *) mxGetData (pargout [1]) ;
    }

    // [I,J,X] = find (A)
    if (GB_VECTOR_OK (A))
    {
        // test extract vector methods
        GrB_Vector v = (GrB_Vector) A ;
        switch (xtype->code)
        {
            case GB_BOOL_code   : METHOD (GrB_Vector_extractTuples (I, (bool     *) X, &nvals, v)) ; break ;
            case GB_INT8_code   : METHOD (GrB_Vector_extractTuples (I, (int8_t   *) X, &nvals, v)) ; break ;
            case GB_UINT8_code  : METHOD (GrB_Vector_extractTuples (I, (uint8_t  *) X, &nvals, v)) ; break ;
            case GB_INT16_code  : METHOD (GrB_Vector_extractTuples (I, (int16_t  *) X, &nvals, v)) ; break ;
            case GB_UINT16_code : METHOD (GrB_Vector_extractTuples (I, (uint16_t *) X, &nvals, v)) ; break ;
            case GB_INT32_code  : METHOD (GrB_Vector_extractTuples (I, (int32_t  *) X, &nvals, v)) ; break ;
            case GB_UINT32_code : METHOD (GrB_Vector_extractTuples (I, (uint32_t *) X, &nvals, v)) ; break ;
            case GB_INT64_code  : METHOD (GrB_Vector_extractTuples (I, (int64_t  *) X, &nvals, v)) ; break ;
            case GB_UINT64_code : METHOD (GrB_Vector_extractTuples (I, (uint64_t *) X, &nvals, v)) ; break ;
            case GB_FP32_code   : METHOD (GrB_Vector_extractTuples (I, (float    *) X, &nvals, v)) ; break ;
            case GB_FP64_code   : METHOD (GrB_Vector_extractTuples (I, (double   *) X, &nvals, v)) ; break ;
            case GB_UCT_code    : 
            case GB_UDT_code    : 
              METHOD (GrB_Vector_extractTuples (I, Xtemp, &nvals, v)) ; break ;
            default             : FREE_ALL ; mexErrMsgTxt ("unsupported class") ;
        }
        if (J != NULL)
        {
            for (int64_t p = 0 ; p < nvals ; p++) J [p] = 0 ;
        }
    }
    else
    {
        switch (xtype->code)
        {
            case GB_BOOL_code   : METHOD (GrB_Matrix_extractTuples (I, J, (bool     *) X, &nvals, A)) ; break ;
            case GB_INT8_code   : METHOD (GrB_Matrix_extractTuples (I, J, (int8_t   *) X, &nvals, A)) ; break ;
            case GB_UINT8_code  : METHOD (GrB_Matrix_extractTuples (I, J, (uint8_t  *) X, &nvals, A)) ; break ;
            case GB_INT16_code  : METHOD (GrB_Matrix_extractTuples (I, J, (int16_t  *) X, &nvals, A)) ; break ;
            case GB_UINT16_code : METHOD (GrB_Matrix_extractTuples (I, J, (uint16_t *) X, &nvals, A)) ; break ;
            case GB_INT32_code  : METHOD (GrB_Matrix_extractTuples (I, J, (int32_t  *) X, &nvals, A)) ; break ;
            case GB_UINT32_code : METHOD (GrB_Matrix_extractTuples (I, J, (uint32_t *) X, &nvals, A)) ; break ;
            case GB_INT64_code  : METHOD (GrB_Matrix_extractTuples (I, J, (int64_t  *) X, &nvals, A)) ; break ;
            case GB_UINT64_code : METHOD (GrB_Matrix_extractTuples (I, J, (uint64_t *) X, &nvals, A)) ; break ;
            case GB_FP32_code   : METHOD (GrB_Matrix_extractTuples (I, J, (float    *) X, &nvals, A)) ; break ;
            case GB_FP64_code   : METHOD (GrB_Matrix_extractTuples (I, J, (double   *) X, &nvals, A)) ; break;
            case GB_UCT_code    :
            case GB_UDT_code    :
                METHOD (GrB_Matrix_extractTuples (I, J, Xtemp, &nvals, A)) ; break;
            default             : FREE_ALL ; mexErrMsgTxt ("unsupported class") ;
        }
    }

    if (A->type == Complex && nargout > 2)
    {
        // create the MATLAB complex X
        pargout [2] = mxCreateNumericMatrix
            (nvals, 1, mxDOUBLE_CLASS, mxCOMPLEX) ;
        GB_mx_complex_split (nvals, Xtemp, pargout [2]) ;
    }

    FREE_ALL ;
}

