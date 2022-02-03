//------------------------------------------------------------------------------
// GB_mex_dot_iterator: s = X'*Y, dot product of 2 vectors using iterators
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// X and Y must have the same type.  If boolean, the lor_land semiring is used.
// Otherwise, the plus_times semiring is used for the given type.

#include "GB_mex.h"
#include "GB_mex_errors.h"

#define USAGE "s = GB_mex_dot_iterator (X, Y, kind)"

#define FREE_ALL                                    \
{                                                   \
    GrB_Vector_free_(&X) ;                          \
    GrB_Vector_free_(&Y) ;                          \
    GxB_Iterator_free (&X_iterator) ;               \
    GxB_Iterator_free (&Y_iterator) ;               \
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
    GxB_Iterator X_iterator = NULL, Y_iterator ;

    // check inputs
    if (nargout > 1 || nargin < 2 || nargin > 3)
    {
        mexErrMsgTxt ("Usage: " USAGE) ;
    }

    // get X (shallow copy)
    X = GB_mx_mxArray_to_Vector (pargin [0], "X input", false, true) ;
    if (X == NULL)
    {
        FREE_ALL ;
        mexErrMsgTxt ("X failed") ;
    }

    // get Y (shallow copy)
    Y = GB_mx_mxArray_to_Vector (pargin [1], "Y input", false, true) ;
    if (Y == NULL)
    {
        FREE_ALL ;
        mexErrMsgTxt ("Y failed") ;
    }

    // get kind: 0: merge, 1: iterate through x and lookup y (requires y full)
    int GET_SCALAR (2, int, kind, 0) ;

    GrB_Index n, ny ;
    OK (GrB_Vector_size (&n, X)) ;
    OK (GrB_Vector_size (&ny, Y)) ;

    GB_Global_print_one_based_set (0) ;
    // GxB_print (X, 3) ;
    // GxB_print (Y, 3) ;

    if (n != ny)
    {
        FREE_ALL ;
        mexErrMsgTxt ("X and Y must have the same size") ;
    }

    GrB_Type type = X->type ;
    if (type != Y->type)
    {
        FREE_ALL ;
        mexErrMsgTxt ("X and Y must have the same type") ;
    }

//  bool user_complex = (Complex != GxB_FC64) && (type == Complex) ;

    // create the output scalar and set it to zero
    pargout [0] = GB_mx_create_full (1, 1, type) ;
    GB_void *s = mxGetData (pargout [0]) ;
    memset (s, 0, type->size) ;

    //--------------------------------------------------------------------------
    // MULTADD: s += X(i) * Y(i)
    //--------------------------------------------------------------------------

    #define MULTADD                                                         \
    {                                                                       \
        if (type == GrB_BOOL)                                               \
        {                                                                   \
            bool xi = GxB_Iterator_get_BOOL (X_iterator) ;                  \
            bool yi = GxB_Iterator_get_BOOL (Y_iterator) ;                  \
            (*(bool *) s) |= xi && yi ;                                     \
        }                                                                   \
        else if (type == GrB_INT8)                                          \
        {                                                                   \
            int8_t xi = GxB_Iterator_get_INT8 (X_iterator) ;                \
            int8_t yi = GxB_Iterator_get_INT8 (Y_iterator) ;                \
            (*(int8_t *) s) += xi * yi ;                                    \
        }                                                                   \
        else if (type == GrB_INT16)                                         \
        {                                                                   \
            int16_t xi = GxB_Iterator_get_INT16 (X_iterator) ;              \
            int16_t yi = GxB_Iterator_get_INT16 (Y_iterator) ;              \
            (*(int16_t *) s) += xi * yi ;                                   \
        }                                                                   \
        else if (type == GrB_INT32)                                         \
        {                                                                   \
            int32_t xi = GxB_Iterator_get_INT32 (X_iterator) ;              \
            int32_t yi = GxB_Iterator_get_INT32 (Y_iterator) ;              \
            (*(int32_t *) s) += xi * yi ;                                   \
        }                                                                   \
        else if (type == GrB_INT64)                                         \
        {                                                                   \
            int64_t xi = GxB_Iterator_get_INT64 (X_iterator) ;              \
            int64_t yi = GxB_Iterator_get_INT64 (Y_iterator) ;              \
            (*(int64_t *) s) += xi * yi ;                                   \
        }                                                                   \
        else if (type == GrB_UINT8)                                         \
        {                                                                   \
            uint8_t xi = GxB_Iterator_get_UINT8 (X_iterator) ;              \
            uint8_t yi = GxB_Iterator_get_UINT8 (Y_iterator) ;              \
            (*(uint8_t *) s) += xi * yi ;                                   \
        }                                                                   \
        else if (type == GrB_UINT16)                                        \
        {                                                                   \
            uint16_t xi = GxB_Iterator_get_UINT16 (X_iterator) ;            \
            uint16_t yi = GxB_Iterator_get_UINT16 (Y_iterator) ;            \
            (*(uint16_t *) s) += xi * yi ;                                  \
        }                                                                   \
        else if (type == GrB_UINT32)                                        \
        {                                                                   \
            uint32_t xi = GxB_Iterator_get_UINT32 (X_iterator) ;            \
            uint32_t yi = GxB_Iterator_get_UINT32 (Y_iterator) ;            \
            (*(uint32_t *) s) += xi * yi ;                                  \
        }                                                                   \
        else if (type == GrB_UINT64)                                        \
        {                                                                   \
            uint64_t xi = GxB_Iterator_get_UINT64 (X_iterator) ;            \
            uint64_t yi = GxB_Iterator_get_UINT64 (Y_iterator) ;            \
            (*(uint64_t *) s) += xi * yi ;                                  \
        }                                                                   \
        else if (type == GrB_FP32)                                          \
        {                                                                   \
            float xi = GxB_Iterator_get_FP32 (X_iterator) ;                 \
            float yi = GxB_Iterator_get_FP32 (Y_iterator) ;                 \
            (*(float *) s) += xi * yi ;                                     \
        }                                                                   \
        else if (type == GrB_FP64)                                          \
        {                                                                   \
            double xi = GxB_Iterator_get_FP64 (X_iterator) ;                \
            double yi = GxB_Iterator_get_FP64 (Y_iterator) ;                \
            (*(double *) s) += xi * yi ;                                    \
        }                                                                   \
        else if (type == GxB_FC32)                                          \
        {                                                                   \
            GxB_FC32_t xi = GxB_Iterator_get_FC32 (X_iterator) ;            \
            GxB_FC32_t yi = GxB_Iterator_get_FC32 (Y_iterator) ;            \
            (*(GxB_FC32_t *) s) += xi * yi ;                                \
        }                                                                   \
        else if (type == GxB_FC64)                                          \
        {                                                                   \
            GxB_FC64_t xi = GxB_Iterator_get_FC64 (X_iterator) ;            \
            GxB_FC64_t yi = GxB_Iterator_get_FC64 (Y_iterator) ;            \
            (*(GxB_FC64_t *) s) += xi * yi ;                                \
        }                                                                   \
        else if (type == Complex)                                           \
        {                                                                   \
            GxB_FC64_t xi, yi ;                                             \
            GxB_Iterator_get_UDT (X_iterator, (void *) &xi) ;               \
            GxB_Iterator_get_UDT (Y_iterator, (void *) &yi) ;               \
            (*(GxB_FC64_t *) s) += xi * yi ;                                \
        }                                                                   \
        else                                                                \
        {                                                                   \
            mexErrMsgTxt ("type unknown") ;                                 \
        }                                                                   \
    }

    //--------------------------------------------------------------------------
    // s += X'*Y using vector iterators
    //--------------------------------------------------------------------------

    // create the X and Y iterators
    OK (GxB_Iterator_new (&X_iterator)) ;
    OK (GxB_Iterator_new (&Y_iterator)) ;

    GrB_Info X_info = GxB_Vector_Iterator_attach (X_iterator, X, NULL) ;
    GrB_Info Y_info = GxB_Vector_Iterator_attach (Y_iterator, Y, NULL) ;
    OK (X_info) ;
    OK (Y_info) ;

    int x_sparsity, y_sparsity ;
    OK (GxB_Vector_Option_get (X, GxB_SPARSITY_STATUS, &x_sparsity)) ;
    OK (GxB_Vector_Option_get (Y, GxB_SPARSITY_STATUS, &y_sparsity)) ;
    GrB_Index xnvals, ynvals ;
    OK (GrB_Vector_nvals (&xnvals, X)) ;
    OK (GrB_Vector_nvals (&ynvals, Y)) ;

    GrB_Index x_pmax = GxB_Vector_Iterator_getpmax (X_iterator) ;
    Assert (x_pmax == ((x_sparsity == GxB_BITMAP && xnvals > 0) ? n : xnvals)) ;

    GrB_Index y_pmax = GxB_Vector_Iterator_getpmax (Y_iterator) ;
    Assert (y_pmax == ((y_sparsity == GxB_BITMAP && ynvals > 0) ? n : ynvals)) ;

    if (kind == 0)
    {

        // seek to the first entry of X and Y
        X_info = GxB_Vector_Iterator_seek (X_iterator, 0) ;
        Y_info = GxB_Vector_Iterator_seek (Y_iterator, 0) ;

        while (X_info != GxB_EXHAUSTED && Y_info != GxB_EXHAUSTED)
        {
            // get the index of entries x(i) and y(j)
            GrB_Index i = GxB_Vector_Iterator_getIndex (X_iterator) ;
            GrB_Index j = GxB_Vector_Iterator_getIndex (Y_iterator) ;
            if (i < j)
            {
                // consume x(i)
                X_info = GxB_Vector_Iterator_next (X_iterator) ;
            }
            else if (i > j)
            {
                // consume y(j)
                Y_info = GxB_Vector_Iterator_next (Y_iterator) ;
            }
            else // i == j
            {
                // s += x(i) * y(i)
                MULTADD ;
                // consume both x(i) and y(i)
                X_info = GxB_Vector_Iterator_next (X_iterator) ;
                Y_info = GxB_Vector_Iterator_next (Y_iterator) ;
            }
        }
    }
    else
    {

        // This is an absurd algorithm, just to exercise the code.
        // It's not recommended.

        // seek all entries in x, backwards
        for (int64_t p = ((int64_t) x_pmax) - 1 ; p >= 0 ; p--)
        {
            X_info = GxB_Vector_Iterator_seek (X_iterator, (GrB_Index) p) ;
            if (X_info == GrB_SUCCESS)
            {
                GrB_Index p2 = GxB_Vector_Iterator_getp (X_iterator) ;
                if (p != p2) continue ;

                // get x(i)
                GrB_Index i = GxB_Vector_Iterator_getIndex (X_iterator) ;

                // find y(i) via brute force
                Y_info = GxB_Vector_Iterator_seek (Y_iterator, 0) ;
                while (Y_info != GxB_EXHAUSTED)
                {
                    GrB_Index i2 = GxB_Vector_Iterator_getIndex (Y_iterator) ;
                    if (i2 == i)
                    {
                        // s += x(i) * y(i)
                        MULTADD ;
                        break ;
                    }
                    Y_info = GxB_Vector_Iterator_next (Y_iterator) ;
                }
            }
        }
    }

    //--------------------------------------------------------------------------
    // free workspace
    //--------------------------------------------------------------------------

    GB_Global_print_one_based_set (1) ;
    FREE_ALL ;
}

