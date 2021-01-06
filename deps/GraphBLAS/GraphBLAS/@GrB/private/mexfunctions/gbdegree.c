//------------------------------------------------------------------------------
// gbdegree: number of entries in each vector of a GraphBLAS matrix struct
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// The input may be either a GraphBLAS matrix struct or a standard MATLAB
// sparse matrix.

//  gbdegree (X, 'row')     row degree
//  gbdegree (X, 'col')     column degree
//  gbdegree (X, true)      native (get degree of each vector):
//                          row degree if X is held by row,
//                          col degree if X is held by col.
//  gbdegree (X, false)     non-native (sum across vectors):
//                          col degree if X is held by row,
//                          row degree if X is held by col.

#include "gb_matlab.h"

#define USAGE "usage: degree = gbdegree (X, dim)"

void mexFunction
(
    int nargout,
    mxArray *pargout [ ],
    int nargin,
    const mxArray *pargin [ ]
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    gb_usage (nargin == 2 && nargout <= 1, USAGE) ;

    //--------------------------------------------------------------------------
    // get the inputs 
    //--------------------------------------------------------------------------

    int64_t *degree = NULL ;
    GrB_Index *list = NULL, nvec = 0 ;
    GrB_Vector d = NULL ;
    GrB_Vector y = NULL ;
    GrB_Matrix T = NULL ;
    GrB_Matrix Z = NULL ;
    GrB_Descriptor desc = NULL ;

    GrB_Matrix X = gb_get_shallow (pargin [0]) ;
    GxB_Format_Value fmt ;
    OK (GxB_Matrix_Option_get (X, GxB_FORMAT, &fmt)) ;

    bool native ;
    if (mxIsChar (pargin [1]))
    {
        #define LEN 256
        char dim_string [LEN+2] ;
        gb_mxstring_to_string (dim_string, LEN, pargin [1], "dim") ;
        if (MATCH (dim_string, "row"))
        { 
            native = (fmt == GxB_BY_ROW) ;
        }
        else // if (MATCH (dim_string, "col"))
        { 
            native = (fmt == GxB_BY_COL) ;
        }
    }
    else
    { 
        native = (mxGetScalar (pargin [1]) != 0) ;
    }

    //--------------------------------------------------------------------------
    // if X is bitmap: create a copy of X and convert it to sparse
    //--------------------------------------------------------------------------

    int sparsity ;
    OK (GxB_Matrix_Option_get (X, GxB_SPARSITY_STATUS, &sparsity)) ;
    if (sparsity == GxB_BITMAP)
    { 
        // Z = deep copy of the shallow matrix X
        OK (GrB_Matrix_dup (&Z, X)) ;
        // convert Z to sparse
        OK (GxB_Matrix_Option_set (Z, GxB_SPARSITY_CONTROL, GxB_SPARSE)) ;
        // free the shallow X and replace it with Z
        OK (GrB_Matrix_free (&X)) ;
        X = Z ;
    }

    //--------------------------------------------------------------------------
    // get the degree of each row or column of X
    //--------------------------------------------------------------------------

    GrB_Index nvals, nrows, ncols ;
    OK (GrB_Matrix_nvals (&nvals, X)) ;
    OK (GrB_Matrix_nrows (&nrows, X)) ;
    OK (GrB_Matrix_ncols (&ncols, X)) ;

    if (native)
    { 

        //----------------------------------------------------------------------
        // get the degree of each vector of X, where X is sparse or hypersparse
        //----------------------------------------------------------------------

        if (!GB_matlab_helper9 (X, &degree, &list, &nvec))
        {
            ERROR ("out of memory") ;
        }
        OK (GxB_Vector_import_CSC (&d, GrB_INT64, X->vdim,
            &list, &degree, nvec, nvec, nvec, false, NULL)) ;

    }
    else
    {

        //----------------------------------------------------------------------
        // get the degree of each index of X, where X is sparse or hypersparse
        //----------------------------------------------------------------------

        // ensure the descriptor is present, and set GxB_SORT to true
        OK (GrB_Descriptor_new (&desc)) ;
        OK (GxB_Desc_set (desc, GxB_SORT, true)) ;

        if (fmt == GxB_BY_COL)
        {

            //------------------------------------------------------------------
            // compute the degree of each row of X, where X is held by column
            //------------------------------------------------------------------

            if (nvals < ncols / 16 && ncols > 256)
            { 

                // X is hypersparse, or might as well be, and held by column,
                // so compute the degree of each vector of T = GrB(X,'by row')
                // instead.

                OK (GrB_Matrix_new (&T, GrB_BOOL, nrows, ncols)) ;
                OK (GxB_Matrix_Option_set (T, GxB_FORMAT, GxB_BY_ROW)) ;
                OK1 (T, GrB_Matrix_apply (T, NULL, NULL, GxB_ONE_BOOL, X,
                    NULL)) ;

                // get the degree of nonempty rows of T
                if (!GB_matlab_helper9 (T, &degree, &list, &nvec))
                {
                    ERROR ("out of memory") ;
                }
                OK (GxB_Vector_import_CSC (&d, GrB_INT64, nrows,
                    &list, &degree, nvec, nvec, nvec, false, NULL)) ;

            }
            else
            { 

                // y = full vector of size ncols-by-1; value is not relevant
                OK (GrB_Vector_new (&y, GrB_BOOL, ncols)) ;
                OK (GrB_Vector_assign_BOOL (y, NULL, NULL, false, GrB_ALL,
                    ncols, NULL)) ;

                // d = X*y using the PLUS_PAIR semiring
                OK (GrB_Vector_new (&d, GrB_INT64, nrows)) ;
                OK (GrB_mxv (d, NULL, NULL, GxB_PLUS_PAIR_INT64, X, y, desc)) ;
            }

        }
        else
        {

            //------------------------------------------------------------------
            // compute the degree of each column of X, where X is held by row
            //------------------------------------------------------------------

            if (nvals < nrows / 16 && nrows > 256)
            { 

                // X is hypersparse, or might as well be, and held by row,
                // so compute the degree of each vector of T = GrB(X,'by col')
                // instead.

                OK (GrB_Matrix_new (&T, GrB_BOOL, nrows, ncols)) ;
                OK (GxB_Matrix_Option_set (T, GxB_FORMAT, GxB_BY_COL)) ;
                OK1 (T, GrB_Matrix_apply (T, NULL, NULL, GxB_ONE_BOOL, X,
                    NULL)) ;

                // get the degree of nonempty columns of T
                if (!GB_matlab_helper9 (T, &degree, &list, &nvec))
                {
                    ERROR ("out of memory") ;
                }
                OK (GxB_Vector_import_CSC (&d, GrB_INT64, ncols,
                    &list, &degree, nvec, nvec, nvec, false, NULL)) ;

            }
            else
            { 

                // y = full vector of size nrows-by-1; value is not relevant
                OK (GrB_Vector_new (&y, GrB_BOOL, nrows)) ;
                OK (GrB_Vector_assign_BOOL (y, NULL, NULL, false, GrB_ALL,
                    nrows, NULL)) ;

                // d = y*X using the PLUS_PAIR semiring
                OK (GrB_Vector_new (&d, GrB_INT64, ncols)) ;
                OK (GrB_vxm (d, NULL, NULL, GxB_PLUS_PAIR_INT64, y, X, desc)) ;
            }
        }
    }

    //--------------------------------------------------------------------------
    // free workspace and export d to MATLAB as a GraphBLAS matrix
    //--------------------------------------------------------------------------

    OK (GrB_Vector_free (&y)) ;
    OK (GrB_Matrix_free (&T)) ;
    OK (GrB_Matrix_free (&X)) ;
    OK (GrB_Descriptor_free (&desc)) ;
    pargout [0] = gb_export (&d, KIND_GRB) ;
    GB_WRAPUP ;
}

