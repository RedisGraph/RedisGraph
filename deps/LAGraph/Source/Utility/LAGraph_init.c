//------------------------------------------------------------------------------
// LAGraph_init:  start LAGraph
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

// LAGraph_init:  start LAGraph, contributed by Tim Davis, Texas A&M

// Initialize GraphBLAS and LAGraph, using default memory allocation functions.
// See also LAGraph_xinit.

#include "LAGraph_internal.h"

GrB_Info LAGraph_init ( )
{
    #ifdef MATLAB_MEX_FILE
    // use MATLAB memory allocation functions
    return (LAGraph_xinit (mxMalloc, mxCalloc, mxRealloc, mxFree, false)) ;
    #else
    // use ANSI C memory allocation functions
    return (LAGraph_xinit (malloc, calloc, realloc, free, true)) ;
    #endif
}

