//------------------------------------------------------------------------------
// GB_mex_export: test import/export
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#include "GB_mex.h"

#define USAGE "C = GB_mex_export (C, format, hyper, csc, dump, clear_nvec)"

#define FREE_ALL                        \
{                                       \
    GrB_Matrix_free_(&C) ;              \
    GB_FREE (Ap) ;                      \
    GB_FREE (Ah) ;                      \
    GB_FREE (Ai) ;                      \
    GB_FREE (Aj) ;                      \
    GB_FREE (Ax) ;                      \
    GB_mx_put_global (true) ;           \
}

#define OK(method)                      \
{                                       \
    info = method ;                     \
    if (info != GrB_SUCCESS)            \
    {                                   \
        if (dump) printf ("%d: %d\n", __LINE__, info) ; \
        FREE_ALL ;                      \
        return (info) ;                 \
    }                                   \
}

GrB_Vector v = NULL ;
GrB_Matrix C = NULL ;
GrB_Index *Ap = NULL ;
GrB_Index *Ah = NULL ;
GrB_Index *Ai = NULL ;
GrB_Index *Aj = NULL ;
GrB_Index nrows = 0 ;
GrB_Index ncols = 0 ;
GrB_Index nvals = 0 ;
GrB_Index nvec = 0 ;
GrB_Index Ai_size = 0 ;
GrB_Index Ax_size = 0 ;
GrB_Index Ap_size = 0 ;
GrB_Index Aj_size = 0 ;
GrB_Index Ab_size = 0 ;
GrB_Index Ah_size = 0 ;
int64_t ignore = -1 ;
char *Ax = NULL ;
int format = 0 ;
bool jumbled = false ;
bool is_hyper = false ;
bool clear_nvec = false ;
bool is_csc = true ;
GrB_Info info = GrB_SUCCESS ;
GrB_Descriptor desc = NULL ;
bool dump = false ;
GrB_Type type = NULL ;
size_t asize = 0 ;
GrB_Info import_export (void) ;
GrB_Info import_export2 (void) ;

//------------------------------------------------------------------------------

