//------------------------------------------------------------------------------
// LAGraph.h: user-visible include file for LAGraph
//------------------------------------------------------------------------------

// LAGraph, (c) 2021 by The LAGraph Contributors, All Rights Reserved.
// SPDX-License-Identifier: BSD-2-Clause
//
// See additional acknowledgments in the LICENSE file,
// or contact permission@sei.cmu.edu for the full terms.

//------------------------------------------------------------------------------

// LAGraph is a package of graph algorithms based on GraphBLAS.  GraphBLAS
// defines a set of sparse matrix operations on an extended algebra of
// semirings, using an almost unlimited variety of operators and types.  When
// applied to sparse adjacency matrices, these algebraic operations are
// equivalent to computations on graphs.  GraphBLAS provides a powerful and
// expressive framework creating graph algorithms based on the elegant
// mathematics of sparse matrix operations on a semiring.

// However, GraphBLAS itself does not have graph algorithms.  The purpose of
// LAGraph is to provide a robust, easy-to-use high-performance library of
// graph algorithms that rely on GraphBLAS.

//------------------------------------------------------------------------------

#ifndef LAGRAPH_H
#define LAGRAPH_H

//==============================================================================
// include files
//==============================================================================

#include <time.h>
#include <ctype.h>

#include <GraphBLAS.h>
// #include "GxB_hiding.h"

#include "LAGraph_platform.h"
#include "LAGraph_internal.h"


//#include <complex.h>
// "I" is defined by <complex.h>, but is used in LAGraph and GraphBLAS to
// denote a list of row indices; remove it here.
//#undef I

//==============================================================================
// LAGraph error handling
//==============================================================================

// Each LAGraph function returns an int.  Negative values indicate an error,
// zero denotes success, and positive values denote success but with some kind
// of algorithm-specific note or warning.  In addition, all LAGraph functions
// have a final parameter that is a pointer to a user-allocated string in which
// an algorithm-specific error message can be returned.  If NULL, no error
// message is returned.  This is not itself an error condition, it just
// indicates that the caller does not need the message returned.  If the
// message string is provided but no error occurs, an empty string is returned.

// LAGRAPH_MSG_LEN: The maximum required length of a message string
#define LAGRAPH_MSG_LEN 256

// For example, the following call computes the breadth-first-search of an
// LAGraph_Graph G, starting at a given source node.  It returns a status of
// zero if it succeeds and a negative value on failure.

/*
    GrB_Vector level, parent ;
    char msg [LAGRAPH_MSG_LEN] ;
    int status = LAGraph_BreadthFirstSearch (&level, &parent, G, src, msg) ;
    if (status < 0)
    {
        printf ("error: %s\n", msg) ;
        // take corrective action ...
    }
*/

//------------------------------------------------------------------------------
// LAGraph_TRY: try an LAGraph method and check for errors
//------------------------------------------------------------------------------

// In a robust application, the return values from each call to LAGraph and
// GraphBLAS should be checked, and corrective action should be taken if an
// error occurs.  The LAGraph_TRY and GrB_TRY macros assist in this effort.

// LAGraph and GraphBLAS are written in C, and so they cannot rely on the
// try/catch mechanism of C++.  To accomplish a similar goal, each LAGraph file
// must #define its own file-specific macro called LAGraph_CATCH.  The typical
// usage of macro is to free any temporary matrices/vectors or workspace when
// an error occurs, and then "throw" the error by returning to the caller.  A
// user application may also #define LAGraph_CATCH and use these macros.

// A typical example of a user function that calls LAGraph might #define
// LAGraph_CATCH as follows.  Suppose workvector is a GrB_vector used for
// computations internal to the mybfs function, and W is a (double *) space
// created by malloc.

#if example_usage_only

    // an example user-defined LAGraph_CATCH macro
    #define LAGraph_CATCH(status)                                   \
    {                                                               \
        /* an LAGraph error has occurred */                         \
        printf ("LAGraph error: (%d): file: %s, line: %d\n%s\n",    \
            status, __FILE__, __LINE__, msg) ;                      \
        /* free any internal workspace and return the status */     \
        GrB_free (*parent) ;                                        \
        GrB_free (workvector) ;                                     \
        LAGraph_Free ((void **) &W) ;                               \
        return (status) ;                                           \
    }

    // an example user function that uses LAGraph_TRY / LAGraph_CATCH
    int mybfs (LAGraph_Graph G, GrB_Vector *parent, int64_t src)
    {
        GrB_Vector workvector = NULL ;
        double *W = NULL ;
        char msg [LAGRAPH_MSG_LEN] ;
        (*parent) = NULL ;
        LAGraph_TRY (LAGraph_BreadthFirstSearch (NULL, parent, G, src, msg)) ;
        // ...
        return (0) ;
    }

#endif

#define LAGraph_TRY(LAGraph_method)             \
{                                               \
    int LAGraph_status = LAGraph_method ;       \
    if (LAGraph_status < 0)                     \
    {                                           \
        LAGraph_CATCH (LAGraph_status) ;        \
    }                                           \
}

