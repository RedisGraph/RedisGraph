//------------------------------------------------------------------------------
// LAGraph.h:  include file for user applications that use LAGraph
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

// TODO: add more comments to this file.

//------------------------------------------------------------------------------
// include files and global #defines
//------------------------------------------------------------------------------

#ifndef LAGRAPH_INCLUDE
#define LAGRAPH_INCLUDE

#include "GraphBLAS.h"
#include <complex.h>
#include <ctype.h>

// "I" is defined by <complex.h>, but is used in LAGraph and GraphBLAS to
// denote a list of row indices; remove it here.
#undef I

#define _POSIX_C_SOURCE 200809L
#include <time.h>

#if defined ( __linux__ )
#include <sys/time.h>
#endif

#if defined ( _OPENMP )
#include <omp.h>
#endif

#if defined ( __MACH__ )
#include <mach/clock.h>
#include <mach/mach.h>
#endif

#if defined __INTEL_COMPILER
// disable icc warnings
//  161:  unrecognized pragma
#pragma warning (disable: 161)
#endif

#define LAGRAPH_RAND_MAX 32767

// suitable for integers, and non-NaN floating point:
#define LAGRAPH_MAX(x,y) (((x) > (y)) ? (x) : (y))
#define LAGRAPH_MIN(x,y) (((x) < (y)) ? (x) : (y))

// free a block of memory and set the pointer to NULL
#define LAGRAPH_FREE(p)     \
{                           \
    LAGraph_free (p) ;      \
    p = NULL ;              \
}

//------------------------------------------------------------------------------
// LAGRAPH_OK: call LAGraph or GraphBLAS and check the result
//------------------------------------------------------------------------------

// To use LAGRAPH_OK, the #include'ing file must declare a scalar GrB_Info
// info, and must define LAGRAPH_FREE_ALL as a macro that frees all workspace
// if an error occurs.  The method can be a GrB_Info scalar as well, so that
// LAGRAPH_OK(info) works.  The function that uses this macro must return
// GrB_Info, or int.

#define LAGRAPH_ERROR(message,info)                                         \
{                                                                           \
    fprintf (stderr, "LAGraph error: %s\n[%d]\n%s\nFile: %s Line: %d\n",    \
        message, info, GrB_error ( ), __FILE__, __LINE__) ;                 \
    LAGRAPH_FREE_ALL ;                                                      \
    return (info) ;                                                         \
}

#define LAGRAPH_OK(method)                                                  \
{                                                                           \
    info = method ;                                                         \
    if (! (info == GrB_SUCCESS || info == GrB_NO_VALUE))                    \
    {                                                                       \
        LAGRAPH_ERROR ("", info) ;                                          \
    }                                                                       \
}

//------------------------------------------------------------------------------
// LAGraph_Context:
//------------------------------------------------------------------------------

// All LAGraph functions will use a Context for global parameters, error
// status, and the like.  So far, the parameter is only for LAGraph_random.

typedef struct
{
    int nthreads ;          // # of threads to use.  If <= 0, use defaults
                            // (from omp_get_max_threads)

    // TODO more can go here, like info, the GrB_error() results, etc.
}
LAGraph_Context ;

//------------------------------------------------------------------------------
// global objects
//------------------------------------------------------------------------------

// LAGraph_Complex is a GrB_Type containing the ANSI C11 double complex
// type.  This is required so that any arbitrary Matrix Market format
// can be read into GraphBLAS.
extern GrB_Type LAGraph_Complex ;

extern GrB_BinaryOp

    // binary operators to test for symmetry, skew-symmetry
    // and Hermitian property
    LAGraph_EQ_Complex          ,
    LAGraph_SKEW_INT8           ,
    LAGraph_SKEW_INT16          ,
    LAGraph_SKEW_INT32          ,
    LAGraph_SKEW_INT64          ,
    LAGraph_SKEW_FP32           ,
    LAGraph_SKEW_FP64           ,
    LAGraph_SKEW_Complex        ,
    LAGraph_Hermitian           ,
    LAGraph_LOR_UINT32          ;

