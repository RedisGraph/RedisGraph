
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

// Contributed by Jinhao Chen, Texas A&M

#include "LAGraph.h"

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
    const double *W  // weight vector, W(i) = weight of edge (I(i),J(i))
);

GrB_Info LAGraph_BF_basic_mxv
(
    GrB_Vector *pd_output,      //the pointer to the vector of distance
    const GrB_Matrix AT,        //transposed adjacency matrix for the graph
    const GrB_Index s           //given index of the source
);

GrB_Info LAGraph_BF_full_mxv
( 
    GrB_Vector *pd_output,      //the pointer to the vector of distance 
    GrB_Vector *ppi_output,     //the pointer to the vector of parent 
    GrB_Vector *ph_output,      //the pointer to the vector of hops 
    const GrB_Matrix AT,        //transposed adjacency matrix for the graph 
    const GrB_Index s           //given index of the source 
);

