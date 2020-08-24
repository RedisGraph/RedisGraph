//------------------------------------------------------------------------------
// LAGraph_isall: check two matrices
//------------------------------------------------------------------------------

/*
    LAGraph:  graph algorithms based on GraphBLAS

    Copyright 2019 LAGraph Contributors.

    (see Contributors.txt for a full list of Contributors; see
    ContributionInstructions.txt for information on how you can Contribute to
    this project).

    All Rights Reserved.

    NO WARRANTY. THIS MATERIAL IS FURNISHED ON AN "AS-IS" BASIS. THE LAGRAPH
    CONTRIBUTORS MAKE NO WARRANTIES OF ANY KIND, EITHER EXPRESSED OR IMPLIED,
    AS TO ANY MATTER INCLUDING, BUT NOT LIMITED TO, WARRANTY OF FITNESS FOR
    PURPOSE OR MERCHANTABILITY, EXCLUSIVITY, OR RESULTS OBTAINED FROM USE OF
    THE MATERIAL. THE CONTRIBUTORS DO NOT MAKE ANY WARRANTY OF ANY KIND WITH
    RESPECT TO FREEDOM FROM PATENT, TRADEMARK, OR COPYRIGHT INFRINGEMENT.

    Released under a BSD license, please see the LICENSE file distributed with
    this Software or contact permission@sei.cmu.edu for full terms.

    Created, in part, with funding and support from the United States
    Government.  (see Acknowledgments.txt file).

    This program includes and/or can make use of certain third party source
    code, object code, documentation and other files ("Third Party Software").
    See LICENSE file for more details.

*/

//------------------------------------------------------------------------------

// LAGraph_isall: check two matrices.  Contributed by Tim Davis, Texas A&M

// Applies a binary operator to two matrices A and B, and returns result = true
// if the pattern of A and B are identical, and if the result of C = A op B is
// true for all entries in C.

#include "LAGraph_internal.h"

#define LAGRAPH_FREE_ALL    \
    GrB_free (&C) ;         \

GrB_Info LAGraph_isall      // return GrB_SUCCESS if successful
(
    bool *result,           // true if A == B, false if A != B or error
    GrB_Matrix A,
    GrB_Matrix B,
    GrB_BinaryOp op         // GrB_EQ_<type>, for the type of A and B,
                            // to check for equality.  Or use any desired
                            // operator.  The operator should return GrB_BOOL.
)
{

    GrB_Info info ;
    GrB_Matrix C = NULL ;
    GrB_Index nrows1, ncols1, nrows2, ncols2, nvals, nvals1, nvals2 ;

    // check inputs
    if (result == NULL)
    {
        // error: required parameter, result, is NULL
        // printf ("LAGraph_isall: bad args \n") ;
        return (GrB_NULL_POINTER) ;
    }
    (*result) = false ;

    // check the size of A and B
    LAGRAPH_OK (GrB_Matrix_nrows (&nrows1, A)) ;
    LAGRAPH_OK (GrB_Matrix_nrows (&nrows2, B)) ;
    if (nrows1 != nrows2)
    {
        // # of rows differ
        return (GrB_SUCCESS) ;
    }

    LAGRAPH_OK (GrB_Matrix_ncols (&ncols1, A)) ;
    LAGRAPH_OK (GrB_Matrix_ncols (&ncols2, B)) ;
    if (ncols1 != ncols2)
    {
        // # of cols differ
        return (GrB_SUCCESS) ;
    }

    // check the # entries in A and B
    LAGRAPH_OK (GrB_Matrix_nvals (&nvals1, A)) ;
    LAGRAPH_OK (GrB_Matrix_nvals (&nvals2, B)) ;
    if (nvals1 != nvals2)
    {
        // # of entries differ
        return (GrB_SUCCESS) ;
    }

    // C = A .* B, where the pattern of C is the intersection of A and B
    LAGRAPH_OK (GrB_Matrix_new (&C, GrB_BOOL, nrows1, ncols1)) ;
    LAGRAPH_OK (GrB_eWiseMult (C, NULL, NULL, op, A, B, NULL)) ;

    // ensure C has the same number of entries as A and B
    LAGRAPH_OK (GrB_Matrix_nvals (&nvals, C)) ;
    if (nvals != nvals1)
    {
        // pattern of A and B are different
        GrB_free (&C) ;
        return (GrB_SUCCESS) ;
    }

    // result = and (C)
    LAGRAPH_OK (GrB_reduce (result, NULL, LAGraph_LAND_MONOID, C, NULL)) ;

    // free workspace and return result
    GrB_free (&C) ;
    return (GrB_SUCCESS) ;
}

