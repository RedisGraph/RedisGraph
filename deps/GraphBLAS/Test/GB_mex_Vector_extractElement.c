//------------------------------------------------------------------------------
// GB_mex_Vector_extractElement: MATLAB interface for x = v(i)
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB_mex.h"

#define USAGE "x = GB_mex_Vector_extractElement v, I, xclass)"

#define FREE_ALL                        \
{                                       \
    GrB_free (&v) ;                     \
    GB_FREE_MEMORY (Xtemp, ni, sizeof (double complex)) ; \
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
    GrB_Vector v = NULL ;
    void *Y = NULL ;
    void *Xtemp = NULL ;
    mxClassID xclass ;
    GrB_Type xtype ;
    GrB_Index *I = NULL, ni = 0, I_range [3] ;
    bool is_list ;

    // check inputs
    GB_WHERE (USAGE) ;
    if (nargout > 1 || nargin < 2 || nargin > 3)
    {
        mexErrMsgTxt ("Usage: " USAGE) ;
    }

    #define GET_DEEP_COPY ;
    #define FREE_DEEP_COPY ;

    // get v (shallow copy)
    v = GB_mx_mxArray_to_Vector (pargin [0], "v input", false, true) ;
    if (v == NULL)
    {
        FREE_ALL ;
        mexErrMsgTxt ("v failed") ;
    }
    mxClassID aclass = GB_mx_Type_to_classID (v->type) ;

    // get I
    if (!GB_mx_mxArray_to_indices (&I, pargin [1], &ni, I_range, &is_list))
    {
        FREE_ALL ;
        mexErrMsgTxt ("I failed") ;
    }
    if (!is_list)
    {
        mexErrMsgTxt ("I must be a list") ;
    }

    // get xclass, default is class (A), and the corresponding xtype
    if (v->type == Complex)
    {
        // input argument xclass is ignored
        xtype = Complex ;
        xclass = mxDOUBLE_CLASS ;
        // create X
        GB_CALLOC_MEMORY (Xtemp, ni, sizeof (double complex)) ;
    }
    else
    {
        xclass = GB_mx_string_to_classID (aclass, PARGIN (2)) ;
        xtype = GB_mx_classID_to_Type (xclass) ;
        if (xtype == NULL)
        {
            FREE_ALL ;
            mexErrMsgTxt ("X must be numeric") ;
        }
        // create Y
        pargout [0] = mxCreateNumericMatrix (ni, 1, xclass, mxREAL) ;
        Y = mxGetData (pargout [0]) ;
    }

    size_t s = sizeof (double complex) ;

    // x = v (i)
    switch (xtype->code)
    {
        case GB_BOOL_code   :

            for (int64_t k = 0 ; k < ni ; k++)
            {
                bool *X = Y ;
                METHOD (GrB_Vector_extractElement (&X [k], v, I [k])) ;
            }
            break ;

        case GB_INT8_code   :

            for (int64_t k = 0 ; k < ni ; k++)
            {
                int8_t *X = Y ;
                METHOD (GrB_Vector_extractElement (&X [k], v, I [k])) ;
            }
            break ;

        case GB_UINT8_code  :

            for (int64_t k = 0 ; k < ni ; k++)
            {
                uint8_t *X = Y ;
                METHOD (GrB_Vector_extractElement (&X [k], v, I [k])) ;
            }
            break ;

        case GB_INT16_code  :

            for (int64_t k = 0 ; k < ni ; k++)
            {
                int16_t *X = Y ;
                METHOD (GrB_Vector_extractElement (&X [k], v, I [k])) ;
            }
            break ;

        case GB_UINT16_code :

            for (int64_t k = 0 ; k < ni ; k++)
            {
                uint16_t *X = Y ;
                METHOD (GrB_Vector_extractElement (&X [k], v, I [k])) ;
            }
            break ;

        case GB_INT32_code  :

            for (int64_t k = 0 ; k < ni ; k++)
            {
                int32_t *X = Y ;
                METHOD (GrB_Vector_extractElement (&X [k], v, I [k])) ;
            }
            break ;

        case GB_UINT32_code :

            for (int64_t k = 0 ; k < ni ; k++)
            {
                uint32_t *X = Y ;
                METHOD (GrB_Vector_extractElement (&X [k], v, I [k])) ;
            }
            break ;

        case GB_INT64_code  :

            for (int64_t k = 0 ; k < ni ; k++)
            {
                int64_t *X = Y ;
                METHOD (GrB_Vector_extractElement (&X [k], v, I [k])) ;
            }
            break ;

        case GB_UINT64_code :

            for (int64_t k = 0 ; k < ni ; k++)
            {
                uint64_t *X = Y ;
                METHOD (GrB_Vector_extractElement (&X [k], v, I [k])) ;
            }
            break ;

        case GB_FP32_code   :

            for (int64_t k = 0 ; k < ni ; k++)
            {
                float *X = Y ;
                METHOD (GrB_Vector_extractElement (&X [k], v, I [k])) ;
            }
            break ;

        case GB_FP64_code   :

            for (int64_t k = 0 ; k < ni ; k++)
            {
                double *X = Y ;
                METHOD (GrB_Vector_extractElement (&X [k], v, I [k])) ;
            }
            break;

        case GB_UCT_code   :
        case GB_UDT_code   :
            {
                // user-defined complex type
                for (int64_t k = 0 ; k < ni ; k++)
                {
                    METHOD (GrB_Vector_extractElement (Xtemp +(k*s), v, I [k]));
                }
            }
            break;

        default              : FREE_ALL ; mexErrMsgTxt ("unsupported class") ;
    }

    if (v->type == Complex)
    {
        // create the MATLAB complex X
        pargout [0] = mxCreateNumericMatrix (ni, 1, mxDOUBLE_CLASS, mxCOMPLEX) ;
        GB_mx_complex_split (ni, Xtemp, pargout [0]) ;
    }    

    FREE_ALL ;
}

