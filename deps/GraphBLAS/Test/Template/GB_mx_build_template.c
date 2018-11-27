//------------------------------------------------------------------------------
// GB_mx_build_template: build a sparse vector or matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// This is not a stand-alone function; it is #include'd in
// GB_mex_Matrix_build.c and GB_mex_Vector_build.c.

// The function works the same as C = sparse (I,J,X,nrows,ncols) in MATLAB,
// except an optional operator, dup, can be provided.  The operator defines
// how duplicates are assembled.

// The type of dup and C may differ.  A matrix T is first built that has the
// same type as dup, and then typecasted to the desired type of C, given by
// the last argument, "class".

// This particular GraphBLAS implementation provides a well-defined order of
// 'summation'.  Entries in [I,J,X] are first sorted in increasing order of row
// index via a stable sort, with ties broken by the position of the tuple in
// the [I,J,X] list.  If duplicates appear, they are 'summed' in the order they
// appear in the [I,J,X] input.  That is, if the same indices i and j appear in
// positions k1, k2, k3, and k3 in [I,J,X], where k1 < k2 < k3 < k4, then the
// following operations will occur in order:

//      T (i,j) = X (k1) ;
//      T (i,j) = dup (T (i,j), X (k2)) ;
//      T (i,j) = dup (T (i,j), X (k3)) ;
//      T (i,j) = dup (T (i,j), X (k4)) ;

// Thus, dup need not be associative, and the results are still well-defined.
// Using the FIRST operator (a string 'first') means the first value (X (k1))
// is used and the rest are ignored.  The SECOND operator means the last value
// (X (k4)) is used instead.

#ifdef MATRIX
#define MAX_NARGIN 8
#define MIN_NARGIN 3
#define USAGE "GB_mex_Matrix_build (I,J,X,nrows,ncols,dup,class,csc)"
#define I_ARG 0
#define J_ARG 1
#define X_ARG 2
#define NROWS_ARG 3
#define NCOLS_ARG 4
#define DUP_ARG 5
#define CLASS_ARG 6
#define CSC_ARG 7

#define FREE_ALL                \
{                               \
    GB_MATRIX_FREE (&C) ;       \
    GB_mx_put_global (true, 0) ;        \
}

#else
#define MAX_NARGIN 5
#define MIN_NARGIN 2
#define USAGE "GB_mex_Vector_build (I,X,nrows,dup,class)"
#define I_ARG 0
#define X_ARG 1
#define NROWS_ARG 2
#define DUP_ARG 3
#define CLASS_ARG 4

#define FREE_ALL                \
{                               \
    GrB_free (&C) ;             \
    GB_mx_put_global (true, 0) ;        \
}

#endif

#define GET_DEEP_COPY ;
#define FREE_DEEP_COPY ;

bool malloc_debug = false ;

//------------------------------------------------------------------------------

GrB_Info builder
(
    #ifdef MATRIX
    GrB_Matrix *Chandle,
    #else
    GrB_Vector *Chandle,
    #endif
    GrB_Type ctype,
    GrB_Index nrows,
    GrB_Index ncols,
    GrB_Index *I,
    GrB_Index *J,
    void *X,
    GrB_Index ni,
    GrB_BinaryOp dup,
    bool C_is_csc,
    mxClassID xclass,
    GB_Context Context
)
{

    GrB_Info info ;
    (*Chandle) = NULL ;

    // create the GraphBLAS output object C
    #ifdef MATRIX
    if (C_is_csc)
    {
        // create a hypersparse CSC matrix
        info = GrB_Matrix_new (Chandle, ctype, nrows, ncols) ;
    }
    else
    {
        // create a hypersparse CSR matrix
        info = GB_new (Chandle, ctype, ncols, nrows, GB_Ap_calloc,
            false, GB_AUTO_HYPER, GB_HYPER_DEFAULT, 1, Context) ;
    }
    #else
    info = GrB_Vector_new (Chandle, ctype, nrows) ;
    #endif

    #ifdef MATRIX
    GrB_Matrix C = (*Chandle) ;
    #else
    GrB_Vector C = (*Chandle) ;
    #endif

    if (info != GrB_SUCCESS)
    {
        FREE_ALL ;
        return (info) ;
    }

    // build the matrix or vector from the tuples
    #ifdef MATRIX
    #define BUILD(type) info = GrB_Matrix_build (C,I,J,(const type *)X,ni,dup)
    #else
    #define BUILD(type) info = GrB_Vector_build (C,I,  (const type *)X,ni,dup)
    #endif

    ASSERT_OK (GB_check (ctype, "ctype for build", GB0)) ;
    ASSERT_OK (GB_check (dup, "dup for build", GB0)) ;
    // printf ("code %d biulding ni "GBd"\n", ctype->code, ni) ;

    switch (xclass)
    {
        case mxLOGICAL_CLASS  : BUILD (bool    ) ; break ;
        case mxINT8_CLASS     : BUILD (int8_t  ) ; break ;
        case mxUINT8_CLASS    : BUILD (uint8_t ) ; break ;
        case mxINT16_CLASS    : BUILD (int16_t ) ; break ;
        case mxUINT16_CLASS   : BUILD (uint16_t) ; break ;
        case mxINT32_CLASS    : BUILD (int32_t ) ; break ;
        case mxUINT32_CLASS   : BUILD (uint32_t) ; break ;
        case mxINT64_CLASS    : BUILD (int64_t ) ; break ;
        case mxUINT64_CLASS   : BUILD (uint64_t) ; break ;
        case mxSINGLE_CLASS   : BUILD (float   ) ; break ;
        case mxDOUBLE_CLASS   : BUILD (double  ) ; break ;
        case mxCELL_CLASS     :
        case mxCHAR_CLASS     :
        case mxUNKNOWN_CLASS  :
        case mxFUNCTION_CLASS :
        case mxSTRUCT_CLASS   :
        default               :
            FREE_ALL ;
            mexErrMsgTxt ("X class not supported")  ;
    }

    // printf ("info %d\n", info) ;

    if (info == GrB_SUCCESS)
    {
        ASSERT_OK (GB_check (C, "C built", GB0)) ;
    }
    else
    {
        FREE_ALL ;
    }

    return (info) ;
}

