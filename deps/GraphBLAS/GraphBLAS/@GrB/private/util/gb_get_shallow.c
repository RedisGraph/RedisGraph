//------------------------------------------------------------------------------
// gb_get_shallow: create a shallow copy of a MATLAB sparse matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// A = gb_get_shallow (X) constructs a shallow GrB_Matrix from a MATLAB
// mxArray, which can either be a MATLAB sparse matrix (double, complex, or
// logical) or a MATLAB struct that contains a GraphBLAS matrix.

// X must not be NULL, but it can be an empty matrix, as X = [ ] or even X = ''
// (the empty string).  In this case, A is returned as NULL.  This is not an
// error here, since the caller might be getting an optional input matrix, such
// as Cin or the Mask.

#include "gb_matlab.h"

#define IF(error,message) \
    CHECK_ERROR (error, "invalid GraphBLAS struct (" message ")" ) ;

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
        mxArray *mx_type = mxGetField (X, 0, "GraphBLAS") ;
        CHECK_ERROR (mx_type == NULL, "not a GraphBLAS struct") ;
        GrB_Type type = gb_mxstring_to_type (mx_type) ;

        // allocate the header, with no content
        OK (GrB_Matrix_new (&A, type, 0, 0)) ;
        gb_mxfree (&(A->p)) ;
        gb_mxfree (&(A->h)) ;
        gb_mxfree (&(A->i)) ;
        gb_mxfree (&(A->x)) ;

        // get the scalar info
        mxArray *opaque = mxGetField (X, 0, "s") ;
        IF (opaque == NULL, ".s missing") ;
        double *s = mxGetDoubles (opaque) ;
        A->hyper_ratio   = s [0] ;
        A->plen          = (int64_t) s [1] ;
        A->vlen          = (int64_t) s [2] ;
        A->vdim          = (int64_t) s [3] ;
        A->nvec          = (int64_t) s [4] ;
        A->nvec_nonempty = (int64_t) s [5] ;
        A->is_hyper      = (int64_t) s [6] ;
        A->is_csc        = (int64_t) s [7] ;    // format already defined
        A->nzmax         = (int64_t) s [8] ;

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

        // get the values
        mxArray *Ax = mxGetField (X, 0, "x") ;
        IF (Ax == NULL, ".x missing") ;
        IF (mxGetM (Ax) != 1, ".x wrong size") ;
        IF (mxGetN (Ax) != MAX (A->type_size*A->nzmax, 1), ".x wrong size") ;
        A->x = (A->nzmax == 0) ? NULL : ((void *) mxGetUint8s (Ax)) ;
        IF (A->x == NULL && A->nzmax > 0, ".x wrong type") ;

        A->h = NULL ;
        if (A->is_hyper)
        { 
            // get the hyperlist
            mxArray *Ah = mxGetField (X, 0, "h") ;
            IF (Ah == NULL, ".h missing") ;
            IF (mxGetM (Ah) != 1, ".h wrong size") ;
            IF (mxGetN (Ah) != MAX (A->plen, 1), ".h wrong size") ;
            A->h = (void *) mxGetInt64s (Ah) ;
            IF (A->h == NULL, ".h wrong type") ;
        }

        // tell GraphBLAS the matrix is shallow
        A->p_shallow = true ;
        A->i_shallow = (A->i != NULL) ;
        A->x_shallow = (A->x != NULL) ;
        A->h_shallow = (A->h != NULL) ;

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
            // X is a MATLAB dense matrix; create a partially shallow
            // GrB_Matrix copy by allocating the row indices Xi and pointers Xp
            // but keeping Xx shallow.
            nzmax = MAX (nrows * ncols, 1) ;
            Xp = (GrB_Index *) mxMalloc ((ncols+1) * sizeof (GrB_Index)) ;
            Xi = (GrB_Index *) mxMalloc (nzmax * sizeof (GrB_Index)) ;
            GB_matlab_helper2 (Xp, Xi, (int64_t) ncols, (int64_t) nrows) ;
        }

        // get the numeric data
        void *Xx = NULL ;
        if (type == GrB_FP64)
        { 
            // MATLAB sparse or dense double matrix
            Xx = mxGetDoubles (X) ;
        }
        #ifdef GB_COMPLEX_TYPE
        else if (type == gb_complex_type)
        {
            // MATLAB sparse or dense double complex matrix
            Xx = mxGetComplexDoubles (X) ;
        }
        #endif
        else if (type == GrB_BOOL)
        { 
            // MATLAB sparse or dense logical matrix
            Xx = mxGetData (X) ;
        }
        else
        {
            // MATLAB does not support any other kinds of sparse matrices
            if (X_is_sparse)
            {
                ERROR ("unsupported type") ;
            }
            else if (type == GrB_INT8)
            { 
                Xx = mxGetInt8s (X) ;
            }
            else if (type == GrB_INT16)
            { 
                Xx = mxGetInt16s (X) ;
            }
            else if (type == GrB_INT32)
            { 
                Xx = mxGetInt32s (X) ;
            }
            else if (type == GrB_INT64)
            { 
                Xx = mxGetInt64s (X) ;
            }
            else if (type == GrB_UINT8)
            { 
                Xx = mxGetUint8s (X) ;
            }
            else if (type == GrB_UINT16)
            { 
                Xx = mxGetUint16s (X) ;
            }
            else if (type == GrB_UINT32)
            { 
                Xx = mxGetUint32s (X) ;
            }
            else if (type == GrB_UINT64)
            { 
                Xx = mxGetUint64s (X) ;
            }
            else if (type == GrB_FP32)
            { 
                Xx = mxGetSingles (X) ;
            }
            else
            {
                ERROR ("unsupported type") ;
            }
        }

        // import the matrix in CSC format.  This sets Xp, Xi, and Xx to NULL,
        // but it does not change the MATLAB matrix they came from.
        OK (GxB_Matrix_import_CSC (&A, type, nrows, ncols, nzmax, -1,
            &Xp, &Xi, &Xx, NULL)) ;

        // tell GraphBLAS the matrix is shallow
        if (X_is_sparse)
        { 
            A->p_shallow = true ;
            A->i_shallow = (A->i != NULL) ;
        }
        else
        { 
            A->p_shallow = false ;
            A->i_shallow = false ;
        }
        A->h_shallow = (A->h != NULL) ;
        A->x_shallow = (A->x != NULL) ;
    }

    //--------------------------------------------------------------------------
    // return the result
    //--------------------------------------------------------------------------

    return (A) ;
}