extern GrB_UnaryOp

    // unary operators to check if the entry is equal to 1
    LAGraph_ISONE_INT8          ,
    LAGraph_ISONE_INT16         ,
    LAGraph_ISONE_INT32         ,
    LAGraph_ISONE_INT64         ,
    LAGraph_ISONE_UINT8         ,
    LAGraph_ISONE_UINT16        ,
    LAGraph_ISONE_UINT32        ,
    LAGraph_ISONE_UINT64        ,
    LAGraph_ISONE_FP32          ,
    LAGraph_ISONE_FP64          ,
    LAGraph_ISONE_Complex       ,

    // unary operators to check if the entry is equal to 2
    LAGraph_ISTWO_UINT32        ,

    // unary operators that decrement by 1
    LAGraph_DECR_INT32          ,
    LAGraph_DECR_INT64          ,

    // unary operator for lcc
    LAGraph_COMB_FP64           ,

    // unary ops to check if greater than zero
    LAGraph_GT0_FP32            ,
    LAGraph_GT0_FP64            ,

    // unary YMAX ops for DNN
    LAGraph_YMAX_FP32           ,
    LAGraph_YMAX_FP64           ,

    // unary operators that return 1
    LAGraph_ONE_UINT32          ,
    LAGraph_ONE_FP64            ,
    LAGraph_TRUE_BOOL           ,
    LAGraph_TRUE_BOOL_Complex   ;

// monoids and semirings
extern GrB_Monoid

    LAGraph_PLUS_INT64_MONOID   ,
    LAGraph_MAX_INT32_MONOID    ,
    LAGraph_LAND_MONOID         ,
    LAGraph_LOR_MONOID          ,
    LAGraph_MIN_INT32_MONOID    ,
    LAGraph_MIN_INT64_MONOID    ,
    LAGraph_PLUS_UINT32_MONOID  ,
    LAGraph_PLUS_FP32_MONOID    ,
    LAGraph_PLUS_FP64_MONOID    ;

extern GrB_Semiring

    LAGraph_LOR_LAND_BOOL       ,
    LAGraph_LOR_SECOND_BOOL     ,
    LAGraph_LOR_FIRST_BOOL      ,
    LAGraph_MIN_SECOND_INT32    ,
    LAGraph_MIN_FIRST_INT32     ,
    LAGraph_MIN_SECOND_INT64    ,
    LAGraph_MIN_FIRST_INT64     ,
    LAGraph_PLUS_TIMES_UINT32   ,
    LAGraph_PLUS_TIMES_FP64     ,
    LAGraph_PLUS_PLUS_FP64      ,
    LAGraph_PLUS_TIMES_FP32     ,
    LAGraph_PLUS_PLUS_FP32      ;

// all 16 descriptors
// syntax: 4 characters define the following.  'o' is the default:
// 1: o or t: A transpose
// 2: o or t: B transpose
// 3: o or c: complemented mask
// 4: o or r: replace
extern GrB_Descriptor

    LAGraph_desc_oooo ,   // default (NULL)
    LAGraph_desc_ooor ,   // replace
    LAGraph_desc_ooco ,   // compl mask
    LAGraph_desc_oocr ,   // compl mask, replace

    LAGraph_desc_otoo ,   // A'
    LAGraph_desc_otor ,   // A', replace
    LAGraph_desc_otco ,   // A', compl mask
    LAGraph_desc_otcr ,   // A', compl mask, replace

    LAGraph_desc_tooo ,   // B'
    LAGraph_desc_toor ,   // B', replace
    LAGraph_desc_toco ,   // B', compl mask
    LAGraph_desc_tocr ,   // B', compl mask, replace

    LAGraph_desc_ttoo ,   // A', B'
    LAGraph_desc_ttor ,   // A', B', replace
    LAGraph_desc_ttco ,   // A', B', compl mask
    LAGraph_desc_ttcr ;   // A', B', compl mask, replace

extern GxB_SelectOp LAGraph_support ;

//------------------------------------------------------------------------------
// memory management functions
//------------------------------------------------------------------------------

// use the ANSI C functions by default (or mx* functions if the #ifdef
// above redefines them).  See Source/Utility/LAGraph_malloc.c.

extern void * (* LAGraph_malloc_function  ) (size_t)         ;
extern void * (* LAGraph_calloc_function  ) (size_t, size_t) ;
extern void * (* LAGraph_realloc_function ) (void *, size_t) ;
extern void   (* LAGraph_free_function    ) (void *)         ;
extern bool LAGraph_malloc_is_thread_safe ;

//------------------------------------------------------------------------------
// user-callable utility functions
//------------------------------------------------------------------------------

typedef void (*LAGraph_binary_function) (void *, const void *, const void *) ;

GrB_Info LAGraph_init ( ) ;         // start LAGraph

GrB_Info LAGraph_xinit              // start LAGraph (alternative method)
(
    // pointers to memory management functions
    void * (* user_malloc_function  ) (size_t),
    void * (* user_calloc_function  ) (size_t, size_t),
    void * (* user_realloc_function ) (void *, size_t),
    void   (* user_free_function    ) (void *),
    bool user_malloc_is_thread_safe
) ;

