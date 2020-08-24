//------------------------------------------------------------------------------
// LAGraph_bfs_pushpull:  push-pull breadth-first search
//------------------------------------------------------------------------------

/*
    LAGraph:  graph algorithms based on GraphBLAS

    Copyright 2020 LAGraph Contributors.

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
#include "LAGraph_bfs_both.h"

//------------------------------------------------------------------------------

// LAGraph_bfs_pushpull: direction-optimized push/pull breadth first search,
// contributed by Tim Davis, Texas A&M.

// LAGraph_bfs_pushpull computes the BFS of a graph from a single given
// source node.  The result is a vector v where v(i)=k if node i was placed
// at level k in the BFS.

// Usage:

// info = LAGraph_bfs_pushpull (&v, &pi, A, AT, source, max_level, vsparse) ;

//      GrB_Vector *v:  a vector containing the result, created on output.
//          v(i) = k is the BFS level of node i in the graph, where a source
//          node has v(source)=1.  v(i) is implicitly zero if it is unreachable
//          from the source node.  That is, GrB_Vector_nvals (&nreach,v) is the
//          size of the reachable set of the source node, for a single-source
//          BFS.  v may be returned as sparse, or full.  If full, v(i)=0
//          indicates that node i was not reached.  If sparse, the pattern of v
//          indicates the set of nodes reached.

//      GrB_Vector *pi:  a vector containing the BFS tree, in 1-based indexing.
//          pi(source) = source+1 for source node.  pi(i) = p+1 if p is the
//          parent of i.  If pi is sparse, and pi(i) is not present, then node
//          i has not been reached.  Otherwise, if pi is full, then pi(i)=0
//          indicates that node i was not reached.

//      GrB_Matrix A: a square matrix of any type.  The values of A are not
//          accessed.  The presence of the entry A(i,j) indicates the edge
//          (i,j).  That is, an explicit entry A(i,j)=0 is treated as an edge.

//      GrB_Matrix AT: an optional matrix of any type.  If NULL, the algorithm
//          is a conventional push-only BFS.  If not NULL, AT must be the
//          transpose of A, and a push-pull algorithm is used (NOTE: this
//          assumes GraphBLAS stores its matrix in CSR form; see discussion
//          below).  Results are undefined if AT is not NULL but not identical
//          to the transpose of A.

//      int64_t source: the source node for the BFS.

//      int64_t max_level:  An optional limit on the levels searched for the
//          single-source BFS.  If zero, then no limit is enforced.  If > 0,
//          then only nodes with v(i) <= max_level will be visited.  That is:
//          1: just the source node, 2: the source and its neighbors, 3: the
//          source node, its neighbors, and their neighbors, etc.

//      bool vsparse:  if the result v may remain very sparse, then set this
//          parameter to true.  If v might have many entries, set it false.  If
//          you are unsure, then set it to true.  This parameter speeds up
//          the handling of v.  If you guess wrong, there is a slight
//          performance penalty.  The results are not affected by this
//          parameter, just the performance.  This parameter is used only for
//          the single-source BFS.

// single-source BFS:
//      Given a graph A, a source node, find all nodes reachable from the
//      source node.  v(source)=1, v(i)=2 if edge (source,i) appears in the
//      graph, and so on.  If node i is not reachable from source, then
//      implicitly v(i)=0.  v is returned as a sparse vector, and v(i) is not
//      an entry in this vector.

// This algorithm can use the push-pull strategy, which requires both A and
// AT=A' to be passed in.  If the graph is known to be symmetric, then the same
// matrix A can be passed in for both arguments.  Results are undefined if AT
// is not the transpose of A.

// If only A or AT is passed in, then only single strategy will be used: push
// or pull, but not both.  In general, push-only performs well.  A pull-only
// strategy is possible but it is exceedingly slow.  Assuming A and AT are both
// in CSR format, then (let s = source node):

//      LAGraph_bfs_pushpull (..., A, AT,    s, ...) ;  // push-pull (fastest)
//      LAGraph_bfs_pushpull (..., A, NULL,  s, ...) ;  // push-only (good)
//      LAGraph_bfs_pushpull (..., NULL, AT, s, ...) ;  // pull-only (slow!)

// If A and AT are both in CSC format, then:

//      LAGraph_bfs_pushpull (..., A, AT,    s, ...) ;  // push-pull (fastest)
//      LAGraph_bfs_pushpull (..., NULL, AT, s, ...) ;  // push-only (good)
//      LAGraph_bfs_pushpull (..., A, NULL,  s, ...) ;  // pull-only (slow!)

// Since the pull-only method is exceedingly slow, SuiteSparse:GraphBLAS
// detects this case and refuses to do it.

// The basic step of this algorithm computes A'*q where q is the 'queue' of
// nodes in the current level.  This can be done with GrB_vxm(q,A) = (q'*A)' =
// A'*q, or by GrB_mxv(AT,q) = AT*q = A'*q.  Both steps compute the same thing,
// just in a different way.  In GraphBLAS, unlike MATLAB, a GrB_Vector is
// simultaneously a row and column vector, so q and q' are interchangeable.

// To implement an efficient BFS using GraphBLAS, an assumption must be made in
// LAGraph about how the matrix is stored, whether by row or by column (or
// perhaps some other opaque data structure).  The storage format has a huge
// impact on the relative performance of vxm(q,A) and mxv(AT,q).

// Storing A by row, if A(i,j) is the edge (i,j), means that A(i,:) is easily
// accessible.  In terms of the graph A, this means that the out-adjacency
// list of node i can be traversed in time O(out-degree of node i).
// If AT is stored by row, then AT(i,:) is the in-adjacency list of node i,
// and traversing row i of AT can be done in O(in-degree of node i) time.
// The CSR (Compressed Sparse Row) format is the default for
// SuiteSparse:GraphBLAS, but no assumption can be made about any particular
// GraphBLAS library implementation.

// If A and AT are both stored by column instead, then A(i,:) is not easy to
// access.  Instead, A(:,i) is the easily-accessible in-adjacency of node i,
// and AT(:,i) is the out-adjancency.

// A push step requires the out-adjacencies of each node, where as
// a pull step requires the in-adjacencies of each node.

//      vxm(q,A)  = A'*q, with A  stored by row:  a push step
//      mxv(AT,q) = A'*q, with AT stored by row:  a pull step

//      vxm(q,A)  = A'*q, with A  stored by col:  a pull step
//      mxv(AT,q) = A'*q, with AT stored by col:  a push step

// The GraphBLAS data structure is opaque.  An implementation may decide to
// store the matrix A in both formats, internally, so that it easily traverse
// both in- and out-adjacencies of each node (equivalently, A(i,:) and A(:,i)
// can both be easily traversed).  This would make a push-pull BFS easy to
// implement using just the opaque GrB_Matrix A, but it doubles the storage.
// Deciding which format to use automatically is not a simple task,
// particularly since the decision must work well throughout GraphBLAS, not
// just for the BFS.

// MATLAB stores its sparse matrices in CSC format (Compressed Sparse Column).
// As a result, the MATLAB expression x=AT*q is a push step, computed using a
// saxpy-based algorithm internally, and x=A'*q is a pull step, computed using
// a dot product.

// SuiteSparse:GraphBLAS can store a matrix in either format, but this requires
// an extension to the GraphBLAS C API (GxB_set (A, GxB_FORMAT, f)). where
// f = GxB_BY_ROW (that is, CSR) or GxB_BY_COL (that is, CSC).  The library
// could be augmented in the future with f = Gxb_BY_BOTH.  It currently does
// not select the format automatically.  As a result, if GxB_set is not used,
// all its GrB_Matrix objects are stored by row (CSR).

// SuiteSparse:GraphBLAS allows the user to query (via GxB_get) an set (via
// GxB_set) the format, whether by row or by column.  The hypersparsity of
// A is selected automatically, with optional hints from the user application,
// but a selection between hypersparsity vs standard CSR and CSC has no effect
// on the push vs pull decision made here.

// The push/pull and saxpy/dot connection can be described as follows.
// Assume for these first two examples that MATLAB stores its matrices in CSR
// format, where accessing A(i,:) is fast.

// If A is stored by row, then x = vxm(q,A) = q'*A can be written in MATLAB
// notation as:

/*
    function x = vxm (q,A)
    % a push step: compute x = q'*A where q is a column vector
    x = sparse (1,n)
    for i = 1:n
        % a saxpy operation, using the ith row of A and the scalar q(i)
        x = x + q (i) * A (i,:)
    end
*/

