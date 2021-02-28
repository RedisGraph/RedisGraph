//------------------------------------------------------------------------------
// ktruss_graphblas_def.h:  include file for k-truss methods using GraphBLAS
//------------------------------------------------------------------------------

#include "GraphBLAS.h"

// GraphBLAS/Demo:  uses simple_tic, simple_toc, OK(...) macro, and get_matrix
#include "demos.h"

GrB_Info ktruss_graphblas       // compute the k-truss of a graph
(
    GrB_Matrix *p_C_output,     // output k-truss subgraph, C
    GrB_Matrix A_input,         // input adjacency matrix, A, not modified
    const int64_t k,            // find the k-truss, where k >= 3
    int64_t *p_nsteps           // # of steps taken
) ;

GrB_Info allktruss_graphblas    // compute all k-trusses of a graph
(
    GrB_Matrix *Cset,           // output k-truss subgraphs (optional)
    GrB_Matrix A,               // input adjacency matrix, A, not modified

    // output statistics
    int64_t *kmax,              // smallest k where k-truss is empty
    int64_t *ntris,             // size n, ntris [k] is #triangles in k-truss
    int64_t *nedges,            // size n, nedges [k] is #edges in k-truss
    int64_t *nstepss            // size n, nstepss [k] is #steps for k-truss
) ;