GrB_Info LAGraph_finalize ( ) ;     // end LAGraph

GrB_Info LAGraph_mmread
(
    GrB_Matrix *A,      // handle of matrix to create
    FILE *f             // file to read from, already open
) ;

GrB_Info LAGraph_mmwrite
(
    GrB_Matrix A,           // matrix to write to the file
    FILE *f                 // file to write it to
    // TODO , FILE *fcomments         // optional file with extra comments
) ;

GrB_Info LAGraph_tsvread        // returns GrB_SUCCESS if successful
(
    GrB_Matrix *Chandle,        // C, created on output
    FILE *f,                    // file to read from (already open)
    GrB_Type type,              // the type of C to create
    GrB_Index nrows,            // C is nrows-by-ncols
    GrB_Index ncols
) ;

GrB_Info LAGraph_ispattern  // return GrB_SUCCESS if successful
(
    bool *result,           // true if A is all one, false otherwise
    GrB_Matrix A,
    GrB_UnaryOp userop      // for A with arbitrary user-defined type.
                            // Ignored if A and B are of built-in types or
                            // LAGraph_Complex.
) ;

GrB_Info LAGraph_pattern    // return GrB_SUCCESS if successful
(
    GrB_Matrix *C,          // a boolean matrix with the pattern of A
    GrB_Matrix A
) ;

GrB_Info LAGraph_isequal    // return GrB_SUCCESS if successful
(
    bool *result,           // true if A == B, false if A != B or error
    GrB_Matrix A,
    GrB_Matrix B,
    GrB_BinaryOp userop     // for A and B with arbitrary user-defined types.
                            // Ignored if A and B are of built-in types or
                            // LAGraph_Complex.
) ;

GrB_Info LAGraph_Vector_isequal    // return GrB_SUCCESS if successful
(
    bool *result,           // true if A == B, false if A != B or error
    GrB_Vector A,
    GrB_Vector B,
    GrB_BinaryOp userop     // for A and B with arbitrary user-defined types.
                            // Ignored if A and B are of built-in types or
                            // LAGraph_Complex.
) ;

GrB_Info LAGraph_isall      // return GrB_SUCCESS if successful
(
    bool *result,           // true if A == B, false if A != B or error
    GrB_Matrix A,
    GrB_Matrix B,
    GrB_BinaryOp op         // GrB_EQ_<type>, for the type of A and B,
                            // to check for equality.  Or use any desired
                            // operator.  The operator should return GrB_BOOL.
) ;

GrB_Info LAGraph_Vector_isall      // return GrB_SUCCESS if successful
(
    bool *result,           // true if A == B, false if A != B or error
    GrB_Vector A,
    GrB_Vector B,
    GrB_BinaryOp op         // GrB_EQ_<type>, for the type of A and B,
                            // to check for equality.  Or use any desired
                            // operator.  The operator should return GrB_BOOL.
) ;

uint64_t LAGraph_rand (uint64_t *seed) ;

uint64_t LAGraph_rand64 (uint64_t *seed) ;

double LAGraph_randx (uint64_t *seed) ;

GrB_Info LAGraph_random         // create a random matrix
(
    GrB_Matrix *A,              // handle of matrix to create
    GrB_Type type,              // built-in type, or LAGraph_Complex
    GrB_Index nrows,            // number of rows
    GrB_Index ncols,            // number of columns
    GrB_Index nvals,            // number of values
    bool make_pattern,          // if true, A is a pattern
    bool make_symmetric,        // if true, A is symmetric
    bool make_skew_symmetric,   // if true, A is skew-symmetric
    bool make_hermitian,        // if trur, A is hermitian
    bool no_diagonal,           // if true, A has no entries on the diagonal
    uint64_t *seed              // random number seed; modified on return
) ;

GrB_Info LAGraph_alloc_global ( ) ;

GrB_Info LAGraph_free_global ( ) ;

void *LAGraph_malloc        // wrapper for malloc
(
    size_t nitems,          // number of items
    size_t size_of_item     // size of each item
) ;

void LAGraph_free           // wrapper for free
(
    void *p
) ;

void LAGraph_tic            // gets current time in seconds and nanoseconds
(
    double tic [2]          // tic [0]: seconds, tic [1]: nanoseconds
) ;

double LAGraph_toc          // returns time since last LAGraph_tic
(
    const double tic [2]    // tic from last call to LAGraph_tic
) ;

GrB_Info LAGraph_prune_diag // remove all entries from the diagonal
(
    GrB_Matrix A
) ;

