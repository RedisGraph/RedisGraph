
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

// Contributed by Tim Davis, Texas A&M

#include "LAGraph.h"

GrB_Info LAGraph_bfs_pushpull_old       // TODO
(
    GrB_Vector *v,          // v [i] is the BFS level of node i in the graph
    const GrB_Matrix A,     // input graph, treated as if boolean in semiring
    const GrB_Matrix AT,    // transpose of A
    GrB_Index s,            // starting node of the BFS
    int32_t max_level       // max # of levels to search (<0: nothing,
                            // 1: just the source, 2: source and neighbors, etc)
) ;

GrB_Info LAGraph_bfs_pull
(
    GrB_Vector *v_output,   // v [i] is the BFS level of node i in the graph
    const GrB_Matrix A,     // input graph, treated as if boolean in semiring
    const GrB_Matrix AT,    // transpose of A
    GrB_Index s,            // starting node of the BFS
    int32_t max_level       // max # of levels to search (<0: nothing,
                            // 1: just the source, 2: source and neighbors, etc)
) ;

GrB_Info LAGraph_bfs2   // push (TODO rename this)
(
    GrB_Vector *v_output,   // v [i] is the BFS level of node i in the graph
    const GrB_Matrix A,     // input graph, treated as if boolean in semiring
    GrB_Index s,            // starting node of the BFS
    int32_t max_level       // max # of levels to search (<0: nothing,
                            // 1: just the source, 2: source and neighbors, etc)
) ;


