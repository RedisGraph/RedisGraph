//------------------------------------------------------------------------------
// GB_mex_export_import: export and then reimport a matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

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
    GB_FREE (Cp) ;                      \
    GB_FREE (Ch) ;                      \
    GB_FREE (Ci) ;                      \
    GB_FREE (Cx) ;                      \
    GB_FREE (Cb) ;                      \
    GrB_Matrix_free_(&C) ;              \
}

#define FREE_ALL                        \
{                                       \
    FREE_WORK ;                         \
    GrB_Matrix_free_(&A) ;              \
    GB_mx_put_global (true) ;           \
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
GB_void *Cx = NULL ;
int8_t *Cb = NULL ;
GB_Context Context = NULL ;
GrB_Index nvec = 0, nvals = 0, nrows = 0, ncols = 0 ;

GrB_Index Ci_size = 0 ;
GrB_Index Cx_size = 0 ;
GrB_Index Cp_size = 0 ;
GrB_Index Cj_size = 0 ;
GrB_Index Cb_size = 0 ;
GrB_Index Ch_size = 0 ;

int64_t ignore = -1 ;
bool jumbled = false ;
GrB_Type type = NULL ;
GrB_Info info = GrB_SUCCESS ;

GrB_Info export_import ( int format_matrix, int format_export) ;
GrB_Info vector_export_import ( int format_matrix, int format_export) ;

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

    // get matrix format (1 to 8, and -1 to -8)
    int GET_SCALAR (1, int, format_matrix, 0) ;
    bool do_matrix = (format_matrix > 0) ;
    if (format_matrix < 0)
    {
        format_matrix = -format_matrix ;
    }

    // get export/import format (0 to 11)
    int GET_SCALAR (2, int, format_export, 0) ;

    #define GET_DEEP_COPY   GrB_Matrix_dup (&C, A) ;
    #define FREE_DEEP_COPY  GrB_Matrix_free (&C) ;

    // C = deep copy of A
    GET_DEEP_COPY ;

    // convert matrix, export, then import
    if (do_matrix)
    {
        METHOD (export_import (format_matrix, format_export)) ;
    }

    FREE_DEEP_COPY ;
    GET_DEEP_COPY ;

    // convert vector, export, then import, if C can be cast as a GrB_Vector
    if (GB_VECTOR_OK (C))
    {
        METHOD (vector_export_import (format_matrix, format_export)) ;
    }

    // return C to MATLAB as a struct and free the GraphBLAS C
    pargout [0] = GB_mx_Matrix_to_mxArray (&C, "C output", true) ;
    FREE_ALL ;
}



//------------------------------------------------------------------------------