// If AT is stored by row, then x = mvx(AT,q) = AT*q = A'*q becomes
// a dot product:

/*
    function x = mxv (AT,q)
    % a pull step: compute x = AT*q where q is a column vector
    for i = 1:n
        % a dot-product of the ith row of AT and the column vector q
        x (i) = AT (i,:) * q
    end
*/

// The above snippets describe how SuiteSparse:GraphBLAS computes vxm(q,A) and
// mxv(AT,q) by default, where A and AT are stored by row by default.  However,
// they would be very slow in MATLAB, since it stores its sparse matrices in
// CSC format.  In that case, if A is stored by column and thus accessing
// A(:,j) is efficient, then x = vxm(q,A) =  q'*A becomes the dot product
// instead.  These two snippets assume the matrices are both in CSR for, and
// thus make more efficient use of MATLAB:

/*
    function x = vxm (q,A)
    % a pull step: compute x = q'*A where q is a column vector
    for j = 1:n
        % a dot product of the row vector q' and the jth column of A
        x (j) = q' * A (:,j)
    end
*/

// If AT is stored by column, then x = mvx(AT,q) is

/*
    function x = mxv (AT,q)
    % a push step: compute x = AT*q where q is a column vector
    for j = 1:n
        % a saxpy operation, using the jth column of AT and the scalar q(i)
        x = x + AT (:,j) * q
    end
*/

