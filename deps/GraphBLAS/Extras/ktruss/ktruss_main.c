//------------------------------------------------------------------------------
// ktruss_main.c: construct k-trusses of a graph (without using GraphBLAS)
//------------------------------------------------------------------------------

// Read a graph from a file and construct all k-trusses
// Usage:
//
//  ktruss_main < infile
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
    int64_t nedges = nnz / 2 ;
    printf ("n %10"PRId64" edges %12"PRId64" read time: %10.4f sec\n",
        (int64_t) n, nedges, tread) ;

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

    //--------------------------------------------------------------------------
    // construct all k-trusses, with different # of threads
    //--------------------------------------------------------------------------

    // for further MATLAB analysis
    FILE *fm = fopen ("ktruss_results.m", "a") ;
    fprintf (fm, "\n%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n") ;
    fprintf (fm, "id = id + 1 ;\n") ;
    fprintf (fm, "N (id) = %"PRIu64" ;\n", n) ;
    fprintf (fm, "Nedges (id) = %"PRIu64" ;\n", nedges) ;
    fprintf (fm, "Time  = nan (2,%d) ;\n", MAX_THREADS) ;
    fprintf (fm, "%% Time (3:kmax, nthreads) = time for each k-truss\n") ;

    // for (Index k = 3 ; ; k++)
    Index k = 3 ;
    {
        printf ("k %"PRId64"\n", k) ;
        int64_t ne = 0 ;
    
        fprintf (fm, "Time = [Time ; nan(1,%d)] ;\n", MAX_THREADS) ;

        FOR_ALL_THREADS (MAX_THREADS)
        {
            memcpy (Sp, Ap, (n+1) * sizeof (int64_t)) ;
            memcpy (Si, Ai, (nnz+1) * sizeof (Index)) ;

            printf ("start ktruss\n") ;

            tic = omp_get_wtime ( ) ;
            int64_t nsteps = ktruss (Sp, Si, Sx, n, k-2, nthreads, CHUNK) ;
            double t = omp_get_wtime ( ) - tic ;
            printf ("did ktruss %12.6f sec\n", t) ;
            Time [nthreads] = t ;
            ne = Sp [n] / 2 ;
            int64_t nt = ktruss_ntriangles (Sp [n], Sx) ;
            printf (
            "ktruss        nthreads %3d : k %4"PRId64" ne %10"PRId64" nt %10"PRId64" %12.6f sec"
            " steps %4"PRId64" rate %7.2f speedup %6.2f\n",
            nthreads, k, ne, nt, t, nsteps, 1e-6*nedges/t, Time [1] / t) ;
            fprintf (fm, "Time (%"PRId64",%d) = %12.6g ;\n", k, nthreads, t) ;
        }

        fprintf (fm, "T {id} = Time ;\n") ;

        // if (ne == 0) break ;
    }

    fprintf (fm, "File {id} = filetrim (file) ;\n\n") ;
    fclose (fm) ;
}

