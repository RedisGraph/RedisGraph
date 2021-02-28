//------------------------------------------------------------------------------
// allktruss_main.c: construct all k-trusses of a graph (without GraphBLAS)
//------------------------------------------------------------------------------

// Read a graph from a file and construct all k-trusses
// Usage:
//
//  allktruss_main < infile
//
// See the "kgo" script for the whole GraphChallenge collection.

#include "ktruss_def.h"

#define CHUNK 1000

// select the system:

// cholesky.cse.tamu.edu: 160 hardware threads (20 cores, SMT8),
// IBM Power8 8335-GTB, 4GHz, 1TB RAM

#define MAX_THREADS 160
#define FOR_ALL_THREADS(max_threads) \
    for (int nthreads = 1 ; nthreads <= max_threads ; \
        nthreads = (nthreads == 128) ? 160 : (2*nthreads))
/*
*/

// backslash.cse.tamu.edu: 24 cores, Intel Xeon CPU E5-2695 v2 @ 2.4GHz
// 3/4 TB RAM

/*
#define MAX_THREADS 48
#define FOR_ALL_THREADS(max_threads) \
    for (int nthreads = 1 ; nthreads <= max_threads ; \
        nthreads = (nthreads == 16) ? 24 : (2*nthreads))
*/

// slash MacBook: 4 cores, Intel Core i7, 2.8Ghz, 16GB RAM
/*
#define MAX_THREADS 4
#define FOR_ALL_THREADS(max_threads) \
    for (int nthreads = 1 ; nthreads <= max_threads ; nthreads++)
*/

int main (int argc, char **argv)
{

    double tic, Time [MAX_THREADS+1] ;

    //--------------------------------------------------------------------------
    // get a 1-based symmetric matrix with no self edges, from stdin
    //--------------------------------------------------------------------------

    FILE *f ;
    int64_t *Ap ;
    Index *Ai, n ;

    printf ("=============================================================\n") ;

    tic = omp_get_wtime ( ) ;
    if (argc > 1)
    {
        fprintf (stderr, "%s\n", argv [1]) ;
        printf ("\nfile: %s ", argv [1]) ;
        f = fopen (argv [1], "r") ;
        if (f == NULL) { printf (": no such file\n") ; exit (1) ; }
    }
    else
    {
        f = stdin ;
    }
    if (!ktruss_read (f, &Ap, &Ai, &n))
    {
        printf ("failed to read matrix\n") ;
        exit (1) ;
    }
    if (f != stdin) fclose (f) ;
    double tread = omp_get_wtime ( ) - tic ;

    int64_t nnz = Ap [n] ;
    int64_t anedges = nnz / 2 ;
    printf ("n %10"PRId64" edges %12"PRId64" read time: %10.4f sec\n",
        (int64_t) n, anedges, tread) ;

    /*
    Index *Ax = (Index *) malloc ((nnz+1) * sizeof (Index)) ;
    if (Ax == NULL)
    {
        printf ("out of memory\n") ;
        exit (1) ;
    }
    */

    int64_t *Sp = (int64_t *) malloc ((n+1)   * sizeof (int64_t)) ;
    Index   *Si = (Index   *) malloc ((nnz+1) * sizeof (Index)) ;
    Index   *Sx = (Index   *) malloc ((nnz+1) * sizeof (Index)) ;
    if (Sp == NULL || Si == NULL || Sx == NULL)
    {
        printf ("out of memory\n") ;
        exit (1) ;
    }

    int64_t kmax ;              // smallest k where k-truss is empty
    int64_t *ntris = NULL ;     // size n, ntris [k] is #triangles in k-truss
    int64_t *nedges = NULL ;    // size n, nedges [k] is #edges in k-truss
    int64_t *nstepss = NULL ;   // size n, nsteps [k] is #steps for k-truss

    ntris   = (int64_t *) malloc ((n+1) * sizeof (int64_t)) ;
    nedges  = (int64_t *) malloc ((n+1) * sizeof (int64_t)) ;
    nstepss = (int64_t *) malloc ((n+1) * sizeof (int64_t)) ;

    if (ntris == NULL || nedges == NULL || nstepss == NULL)
    {
        printf ("out of memory\n") ;
        exit (1) ;
    }

    //--------------------------------------------------------------------------

    printf ("\n") ;

    int64_t **Cps = NULL ;
    Index   **Cpi = NULL ;
    Index   **Cpx = NULL ;

    // for further MATLAB analysis
    FILE *fm = fopen ("allktruss_results.m", "a") ;
    fprintf (fm, "\n%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n") ;
    fprintf (fm, "id = id + 1 ;\n") ;
    fprintf (fm, "N (id) = %" PRIu64 " ;\n", n) ;
    fprintf (fm, "Nedges (id) = %" PRIu64 " ;\n", anedges) ;
    fprintf (fm, "%% T (keep, nthreads, id) = time for all-k-truss\n") ;
    fprintf (fm, "T (1:2, 1:%d, id) = nan ;\n", MAX_THREADS) ;

    // for (int keep = 0 ; keep <= 1 ; keep++)
    int keep = 0 ;
    {
        printf ("keep %d\n", keep) ;

        if (keep)
        {
            printf ("\n=== keep all k-trusses:\n") ;
            Cps = (int64_t **) malloc ((n+1) * sizeof (int64_t *)) ;
            Cpi = (Index   **) malloc ((n+1) * sizeof (Index   *)) ;
            Cpx = (Index   **) malloc ((n+1) * sizeof (Index   *)) ;
        }
        else
        {
            printf ("\n=== compute but then discard all k-trusses:\n") ;
        }

        FOR_ALL_THREADS (MAX_THREADS)
        {
            printf ("allktruss, threads: %d\n", nthreads) ;

            memcpy (Sp, Ap, (n+1) * sizeof (int64_t)) ;
            memcpy (Si, Ai, (nnz+1) * sizeof (Index)) ;

            tic = omp_get_wtime ( ) ;
            if (!allktruss (Sp, Si, Sx, n, nthreads, CHUNK,
                &kmax, ntris, nedges, nstepss, Cps, Cpi, Cpx))
            {
                printf ("failure\n") ;
                exit (1) ;
            }
            double t = omp_get_wtime ( ) - tic ;
            Time [nthreads] = t ;

            for (Index k = 3 ; k <= kmax ; k++)
            {
                int64_t nt = ntris [k] ;
                int64_t ne = nedges [k] ;
                int64_t nsteps = nstepss [k] ;
                printf ("allktruss : k %4"PRId64" ne %10"PRId64" nt %10"PRId64" steps %4"PRId64"\n",
                    k, ne, nt, nsteps) ;
            }
            printf ("allktruss     nthreads %3d : %12.6f sec, rate %7.2f"
                " speedup %6.2f\n", nthreads, t, (kmax-2) * 1e-6*anedges/t,
                Time [1] / t) ;
        }

        if (keep == 0) fprintf (fm, "Kmax (id) = %" PRId64 " ;\n", kmax) ;

        FOR_ALL_THREADS (MAX_THREADS)
        {
            fprintf (fm, "T (%d,%d,id) = %12.6g ;\n",
                1+keep, nthreads, Time [nthreads]) ;
        }
    }
    fprintf (fm, "File {id} = filetrim (file) ;\n\n") ;

    fclose (fm) ;
}

