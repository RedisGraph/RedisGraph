//------------------------------------------------------------------------------
// BF_pure_c.c: implementation of Bellman-Ford method for shortest paths in
// given graph using conventional method with c
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

// LAGraph_BF_full_mxv: Bellman-Ford single source shortest paths, returning
// both the path lenths and the shortest-path tree.  contributed by Jinhao Chen
// and Tim Davis, Texas A&M.

// LAGraph_BF_pure_c performs a Bellman-Ford to find out shortest path
// length, parent nodes along the path from given source vertex s in the range
// of [0, n) on graph with n nodes. It is implemented purely using conventional
// method. It is used here for checking the correctness of the result and
// comparison with the Bellman Ford implemented based on LAGraph. Therefore, it
// require the graph represented as triplet format (I, J, W), which is an edge
// from vertex I(k) to vertex J(k) with weight W(k), and also the number of
// vertices and number of edges.

// TODO: think about the retrun values
// LAGraph_BF_pure_c returns GrB_SUCCESS regardless of existence of negative-
// weight cycle. However, the vector d(k) and pi(k) (i.e., *pd, and *ppi
// respectively) will be NULL when negative-weight cycle detected. Otherwise,
// the vector d has d(k) as the shortest distance from s to k. pi(k) = p, where
// p is the parent node of k-th node in the shortest path. In particular,
// pi(s) = -1.

//------------------------------------------------------------------------------

#include "BF_test.h"

#define LAGRAPH_FREE_ALL      \
{                             \
    free(d);                  \
    free(pi);                 \
}

// Given the edges and corresponding weights of a graph in tuple
// form {I, J, W} and a source vertex s. If there is no negative-weight
// cycle reachable from s, returns GrB_SUCCESS and the shortest distance
// d and the shortest path tree pi. Otherwise return NULL pointer for d
// and pi.
GrB_Info LAGraph_BF_pure_c
(
    double **pd,     // pointer to distance vector d, d(k) = shorstest distance
                     // between s and k if k is reachable from s
    int64_t **ppi,   // pointer to parent index vector pi, pi(k) = parent of
                     // node k in the shortest path tree
    const int64_t s, // given source node index
    const int64_t n, // number of nodes
    const int64_t nz,// number of edges
    const int64_t *I,// row index vector
    const int64_t *J,// column index vector
    const double  *W // weight vector, W(i) = weight of edge (I(i),J(i))
)
{
    int64_t i, j, k;
    double *d = NULL;
    int64_t *pi = NULL;
    if (I == NULL || J == NULL || W == NULL || pd == NULL || ppi == NULL)
    {
        // required argument is missing
        LAGRAPH_ERROR ("required arguments are NULL", GrB_NULL_POINTER) ;
    }

    if (*pd != NULL) {free(*pd);}
    if (*ppi != NULL) {free(*ppi);}
    *pd = NULL; *ppi = NULL;

    if (s >= n || s < 0)
    {
        LAGRAPH_ERROR ("invalid value for source vertex s", GrB_INVALID_VALUE) ;
    }

    // allocate d and pi
    d = LAGraph_malloc(n, sizeof(double));
    pi = LAGraph_malloc(n, sizeof(int64_t));
    if (d == NULL || pi == NULL)
    {
        // out of memory
         LAGRAPH_ERROR ("out of memory", GrB_OUT_OF_MEMORY) ;
    }

    // initialize d to a vector of INF while set d(s) = 0
    // and pi to a vector of -1
    for (i = 0; i < n; i++)
    {
        d[i] = INFINITY;
        pi[i] = -1;
    }
    d[s] = 0;
    // start the RELAX process and print results after each loop
    bool new_path = true;     //variable indicating if new path is found
    int64_t count = 0;        //number of loops
    // terminate when no new path is found or more than n-1 loops
    while(new_path && count < n-1)
    {
        new_path = false;
        for (k = 0; k < nz; k++)
        {
            i = I[k];
            j = J[k];
            if (d[j] > d[i] + W[k])
            {
                d[j] = d[i] + W[k];
                pi[j] = i;
                new_path = true;
            }
        }
        count++;
    }

    // check for negative-weight cycle only when there was a new path in the
    // last loop, otherwise, there can't be a negative-weight cycle.
    if (new_path)
    {
        // Do another loop of RELAX to check for negative loop,
        // return true if there is negative-weight cycle;
        // otherwise, print the distance vector and return false.
        for (k = 0; k < nz; k++)
        {
            i = I[k];
            j = J[k];
            if (d[j] > d[i] + W[k])
            {
                // printf("A negative-weight cycle exists. \n");
                LAGRAPH_FREE_ALL;
                return (GrB_SUCCESS) ;
            }
        }
    }

    *pd = d;
    *ppi = pi;
    return (GrB_SUCCESS) ;
 }
