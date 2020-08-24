//------------------------------------------------------------------------------
// LAGraph_set_nthreads: set the # of threads to use
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

// LAGraph_set_nthreads: set # of threads to use
// contributed by Tim Davis, Texas A&M

// See LAGraph_get_nthreads for details on what this function does.

#include "LAGraph_internal.h"

int LAGraph_set_nthreads        // returns # threads set, 0 if nothing done
(
    int nthreads
)
{

    #if defined ( GxB_SUITESPARSE_GRAPHBLAS )
    GxB_set (GxB_NTHREADS, nthreads) ;
    #elif defined ( _OPENMP )
    omp_set_num_threads (nthreads) ;
    #else
    // nothing to do ...
    nthreads = 0 ;
    #endif

    return (nthreads) ;
}

