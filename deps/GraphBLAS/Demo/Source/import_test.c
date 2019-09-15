//------------------------------------------------------------------------------
// SuiteSparse/GraphBLAS/Demo/Source/import_test: test import/export
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "demos.h"

#if defined __INTEL_COMPILER
#pragma warning (disable: 556)
#endif

#define FREE_ALL                    \
{                                   \
    GrB_free (C_handle) ;           \
    if (Ap != NULL) free (Ap) ;     \
    if (Ah != NULL) free (Ah) ;     \
    if (Ai != NULL) free (Ai) ;     \
    if (Aj != NULL) free (Aj) ;     \
    if (Ax != NULL) free (Ax) ;     \
}

// typecast the values to the exported type
#define GETVAL \
{                                                               \
    if      (type == GrB_BOOL  ) Ax_bool   = (bool     *) Ax ;  \
    else if (type == GrB_INT8  ) Ax_int8   = (int8_t   *) Ax ;  \
    else if (type == GrB_INT16 ) Ax_int16  = (int16_t  *) Ax ;  \
    else if (type == GrB_INT32 ) Ax_int32  = (int32_t  *) Ax ;  \
    else if (type == GrB_INT64 ) Ax_int64  = (int64_t  *) Ax ;  \
    else if (type == GrB_UINT8 ) Ax_uint8  = (uint8_t  *) Ax ;  \
    else if (type == GrB_UINT16) Ax_uint16 = (uint16_t *) Ax ;  \
    else if (type == GrB_UINT32) Ax_uint32 = (uint32_t *) Ax ;  \
    else if (type == GrB_UINT64) Ax_uint64 = (uint64_t *) Ax ;  \
    else if (type == GrB_FP32  ) Ax_fp32   = (float    *) Ax ;  \
    else if (type == GrB_FP64  ) Ax_fp64   = (double   *) Ax ;  \
    else return (GrB_INVALID_VALUE) ;                           \
}

// print a value
#define PRINTVAL(p) \
{                                                                        \
    if      (type == GrB_BOOL  ) printf ("%g", (double) Ax_bool   [p]) ; \
    else if (type == GrB_INT8  ) printf ("%g", (double) Ax_int8   [p]) ; \
    else if (type == GrB_INT16 ) printf ("%g", (double) Ax_int16  [p]) ; \
    else if (type == GrB_INT32 ) printf ("%g", (double) Ax_int32  [p]) ; \
    else if (type == GrB_INT64 ) printf ("%g", (double) Ax_int64  [p]) ; \
    else if (type == GrB_UINT8 ) printf ("%g", (double) Ax_uint8  [p]) ; \
    else if (type == GrB_UINT16) printf ("%g", (double) Ax_uint16 [p]) ; \
    else if (type == GrB_UINT32) printf ("%g", (double) Ax_uint32 [p]) ; \
    else if (type == GrB_UINT64) printf ("%g", (double) Ax_uint64 [p]) ; \
    else if (type == GrB_FP32  ) printf ("%g", (double) Ax_fp32   [p]) ; \
    else if (type == GrB_FP64  ) printf ("%g", (double) Ax_fp64   [p]) ; \
}

#include "../Source/GB.h"