//------------------------------------------------------------------------------
// GrB_TRY: try a GraphBLAS method and check for errors
//------------------------------------------------------------------------------

// LAGraph provides a similar functionality for calling GraphBLAS methods.
// GraphBLAS returns info = 0 (GrB_SUCCESS) or 1 (GrB_NO_VALUE) on success, and
// a value > 1 on failure.  The user application must #define GrB_CATCH to use
// GrB_TRY.  Note that GraphBLAS_info is internal to this macro.  If the
// user application or LAGraph method wants a copy, a statement such as
// info = GraphBLAS_info ; where info is defined outside of this macro.

#define GrB_TRY(GrB_method)                                                  \
{                                                                            \
    GrB_Info GraphBLAS_info = GrB_method ;                                   \
    if (! (GraphBLAS_info == GrB_SUCCESS || GraphBLAS_info == GrB_NO_VALUE)) \
    {                                                                        \
        GrB_CATCH (GraphBLAS_info) ;                                         \
    }                                                                        \
}

//==============================================================================
// LAGraph memory management
//==============================================================================

// LAGraph provides wrappers for the malloc/calloc/realloc/free set of memory
// management functions, initialized by LAGraph_Init or LAGraph_Xinit.  By
// default, they are pointers to the ANSI C11 malloc/calloc/realloc/free
// functions.  Unlike all other LAGraph utility functions, these methods do not
// return an int, and do not have a final char *msg parameter.  Instead, they
// closely match the sytax of malloc/calloc/realloc/free.  LAGraph_Calloc and
// LAGraph_free have the same syntax as calloc and free.  LAGraph_Malloc has
// the syntax of calloc instead of malloc.  LAGraph_Realloc is very different
// from realloc, since the ANSI C11 realloc syntax is difficult to use safely.

// Only LAGraph_Malloc_function and LAGraph_Free_function are required.
// LAGraph_Calloc_function may be NULL, in which case LAGraph_Malloc and memset
// are used.  Likewise, LAGraph_Realloc_function may be NULL, in which case
// LAGraph_Malloc, memcpy, and LAGraph_Free are used.

extern void * (* LAGraph_Malloc_function  ) (size_t)         ;
extern void * (* LAGraph_Calloc_function  ) (size_t, size_t) ;
extern void * (* LAGraph_Realloc_function ) (void *, size_t) ;
extern void   (* LAGraph_Free_function    ) (void *)         ;
extern bool LAGraph_Malloc_is_thread_safe ;

// LAGraph_Malloc:  allocate a block of memory (wrapper for malloc)
void *LAGraph_Malloc
(
    size_t nitems,          // number of items
    size_t size_of_item     // size of each item
) ;

// LAGraph_Calloc:  allocate a block of memory (wrapper for calloc)
void *LAGraph_Calloc
(
    size_t nitems,          // number of items
    size_t size_of_item     // size of each item
) ;

void *LAGraph_Realloc       // returns pointer to reallocated block of memory,
(                           // or original block if reallocation fails.
    size_t nitems_new,      // new number of items in the object
    size_t nitems_old,      // old number of items in the object
    size_t size_of_item,    // size of each item
    // input/output
    void *p,                // old block to reallocate
    // output
    bool *ok                // true if successful, false otherwise
) ;

// LAGraph_Free:  free a block of memory (wrapper for free)
void LAGraph_Free
(
    void **p                // pointer to object to free, does nothing if NULL
) ;

//==============================================================================
// LAGraph data structures
//==============================================================================

// In addition to relying on underlying GraphBLAS objects (GrB_Matrix,
// GrB_Vector, GrB_Descriptor, ...), LAGraph adds the LAGraph_Graph.  This
// object contains a representation of a graph and its associated data.

// LAGRAPH_UNKNOWN is used for all scalars whose value is not known
#define LAGRAPH_UNKNOWN (-1)

//------------------------------------------------------------------------------
// LAGraph_Kind: the kind of a graph
//------------------------------------------------------------------------------

// Currently, only two types of graphs are supported: undirected graphs and
// directed graphs.  Both kinds of graphs can be weighted or unweighted (in the
// future).  Additional types of graphs will be added in the future.

typedef enum
{
    LAGRAPH_ADJACENCY_UNDIRECTED = 0, //  A is square, symmetric (both tril and triu present)
    LAGRAPH_ADJACENCY_DIRECTED = 1,   //  A is square, unsymmetric or might happen to symmetric

    // possible future kinds of graphs:
    // LAGRAPH_ADJACENCY_UNDIRECTED_UNWEIGHTED
    // LAGRAPH_ADJACENCY_DIRECTED_UNWEIGHTED
    // LAGRAPH_ADJACENCY_UNDIRECTED_TRIL
    // LAGRAPH_ADJACENCY_UNDIRECTED_TRIU
    // LAGRAPH_BIPARTITE
    // LAGRAPH_BIPARTITE_DIRECTED
    // LAGRAPH_BIPARTITE_UNDIRECTED
    // LAGRAPH_INCIDENCE_*
    // LAGRAPH_MULTIGRAPH_*
    // LAGRAPH_HYPERGRAPH
    // LAGRAPH_HYPERGRAPH_DIRECTED
    // ...

    LAGRAPH_KIND_UNKNOWN = LAGRAPH_UNKNOWN      // the graph kind is unknown
}
LAGraph_Kind ;

