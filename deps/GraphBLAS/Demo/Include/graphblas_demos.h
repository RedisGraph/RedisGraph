//------------------------------------------------------------------------------
// GraphBLAS/Demo/Include/graphblas_demos.h: include file for all demo programs
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#ifndef GRAPHBLAS_DEMOS_H
#define GRAPHBLAS_DEMOS_H

//------------------------------------------------------------------------------
// manage compiler warnings
//------------------------------------------------------------------------------

#if defined __INTEL_COMPILER
#pragma warning (disable: 58 167 144 177 181 186 188 589 593 869 981 1418 1419 1572 1599 2259 2282 2557 2547 3280 )
#elif defined __GNUC__

// disable warnings for gcc 5.x and higher:
#if (__GNUC__ > 4)
// disable warnings
// #pragma GCC diagnostic ignored "-Wunknown-warning-option"
#pragma GCC diagnostic ignored "-Wint-in-bool-context"
#pragma GCC diagnostic ignored "-Wformat-truncation="
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
// enable these warnings as errors
#pragma GCC diagnostic error "-Wmisleading-indentation"
#endif

#pragma GCC diagnostic ignored "-Wunknown-pragmas"
#if !defined ( __cplusplus )
#pragma GCC diagnostic ignored "-Wincompatible-pointer-types"
#else
#pragma GCC diagnostic ignored "-Wwrite-strings"
#endif
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-result"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wtype-limits"

// enable these warnings as errors
#pragma GCC diagnostic error "-Wswitch-default"
#endif

#ifndef GB_MICROSOFT
#if ( _MSC_VER && !__INTEL_COMPILER )
#define GB_MICROSOFT 1
#else
#define GB_MICROSOFT 0
#endif
#endif

#include "GraphBLAS.h"
#include "simple_rand.h"
#include "simple_timer.h"
#include "usercomplex.h"
#include "prand.h"

#ifdef MATLAB_MEX_FILE
#include "mex.h"
#include "matrix.h"
#define malloc  mxMalloc
#define free    mxFree
#define calloc  mxCalloc
#define realloc mxRealloc
#endif

#undef MIN
#undef MAX
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#define MAX(a,b) (((a) > (b)) ? (a) : (b))

GB_PUBLIC
GrB_Info bfs5m              // BFS of a graph (using vector assign & reduce)
(
    GrB_Vector *v_output,   // v [i] is the BFS level of node i in the graph
    const GrB_Matrix A,     // input graph, treated as if boolean in semiring
    GrB_Index s             // starting node of the BFS
) ;

GB_PUBLIC
GrB_Info bfs5m_check        // BFS of a graph (using vector assign & reduce)
(
    GrB_Vector *v_output,   // v [i] is the BFS level of node i in the graph
    const GrB_Matrix A,     // input graph, treated as if boolean in semiring
    GrB_Index s             // starting node of the BFS
) ;

GB_PUBLIC
GrB_Info bfs6               // BFS of a graph (using apply)
(
    GrB_Vector *v_output,   // v [i] is the BFS level of node i in the graph
    const GrB_Matrix A,     // input graph, treated as if boolean in semiring
    GrB_Index s             // starting node of the BFS
) ;

GB_PUBLIC
GrB_Info bfs6_check         // BFS of a graph (using apply)
(
    GrB_Vector *v_output,   // v [i] is the BFS level of node i in the graph
    const GrB_Matrix A,     // input graph, treated as if boolean in semiring
    GrB_Index s             // starting node of the BFS
) ;

GB_PUBLIC
GrB_Info read_matrix        // read a double-precision matrix
(
    GrB_Matrix *A,          // handle of matrix to create
    FILE *f,                // file to read the tuples from
    bool make_symmetric,    // if true, return A as symmetric
    bool no_self_edges,     // if true, then remove self edges from A
    bool one_based,         // if true, input matrix is 1-based
    bool boolean,           // if true, input is GrB_BOOL, otherwise GrB_FP64
    bool printstuff         // if true, print status to stdout
) ;

GB_PUBLIC
GrB_Info mis                    // compute a maximal independent set
(
    GrB_Vector *iset_output,    // iset(i) = true if i is in the set
    const GrB_Matrix A,         // symmetric Boolean matrix
    int64_t seed                // random number seed
) ;

GB_PUBLIC
GrB_Info mis_check              // compute a maximal independent set
(
    GrB_Vector *iset_output,    // iset(i) = true if i is in the set
    const GrB_Matrix A,         // symmetric Boolean matrix
    int64_t seed                // random number seed
) ;

GB_PUBLIC
void mis_score  (void *result, const void *degree) ;

GB_PUBLIC
void mis_score2 (void *result, const void *degree, const void *xrand) ;

GB_PUBLIC
GrB_Info random_matrix      // create a random double-precision matrix
(
    GrB_Matrix *A_output,   // handle of matrix to create
    bool make_symmetric,    // if true, return A as symmetric
    bool no_self_edges,     // if true, then do not create self edges
    int64_t nrows,          // number of rows
    int64_t ncols,          // number of columns
    int64_t ntuples,        // number of entries (x2 if made symmetric)
    int method,             // method to use: 0:setElement, 1:build
    bool A_complex          // if true, create a Complex matrix
) ;

GB_PUBLIC
GrB_Info get_matrix         // get a matrix from stdin, or create random one
(
    GrB_Matrix *A_output,   // matrix to create
    int argc,               // command-line arguments
    char **argv,
    bool no_self_edges,     // if true, ensure the matrix has no self-edges
    bool boolean            // if true, file is read as GrB_BOOL, else GrB_FP64
) ;

