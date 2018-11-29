//------------------------------------------------------------------------------
// allktruss: construct all k-trusses of a graph (OpenMP, not with GraphBLAS)
//------------------------------------------------------------------------------

// C = allktruss (A), all k-trusses of the graph A

// On input, A is the adjacency matrix of a graph, which must be square with
// symmetric pattern, and no diagonal entries.  These conditions are not
// checked.  A is treated as if binary on input so the content of Ax is ignored
// on input.  The matrix A is represented in compressed sparse column form as
// Ap, Ai, and n on input.  That is, the pattern of column A(:,j) is held in
// Ai [Ap [j] ... Ap [j+1]-1], where Ap [0] = 0 and Ap [n] = nnz (A).

// The value of k for the requested k-truss is provided as the scalar input,
// support = k-2, which must be > 0.

// On output, the input graph A is destroyed.  The k-trusses of A are returned
// as a list of matrices (Cps, Cis, and Cxs).  The k-truss of A is held in:
//      Cp = Cps [k]: array of size n+1, column pointers.
//      Ci = Cis [k]: row indices, of size nz = Cp [n].
//      Cx = Cxs [k]: values, of size nz.
// In this matrix, C(i,j) is the # of ntriangles in A that are in the k-truss
// of A.  The edges of the k-truss are a subset of the input graph A.  Each
// edge in C is part of at least k-2 triangles in C.  The pattern of C, (that
// is, spones(C) in MATLAB notation), is the adjacency matrix of the k-truss
// subgraph of A.  The edge weights of C are the support of each edge.  That
// is, C(i,j)=nt if the edge (i,j) is part of nt triangles in C.  All edges in
// C have support of at least k-2.  The total number of triangles in C is
// sum(sum(C))/6 in MATLAB notation.  The number of edges in C is nnz(C)/2, in
// MATLAB notation, or Cp [n]/2.  C is returned as symmetric with a zero-free
// diagonal, with all entries greater than or equal to k-2.  The matrix C is
// returned on output in compressed sparse column form in Cp, Ci, Cx, in the
// entries Cps [k], Cis [k], and Cxs [k], respectively.  That is, the pattern
// and values of C(:,j) are held in Ci [Cp [j] ... Cp [j+1]-1] and Cx [Cp [j]
// ... Cp [j+1]-1], where Cp [0] = 0 and Cp [n] = nnz (C).  Note that returning
// the k-trusses is optional; they are only returned if Cps, Cis, and Cxs are
// non-NULL on input.  The last k-truss, when k=kmax, is empty and this matrix
// is not returned (Cps [kmax], Cis [kmax], and Cxs [kmax] are NULL).

#include "ktruss_def.h"

//------------------------------------------------------------------------------
// cmult: C<A>=A*A
//------------------------------------------------------------------------------

// workspace for each thread:
_Thread_local Index *restrict w    = NULL ;
_Thread_local bool  *restrict Mark = NULL ;

void cmult                      // C = (A*A) .* A, overwriting A with C
(
    // input/output:
    int64_t *restrict Ap,       // column pointers, size n+1
    Index   *restrict Ai,       // row indices, size anz = Ap [n]
    // output, content not defined on input:
    Index   *restrict Ax,       // values
    // input, not modified:
    const Index n,              // A is n-by-n
    const int nthreads,         // # of threads
    const Index chunk           // scheduler chunk size
)
{

    //----------------------------------------------------------------------
    // C = (A*A) .* A
    //----------------------------------------------------------------------

    // This step computes the values of C in Ax.  The pattern of A and C
    // are the same, and are in Ap, Ai, and n.  A is treated as if binary
    // so its values are ignored.

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
}

//------------------------------------------------------------------------------
// allktruss: construct all k-trusses of a graph
//------------------------------------------------------------------------------

