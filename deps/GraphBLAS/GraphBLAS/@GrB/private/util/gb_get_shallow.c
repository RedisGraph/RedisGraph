//------------------------------------------------------------------------------
// gb_get_shallow: create a shallow copy of a built-in sparse matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: GPL-3.0-or-later

//------------------------------------------------------------------------------

// A = gb_get_shallow (X) constructs a shallow GrB_Matrix from a built-in
// mxArray, which can either be a built-in sparse matrix (double, complex, or
// logical) or a built-in struct that contains a GraphBLAS matrix.

// X must not be NULL, but it can be an empty matrix, as X = [ ] or even X = ''
// (the empty string).  In this case, A is returned as NULL.  This is not an
// error here, since the caller might be getting an optional input matrix, such
// as Cin or the Mask.

// For v4, iso is false, and the s component has length 9.
// For v5, iso is present but false, and the s component has length 10.
// For v5_1, iso is true/false, and the s component has length 10.

// mxGetData is used instead of the MATLAB-recommended mxGetDoubles, etc,
// because mxGetData works best for Octave, and it works fine for MATLAB
// since GraphBLAS requires R2018a with the interleaved complex data type.

#include "gb_interface.h"
#include "GB_make_shallow.h"

#define IF(error,message) \
    CHECK_ERROR (error, "invalid GraphBLAS struct (" message ")" ) ;

