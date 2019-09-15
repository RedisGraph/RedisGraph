//------------------------------------------------------------------------------
// GB_mex_export_import: export and then reimport a matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// format:
//  0: standard CSR
//  1: standard CSC
//  3: hyper CSR
//  4: hyper CSC

#include "GB_mex.h"

#define USAGE "C = GB_mex_export_import (A, format_matrix, format_export)"

#define FREE_WORK                       \
{                                       \
    GB_FREE_MEMORY (Cp, nvec+1, sizeof (GrB_Index)) ; \
    GB_FREE_MEMORY (Ch, nvec  , sizeof (GrB_Index)) ; \
    GB_FREE_MEMORY (Ci, nvals , sizeof (GrB_Index)) ; \
    GB_FREE_MEMORY (Cx, nvals , csize) ; \
    GB_MATRIX_FREE (&C) ;               \
}

#define FREE_ALL                        \
{                                       \
    FREE_WORK ;                         \
    GB_MATRIX_FREE (&A) ;               \
    GB_mx_put_global (true, 0) ;        \
}

#define OK(method)                              \
{                                               \
    info = method ;                             \
    if (info != GrB_SUCCESS)                    \
    {                                           \
        FREE_WORK ;                             \
        return (info) ;                         \
    }                                           \
}

GrB_Matrix A = NULL ;
GrB_Matrix C = NULL ;
GrB_Index *Cp = NULL, *Ch = NULL, *Ci = NULL ;
void *Cx = NULL ;
GB_Context Context = NULL ;
size_t csize = 0 ;
GrB_Index nvec = 0, nvals = 0, nrows = 0, ncols = 0 ;
int64_t nonempty = -1 ;
GrB_Type type = NULL, atype = NULL;
GrB_Info info = GrB_SUCCESS ;

GrB_Info export_import ( int format_matrix, int format_export) ;

//------------------------------------------------------------------------------

GrB_Info export_import
(
    int format_matrix,
    int format_export
)
{

    GxB_Matrix_type (&atype, A) ;
    GxB_Type_size (&csize, atype) ;

    OK (GrB_Matrix_dup (&C, A)) ;

    //--------------------------------------------------------------------------
    // convert C to the requested format
    //--------------------------------------------------------------------------

    switch (format_matrix)
    {

        //----------------------------------------------------------------------
        case 0 :    // standard CSR
        //----------------------------------------------------------------------

            OK (GxB_set (C, GxB_HYPER,  GxB_NEVER_HYPER)) ;
            OK (GxB_set (C, GxB_FORMAT, GxB_BY_ROW)) ;
            break ;

        //----------------------------------------------------------------------
        case 1 :    // standard CSC
        //----------------------------------------------------------------------

            OK (GxB_set (C, GxB_HYPER,  GxB_NEVER_HYPER)) ;
            OK (GxB_set (C, GxB_FORMAT, GxB_BY_COL)) ;
            break ;

        //----------------------------------------------------------------------
        case 2 :    // hypersparse CSR
        //----------------------------------------------------------------------

            OK (GxB_set (C, GxB_HYPER,  GxB_ALWAYS_HYPER)) ;
            OK (GxB_set (C, GxB_FORMAT, GxB_BY_ROW)) ;
            break ;

        //----------------------------------------------------------------------
        case 3 :    // hypersparse CSC
        //----------------------------------------------------------------------

            OK (GxB_set (C, GxB_HYPER,  GxB_ALWAYS_HYPER)) ;
            OK (GxB_set (C, GxB_FORMAT, GxB_BY_COL)) ;
            break ;

        default : mexErrMsgTxt ("invalid format") ;
    }

    //--------------------------------------------------------------------------
    // export then import
    //--------------------------------------------------------------------------

    switch (format_export)
    {

        //----------------------------------------------------------------------
        case 0 :    // standard CSR
        //----------------------------------------------------------------------

            OK (GxB_Matrix_export_CSR (&C, &type, &nrows, &ncols, &nvals,
                &nonempty, &Cp, &Ci, &Cx, NULL)) ;
            nvec = nrows ;

            OK (GxB_Matrix_import_CSR (&C, type, nrows, ncols, nvals,
                nonempty, &Cp, &Ci, &Cx, NULL)) ;
            break ;

        //----------------------------------------------------------------------
        case 1 :    // standard CSC
        //----------------------------------------------------------------------

            OK (GxB_Matrix_export_CSC (&C, &type, &nrows, &ncols, &nvals,
                &nonempty, &Cp, &Ci, &Cx, NULL)) ;
            nvec = ncols ;

            OK (GxB_Matrix_import_CSC (&C, type, nrows, ncols, nvals,
                nonempty, &Cp, &Ci, &Cx, NULL)) ;
            break ;

        //----------------------------------------------------------------------
        case 2 :    // hypersparse CSR
        //----------------------------------------------------------------------

            OK (GxB_Matrix_export_HyperCSR (&C, &type, &nrows, &ncols, &nvals,
                &nonempty, &nvec, &Ch, &Cp, &Ci, &Cx, NULL)) ;

            OK (GxB_Matrix_import_HyperCSR (&C, type, nrows, ncols, nvals,
                nonempty, nvec, &Ch, &Cp, &Ci, &Cx, NULL)) ;
            break ;

        //----------------------------------------------------------------------
        case 3 :    // hypersparse CSC
        //----------------------------------------------------------------------

            OK (GxB_Matrix_export_HyperCSC (&C, &type, &nrows, &ncols, &nvals,
                &nonempty, &nvec, &Ch, &Cp, &Ci, &Cx, NULL)) ;

            OK (GxB_Matrix_import_HyperCSC (&C, type, nrows, ncols, nvals,
                nonempty, nvec, &Ch, &Cp, &Ci, &Cx, NULL)) ;
            break ;

        default : mexErrMsgTxt ("invalid format") ;
    }

    return (GrB_SUCCESS) ;
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

    bool malloc_debug = GB_mx_get_global (true) ;

    // check inputs
    GB_WHERE (USAGE) ;
    if (nargout > 1 || nargin != 3)
    {
        mexErrMsgTxt ("Usage: " USAGE) ;
    }

    // get A (shallow copy)
    {
        A = GB_mx_mxArray_to_Matrix (pargin [0], "A input", false, true) ;
        if (A == NULL)
        {
            FREE_ALL ;
            mexErrMsgTxt ("A failed") ;
        }
    }

    // get matrix format (0 to 3)
    int GET_SCALAR (1, int, format_matrix, 0) ;

    // get export/import format (0 to 3)
    int GET_SCALAR (2, int, format_export, 0) ;

    #define GET_DEEP_COPY ;
    #define FREE_DEEP_COPY ;

    // convert matrix, export, then import
    METHOD (export_import (format_matrix, format_export)) ;

    // return C to MATLAB as a struct and free the GraphBLAS C
    pargout [0] = GB_mx_Matrix_to_mxArray (&C, "C output", true) ;

    FREE_ALL ;
}

