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

#pragma once

#include "GraphBLAS.h"


GrB_Info LAGraph_bfs_pushpull   // push-pull BFS, or push-only if AT = NULL
(
	GrB_Vector *v_output,   // v(i) is the BFS level of node i in the graph
	GrB_Vector *pi_output,  // pi(i) = p+1 if p is the parent of node i.
	// if NULL, the parent is not computed.
	GrB_Matrix A,           // input graph, treated as if boolean in semiring
	GrB_Matrix AT,          // transpose of A (optional; push-only if NULL)
	int64_t source,         // starting node of the BFS
	int64_t max_level,      // optional limit of # levels to search
	bool vsparse            // if true, v is expected to be very sparse
);