GrB_Info import_export ( )
{

    OK (GB_Matrix_check (C, "C to export", GB0, stdout)) ;

    //--------------------------------------------------------------------------
    // export/import a vector
    //--------------------------------------------------------------------------

    if (GB_VECTOR_OK (C))
    {
        OK (GxB_Vector_export_CSC ((GrB_Vector *) (&C), &type, &nrows,
            &Ai, &Ax, &Ai_size, &Ax_size, &nvals, &jumbled, desc)) ;

        OK (GxB_Type_size (&asize, type)) ;

        if (dump)
        {
            printf ("export standard CSC vector: %llu-by-1, nvals %llu:\n",
                nrows, nvals) ;
            OK (GB_Type_check (type, "type", GxB_SUMMARY, stdout)) ;
            GB_Type_code code = type->code ;

            for (int64_t p = 0 ; p < nvals ; p++)
            {
                printf ("  row %llu value ", Ai [p]) ;
                GB_code_check (code, Ax + p*asize, 5, stdout) ;
                printf ("\n") ;
            }
        }

        OK (GxB_Vector_import_CSC ((GrB_Vector *) (&C), type, nrows,
            &Ai, &Ax, Ai_size, Ax_size, nvals, jumbled, desc)) ;

        return (GrB_SUCCESS) ;
    }

    //--------------------------------------------------------------------------
    // export/import a matrix
    //--------------------------------------------------------------------------

    switch (format)
    {

        //----------------------------------------------------------------------
        case 0 : 
        //----------------------------------------------------------------------

            OK (GxB_Matrix_export_CSR (&C, &type, &nrows, &ncols,
                    &Ap, &Aj, &Ax, &Ap_size, &Aj_size, &Ax_size,
                    &jumbled, desc)) ;

            OK (GxB_Type_size (&asize, type)) ;
            nvec = nrows ;

            if (dump)
            {
                printf ("\nexport standard CSR: %llu-by-%llu, Ax_size %llu:\n",
                    nrows, ncols, Ax_size) ;
                OK (GB_Type_check (type, "type", GxB_SUMMARY, stdout));
                GB_Type_code code = type->code ;

                for (int64_t i = 0 ; i < nrows ; i++)
                {
                    printf ("Row %lld\n", i) ;
                    for (int64_t p = Ap [i] ; p < Ap [i+1] ; p++)
                    {
                        printf ("  col %llu value ", Aj [p]) ;
                        GB_code_check (code, Ax + p*asize, 5, stdout) ;
                        printf ("\n") ;
                    }
                }
            }

            OK (GxB_Matrix_import_CSR (&C, type, nrows, ncols,
                &Ap, &Aj, &Ax, Ap_size, Aj_size, Ax_size, jumbled, desc)) ;

            OK (GB_Matrix_check (C, "C reimported",
                dump ? GxB_COMPLETE : GxB_SILENT, stdout)) ;
            break ;

        //----------------------------------------------------------------------
        case 1 : 
        //----------------------------------------------------------------------

            OK (GxB_Matrix_export_CSC (&C, &type, &nrows, &ncols,
                    &Ap, &Ai, &Ax, &Ap_size, &Ai_size, &Ax_size,
                    &jumbled, desc)) ;

            nvec = ncols ;
            OK (GxB_Type_size (&asize, type)) ;

            if (dump)
            {
                printf ("\nexport standard CSC: %llu-by-%llu, Ax_size %llu:\n",
                    nrows, ncols, Ax_size) ;
                OK (GB_Type_check (type, "type", GxB_SUMMARY, stdout));
                GB_Type_code code = type->code ;

                for (int64_t j = 0 ; j < ncols ; j++)
                {
                    printf ("Col %lld\n", j) ;
                    for (int64_t p = Ap [j] ; p < Ap [j+1] ; p++)
                    {
                        printf ("  row %llu value ", Ai [p]) ;
                        GB_code_check (code, Ax + p*asize, 5, stdout) ;
                        printf ("\n") ;
                    }
                }
            }

            OK (GxB_Matrix_import_CSC (&C, type, nrows, ncols,
                &Ap, &Ai, &Ax, Ap_size, Ai_size, Ax_size, jumbled, desc)) ;

            OK (GB_Matrix_check (C, "C reimported",
                dump ? GxB_COMPLETE : GxB_SILENT, stdout)) ;
            break ;

        //----------------------------------------------------------------------
        case 2 : 
        //----------------------------------------------------------------------

            OK (GxB_Matrix_export_HyperCSR (&C, &type, &nrows, &ncols,
                &Ap, &Ah, &Aj, &Ax, &Ap_size, &Ah_size, &Aj_size, &Ax_size,
                &nvec, &jumbled, desc)) ;

            OK (GxB_Type_size (&asize, type)) ;

            if (dump)
            {
                printf ("\nexport hyper CSR: %llu-by-%llu, Ax_size %llu, "
                    "nvec %llu:\n", nrows, ncols, Ax_size, nvec) ;
                OK (GB_Type_check (type, "type", GxB_SUMMARY, stdout));
                GB_Type_code code = type->code ;

                for (int64_t k = 0 ; k < nvec ; k++)
                {
                    int64_t i = Ah [k] ;
                    printf ("Row %lld\n", i) ;
                    for (int64_t p = Ap [k] ; p < Ap [k+1] ; p++)
                    {
                        printf ("  col %llu value ", Aj [p]) ;
                        GB_code_check (code, Ax + p*asize, 5, stdout) ;
                        printf ("\n") ;
                    }
                }
            }

            OK (GxB_Matrix_import_HyperCSR (&C, type, nrows, ncols,
                &Ap, &Ah, &Aj, &Ax, Ap_size, Ah_size, Aj_size, Ax_size,
                nvec, jumbled, desc)) ;

            OK (GB_Matrix_check (C, "C reimported",
                dump ? GxB_COMPLETE : GxB_SILENT, stdout));
            break ;

        //----------------------------------------------------------------------
        case 3 : 
        //----------------------------------------------------------------------

            OK (GxB_Matrix_export_HyperCSC (&C, &type, &nrows, &ncols,
                &Ap, &Ah, &Ai, &Ax, &Ap_size, &Ah_size, &Ai_size, &Ax_size,
                &nvec, &jumbled, desc)) ;

            OK (GxB_Type_size (&asize, type)) ;

            if (dump)
            {
                printf ("export hyper CSC: %llu-by-%llu, Ax_size %llu, "
                    "c %llu:\n", nrows, ncols, Ax_size, nvec) ;
                OK (GB_Type_check (type, "type", GxB_SUMMARY, stdout));
                GB_Type_code code = type->code ;

                for (int64_t k = 0 ; k < nvec ; k++)
                {
                    int64_t j = Ah [k] ;
                    printf ("Col %lld\n", j) ;
                    for (int64_t p = Ap [k] ; p < Ap [k+1] ; p++)
                    {
                        printf ("  row %llu value ", Ai [p]) ;
                        GB_code_check (code, Ax + p*asize, 5, stdout) ;
                        printf ("\n") ;
                    }
                }
            }

            OK (GxB_Matrix_import_HyperCSC (&C, type, nrows, ncols,
                &Ap, &Ah, &Ai, &Ax, Ap_size, Ah_size, Ai_size, Ax_size,
                nvec, jumbled, desc)) ;

            OK (GB_Matrix_check (C, "C reimported",
                dump ? GxB_COMPLETE : GxB_SILENT, stdout)) ;
            break ;

        //----------------------------------------------------------------------
        default : 
        //----------------------------------------------------------------------

            mexErrMsgTxt ("bad format") ;
            break ;
    }
    return (GrB_SUCCESS) ;
}