bool allktruss                  // true if successful, false otherwise
(
    // input/output:
    int64_t *restrict Ap,       // column pointers, size n+1
    Index   *restrict Ai,       // row indices, size anz = Ap [n]
    // output, content not defined on input:
    Index   *restrict Ax,       // values
    // input, not modified:
    const Index n,              // A is n-by-n
    const int threads,          // # of threads
    const Index chunk,          // scheduler chunk size

    // output statistics
    int64_t *restrict kmax,     // smallest k where k-truss is empty
    int64_t *restrict ntris,    // size n, ntris [k] is #triangles in k-truss
    int64_t *restrict nedges,   // size n, nedges [k] is #edges in k-truss
    int64_t *restrict nstepss,  // size n, nsteps [k] is #steps for k-truss

    // optional output k-trusses, if present
    int64_t **restrict Cps,     // size n
    Index   **restrict Cis,     // size n
    Index   **restrict Cxs      // size n
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    if (nstepss == NULL || kmax == NULL || ntris == NULL || nedges == NULL)
    {
        return (false) ;
    }

    bool keep_all_ktrusses = (Cps != NULL && Cis != NULL && Cxs != NULL) ;

    int nthreads = (n < chunk) ? 1 : threads ;

    for (Index k = 0 ; k < 3 ; k++)
    {
        if (keep_all_ktrusses)
        {
            Cps [k] = NULL ;
            Cis [k] = NULL ;
            Cxs [k] = NULL ;
        }
        ntris   [k] = 0 ;
        nedges  [k] = 0 ;
        nstepss [k] = 0 ;
    }
    (*kmax) = 0 ;

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

    if (!ok) return (false) ;

    //--------------------------------------------------------------------------
    // C = allktruss (A)
    //--------------------------------------------------------------------------

    double tmult = 0 ;
    double tsel  = 0 ;
    double t1 = omp_get_wtime ( ) ;

    // C = (A*A) .* A, overwriting A with C
    int64_t last_cnz = Ap [n] ;
    cmult (Ap, Ai, Ax, n, nthreads, chunk) ;
    int64_t nsteps = 1 ;

    double t2 = omp_get_wtime ( ) ;
    printf ("cmult time: %g\n", t2-t1) ;
    tmult += (t2-t1) ;

    //--------------------------------------------------------------------------

    for (Index k = 3 ; ; k++)
    {
        Index support = k-2 ;

        while (1)
        {

            //------------------------------------------------------------------
            // C = C .* (C >= support)
            //------------------------------------------------------------------

            // C is now in Ap, Ai, Ax, and n.
            // Prune all entries C(i,j) < support.

            double t1 = omp_get_wtime ( ) ;

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

            t2 = omp_get_wtime ( ) ;
            printf ("select time: %g\n", t2-t1) ;
            tsel += (t2-t1) ;

            //------------------------------------------------------------------
            // check if k-truss has been found
            //------------------------------------------------------------------

            if (cnz == last_cnz)
            {
                // k-truss has been found
                ntris   [k] = ktruss_ntriangles (cnz, Ax) ;
                nedges  [k] = cnz / 2 ;
                nstepss [k] = nsteps ;
                nsteps = 0 ;
                if (cnz == 0)
                {
                    // this is the last k-truss (an empty matrix)
                    if (keep_all_ktrusses)
                    {
                        Cps [k] = NULL ;
                        Cis [k] = NULL ;
                        Cxs [k] = NULL ;
                    }
                    (*kmax) = k ;
                    #pragma omp parallel num_threads(nthreads)
                    {
                        free (w) ;
                        free (Mark) ;
                    }
                    printf ("allktruss nthreads %d done: tmult %g tsel %g\n", nthreads, tmult, tsel) ;
                    return (true) ;
                }
                else if (keep_all_ktrusses)
                {
                    // save the k-truss in the list of output k-trusses
                    int64_t *Cp = (int64_t *) malloc ((n+1) * sizeof (int64_t));
                    Index   *Ci = (Index   *) malloc (cnz   * sizeof (Index)) ;
                    Index   *Cx = (Index   *) malloc (cnz   * sizeof (Index)) ;
                    Cps [k] = Cp ;
                    Cis [k] = Ci ;
                    Cxs [k] = Cx ;
                    if (Cp == NULL || Ci == NULL || Cx == NULL)
                    {
                        // out of memory: free all outputs
                        for (Index j = 3 ; j <= k ; j++)
                        {
                            if (Cps [j] != NULL) free (Cps [j]) ;
                            if (Cis [j] != NULL) free (Cis [j]) ;
                            if (Cxs [j] != NULL) free (Cxs [j]) ;
                            Cps [j] = NULL ;
                            Cis [j] = NULL ;
                            Cxs [j] = NULL ;
                        }
                        #pragma omp parallel num_threads(nthreads)
                        {
                            free (w) ;
                            free (Mark) ;
                        }
                        return (false) ;
                    }
                    memcpy (Cp, Ap, (n+1) * sizeof (int64_t)) ;
                    memcpy (Ci, Ai, cnz   * sizeof (Index)) ;
                    memcpy (Cx, Ax, cnz   * sizeof (Index)) ;
                }

                // start finding the next k-truss
                break ;
            }

            //------------------------------------------------------------------
            // continue searching for this k-truss
            //------------------------------------------------------------------

            nsteps++ ;
            
            //------------------------------------------------------------------
            // count the triangles for the next iteration
            //------------------------------------------------------------------

            t1 = omp_get_wtime ( ) ;

            // C = (A*A) .* A, overwriting A with C
            cmult (Ap, Ai, Ax, n, nthreads, chunk) ;
            last_cnz = Ap [n] ;

            t2 = omp_get_wtime ( ) ;
            printf ("mult time: %g\n", t2-t1) ;
            tmult += (t2-t1) ;
        }
    }

    return (false) ;
}

