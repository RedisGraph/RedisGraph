//------------------------------------------------------------------------------
// GraphBLAS/Demo/Program/reduce_demo: reduce a matrix to a scalar
//------------------------------------------------------------------------------

#include "GraphBLAS.h"

// #define N 65536
   #define N 16384

int main (void)
{

    #if defined ( _OPENMP )
    double t0 = omp_get_wtime ( ) ;
    #endif

    // start GraphBLAS
    GrB_init (GrB_NONBLOCKING) ;
    printf ("demo: reduce a matrix to a scalar\n") ;

    int nthreads_max ;
    GxB_get (GxB_NTHREADS, &nthreads_max) ;
    printf ("# of threads: %d\n", nthreads_max) ;

    #if defined ( _OPENMP )
    t0 = omp_get_wtime ( ) - t0 ;
    printf ("GPU warmup time: %g\n", t0) ;
    t0 = omp_get_wtime ( ) ;
    #endif

    GrB_Index nrows = N ;
    GrB_Index ncols = N ;
    GrB_Matrix A ;
    GrB_Matrix_new (&A, GrB_INT64, nrows, ncols) ;

    GrB_Index *I = malloc (nrows * ncols * sizeof (GrB_Index)) ;
    GrB_Index *J = malloc (nrows * ncols * sizeof (GrB_Index)) ;
    int64_t   *X = malloc (nrows * ncols * sizeof (int64_t)) ;

    #pragma omp parallel for num_threads(nthreads_max) collapse(2) \
        schedule(static)
    for (int64_t i = 0 ; i < nrows ; i++)
    {
        for (int64_t j = 0 ; j < ncols ; j++)
        {
            int64_t k = i * N + j ;
            // int x = (int) (rand ( ) & 0xFF) ;
            int x = (int) (k & 0xFF) ;
            I [k] = i ;
            J [k] = j ;
            X [k] = x ;
        }
    }

    GrB_Index nvals = N*N ;
    GrB_Matrix_build (A, I, J, X, nvals, GrB_PLUS_INT64) ;

    free (I) ;
    free (J) ;
    free (X) ;

    #if defined ( _OPENMP )
    t0 = omp_get_wtime ( ) - t0 ;
    printf ("time to create matrix: %g\n", t0) ;
    #endif

    GrB_Index result ;

    double t1 ;

    printf ("\nreduce to a scalar:\n") ;

    for (int nthreads = 1 ; nthreads <= nthreads_max ; nthreads++)
    {
        GxB_set (GxB_NTHREADS, nthreads) ;
        #if defined ( _OPENMP )
        double t = omp_get_wtime ( ) ;
        #endif
        GrB_reduce (&result, NULL, GxB_PLUS_INT64_MONOID, A, NULL) ;
        #if defined ( _OPENMP )
        t = omp_get_wtime ( ) - t ;
        if (nthreads == 1) t1 = t ;
        printf ("nthreads %3d time: %12.6f speedup %8.2f\n", 
            nthreads, t, t1/t) ;
        #endif
    }

    printf ("result %"PRId64"\n", result) ;

    // free everyting
    GrB_free (&A) ;
    GrB_finalize ( ) ;
}