// In MATLAB, if q is a sparse column vector and A is a sparse matrix, then
// x=A*q does in fact use a saxpy-based method, internally, and x=A'*q uses a
// dot product.  You can view the code used internally in MATLAB for its sparse
// matrix multiplication in the SuiteSparse/MATLAB_Tools/SSMULT and SFMULT
// packages, at http://suitesparse.com.

// This raises an interesting puzzle for LAGraph, which is intended on being a
// graph library that can be run on any implementation of GraphBLAS.  There are
// no mechanisms in the GraphBLAS C API for LAGraph (or other external packages
// or user applications) to provide hints to GraphBLAS.  Likely, there are no
// query mechanisms where LAGraph can ask GraphBLAS how its matrices might be
// stored (LAGraphs asks, "Is A(i,:) fast?  Or A(:,j)?  Or both?"; the answer
// from GraphBLAS is silence).  The GraphBLAS data structure is opaque, and it
// does not answer this query.

// There are two solutions to this puzzle.  The most elegant one is for
// GraphBLAS to handle all this internally, and change formats as needed.  It
// could choose to store A in both CSR and CSC format, or use an entirely
// different data structure, and it would make the decision between the push or
// pull, at each step of the BFS.  This is not a simple task since the API is
// complex.  Furthermore, the selection of the data structure for A has
// implications on all other GraphBLAS operations (submatrix assignment and
// extraction, for example).

// However, if A were to be stored in both CSR and CSC format, inside the
// opaque GraphBLAS GrB_Matrix data structure, then LAGraph_bfs_simple would
// become a push-pull BFS.

// The second solution is to allow the user application or library such as
// LAGraph to provide hints and allow it to query the GraphBLAS library.
// There are no such features in the GraphBLAS C API.

// SuiteSparse:GraphBLAS takes the second approach:  It adds two functions that
// are extensions to the API:  GxB_set changes the format (CSR or CSC), and
// GxB_get can query the format.  Even this this simplication,
// SuiteSparse:GraphBLAS uses 24 different algorithmic variants inside GrB_mxm
// (per semiring), and selects between them automatically.  By default, all of
// its matrices are stored in CSR format (either sparse or hypersparse,
// selected automatically).  So if no GxB_* extensions are used, all matrices
// are in CSR format.

// If a GraphBLAS library other than SuiteSparse:GraphBLAS is in use, this
// particular function assumes that its input matrices are in CSR format, or at
// least A(i,:) and AT(i,:) can be easily accessed.  With this assumption, it
// is the responsibilty of this function to select between using a push or a
// pull, for each step in the BFS.