//------------------------------------------------------------------------------

void mexFunction
(
    int nargout,
    mxArray *pargout [ ],
    int nargin,
    const mxArray *pargin [ ]
)
{

    GrB_Info info ;
    malloc_debug = GB_mx_get_global (true) ;
    GrB_Index *I = NULL, ni = 0, I_range [3] ;
    GrB_Index *J = NULL, nj = 0, J_range [3] ;
    bool is_list ; 
    #ifdef MATRIX
    GrB_Matrix C = NULL ;
    #else
    GrB_Vector C = NULL ;
    #endif

    GB_WHERE (USAGE) ;

    // check inputs
    if (nargout > 1 || nargin < MIN_NARGIN || nargin > MAX_NARGIN)
    {
        mexErrMsgTxt ("Usage: C = " USAGE) ;
    }

    // get I
    if (!GB_mx_mxArray_to_indices (&I, pargin [I_ARG], &ni, I_range, &is_list))
    {
        FREE_ALL ;
        mexErrMsgTxt ("I failed") ;
    }
    if (!is_list)
    {
        mexErrMsgTxt ("I is invalid; must be a list") ;
    }

    #ifdef MATRIX
    // get J for a matrix
    if (!GB_mx_mxArray_to_indices (&J, pargin [J_ARG], &nj, J_range, &is_list))
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
    #endif

    // get X
    if (ni != mxGetNumberOfElements (pargin [X_ARG]))
    {
        FREE_ALL ;
        mexErrMsgTxt ("I and X must be the same size") ;
    }
    if (!(mxIsNumeric (pargin [X_ARG]) || mxIsLogical (pargin [X_ARG])))
    {
        FREE_ALL ;
        mexErrMsgTxt ("X must be a numeric or logical array") ;
    }
    if (mxIsSparse (pargin [X_ARG]))
    {
        FREE_ALL ;
        mexErrMsgTxt ("X cannot be sparse") ;
    }
    void *X = mxGetData (pargin [X_ARG]) ;
    mxClassID xclass = mxGetClassID (pargin [X_ARG]) ;
    GrB_Type xtype = GB_mx_classID_to_Type (xclass) ;
    if (xtype == NULL)
    {
        FREE_ALL ;
        mexErrMsgTxt ("X must be numeric") ;
    }

    // get the number of rows
    uint64_t nrows = 0 ;
    if (nargin > NROWS_ARG)
    {
        nrows = (uint64_t) mxGetScalar (pargin [NROWS_ARG]) ;
    }
    else
    {
        for (int64_t k = 0 ; k < ni ; k++)
        {
            nrows = GB_IMAX (nrows, I [k]) ;
        }
        nrows++ ;
    }

    // get the number of columns of a matrix
    uint64_t ncols = 1 ;
    #ifdef MATRIX
    if (nargin > NCOLS_ARG)
    {
        ncols = (uint64_t) mxGetScalar (pargin [NCOLS_ARG]) ;
    }
    else
    {
        ncols = 0 ;
        for (int64_t k = 0 ; k < ni ; k++)
        {
            ncols = GB_IMAX (ncols, J [k]) ;
        }
        ncols++ ;
    }
    #endif

    // get dup; default: PLUS, default class is class(X)
    GrB_BinaryOp dup ;
    if (!GB_mx_mxArray_to_BinaryOp (&dup, PARGIN (DUP_ARG), "dup",
        GB_PLUS_opcode, xclass, false, false) || dup == NULL)
    {
        FREE_ALL ;
        mexErrMsgTxt ("dup failed") ;
    }

    // get the type for C, default is same as xclass and dup->ztype
    mxClassID cclass = GB_mx_string_to_classID (xclass, PARGIN (CLASS_ARG)) ;
    GrB_Type ctype = GB_mx_classID_to_Type (cclass) ;

    bool C_is_csc = true ;
    #ifdef MATRIX
    // get the CSC/CSR format
    if (nargin > CSC_ARG)
    {
        C_is_csc = (bool) mxGetScalar (pargin [CSC_ARG]) ;
    }
    #endif

    METHOD (builder (&C, ctype, nrows, ncols, I, J, X, ni, dup,
        C_is_csc, xclass, Context)) ;

    ASSERT_OK (GB_check (C, "C built", GB0)) ;

    // return C to MATLAB as a struct and free the GraphBLAS C
    #ifdef MATRIX
    pargout [0] = GB_mx_Matrix_to_mxArray (&C, "C output", true) ;
    #else
    pargout [0] = GB_mx_Vector_to_mxArray (&C, "C output", true) ;
    #endif

    FREE_ALL ;
}