GrB_Info export_import
(
    int format_matrix,
    int format_export
)
{

    //--------------------------------------------------------------------------
    // convert C to the requested format
    //--------------------------------------------------------------------------

    switch (format_matrix)
    {

        //----------------------------------------------------------------------
        case 1 :    // standard CSR
        //----------------------------------------------------------------------

            OK (GxB_Matrix_Option_set_(C, GxB_HYPER_SWITCH, GxB_NEVER_HYPER)) ;
            OK (GxB_Matrix_Option_set_(C, GxB_SPARSITY_CONTROL, GxB_SPARSE)) ;
            OK (GxB_Matrix_Option_set_(C, GxB_FORMAT, GxB_BY_ROW)) ;
            break ;

        //----------------------------------------------------------------------
        case 2 :    // standard CSC
        //----------------------------------------------------------------------

            OK (GxB_Matrix_Option_set_(C, GxB_HYPER_SWITCH, GxB_NEVER_HYPER)) ;
            OK (GxB_Matrix_Option_set_(C, GxB_SPARSITY_CONTROL, GxB_SPARSE)) ;
            OK (GxB_Matrix_Option_set_(C, GxB_FORMAT, GxB_BY_COL)) ;
            break ;

        //----------------------------------------------------------------------
        case 3 :    // hypersparse CSR
        //----------------------------------------------------------------------

            OK (GxB_Matrix_Option_set_(C, GxB_HYPER_SWITCH, GxB_ALWAYS_HYPER)) ;
            OK (GxB_Matrix_Option_set_(C, GxB_SPARSITY_CONTROL,
                GxB_HYPERSPARSE)) ;
            OK (GxB_Matrix_Option_set_(C, GxB_FORMAT, GxB_BY_ROW)) ;
            break ;

        //----------------------------------------------------------------------
        case 4 :    // hypersparse CSC
        //----------------------------------------------------------------------

            OK (GxB_Matrix_Option_set_(C, GxB_HYPER_SWITCH, GxB_ALWAYS_HYPER)) ;
            OK (GxB_Matrix_Option_set_(C, GxB_SPARSITY_CONTROL,
                GxB_HYPERSPARSE)) ;
            OK (GxB_Matrix_Option_set_(C, GxB_FORMAT, GxB_BY_COL)) ;
            break ;

        //----------------------------------------------------------------------
        case 5 :    // bitmapR
        //----------------------------------------------------------------------

            OK (GxB_Matrix_Option_set_(C, GxB_SPARSITY_CONTROL, GxB_BITMAP)) ;
            OK (GxB_Matrix_Option_set_(C, GxB_FORMAT, GxB_BY_ROW)) ;
            break ;

        //----------------------------------------------------------------------
        case 6 :    // bitmapC
        //----------------------------------------------------------------------

            OK (GxB_Matrix_Option_set_(C, GxB_SPARSITY_CONTROL, GxB_BITMAP)) ;
            OK (GxB_Matrix_Option_set_(C, GxB_FORMAT, GxB_BY_COL)) ;
            break ;

        //----------------------------------------------------------------------
        case 7 :    // FullR
        //----------------------------------------------------------------------

            OK (GxB_Matrix_Option_set_(C, GxB_SPARSITY_CONTROL, GxB_FULL)) ;
            OK (GxB_Matrix_Option_set_(C, GxB_FORMAT, GxB_BY_ROW)) ;
            break ;

        //----------------------------------------------------------------------
        case 8 :    // FullC
        //----------------------------------------------------------------------

            OK (GxB_Matrix_Option_set_(C, GxB_SPARSITY_CONTROL, GxB_FULL)) ;
            OK (GxB_Matrix_Option_set_(C, GxB_FORMAT, GxB_BY_COL)) ;
            break ;

        //----------------------------------------------------------------------
        case 9 :    // to control == 11, then bitmap
        //----------------------------------------------------------------------

            OK (GxB_Matrix_Option_set_(C, GxB_SPARSITY_CONTROL, GxB_BITMAP)) ;
            OK (GxB_Matrix_Option_set_(C, GxB_SPARSITY_CONTROL,
               GxB_HYPERSPARSE + GxB_SPARSE + GxB_FULL)) ;
            OK (GxB_Matrix_Option_set_(C, GxB_FORMAT, GxB_BY_COL)) ;
            break ;

        default : mexErrMsgTxt ("invalid mtx format") ;
    }

    //--------------------------------------------------------------------------
    // export then import
    //--------------------------------------------------------------------------

    switch (format_export)
    {

        //----------------------------------------------------------------------
        case 0 :    // standard CSR
        //----------------------------------------------------------------------

            OK (GxB_Matrix_export_CSR (&C, &type, &nrows, &ncols,
                &Cp, &Ci, &Cx, &Cp_size, &Ci_size, &Cx_size, &jumbled, NULL)) ;

            OK (GxB_Matrix_import_CSR (&C, type, nrows, ncols,
                &Cp, &Ci, &Cx, Cp_size, Ci_size, Cx_size, jumbled, NULL)) ;

            break ;

        //----------------------------------------------------------------------
        case 1 :    // standard CSC
        //----------------------------------------------------------------------

            OK (GxB_Matrix_export_CSC (&C, &type, &nrows, &ncols,
                &Cp, &Ci, &Cx, &Cp_size, &Ci_size, &Cx_size, &jumbled, NULL)) ;

            OK (GxB_Matrix_import_CSC (&C, type, nrows, ncols,
                &Cp, &Ci, &Cx, Cp_size, Ci_size, Cx_size, jumbled, NULL)) ;

            break ;

        //----------------------------------------------------------------------
        case 2 :    // hypersparse CSR
        //----------------------------------------------------------------------

            OK (GxB_Matrix_export_HyperCSR (&C, &type, &nrows, &ncols,
                &Cp, &Ch, &Ci, &Cx, &Cp_size, &Ch_size, &Ci_size, &Cx_size,
                &nvec, &jumbled, NULL)) ;

            OK (GxB_Matrix_import_HyperCSR (&C, type, nrows, ncols,
                &Cp, &Ch, &Ci, &Cx, Cp_size, Ch_size, Ci_size, Cx_size,
                nvec, jumbled, NULL)) ;

            break ;

        //----------------------------------------------------------------------
        case 3 :    // hypersparse CSC
        //----------------------------------------------------------------------

            OK (GxB_Matrix_export_HyperCSC (&C, &type, &nrows, &ncols,
                &Cp, &Ch, &Ci, &Cx, &Cp_size, &Ch_size, &Ci_size, &Cx_size,
                &nvec, &jumbled, NULL)) ;

            OK (GxB_Matrix_import_HyperCSC (&C, type, nrows, ncols,
                &Cp, &Ch, &Ci, &Cx, Cp_size, Ch_size, Ci_size, Cx_size,
                nvec, jumbled, NULL)) ;

            break ;

        //----------------------------------------------------------------------
        case 4 :    // bitmapR
        //----------------------------------------------------------------------

            OK (GxB_Matrix_export_BitmapR (&C, &type, &nrows, &ncols,
                &Cb, &Cx, &Cb_size, &Cx_size, &nvals, NULL)) ;

            OK (GxB_Matrix_import_BitmapR (&C, type, nrows, ncols,
                &Cb, &Cx, Cb_size, Cx_size, nvals, NULL)) ;

            break ;

        //----------------------------------------------------------------------
        case 5 :    // bitmapC
        //----------------------------------------------------------------------

            OK (GxB_Matrix_export_BitmapC (&C, &type, &nrows, &ncols,
                &Cb, &Cx, &Cb_size, &Cx_size, &nvals, NULL)) ;

            OK (GxB_Matrix_import_BitmapC (&C, type, nrows, ncols,
                &Cb, &Cx, Cb_size, Cx_size, nvals, NULL)) ;

            break ;

        //----------------------------------------------------------------------
        case 6 :    // FullR
        //----------------------------------------------------------------------

            OK (GxB_Matrix_export_FullR (&C, &type, &nrows, &ncols,
                &Cx, &Cx_size, NULL)) ;

            OK (GxB_Matrix_import_FullR (&C, type, nrows, ncols,
                &Cx, Cx_size, NULL)) ;

            break ;

        //----------------------------------------------------------------------
        case 7 :    // FullC
        //----------------------------------------------------------------------

            OK (GxB_Matrix_export_FullC (&C, &type, &nrows, &ncols,
                &Cx, &Cx_size, NULL)) ;

            OK (GxB_Matrix_import_FullC (&C, type, nrows, ncols,
                &Cx, Cx_size, NULL)) ;

            break ;

        //----------------------------------------------------------------------
        case 8 :    // standard CSR, not jumbled
        //----------------------------------------------------------------------

            OK (GxB_Matrix_export_CSR (&C, &type, &nrows, &ncols,
                &Cp, &Ci, &Cx, &Cp_size, &Ci_size, &Cx_size, NULL, NULL)) ;

            OK (GxB_Matrix_import_CSR (&C, type, nrows, ncols,
                &Cp, &Ci, &Cx, Cp_size, Ci_size, Cx_size, false, NULL)) ;

            break ;

        //----------------------------------------------------------------------
        case 9 :    // standard CSC, not jumbled
        //----------------------------------------------------------------------

            OK (GxB_Matrix_export_CSC (&C, &type, &nrows, &ncols,
                &Cp, &Ci, &Cx, &Cp_size, &Ci_size, &Cx_size, NULL, NULL)) ;

            OK (GxB_Matrix_import_CSC (&C, type, nrows, ncols,
                &Cp, &Ci, &Cx, Cp_size, Ci_size, Cx_size, false, NULL)) ;

            break ;

        //----------------------------------------------------------------------
        case 10 :    // hypersparse CSR, not jumbled
        //----------------------------------------------------------------------

            OK (GxB_Matrix_export_HyperCSR (&C, &type, &nrows, &ncols,
                &Cp, &Ch, &Ci, &Cx, &Cp_size, &Ch_size, &Ci_size, &Cx_size,
                &nvec, NULL, NULL)) ;

            OK (GxB_Matrix_import_HyperCSR (&C, type, nrows, ncols,
                &Cp, &Ch, &Ci, &Cx, Cp_size, Ch_size, Ci_size, Cx_size,
                nvec, false, NULL)) ;

            break ;

        //----------------------------------------------------------------------
        case 11 :    // hypersparse CSC, not jumbled
        //----------------------------------------------------------------------

            OK (GxB_Matrix_export_HyperCSC (&C, &type, &nrows, &ncols,
                &Cp, &Ch, &Ci, &Cx, &Cp_size, &Ch_size, &Ci_size, &Cx_size,
                &nvec, NULL, NULL)) ;

            OK (GxB_Matrix_import_HyperCSC (&C, type, nrows, ncols,
                &Cp, &Ch, &Ci, &Cx, Cp_size, Ch_size, Ci_size, Cx_size,
                nvec, false, NULL)) ;

            break ;


        default : mexErrMsgTxt ("invalid export format") ;
    }

    return (GrB_SUCCESS) ;
}

