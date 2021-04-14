//------------------------------------------------------------------------------
// LAGraph_cdlp: community detection using label propagation
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

// LAGraph_cdlp: Contributed by Gabor Szarnyas and Balint Hegyi,
// Budapest University of Technology and Economics
// (with accented characters: G\'{a}bor Sz\'{a}rnyas and B\'{a}lint Hegyi,
// using LaTeX syntax). https://inf.mit.bme.hu/en/members/szarnyasg .

// ## Background
//
// This function was originally written for the LDBC Graphalytics benchmark.
//
// The community detection using label propagation (CDLP) algorithm is
// defined both for directed and undirected graphs.
//
// The definition implemented here is described in the following document:
// https://ldbc.github.io/ldbc_graphalytics_docs/graphalytics_spec.pdf
//
// The algorithm is based on the one given in the following paper:
//
// Usha Raghavan, Reka Albert, and Soundar Kumara. "Near linear time algorithm
// to detect community structures in large-scale networks". In: Physical
// Review E 76.3 (2007), p. 036106, https://arxiv.org/abs/0709.2938
//
// The key idea of the algorithm is that each vertex is assigned the label
// that is most frequent among its neighbors. To allow reproducible
// experiments, the algorithm is modified to guarantee deterministic behavior:
// it always picks the smallest label in case of a tie:
//
// min ( argmax_{l} (#neighbors with label l) )
//
// In other words, we need to compute the *minimum mode value* (minmode) for
// the labels among the neighbors.
//
// For directed graphs, a label on a neighbor that is connected through both
// an outgoing and on an incoming edge counts twice:
//
// min ( argmax_{l} (#incoming neighbors with l + #outgoing neighbors with l) )
//
// ## Example (undirected)
//
// For an example, let's assume an undirected graph where vertex 1 has four
// neighbors {2, 3, 4, 5}, and the current labels in the graph are
// L = [3, 5, 4, 5, 4].
//
// In this example, the distribution of labels among the neighbors of vertex 1
// is {4 => 2, 5 => 2}, therefore, the minimum mode value is 4.
//
// Next, we capture this operation using GraphBLAS operations and
// data structures. Notice that the neighbors of vertex 1 are encoded
// as a sparse vector in the adjacency matrix:
//
// A = | 0 1 1 1 1 |
//     | 1 . . .   |
//     | 1 .       |
//     | 1 .       |
//     | 1         |
//
// To allow propagating the labels along edges, we use a diagonal matrix
// with the elements of the diagonal set to the values of L:
//
// diag(L) = | 3 0 0 0 0 |
//           | 0 5 0 0 0 |
//           | 0 0 4 0 0 |
//           | 0 0 0 5 0 |
//           | 0 0 0 0 4 |
//
// If we multiply adjacency matrix with diag(L), we get a matrix
// containing the labels of the neighbor nodes. We use the 'sel2nd' operator
// for multiplication to avoid having to lookup the value on the left.
// The conventional plus.times semiring would also work: 1 * y = sel2nd(1, y).
// Note that we multiply with a diagonal matrix so the addition operator
// is not used. In the implementation, we use "min" so the semiring is
// "min.sel2nd" on uint64 values.
//
// In the example, this gives the following:
//
// AL = A min.sel2nd diag(L) = | 0 5 4 5 4 |
//                             | 3 . . . . |
//
// ## Selecting the minimum mode value
//
// Next, we need to compute the minimum mode value for each row. As it is
// difficult to capture this operation as a monoid, we use a sort operation
// on each row. In the undirected case, we extract tuples <I, _, X> from the
// matrix, then use <I, X> for sorting. In the directed case, we extract
// tuples <I1, _, X1> and <I2, _, X2>, then use <I1+I2, X1+X2>,
// where '+' denotes concatenation. Column indices (J) are not used.
//
// The resulting two-tuples are sorted using a parallel merge sort.
// Finally, we use the sorted arrays compute the minimum mode value for each
// row.
//
// ## Fixed point
//
// At the end of each iteration, we check whether L[i-1] == L[i] and
// terminate if we reached a fixed point.
//
// ## Further optimizations
//
// A possible optimization is that the first iteration is rather trivial:
//
// * in the undirected case, each vertex gets the minimal initial label (=id)
//   of its neighbors.
// * in the directed case, each vertex gets the minimal initial label (=id)
//   of its neighbors which are doubly-linked (on an incoming and on an
//   outgoing edge). In the absence of such a neighbor, it picks the minimal
//   label of its neighbors (connected through either an incoming or through
//   an outgoing edge).

#include "shared/LAGraph_internal.h"