GB_PUBLIC
GrB_Info wathen             // construct a random Wathen matrix
(
    GrB_Matrix *A_output,   // output matrix
    int64_t nx,             // grid dimension nx
    int64_t ny,             // grid dimension ny
    bool scale,             // if true, scale the rows
    int method,             // 0 to 3
    double *rho_given       // nx-by-ny dense matrix, if NULL use random rho
) ;

GB_PUBLIC
GrB_Info triu               // C = triu (A,1)
(
    GrB_Matrix *C_output,   // output matrix
    const GrB_Matrix A      // input matrix, boolean or double
) ;

GB_PUBLIC
GrB_Info tricount           // count # of triangles
(
    int64_t *ntri,          // # of triangles in the graph
    const int method,       // 0 to 4, see above
    const GrB_Matrix A,     // adjacency matrix for methods 0, 1, and 2
    const GrB_Matrix E,     // edge incidence matrix for method 0
    const GrB_Matrix L,     // L=tril(A) for methods 2, 4, and 4
    const GrB_Matrix U,     // U=triu(A) for methods 2, 3, and 5
    double t [2]            // t [0]: multiply time, t [1]: reduce time
) ;

GB_PUBLIC
GrB_Info isequal_type       // return GrB_SUCCESS if successful
(
    bool *result,           // true if A == B, false if A != B or error
    GrB_Matrix A,
    GrB_Matrix B,
    GrB_BinaryOp op         // should be GrB_EQ_<type>, for the type of A and B
) ;

GB_PUBLIC
GrB_Info isequal            // return GrB_SUCCESS if successful
(
    bool *result,           // true if A == B, false if A != B or error
    GrB_Matrix A,
    GrB_Matrix B,
    GrB_BinaryOp userop     // for A and B with user-defined types.  ignored
                            // if A and B are of built-in types
) ;

//------------------------------------------------------------------------------
// page rank
//------------------------------------------------------------------------------

// dpagerank computes an array of structs for its result
typedef struct
{
    double pagerank ;   // the pagerank of a node
    GrB_Index page ;    // the node number itself
}
PageRank ;

// ipagerank computes an array of structs for its result
typedef struct
{
    uint64_t pagerank ;     // the pagerank of a node
    GrB_Index page ;        // the node number itself
}
iPageRank ;

// using a standard semiring and FP64 arithmetic
GB_PUBLIC
GrB_Info dpagerank          // GrB_SUCCESS or error condition
(
    PageRank **Phandle,     // output: pointer to array of PageRank structs
    GrB_Matrix A
) ;

// like dpagerank but with user-defined type, operators, and semiring;
// also a stopping critirion
GB_PUBLIC
GrB_Info dpagerank2         // GrB_SUCCESS or error condition
(
    PageRank **Phandle,     // output: pointer to array of PageRank structs
    GrB_Matrix A,           // input graph, not modified
    int itermax,            // max number of iterations
    double tol,             // stop when norm (r-rnew,2) < tol
    int *iters,             // number of iterations taken
    GrB_Desc_Value method   // method to use for GrB_vxm (for testing only)
) ;

GB_PUBLIC
GrB_Info drowscale          // GrB_SUCCESS or error condition
(
    GrB_Matrix *Chandle,    // output matrix C = rowscale (A)
    GrB_Matrix A            // input matrix, not modified
) ;

GB_PUBLIC
GrB_Info ipagerank          // GrB_SUCCESS or error condition
(
    iPageRank **Phandle,    // output: pointer to array of iPageRank structs
    GrB_Matrix A            // input graph, not modified
) ;

GB_PUBLIC
GrB_Info irowscale          // GrB_SUCCESS or error condition
(
    GrB_Matrix *Chandle,    // output matrix C = rowscale (A)
    GrB_Matrix A            // input matrix, not modified
) ;

// multiplicative scaling factor for ipagerank, ZSCALE = 2^30
#define ZSCALE ((uint64_t) 1073741824)

//------------------------------------------------------------------------------
// import/export test
//------------------------------------------------------------------------------

GB_PUBLIC
GrB_Info import_test (GrB_Matrix *C_handle, int format, bool dump) ;

//------------------------------------------------------------------------------
// CHECK: expr must be true; if not, return an error condition
//------------------------------------------------------------------------------

// the #include'ing file must define the FREE_ALL macro

#define CHECK(expr,info)                                                \
{                                                                       \
    if (! (expr))                                                       \
    {                                                                   \
        /* free the result and all workspace, and return NULL */        \
        FREE_ALL ;                                                      \
        printf ("Failure: line %d file %s\n", __LINE__, __FILE__) ;     \
        return (info) ;                                                 \
    }                                                                   \
}

//------------------------------------------------------------------------------
// OK  call a GraphBLAS method and check the result
//------------------------------------------------------------------------------

// OK(method) is a macro that calls a GraphBLAS method and checks the status;
// if a failure occurs, it handles the error via the CHECK macro above, and
// returns the error status to the caller.

#define OK(method)                                                      \
{                                                                       \
    info = method ;                                                     \
    if (!(info == GrB_SUCCESS || info == GrB_NO_VALUE))                 \
    {                                                                   \
        printf ("GraphBLAS error: %d\n", info) ;                        \
        CHECK (false, info) ;                                           \
    }                                                                   \
}

#endif
