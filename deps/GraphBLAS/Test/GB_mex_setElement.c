//------------------------------------------------------------------------------
// GB_mex_setElement: MATLAB interface for A(i,j) = x
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// x = A (i,j), where i and j are zero-based.  If i and j arrays, then
// x (k) = A (i (k), j (k)) is done for all k.

// I and J and zero-based

#include "GB_mex.h"

#define USAGE "A = GB_mex_setElement (A, I, J, X)"

#define FREE_ALL                        \
{                                       \
    GB_MATRIX_FREE (&A) ;               \
    GB_FREE_MEMORY (Xtemp, ni, sizeof (double complex)) ; \
    GB_mx_put_global (true, 0) ;        \
}


// set all elements of a matrix and return if an error is encountered
#define setEl(name,type)                                                    \
GrB_Info set_ ## name                                                       \
(GrB_Matrix A, type *X, GrB_Index *I, GrB_Index *J, GrB_Index ni)           \
{                                                                           \
    for (int64_t k = 0 ; k < ni ; k++)                                      \
    {                                                                       \
        GrB_Info info = GrB_Matrix_setElement_ ## name                      \
            (A, AMPERSAND (X [k]), I [k], J [k]) ;                          \
        if (info != GrB_SUCCESS) return (info) ;                            \
    }                                                                       \
    return (GrB_SUCCESS) ;                                                  \
}

// create all the local set_TYPE functions
#define AMPERSAND(x) x
setEl (BOOL   , bool          ) ;
setEl (INT8   , int8_t        ) ;
setEl (UINT8  , uint8_t       ) ;
setEl (INT16  , int16_t       ) ;
setEl (UINT16 , uint16_t      ) ;
setEl (INT32  , int32_t       ) ;
setEl (UINT32 , uint32_t      ) ;
setEl (INT64  , int64_t       ) ;
setEl (UINT64 , uint64_t      ) ;
setEl (FP32   , float         ) ;
setEl (FP64   , double        ) ;
#undef  AMPERSAND
#define AMPERSAND(x) &x
setEl (UDT    , double complex) ;
#undef  AMPERSAND


// set all elements of a vector and return if an error is encountered
#define vsetEl(name,type)                                                   \
GrB_Info vset_ ## name                                                      \
(GrB_Matrix A, type *X, GrB_Index *I, GrB_Index ni)                         \
{                                                                           \
    GrB_Vector w = (GrB_Vector) A ;                                         \
    for (int64_t k = 0 ; k < ni ; k++)                                      \
    {                                                                       \
        GrB_Info info = GrB_Vector_setElement_ ## name                      \
            (w, AMPERSAND (X [k]), I [k]) ;                                 \
        if (info != GrB_SUCCESS) return (info) ;                            \
    }                                                                       \
    return (GrB_SUCCESS) ;                                                  \
}

