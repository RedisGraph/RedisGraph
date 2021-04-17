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

GrB_Info LAGraph_cdlp
(
    GrB_Vector *CDLP_handle, // output vector
    const GrB_Matrix A,      // input matrix
    bool symmetric,          // denote whether the matrix is symmetric
    bool sanitize,           // if true, ensure A is binary
    int itermax,             // max number of iterations,
    double *t                // t [0] = sanitize time, t [1] = cdlp time,
                             // in seconds
);