//------------------------------------------------------------------------------
// LAGraph_BooleanProperty: true, false, or unknown
//------------------------------------------------------------------------------

typedef enum
{
    LAGRAPH_FALSE = 0,
    LAGRAPH_TRUE = 1,
    LAGRAPH_BOOLEAN_UNKNOWN = LAGRAPH_UNKNOWN
}
LAGraph_BooleanProperty ;

//------------------------------------------------------------------------------
// LAGraph_Graph: the primary graph data structure of LAGraph
//------------------------------------------------------------------------------

// The LAGraph_Graph object contains a GrB_Matrix A as its primary component,
// as the adjacency matrix of the graph.  Typically, A(i,j) denotes the edge
// (i,j).  Unlike GrB_* objects in GraphBLAS, the LAGraph_Graph data structure
// is not opaque.  User applications have full access to its contents.

// An LAGraph_Graph G contains two kinds of components:

// (1) primary components of the graph, which fully define the graph:
//      A           the adjacency matrix of the graph
//      kind        the kind of graph (undirected, directed, bipartite, ...)

// (2) cached properties of the graph, which can be recreated any time:
//      AT          AT = A'
//      rowdegree   rowdegree(i) = # of entries in A(i,:)
//      coldegree   coldegree(j) = # of entries in A(:,j)
//      A_pattern_is_symmetric: true if the pattern of A is symmetric

struct LAGraph_Graph_struct
{

    //--------------------------------------------------------------------------
    // primary components of the graph
    //--------------------------------------------------------------------------

    GrB_Matrix   A;          // the adjacency matrix of the graph
    GrB_Type     A_type;     // the type of scalar stored in A
    LAGraph_Kind kind;       // the kind of graph

    // possible future components:
    // multigraph ..
    // GrB_Matrix *Amult ; // array of size nmatrices
    // int nmatrices ;
    // GrB_Vector VertexWeights ;

    //--------------------------------------------------------------------------
    // cached properties of the graph
    //--------------------------------------------------------------------------

    // All of these components may be deleted or set to 'unknown' at any time.
    // For example, if AT is NULL, then the transpose of A has not been
    // computed.  A scalar property of type LAGraph_BooleanProperty would be
    // set to LAGRAPH_UNKNOWN to denote that its value is unknown.

    // If present, the properties must be valid and accurate.  If the graph
    // changes, these properties can either be recomputed or deleted to denoted
    // the fact that they are unknown.  This choice is up to individual LAGraph
    // methods and utilities (TODO: define a default rule, and give user
    // control over this decision).

    // LAGraph methods can set non-scalar properties only if they are
    // constructing the graph.  They cannot modify them or create them if the
    // graph is declared as a read-only object in the parameter list of the
    // method.

    // TODO: discuss this: "However, scalar properties can be set even if the
    // graph is a read-only parameter, but only if they are accessed with
    // OpenMP atomic read/write operations."  OK?

    GrB_Matrix AT;          // AT = A', the transpose of A
    GrB_Type   AT_type;     // The type of scalar stored in AT

    GrB_Vector rowdegree;   // a GrB_INT64 vector of length m, if A is m-by-n.
           // where rowdegree(i) is the number of entries in A(i,:).  If
           // rowdegree is sparse and the entry rowdegree(i) is not present,
           // then it is assumed to be zero.
    GrB_Type   rowdegree_type;   // the type of scalar stored in rowdegree

    GrB_Vector coldegree ;  // a GrB_INT64 vector of length n, if A is m-by-n.
            // where coldegree(j) is the number of entries in A(:,j).  If
            // coldegree is sparse and the entry coldegree(j) is not present,
            // then it is assumed to be zero.  If A is known to have a
            // symmetric pattern, the convention is that the degree is held in
            // rowdegree, and coldegree is left as NULL.
    GrB_Type   coldegree_type;   // the type of scalar stored in coldegree

    LAGraph_BooleanProperty A_pattern_is_symmetric ;    // For an undirected
            // graph, this property will always be implicitly true and can be
            // ignored.  The matrix A for a directed weighted graph will
            // typically by unsymmetric, but might have a symmetric pattern.
            // In that case, this scalar property can be set to true.

    int64_t ndiag ; // # of entries on the diagonal of A, or -1 if unknown.
            // For the adjacency matrix of a directed or undirected graph,
            // this is the # of self-edges in the graph.
            // TODO: discuss this.

    // possible future cached properties:
    // GrB_Vector rowsum, colsum ;
    // rowsum (i) = sum (A (i,:)), regardless of kind
    // colsum (j) = sum (A (:,j)), regardless of kind
    // LAGraph_BooleanProperty connected ;   // true if G is a connected graph
} ;

typedef struct LAGraph_Graph_struct *LAGraph_Graph ;

