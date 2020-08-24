//------------------------------------------------------------------------------
// LAGraph_prune_diag: remove diagonal entries from a matrix
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

// LAGraph_prune_diag: contributed by Tim Davis.  Removes all diagonal entries
// from a matrix.

//------------------------------------------------------------------------------

#define LAGRAPH_FREE_ALL GrB_free (&M)

#include "LAGraph_internal.h"

GrB_Info LAGraph_prune_diag // remove all entries from the diagonal
(
    GrB_Matrix A
)
{

    GrB_Info info ;
    GrB_Matrix M = NULL ;

    GrB_Index m, n ;
    LAGRAPH_OK (GrB_Matrix_nrows (&m, A)) ;
    LAGRAPH_OK (GrB_Matrix_nrows (&n, A)) ;
    int64_t k = LAGRAPH_MIN (m, n) ;

    // M = diagonal mask matrix
    LAGRAPH_OK (GrB_Matrix_new (&M, GrB_BOOL, m, n)) ;
    for (int64_t i = 0 ; i < k ; i++)
    {
        // M(i,i) = true ;
        LAGRAPH_OK (GrB_Matrix_setElement (M, (bool) true, i, i)) ;
    }

    // remove self edges (via M)
    LAGRAPH_OK (GrB_assign (A, M, NULL, A, GrB_ALL, m, GrB_ALL, n,
        LAGraph_desc_oocr)) ;
    GrB_free (&M) ;

    return (GrB_SUCCESS) ;
}

