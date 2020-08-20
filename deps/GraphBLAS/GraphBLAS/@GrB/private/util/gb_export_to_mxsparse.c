//------------------------------------------------------------------------------
// gb_export_to_mxsparse: export a GrB_Matrix to a MATLAB sparse matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// The input GrB_Matrix A is exported to a MATLAB sparse mxArray S, and freed.

// The input GrB_Matrix A may be shallow or deep.  The output is a standard
// MATLAB sparse matrix as an mxArray.

#include "gb_matlab.h"

mxArray *gb_export_to_mxsparse  // return exported MATLAB sparse matrix S
(
    GrB_Matrix *A_handle        // matrix to export; freed on output
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    CHECK_ERROR (A_handle == NULL || (*A_handle) == NULL, "internal error 2") ;

    //--------------------------------------------------------------------------
    // typecast to a native MATLAB sparse type and free A
    //--------------------------------------------------------------------------

    GrB_Matrix T ;              // T will always be deep
    GrB_Type type ;
    OK (GxB_Matrix_type (&type, *A_handle)) ;
    GxB_Format_Value fmt ;
    OK (GxB_Matrix_Option_get (*A_handle, GxB_FORMAT, &fmt)) ;

    if (fmt == GxB_BY_COL && (type == GrB_BOOL || type == GrB_FP64
        #ifdef GB_COMPLEX_TYPE
        || type == gb_complex_type
        #endif
        ))
    {

        //----------------------------------------------------------------------
        // A is already in a native MATLAB sparse matrix type, by column
        //----------------------------------------------------------------------

        if (gb_is_shallow (*A_handle))
        { 
            // A is shallow so make a deep copy
            OK (GrB_Matrix_dup (&T, *A_handle)) ;
            OK (GrB_Matrix_free (A_handle)) ;
        }
        else
        { 
            // A is already deep; just transplant it into T
            T = (*A_handle) ;
            (*A_handle) = NULL ;
        }

    }
    else
    { 

        //----------------------------------------------------------------------
        // typecast A to double, and format by column
        //----------------------------------------------------------------------

        // MATLAB supports only logical, double, and double complex sparse
        // matrices.  These correspond to GrB_BOOL, GrB_FP64, and
        // gb_complex_type, respectively.  A is typecasted to double, and
        // converted to CSC format if not already in that format.

        T = gb_typecast (GrB_FP64, GxB_BY_COL, *A_handle) ;
        OK (GrB_Matrix_free (A_handle)) ;
    }

    // ensure T is deep
    CHECK_ERROR (gb_is_shallow (T), "internal error 7") ;

    //--------------------------------------------------------------------------
    // drop zeros from T
    //--------------------------------------------------------------------------

    OK (GxB_Matrix_select (T, NULL, NULL, GxB_NONZERO, T, NULL, NULL)) ;

    //--------------------------------------------------------------------------
    // create the new MATLAB sparse matrix
    //--------------------------------------------------------------------------

    GrB_Index nrows, ncols, nvals ;
    OK (GrB_Matrix_nvals (&nvals, T)) ;
    OK (GrB_Matrix_nrows (&nrows, T)) ;
    OK (GrB_Matrix_ncols (&ncols, T)) ;

    mxArray *S ;

    if (nvals == 0)
    {

        //----------------------------------------------------------------------
        // allocate an empty sparse matrix of the right type and size
        //----------------------------------------------------------------------

        if (type == GrB_BOOL)
        { 
            S = mxCreateSparseLogicalMatrix (nrows, ncols, 1) ;
        }
        #ifdef GB_COMPLEX_TYPE
        else if (type == gb_complex_type)
        {
            S = mxCreateSparse (nrows, ncols, 1, mxCOMPLEX) ;
        }
        #endif
        else
        { 
            S = mxCreateSparse (nrows, ncols, 1, mxREAL) ;
        }

    }
    else
    {

        //----------------------------------------------------------------------
        // export the content of T
        //----------------------------------------------------------------------

        GrB_Index nzmax ;
        int64_t nonempty, *Tp, *Ti ;
        void *Tx ;

        OK (GxB_Matrix_export_CSC (&T, &type, &nrows, &ncols, &nzmax, &nonempty,
            &Tp, &Ti, &Tx, NULL)) ;

        CHECK_ERROR (nzmax == 0, "internal error 8") ;
        CHECK_ERROR (Tp == NULL || Ti == NULL || Tx == NULL,
            "internal error 9") ;

        //----------------------------------------------------------------------
        // allocate an empty sparse matrix of the right type, then set content
        //----------------------------------------------------------------------

        if (type == GrB_BOOL)
        { 
            S = mxCreateSparseLogicalMatrix (0, 0, 1) ;
        }
        #ifdef GB_COMPLEX_TYPE
        else if (type == gb_complex_type)
        {
            S = mxCreateSparse (0, 0, 1, mxCOMPLEX) ;
        }
        #endif
        else
        { 
            S = mxCreateSparse (0, 0, 1, mxREAL) ;
        }

        // set the size
        mxSetM (S, nrows) ;
        mxSetN (S, ncols) ;
        mxSetNzmax (S, nzmax) ;

        // set the column pointers
        void *p = mxGetJc (S) ;
        gb_mxfree (&p) ;
        mxSetJc (S, Tp) ;

        // set the row indices
        p = mxGetIr (S) ;
        gb_mxfree (&p) ;
        mxSetIr (S, Ti) ;

        // set the values
        if (type == GrB_BOOL)
        { 
            p = mxGetData (S) ;
            gb_mxfree (&p) ;
            mxSetData (S, Tx) ;
        }
        #ifdef GB_COMPLEX_TYPE
        else if (type == gb_complex_type)
        {
            p = mxGetComplexDoubles (S) ;
            gb_mxfree (&p) ;
            mxSetComplexDoubles (S, Tx) ;
        }
        #endif
        else
        { 
            p = mxGetDoubles (S) ;
            gb_mxfree (&p) ;
            mxSetDoubles (S, Tx) ;
        }
    }

    //--------------------------------------------------------------------------
    // return the new MATLAB sparse matrix
    //--------------------------------------------------------------------------

    return (S) ;
}