//==============================================================================
// LAGraph utilities
//==============================================================================

// LAGraph_Init: start GraphBLAS and LAGraph
int LAGraph_Init (char *msg) ;      // return 0 if success, -1 if failure

// LAGraph_Xinit: start GraphBLAS and LAGraph, and set malloc/etc functions
int LAGraph_Xinit           // returns 0 if successful, -1 if failure
(
    // pointers to memory management functions
    void * (* user_malloc_function  ) (size_t),
    void * (* user_calloc_function  ) (size_t, size_t),
    void * (* user_realloc_function ) (void *, size_t),
    void   (* user_free_function    ) (void *),
    char *msg
) ;

// LAGraph_Finalize: finish LAGraph
int LAGraph_Finalize (char *msg) ;  // returns 0 if successful, -1 if failure

// LAGraph_MIN/MAX: suitable for integers, and non-NaN floating point
#define LAGraph_MIN(x,y) (((x) < (y)) ? (x) : (y))
#define LAGraph_MAX(x,y) (((x) > (y)) ? (x) : (y))

// LAGraph_New: create a new graph
int LAGraph_New         // returns 0 if successful, -1 if failure
(
    LAGraph_Graph *G,       // the graph to create, NULL if failure
    GrB_Matrix    *A,       // the adjacency matrix of the graph, may be NULL
    GrB_Type       A_type,  // the type of scalar stored in A
    LAGraph_Kind   kind,    // the kind of graph, may be LAGRAPH_UNKNOWN
    char *msg
) ;

// LAGraph_Delete: free a graph and all its contents
int LAGraph_Delete      // returns 0 if successful, -1 if failure
(
    LAGraph_Graph *G,   // the graph to delete; G set to NULL on output
    char *msg
) ;

// LAGraph_DeleteProperties: free any internal cached properties of a graph
int LAGraph_DeleteProperties    // returns 0 if successful, -1 if failure
(
    LAGraph_Graph G,        // G stays valid, only cached properties are freed
    char *msg
) ;

// LAGraph_Property_AT: construct G->AT for a graph
int LAGraph_Property_AT     // returns 0 if successful, -1 if failure
(
    LAGraph_Graph G,        // graph to compute G->AT for
    char *msg
) ;

// LAGraph_Property_ASymmetricPattern: determine G->A_pattern_is_symmetric
int LAGraph_Property_ASymmetricPattern  // 0 if successful, -1 if failure
(
    LAGraph_Graph G,        // graph to determine the symmetry of pattern of A
    char *msg
) ;

// LAGraph_Property_RowDegree: determine G->rowdegree
int LAGraph_Property_RowDegree  // 0 if successful, -1 if failure
(
    LAGraph_Graph G,        // graph to determine G->rowdegree
    char *msg
) ;

// LAGraph_Property_ColDegree: determine G->coldegree
int LAGraph_Property_ColDegree  // 0 if successful, -1 if failure
(
    LAGraph_Graph G,        // graph to determine G->coldegree
    char *msg
) ;

// LAGraph_Property_NDiag: determine G->ndiag
int LAGraph_Property_NDiag  // returns 0 if successful, -1 if failure
(
    LAGraph_Graph G,        // graph to compute G->ndiag for
    char *msg
) ;

//  LAGraph_DeleteDiag: remove all diagonal entries fromG->A
int LAGraph_DeleteDiag      // returns 0 if successful, < 0 if failure
(
    LAGraph_Graph G,        // diagonal entries removed, most properties cleared
    char *msg
) ;

// LAGraph_CheckGraph: determine if a graph is valid
int LAGraph_CheckGraph      // returns 0 if successful, -1 if failure
(
    LAGraph_Graph G,        // graph to check
    // TODO: int level,     // 0:quick, O(1), 1:a bit more, 2: still more, 3: exhaustive!
    char *msg
) ;

// LAGraph_GetNumThreads: determine # of OpenMP threads to use
int LAGraph_GetNumThreads   // returns 0 if successful, or -1 if failure
(
    int *nthreads,          // # of threads to use
    char *msg
) ;

// LAGraph_SetNumThreads: set # of OpenMP threads to use
int LAGraph_SetNumThreads   // returns 0 if successful, or -1 if failure
(
    int nthreads,           // # of threads to use
    char *msg
) ;

// LAGraph_Tic: start the timer
int LAGraph_Tic             // returns 0 if successful, -1 if failure
(
    double tic [2],         // tic [0]: seconds, tic [1]: nanoseconds
    char *msg
) ;

// LAGraph_Toc: return time since last call to LAGraph_Tic
int LAGraph_Toc             // returns 0 if successful, -1 if failure
(
    double *t,              // time since last call to LAGraph_Tic
    const double tic [2],   // tic from last call to LAGraph_Tic
    char *msg
) ;


