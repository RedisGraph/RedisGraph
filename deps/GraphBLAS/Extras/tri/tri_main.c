//------------------------------------------------------------------------------
// tri_main.c: count triangles
//------------------------------------------------------------------------------

// Read a graph from a file and count the # of triangles using two methods.
// Usage:
//
//  tri_main < infile
//
// See the "go" script for the whole GraphChallenge collection.

#include "tri_def.h"

#define NPREP 1
#define NMETHODS 5
#define CHUNK 1000

// select the system:

// cholesky.cse.tamu.edu: 160 hardware threads (20 cores, SMT8),
// IBM Power8 8335-GTB, 4GHz, 1TB RAM
#define MAX_THREADS 160
#define FOR_ALL_THREADS(max_threads) \
    for (int nthreads = 1 ; nthreads <= max_threads ; \
        nthreads = (nthreads == 128) ? 160 : (2*nthreads))

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
    for (int nthreads = 1 ; nthreads <= max_threads ; nthreads *= 2)
*/

// sequential
/*
#define MAX_THREADS 1
#define FOR_ALL_THREADS(max_threads) \
    for (int nthreads = 1 ; nthreads <= max_threads ; nthreads++)
*/

// uplo: -1 lower, 1 upper, 0 any
/*
void dump (char *name, int64_t *Ap, Index *Ai, Index n, int uplo)
{
    printf ("\n---- Matrix %s, n %"PRId64"\n", name, n) ;
    for (int64_t j = 0 ; j < n ; j++)
    {
        printf ("column %"PRId64": ", j) ;
        for (int64_t p = Ap [j] ; p < Ap [j+1] ; p++)
        {
            printf (" %"PRId64, Ai [p]) ;
            bool ok = true ;
            if (uplo == -1)
            {
                ok = (Ai [p] > j) ;
            }
            else if (uplo == 1)
            {
                ok = (Ai [p] < j) ;
            }
            if (!ok) { printf (" !!!\n") ; abort ( ) ; }
        }
        printf ("\n") ;
    }
}
*/