// The following analysis assumes CSR format, and it assumes that dot-product
// (a pull step) can terminate early via a short-circuit rule with the OR
// monoid, as soon as it encounters a TRUE value.  This cuts the time for the
// dot-product.  Not all GraphBLAS libraries may use this, but SuiteSparse:
// GraphBLAS does (in version 2.3.0 and later).  Early termination cannot be
// done for the saxpy (push step) method.

// The work done by the push method (saxpy) is very predictable.  BFS uses a
// complemented mask.  There is no simple way to exploit a complemented mask,
// and saxpy has no early termination rule.  If the set of nodes in the current
// level is q, the work is nnz(A(q,:)).  If d = nnz(A)/n is the average degree,
// this becomes d*nq where nq = length (q):

//      pushwork = d*nq

// The work done by the pull (dot product) method is less predictable.  It can
// exploit the complemented mask, and so it only computes (n-nvisited) dot
// products, if nvisited is the # of nodes visited so far (in all levels).
// With no early-termination, the dot product will take d * log2 (nq) time,
// assuming that q is large and a binary search is used internally.  That is,
// the dot product will scan through the d entries in A(i,:), and do a binary
// search for each entry in q.  To account for the higher constant of a binary
// search, log2(nq) is replaced with (3*(1+log2(nq))).  With early termination,
// d is too high.  If the nodes are randomly marked, the probability of each
// node being marked is nvisited/n.  The expected number of trials until
// success, for a sequence of events with probabilty p, is 1/p.  Thus, the
// expected number of iterations in a dot product before an early termination
// is 1/p = (n/nvisited+1), where +1 is added to avoid a divide by zero.
// However, it cannot exceed d.  Thus, the total work for the dot product
// (pull) method can be estimated as:

//      per_dot = min (d, n / (nvisited+1))
//      pullwork = (n-nvisited) * per_dot * (3 * (1 + log2 ((double) nq)))

// The above expressions are valid for SuiteSparse:GraphBLAS v2.3.0 and later,
// and may be reasonable for other GraphBLAS implementations.  Push or pull
// is selected as the one with the least work.

// TODO: change the formula for v3.2.0

// The push/pull decision requires that both A and AT be passed in, but this
// function can use just one or the other.  If only A is passed in and AT is
// NULL, then only vxm(q,A) will be used (a push step if A is CSR, or a pull
// step if A is CSC).  If only AT is passed in and A is NULL, then only
// mxv(AT,q) will be used (a pull step if AT is CSR, or a push step if AT is
// CSC).

// In general, while a push-pull strategy is the fastest, a push-only BFS will
// give good peformance.  In particular, the time to compute AT=A' plus the
// time for the push-pull BFS is typically higher than just a push-only BFS.
// This why this function does not compute AT=A'.  To take advantage of the
// push-pull method, both A and AT must already be available, with the cost to
// construct them amortized across other computations such as this one.

// A pull-only strategy will be *exceeding* slow.

// The input matrix A must be square.  It can be non-binary, but best
// performance will be obtained if it is GrB_BOOL.  It can have explicit
// entries equal to zero.  These are safely ignored, and are treated as
// non-edges.

// SuiteSparse:GraphBLAS can detect the CSR vs CSC format of its inputs.
// In this case, if both matrices are provided, they must be in the same
// format (both GxB_BY_ROW or both GxB_BY_COL).  If the matrices are in CSC
// format, vxm(q,A) is the pull step and mxv(AT,q) is the push step.
// If only A or AT are provided, and the result is a pull-only algorithm,
// an error is returned.

// References:

// Carl Yang, Aydin Buluc, and John D. Owens. 2018. Implementing Push-Pull
// Efficiently in GraphBLAS. In Proceedings of the 47th International
// Conference on Parallel Processing (ICPP 2018). ACM, New York, NY, USA,
// Article 89, 11 pages. DOI: https://doi.org/10.1145/3225058.3225122

// Scott Beamer, Krste Asanovic and David A. Patterson,
// The GAP Benchmark Suite, http://arxiv.org/abs/1508.03619, 2015.
// http://gap.cs.berkeley.edu/