/****************************************************************************
 *
 * LAGraph_MMRead:  read a matrix from a Matrix Market file.
 *
 * The file format used here is compatible with all variations of the Matrix
 * Market "coordinate" and "array" format (http://www.nist.gov/MatrixMarket).
 * The format is fully described in LAGraph/Doc/MatrixMarket.pdf, and
 * summarized here (with extensions for LAGraph).
 *
 * The first line of the file starts with %%MatrixMarket, with the following
 * format:
 *
 *      %%MatrixMarket matrix <fmt> <type> <storage>
 *
 *      <fmt> is one of: coordinate or array.  The former is a sparse matrix in
 *      triplet form.  The latter is a dense matrix in column-major form.
 *      Both formats are returned as a GrB_Matrix.
 *
 *      <type> is one of: real, complex, pattern, or integer.  The real,
 *      integer, and pattern formats are returned as GrB_FP64, GrB_INT64, and
 *      GrB_BOOL, respectively, but these types are modified the %GraphBLAS
 *      structured comment described below.  Complex matrices are currently not
 *      supported.
 *
 *      <storage> is one of: general, Hermitian, symmetric, or skew-symmetric.
 *      The Matrix Market format is case-insensitive, so "hermitian" and
 *      "Hermitian" are treated the same).
 *
 *      Not all combinations are permitted.  Only the following are meaningful:
 *
 *      (1) (coordinate or array) x (real, integer, or complex)
 *          x (general, symmetric, or skew-symmetric)
 *
 *      (2) (coordinate or array) x (complex) x (Hermitian)
 *
 *      (3) (coodinate) x (pattern) x (general or symmetric)
 *
 * The second line is an optional extension to the Matrix Market format:
 *
 *      %%GraphBLAS <entrytype>
 *
 *      <entrytype> is one of the 13 built-in types (GrB_BOOL, GrB_INT8,
 *      GrB_INT16, GrB_INT32, GrB_INT64, GrB_UINT8, GrB_UINT16, GrB_UINT32,
 *      GrB_UINT64, GrB_FP32, GrB_FP64, GxB_FC32, or GxB_FC64).
 *
 *      If this second line is included, it overrides the default GraphBLAS
 *      types for the Matrix Market <type> on line one of the file: real,
 *      pattern, and integer.  The Matrix Market complex <type> is not
 *      modified, and is always returned as GxB_FC64.
 *
 * Any other lines starting with "%" are treated as comments, and are ignored.
 * Comments may be interspersed throughout the file.  Blank lines are ignored.
 * The Matrix Market header is optional in this routine (it is not optional in
 * the Matrix Market format).  If not present, the <fmt> defaults to
 * coordinate, <type> defaults to real, and <storage> defaults to general.  The
 * remaining lines are space delimited, and free format (one or more spaces can
 * appear, and each field has arbitrary width).
 *
 * The Matrix Market file <fmt> can be coordinate or array:
 *
 *      coordinate:  for this format, the first non-comment line must appear,
 *          and it must contain three integers:
 *
 *              nrows ncols nvals
 *
 *          For example, a 5-by-12 matrix with 42 entries would have:
 *
 *              5 12 42
 *
 *          Each of the remaining lines defines one entry.  The order is
 *          arbitrary.  If the Matrix Market <type> is real or integer, each
 *          line contains three numbers: row index, column index, and value.
 *          For example, if A(3,4) is equal to 5.77, a line:
 *
 *              3 4 5.77
 *
 *          would appear in the file.  The indices in the Matrix Market are
 *          1-based, so this entry becomes A(2,3) in the GrB_Matrix returned to
 *          the caller.  If the <type> is pattern, then only the row and column
 *          index appears.  If <type> is complex, four values appear.  If
 *          A(8,4) has a real part of 6.2 and an imaginary part of -9.3, then
 *          the line is:
 *
 *              8 4 6.2 -9.3
 *
 *          and since the file is 1-based but a GraphBLAS matrix is always
 *          0-based, one is subtracted from the row and column indices in the
 *          file, so this entry becomes A(7,3).
 *
 *      array: for this format, the first non-comment line must appear, and
 *          it must contain just two integers:
 *
 *              nrows ncols
 *
 *          A 5-by-12 matrix would thus have the line
 *
 *              5 12
 *
 *          Each of the remaining lines defines one entry, in column major
 *          order.  If the <type> is real or integer, this is the value of the
 *          entry.  An entry if <type> of complex consists of two values, the
 *          real and imaginary part.  The <type> cannot be pattern in this
 *          case.
 *
 *      For both coordinate and array formats, real and complex values may use
 *      the terms INF, +INF, -INF, and NAN to represent floating-point infinity
 *      and NaN values.
 *
 * The <storage> token is general, symmetric, skew-symmetric, or Hermitian:
 *
 *      general: the matrix has no symmetry properties (or at least none
 *          that were exploited when the file was created).
 *
 *      symmetric:  A(i,j) == A(j,i).  Only entries on or below the diagonal
 *          appear in the file.  Each off-diagonal entry in the file creates
 *          two entries in the GrB_Matrix that is returned.
 *
 *      skew-symmetric:  A(i,j) == -A(i,j).  There are no entries on the
 *          diagonal.  Only entries below the diagonal appear in the file.
 *          Each off-diagonal entry in the file creates two entries in the
 *          GrB_Matrix that is returned.
 *
 *      Hermitian: square complex matrix with A(i,j) = conj (A(j,i)).
 *          All entries on the diagonal are real.  Each off-diagonal entry in
 *          the file creates two entries in the GrB_Matrix that is returned.
 *
 * According to the Matrix Market format, entries are always listed in
 * column-major order.  This rule is follwed by LAGraph_MMWrite.  However,
 * LAGraph_MMRead can read the entries in any order.
 *
 * @param[out]    A       handle of the matrix to create
 * @param[out]    A_type  type of the scalar stored in A
 * @param[in]     f       handle to an open file to read from
 * @param[in,out] msg     any error messages
 *
 * @retval 0 successful
 * @retval -1 if failure
 *
 */

