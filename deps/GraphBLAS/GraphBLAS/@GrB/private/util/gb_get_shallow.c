//------------------------------------------------------------------------------
// gb_get_shallow: create a shallow copy of a MATLAB sparse matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// A = gb_get_shallow (X) constructs a shallow GrB_Matrix from a MATLAB
// mxArray, which can either be a MATLAB sparse matrix (double, complex, or
// logical) or a MATLAB struct that contains a GraphBLAS matrix.

// X must not be NULL, but it can be an empty matrix, as X = [ ] or even X = ''
// (the empty string).  In this case, A is returned as NULL.  This is not an
// error here, since the caller might be getting an optional input matrix, such
// as Cin or the Mask.

// FUTURE: it would be better to use the GxB* import/export functions,
// instead of accessing the opaque content of the GrB_Matrix directly.

#include "gb_matlab.h"

#define IF(error,message) \
    CHECK_ERROR (error, "invalid GraphBLASv4 struct (" message ")" ) ;

GrB_Matrix gb_get_shallow   // return a shallow copy of MATLAB sparse matrix
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

    GrB_Matrix A ;

    if (gb_mxarray_is_empty (X))
    { 

        //----------------------------------------------------------------------
        // matrix is empty
        //----------------------------------------------------------------------

        // X is a 0-by-0 MATLAB matrix.  Create a new 0-by-0 matrix of the same
        // type as X, with the default format.
        OK (GrB_Matrix_new (&A, gb_mxarray_type (X), 0, 0)) ;

    }
    else if (mxIsStruct (X))
    { 

        //----------------------------------------------------------------------
        // construct a shallow GrB_Matrix copy from a MATLAB struct
        //----------------------------------------------------------------------

        // get the type
        mxArray *mx_type = mxGetField (X, 0, "GraphBLASv4") ;
        CHECK_ERROR (mx_type == NULL, "not a GraphBLASv4 struct") ;
        GrB_Type type = gb_mxstring_to_type (mx_type) ;

        // allocate the header, with no content
        OK (GrB_Matrix_new (&A, type, 0, 0)) ;
        gb_mxfree (&(A->p)) ;
        gb_mxfree (&(A->h)) ;
        gb_mxfree (&(A->i)) ;
        gb_mxfree (&(A->x)) ;
        gb_mxfree (&(A->b)) ;

        // get the scalar info
        mxArray *opaque = mxGetField (X, 0, "s") ;
        IF (opaque == NULL, ".s missing") ;
        IF (mxGetM (opaque) != 1, ".s wrong size") ;
        IF (mxGetN (opaque) != 9, ".s wrong size") ;
        int64_t *s = mxGetInt64s (opaque) ;
        A->hyper_switch  = GB_Global_hyper_switch_get ( ) ;
        A->plen          = s [0] ;
        A->vlen          = s [1] ;
        A->vdim          = s [2] ;
        A->nvec          = s [3] ;
        A->nvec_nonempty = s [4] ;
        A->sparsity      = (int) (s [5]) ;
        A->is_csc        = (bool) (s [6]) ;
        A->nzmax         = s [7] ;
        A->nvals         = s [8] ;
        A->bitmap_switch = GB_Global_bitmap_switch_matrix_get (A->vlen, A->vdim) ;

        int nfields = mxGetNumberOfFields (X) ;

        if (nfields == 5 || nfields == 6)
        {
            // A is hypersparse or sparse
            // get the pointers
            mxArray *Ap = mxGetField (X, 0, "p") ;
            IF (Ap == NULL, ".p missing") ;
            IF (mxGetM (Ap) != 1, ".p wrong size") ;
            IF (mxGetN (Ap) != A->plen+1, ".p wrong size") ;
            A->p = mxGetInt64s (Ap) ;
            IF (A->p == NULL, ".p wrong type") ;
            // get the indices
            mxArray *Ai = mxGetField (X, 0, "i") ;
            IF (Ai == NULL, ".i missing") ;
            IF (mxGetM (Ai) != 1, ".i wrong size") ;
            IF (mxGetN (Ai) != MAX (A->nzmax, 1), ".i wrong size") ;
            A->i = (A->nzmax == 0) ? NULL : mxGetInt64s (Ai) ;
            IF (A->i == NULL && A->nzmax > 0, ".i wrong type") ;
        }

        // get the values
        mxArray *Ax = mxGetField (X, 0, "x") ;
        IF (Ax == NULL, ".x missing") ;
        IF (mxGetM (Ax) != 1, ".x wrong size") ;
        IF (mxGetN (Ax) != MAX (A->type->size*A->nzmax, 1), ".x wrong size") ;
        A->x = (A->nzmax == 0) ? NULL : ((void *) mxGetUint8s (Ax)) ;
        IF (A->x == NULL && A->nzmax > 0, ".x wrong type") ;

        A->h = NULL ;
        if (nfields == 6)
        { 
            // A is hypersparse
            // get the hyperlist
            mxArray *Ah = mxGetField (X, 0, "h") ;
            IF (Ah == NULL, ".h missing") ;
            IF (mxGetM (Ah) != 1, ".h wrong size") ;
            IF (mxGetN (Ah) != MAX (A->plen, 1), ".h wrong size") ;
            A->h = (void *) mxGetInt64s (Ah) ;
            IF (A->h == NULL, ".h wrong type") ;
        }

        A->b = NULL ;
        if (nfields == 4)
        { 
            // A is bitmap
            // get the bitmap
            mxArray *Ab = mxGetField (X, 0, "b") ;
            IF (Ab == NULL, ".b missing") ;
            IF (mxGetM (Ab) != 1, ".b wrong size") ;
            IF (mxGetN (Ab) != MAX (A->nzmax, 1), ".b wrong size") ;
            A->b = (void *) mxGetInt8s (Ab) ;
            IF (A->b == NULL, ".b wrong type") ;
        }

        // tell GraphBLAS the matrix is shallow
        A->p_shallow = (A->p != NULL) ;
        A->i_shallow = (A->i != NULL) ;
        A->x_shallow = (A->x != NULL) ;
        A->h_shallow = (A->h != NULL) ;
        A->b_shallow = (A->b != NULL) ;

        // matrix is now initialized
        A->magic = GB_MAGIC ;

    }
    else
    {

        //----------------------------------------------------------------------
        // construct a shallow GrB_Matrix copy of a MATLAB matrix
        //----------------------------------------------------------------------

        // get the type and dimensions
        bool X_is_sparse = mxIsSparse (X) ;

        GrB_Type type = gb_mxarray_type (X) ;
        GrB_Index nrows = (GrB_Index) mxGetM (X) ;
        GrB_Index ncols = (GrB_Index) mxGetN (X) ;

        // get Xp, Xi, nzmax, or create them
        GrB_Index *Xp, *Xi, nzmax ;
        if (X_is_sparse)
        { 
            // get the nzmax, Xp, and Xi from the MATLAB sparse matrix X
            nzmax = (GrB_Index) mxGetNzmax (X) ;
            Xp = (GrB_Index *) mxGetJc (X) ;
            Xi = (GrB_Index *) mxGetIr (X) ;
        }
        else
        { 
            // X is a MATLAB full matrix; so is the GrB_Matrix
            nzmax = nrows * ncols ;
            Xp = NULL ;
            Xi = NULL ;
        }

        // get the numeric data
        void *Xx = NULL ;
        if (type == GrB_FP64)
        { 
            // MATLAB sparse or full double matrix
            Xx = mxGetDoubles (X) ;
        }
        else if (type == GxB_FC64)
        { 
            // MATLAB sparse or full double complex matrix
            Xx = mxGetComplexDoubles (X) ;
        }
        else if (type == GrB_BOOL)
        { 
            // MATLAB sparse or full logical matrix
            Xx = mxGetData (X) ;
        }
        else if (X_is_sparse)
        {
            // MATLAB does not support any other kinds of sparse matrices
            ERROR ("unsupported type") ;
        }
        else if (type == GrB_INT8)
        { 
            // full int8 matrix
            Xx = mxGetInt8s (X) ;
        }
        else if (type == GrB_INT16)
        { 
            // full int16 matrix
            Xx = mxGetInt16s (X) ;
        }
        else if (type == GrB_INT32)
        { 
            // full int32 matrix
            Xx = mxGetInt32s (X) ;
        }
        else if (type == GrB_INT64)
        { 
            // full int64 matrix
            Xx = mxGetInt64s (X) ;
        }
        else if (type == GrB_UINT8)
        { 
            // full uint8 matrix
            Xx = mxGetUint8s (X) ;
        }
        else if (type == GrB_UINT16)
        { 
            // full uint16 matrix
            Xx = mxGetUint16s (X) ;
        }
        else if (type == GrB_UINT32)
        { 
            // full uint32 matrix
            Xx = mxGetUint32s (X) ;
        }
        else if (type == GrB_UINT64)
        { 
            // full uint64 matrix
            Xx = mxGetUint64s (X) ;
        }
        else if (type == GrB_FP32)
        { 
            // full single matrix
            Xx = mxGetSingles (X) ;
        }
        else if (type == GxB_FC32)
        { 
            // full single complex matrix
            Xx = mxGetComplexSingles (X) ;
        }
        else
        {
            ERROR ("unsupported type") ;
        }

        if (X_is_sparse)
        { 
            // import the matrix in CSC format.  This sets Xp, Xi, and Xx to
            // NULL, but it does not change the MATLAB matrix they came from.
            OK (GxB_Matrix_import_CSC (&A, type, nrows, ncols,
                &Xp, &Xi, &Xx, ncols+1, nzmax, nzmax, false, NULL)) ;

            // tell GraphBLAS the matrix is shallow
            A->p_shallow = true ;
            A->i_shallow = (A->i != NULL) ;
        }
        else
        { 
            // import a full matrix
            OK (GxB_Matrix_import_FullC (&A, type, nrows, ncols, &Xx,
                nzmax, NULL)) ;
        }

        // tell GraphBLAS the matrix is shallow
        A->b_shallow = (A->b != NULL) ;
        A->h_shallow = (A->h != NULL) ;
        A->x_shallow = (A->x != NULL) ;
    }

    //--------------------------------------------------------------------------
    // return the result
    //--------------------------------------------------------------------------

    return (A) ;
}