GrB_Matrix gb_get_shallow   // return a shallow copy of built-in sparse matrix
(
    const mxArray *X
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    CHECK_ERROR (X == NULL, "matrix missing") ;

    //--------------------------------------------------------------------------
    // construct the shallow GrB_Matrix
    //--------------------------------------------------------------------------

    GrB_Matrix A = NULL ;

    if (gb_mxarray_is_empty (X))
    { 

        //----------------------------------------------------------------------
        // matrix is empty
        //----------------------------------------------------------------------

        // X is a 0-by-0 built-in matrix.  Create a new 0-by-0 matrix of the
        // same type as X, with the default format.
        OK (GrB_Matrix_new (&A, gb_mxarray_type (X), 0, 0)) ;

    }
    else if (mxIsStruct (X))
    { 

        //----------------------------------------------------------------------
        // construct a shallow GrB_Matrix copy from a built-in struct
        //----------------------------------------------------------------------

        bool GraphBLASv4 = false ;
        bool GraphBLASv3 = false ;

        // get the type
        mxArray *mx_type = mxGetField (X, 0, "GraphBLASv5_1") ;
        if (mx_type == NULL)
        {
            // check if it is a GraphBLASv5 struct
            mx_type = mxGetField (X, 0, "GraphBLASv5") ;
        }
        if (mx_type == NULL)
        {
            // check if it is a GraphBLASv4 struct
            mx_type = mxGetField (X, 0, "GraphBLASv4") ;
            GraphBLASv4 = true ;
        }
        if (mx_type == NULL)
        {
            // check if it is a GraphBLASv3 struct
            mx_type = mxGetField (X, 0, "GraphBLAS") ;
            GraphBLASv3 = true ;
        }
        CHECK_ERROR (mx_type == NULL, "not a GraphBLAS struct") ;

        GrB_Type type = gb_mxstring_to_type (mx_type) ;
        size_t type_size ;
        OK (GxB_Type_size (&type_size, type)) ;

        // get the scalar info
        mxArray *opaque = mxGetField (X, 0, "s") ;
        IF (opaque == NULL, ".s missing") ;
        IF (mxGetM (opaque) != 1, ".s wrong size") ;
        size_t s_size = mxGetN (opaque) ;
        if (GraphBLASv3)
        {
            IF (s_size != 8, ".s wrong size") ;
        }
        else if (GraphBLASv4)
        {
            IF (s_size != 9, ".s wrong size") ;
        }
        else
        {
            IF (s_size != 10, ".s wrong size") ;
        }
        int64_t *s = (int64_t *) mxGetData (opaque) ;
        int64_t plen          = s [0] ;
        int64_t vlen          = s [1] ;
        int64_t vdim          = s [2] ;
        int64_t nvec          = s [3] ;
        int64_t nvec_nonempty = s [4] ;
        bool    by_col        = (bool) (s [6]) ;
        int64_t nzmax         = s [7] ;

        int sparsity_status, sparsity_control ;
        int64_t nvals ;
        bool iso ;
        if (GraphBLASv3)
        {
            // GraphBLASv3 struct: sparse or hypersparse only
            sparsity_control = GxB_AUTO_SPARSITY ;
            nvals            = 0 ;
            iso              = false ;
        }
        else
        {
            // GraphBLASv4 or v5 struct: sparse, hypersparse, bitmap, or full
            sparsity_control = (int) (s [5]) ;
            nvals            = s [8] ;
            if (GraphBLASv4)
            {
                // GraphBLASv4: iso is always false
                iso = false ;
            }
            else
            {
                // GraphBLASv5 and GraphBLASv5_1: iso is present as s [9]
                // GraphBLASv5: iso is present as s [9] but always false
                iso = (bool) s [9] ;
            }
        }

        int nfields = mxGetNumberOfFields (X) ;
        switch (nfields)
        {
            case 3 :
                // A is full, with 3 fields: GraphBLAS*, s, x
                sparsity_status = GxB_FULL ;
                break ;

            case 5 :
                // A is sparse, with 5 fields: GraphBLAS*, s, x, p, i
                sparsity_status = GxB_SPARSE ;
                break ;

            case 6 :
                // A is hypersparse, with 6 fields: GraphBLAS*, s, x, p, i, h
                sparsity_status = GxB_HYPERSPARSE ;
                break ;

            case 4 :
                // A is bitmap, with 4 fields: GraphBLAS*, s, x, b
                sparsity_status = GxB_BITMAP ;
                break ;

            default : ERROR ("invalid GraphBLAS struct") ;
        }

        // each component
        uint64_t *Ap = NULL, *Ai = NULL, *Ah = NULL ;
        int8_t *Ab = NULL ;
        void *Ax = NULL ;

        // size of each component
        size_t Ap_size = 0 ;
        size_t Ah_size = 0 ;
        size_t Ab_size = 0 ;
        size_t Ai_size = 0 ;
        size_t Ax_size = 0 ;

        if (sparsity_status == GxB_HYPERSPARSE || sparsity_status == GxB_SPARSE)
        {
            // A is hypersparse or sparse

            // get Ap
            mxArray *Ap_mx = mxGetField (X, 0, "p") ;
            IF (Ap_mx == NULL, ".p missing") ;
            IF (mxGetM (Ap_mx) != 1, ".p wrong size") ;
            Ap = (int64_t *) mxGetData (Ap_mx) ;
            IF (Ap == NULL, ".p wrong type") ;
            Ap_size = mxGetN (Ap_mx) * sizeof (int64_t) ;

            // get Ai
            mxArray *Ai_mx = mxGetField (X, 0, "i") ;
            IF (Ai_mx == NULL, ".i missing") ;
            IF (mxGetM (Ai_mx) != 1, ".i wrong size") ;
            Ai_size = mxGetN (Ai_mx) * sizeof (int64_t) ;
            Ai = (Ai_size == 0) ? NULL : ((int64_t *) mxGetData (Ai_mx)) ;
            IF (Ai == NULL && Ai_size > 0, ".i wrong type") ;
        }

        // get the values
        mxArray *Ax_mx = mxGetField (X, 0, "x") ;
        IF (Ax_mx == NULL, ".x missing") ;
        IF (mxGetM (Ax_mx) != 1, ".x wrong size") ;
        Ax_size = mxGetN (Ax_mx) ;
        Ax = (Ax_size == 0) ? NULL : ((void *) mxGetData (Ax_mx)) ;
        IF (Ax == NULL && Ax_size > 0, ".x wrong type") ;

        if (sparsity_status == GxB_HYPERSPARSE)
        { 
            // A is hypersparse
            // get the hyperlist
            mxArray *Ah_mx = mxGetField (X, 0, "h") ;
            IF (Ah_mx == NULL, ".h missing") ;
            IF (mxGetM (Ah_mx) != 1, ".h wrong size") ;
            Ah_size = mxGetN (Ah_mx) * sizeof (int64_t) ;
            Ah = (Ah_size == 0) ? NULL : ((int64_t *) mxGetData (Ah_mx)) ;
            IF (Ah == NULL && Ah_size > 0, ".h wrong type") ;
        }

        if (sparsity_status == GxB_BITMAP)
        { 
            // A is bitmap
            // get the bitmap
            mxArray *Ab_mx = mxGetField (X, 0, "b") ;
            IF (Ab_mx == NULL, ".b missing") ;
            IF (mxGetM (Ab_mx) != 1, ".b wrong size") ;
            Ab_size = mxGetN (Ab_mx) ;
            Ab = (Ab_size == 0) ? NULL : ((int8_t *) mxGetData (Ab_mx)) ;
            IF (Ab == NULL && Ab_size > 0, ".b wrong type") ;
        }

        //----------------------------------------------------------------------
        // import the matrix
        //----------------------------------------------------------------------

        int64_t nrows = (by_col) ? vlen : vdim ;
        int64_t ncols = (by_col) ? vdim : vlen ;
        OK (GrB_Matrix_new (&A, type, nrows, ncols)) ;

        switch (sparsity_status)
        {
            case GxB_FULL :
                if (by_col)
                {
                    OK (GxB_Matrix_pack_FullC (A,
                        &Ax, Ax_size, iso, NULL)) ;
                }
                else
                {
                    OK (GxB_Matrix_pack_FullR (A,
                        &Ax, Ax_size, iso, NULL)) ;
                }
                break ;

            case GxB_SPARSE :
                if (by_col)
                {
                    OK (GxB_Matrix_pack_CSC (A,
                        &Ap, &Ai, &Ax, Ap_size, Ai_size, Ax_size, iso,
                        false, NULL)) ;
                }
                else
                {
                    OK (GxB_Matrix_pack_CSR (A,
                        &Ap, &Ai, &Ax, Ap_size, Ai_size, Ax_size, iso,
                        false, NULL)) ;
                }
                break ;

            case GxB_HYPERSPARSE :
                if (by_col)
                {
                    OK (GxB_Matrix_pack_HyperCSC (A,
                        &Ap, &Ah, &Ai, &Ax,
                        Ap_size, Ah_size, Ai_size, Ax_size, iso,
                        nvec, false, NULL)) ;
                }
                else
                {
                    OK (GxB_Matrix_pack_HyperCSR (A,
                        &Ap, &Ah, &Ai, &Ax,
                        Ap_size, Ah_size, Ai_size, Ax_size, iso,
                        nvec, false, NULL)) ;
                }
                break ;

            case GxB_BITMAP :
                if (by_col)
                {
                    OK (GxB_Matrix_pack_BitmapC (A,
                        &Ab, &Ax, Ab_size, Ax_size, iso, nvals, NULL)) ;
                }
                else
                {
                    OK (GxB_Matrix_pack_BitmapR (A,
                        &Ab, &Ax, Ab_size, Ax_size, iso, nvals, NULL)) ;
                }
                break ;

            default: ;
        }

        // tell GraphBLAS the matrix is shallow
        GB (make_shallow) (A) ;

    }
    else
    {

        //----------------------------------------------------------------------
        // construct a shallow GrB_Matrix copy of a built-in matrix
        //----------------------------------------------------------------------

        // get the type and dimensions
        bool X_is_sparse = mxIsSparse (X) ;

        GrB_Type type = gb_mxarray_type (X) ;
        GrB_Index nrows = (GrB_Index) mxGetM (X) ;
        GrB_Index ncols = (GrB_Index) mxGetN (X) ;
        OK (GrB_Matrix_new (&A, type, nrows, ncols)) ;

        // get Xp, Xi, nzmax, or create them
        GrB_Index *Xp, *Xi, nzmax ;
        if (X_is_sparse)
        { 
            // get the nzmax, Xp, and Xi from the built-in sparse matrix X
            nzmax = (GrB_Index) mxGetNzmax (X) ;
            Xp = (GrB_Index *) mxGetJc (X) ;
            Xi = (GrB_Index *) mxGetIr (X) ;
        }
        else
        { 
            // X is a built-in full matrix; so is the GrB_Matrix
            nzmax = nrows * ncols ;
            Xp = NULL ;
            Xi = NULL ;
        }

        // get the numeric data
        void *Xx = NULL ;
        size_t type_size = 0 ;
        if (type == GrB_FP64)
        { 
            // built-in sparse or full double matrix
            Xx = mxGetData (X) ;
            type_size = sizeof (double) ;
        }
        else if (type == GxB_FC64)
        { 
            // built-in sparse or full double complex matrix
            Xx = mxGetData (X) ;
            type_size = 2 * sizeof (double) ;
        }
        else if (type == GrB_BOOL)
        { 
            // built-in sparse or full logical matrix
            Xx = mxGetData (X) ;
            type_size = sizeof (bool) ;
        }
        else if (X_is_sparse)
        {
            // Built-in sparse matrices do not support any other kinds
            ERROR ("unsupported type") ;
        }
        else if (type == GrB_INT8)
        { 
            // full int8 matrix
            Xx = mxGetData (X) ;
            type_size = sizeof (int8_t) ;
        }
        else if (type == GrB_INT16)
        { 
            // full int16 matrix
            Xx = mxGetData (X) ;
            type_size = sizeof (int16_t) ;
        }
        else if (type == GrB_INT32)
        { 
            // full int32 matrix
            Xx = mxGetData (X) ;
            type_size = sizeof (int32_t) ;
        }
        else if (type == GrB_INT64)
        { 
            // full int64 matrix
            Xx = mxGetData (X) ;
            type_size = sizeof (int64_t) ;
        }
        else if (type == GrB_UINT8)
        { 
            // full uint8 matrix
            Xx = mxGetData (X) ;
            type_size = sizeof (uint8_t) ;
        }
        else if (type == GrB_UINT16)
        { 
            // full uint16 matrix
            Xx = mxGetData (X) ;
            type_size = sizeof (uint16_t) ;
        }
        else if (type == GrB_UINT32)
        { 
            // full uint32 matrix
            Xx = mxGetData (X) ;
            type_size = sizeof (uint32_t) ;
        }
        else if (type == GrB_UINT64)
        { 
            // full uint64 matrix
            Xx = mxGetData (X) ;
            type_size = sizeof (uint64_t) ;
        }
        else if (type == GrB_FP32)
        { 
            // full single matrix
            Xx = mxGetData (X) ;
            type_size = sizeof (float) ;
        }
        else if (type == GxB_FC32)
        { 
            // full single complex matrix
            Xx = mxGetData (X) ;
            type_size = 2 * sizeof (float) ;
        }
        else
        {
            ERROR ("unsupported type") ;
        }

        if (X_is_sparse)
        { 
            // import the matrix in CSC format.  This sets Xp, Xi, and Xx to
            // NULL, but it does not change the built-in matrix they came from.
            OK (GxB_Matrix_pack_CSC (A,
                &Xp, &Xi, &Xx,
                (ncols+1) * sizeof (int64_t),
                nzmax * sizeof (int64_t),
                nzmax * type_size, false, false, NULL)) ;
        }
        else
        { 
            // import a full matrix
            OK (GxB_Matrix_pack_FullC (A,
                &Xx, nzmax * type_size, false, NULL)) ;
        }

        // tell GraphBLAS the matrix is shallow
        GB (make_shallow) (A) ;
    }

    //--------------------------------------------------------------------------
    // return the result
    //--------------------------------------------------------------------------

    return (A) ;
}