// LAGraph_MMRead: read a matrix in MatrixMarket format
int LAGraph_MMRead
(
    GrB_Matrix *A,          // handle of matrix to create
    GrB_Type   *A_type,     // type of the scalar stored in A
    FILE *f,                // file to read from, already open
    char *msg
);

// LAGraph_MMWrite: write a matrix in MatrixMarket format with given type
int LAGraph_MMWrite_type
(
    GrB_Matrix A,       // matrix to write to the file
    GrB_Type type,      // type to write to the file
    FILE *f,            // file to write it to, must be already open
    FILE *fcomments,    // optional file with extra comments, may be NULL
    char *msg
) ;

// LAGraph_MMWrite: write a matrix in MatrixMarket format, auto select type
int LAGraph_MMWrite
(
    GrB_Matrix A,       // matrix to write to the file
    FILE *f,            // file to write it to, must be already open
    FILE *fcomments,    // optional file with extra comments, may be NULL
    char *msg
) ;

// LAGraph_Pattern: return the pattern of a matrix (spones(A) in MATLAB)
int LAGraph_Pattern     // return 0 if successful, -1 if failure
(
    GrB_Matrix *C,      // a boolean matrix with the pattern of A
    GrB_Matrix A,
    char *msg
) ;

// LAGraph_TypeName: return the name of a type
int LAGraph_TypeName        // returns 0 if successful, -1 if failure
(
    char **name,            // name of the type
    GrB_Type type,          // GraphBLAS type
    char *msg
) ;

// LAGraph_KindName: return the name of a kind
int LAGraph_KindName        // returns 0 if successful, -1 if failure
(
    char **name,            // name of the kind
    LAGraph_Kind kind,      // graph kind
    char *msg
) ;

// LAGraph_SortByDegree: sort a graph by its row or column degree
int LAGraph_SortByDegree    // returns 0 if successful, -1 if failure
(
    // output
    int64_t **P_handle,     // P is returned as a permutation vector of size n
    // input
    LAGraph_Graph G,        // graph of n nodes
    bool byrow,             // if true, sort G->rowdegree, else G->coldegree
    bool ascending,         // sort in ascending or descending order
    char *msg
) ;

// LAGraph_SampleDegree: sample the degree median and mean
int LAGraph_SampleDegree        // returns 0 if successful, -1 if failure
(
    double *sample_mean,        // sampled mean degree
    double *sample_median,      // sampled median degree
    // input
    LAGraph_Graph G,        // graph of n nodes
    bool byrow,             // if true, sample G->rowdegree, else G->coldegree
    int64_t nsamples,       // number of samples
    uint64_t seed,          // random number seed
    char *msg
) ;

// LAGraph_DisplayGraph: print the contents of a graph
int LAGraph_DisplayGraph    // returns 0 if successful, -1 if failure
(
    LAGraph_Graph G,        // graph to display
    int pr,                 // 0: nothing, 1: terse, 2: summary, 3: all,
                            // 4: same as 2 but with %0.15g for doubles
                            // 5: same as 3 but with %0.15g for doubles
    FILE *f,                // file to write to, must already be open
    char *msg
) ;

// LAGraph_IsEqual: compare for exact equality, auto selection of type
int LAGraph_IsEqual         // returns 0 if successful, -1 if failure
(
    bool *result,           // true if A == B, false if A != B or error
    GrB_Matrix A,
    GrB_Matrix B,
    char *msg
) ;

// LAGraph_IsEqual_type: compare two matrices for exact equality
int LAGraph_IsEqual_type    // returns 0 if successful, < 0 if failure
(
    bool *result,           // true if A == B, false if A != B or error
    GrB_Matrix A,
    GrB_Matrix B,
    GrB_Type type,          // use GrB_EQ_type operator to compare A and B
    char *msg
) ;

// LAGraph_Matrix_print: pretty-print a matrix, determining type automatically
int LAGraph_Matrix_print
(
    GrB_Matrix A,       // matrix to pretty-print to the file
    int pr,             // print level: -1 nothing, 0: one line, 1: terse,
                        //      2: summary, 3: all,
                        //      4: as 2 but with %0.15g for float/double
                        //      5: as 3 but with %0.15g for float/double
    FILE *f,            // file to write it to, must be already open; use
                        // stdout or stderr to print to those locations.
    char *msg
) ;