//------------------------------------------------------------------------------

GrB_Info import_export2 (void)
{
    OK (import_export ( )) ;
    OK (import_export ( )) ;
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
    if (nargout > 1 || nargin < 1 || nargin > 6)
    {
        mexErrMsgTxt ("Usage: " USAGE) ;
    }

    // get format for import/export
    GET_SCALAR (1, int, format, 0) ;

    // get hyper flag 
    GET_SCALAR (2, bool, is_hyper, false) ;

    // get csc flag 
    GET_SCALAR (3, bool, is_csc, true) ;

    // get dump flag 
    GET_SCALAR (4, bool, dump, false) ;

    // get clear_nvec flag 
    GET_SCALAR (5, bool, clear_nvec, false) ;

    // get C (make a deep copy)
    #define GET_DEEP_COPY \
    {                                                                       \
        C = GB_mx_mxArray_to_Matrix (pargin [0], "C input", true, true) ;   \
        if (!is_csc)                                                        \
        {                                                                   \
            /* convert C to CSR */                                          \
            GB_transpose (NULL, NULL, false, C, NULL, NULL, NULL, false,    \
                NULL) ;                                                     \
        }                                                                   \
        if (is_hyper && !GB_IS_FULL (C))                                    \
        {                                                                   \
            /* convert C to hypersparse */                                  \
            GB_convert_sparse_to_hyper (C, NULL) ;                          \
        }                                                                   \
    }
    #define FREE_DEEP_COPY GrB_Matrix_free_(&C) ;
    GET_DEEP_COPY ;
    if (C == NULL)
    {
        FREE_ALL ;
        mexErrMsgTxt ("C failed") ;
    }

    // import/export
    METHOD (import_export2 ( )) ;

    // return C to MATLAB
    pargout [0] = GB_mx_Matrix_to_mxArray (&C, "C export/import", false) ;

    FREE_ALL ;
}

