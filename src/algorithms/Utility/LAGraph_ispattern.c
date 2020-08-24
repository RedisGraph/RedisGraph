//------------------------------------------------------------------------------
// LAGraph_ispattern: check if a matrix is all 1
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

// LAGraph_ispattern: check if a matrix values are all equal to 1.
// Contributed by Tim Davis, Texas A&M.

#include "LAGraph_internal.h"

#define LAGRAPH_FREE_ALL    \
    GrB_free (&C) ;

GrB_Info LAGraph_ispattern  // return GrB_SUCCESS if successful
(
    bool *result,           // true if A is all one, false otherwise
    GrB_Matrix A,
    GrB_UnaryOp userop      // for A with arbitrary user-defined type.
                            // Ignored if A and B are of built-in types or
                            // LAGraph_ComplexFP64.
)
{

    GrB_Info info ;
    GrB_Matrix C = NULL ;
    GrB_Type type ;
    GrB_Index nrows, ncols ;

    // check inputs
    if (result == NULL)
    {
        // error: required parameter, result, is NULL
        return (GrB_NULL_POINTER) ;
    }
    (*result) = false ;

    // get the type and size of A
    LAGRAPH_OK (GxB_Matrix_type  (&type,  A)) ;
    LAGRAPH_OK (GrB_Matrix_nrows (&nrows, A)) ;
    LAGRAPH_OK (GrB_Matrix_ncols (&ncols, A)) ;

    if (type == GrB_BOOL)
    {
        // result = and (A)
        LAGRAPH_OK (GrB_reduce (result, NULL, LAGraph_LAND_MONOID, A, NULL)) ;
    }
    else
    {

        // select the unary operator
        GrB_UnaryOp op = NULL ;
        if      (type == GrB_INT8  ) op = LAGraph_ISONE_INT8   ;
        else if (type == GrB_INT16 ) op = LAGraph_ISONE_INT16  ;
        else if (type == GrB_INT32 ) op = LAGraph_ISONE_INT32  ;
        else if (type == GrB_INT64 ) op = LAGraph_ISONE_INT64  ;
        else if (type == GrB_UINT8 ) op = LAGraph_ISONE_UINT8  ;
        else if (type == GrB_UINT16) op = LAGraph_ISONE_UINT16 ;
        else if (type == GrB_UINT32) op = LAGraph_ISONE_UINT32 ;
        else if (type == GrB_UINT64) op = LAGraph_ISONE_UINT64 ;
        else if (type == GrB_FP32  ) op = LAGraph_ISONE_FP32   ;
        else if (type == GrB_FP64  ) op = LAGraph_ISONE_FP64   ;
        else if (type == LAGraph_ComplexFP64) op = LAGraph_ISONE_ComplexFP64 ;
        else op = userop ;

        if (op == NULL)
        {
            // printf ("LAGraph_ispattern: userop is NULL\n") ;
            return (GrB_NULL_POINTER) ;
        }

        // C = isone (A)
        LAGRAPH_OK (GrB_Matrix_new (&C, GrB_BOOL, nrows, ncols)) ;

        LAGRAPH_OK (GrB_apply (C, NULL, NULL, op, A, NULL)) ;

        // result = and (C)
        LAGRAPH_OK (GrB_reduce (result, NULL, LAGraph_LAND_MONOID, C, NULL)) ;
    }

    // free workspace and return result
    LAGRAPH_FREE_ALL ;
    return (GrB_SUCCESS) ;
}

