//------------------------------------------------------------------------------
// LAGraph_internal.h: include file for internal use in LAGraph
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

// LAGraph_internal.h: contributed by Tim Davis, Texas A&M

// Include file for LAGraph functions.  This file should not be included in
// user applications.  See LAGraph.h instead.

#include "LAGraph.h"

#ifndef CMPLX
#define CMPLX(real,imag) \
    ( \
    (double complex)((double)(real)) + \
    (double complex)((double)(imag) * _Complex_I) \
    )
#endif

#ifdef MATLAB_MEX_FILE
// compiling LAGraph for a MATLAB mexFunction.  Use mxMalloc, mxFree, etc.
#include "mex.h"
#include "matrix.h"
#define malloc  mxMalloc
#define free    mxFree
#define calloc  mxCalloc
#define realloc mxRealloc
#endif

//------------------------------------------------------------------------------
// code development settings
//------------------------------------------------------------------------------

// LAGRAPH_XSTR: convert the content of x into a string "x"
#define LAGRAPH_XSTR(x) LAGRAPH_STR(x)
#define LAGRAPH_STR(x) #x

// turn off debugging; do not edit these three lines
#ifndef NDEBUG
#define NDEBUG
#endif

// These flags are used for code development.  Uncomment them as needed.

// to turn on debugging, uncomment this line:
// #undef NDEBUG

#undef ASSERT

#ifndef NDEBUG

    // debugging enabled
    #ifdef MATLAB_MEX_FILE
    #define ASSERT(x) \
    {                                                                       \
        if (!(x))                                                           \
        {                                                                   \
            mexErrMsgTxt ("failure: " __FILE__ " line: "                    \
                LAGRAPH_XSTR(__LINE__)) ;                                   \
        }                                                                   \
    }
    #else
    #include <assert.h>
    #define ASSERT(x) assert (x) ;
    #endif

#else

    // debugging disabled
    #define ASSERT(x)

#endif

//------------------------------------------------------------------------------
// Matrix Market format
//------------------------------------------------------------------------------

// %%MatrixMarket matrix <fmt> <type> <storage> uses the following enums:

typedef enum
{
    MM_coordinate,
    MM_array,
}
MM_fmt_enum ;

typedef enum
{
    MM_real,
    MM_integer,
    MM_complex,
    MM_pattern
}
MM_type_enum ;

typedef enum
{
    MM_general,
    MM_symmetric,
    MM_skew_symmetric,
    MM_hermitian
}
MM_storage_enum ;

// maximum length of each line in the Matrix Market file format

// The MatrixMarket format specificies a maximum line length of 1024.
// This is currently sufficient for GraphBLAS but will need to be relaxed
// if this function is extended to handle arbitrary user-defined types.
#define MMLEN 1024
#define MAXLINE MMLEN+6