int main (int argc, char **argv)
{

    double tic, T_prep [2][NPREP][MAX_THREADS+1],
            Time [NMETHODS][NPREP][MAX_THREADS+1] ;
    int64_t Ntri [NMETHODS][NPREP][MAX_THREADS+1] ;

    //--------------------------------------------------------------------------
    // get a 1-based symmetric matrix with no self edges, from stdin
    //--------------------------------------------------------------------------

    FILE *f ;
    int64_t *Ap ;
    Index *Ai, n ;

    printf ("=============================================================\n") ;

    tic = omp_get_wtime ( ) ;

    f = stdin ;

    bool skip_simple = (argc > 1) ;
    fprintf (stderr, "skip simple: %d\n", skip_simple) ;

/*
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
*/

    if (!tri_read (f, &Ap, &Ai, &n))
    {
        printf ("failed to read matrix\n") ;
        exit (1) ;
    }
    if (f != stdin) fclose (f) ;
    double tread = omp_get_wtime ( ) - tic ;

    int64_t nnz = Ap [n] ;
    int64_t nedges = nnz / 2 ;
    printf ("n %"PRId64" edges %"PRId64" read time: %10.4f sec\n",
         n,  nedges, tread) ;
    fprintf (stderr, "n %"PRId64" edges %"PRId64" read time: %10.4f sec\n",
         n,  nedges, tread) ;

    // allocate space for S and R
    // use calloc to page in the space for more consistent timing
    int64_t *Sp = calloc ((n+1) , sizeof (int64_t)) ;
    Index   *Si = calloc ((nedges+1) , sizeof (Index)) ;
    int64_t *Rp = calloc ((n+1) , sizeof (int64_t)) ;
    Index   *Ri = calloc ((nedges+1) , sizeof (Index)) ;
    if (Sp == NULL || Si == NULL || Rp == NULL || Ri == NULL)
    {
        printf ("out of memory for R and S\n") ;
        exit (1) ;
    }

    //--------------------------------------------------------------------------
    // try each method
    //--------------------------------------------------------------------------

    for (int prep_method = 0 ; prep_method < NPREP ; prep_method++)
    {

        // warmup for consistent timing
        tri_prep (Sp, Si, Ap, Ai, n, prep_method, 1) ;

        //----------------------------------------------------------------------
        // create S
        //----------------------------------------------------------------------

        FOR_ALL_THREADS (MAX_THREADS)
        {
            tic = omp_get_wtime ( ) ;
            if (!tri_prep (Sp, Si, Ap, Ai, n, prep_method, nthreads))
            {
                printf ("matrix invalid or out of memory\n") ; exit (1) ;
            }
            T_prep [0][prep_method][nthreads] = omp_get_wtime ( ) - tic ;
            fprintf (stderr, "S prep:%d time %12.5f threads %3d\n",
            prep_method, T_prep [0][prep_method][nthreads], nthreads) ;
        }

        //----------------------------------------------------------------------
        // compute R=S' for dot-product method
        //----------------------------------------------------------------------

        // select the method that computes R=S'
        int R_prep_method ;
        switch (prep_method)
        {
            case 0: R_prep_method = 1 ; break ;
            case 1: R_prep_method = 0 ; break ;
            case 3: R_prep_method = 3 ; break ;
            case 4: R_prep_method = 4 ; break ;
        }

        FOR_ALL_THREADS (MAX_THREADS)
        {
            tic = omp_get_wtime ( ) ;
            if (!tri_prep (Rp, Ri, Ap, Ai, n, R_prep_method, nthreads))
            {
                printf ("failed to construct R\n") ; exit (1) ;
            }
            T_prep [1][prep_method][nthreads] = omp_get_wtime ( ) - tic ;
            fprintf (stderr, "R prep:%d time %12.5f threads %3d\n",
            prep_method, T_prep [1][prep_method][nthreads], nthreads) ;
        }

        //----------------------------------------------------------------------
        // warmup for more accurate timing
        //----------------------------------------------------------------------

        tri_mark_parallel (Sp, Si, n, 1, CHUNK) ;

        //----------------------------------------------------------------------
        // 0: triangle counting with bool Mark array, outer-product method
        //----------------------------------------------------------------------

        // make sure S has the right structure
        /*
        if (prep_method == 0) dump ("S = L for tri_mark", Sp, Si, n, -1) ;
        if (prep_method == 1) dump ("S = U for tri_mark", Sp, Si, n,  1) ;
        if (prep_method == 2) dump ("S = perm for tri_mark", Sp, Si, n,  0) ;
        if (prep_method == 3) dump ("S = perm for tri_mark", Sp, Si, n,  0) ;
        */

        FOR_ALL_THREADS (MAX_THREADS)
        {
            tic = omp_get_wtime ( ) ;
            int64_t nt = tri_mark_parallel (Sp, Si, n, nthreads, CHUNK) ;
            double t = omp_get_wtime ( ) - tic ;
            Ntri [0][prep_method][nthreads] = nt ;
            Time [0][prep_method][nthreads] = t ;
            fprintf (stderr, "%d: tri_mark     nthreads %3d : %" PRId64
                " %12.6f sec rate %7.2f\n", prep_method, nthreads, nt,
                t, 1e-6*nedges/t) ;
        }

        //----------------------------------------------------------------------
        // 1: triangle counting with bit-vector Mark, outer-product method
        //----------------------------------------------------------------------

        FOR_ALL_THREADS (MAX_THREADS)
        {
            tic = omp_get_wtime ( ) ;
            int64_t nt = tri_bit_parallel (Sp, Si, n, nthreads, CHUNK) ;
            double t = omp_get_wtime ( ) - tic ;
            Ntri [1][prep_method][nthreads] = nt ;
            Time [1][prep_method][nthreads] = t ;
            fprintf (stderr, "%d: tri_bit      nthreads %3d : %" PRId64
                " %12.6f sec rate %7.2f\n", prep_method, nthreads, nt,
                t, 1e-6*nedges/t) ;
        }

        //----------------------------------------------------------------------
        // 2: triangle counting, dot-product method
        //----------------------------------------------------------------------

        FOR_ALL_THREADS (MAX_THREADS)
        {
            tic = omp_get_wtime ( ) ;
            int64_t nt = tri_dot_parallel (Rp, Ri, Sp, Si, n, nthreads, CHUNK) ;
            double t = omp_get_wtime ( ) - tic ;
            Ntri [2][prep_method][nthreads] = nt ;
            Time [2][prep_method][nthreads] = t ;
            fprintf (stderr, "%d: tri_dot      nthreads %3d : %" PRId64
                " %12.6f sec rate %7.2f\n", prep_method, nthreads, nt,
                t, 1e-6*nedges/t) ;
        }

        //----------------------------------------------------------------------
        // 3: tri_logmark
        //----------------------------------------------------------------------

        FOR_ALL_THREADS (MAX_THREADS)
        {
            tic = omp_get_wtime ( ) ;
            int64_t nt = tri_logmark_parallel (Sp, Si, n, nthreads, CHUNK) ;
            double t = omp_get_wtime ( ) - tic ;
            Ntri [3][prep_method][nthreads] = nt ;
            Time [3][prep_method][nthreads] = t ;
            fprintf (stderr, "%d: tri_logmark  nthreads %3d : %" PRId64
                " %12.6f sec rate %7.2f\n", prep_method, nthreads, nt,
                t, 1e-6*nedges/t) ;
        }

        //----------------------------------------------------------------------
        // 4: tri_simple (one thread only)
        //----------------------------------------------------------------------

        fprintf (stderr, "\ntri_simple:\n") ;
        if (skip_simple)
        {
            Ntri [4][prep_method][1] = -1 ;
            Time [4][prep_method][1] = 9e99 ;
            fprintf (stderr, "%d: tri_simple   skipped\n", prep_method) ;
        }
        else
        {
            tic = omp_get_wtime ( ) ;
            int64_t nt = tri_simple (Sp, Si, n) ;
            double t = omp_get_wtime ( ) - tic ;
            Ntri [4][prep_method][1] = nt ;
            Time [4][prep_method][1] = t ;
            fprintf (stderr, "%d: tri_simple   nthreads %3d : %" PRId64
                " %12.6f sec rate %7.2f\n", prep_method, 1, nt,
                t, 1e-6*nedges/t) ;
        }

    }

    free (Rp) ;
    free (Ri) ;
    free (Sp) ;
    free (Si) ;
    free (Ap) ;
    free (Ai) ;

    //--------------------------------------------------------------------------
    // report results
    //--------------------------------------------------------------------------

    int64_t ntri = -1 ;
    double tbest = 1e99, tbest1 = 1e99 ;
    int nthreads_best, prep_best, best_method, max_threads ;
    int prep1_best, best1_method ;
    double speedup_method ;

    ntri = Ntri [0][0][1] ;

    printf ("-------------------------------------------------------------\n"
            "RESULTS:\n"
            "-------------------------------------------------------------\n"
            "# triangles %" PRId64 "\n\n", ntri) ;

    printf ("prep time:\n") ;
    for (int e = 0 ; e <= 1 ; e++)
    {
        if (e == 0) printf ("prep for all methods:\n") ;
        else printf ("extra prep for tri_dot:\n") ;

        printf (" #  | 0:S=tril(A)      | 1:S=triu(A)      |"
                " 2:sort inc       | 3: sort dec      |\n") ;
        printf ("thr |") ;
        for (int prep_method = 0 ; prep_method < NPREP ; prep_method++)
        {
            printf ("     time speedup |") ;
        }
        printf ("\n") ;
        FOR_ALL_THREADS (MAX_THREADS)
        {
            printf ("%3d |", nthreads) ;
            for (int prep_method = 0 ; prep_method < NPREP ; prep_method++)
            {
                printf (" %10.5f %5.1f |",
                    T_prep [e][prep_method][nthreads],
                    T_prep [e][prep_method][1] /
                    T_prep [e][prep_method][nthreads]) ;
            }
            printf ("\n") ;
        }
        printf ("\n") ;
    }

    // for later MATLAB analysis:
    FILE *fm = fopen ("tri_results.m", "a") ;
    fprintf (fm, "\n%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n") ;
    fprintf (fm, "id = id + 1 ;\n") ;
    fprintf (fm, "N (id) = %ld ;\n", n) ;
    fprintf (fm, "Nedges (id) = %ld ;\n", nedges) ;
    fprintf (fm, "Ntri (id) = %ld ;\n", ntri) ;
    fprintf (fm, "T_prep = nan (2, %d, %d) ; \n", NPREP, MAX_THREADS) ;
    fprintf (fm, "%% prep: 1:L, 2:U, 3:Lperm, 4:Uperm\n") ;
    fprintf (fm, "%% T_prep (1, prep_method, nthreads): for all tri methods\n");
    fprintf (fm, "%% T_prep (2, prep_method, nthreads): just for tri_dot\n") ;
    fprintf (fm, "%% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple\n") ;
    fprintf (fm, "%% Time (tri_method, prep_method, nthreads)\n") ;
    for (int e = 0 ; e <= 1 ; e++)
    {
        FOR_ALL_THREADS (MAX_THREADS)
        {
            for (int prep_method = 0 ; prep_method < NPREP ; prep_method++)
            {
                fprintf (fm, "T_prep (%d,%d,%d) = %12.6g ;\n",
                        1+e, 1+prep_method, nthreads,
                        T_prep [e][prep_method][nthreads]) ;
            }
        }
    }
    fprintf (fm, "Tprep {id} = T_prep ;\n") ;
    fprintf (fm, "Time = nan (2, %d, %d) ; \n", NPREP, MAX_THREADS) ;
    for (int method = 0 ; method < NMETHODS ; method++)
    {
        FOR_ALL_THREADS (MAX_THREADS)
        {
            for (int prep_method = 0 ; prep_method < NPREP ; prep_method++)
            {
                if (method < 4 || nthreads == 1)
                    fprintf (fm, "Time (%d,%d,%d) = %12.6g ;\n",
                        1+method, 1+prep_method, nthreads,
                        Time [method][prep_method][nthreads]) ;
            }
        }
    }
    fprintf (fm, "T {id} = Time ;\n\n") ;
    fprintf (fm, "File {id} = filetrim (file) ;\n\n") ;
    fclose (fm) ;

    for (int just_tri = 0 ; just_tri <= 1 ; just_tri++)
    {
        if (just_tri)
        {
            printf (
            "\n----------------------------------------\n"
            "performance excluding prep time"
            "\n----------------------------------------\n") ;
        }
        else
        {
            printf (
            "\n----------------------------------------\n"
            "performance including prep time"
            "\n----------------------------------------\n") ;
        }
        for (int method = 0 ; method < NMETHODS ; method++)
        {
            printf ("\nmethod: ") ;
            switch (method)
            {
                case 0: printf ("tri_mark\n") ;
                    max_threads = MAX_THREADS ;
                    break ;

                case 1: printf ("tri_bit\n") ;
                    max_threads = MAX_THREADS ;
                    break ;

                case 2:
                    printf ("tri_dot\n") ;
                    max_threads = MAX_THREADS ;
                    break ;

                case 3:
                    printf ("tri_logmark\n") ;
                    max_threads = MAX_THREADS ;
                    break ;

                case 4:
                    printf ("tri_simple\n") ;
                    max_threads = 1 ;
                    break ;

            }

            printf (" #  |") ;
            for (int prep_method = 0 ; prep_method < NPREP ; prep_method++)
            {
                printf ("prep:%d                 |", prep_method) ;
            }
            printf ("\n") ;

            printf ("thr |") ;
            for (int prep_method = 0 ; prep_method < NPREP ; prep_method++)
            {
                printf ("   time    rate speedup|") ;
            }
            printf ("\n") ;

            // find the best sequential method
            for (int prep_method = 0 ; prep_method < NPREP ; prep_method++)
            {

                // sequential time
                double t1 = Time [method][prep_method][1] ;
                if (!just_tri)
                {
                    t1 += T_prep [0][prep_method][1] ;
                    if (method == 2) t1 += T_prep [1][prep_method][1] ;
                }
                if (t1 < tbest1)
                {
                    prep1_best = prep_method ;
                    best1_method = method ;
                    tbest1 = t1 ;
                }
            }

            // print results and find the best parallel method
            FOR_ALL_THREADS (max_threads)
            {
                printf ("%3d |", nthreads) ;
                for (int prep_method = 0 ; prep_method < NPREP ; prep_method++)
                {
                    int64_t nt = Ntri [method][prep_method][nthreads] ;

                    // parallel time
                    double t = Time [method][prep_method][nthreads] ;
                    if (!just_tri)
                    {
                        t += T_prep [0][prep_method][nthreads] ;
                        if (method == 2)
                        {
                            t += T_prep [1][prep_method][nthreads];
                        }
                    }

                    // sequential time
                    double t1 = Time [method][prep_method][1] ;
                    if (!just_tri)
                    {
                        t1 += T_prep [0][prep_method][nthreads] ;
                        if (method == 2)
                        {
                            t1 += T_prep [1][prep_method][nthreads] ;
                        }
                    }

                    double speedup = t1 / t ;

                    if (ntri == -1) ntri = nt ;
                    if (ntri != nt) {
                        printf ("nt is %g\n", (double) nt) ;
                        printf ("ntri is %g\n", (double) ntri) ;
                        /* printf ("error!\n") ; exit (1) ;  */
                        }
                    printf ("%8.3f %7.2f %5.1f |", t, 1e-6 * nedges / t,
                        speedup) ;
                    if (t < tbest)
                    {
                        nthreads_best = nthreads ;
                        prep_best = prep_method ;
                        best_method = method ;
                        speedup_method = speedup ;
                        tbest = t ;
                    }
                }
                printf ("\n") ;
            }
        }

        printf ("\n") ;

        printf ("best 1-thread: ") ;
        switch (best1_method)
        {
            case 0: printf ("tri_mark   ") ; break ;
            case 1: printf ("tri_bit    ") ; break ;
            case 2: printf ("tri_dot    ") ; break ;
            case 3: printf ("tri_logmark") ; break ;
            case 4: printf ("tri_simple ") ; break ;
        }
        printf (" threads: %3d prep: %d rate %8.2f              ",
            1, prep1_best, 1e-6 * nedges / tbest1) ;
        if (just_tri)
        {
            printf (" (excl prep)\n") ;
        }
        else
        {
            printf (" (with prep)\n") ;
        }

        fprintf (stderr, "best 1-thread: ") ;
        switch (best1_method)
        {
            case 0: fprintf (stderr, "tri_mark   ") ; break ;
            case 1: fprintf (stderr, "tri_bit    ") ; break ;
            case 2: fprintf (stderr, "tri_dot    ") ; break ;
            case 3: fprintf (stderr, "tri_logmark") ; break ;
            case 4: fprintf (stderr, "tri_simple ") ; break ;
        }
        fprintf (stderr, " threads: %3d prep: %d rate %8.2f              ",
            1, prep1_best, 1e-6 * nedges / tbest1) ;
        if (just_tri)
        {
            fprintf (stderr, " (excl prep)\n") ;
        }
        else
        {
            fprintf (stderr, " (with prep)\n") ;
        }

        printf ("best parallel: ") ;
        switch (best_method)
        {
            case 0: printf ("tri_mark   ") ; break ;
            case 1: printf ("tri_bit    ") ; break ;
            case 2: printf ("tri_dot    ") ; break ;
            case 3: printf ("tri_logmark") ; break ;
            case 4: printf ("tri_simple ") ; break ;
        }
        printf (" threads: %3d prep: %d rate %8.2f speedup %5.1f",
            nthreads_best, prep_best, 1e-6 * nedges / tbest, speedup_method) ;
        if (just_tri)
        {
            printf (" (excl prep)\n") ;
        }
        else
        {
            printf (" (with prep)\n") ;
        }

        fprintf (stderr, "best parallel: ") ;
        switch (best_method)
        {
            case 0: fprintf (stderr, "tri_mark   ") ; break ;
            case 1: fprintf (stderr, "tri_bit    ") ; break ;
            case 2: fprintf (stderr, "tri_dot    ") ; break ;
            case 3: fprintf (stderr, "tri_logmark") ; break ;
            case 4: fprintf (stderr, "tri_simple ") ; break ;
        }
        fprintf (stderr, " threads: %3d prep: %d rate %8.2f speedup %5.1f",
            nthreads_best, prep_best, 1e-6 * nedges / tbest, speedup_method) ;
        if (just_tri)
        {
            fprintf (stderr, " (excl prep)\n") ;
        }
        else
        {
            fprintf (stderr, " (with prep)\n") ;
        }
    }
}