// create all the local set_TYPE functions
#define AMPERSAND(x) x
vsetEl (BOOL   , bool          ) ;
vsetEl (INT8   , int8_t        ) ;
vsetEl (UINT8  , uint8_t       ) ;
vsetEl (INT16  , int16_t       ) ;
vsetEl (UINT16 , uint16_t      ) ;
vsetEl (INT32  , int32_t       ) ;
vsetEl (UINT32 , uint32_t      ) ;
vsetEl (INT64  , int64_t       ) ;
vsetEl (UINT64 , uint64_t      ) ;
vsetEl (FP32   , float         ) ;
vsetEl (FP64   , double        ) ;
#undef  AMPERSAND
#define AMPERSAND(x) &x
vsetEl (UDT    , double complex) ;
#undef  AMPERSAND

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
    void *Y ;
    GrB_Type xtype ;
    void *Xtemp = NULL ;
    GrB_Index *I = NULL, ni = 0, I_range [3] ;
    GrB_Index *J = NULL, nj = 0, J_range [3] ;
    bool is_list ;

    // check inputs
    GB_WHERE (USAGE) ;
    if (nargout > 1 || nargin != 4)
    {
        mexErrMsgTxt ("Usage: " USAGE) ;
    }

    // get A (deep copy)
    #define GET_DEEP_COPY \
    A = GB_mx_mxArray_to_Matrix (pargin [0], "A input", true, true) ;
    #define FREE_DEEP_COPY GB_MATRIX_FREE (&A) ;
    GET_DEEP_COPY ;
    if (A == NULL)
    {
        FREE_ALL ;
        mexErrMsgTxt ("A failed") ;
    }
    mxClassID aclass = GB_mx_Type_to_classID (A->type) ;

    // get I
    if (!GB_mx_mxArray_to_indices (&I, pargin [1], &ni, I_range, &is_list))
    {
        FREE_ALL ;
        mexErrMsgTxt ("I failed") ;
    }
    if (!is_list)
    {
        mexErrMsgTxt ("I is invalid; must be a list") ;
    }

    // get J
    if (!GB_mx_mxArray_to_indices (&J, pargin [2], &nj, J_range, &is_list))
    {
        FREE_ALL ;
        mexErrMsgTxt ("J failed") ;
    }
    if (!is_list)
    {
        mexErrMsgTxt ("J is invalid; must be a list") ;
    }

    if (ni != nj)
    {
        FREE_ALL ;
        mexErrMsgTxt ("I and J must be the same size") ;
    }

    // get X
    if (ni != mxGetNumberOfElements (pargin [3]))
    {
        FREE_ALL ;
        mexErrMsgTxt ("I and X must be the same size") ;
    }
    if (!(mxIsNumeric (pargin [3]) || mxIsLogical (pargin [3])))
    {
        FREE_ALL ;
        mexErrMsgTxt ("X must be a numeric or logical array") ;
    }
    if (mxIsSparse (pargin [3]))
    {
        FREE_ALL ;
        mexErrMsgTxt ("X cannot be sparse") ;
    }

    if (mxIsComplex (pargin [3]))
    {
        // copy the MATLAB complex
        xtype = Complex ;
        GB_MALLOC_MEMORY (Xtemp, ni, sizeof (double complex)) ;
        GB_mx_complex_merge (ni, Xtemp, pargin [3]) ;
        Y = Xtemp ;
    }
    else
    {
        Y = mxGetData (pargin [3]) ;
        mxClassID xclass = mxGetClassID (pargin [3]) ;
        xtype = GB_mx_classID_to_Type (xclass) ;
        if (xtype == NULL)
        {
            FREE_ALL ;
            mexErrMsgTxt ("X must be numeric") ;
        }
    }

    size_t s = sizeof (double complex) ;

    // A (i,j) = x, for a list of elements

    // the METHOD (...) macro is not used on each call to setElement, but
    // to all of them.  Thus, if any failure occurs, the computation is rolled
    // back to the very beginning, and another fresh, deep, copy of A is made,
    // and the sequence of setElements is tried again.  If a setElement fails
    // by running out of memory, it clears to whole matrix, so recovery cannot
    // be made.

    if (A->vdim == 1)
    {
        // test GrB_Vector_setElement
        switch (xtype->code)
        {
            case GB_BOOL_code   : METHOD (vset_BOOL   (A, Y, I, ni)) ; break ;
            case GB_INT8_code   : METHOD (vset_INT8   (A, Y, I, ni)) ; break ;
            case GB_UINT8_code  : METHOD (vset_UINT8  (A, Y, I, ni)) ; break ;
            case GB_INT16_code  : METHOD (vset_INT16  (A, Y, I, ni)) ; break ;
            case GB_UINT16_code : METHOD (vset_UINT16 (A, Y, I, ni)) ; break ;
            case GB_INT32_code  : METHOD (vset_INT32  (A, Y, I, ni)) ; break ;
            case GB_UINT32_code : METHOD (vset_UINT32 (A, Y, I, ni)) ; break ;
            case GB_INT64_code  : METHOD (vset_INT64  (A, Y, I, ni)) ; break ;
            case GB_UINT64_code : METHOD (vset_UINT64 (A, Y, I, ni)) ; break ;
            case GB_FP32_code   : METHOD (vset_FP32   (A, Y, I, ni)) ; break ;
            case GB_FP64_code   : METHOD (vset_FP64   (A, Y, I, ni)) ; break ;
            case GB_UCT_code    :
            case GB_UDT_code    : METHOD (vset_UDT    (A, Y, I, ni)) ; break ;
            default:
                FREE_ALL ;
                mexErrMsgTxt ("unsupported class") ;
        }
    }
    else
    {
        // test GrB_Matrix_setElement
        switch (xtype->code)
        {
            case GB_BOOL_code   : METHOD (set_BOOL   (A, Y, I, J, ni)) ; break ;
            case GB_INT8_code   : METHOD (set_INT8   (A, Y, I, J, ni)) ; break ;
            case GB_UINT8_code  : METHOD (set_UINT8  (A, Y, I, J, ni)) ; break ;
            case GB_INT16_code  : METHOD (set_INT16  (A, Y, I, J, ni)) ; break ;
            case GB_UINT16_code : METHOD (set_UINT16 (A, Y, I, J, ni)) ; break ;
            case GB_INT32_code  : METHOD (set_INT32  (A, Y, I, J, ni)) ; break ;
            case GB_UINT32_code : METHOD (set_UINT32 (A, Y, I, J, ni)) ; break ;
            case GB_INT64_code  : METHOD (set_INT64  (A, Y, I, J, ni)) ; break ;
            case GB_UINT64_code : METHOD (set_UINT64 (A, Y, I, J, ni)) ; break ;
            case GB_FP32_code   : METHOD (set_FP32   (A, Y, I, J, ni)) ; break ;
            case GB_FP64_code   : METHOD (set_FP64   (A, Y, I, J, ni)) ; break ;
            case GB_UCT_code    :
            case GB_UDT_code    : METHOD (set_UDT    (A, Y, I, J, ni)) ; break ;
            default:
                FREE_ALL ;
                mexErrMsgTxt ("unsupported class") ;
        }
    }

    // only do debug checks after adding lots of tuples
    if (ni > 1000) ASSERT_OK (GB_check (A, "A added pending tuples", GB0)) ;

    // GB_wait (A) ;
    // if (ni > 1000) ASSERT_OK (GB_check (A, "A wiated", GB0)) ;

    // return A to MATLAB as a struct and free the GraphBLAS A
    pargout [0] = GB_mx_Matrix_to_mxArray (&A, "A output", true) ;

    FREE_ALL ;
}

