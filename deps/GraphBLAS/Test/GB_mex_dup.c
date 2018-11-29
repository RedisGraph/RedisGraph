//------------------------------------------------------------------------------
// GB_mex_dup: copy a matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// copy and typecast a matrix

#include "GB_mex.h"

#define USAGE "C = GB_mex_dup (A, cclass, method)"

#define FREE_ALL                        \
{                                       \
    GrB_free (&A) ;                     \
    GrB_free (&C) ;                     \
    GrB_free (&desc) ;                  \
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
    GrB_Matrix A = NULL, C = NULL ;
    GrB_Descriptor desc = NULL ;

    // check inputs
    GB_WHERE (USAGE) ;
    if (nargout > 1 || nargin < 1 || nargin > 3)
    {
        mexErrMsgTxt ("Usage: " USAGE) ;
    }

    #define GET_DEEP_COPY  ;
    #define FREE_DEEP_COPY ;

    // get A (shallow copy)
    A = GB_mx_mxArray_to_Matrix (pargin [0], "A input", false, true) ;
    if (A == NULL)
    {
        FREE_ALL ;
        mexErrMsgTxt ("A failed") ;
    }
    mxClassID aclass = GB_mx_Type_to_classID (A->type) ;

    // get cclass and ctype of output matrix
    mxClassID cclass ;
    GrB_Type ctype ;
    if (A->type == Complex)
    {
        ctype = Complex ;
        cclass = mxDOUBLE_CLASS ;
    }
    else
    {
        cclass = GB_mx_string_to_classID (aclass, PARGIN (1)) ;
        ctype = GB_mx_classID_to_Type (cclass) ;
        if (ctype == NULL)
        {
            FREE_ALL ;
            mexErrMsgTxt ("C must be numeric") ;
        }
    }

    // get method
    int GET_SCALAR (2, int, method, 0) ;

    if (ctype == A->type)
    {
        // copy C with the same type as A
        if (method == 0)
        {
            // printf ("dup\n") ;
            METHOD (GrB_Matrix_dup (&C, A)) ;
        }
        else
        {
            // try another method, just for testing (see User Guide)

            // C = create an exact copy of A, just like GrB_Matrix_dup
            // printf ("tran dup\n") ;
            GrB_Type type ;
            GrB_Index nrows, ncols ;

            #undef GET_DEEP_COPY
            #undef FREE_DEEP_COPY

            #define GET_DEEP_COPY                               \
            {                                                   \
                GxB_Matrix_type (&type, A) ;                    \
                GrB_Matrix_nrows (&nrows, A) ;                  \
                GrB_Matrix_ncols (&ncols, A) ;                  \
                GrB_Matrix_new (&C, type, nrows, ncols) ;       \
                GrB_Descriptor_new (&desc) ;                    \
                GxB_set (desc, GrB_INP0, GrB_TRAN) ;            \
            }
            #define FREE_DEEP_COPY                              \
            {                                                   \
                GrB_free (&C) ;                                 \
                GrB_free (&desc) ;                              \
            }

            GET_DEEP_COPY ;
            METHOD (GrB_transpose (C, NULL, NULL, A, desc)) ;

            #undef GET_DEEP_COPY
            #undef FREE_DEEP_COPY

        }
    }
    else
    {
        // typecast

        // C = (ctype) A
        // printf ("cast\n") ;
        GrB_Index nrows, ncols ;
        #define GET_DEEP_COPY                               \
        {                                                   \
            GrB_Matrix_nrows (&nrows, A) ;                  \
            GrB_Matrix_ncols (&ncols, A) ;                  \
            GrB_Matrix_new (&C, ctype, nrows, ncols) ;      \
            GrB_Descriptor_new (&desc) ;                    \
            GxB_set (desc, GrB_INP0, GrB_TRAN) ;            \
        }
        #define FREE_DEEP_COPY                              \
        {                                                   \
            GrB_free (&C) ;                                 \
            GrB_free (&desc) ;                              \
        }

        GET_DEEP_COPY ;
        METHOD (GrB_transpose (C, NULL, NULL, A, desc)) ;

        #undef GET_DEEP_COPY
        #undef FREE_DEEP_COPY
    }

    // return C to MATLAB as a struct and free the GraphBLAS C
    pargout [0] = GB_mx_Matrix_to_mxArray (&C, "C output", true) ;

    FREE_ALL ;
}