#define LAGRAPH_FREE_ALL    \
{                           \
    GrB_free (&v) ;         \
    GrB_free (&t) ;         \
    GrB_free (&q) ;         \
    GrB_free (&pi) ;        \
}

GrB_Info LAGraph_bfs_both       // push-pull BFS, or push-only if AT = NULL
(
    GrB_Vector *v_output,   // v(i) is the BFS level of node i in the graph
    GrB_Vector *pi_output,  // pi(i) = p+1 if p is the parent of node i.
                            // if NULL, the parent is not computed.
    GrB_Matrix A,           // input graph, treated as if boolean in semiring
    GrB_Matrix AT,          // transpose of A (optional; push-only if NULL)
    int64_t source,         // starting node of the BFS
    int64_t max_level,      // optional limit of # levels to search
    bool vsparse            // if true, v is expected to be very sparse
    , FILE * logfile
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GrB_Info info ;
    GrB_Vector q = NULL ;           // nodes visited at each level
    GrB_Vector v = NULL ;           // result vector
    GrB_Vector t = NULL ;           // temporary vector
    GrB_Vector pi = NULL ;          // parent vector

    if (v_output == NULL || (A == NULL && AT == NULL))
    {
        // required output argument is missing
        LAGRAPH_ERROR ("required arguments are NULL", GrB_NULL_POINTER) ;
    }

    (*v_output) = NULL ;
    bool compute_tree = (pi_output != NULL) ;

    #if defined ( GxB_SUITESPARSE_GRAPHBLAS ) \
        && ( GxB_IMPLEMENTATION >= GxB_VERSION (3,2,0) )
    GrB_Descriptor desc_s  = GrB_DESC_S ;
    GrB_Descriptor desc_sc = GrB_DESC_SC ;
    GrB_Descriptor desc_rc = GrB_DESC_RC ;
    GrB_Descriptor desc_r  = GrB_DESC_R ;
    #else
    GrB_Descriptor desc_s  = NULL ;
    GrB_Descriptor desc_sc = LAGraph_desc_ooco ;
    GrB_Descriptor desc_rc = LAGraph_desc_oocr ;
    GrB_Descriptor desc_r  = LAGraph_desc_ooor ;
    #endif

    bool use_vxm_with_A ;
    GrB_Index nrows, ncols, nvalA, ignore, nvals ;
    if (A == NULL)
    {
        // only AT is provided
        LAGr_Matrix_ncols (&nrows, AT) ;
        LAGr_Matrix_nrows (&ncols, AT) ;
        LAGr_Matrix_nvals (&nvalA, AT) ;
        use_vxm_with_A = false ;
    }
    else
    {
        // A is provided.  AT may or may not be provided
        LAGr_Matrix_nrows (&nrows, A) ;
        LAGr_Matrix_ncols (&ncols, A) ;
        LAGr_Matrix_nvals (&nvalA, A) ;
        use_vxm_with_A = true ;
    }

    // push/pull requires both A and AT
    bool push_pull = (A != NULL && AT != NULL) ;

    if (nrows != ncols)
    {
        // A must be square
        LAGRAPH_ERROR ("A must be square", GrB_NULL_POINTER) ;
    }

    //--------------------------------------------------------------------------
    // check the format of A and AT
    //--------------------------------------------------------------------------

    bool csr = true ;

    // csr is true if A and AT are known (or assumed) to be in CSR format; if
    // false, they are known to be in CSC format.

    // This can be tested in SuiteSparse:GraphBLAS.  Other libraries can use
    // this section for their own library-specific tests, if they have them.

    // LAGraph_bfs_pushpull will work just fine if nothing is changed or if the
    // following is disabled (even SuiteSparse:GraphBLAS).  The push/pull
    // behaviour will be unpredicatble, however, unless the library default
    // format is CSR.

    #ifdef GxB_SUITESPARSE_GRAPHBLAS

        // The CSR vs CSC status can be tested in SuiteSparse:GraphBLAS.
        // However, even with SuiteSparse:GraphBLAS, this step is optional.
        GxB_Format_Value A_format = -1, AT_format = -1 ;
        bool A_csr = true, AT_csr = true ;
        if (A != NULL)
        {
            // A_csr is true if accessing A(i,:) is fast
            LAGr_get (A , GxB_FORMAT, &A_format) ;
            A_csr = (A_format == GxB_BY_ROW) ;
        }
        if (AT != NULL)
        {
            // AT_csr is true if accessing AT(i,:) is fast
            LAGr_get (AT, GxB_FORMAT, &AT_format) ;
            AT_csr = (AT_format == GxB_BY_ROW) ;
        }
        // Assume CSR if A(i,:) and AT(i,:) are both fast.  If csr is false,
        // then the algorithm below will reverse the use of vxm and mxv.
        csr = A_csr && AT_csr ;
        if (push_pull)
        {
            // both A and AT are provided.  Require they have the same format.
            // Either both A(i,:) and AT(i,:) are efficient to accesss, or both
            // A(:,j) and AT(:,j) are efficient to access.
            if (A_csr != AT_csr)
            {
                LAGRAPH_ERROR ("A and AT must in the same format:\n"
                    "both GxB_BY_ROW, or both GxB_BY_COL",
                    GrB_INVALID_VALUE) ;
            }
        }
        else
        {
            // only A or AT are provided.  Refuse to do the pull-only version.
            if (A != NULL && A_format == GxB_BY_COL)
            {
                // this would result in a pull-only BFS ... exceedingly slow
                LAGRAPH_ERROR (
                    "SuiteSparse: AT not provided, so A must be GxB_BY_ROW\n"
                    "(or provide both A and AT, both in the same format,\n"
                    "either both GxB_BY_COL or both GxB_BY_ROW)",
                    GrB_INVALID_VALUE) ;
            }
            if (AT != NULL && AT_format == GxB_BY_ROW)
            {
                // this would result in a pull-only BFS ... exceedingly slow
                LAGRAPH_ERROR (
                    "SuiteSparse: A not provided, so AT must be GxB_BY_COL\n"
                    "(or provide both A and AT, both in the same format,\n"
                    "either both GxB_BY_COL or both GxB_BY_ROW)",
                    GrB_INVALID_VALUE) ;
            }
        }

    #endif

    //--------------------------------------------------------------------------
    // initializations
    //--------------------------------------------------------------------------

    GrB_Index n = nrows ;

    int nthreads = LAGraph_get_nthreads ( ) ;
    nthreads = LAGRAPH_MIN (n / 4096, nthreads) ;
    nthreads = LAGRAPH_MAX (nthreads, 1) ;

    // just traverse from the source node
    max_level = (max_level <= 0) ? n : LAGRAPH_MIN (n, max_level) ;

    // create an empty vector v
    GrB_Type int_type = (n > INT32_MAX) ? GrB_INT64 : GrB_INT32 ;
    LAGr_Vector_new (&v, int_type, n) ;

    // make v dense if requested
    int64_t vlimit = LAGRAPH_MAX (256, sqrt ((double) n)) ;
    if (!vsparse)
    {
        // v is expected to have many entries, so convert v to dense.
        // If the guess is wrong, v can be made dense later on.
        LAGr_assign (v, NULL, NULL, 0, GrB_ALL, n, NULL) ;
    }

    GrB_Semiring first_semiring, second_semiring ;
    if (compute_tree)
    {
        // create an integer vector q, and set q(source) to source+1
        LAGr_Vector_new (&q, int_type, n) ;
        LAGr_Vector_setElement (q, source+1, source) ;

        if (n > INT32_MAX)
        {
            #if defined ( GxB_SUITESPARSE_GRAPHBLAS ) \
                && ( GxB_IMPLEMENTATION >= GxB_VERSION (3,2,0) )
                // terminates as soon as it finds any parent; nondeterministic
                first_semiring  = GxB_ANY_FIRST_INT64 ;
                second_semiring = GxB_ANY_SECOND_INT64 ;
            #else
                // deterministic, but cannot terminate early
                first_semiring  = LAGraph_MIN_FIRST_INT64 ;
                second_semiring = LAGraph_MIN_SECOND_INT64 ;
            #endif
        }
        else
        {
            #if defined ( GxB_SUITESPARSE_GRAPHBLAS ) \
                && ( GxB_IMPLEMENTATION >= GxB_VERSION (3,2,0) )
                // terminates as soon as it finds any parent; nondeterministic
                first_semiring  = GxB_ANY_FIRST_INT32 ;
                second_semiring = GxB_ANY_SECOND_INT32 ;
            #else
                // deterministic, but cannot terminate early
                first_semiring  = LAGraph_MIN_FIRST_INT32 ;
                second_semiring = LAGraph_MIN_SECOND_INT32 ;
            #endif
        }

        // create the empty parent vector
        LAGr_Vector_new (&pi, int_type, n) ;
        if (!vsparse)
        {
            // make pi a dense vector of all zeros
            LAGr_assign (pi, NULL, NULL, 0, GrB_ALL, n, NULL) ;
        }
        // pi (source) = source+1 denotes a root of the BFS tree
        LAGr_Vector_setElement (pi, source+1, source) ;
    }
    else
    {
        // create a boolean vector q, and set q(source) to true
        LAGr_Vector_new (&q, GrB_BOOL, n) ;
        LAGr_Vector_setElement (q, true, source) ;

        #if defined ( GxB_SUITESPARSE_GRAPHBLAS ) \
            && ( GxB_IMPLEMENTATION >= GxB_VERSION (3,2,0) )
            // terminates as soon as it finds any pair
            first_semiring  = GxB_ANY_PAIR_BOOL ;
            second_semiring = GxB_ANY_PAIR_BOOL ;
        #else
            // can terminate early, but requires more data movement internally
            first_semiring  = LAGraph_LOR_FIRST_BOOL ;
            second_semiring = LAGraph_LOR_SECOND_BOOL ;
        #endif
    }

    // average node degree
    double d = (n == 0) ? 0 : (((double) nvalA) / (double) n) ;

    int64_t nvisited = 0 ;      // # nodes visited so far
    GrB_Index nq = 1 ;          // number of nodes in the current level

    //--------------------------------------------------------------------------
    // BFS traversal and label the nodes
    //--------------------------------------------------------------------------

    for (int64_t level = 1 ; ; level++)
    {

        //----------------------------------------------------------------------
        // set v to the current level, for all nodes in q
        //----------------------------------------------------------------------

        // v<q> = level: set v(i) = level for all nodes i in q
        LAGr_assign (v, q, NULL, level, GrB_ALL, n, desc_s) ;

        //----------------------------------------------------------------------
        // check if done
        //----------------------------------------------------------------------

        nvisited += nq ;
        if (nq == 0 || nvisited == n || level >= max_level) break ;

        //----------------------------------------------------------------------
        // check if v should be converted to dense
        //----------------------------------------------------------------------

        if (vsparse && nvisited > vlimit)
        {
            // Convert v from sparse to dense to speed up the rest of the work.
            // If this case is triggered, it would have been faster to pass in
            // vsparse = false on input.
            // v <!v> = 0
            LAGr_assign (v, v, NULL, 0, GrB_ALL, n, desc_sc) ;
            LAGr_Vector_nvals (&ignore, v) ;

            if (compute_tree)
            {
                // Convert pi from sparse to dense, to speed up the work.
                // pi<!pi> = 0
                LAGr_assign (pi, pi, NULL, 0, GrB_ALL, n, desc_sc) ;
                LAGr_Vector_nvals (&ignore, pi) ;
            }

            vsparse = false ;
        }

        //----------------------------------------------------------------------
        // select push vs pull
        //----------------------------------------------------------------------

//      if (push_pull)
//      {
            double pushwork = d * nq ;
            double expected = (double) n / (double) (nvisited+1) ;
            double per_dot = LAGRAPH_MIN (d, expected) ;
            double binarysearch = (3 * (1 + log2 ((double) nq))) ;
            double pullwork = (n-nvisited) * per_dot * binarysearch ;
//          use_vxm_with_A = (pushwork < pullwork) ;

//          if (!csr)
//          {
//              // Neither A(i,:) nor AT(i,:) is efficient.  Instead, both
//              // A(:,j) and AT(:,j) is fast (that is, the two matrices
//              // are in CSC format).  Swap the
//              use_vxm_with_A = !use_vxm_with_A ;
//          }
//      }

        //----------------------------------------------------------------------
        // q = next level of the BFS
        //----------------------------------------------------------------------

double tic [2] ;
LAGraph_tic (tic) ;

        {
            // q<!v> = AT*q
            // this is a pull step if AT is in CSR format; push if CSC
            GrB_Vector q2 ;
            LAGr_Vector_new (&q2, compute_tree ? int_type : GrB_BOOL, n) ;
            LAGr_mxv (q2, v, NULL, second_semiring, AT, q, desc_rc) ;
            LAGr_free (&q2) ;
        }

double t_pull = LAGraph_toc (tic) ;
LAGraph_tic (tic) ;

        {
            // q'<!v> = q'*A
            // this is a push step if A is in CSR format; pull if CSC
            LAGr_vxm (q, v, NULL, first_semiring, q, A, desc_rc) ;
        }
double t_push = LAGraph_toc (tic) ;

// log the timings
fprintf (logfile, "%g %g %g %g\n",
    (double) nq, (double) nvisited, t_pull, t_push) ;
fflush (logfile) ;

        //----------------------------------------------------------------------
        // move to next level
        //----------------------------------------------------------------------

        if (compute_tree)
        {

            //------------------------------------------------------------------
            // assign parents
            //------------------------------------------------------------------

            // q(i) currently contains the parent of node i in tree (off by one
            // so it won't have any zero values, for valued mask).
            // pi<q> = q
            LAGr_assign (pi, q, NULL, q, GrB_ALL, n, desc_s) ;

            //------------------------------------------------------------------
            // replace q with current node numbers
            //------------------------------------------------------------------

            // TODO this could be a unaryop
            // q(i) = i+1 for all entries in q.

            #ifdef GxB_SUITESPARSE_GRAPHBLAS
            GrB_Index *qi ;
            if (n > INT32_MAX)
            {
                int64_t *qx ;
                LAGr_Vector_export (&q, &int_type, &n, &nq, &qi,
                    (void **) (&qx), NULL) ;
                int nth = LAGRAPH_MIN (nq / (64*1024), nthreads) ;
                nth = LAGRAPH_MAX (nth, 1) ;
                #pragma omp parallel for num_threads(nth) schedule(static)
                for (int64_t k = 0 ; k < nq ; k++)
                {
                    qx [k] = qi [k] + 1 ;
                }
                LAGr_Vector_import (&q, int_type, n, nq, &qi,
                    (void **) (&qx), NULL) ;
            }
            else
            {
                int32_t *qx ;
                LAGr_Vector_export (&q, &int_type, &n, &nq, &qi,
                    (void **) (&qx), NULL) ;
                int nth = LAGRAPH_MIN (nq / (64*1024), nthreads) ;
                nth = LAGRAPH_MAX (nth, 1) ;
                #pragma omp parallel for num_threads(nth) schedule(static)
                for (int32_t k = 0 ; k < nq ; k++)
                {
                    qx [k] = qi [k] + 1 ;
                }
                LAGr_Vector_import (&q, int_type, n, nq, &qi,
                    (void **) (&qx), NULL) ;
            }

            #else

            // TODO: use extractTuples and build instead

            // Or use something like:
            // extract tuples into I
            // let e = 1:n be created once, in initialization phase
            // q<q> = e (I)
            fprintf (stderr, "TODO: use extractTuples here\n") ;
            abort ( ) ;

            #endif

        }
        else
        {

            //------------------------------------------------------------------
            // count the nodes in the current level
            //------------------------------------------------------------------

            LAGr_Vector_nvals (&nq, q) ;
        }
    }

    //--------------------------------------------------------------------------
    // return the parent vector, if computed
    //--------------------------------------------------------------------------

    if (compute_tree)
    {
        (*pi_output) = pi ;
        pi = NULL ;
    }

    //--------------------------------------------------------------------------
    // free workspace and return result
    //--------------------------------------------------------------------------

    (*v_output) = v ;       // return result
    v = NULL ;              // set to NULL so LAGRAPH_FREE_ALL doesn't free it
    LAGRAPH_FREE_ALL ;      // free all workspace (except for result v)
    return (GrB_SUCCESS) ;
}

