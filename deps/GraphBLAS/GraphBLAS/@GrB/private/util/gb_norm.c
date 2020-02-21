//------------------------------------------------------------------------------
// gb_norm: compute the norm of a GraphBLAS matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "gb_matlab.h"

double gb_norm              // compute norm (A,kind)
(
    GrB_Matrix A,
    int64_t norm_kind       // 0, 1, 2, INT64_MAX, or INT64_MIN
)
{

    //--------------------------------------------------------------------------
    // get input matrix, select types and operators, and allocate X
    //--------------------------------------------------------------------------

    GrB_Index nrows, ncols, nvals ;
    OK (GrB_Matrix_nvals (&nvals, A)) ;
    if (nvals == 0) return ((double) 0) ;

    GrB_Type atype, xtype ;
    OK (GrB_Matrix_nrows (&nrows, A)) ;
    OK (GrB_Matrix_ncols (&ncols, A)) ;
    OK (GxB_Matrix_type (&atype, A)) ;

    GrB_UnaryOp absop ;
    GrB_BinaryOp timesop ;
    GrB_Monoid sumop, maxop, minop ;
    GrB_Vector t = NULL ;
    GrB_Matrix X = NULL ;

    if (atype == GrB_FP32)
    {
        // if A is FP32, use the FP32 type and operators
        xtype = GrB_FP32 ;
        absop = GxB_ABS_FP32 ;
        sumop = GxB_PLUS_FP32_MONOID ;
        timesop = GrB_TIMES_FP32 ;
        maxop = GxB_MAX_FP32_MONOID ;
        minop = GxB_MIN_FP32_MONOID ;
    }
    else
    {
        // otherwise, use FP64 type and operators; this will typecast the 
        // input matrix to FP64 if A is not of that type.
        xtype = GrB_FP64 ;
        absop = GxB_ABS_FP64 ;
        sumop = GxB_PLUS_FP64_MONOID ;
        timesop = GrB_TIMES_FP64 ;
        maxop = GxB_MAX_FP64_MONOID ;
        minop = GxB_MIN_FP64_MONOID ;
    }

    OK (GrB_Matrix_new (&X, xtype, nrows, ncols)) ;

    //--------------------------------------------------------------------------
    // compute the norm
    //--------------------------------------------------------------------------

    double s = 0 ;

    if (nrows == 1 || ncols == 1 || norm_kind == 0)
    {

        //----------------------------------------------------------------------
        // vector or Frobenius norm
        //----------------------------------------------------------------------

        switch (norm_kind)
        {

            case 0:     // Frobenius norm
            case 2:     // 2-norm

                // X = A .* A
                OK (GrB_eWiseMult_Matrix_BinaryOp (X, NULL, NULL, timesop,
                    A, A, NULL)) ;
                // s = sum (X)
                OK (GrB_Matrix_reduce_FP64 (&s, NULL, sumop, X, NULL)) ;
                s = sqrt (s) ;
                break ;

            case 1:     // 1-norm

                // X = abs (A)
                OK (GrB_Matrix_apply (X, NULL, NULL, absop, A, NULL)) ;
                // s = sum (X)
                OK (GrB_Matrix_reduce_FP64 (&s, NULL, sumop, X, NULL)) ;
                break ;

            case INT64_MAX:     // inf-norm

                // X = abs (A)
                OK (GrB_Matrix_apply (X, NULL, NULL, absop, A, NULL)) ;
                // s = max (X)
                OK (GrB_Matrix_reduce_FP64 (&s, NULL, maxop, X, NULL)) ;
                break ;

            case INT64_MIN:     // (-inf)-norm

                if (!GB_is_dense (A))
                {
                    // X = abs (A)
                    OK (GrB_Matrix_apply (X, NULL, NULL, absop, A, NULL)) ;
                    // s = min (X)
                    OK (GrB_Matrix_reduce_FP64 (&s, NULL, minop, X, NULL)) ;
                }
                break ;

            default:

                ERROR ("unknown norm") ;
                break ;
        }

    }
    else
    {

        //----------------------------------------------------------------------
        // matrix norm
        //----------------------------------------------------------------------

        switch (norm_kind)
        {

            case 2:     // 2-norm

                ERROR ("2-norm not available for GrB matrices") ;
                break ;

            case 1:     // 1-norm:  max sum of columns of abs (A)

                // X = abs (A)
                OK (GrB_Matrix_apply (X, NULL, NULL, absop, A, NULL)) ;
                // t = zeros (ncols,1)
                OK (GrB_Vector_new (&t, xtype, ncols)) ;
                // t(j) = sum of the ith column, X(:,j)
                OK (GrB_Matrix_reduce_Monoid (t, NULL, NULL, sumop, X,
                    GrB_DESC_T0)) ;
                // s = max (t)
                OK (GrB_Vector_reduce_FP64 (&s, NULL, maxop, t, NULL)) ;
                break ;

            case INT64_MAX:     // inf-norm:  max sum of rows of abs (A)

                // X = abs (A)
                OK (GrB_Matrix_apply (X, NULL, NULL, absop, A, NULL)) ;
                // t = zeros (nrows,1)
                OK (GrB_Vector_new (&t, xtype, nrows)) ;
                // t(i) = sum of the ith row, X(i,:)
                OK (GrB_Matrix_reduce_Monoid (t, NULL, NULL, sumop, X, NULL)) ;
                // s = max (t)
                OK (GrB_Vector_reduce_FP64 (&s, NULL, maxop, t, NULL)) ;
                break ;

            case INT64_MIN:

                ERROR ("(-inf)-norm not available for GrB matrices") ;
                break ;

            default:

                ERROR ("unknown norm") ;
                break ;
        }
    }

    //--------------------------------------------------------------------------
    // free workspace and return result
    //--------------------------------------------------------------------------

    OK (GrB_Matrix_free (&X)) ;
    OK (GrB_Vector_free (&t)) ;
    return (s) ;
}

