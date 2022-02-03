//------------------------------------------------------------------------------
// GB_mex_mxv_iterator: Y = A*X using an iterator
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// A and X must have the same type.  If boolean, the lor_land semiring is used.
// Otherwise, the plus_times semiring is used for the given type.

#include "GB_mex.h"
#include "GB_mex_errors.h"

#define USAGE "Y = GB_mex_mxv_iterator (A, X, kind)"

#define FREE_ALL                                    \
{                                                   \
    GrB_Vector_free_(&X) ;                          \
    GrB_Vector_free_(&Y) ;                          \
    GrB_Matrix_free_(&A) ;                          \
    GxB_Iterator_free (&iterator) ;                 \
    GB_mx_put_global (true) ;                       \
}

#define Assert(x)                                   \
{                                                   \
    if (!(x))                                       \
    {                                               \
        printf ("Failure at %d\n", __LINE__) ;      \
        mexErrMsgTxt ("fail") ;                     \
    }                                               \
}

void mexFunction
(
    int nargout,
    mxArray *pargout [ ],
    int nargin,
    const mxArray *pargin [ ]
)
{

    GrB_Info info ;
    bool malloc_debug = GB_mx_get_global (true) ;
    GrB_Vector Y = NULL, X = NULL ;
    GrB_Matrix A = NULL ;
    GxB_Iterator iterator = NULL ;

    // check inputs
    if (nargout > 1 || nargin < 2 || nargin > 3)
    {
        mexErrMsgTxt ("Usage: " USAGE) ;
    }

    // get A (shallow copy)
    A = GB_mx_mxArray_to_Matrix (pargin [0], "A input", false, true) ;
    if (A == NULL)
    {
        FREE_ALL ;
        mexErrMsgTxt ("A failed") ;
    }
    GrB_Index nrows, ncols ;
    OK (GrB_Matrix_nrows (&nrows, A)) ;
    OK (GrB_Matrix_ncols (&ncols, A)) ;
    GB_Global_print_one_based_set (0) ;
    // GxB_print (A, 3) ;

    // get X (shallow copy)
    X = GB_mx_mxArray_to_Vector (pargin [1], "X input", false, true) ;
    if (X == NULL)
    {
        FREE_ALL ;
        mexErrMsgTxt ("X failed") ;
    }
    // GxB_print (X, 3) ;

    GrB_Index n ;
    OK (GrB_Vector_size (&n, X)) ;
    if (n != ncols)
    {
        FREE_ALL ;
        mexErrMsgTxt ("X has the wrong size") ;
    }

    GrB_Type type = A->type ;
    if (type != X->type)
    {
        FREE_ALL ;
        mexErrMsgTxt ("A and X must have the same type") ;
    }

    // get kind: 0: by row, 1: by col, otherwise: by entry
    int GET_SCALAR (2, int, kind, 0) ;

    // make sure X is full
    int sparsity ;
    OK (GxB_Vector_Option_get (X, GxB_SPARSITY_STATUS, &sparsity)) ;
    if (sparsity != GxB_FULL)
    {
        FREE_ALL ;
        mexErrMsgTxt ("X must be full") ;
    }

    bool user_complex = (Complex != GxB_FC64) && (type == Complex) ;

    // create Y and make it full: all zero but non-iso
    OK (GrB_Vector_new (&Y, type, nrows)) ;
    if (user_complex)
    {
        double z [2] ;
        z [0] = 0 ;
        z [1] = 0 ;
        OK (GrB_Vector_assign_UDT (Y, NULL, NULL, z, GrB_ALL, nrows, NULL)) ;
        z [0] = 1 ;
        OK (GrB_Vector_setElement_UDT (Y, z, 0)) ;
        z [0] = 0 ;
        OK (GrB_Vector_setElement_UDT (Y, z, 0)) ;
    }
    else
    {
        OK (GrB_Vector_assign_INT32 (Y, NULL, NULL, 0, GrB_ALL, nrows, NULL)) ;
        OK (GrB_Vector_setElement_INT32 (Y, 1, 0)) ;
        OK (GrB_Vector_setElement_INT32 (Y, 0, 0)) ;
    }
    OK (GrB_Vector_wait (Y, GrB_MATERIALIZE)) ;

    //--------------------------------------------------------------------------
    // MULTADD: Y (i) += A(i,j) * X(j)
    //--------------------------------------------------------------------------

    #define MULTADD                                                         \
    {                                                                       \
        if (type == GrB_BOOL)                                               \
        {                                                                   \
            bool aij = GxB_Iterator_get_BOOL (iterator) ;                   \
            ((bool *) Y->x) [i] |= aij && ((bool *) X->x) [j] ;             \
        }                                                                   \
        else if (type == GrB_INT8)                                          \
        {                                                                   \
            int8_t aij = GxB_Iterator_get_INT8 (iterator) ;                 \
            ((int8_t *) Y->x) [i] += aij * ((int8_t *) X->x) [j] ;          \
        }                                                                   \
        else if (type == GrB_INT16)                                         \
        {                                                                   \
            int16_t aij = GxB_Iterator_get_INT16 (iterator) ;               \
            ((int16_t *) Y->x) [i] += aij * ((int16_t *) X->x) [j] ;        \
        }                                                                   \
        else if (type == GrB_INT32)                                         \
        {                                                                   \
            int32_t aij = GxB_Iterator_get_INT32 (iterator) ;               \
            ((int32_t *) Y->x) [i] += aij * ((int32_t *) X->x) [j] ;        \
        }                                                                   \
        else if (type == GrB_INT64)                                         \
        {                                                                   \
            int64_t aij = GxB_Iterator_get_INT64 (iterator) ;               \
            ((int64_t *) Y->x) [i] += aij * ((int64_t *) X->x) [j] ;        \
        }                                                                   \
        else if (type == GrB_UINT8)                                         \
        {                                                                   \
            uint8_t aij = GxB_Iterator_get_UINT8 (iterator) ;               \
            ((uint8_t *) Y->x) [i] += aij * ((uint8_t *) X->x) [j] ;        \
        }                                                                   \
        else if (type == GrB_UINT16)                                        \
        {                                                                   \
            uint16_t aij = GxB_Iterator_get_UINT16 (iterator) ;             \
            ((uint16_t *) Y->x) [i] += aij * ((uint16_t *) X->x) [j] ;      \
        }                                                                   \
        else if (type == GrB_UINT32)                                        \
        {                                                                   \
            uint32_t aij = GxB_Iterator_get_UINT32 (iterator) ;             \
            ((uint32_t *) Y->x) [i] += aij * ((uint32_t *) X->x) [j] ;      \
        }                                                                   \
        else if (type == GrB_UINT64)                                        \
        {                                                                   \
            uint64_t aij = GxB_Iterator_get_UINT64 (iterator) ;             \
            ((uint64_t *) Y->x) [i] += aij * ((uint64_t *) X->x) [j] ;      \
        }                                                                   \
        else if (type == GrB_FP32)                                          \
        {                                                                   \
            float aij = GxB_Iterator_get_FP32 (iterator) ;                  \
            ((float *) Y->x) [i] += aij * ((float *) X->x) [j] ;            \
        }                                                                   \
        else if (type == GrB_FP64)                                          \
        {                                                                   \
            double aij = GxB_Iterator_get_FP64 (iterator) ;                 \
            ((double *) Y->x) [i] += aij * ((double *) X->x) [j] ;          \
        }                                                                   \
        else if (type == GxB_FC32)                                          \
        {                                                                   \
            GxB_FC32_t aij = GxB_Iterator_get_FC32 (iterator) ;             \
            ((GxB_FC32_t *) Y->x) [i] += aij * ((GxB_FC32_t *) X->x) [j] ;  \
        }                                                                   \
        else if (type == GxB_FC64)                                          \
        {                                                                   \
            GxB_FC64_t aij = GxB_Iterator_get_FC64 (iterator) ;             \
            ((GxB_FC64_t *) Y->x) [i] += aij * ((GxB_FC64_t *) X->x) [j] ;  \
        }                                                                   \
        else if (type == Complex)                                           \
        {                                                                   \
            GxB_FC64_t aij ;                                                \
            GxB_Iterator_get_UDT (iterator, &aij) ;                         \
            ((GxB_FC64_t *) Y->x) [i] += aij * ((GxB_FC64_t *) X->x) [j] ;  \
        }                                                                   \
        else                                                                \
        {                                                                   \
            mexErrMsgTxt ("type unknown") ;                                 \
        }                                                                   \
    }

    //--------------------------------------------------------------------------
    // Y += A*X using an iterator
    //--------------------------------------------------------------------------

    // create an iterator
    OK (GxB_Iterator_new (&iterator)) ;

    if (kind == 0)
    {

        //----------------------------------------------------------------------
        // Y = A*X using a row iterator
        //----------------------------------------------------------------------

        // GxB_print (A, 3) ;

        // attach it to the matrix A
        OK (GxB_rowIterator_attach (iterator, A, NULL)) ;
        // get the kount
        GrB_Index kount2 = 0 ;
        GrB_Index kount1 = GxB_rowIterator_kount (iterator) ;
        // seek to A(0,:)
        info = GxB_rowIterator_seekRow (iterator, 0) ;
        OK (info) ;
        while (info != GxB_EXHAUSTED)
        {
            // iterate over entries in A(i,:)
            kount2++ ;
            GrB_Index i = GxB_rowIterator_getRowIndex (iterator) ;
            Assert (i >= 0 && i < nrows) ;
            while (info == GrB_SUCCESS)
            {
                // get the entry A(i,j)
                GrB_Index j = GxB_rowIterator_getColIndex (iterator) ;
                Assert (j >= 0 && j < ncols) ;
                // Y (i) += A(i,j) * X (j)
                MULTADD ;
                // move to the next entry in A(i,:)
                info = GxB_rowIterator_nextCol (iterator) ;
                OK (info) ;
            }
            // move to the next row, A(i+1,:)
            info = GxB_rowIterator_nextRow (iterator) ;
            OK (info) ;
        }

        Assert (kount1 == kount2) ;

        // check the return value when the iterator is exhausted
        GrB_Index i = GxB_rowIterator_getRowIndex (iterator) ;
        Assert (i == nrows) ;

    }
    else if (kind == 1)
    {

        //----------------------------------------------------------------------
        // Y = A*X using a col iterator
        //----------------------------------------------------------------------

        // attach it to the matrix A
        OK (GxB_colIterator_attach (iterator, A, NULL)) ;
        // get the kount
        GrB_Index kount2 = 0 ;
        GrB_Index kount1 = GxB_colIterator_kount (iterator) ;
        // seek to A(:,0)
        info = GxB_colIterator_seekCol (iterator, 0) ;
        OK (info) ;
        while (info != GxB_EXHAUSTED)
        {
            // iterate over entries in A(:,j)
            kount2++ ;
            GrB_Index j = GxB_colIterator_getColIndex (iterator) ;
            Assert (j >= 0 && j < ncols) ;
            while (info == GrB_SUCCESS)
            {
                // get the entry A(i,j)
                GrB_Index i = GxB_colIterator_getRowIndex (iterator) ;
                Assert (i >= 0 && i < nrows) ;
                // Y (i) += A(i,j) * X (j)
                MULTADD ;
                // move to the next entry in A(:,j)
                info = GxB_colIterator_nextRow (iterator) ;
                OK (info) ;
            }
            // move to the next column, A(:,j+1)
            info = GxB_colIterator_nextCol (iterator) ;
            OK (info) ;
        }

        Assert (kount1 == kount2) ;

        // check the return value when the iterator is exhausted
        GrB_Index j = GxB_rowIterator_getRowIndex (iterator) ;
        Assert (j == ncols) ;

    }
    else if (kind == 2)
    {

        //----------------------------------------------------------------------
        // Y = A*X using a matrix iterator
        //----------------------------------------------------------------------

        // useless descriptor, just to test nthreads extraction for GB_wait
        A->jumbled = true ;

        // attach it to the matrix A
        OK (GxB_Matrix_Iterator_attach (iterator, A, GrB_DESC_S)) ;
        // seek to the first entry
        OK (GxB_Matrix_Iterator_seek (iterator, 0)) ;
        while (info != GxB_EXHAUSTED)
        {
            // get the entry A(i,j)
            GrB_Index i, j ;
            GxB_Matrix_Iterator_getIndex (iterator, &i, &j) ;
            Assert (i >= 0 && i < nrows) ;
            Assert (j >= 0 && j < ncols) ;
            // Y (i) += A(i,j) * X (j)
            MULTADD ;
            // move to the next entry in A
            info = GxB_Matrix_Iterator_next (iterator) ;
            OK (info);
        }

        GrB_Index p = GxB_Matrix_Iterator_getp (iterator) ;
        GrB_Index pmax = GxB_Matrix_Iterator_getpmax (iterator) ;
        Assert (p == pmax) ;

    }
    else if (kind == 3)
    {

        //----------------------------------------------------------------------
        // Y = A*X using a row iterator, but backwards with kseek
        //----------------------------------------------------------------------

        // attach it to the matrix A
        OK (GxB_rowIterator_attach (iterator, A, NULL)) ;
        // get the kount
        GrB_Index kount = GxB_rowIterator_kount (iterator) ;
        for (int k = kount-1 ; k >= 0 ; k--)
        {
            // seek to A(k,:)
            info = GxB_rowIterator_kseek (iterator, (GrB_Index) k) ;
            Assert (info == GrB_SUCCESS || info == GrB_NO_VALUE) ;

            // get the row index
            GrB_Index i = GxB_rowIterator_getRowIndex (iterator) ;
            Assert (i >= 0 && i <= nrows) ;

            // iterate over entries in A(i,:)
            while (info == GrB_SUCCESS)
            {
                // get the entry A(i,j)
                GrB_Index j = GxB_rowIterator_getColIndex (iterator) ;
                Assert (j >= 0 && j < ncols) ;
                // Y (i) += A(i,j) * X (j)
                MULTADD ;
                // move to the next entry in A(i,:)
                info = GxB_rowIterator_nextCol (iterator) ;
                OK (info) ;
            }
        }

    }
    else if (kind == 4)
    {

        //----------------------------------------------------------------------
        // Y = A*X using a col iterator, but backwards with kseek
        //----------------------------------------------------------------------

        // attach it to the matrix A
        OK (GxB_colIterator_attach (iterator, A, NULL)) ;
        // get the kount
        GrB_Index kount = GxB_colIterator_kount (iterator) ;
        for (int k = kount-1 ; k >= 0 ; k--)
        {
            // seek to A(:,k)
            info = GxB_colIterator_kseek (iterator, (GrB_Index) k) ;
            Assert (info == GrB_SUCCESS || info == GrB_NO_VALUE) ;

            // get the col index
            GrB_Index j = GxB_colIterator_getColIndex (iterator) ;
            Assert (j >= 0 && j <= ncols) ;

            // iterate over entries in A(:,j)
            while (info == GrB_SUCCESS)
            {
                // get the entry A(i,j)
                GrB_Index i = GxB_colIterator_getRowIndex (iterator) ;
                Assert (i >= 0 && i < nrows) ;
                // Y (i) += A(i,j) * X (j)
                MULTADD ;
                // move to the next entry in A(:,j)
                info = GxB_colIterator_nextRow (iterator) ;
                OK (info) ;
            }
        }

    }
    else if (kind == 5)
    {

        //----------------------------------------------------------------------
        // Y = A*X using a row iterator, but with seekRow
        //----------------------------------------------------------------------

        // attach it to the matrix A
        OK (GxB_rowIterator_attach (iterator, A, NULL)) ;

        for (int k = 0 ; k < nrows ; k++)
        {
            // seek to A(k,:)
            info = GxB_rowIterator_seekRow (iterator, (GrB_Index) k) ;
            Assert (info >= GrB_SUCCESS) ;

            // get the row index
            GrB_Index i = GxB_rowIterator_getRowIndex (iterator) ;
            Assert (i >= 0 && i <= nrows) ;

            // if the matrix is hypersparse, seekRow can skip
            if (i != k) continue ;

            // iterate over entries in A(i,:)
            while (info == GrB_SUCCESS)
            {
                // get the entry A(i,j)
                GrB_Index j = GxB_rowIterator_getColIndex (iterator) ;
                Assert (j >= 0 && j < ncols) ;
                // Y (i) += A(i,j) * X (j)
                MULTADD ;
                // move to the next entry in A(i,:)
                info = GxB_rowIterator_nextCol (iterator) ;
                OK (info) ;
            }
        }

        // try exhaustion
        info = GxB_rowIterator_seekRow (iterator, nrows) ;
        Assert (info == GxB_EXHAUSTED) ;

    }
    else if (kind == 6)
    {

        //----------------------------------------------------------------------
        // Y = A*X using a col iterator, but with seekCol
        //----------------------------------------------------------------------

        // attach it to the matrix A
        OK (GxB_colIterator_attach (iterator, A, NULL)) ;
        for (int k = 0 ; k < ncols ; k++)
        {
            // seek to A(:,k)
            info = GxB_colIterator_seekCol (iterator, (GrB_Index) k) ;
            Assert (info >= GrB_SUCCESS) ;

            // get the col index
            GrB_Index j = GxB_colIterator_getColIndex (iterator) ;
            Assert (j >= 0 && j <= ncols) ;

            // if the matrix is hypersparse, seekCol can skip
            if (j != k) continue ;

            // iterate over entries in A(:,j)
            while (info == GrB_SUCCESS)
            {
                // get the entry A(i,j)
                GrB_Index i = GxB_colIterator_getRowIndex (iterator) ;
                Assert (i >= 0 && i < nrows) ;
                // Y (i) += A(i,j) * X (j)
                MULTADD ;
                // move to the next entry in A(:,j)
                info = GxB_colIterator_nextRow (iterator) ;
                OK (info) ;
            }
        }

        // try exhaustion
        info = GxB_colIterator_seekCol (iterator, ncols) ;
        Assert (info == GxB_EXHAUSTED) ;

    }
    else if (kind == 7)
    {

        //----------------------------------------------------------------------
        // Y = A*X using a matrix iterator but with seek
        //----------------------------------------------------------------------

        // attach it to the matrix A
        OK (GxB_Matrix_Iterator_attach (iterator, A, NULL)) ;
        GrB_Index pmax = GxB_Matrix_Iterator_getpmax (iterator) ;

        for (GrB_Index p = 0 ; p < pmax ; p++)
        {
            // seek to the pth entry
            OK (GxB_Matrix_Iterator_seek (iterator, p)) ;
            // check if the pth entry exists (for bitmap case)
            if (p == GxB_Matrix_Iterator_getp (iterator))
            {
                // get the entry A(i,j)
                GrB_Index i, j ;
                GxB_Matrix_Iterator_getIndex (iterator, &i, &j) ;
                Assert (i >= 0 && i < nrows) ;
                Assert (j >= 0 && j < ncols) ;
                // Y (i) += A(i,j) * X (j)
                MULTADD ;
            }
        }
        // try exhaustion
        info = GxB_Matrix_Iterator_seek (iterator, pmax) ;
        Assert (info == GxB_EXHAUSTED) ;

    }
    //--------------------------------------------------------------------------
    // error handling
    //--------------------------------------------------------------------------

    if (A->is_csc)
    {
        info = GxB_rowIterator_attach (iterator, A, NULL) ;
        Assert (info == GrB_NOT_IMPLEMENTED) ;
    }
    else
    {
        info = GxB_colIterator_attach (iterator, A, NULL) ;
        Assert (info == GrB_NOT_IMPLEMENTED) ;
    }

    //--------------------------------------------------------------------------
    // return Y as a struct and free the GraphBLAS Y
    //--------------------------------------------------------------------------

    // GxB_print (Y, 3) ;
    GB_Global_print_one_based_set (1) ;
    pargout [0] = GB_mx_Vector_to_mxArray (&Y, "Y output", true) ;
    FREE_ALL ;
}

