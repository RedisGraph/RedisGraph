//------------------------------------------------------------------------------
// ktruss: construct the k_truss of a graph
//------------------------------------------------------------------------------

// C = ktruss (A,k), the k-truss of the graph A

// On input, A is the adjacency matrix of a graph, which must be square with
// symmetric pattern, and no diagonal entries.  These conditions are not
// checked.  A is treated as if binary on input so the content of Ax is ignored
// on input.  The matrix A is represented in compressed sparse column form as
// Ap, Ai, and n on input.  That is, the pattern of column A(:,j) is held in
// Ai [Ap [j] ... Ap [j+1]-1], where Ap [0] = 0 and Ap [n] = nnz (A).

// The value of k for the requested k-truss is provided as the scalar input,
// support = k-2, which must be > 0.

// On output, the input graph A is overwitten with the graph C, which is the
// k-truss subgraph of A.  Its edges are a subset of the input graph A.  Each
// edge in C is part of at least k-2 triangles in C.  The pattern of C, (that
// is, spones(C) in MATLAB notation), is the adjacency matrix of the k-truss
// subgraph of A.  The edge weights of C are the support of each edge.  That
// is, C(i,j)=nt if the edge (i,j) is part of nt triangles in C.  All edges in
// C have support of at least k-2.  The total number of triangles in C is
// sum(sum(C))/6 in MATLAB notation.  The number of edges in C is nnz(C)/2, in
// MATLAB notation, or Ap [n]/2.  C is returned as symmetric with a zero-free
// diagonal, with all entries greater than or equal to k-2.  The matrix C is
// returned on output in compressed sparse column form in Ap, Ai, Ax, and n (n
// doesn't change).  That is, the pattern and values of C(:,j) are held in Ai
// [Ap [j] ... Ap [j+1]-1] and Ax [Ap [j] ... Ap [j+1]-1], where Ap [0] = 0 and
// Ap [n] = nnz (C).

// The return value is the # of steps the algorithm performed, or <= 0 on
// error, where 0 indicates that the support input was invalid, and -1
// indicates an out-of-memory condition.

#include "ktruss_def.h"

_Thread_local Index *restrict w    = NULL ;
_Thread_local bool  *restrict Mark = NULL ;

int64_t ktruss                  // # steps taken, or <= 0 if error
(
    // input/output:
    int64_t *restrict Ap,       // column pointers, size n+1
    Index   *restrict Ai,       // row indices, size anz = Ap [n]
    // output, content not defined on input:
    Index   *restrict Ax,       // values

    // input, not modified:
    const Index n,              // A is n-by-n
    const Index support,        // support = (k-2) for a k-truss, must be > 0
    const int threads,          // # of threads
    const Index chunk           // scheduler chunk size
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    if (support <= 0) return (0) ;

    int nthreads = (n < chunk) ? 1 : threads ;

    //--------------------------------------------------------------------------
    // allocate workspace
    //--------------------------------------------------------------------------

    bool ok = true ;

    #pragma omp parallel num_threads(nthreads) reduction(&&:ok)
    {
        w    = (Index *) calloc (n, sizeof (Index)) ;
        Mark = (bool  *) calloc (n, sizeof (bool )) ;
        ok = (Mark != NULL && w != NULL) ;
    }

    #pragma omp parallel num_threads(nthreads)
    {
        if (!ok)
        {
            // out of memory
            if (w    != NULL) free (w   ) ;
            if (Mark != NULL) free (Mark) ;
        }
    }

    if (!ok) return (-1) ;

    double tmult = 0 ;
    double tsel  = 0 ;

    //--------------------------------------------------------------------------
    // C = ktruss (A)
    //--------------------------------------------------------------------------

    for (int64_t nsteps = 1 ; ; nsteps++)
    {

        //----------------------------------------------------------------------
        // C = (A*A) .* A
        //----------------------------------------------------------------------

        // This step computes the values of C in Ax.  The pattern of A and C
        // are the same, and are in Ap, Ai, and n.  A is treated as if binary
        // so its values are ignored.  This computation is a mimic for
        // GrB_mxm (C, A, NULL, GxB_PLUS_LAND_INT64, A, A, NULL), except that
        // here, the output C can overwrite the input A.

        printf ("step %" PRId64"\n", nsteps) ;
        double t1 = omp_get_wtime ( ) ;

        #pragma omp parallel for num_threads(nthreads) schedule(dynamic,chunk)
        for (Index j = 0 ; j < n ; j++)
        {
            // scatter A(:,j) into Mark.  All of w is zero.
            for (int64_t p = Ap [j] ; p < Ap [j+1] ; p++)
            {
                Mark [Ai [p]] = 1 ;
            }
            // C(:,j) = (A * A(:,j)) .* Mark
            for (int64_t p = Ap [j] ; p < Ap [j+1] ; p++)
            {
                const Index k = Ai [p] ;                // (row k, not k-truss)
                // C(:,j) += (A(:,k) * A(k,j)) .* Mark
                for (int64_t pa = Ap [k] ; pa < Ap [k+1] ; pa++)
                {
                    // C(i,j) += (A(i,k) * A(k,j)) .* Mark
                    Index i = Ai [pa] ;
                    if (Mark [i]) w [i]++ ;
                }
            }
            // gather C(:,j) from the workspace and clear the Mark
            for (int64_t p = Ap [j] ; p < Ap [j+1] ; p++)
            {
                Index i = Ai [p] ;
                Ax [p] = w [i] ;
                Mark [i] = 0 ;
                w [i] = 0 ;
            }
        }

        double t2 = omp_get_wtime ( ) ;
        printf ("C<C>=C*C time: %g\n", t2-t1) ;
        tmult += (t2-t1) ;

        //----------------------------------------------------------------------
        // anz = nnz (A)
        //----------------------------------------------------------------------

        int64_t anz = Ap [n] ;

        //----------------------------------------------------------------------
        // C = C .* (C >= support)
        //----------------------------------------------------------------------

        // C is now in Ap, Ai, Ax, and n.  Prune all entries C(i,j) < support.
        // This code snippet is a mimic for
        // GxB_select (T, NULL, NULL, supportop, C, Support, NULL)
        // except that this code can operate on C in-place.

        int64_t cnz = 0 ;
        for (Index j = 0 ; j < n ; j++)
        {
            // log the start of column C(:,j)
            int64_t p1 = Ap [j] ;
            Ap [j] = cnz ;
            for (int64_t p = p1 ; p < Ap [j+1] ; p++)
            {
                // consider the edge C(i,j)
                Index i   = Ai [p] ;
                Index cij = Ax [p] ;
                if (cij >= support)
                {
                    // the edge C(i,j) has enough support; keep it
                    Ai [cnz  ] = i ;
                    Ax [cnz++] = cij ;
                }
            }
        }
        Ap [n] = cnz ;

        double t3 = omp_get_wtime ( ) ;
        printf ("select time: %g\n", t3-t2) ;
        tsel += (t3-t2) ;

        //----------------------------------------------------------------------
        // if (nnz (C) == nnz (A)) return
        //----------------------------------------------------------------------

        if (cnz == anz)
        {
            // k-truss has been found, free workspace and return result
            printf ("ktruss nthreads %d done: tmult %g tsel %g\n",
                nthreads, tmult, tsel) ;
            #pragma omp parallel num_threads(nthreads)
            {
                free (w) ;
                free (Mark) ;
            }
            return (nsteps) ;
        }
    }
}