//------------------------------------------------------------------------------
// user-callable algorithms
//------------------------------------------------------------------------------

GrB_Info LAGraph_bfs_pushpull   // push-pull BFS, or push-only if AT = NULL
(
    GrB_Vector *v_output,   // v(i) is the BFS level of node i in the graph
    GrB_Vector *pi_output,  // pi(i) is the parent of node i in the graph.
                            // if NULL, the parent is not computed
    GrB_Matrix A,           // input graph, treated as if boolean in semiring
    GrB_Matrix AT,          // transpose of A (optional; push-only if NULL)
    int64_t s,              // starting node of the BFS (s < 0: whole graph)
    int64_t max_level,      // optional limit of # levels to search
    bool vsparse            // if true, v is expected to be very sparse
) ;

GrB_Info LAGraph_bfs_simple     // push-only BFS
(
    GrB_Vector *v_output,   // v(i) is the BFS level of node i in the graph
    const GrB_Matrix A,     // input graph, treated as if boolean in semiring
    GrB_Index s             // starting node of the BFS
) ;

GrB_Info LAGraph_lacc (GrB_Matrix A) ;

// LAGraph_pagrank computes an array of structs for its result
typedef struct
{
    double pagerank ;   // the pagerank of a node
    GrB_Index page ;    // the node number itself
}
LAGraph_PageRank ;

GrB_Info LAGraph_pagerank       // GrB_SUCCESS or error condition
(
    LAGraph_PageRank **Phandle, // output: array of LAGraph_PageRank structs
    GrB_Matrix A,               // binary input graph, not modified
    int itermax,                // max number of iterations
    double tol,                 // stop when norm (r-rnew,2) < tol
    int *iters                  // number of iterations taken
) ;

GrB_Info LAGraph_tricount   // count # of triangles
(
    int64_t *ntri,          // # of triangles
    const int method,       // 0 to 5, see above
    const GrB_Matrix A,     // adjacency matrix for methods 0, 1, and 2
    const GrB_Matrix E,     // edge incidence matrix for method 0
    const GrB_Matrix L,     // L=tril(A) for methods 2, 4, and 4
    const GrB_Matrix U,     // U=triu(A) for methods 2, 3, and 5
    double t [2]            // t [0]: multiply time, t [1]: reduce time
) ;

GrB_Info LAGraph_ktruss         // compute the k-truss of a graph
(
    GrB_Matrix *Chandle,        // output k-truss subgraph, C
    const GrB_Matrix A,         // input adjacency matrix, A, not modified
    const uint32_t k,           // find the k-truss, where k >= 3
    int32_t *nsteps             // # of steps taken (ignored if NULL)
) ;

GrB_Info LAGraph_allktruss      // compute all k-trusses of a graph
(
    GrB_Matrix *Cset,           // size n, output k-truss subgraphs (optional)
    GrB_Matrix A,               // input adjacency matrix, A, not modified
    // output statistics
    int64_t *kmax,              // smallest k where k-truss is empty
    int64_t *ntris,             // size n, ntris [k] is #triangles in k-truss
    int64_t *nedges,            // size n, nedges [k] is #edges in k-truss
    int64_t *nstepss            // size n, nstepss [k] is #steps for k-truss
) ;

GrB_Info LAGraph_BF_full
(
    GrB_Vector *pd,             //the pointer to the vector of distance
    GrB_Vector *ppi,            //the pointer to the vector of parent
    GrB_Vector *ph,             //the pointer to the vector of hops
    const GrB_Matrix A,         //matrix for the graph
    const GrB_Index s           //given index of the source
) ;

GrB_Info LAGraph_BF_basic
(
    GrB_Vector *pd,             //the pointer to the vector of distance
    const GrB_Matrix A,         //matrix for the graph
    const GrB_Index s           //given index of the source
) ;

GrB_Info LAGraph_lcc            // compute lcc for all nodes in A
(
    GrB_Vector *LCC_handle,     // output vector
    const GrB_Matrix A,         // input matrix
    bool sanitize               // if true, ensure A is binary
) ;

GrB_Info LAGraph_dnn    // returns GrB_SUCCESS if successful
(
    // output
    GrB_Matrix *Yhandle,    // Y, created on output
    // input: not modified
    GrB_Matrix *W,      // W [0..nlayers-1], each nneurons-by-nneurons
    GrB_Matrix *Bias,   // Bias [0..nlayers-1], diagonal nneurons-by-nneurons
    int nlayers,        // # of layers
    GrB_Matrix Y0       // input features: nfeatures-by-nneurons
) ;

#endif