//------------------------------------------------------------------------------

GrB_Info vector_export_import
(
    int format_matrix,
    int format_export
)
{

    //--------------------------------------------------------------------------
    // convert C as a vector to the requested format, if available
    //--------------------------------------------------------------------------

    switch (format_matrix)
    {

        //----------------------------------------------------------------------
        case 1 :    // standard CSR
        //----------------------------------------------------------------------

            return (GrB_SUCCESS) ;

        //----------------------------------------------------------------------
        case 2 :    // standard CSC
        //----------------------------------------------------------------------

            OK (GxB_Vector_Option_set_((GrB_Vector) C,
                GxB_SPARSITY_CONTROL, GxB_SPARSE)) ;
            break ;

        //----------------------------------------------------------------------
        case 3 :    // hypersparse CSR 
        //----------------------------------------------------------------------

            return (GrB_SUCCESS) ;

        //----------------------------------------------------------------------
        case 4 :    // hypersparse CSC (will be sparse, not hypersparse)
        //----------------------------------------------------------------------

            OK (GxB_Vector_Option_set_((GrB_Vector) C,
                GxB_SPARSITY_CONTROL, GxB_HYPERSPARSE)) ;
            break ;

        //----------------------------------------------------------------------
        case 5 :    // bitmapR
        //----------------------------------------------------------------------

            return (GrB_SUCCESS) ;

        //----------------------------------------------------------------------
        case 6 :    // bitmapC
        case 9 :    // bitmapC
        //----------------------------------------------------------------------

            OK (GxB_Vector_Option_set_((GrB_Vector) C,
                GxB_SPARSITY_CONTROL, GxB_BITMAP)) ;
            break ;

        //----------------------------------------------------------------------
        case 7 :    // FullR
        //----------------------------------------------------------------------

            return (GrB_SUCCESS) ;

        //----------------------------------------------------------------------
        case 8 :    // FullC
        //----------------------------------------------------------------------

            OK (GxB_Vector_Option_set_((GrB_Vector) C,
                GxB_SPARSITY_CONTROL, GxB_FULL)) ;
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

            return (GrB_SUCCESS) ;

        //----------------------------------------------------------------------
        case 1 :    // standard CSC
        //----------------------------------------------------------------------

            OK (GxB_Vector_export_CSC ((GrB_Vector *) &C, &type, &nrows,
                &Ci, &Cx, &Ci_size, &Cx_size, &nvals, &jumbled, NULL)) ;

            OK (GxB_Vector_import_CSC ((GrB_Vector *) &C, type, nrows,
                &Ci, &Cx, Ci_size, Cx_size, nvals, jumbled, NULL)) ;

            break ;

        //----------------------------------------------------------------------
        case 2 :    // hypersparse CSR
        //----------------------------------------------------------------------

            return (GrB_SUCCESS) ;

        //----------------------------------------------------------------------
        case 3 :    // hypersparse CSC
        //----------------------------------------------------------------------

            return (GrB_SUCCESS) ;

        //----------------------------------------------------------------------
        case 4 :    // bitmapR
        //----------------------------------------------------------------------

            return (GrB_SUCCESS) ;

        //----------------------------------------------------------------------
        case 5 :    // bitmapC
        //----------------------------------------------------------------------

            OK (GxB_Vector_export_Bitmap ((GrB_Vector *) &C, &type, &nrows,
                &Cb, &Cx, &Cb_size, &Cx_size, &nvals, NULL)) ;

            OK (GxB_Vector_import_Bitmap ((GrB_Vector *) &C, type, nrows,
                &Cb, &Cx, Cb_size, Cx_size, nvals, NULL)) ;

            break ;

        //----------------------------------------------------------------------
        case 6 :    // FullR
        //----------------------------------------------------------------------

            return (GrB_SUCCESS) ;

        //----------------------------------------------------------------------
        case 7 :    // FullC
        //----------------------------------------------------------------------

            OK (GxB_Vector_export_Full ((GrB_Vector *) &C, &type, &nrows,
                &Cx, &Cx_size, NULL)) ;

            OK (GxB_Vector_import_Full ((GrB_Vector *) &C, type, nrows,
                &Cx, Cx_size, NULL)) ;

            break ;

        //----------------------------------------------------------------------
        case 8 :    // standard CSR, not jumbled
        //----------------------------------------------------------------------

            return (GrB_SUCCESS) ;

        //----------------------------------------------------------------------
        case 9 :    // standard CSC, not jumbled
        //----------------------------------------------------------------------

            OK (GxB_Vector_export_CSC ((GrB_Vector *) &C, &type, &nrows,
                &Ci, &Cx, &Ci_size, &Cx_size, &nvals, NULL, NULL)) ;

            OK (GxB_Vector_import_CSC ((GrB_Vector *) &C, type, nrows,
                &Ci, &Cx, Ci_size, Cx_size, nvals, false, NULL)) ;

            break ;

        //----------------------------------------------------------------------
        case 10 :    // hypersparse CSR, not jumbled
        //----------------------------------------------------------------------

            return (GrB_SUCCESS) ;

        //----------------------------------------------------------------------
        case 11 :    // hypersparse CSC, not jumbled
        //----------------------------------------------------------------------

            return (GrB_SUCCESS) ;

        default : mexErrMsgTxt ("invalid format") ;
    }

    return (GrB_SUCCESS) ;
}