// LAGraph_Matrix_print_type: pretty-print a matrix with a given type
int LAGraph_Matrix_print_type
(
    GrB_Matrix A,       // matrix to pretty-print to the file
    GrB_Type type,      // type to print
    int pr,             // print level: -1 nothing, 0: one line, 1: terse,
                        //      2: summary, 3: all,
                        //      4: as 2 but with %0.15g for float/double
                        //      5: as 3 but with %0.15g for float/double
    FILE *f,            // file to write it to, must be already open; use
                        // stdout or stderr to print to those locations.
    char *msg
) ;

// LAGraph_Vector_print: pretty-print a matrix, determining type automatically
int LAGraph_Vector_print
(
    GrB_Vector v,       // vector to pretty-print to the file
    int pr,             // print level: -1 nothing, 0: one line, 1: terse,
                        //      2: summary, 3: all,
                        //      4: as 2 but with %0.15g for float/double
                        //      5: as 3 but with %0.15g for float/double
    FILE *f,            // file to write it to, must be already open; use
                        // stdout or stderr to print to those locations.
    char *msg
) ;

// LAGraph_Vector_print_type: pretty-print a matrix with a given type
int LAGraph_Vector_print_type
(
    GrB_Vector v,       // vector to pretty-print to the file
    GrB_Type type,      // type to print
    int pr,             // print level: -1 nothing, 0: one line, 1: terse,
                        //      2: summary, 3: all,
                        //      4: as 2 but with %0.15g for float/double
                        //      5: as 3 but with %0.15g for float/double
    FILE *f,            // file to write it to, must be already open; use
                        // stdout or stderr to print to those locations.
    char *msg
) ;

int LAGraph_Matrix_wait     // wait on a matrix
(
    GrB_Matrix A,
    char *msg
) ;

int LAGraph_Vector_wait     // wait on a vector
(
    GrB_Vector v,
    char *msg
) ;

//==============================================================================
// LAGraph Basic algorithms
//==============================================================================

// Basic algorithm are meant to be easy to use.  They may encompass many
// underlying Advanced algorithms, each with various parameters that may be
// controlled.  For the Basic API, these parameters are determined
// automatically.  Graph properties may be determined, and as a result, the
// graph G is both an input and an output of these methods, since they may be
// modified.

/****************************************************************************
 *
 * Perform breadth-first traversal, computing parent vertex ID's
 * and/or level encountered.
 *
 * @param[out]    level      If non-NULL on input, on successful return, it
 *                           contains the levels of each vertex reached. The
 *                           src vertex is assigned level 0. If a vertex i is not
 *                           reached, parent(i) is not present.
 *                           The level vector is not computed if NULL.
 * @param[out]    parent     If non-NULL on input, on successful return, it
 *                           contains parent vertex IDs for each vertex reached.
 *                           The src vertex will have itself as its parent. If a
 *                           vertex i is not reached, parent(i) is not present.
 *                           The parent vector is not computed if NULL.
 * @param[in]     G          The graph, directed or undirected.
 * @param[in]     src        The index of the src vertex (0-based)
 * @param[in]     pushpull   if true, use push/pull; otherwise, use pushonly.
 *                           Push/pull requires G->AT, G->rowdegree,
 *                           and library support.
 *                           TODO: consider removing this option or reverse logic
 * @param[out]    msg        Error message if a failure code is returned.
 *
 * @todo pick return values that do not conflict with GraphBLAS errors.
 *
 * @retval         0         successful
 * @retval      -102         Graph is invalid (LAGraph_CheckGraph failed)
 */
int LAGraph_BreadthFirstSearch
(
    GrB_Vector    *level,
    GrB_Vector    *parent,
    LAGraph_Graph  G,
    GrB_Index      src,
    bool           pushpull,
    char          *msg
);

//****************************************************************************
// the following is a draft:
typedef enum
{
    LAGRAPH_CENTRALITY_BETWEENNESS = 0,     // node or edge centrality
    LAGRAPH_CENTRALITY_PAGERANKGAP = 1,     // GAP-style PageRank
    LAGRAPH_CENTRALITY_PAGERANK = 2,        // PageRank (handle dangling nodes)
    // ...
}
LAGraph_Centrality_Kind ;

int LAGraph_VertexCentrality    // returns -1 on failure, 0 if successful
(
    // output:
    GrB_Vector *centrality,     // centrality(i): centrality metric of node i
    // inputs:
    LAGraph_Graph G,            // input graph
    LAGraph_Centrality_Kind kind,    // kind of centrality to compute
//  int accuracy,               // TODO?: 0:quick, 1:better, ... max:exact
    char *msg
) ;

/****************************************************************************
 *
 * Count the triangles in a graph.
 *
 * @param[out]    ntriangles On successful return, contains the number of tris.
 * @param[in,out] G          The graph, symmetric, no self loops.
 * @param[out]    msg        Error message if a failure code is returned.
 *
 * @todo pick return values that do not conflict with GraphBLAS errors.
 *
 * @retval         0         successful
 * @retval      -102         Graph is invalid (LAGraph_CheckGraph failed)
 * @retval      -103         ntriangles is NULL
 * @retval      -104         G->ndiag (self loops) is nonzero
 * @retval      -105         graph is not symmetric
 * @retval      -106         G->rowdegree was not precalculated (for modes 3-6)
 */
