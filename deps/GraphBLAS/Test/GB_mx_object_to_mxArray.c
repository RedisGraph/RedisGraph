//------------------------------------------------------------------------------
// GB_mx_object_to_mxArray
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// Convert a GraphBLAS sparse or full matrix to a MATLAB struct C containing
// C.matrix and a string C.class.  The GraphBLAS matrix is destroyed.

// This could be done using only user-callable GraphBLAS functions, by
// extracting the tuples and converting them into a MATLAB sparse matrix.  But
// that would be much slower and take more memory.  Instead, most of the work
// can be done by pointers, and directly accessing the internal contents of C.
// If C has type GB_BOOL_code or GB_FP64_code, then C can be converted to a
// MATLAB matrix in constant time with essentially no extra memory allocated.
// This is faster, but it means that this MATLAB interface will only work with
// this specific implementation of GraphBLAS.

// Note that the GraphBLAS matrix may contain explicit zeros.  These entries
// should not appear in a MATLAB matrix but MATLAB handles them without
// difficulty.  They are returned to MATLAB in C.matrix.  If any work is done
// in MATLAB on the matrix, these entries will get dropped.  If they are to be
// preserved, do C.pattern = GB_spones_mex (C.matrix) in MATLAB before
// modifying C.matrix.

#include "GB_mex.h"

static const char *MatrixFields [ ] = { "matrix", "class", "values" } ;

