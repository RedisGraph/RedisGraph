//------------------------------------------------------------------------------
// GB_mex_about4: still more basic tests
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// Test lots of random stuff.  The function otherwise serves no purpose.

#include "GB_mex.h"
#include "GB_mex_errors.h"

#define USAGE "GB_mex_about4"

void mexFunction
(
    int nargout,
    mxArray *pargout [ ],
    int nargin,
    const mxArray *pargin [ ]
)
{

    GrB_Info info ;
    GrB_Matrix C = NULL, A = NULL, M = NULL, S = NULL ;
    GrB_Descriptor desc = NULL ;
    GrB_Vector w = NULL ;
    char *err ;

    //--------------------------------------------------------------------------
    // startup GraphBLAS
    //--------------------------------------------------------------------------

    bool malloc_debug = GB_mx_get_global (true) ;
    int expected = GrB_SUCCESS ;
    double ttt = GB_Global_get_wtime ( ) ;

    //--------------------------------------------------------------------------
    // pack/unpack
    //--------------------------------------------------------------------------

    int pr = GxB_SILENT ;

    GrB_Index m = 4, n = 5 ;
    OK (GrB_Matrix_new (&S, GrB_FP64, m, n)) ;
    OK (GxB_Matrix_Option_set (S, GxB_FORMAT, GxB_BY_ROW)) ;
    double x = 0 ;
    for (int i = 0 ; i < m ; i++)
    {
        for (int j = 0 ; j < n ; j++)
        {
            OK (GrB_Matrix_setElement_FP64 (S, x,
                (GrB_Index) i, (GrB_Index) j)) ;
            x++ ;
        }
    }
    OK (GrB_Matrix_wait (&S)) ;
    OK (GxB_Matrix_fprint (S, "initial S by row", pr, NULL)) ;

    double *Cx = NULL ;
    GrB_Index *Cp = NULL, *Ch = NULL, *Ci = NULL ;
    int8_t *Cb = NULL ;
    GrB_Index Cp_size = 0, Ch_size = 0, Cb_size = 0, Ci_size = 0, Cx_size = 0 ;
    bool C_iso = false ;
    bool jumbled = false ;
    GrB_Index nrows = 0, ncols = 0, nvals = 0, nvec = 0 ;

    // full (row to col)
    OK (GrB_Matrix_dup (&C, S)) ;
    OK (GxB_Matrix_unpack_FullR (C, &Cx, &Cx_size, &C_iso, NULL)) ;
    OK (GxB_Matrix_fprint (C, "unpacked C by row", pr, NULL)) ;
    for (int k = 0 ; k < m*n ; k++)
    {
        CHECK (Cx [k] == (double) k) ;
    }
    OK (GxB_Matrix_pack_FullC (C, &Cx, Cx_size, C_iso, NULL)) ;
    OK (GxB_Matrix_fprint (C, "packed C by col", pr, NULL)) ;
    CHECK (Cx == NULL) ;
    OK (GrB_Matrix_nrows (&nrows, C)) ;
    OK (GrB_Matrix_ncols (&ncols, C)) ;
    CHECK (nrows == m) ;
    CHECK (ncols == n) ;
    OK (GrB_Matrix_free (&C)) ;

    // full (col to row)
    OK (GrB_Matrix_dup (&C, S)) ;
    OK (GxB_Matrix_unpack_FullC (C, &Cx, &Cx_size, &C_iso, NULL)) ;
    OK (GxB_Matrix_fprint (C, "unpacked C by col", pr, NULL)) ;
    for (int k = 0 ; k < m*n ; k++)
    {
        int i = k % m ;
        int j = k / m ;
        CHECK (Cx [k] == (double) (i * n + j)) ;
    }
    OK (GxB_Matrix_pack_FullR (C, &Cx, Cx_size, C_iso, NULL)) ;
    OK (GxB_Matrix_fprint (C, "packed C by row", pr, NULL)) ;
    CHECK (Cx == NULL) ;
    OK (GrB_Matrix_nrows (&nrows, C)) ;
    OK (GrB_Matrix_ncols (&ncols, C)) ;
    CHECK (nrows == m) ;
    CHECK (ncols == n) ;
    OK (GrB_Matrix_free (&C)) ;

    // bitmap (row to col)
    OK (GrB_Matrix_dup (&C, S)) ;
    OK (GxB_Matrix_unpack_BitmapR (C, &Cb, &Cx, &Cb_size, &Cx_size,
        &C_iso, &nvals, NULL)) ;
    OK (GxB_Matrix_fprint (C, "unpacked C by row", pr, NULL)) ;
    for (int k = 0 ; k < m*n ; k++)
    {
        CHECK (Cx [k] == (double) k) ;
    }
    OK (GxB_Matrix_pack_BitmapC (C, &Cb, &Cx, Cb_size, Cx_size,
        C_iso, nvals, NULL)) ;
    OK (GxB_Matrix_fprint (C, "packed C by col", pr, NULL)) ;
    CHECK (Cx == NULL) ;
    OK (GrB_Matrix_nrows (&nrows, C)) ;
    OK (GrB_Matrix_ncols (&ncols, C)) ;
    CHECK (nrows == m) ;
    CHECK (ncols == n) ;
    OK (GrB_Matrix_free (&C)) ;

    // bitmap (col to row)
    OK (GrB_Matrix_dup (&C, S)) ;
    OK (GxB_Matrix_unpack_BitmapC (C, &Cb, &Cx, &Cb_size, &Cx_size,
        &C_iso, &nvals, NULL)) ;
    OK (GxB_Matrix_fprint (C, "unpacked C by col", pr, NULL)) ;
    for (int k = 0 ; k < m*n ; k++)
    {
        int i = k % m ;
        int j = k / m ;
        CHECK (Cx [k] == (double) (i * n + j)) ;
    }
    OK (GxB_Matrix_pack_BitmapR (C, &Cb, &Cx, Cb_size, Cx_size,
        C_iso, nvals, NULL)) ;
    OK (GxB_Matrix_fprint (C, "packed C by row", pr, NULL)) ;
    CHECK (Cx == NULL) ;
    OK (GrB_Matrix_nrows (&nrows, C)) ;
    OK (GrB_Matrix_ncols (&ncols, C)) ;
    CHECK (nrows == m) ;
    CHECK (ncols == n) ;
    OK (GrB_Matrix_free (&C)) ;

    // sparse (row)
    OK (GrB_Matrix_dup (&C, S)) ;
    OK (GxB_Matrix_unpack_CSR (C, &Cp, &Ci, &Cx, &Cp_size, &Ci_size, &Cx_size,
        &C_iso, &jumbled, NULL)) ;
    OK (GxB_Matrix_fprint (C, "unpacked C by row", pr, NULL)) ;
    for (int k = 0 ; k < m*n ; k++)
    {
        CHECK (Cx [k] == (double) k) ;
    }
    OK (GxB_Matrix_Option_set (C, GxB_FORMAT, GxB_BY_COL)) ;
    OK (GxB_Matrix_pack_CSR (C, &Cp, &Ci, &Cx, Cp_size, Ci_size, Cx_size,
        C_iso, jumbled, NULL)) ;
    OK (GxB_Matrix_fprint (C, "packed C by row", pr, NULL)) ;
    CHECK (Cx == NULL) ;
    OK (GrB_Matrix_nrows (&nrows, C)) ;
    OK (GrB_Matrix_ncols (&ncols, C)) ;
    CHECK (nrows == m) ;
    CHECK (ncols == n) ;
    OK (GrB_Matrix_free (&C)) ;

    // sparse (col)
    OK (GrB_Matrix_dup (&C, S)) ;
    OK (GxB_Matrix_unpack_CSC (C, &Cp, &Ci, &Cx, &Cp_size, &Ci_size, &Cx_size,
        &C_iso, &jumbled, NULL)) ;
    OK (GxB_Matrix_fprint (C, "unpacked C by col", pr, NULL)) ;
    for (int k = 0 ; k < m*n ; k++)
    {
        int i = k % m ;
        int j = k / m ;
        CHECK (Cx [k] == (double) (i * n + j)) ;
    }
    OK (GxB_Matrix_Option_set (C, GxB_FORMAT, GxB_BY_ROW)) ;
    OK (GxB_Matrix_pack_CSC (C, &Cp, &Ci, &Cx, Cp_size, Ci_size, Cx_size,
        C_iso, jumbled, NULL)) ;
    OK (GxB_Matrix_fprint (C, "packed C by col", pr, NULL)) ;
    CHECK (Cx == NULL) ;
    OK (GrB_Matrix_nrows (&nrows, C)) ;
    OK (GrB_Matrix_ncols (&ncols, C)) ;
    CHECK (nrows == m) ;
    CHECK (ncols == n) ;
    OK (GrB_Matrix_free (&C)) ;

    // hypersparse (row)
    OK (GrB_Matrix_dup (&C, S)) ;
    OK (GxB_Matrix_unpack_HyperCSR (C, &Cp, &Ch, &Ci, &Cx, &Cp_size, &Ch_size,
        &Ci_size, &Cx_size, &C_iso, &nvec, &jumbled, NULL)) ;
    OK (GxB_Matrix_fprint (C, "unpacked C by row", pr, NULL)) ;
    for (int k = 0 ; k < m*n ; k++)
    {
        CHECK (Cx [k] == (double) k) ;
    }
    OK (GxB_Matrix_Option_set (C, GxB_FORMAT, GxB_BY_COL)) ;
    OK (GxB_Matrix_pack_HyperCSR (C, &Cp, &Ch, &Ci, &Cx, Cp_size, Ch_size, 
        Ci_size, Cx_size, C_iso, nvec, jumbled, NULL)) ;
    OK (GxB_Matrix_fprint (C, "packed C by row", pr, NULL)) ;
    CHECK (Cx == NULL) ;
    OK (GrB_Matrix_nrows (&nrows, C)) ;
    OK (GrB_Matrix_ncols (&ncols, C)) ;
    CHECK (nrows == m) ;
    CHECK (ncols == n) ;
    OK (GrB_Matrix_free (&C)) ;

    // hypersparse (col)
    OK (GrB_Matrix_dup (&C, S)) ;
    OK (GxB_Matrix_unpack_HyperCSC (C, &Cp, &Ch, &Ci, &Cx, &Cp_size, &Ch_size,
        &Ci_size, &Cx_size, &C_iso, &nvec, &jumbled, NULL)) ;
    OK (GxB_Matrix_fprint (C, "unpacked C by col", pr, NULL)) ;
    for (int k = 0 ; k < m*n ; k++)
    {
        int i = k % m ;
        int j = k / m ;
        CHECK (Cx [k] == (double) (i * n + j)) ;
    }
    OK (GxB_Matrix_Option_set (C, GxB_FORMAT, GxB_BY_ROW)) ;
    OK (GxB_Matrix_pack_HyperCSC (C, &Cp, &Ch, &Ci, &Cx, Cp_size, Ch_size, 
        Ci_size, Cx_size, C_iso, nvec, jumbled, NULL)) ;
    OK (GxB_Matrix_fprint (C, "packed C by col", pr, NULL)) ;
    CHECK (Cx == NULL) ;
    OK (GrB_Matrix_nrows (&nrows, C)) ;
    OK (GrB_Matrix_ncols (&ncols, C)) ;
    CHECK (nrows == m) ;
    CHECK (ncols == n) ;
    OK (GrB_Matrix_free (&C)) ;

    OK (GrB_Matrix_free (&S)) ;

    //--------------------------------------------------------------------------
    // wrapup
    //--------------------------------------------------------------------------

    GB_mx_put_global (true) ;   
    ttt = GB_Global_get_wtime ( ) - ttt ;
    printf ("\nGB_mex_about4: all tests passed, time: %g\n\n", ttt) ;
}