int LAGraph_TriangleCount   // returns 0 if successful, < 0 if failure
(
    uint64_t      *ntriangles,   // # of triangles
    // input:
    LAGraph_Graph  G,
    char          *msg
) ;

// TODO: this is a Basic method, since G is input/output.
int LAGraph_ConnectedComponents
(
    // output
    GrB_Vector *component,  // component(i)=k if node i is in the kth component
    // inputs
    LAGraph_Graph G,        // input graph, G->A can change (ptr, not contents)
    char *msg
) ;

// TODO: add AIsAllPositive or related as a G->property.
// TODO: Should a Basic method pick delta automatically?
int LAGraph_SingleSourceShortestPath    // returns 0 if successful, -1 if fail
(
    // output:
    GrB_Vector *path_length,    // path_length (i) is the length of the shortest
                                // path from the source vertex to vertex i
    // inputs:
    LAGraph_Graph G,
    GrB_Index source,           // source vertex
    int32_t delta,              // delta value for delta stepping
                                // TODO: use GxB_Scalar for delta?
    // TODO: make this an enum, and add to LAGraph_Graph properties, and then
    // remove it from the inputs to this function
    //      case 0: A can have negative, zero, or positive entries
    //      case 1: A can have zero or positive entries
    //      case 2: A only has positive entries (see FIXME below)
    // TODO: add AIsAllPositive to G->A_is_something...
    bool AIsAllPositive,       // A boolean indicating whether the entries of
                               // matrix A are all positive
    char *msg
) ;

//==============================================================================
// LAGraph Advanced algorithms
//==============================================================================

// The Advanced methods require the caller to select the algorithm and choose
// any parameter settings.  G is not modified, and so it is an input-only
// parameter to these methods.  If an Advanced algorithm requires a graph
// property to be computed, it must be computed prior to calling the Advanced
// method.

int LAGraph_VertexCentrality_Betweenness    // vertex betweenness-centrality
(
    // output:
    GrB_Vector *centrality,     // centrality(i): betweeness centrality of i
    // inputs:
    LAGraph_Graph G,            // input graph
    const GrB_Index *sources,   // source vertices to compute shortest paths
    int32_t ns,                 // number of source vertices
    char *msg
) ;

int LAGraph_VertexCentrality_PageRankGAP // returns -1 on failure, 0 on success
(
    // outputs:
    GrB_Vector *centrality, // centrality(i): GAP-style pagerank of node i
    // inputs:
    LAGraph_Graph G,        // input graph
    float damping,          // damping factor (typically 0.85)
    float tol,              // stopping tolerance (typically 1e-4) ;
    int itermax,            // maximum number of iterations (typically 100)
    int *iters,             // output: number of iterations taken
    char *msg
) ;

/****************************************************************************
 *
 * Count the triangles in a graph. Advanced API
 *
 * @param[out]    ntriangles On successful return, contains the number of tris.
 * @param[in]     G          The graph, symmetric, no self loops, and for some methods
 *                           (3-6), must have the row degree property calculated
 * @param[in]     method     specifies which algorithm to use (todo: use enum)
 *                             0: DISABLED
 *                             1:  Burkhardt:  ntri = sum (sum ((A^2) .* A)) / 6
 *                             2:  Cohen:      ntri = sum (sum ((L * U) .* A)) / 2
 *                             3:  Sandia:     ntri = sum (sum ((L * L) .* L))
 *                             4:  Sandia2:    ntri = sum (sum ((U * U) .* U))
 *                             5:  SandiaDot:  ntri = sum (sum ((L * U') .* L)).
 *                             6:  SandiaDot2: ntri = sum (sum ((U * L') .* U)).
 * @param[in,out] presort    controls the presort of the graph (TODO: enum). If set
 *                           to 2 on input, presort will be set to sort type used
 *                           on output:
 *                             0: no sort
 *                             1: sort by degree, ascending order
 *                            -1: sort by degree, descending order
 *                             2: auto selection:
 *                                Method   Presort return value
 *                                1        0
 *                                2        0
 *                                3        1
 *                                4       -1
 *                                5        1
 *                                6       -1
 * @param[out]    msg        Error message if a failure code is returned.
 *
 * @retval         0         successful
 * @retval      -101         invalid method value
 * @retval      -102         Graph is invalid (LAGraph_CheckGraph failed)
 * @retval      -103         ntriangles is NULL
 * @retval      -104         G->ndiag (self loops) is nonzero
 * @retval      -105         graph is not "known" to be symmetric
 * @retval      -106         G->rowdegree was not precalculated (for modes 3-6)
 */
int LAGraph_TriangleCount_Methods
(
    uint64_t       *ntriangles,
    LAGraph_Graph   G,
    int             method,
    int            *presort,
    char           *msg
) ;

#endif