GrB_Info import_test (GrB_Matrix *C_handle, int format, bool dump)
{
    
    GrB_Type type ;
    GrB_Index nrows, ncols, nvals, nvec ;
    GrB_Index *Ap = NULL, *Ah = NULL, *Ai = NULL, *Aj = NULL ;
    int64_t nonempty ;

    void *Ax = NULL ;

    bool     *Ax_bool   = NULL ;
    int8_t   *Ax_int8   = NULL ;
    int16_t  *Ax_int16  = NULL ;
    int32_t  *Ax_int32  = NULL ;
    int32_t  *Ax_int64  = NULL ;
    uint8_t  *Ax_uint8  = NULL ;
    uint16_t *Ax_uint16 = NULL ;
    uint32_t *Ax_uint32 = NULL ;
    uint32_t *Ax_uint64 = NULL ;
    float    *Ax_fp32   = NULL ;
    double   *Ax_fp64   = NULL ;

    GrB_Info info = GrB_SUCCESS ;

    printf ("\n========================= import_test: format %d\n", format) ;
    OK (GxB_Matrix_fprint (*(C_handle), "C to export",
        dump ? GxB_COMPLETE : GxB_SHORT, stdout)) ;

    switch (format)
    {

        //----------------------------------------------------------------------
        case 0 : 
        //----------------------------------------------------------------------

            OK (GxB_Matrix_export_CSR (C_handle, &type, &nrows, &ncols,
                &nvals, &nonempty, &Ap, &Aj, &Ax, NULL)) ;

            // the export destroys the matrix (*C_handle), returning its
            // contents in Ap, Aj, and Ax.
            CHECK (*C_handle == NULL, GrB_INVALID_VALUE) ;

            if (dump)
            {
                printf ("export standard CSR: %g-by-%g, nvals %g:\n",
                    (double) nrows, (double) ncols, (double) nvals) ;
                OK (GxB_Type_fprint (type, "type", GxB_COMPLETE, stdout)) ;
                GETVAL ;
                printf ("Ap %p Aj %p Ax %p\n", (void *) Ap, (void *) Aj, Ax) ;

                for (int64_t i = 0 ; i < nrows ; i++)
                {
                    printf ("Row %g\n", (double) i) ;
                    for (int64_t p = Ap [i] ; p < Ap [i+1] ; p++)
                    {
                        printf ("  col %g value ", (double) Aj [p]) ;
                        PRINTVAL (p) ;
                        printf ("\n") ;
                    }
                }
            }

            // reimport the matrix
            OK (GxB_Matrix_import_CSR (C_handle, type, nrows, ncols,
                nvals, nonempty, &Ap, &Aj, &Ax, NULL)) ;

            OK (GxB_Matrix_fprint ((*C_handle), "C reimported",
                dump ? GxB_COMPLETE : GxB_SILENT, stdout)) ;
            break ;

        //----------------------------------------------------------------------
        case 1 : 
        //----------------------------------------------------------------------

            OK (GxB_Matrix_export_CSC (C_handle, &type, &nrows, &ncols,
                &nvals, &nonempty, &Ap, &Ai, &Ax, NULL)) ;

            CHECK (*C_handle == NULL, GrB_INVALID_VALUE) ;

            if (dump)
            {
                printf ("export standard CSC: %g-by-%g, nvals %g:\n",
                    (double) nrows, (double) ncols, (double) nvals) ;
                OK (GxB_Type_fprint (type, "type", GxB_COMPLETE, stdout)) ;
                GETVAL ;

                for (int64_t j = 0 ; j < ncols ; j++)
                {
                    printf ("Col %g\n", (double) j) ;
                    for (int64_t p = Ap [j] ; p < Ap [j+1] ; p++)
                    {
                        printf ("  row %g value ", (double) Ai [p]) ;
                        PRINTVAL (p) ;  // print Ax [p]
                        printf ("\n") ;
                    }
                }

            }

            OK (GxB_Matrix_import_CSC (C_handle, type, nrows, ncols,
                nvals, nonempty, &Ap, &Ai, &Ax, NULL)) ;

            OK (GxB_Matrix_fprint ((*C_handle), "C reimported",
                dump ? GxB_COMPLETE : GxB_SILENT, stdout)) ;
            break ;

        //----------------------------------------------------------------------
        case 2 : 
        //----------------------------------------------------------------------

            OK (GxB_Matrix_export_HyperCSR (C_handle, &type, &nrows, &ncols,
                &nvals, &nonempty, &nvec, &Ah, &Ap, &Aj, &Ax, NULL)) ;

            CHECK (*C_handle == NULL, GrB_INVALID_VALUE) ;

            if (dump)
            {
                printf ("export hyper CSR: %g-by-%g, nvals %g, nvec %g:\n",
                (double) nrows, (double) ncols, (double) nvals, (double) nvec) ;
                OK (GxB_Type_fprint (type, "type", GxB_COMPLETE, stdout)) ;
                GETVAL ;

                for (int64_t k = 0 ; k < nvec ; k++)
                {
                    int64_t i = Ah [k] ;
                    printf ("Row %g\n", (double) i) ;
                    for (int64_t p = Ap [k] ; p < Ap [k+1] ; p++)
                    {
                        printf ("  col %g value ", (double) Aj [p]) ;
                        PRINTVAL (p) ;
                        printf ("\n") ;
                    }
                }
            }

            OK (GxB_Matrix_import_HyperCSR (C_handle, type, nrows, ncols,
                nvals, nonempty, nvec, &Ah, &Ap, &Aj, &Ax, NULL)) ;

            OK (GxB_Matrix_fprint ((*C_handle), "C reimported",
                dump ? GxB_COMPLETE : GxB_SILENT, stdout)) ;
            break ;

        //----------------------------------------------------------------------
        case 3 : 
        //----------------------------------------------------------------------

            OK (GxB_Matrix_export_HyperCSC (C_handle, &type, &nrows, &ncols,
                &nvals, &nonempty, &nvec, &Ah, &Ap, &Ai, &Ax, NULL)) ;

            CHECK (*C_handle == NULL, GrB_INVALID_VALUE) ;

            if (dump)
            {
                printf ("export hyper CSC: %g-by-%g, nvals %g, nvec %g:\n",
                (double) nrows, (double) ncols, (double) nvals, (double) nvec) ;
                OK (GxB_Type_fprint (type, "type", GxB_COMPLETE, stdout)) ;
                GETVAL ;

                for (int64_t k = 0 ; k < nvec ; k++)
                {
                    int64_t j = Ah [k] ;
                    printf ("Col %g\n", (double) j) ;
                    for (int64_t p = Ap [k] ; p < Ap [k+1] ; p++)
                    {
                        printf ("  row %g value ", (double) Ai [p]) ;
                        PRINTVAL (p) ;
                        printf ("\n") ;
                    }
                }
            }

            OK (GxB_Matrix_import_HyperCSC (C_handle, type, nrows, ncols,
                nvals, nonempty, nvec, &Ah, &Ap, &Ai, &Ax, NULL)) ;

            OK (GxB_Matrix_fprint ((*C_handle), "C reimported",
                dump ? GxB_COMPLETE : GxB_SILENT, stdout)) ;
            break ;

        //----------------------------------------------------------------------
        default : 
        //----------------------------------------------------------------------

            printf ("bad format\n") ;
            return (GrB_INVALID_VALUE) ;
            break ;
    }

    return (GrB_SUCCESS) ;
}

