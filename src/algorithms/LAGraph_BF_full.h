//------------------------------------------------------------------------------
// LAGraph_BF_full.c: Bellman-Ford single-source shortest paths, returns tree
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

// LAGraph_BF_full: Bellman-Ford single source shortest paths, returning both
// the path lengths and the shortest-path tree.  contributed by Jinhao Chen and
// Tim Davis, Texas A&M.

// LAGraph_BF_full performs a Bellman-Ford to find out shortest path, parent
// nodes along the path and the hops (number of edges) in the path from given
// source vertex s in the range of [0, n) on graph given as matrix A with size
// n*n. The sparse matrix A has entry A(i, j) if there is an edge from vertex i
// to vertex j with weight w, then A(i, j) = w.

// TODO: think about the return values

// LAGraph_BF_full returns GrB_SUCCESS regardless of existence of negative-
// weight cycle. However, the GrB_Vector d(k), pi(k) and h(k)  (i.e.,
// *pd_output, *ppi_output and *ph_output respectively) will be NULL when
// negative-weight cycle detected. Otherwise, the vector d has d(k) as the
// shortest distance from s to k. pi(k) = p+1, where p is the parent node of
// k-th node in the shortest path. In particular, pi(s) = 0. h(k) = hop(s, k),
// the number of edges from s to k in the shortest path.

//------------------------------------------------------------------------------

#pragma once

#include "../../deps/GraphBLAS/Include/GraphBLAS.h"

GrB_Info LAGraph_BF_full
(
	GrB_Vector *pd_output,      //the pointer to the vector of distance
	GrB_Vector *ppi_output,     //the pointer to the vector of parent
	GrB_Vector *ph_output,       //the pointer to the vector of hops
	const GrB_Matrix A,         //matrix for the graph
	const GrB_Index s           //given index of the source
);