mxArray *GB_mx_object_to_mxArray   // returns the MATLAB mxArray
(
    GrB_Matrix *handle,             // handle of GraphBLAS matrix to convert
    const char *name,
    const bool create_struct        // if true, then return a struct
)
{
    GB_CONTEXT ("GB_mx_object_to_mxArray") ;

    // get the inputs
    mxArray *A, *Astruct, *X = NULL ;
    GrB_Matrix C = *handle ;
    GrB_Type ctype = C->type ;

    // may have pending tuples
    ASSERT_MATRIX_OK (C, name, GB0) ;

    // C must not be shallow
    ASSERT (!C->p_shallow) ;
    ASSERT (!C->h_shallow) ;
    ASSERT (!C->b_shallow) ;
    ASSERT (!C->i_shallow) ;
    ASSERT (!C->x_shallow) ;

    // make sure there are no pending computations
    if (GB_IS_FULL (C) || GB_IS_BITMAP (C))
    {
        ASSERT (!GB_JUMBLED (C)) ;
        ASSERT (!GB_ZOMBIES (C)) ;
        ASSERT (!GB_PENDING (C)) ;
    }
    else
    {
        // this may convert C to full
        GrB_Matrix_wait (&C) ;
        C = (*handle) ;
    }

    // must be done after GrB_Matrix_wait:
    int64_t cnz = GB_NNZ (C) ;

    ASSERT_MATRIX_OK (C, "TO MATLAB after assembling pending tuples", GB0) ;

    // ensure C is sparse or full, not hypersparse or bitmap
    GxB_Matrix_Option_set_(C, GxB_SPARSITY_CONTROL, GxB_FULL + GxB_SPARSE) ;
    ASSERT_MATRIX_OK (C, "TO MATLAB, sparse or full", GB0) ;
    ASSERT (!GB_IS_HYPERSPARSE (C)) ;
    ASSERT (!GB_IS_BITMAP (C)) ;

    // get the current sparsity
    int sparsity ;
    GxB_Matrix_Option_get_(C, GxB_SPARSITY_STATUS, &sparsity) ;
    ASSERT (sparsity == GxB_FULL || sparsity == GxB_SPARSE) ;

    // make sure it's CSC
    if (!C->is_csc)
    {
        GxB_Matrix_Option_set_(C, GxB_FORMAT, GxB_BY_COL) ;
    }

    // setting to CSC may have transposed the matrix
    ASSERT (GB_JUMBLED_OK (C)) ;
    GrB_Matrix_wait (&C) ;
    ASSERT (!GB_JUMBLED (C)) ;
    cnz = GB_NNZ (C) ;

    ASSERT_MATRIX_OK (C, "TO MATLAB, non-hyper CSC", GB0) ;
    ASSERT (!GB_JUMBLED (C)) ;
    ASSERT (!GB_IS_HYPERSPARSE (C)) ;
    ASSERT (!GB_IS_BITMAP (C)) ;
    ASSERT (GB_IS_SPARSE (C) || GB_IS_FULL (C)) ;
    ASSERT (C->is_csc) ;

    // MATLAB doesn't want NULL pointers in its empty matrices
    if (C->x == NULL)
    {
        ASSERT (C->nzmax == 0 && cnz == 0) ;
        C->x = GB_MALLOC (2 * sizeof (double), GB_void) ;
        memset (C->x, 0, 2 * sizeof (double)) ;
        C->x_shallow = false ;
    }

    bool C_is_full = (sparsity == GxB_FULL) ;
    if (!C_is_full)
    {
        // MATLAB doesn't want NULL pointers in its empty sparse matrices
        if (C->i == NULL)
        {
            ASSERT (C->nzmax == 0 && cnz == 0) ;
            C->i = GB_MALLOC (1, int64_t) ;
            C->i [0] = 0 ;
            C->i_shallow = false ;
        }
        if (C->p == NULL)
        {
            ASSERT (C->nzmax == 0 && cnz == 0) ;
            C->p = GB_MALLOC (C->vdim + 1, int64_t) ;
            memset (C->p, 0, (C->vdim + 1) * sizeof (int64_t)) ;
            C->p_shallow = false ;
        }
    }

    C->nzmax = GB_IMAX (C->nzmax, 1) ;

    //--------------------------------------------------------------------------
    // create the MATLAB matrix A and link in the numerical values of C
    //--------------------------------------------------------------------------

    if (C_is_full)
    {
        // C is full.  See gb_export_to_mxfull
        // allocate an empty dense matrix of the right type, then set content

        void *Cx = (void *) C->x ;

        if (ctype == GrB_BOOL)
        { 
            A = mxCreateLogicalMatrix (0, 0) ;
            mxSetData (A, Cx) ;
        }
        else if (ctype == GrB_FP32)
        { 
            A = mxCreateNumericMatrix (0, 0, mxSINGLE_CLASS, mxREAL) ;
            mxSetSingles (A, Cx) ;
        }
        else if (ctype == GrB_FP64)
        { 
            A = mxCreateNumericMatrix (0, 0, mxDOUBLE_CLASS, mxREAL) ;
            mxSetDoubles (A, Cx) ;
        }
        else if (ctype == GrB_INT8)
        { 
            A = mxCreateNumericMatrix (0, 0, mxINT8_CLASS, mxREAL) ;
            mxSetInt8s (A, Cx) ;
        }
        else if (ctype == GrB_INT16)
        { 
            A = mxCreateNumericMatrix (0, 0, mxINT16_CLASS, mxREAL) ;
            mxSetInt16s (A, Cx) ;
        }
        else if (ctype == GrB_INT32)
        { 
            A = mxCreateNumericMatrix (0, 0, mxINT32_CLASS, mxREAL) ;
            mxSetInt32s (A, Cx) ;
        }
        else if (ctype == GrB_INT64)
        { 
            A = mxCreateNumericMatrix (0, 0, mxINT64_CLASS, mxREAL) ;
            mxSetInt64s (A, Cx) ;
        }
        else if (ctype == GrB_UINT8)
        { 
            A = mxCreateNumericMatrix (0, 0, mxUINT8_CLASS, mxREAL) ;
            mxSetUint8s (A, Cx) ;
        }
        else if (ctype == GrB_UINT16)
        { 
            A = mxCreateNumericMatrix (0, 0, mxUINT16_CLASS, mxREAL) ;
            mxSetUint16s (A, Cx) ;
        }
        else if (ctype == GrB_UINT32)
        { 
            A = mxCreateNumericMatrix (0, 0, mxUINT32_CLASS, mxREAL) ;
            mxSetUint32s (A, Cx) ;
        }
        else if (ctype == GrB_UINT64)
        { 
            A = mxCreateNumericMatrix (0, 0, mxUINT64_CLASS, mxREAL) ;
            mxSetUint64s (A, Cx) ;
        }
        else if (ctype == GxB_FC32)
        {
            A = mxCreateNumericMatrix (0, 0, mxSINGLE_CLASS, mxCOMPLEX) ;
            mxSetComplexSingles (A, Cx) ;
        }
        else if (ctype == Complex || ctype == GxB_FC64)
        {
            A = mxCreateNumericMatrix (0, 0, mxDOUBLE_CLASS, mxCOMPLEX) ;
            mxSetComplexDoubles (A, Cx) ;
        }
        else
        {
            mexErrMsgTxt ("... unsupported type") ;
        }

        mexMakeMemoryPersistent (C->x) ;
        C->x_shallow = false ;
        AS_IF_FREE (C->x) ;   // unlink C->x from C since it's now in MATLAB C

    }
    else if (C->type == GrB_BOOL)
    {
        // C is boolean, which is the same as a MATLAB logical sparse matrix
        A = mxCreateSparseLogicalMatrix (0, 0, 0) ;
        mexMakeMemoryPersistent (C->x) ;
        mxSetData (A, (bool *) C->x) ;
        C->x_shallow = false ;

        // C->x is treated as if it was freed
        AS_IF_FREE (C->x) ;   // unlink C->x from C since it's now in MATLAB C

    }
    else if (C->type == GrB_FP64)
    {
        // C is double, which is the same as a MATLAB double sparse matrix
        A = mxCreateSparse (0, 0, 0, mxREAL) ;
        mexMakeMemoryPersistent (C->x) ;
        mxSetData (A, C->x) ;
        C->x_shallow = false ;

        // C->x is treated as if it was freed
        AS_IF_FREE (C->x) ;   // unlink C->x from C since it's now in MATLAB C

    }
    else if (C->type == Complex || C->type == GxB_FC64)
    {

        // user-defined Complex type, or GraphBLAS GxB_FC64
        A = mxCreateSparse (C->vlen, C->vdim, C->nzmax, mxCOMPLEX) ;
        memcpy (mxGetComplexDoubles (A), C->x, cnz * sizeof (GxB_FC64_t)) ;

    }
    else if (C->type == GxB_FC32)
    {

        // C is single complex, typecast to sparse double complex
        A = mxCreateSparse (C->vlen, C->vdim, C->nzmax, mxCOMPLEX) ;
        GB_cast_array (mxGetComplexDoubles (A), GB_FC64_code,
            C->x, C->type->code, NULL, C->type->size, cnz, 1) ;

    }
    else
    {

        // otherwise C is cast into a MATLAB double sparse matrix
        A = mxCreateSparse (0, 0, 0, mxREAL) ;
        double *Sx = GB_MALLOC (cnz+1, double) ;
        GB_cast_array (Sx, GB_FP64_code,
            C->x, C->type->code, NULL, C->type->size, cnz, 1) ;
        mexMakeMemoryPersistent (Sx) ;
        mxSetPr (A, Sx) ;

        // Sx was just malloc'd, and given to MATLAB.  Treat it as if
        // GraphBLAS has freed it
        AS_IF_FREE (Sx) ;

        if (create_struct)
        {
            // If C is int64 or uint64, then typecasting can lose information,
            // so keep an uncasted copy of C->x as well.
            X = GB_mx_create_full (0, 0, C->type) ;
            mxSetM (X, cnz) ;
            mxSetN (X, 1) ;
            mxSetData (X, C->x) ;
            mexMakeMemoryPersistent (C->x) ;
            C->x_shallow = false ;
            // treat C->x as if it were freed
            AS_IF_FREE (C->x) ;
        }
    }

    // set nrows, ncols, nzmax, and the pattern of A
    mxSetM (A, C->vlen) ;
    mxSetN (A, C->vdim) ;
    mxSetNzmax (A, C->nzmax) ;

    if (!C_is_full)
    {
        mxFree (mxGetJc (A)) ;
        mxFree (mxGetIr (A)) ;
        mexMakeMemoryPersistent (C->p) ;
        mexMakeMemoryPersistent (C->i) ;
        mxSetJc (A, (size_t *) C->p) ;
        mxSetIr (A, (size_t *) C->i) ;

        // treat C->p as if freed
        AS_IF_FREE (C->p) ;

        // treat C->i as if freed
        C->i_shallow = false ;
        AS_IF_FREE (C->i) ;
    }

    // free C, but leave any shallow components untouched
    // since these have been transplanted into the MATLAB matrix.
    GrB_Matrix_free_(handle) ;

    if (create_struct)
    {
        // create the type
        mxArray *atype = GB_mx_Type_to_mxstring (ctype) ;
        // create the output struct
        Astruct = mxCreateStructMatrix (1, 1,
           (X == NULL) ? 2 : 3, MatrixFields) ;
        mxSetFieldByNumber (Astruct, 0, 0, A) ;
        mxSetFieldByNumber (Astruct, 0, 1, atype) ;
        if (X != NULL)
        {
            mxSetFieldByNumber (Astruct, 0, 2, X) ;
        }
        return (Astruct) ;
    }
    else
    {
        return (A) ;
    }
}