#define LAGRAPH_FREE_ALL                                                       \
{                                                                              \
    GrB_free (&L) ;                                                            \
    GrB_free (&L_prev) ;                                                       \
    if (sanitize) {                                                            \
        LAGRAPH_FREE (AI) ;                                                    \
        LAGRAPH_FREE (AJ) ;                                                    \
        LAGRAPH_FREE (AX) ;                                                    \
        GrB_Matrix_free(&S) ;                                                  \
    }                                                                          \
    GrB_free (&AT) ;                                                           \
    GrB_free (&desc) ;                                                         \
}

GrB_Info LAGraph_cdlp
(
    GrB_Vector *CDLP_handle, // output vector
    const GrB_Matrix A,      // input matrix
    bool symmetric,          // denote whether the matrix is symmetric
    bool sanitize,           // if true, ensure A is binary
    int itermax,             // max number of iterations,
    double *t                // t [0] = sanitize time, t [1] = cdlp time,
                             // in seconds
)
{
    GrB_Info info;

    // Diagonal label matrix
    GrB_Matrix L = NULL;
    GrB_Matrix L_prev = NULL;
    // Source adjacency matrix
    GrB_Matrix S = NULL;
    // Transposed matrix for the unsymmetric case
    GrB_Matrix AT = NULL;
    // Result CDLP vector
    GrB_Vector CDLP = NULL;

    GrB_Descriptor desc = NULL;

    // Arrays holding extracted tuples if the matrix needs to be copied
    GrB_Index *AI = NULL;
    GrB_Index *AJ = NULL;
    GrB_Index *AX = NULL;
    // Arrays holding extracted tuples during the algorithm
    GrB_Index *I = NULL;
    GrB_Index *X = NULL;
    // Array for counting frequency of values in each row
    int64_t *workspace = NULL;

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    if (CDLP_handle == NULL)
    {
        return GrB_NULL_POINTER;
    }

    //--------------------------------------------------------------------------
    // ensure input is binary and has no self-edges
    //--------------------------------------------------------------------------

    // n = size of A (# of nodes in the graph)
    // nz = # of non-zero elements in the matrix
    // nnz = # of non-zero elements used in the computations
    //   (twice as many for directed graphs)
    GrB_Index n, nz, nnz, n_in, n_out;
    LAGRAPH_OK (GrB_Matrix_nrows(&n, A))
    LAGRAPH_OK (GrB_Matrix_nvals(&nz, A))
    if (!symmetric)
    {
        nnz = 2 * nz;
    }
    else
    {
        nnz = nz;
    }

    if (sanitize)
    {

        AI = LAGraph_malloc(nz, sizeof(GrB_Index));
        AJ = LAGraph_malloc(nz, sizeof(GrB_Index));
        LAGRAPH_OK (GrB_Matrix_extractTuples_UINT64(AI, AJ, GrB_NULL, &nz, A))

        AX = LAGraph_malloc(nz, sizeof(GrB_Index));
        LAGRAPH_OK (GrB_Matrix_new(&S, GrB_UINT64, n, n));
        LAGRAPH_OK (GrB_Matrix_build(S, AI, AJ, AX, nz, GrB_PLUS_UINT64))

    }
    else
    {
        // Use the input as-is, and assume it is UINT64(!) with no self edges.
        // Results are undefined if this condition does not hold.
        S = A;
    }

    GxB_Format_Value A_format = -1, global_format = -1 ;
    LAGRAPH_OK (GxB_get(A, GxB_FORMAT, &A_format))
    LAGRAPH_OK (GxB_get(GxB_FORMAT, &global_format))
    if (A_format != GxB_BY_ROW || global_format != GxB_BY_ROW)
    {
        LAGRAPH_ERROR(
            "CDLP algorithm only works on matrices stored by row (CSR)",
            GrB_INVALID_OBJECT
        )
    }

    // TODO: heap is no longer in SuiteSparse, as of 3.2.0draftx.
    // the new saxpy method is used instead (Gustavson + Hash) --> TODO: use saxpy?
    LAGRAPH_OK (GrB_Descriptor_new(&desc))
//    LAGRAPH_OK (GrB_Descriptor_set(desc, GxB_AxB_METHOD, GxB_AxB_SAXPY))

    // Initialize L with diagonal elements 1..n
    I = LAGraph_malloc (n, sizeof (GrB_Index)) ;
    X = LAGraph_malloc (n, sizeof (GrB_Index)) ;
    if (I == NULL || X == NULL)
    {
        LAGRAPH_ERROR ("out of memory", GrB_OUT_OF_MEMORY) ;
    }
    for (GrB_Index i = 0; i < n; i++) {
        I[i] = i;
        X[i] = i;
    }
    // Initialize matrix for storing current labels
    LAGRAPH_OK (GrB_Matrix_new (&L, GrB_UINT64, n, n)) ;
    LAGRAPH_OK (GrB_Matrix_build (L, I, I, X, n, GrB_PLUS_UINT64)) ;
    // Initialize matrix for storing previous labels
    LAGRAPH_OK(GrB_Matrix_new(&L_prev, GrB_UINT64, n, n))
    LAGRAPH_OK (GrB_Matrix_build (L_prev, I, I, X, n, GrB_PLUS_UINT64)) ;
    LAGRAPH_FREE (I) ;
    LAGRAPH_FREE (X) ;

    if (!symmetric)
    {
        // compute AT for the unsymmetric case as it will be used
        // to compute A' = A' min.2nd L in each iteration
        LAGRAPH_OK (GrB_Matrix_new (&AT, GrB_UINT64, n, n)) ;
        LAGRAPH_OK (GrB_transpose (AT, NULL, NULL, A, NULL)) ;
    }

    // Initialize data structures for extraction from 'AL_in' and (for directed graphs) 'AL_out'
    I = LAGraph_malloc(nnz, sizeof(GrB_Index));
    X = LAGraph_malloc(nnz, sizeof(GrB_Index));
    workspace = LAGraph_calloc(nnz, sizeof(int64_t));
    for (int iteration = 0; iteration < itermax; iteration++)
    {

        // A = A min.2nd L
        // (using the "push" (saxpy) method)
        LAGRAPH_OK(GrB_mxm(S, GrB_NULL, GrB_NULL, GxB_MIN_SECOND_UINT64, S, L, desc))
        LAGRAPH_OK(GrB_Matrix_extractTuples_UINT64(I, GrB_NULL, X, &nz, S))
        // Store the number of tuples populated by the adjacency matrix.
        n_out = nz;

        if (!symmetric)
        {
            // A' = A' min.2nd L
            // (using the "push" (saxpy) method)
            LAGRAPH_OK(GrB_mxm(AT, GrB_NULL, GrB_NULL, GxB_MIN_SECOND_UINT64, AT, L, desc))
            LAGRAPH_OK(GrB_Matrix_extractTuples_UINT64(&I[nz], GrB_NULL, &X[nz], &nz, AT))
        }
        // Store the number of tuples populated by the transposed adjacency matrix.
        n_in = nz;

        // save current labels for comparison by swapping L and L_prev
        GrB_Matrix L_swap = L;
        L = L_prev;
        L_prev = L_swap;

        int64_t idx1, idx2;
        int64_t prev_idx1 = 0;
        int64_t prev_idx2 = n_in;
        while(true) {
            // Count the frequency of all values in the current row's outgoing edges.
            for(idx1 = prev_idx1; idx1 < n_out; idx1++)
            {
                if(idx1 > prev_idx1 && I[idx1] != I[idx1-1]) break;
                workspace[X[idx1]] ++;
            }
            // Count the frequency of all values in the current row's incoming edges.
            for(idx2 = prev_idx2; idx2 < n_in + n_out; idx2++)
            {
                if(idx2 > prev_idx2 && I[idx2] != I[idx2-1]) break;
                workspace[X[idx2]] ++;
            }

            // Find (one of) the values with the highest frequency for this row.
            int64_t max = -1;
            GrB_Index max_idx = 0;
            for(int64_t k = 0; k < nnz; k ++)
            {
                if(workspace[k] > max)
                {
                    max = workspace[k];
                    max_idx = k;
                }
            }

            // Update row label within the matrix.
         if(max >= 0)
         {
            GrB_Matrix_setElement(L, max_idx, I[prev_idx1], I[prev_idx1]);
         }

            // Reset the workspace.
            memset(workspace, -1, nnz * sizeof(int64_t));

            // Track the beginning of the next row.
            prev_idx1 = idx1;
            prev_idx2 = idx2;

            // Break if we've exhausted the input.
            if(idx1 >= n_in && idx2 >= n_out) break;
        }

        bool isequal;
        LAGraph_isequal(&isequal, L_prev, L, GrB_NULL);
        if (isequal) {
            break;
        }
    }
    LAGRAPH_FREE (I) ;
    LAGRAPH_FREE (X) ;
    LAGRAPH_FREE (workspace) ;

    //--------------------------------------------------------------------------
    // extract final labels to the result vector
    //--------------------------------------------------------------------------

    LAGRAPH_OK (GrB_Vector_new(&CDLP, GrB_UINT64, n))
    for (GrB_Index i = 0; i < n; i++)
    {
        uint64_t x;
        LAGRAPH_OK(GrB_Matrix_extractElement(&x, L, i, i))
        LAGRAPH_OK(GrB_Vector_setElement(CDLP, x, i))
    }

    //--------------------------------------------------------------------------
    // free workspace and return result
    //--------------------------------------------------------------------------

    (*CDLP_handle) = CDLP;
    CDLP = NULL;            // set to NULL so LAGRAPH_FREE_ALL doesn't free it
    LAGRAPH_FREE_ALL

    return (GrB_SUCCESS);
}
