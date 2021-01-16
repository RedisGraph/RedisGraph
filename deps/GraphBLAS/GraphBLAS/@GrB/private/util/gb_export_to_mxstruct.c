//------------------------------------------------------------------------------
// gb_export_to_mxstruct: export a GrB_Matrix to a MATLAB struct
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// The input GrB_Matrix A is exported to a GraphBLAS matrix struct G, and freed.

// The input GrB_Matrix A must be deep.  The output is a MATLAB struct
// holding the content of the GrB_Matrix.

#include "gb_matlab.h"

// for hypersparse, sparse, or full matrices
static const char *MatrixFields [6] =
{
    "GraphBLASv4",      // 0: "logical", "int8", ... "double",
                        //    "single complex", or "double complex"
    "s",                // 1: all scalar info goes here
    "x",                // 2: array of uint8, size (sizeof(type) * nzmax)
    "p",                // 3: array of int64_t, size plen+1
    "i",                // 4: array of int64_t, size nzmax
    "h"                 // 5: array of int64_t, size plen if hypersparse
} ;

// for bitmap matrices only
static const char *Bitmap_MatrixFields [4] =
{
    "GraphBLASv4",      // 0: "logical", "int8", ... "double",
                        //    "single complex", or "double complex"
    "s",                // 1: all scalar info goes here
    "x",                // 2: array of uint8, size (sizeof(type) * nzmax)
    "b"                 // 3: array of int8_t, size nzmax, for bitmap only
} ;

//------------------------------------------------------------------------------

mxArray *gb_export_to_mxstruct  // return exported MATLAB struct G
(
    GrB_Matrix *A_handle        // matrix to export; freed on output
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    CHECK_ERROR (A_handle == NULL, "matrix missing") ;

    GrB_Matrix T = NULL ;
    if (GB_is_shallow (*A_handle))
    {
        // A is shallow so make a deep copy
        OK (GrB_Matrix_dup (&T, *A_handle)) ;
        OK (GrB_Matrix_free (A_handle)) ;
        (*A_handle) = T ;
    }

    GrB_Matrix A = (*A_handle) ;

    //--------------------------------------------------------------------------
    // make sure the matrix is finished
    //--------------------------------------------------------------------------

    OK1 (A, GrB_Matrix_wait (&A)) ;

    //--------------------------------------------------------------------------
    // construct the output struct
    //--------------------------------------------------------------------------

    int sparsity ;
    OK (GxB_Matrix_Option_get (A, GxB_SPARSITY_STATUS, &sparsity)) ;
    mxArray *G ;

    switch (sparsity)
    {
        case GxB_FULL :
            // A is full, with 3 fields: GraphBLASv4, s, x
            G = mxCreateStructMatrix (1, 1, 3, MatrixFields) ;
            break ;

        case GxB_SPARSE :
            // A is sparse, with 5 fields: GraphBLASv4, s, x, p, i
            G = mxCreateStructMatrix (1, 1, 5, MatrixFields) ;
            break ;

        case GxB_HYPERSPARSE :
            // A is hypersparse, with 6 fields: GraphBLASv4, s, x, p, i, h
            G = mxCreateStructMatrix (1, 1, 6, MatrixFields) ;
            break ;

        case GxB_BITMAP :
            // A is bitmap, with 4 fields: GraphBLASv4, s, x, b
            G = mxCreateStructMatrix (1, 1, 4, Bitmap_MatrixFields) ;
            break ;

        default : ERROR ("invalid GraphBLAS struct") ;
    }

    //--------------------------------------------------------------------------
    // export content into the output struct
    //--------------------------------------------------------------------------

    // export the GraphBLAS type as a string
    mxSetFieldByNumber (G, 0, 0, gb_type_to_mxstring (A->type)) ;

    // export the scalar content
    mxArray *opaque = mxCreateNumericMatrix (1, 9, mxINT64_CLASS, mxREAL) ;
    int64_t *s = mxGetInt64s (opaque) ;
    s [0] = A->plen ;
    s [1] = A->vlen ;
    s [2] = A->vdim ;
    s [3] = A->nvec ;
    s [4] = A->nvec_nonempty ;
    s [5] = A->sparsity ;
    s [6] = (int64_t) (A->is_csc) ;
    s [7] = A->nzmax ;
    s [8] = A->nvals ;
    mxSetFieldByNumber (G, 0, 1, opaque) ;

    // These components do not need to be exported: Pending, nzombies,
    // queue_next, queue_head, enqueued, *_shallow, jumbled, logger,
    // hyper_switch, bitmap_switch.

    if (sparsity == GxB_SPARSE || sparsity == GxB_HYPERSPARSE)
    {
        // export the pointers
        mxArray *Ap = mxCreateNumericMatrix (1, 1, mxINT64_CLASS, mxREAL) ;
        mxSetN (Ap, A->plen+1) ;
        void *p = mxGetInt64s (Ap) ;
        gb_mxfree (&p) ;
        mxSetInt64s (Ap, A->p) ;
        mxSetFieldByNumber (G, 0, 3, Ap) ;

        // export the indices
        mxArray *Ai = mxCreateNumericMatrix (1, 1, mxINT64_CLASS, mxREAL) ;
        if (A->nzmax > 0)
        { 
            mxSetN (Ai, A->nzmax) ;
            p = mxGetInt64s (Ai) ;
            gb_mxfree (&p) ;
            mxSetInt64s (Ai, A->i) ;
        }
        mxSetFieldByNumber (G, 0, 4, Ai) ;
    }

    // export the values as uint8
    mxArray *Ax = mxCreateNumericMatrix (1, 1, mxUINT8_CLASS, mxREAL) ;
    if (A->nzmax > 0)
    { 
        mxSetN (Ax, A->nzmax * A->type->size) ;
        void *p = mxGetUint8s (Ax) ;
        gb_mxfree (&p) ;
        mxSetUint8s (Ax, A->x) ;
    }
    mxSetFieldByNumber (G, 0, 2, Ax) ;

    if (sparsity == GxB_HYPERSPARSE)
    {
        // export the hyperlist
        if (A->nvec < A->plen)
        { 
            // This is unused space in A->h; it might contain garbage, so
            // set it to zero to simplify the view of the MATLAB struct.
            A->h [A->nvec] = 0 ;
        }
        mxArray *Ah = mxCreateNumericMatrix (1, 1, mxINT64_CLASS, mxREAL) ;
        if (A->plen > 0)
        { 
            mxSetN (Ah, A->plen) ;
            void *p = mxGetInt64s (Ah) ;
            gb_mxfree (&p) ;
            mxSetInt64s (Ah, A->h) ;
        }
        mxSetFieldByNumber (G, 0, 5, Ah) ;
    }

    if (sparsity == GxB_BITMAP)
    { 
        // export the bitmap
        mxArray *Ab = mxCreateNumericMatrix (1, 1, mxINT8_CLASS, mxREAL) ;
        if (A->nzmax > 0)
        { 
            mxSetN (Ab, A->nzmax) ;
            void *p = mxGetInt8s (Ab) ;
            gb_mxfree (&p) ;
            mxSetInt8s (Ab, A->b) ;
        }
        mxSetFieldByNumber (G, 0, 3, Ab) ;
    }

    //--------------------------------------------------------------------------
    // free the header of A
    //--------------------------------------------------------------------------

    A->p = NULL ;
    A->h = NULL ;
    A->b = NULL ;
    A->i = NULL ;
    A->x = NULL ;
    OK (GrB_Matrix_free (A_handle)) ;

    //--------------------------------------------------------------------------
    // return the MATLAB struct
    //--------------------------------------------------------------------------

    return (G) ;
}

