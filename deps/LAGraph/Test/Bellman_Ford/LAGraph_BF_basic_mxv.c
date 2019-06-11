//------------------------------------------------------------------------------
// LAGraph_BF_basic: Bellman-Ford method for single source shortest paths
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

// LAGraph_BF_basic_mxv: Bellman-Ford single source shortest paths, returning
// just the shortest path lengths.  Contributed by Jinhao Chen and Tim Davis,
// Texas A&M.

// LAGraph_BF_basic_mxv performs a Bellman-Ford to find out shortest path length
// from given source vertex s in the range of [0, n) on graph with n nodes.
// It works almost the same as LAGraph_BF_basic except that it performs update
// using GrB_mxv instead of GrB_vxm, therefore, it require the input matrix as 
// the transpose of adjacency matrix A with size n by n. That is, the input
// sparse matrix has entry AT(i, j) if there is edge from vertex j to vertex i
// with weight w, then AT(i, j) = w. While same as LAGraph_BF_basic, it requires
// AT(i, i) = 0 for all 0 <= i < n.

// LAGraph_BF_basic_mxv returns GrB_SUCCESS regardless of existence of
// negative-weight cycle. However, the GrB_Vector d(k) (i.e., *pd_output) will
// be NULL when negative-weight cycle detected. Otherwise, the vector d has
// d(k) as the shortest distance from s to k.

//------------------------------------------------------------------------------

#include "BF_test.h"

#define LAGRAPH_FREE_ALL   \
{                          \
    GrB_free(&d) ;         \
    GrB_free(&dtmp) ;      \
}


// Given the transposed of a n-by-n adjacency matrix A and a source vertex s.
// If there is no negative-weight cycle reachable from s, return the distances
// of shortest paths from s as vector d. Otherwise, return d=NULL if there is
// negative-weight cycle.
// pd_output = &d, where d is a GrB_Vector with d(k) as the shortest distance
// from s to k when no negative-weight cycle detected, otherwise, d = NULL.
// AT has zeros on diagonal and weights on corresponding entries of edges
// s is given index for source vertex
GrB_Info LAGraph_BF_basic_mxv
(
    GrB_Vector *pd_output,      //the pointer to the vector of distance
    const GrB_Matrix AT,        //transposed adjacency matrix for the graph
    const GrB_Index s           //given index of the source
)
{
    GrB_Info info;
    GrB_Index nrows, ncols;
    // tmp vector to store distance vector after n loops
    GrB_Vector d = NULL, dtmp = NULL;
  
    if (AT == NULL || pd_output == NULL)
    {
        // required argument is missing
        LAGRAPH_ERROR ("required arguments are NULL", GrB_NULL_POINTER) ;
    }

    *pd_output = NULL;
    LAGRAPH_OK (GrB_Matrix_nrows (&nrows, AT)) ;
    LAGRAPH_OK (GrB_Matrix_ncols (&ncols, AT)) ;
    if (nrows != ncols)
    {
        // AT must be square
        LAGRAPH_ERROR ("AT must be square", GrB_INVALID_VALUE) ;
    }
    GrB_Index n = nrows;           // n = # of vertices in graph
    
    if (s >= n || s < 0)
    {
        LAGRAPH_ERROR ("invalid value for source vertex s", GrB_INVALID_VALUE) ;
    }

    // Initialize distance vector, change the d[s] to 0
    LAGRAPH_OK (GrB_Vector_new(&d, GrB_FP64, n));
    LAGRAPH_OK (GrB_Vector_setElement_FP64(d, 0, s));

    // copy d to dtmp in order to create a same size of vector
    LAGRAPH_OK (GrB_Vector_dup(&dtmp, d));
 
    int64_t iter = 0;      //number of iterations
    bool same = false;     //variable indicating if d == dtmp

    // terminate when no new path is found or more than n-1 loops
    while (!same && iter < n - 1)
    {
        // excute semiring on d and AT, and save the result to d
        LAGRAPH_OK (GrB_mxv(dtmp, GrB_NULL, GrB_NULL, GxB_MIN_PLUS_FP64, AT, 
            d, GrB_NULL));

        LAGRAPH_OK (LAGraph_Vector_isequal(&same, dtmp, d, GrB_NULL));
        if (!same)
        {
            GrB_Vector ttmp = dtmp;
            dtmp = d;
            d = ttmp;
        }
        iter++;
    }

    // check for negative-weight cycle only when there was a new path in the 
    // last loop, otherwise, there can't be a negative-weight cycle.
    if (!same)
    {
        // excute semiring again to check for negative-weight cycle
        LAGRAPH_OK (GrB_mxv(dtmp, GrB_NULL, GrB_NULL, GxB_MIN_PLUS_FP64, AT, 
            d, GrB_NULL));

	// if d != dtmp, then there is a negative-weight cycle in the graph
        LAGRAPH_OK (LAGraph_Vector_isequal(&same, dtmp, d, GrB_NULL));
        if (!same)
        {
            // printf("AT negative-weight cycle found. \n");
            LAGRAPH_FREE_ALL;
            return (GrB_SUCCESS) ;
        }
    }

    (*pd_output) = d;
    d = NULL;
    LAGRAPH_FREE_ALL;
    return (GrB_SUCCESS) ;
}
