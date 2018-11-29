//------------------------------------------------------------------------------
// GraphBLAS/Demo/Program/openmp_demo: example of user multithreading
//------------------------------------------------------------------------------

// This demo uses OpenMP, and should work if GraphBLAS is compiled to
// use either OpenMP or pthreads to synchronize multiple user threadds.
// If OpenMP is not available, this program will work fine without it, in a
// single user thread, regardless of the thread mechanism used by GraphBLAS.

#include "GraphBLAS.h"

#ifdef _OPENMP
#include <omp.h>
#endif

#if defined __INTEL_COMPILER
#pragma warning (disable: 58 167 144 177 181 186 188 589 593 869 981 1418 1419 1572 1599 2259 2282 2557 2547 3280 )
#elif defined __GNUC__
#pragma GCC diagnostic ignored "-Wunknown-pragmas"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wincompatible-pointer-types"
#endif

#define NTHREADS 8
#define NTRIALS 10
#define N 6

#define OK(method)                                                  \
{                                                                   \
    GrB_Info info = method ;                                        \
    if (! (info == GrB_SUCCESS || info == GrB_NO_VALUE))            \
    {                                                               \
        printf ("Failure (id: %d, info: %d): %s\n",                 \
            id, info, GrB_error ( )) ;                              \
        /* return to caller (do not use inside critical section) */ \
        return (0) ;                                                \
    }                                                               \
}

//------------------------------------------------------------------------------
// worker
//------------------------------------------------------------------------------

int worker (GrB_Matrix *Ahandle, int id)
{

    printf ("\n================= worker %d starts:\n", id) ;
    fprintf (stderr, "worker %d\n", id) ;

    OK (GrB_Matrix_new (Ahandle, GrB_FP64, N, N)) ;
    GrB_Matrix A = *Ahandle ;

    // worker generates an intentional error message
    GrB_Matrix_setElement (A, 42, 1000+id, 1000+id) ;

    // print the intentional error generated when the worker started
    #pragma omp critical
    {
        // critical section
        printf ("\n----------------- worker %d intentional error:\n", id) ;
        printf ("%s\n", GrB_error ( )) ;
    }

    for (int hammer_hard = 0 ; hammer_hard < NTRIALS ; hammer_hard++)
    {
        for (int i = 0 ; i < N ; i++)
        {
            for (int j = 0 ; j < N ; j++)
            {
                double x = (i+1)*100000 + (j+1)*1000 + id ;
                OK (GrB_Matrix_setElement (A, x, i, j)) ;
            } 
        }

        // force completion
        GrB_Index nvals ;
        OK (GrB_Matrix_nvals (&nvals, A)) ;
    }

    // Printing is done in a critical section, just so it is not overly
    // jumbled.  Each matrix and error will print in a single body of text,
    // but the order of the matrices and errors printed will be out of order
    // because the critical section does not enforce the order that the
    // threads enter.

    GrB_Info info2 ;
    #pragma omp critical
    {
        // critical section
        printf ("\n----------------- worker %d is done:\n", id) ;
        info2 = GxB_print (A, GxB_SHORT) ;
    }
    OK (info2) ;

    // worker generates an intentional error message
    GrB_Matrix_setElement (A, 42, 1000+id, 1000+id) ;

    // print the intentional error generated when the worker started
    // It should be unchanged.
    #pragma omp critical
    {
        // critical section
        printf ("\n----------------- worker %d error should be same:\n", id) ;
        printf ("%s\n", GrB_error ( )) ;
    }
    return (0) ;
}

//------------------------------------------------------------------------------
// openmp_demo main program
//------------------------------------------------------------------------------

int main (int argc, char **argv)
{
    fprintf (stderr, "Demo: %s:\n", argv [0]) ;
    printf ("Demo: %s:\n", argv [0]) ;

    // initialize the mutex
    int id = -1 ;

    // start GraphBLAS
    OK (GrB_init (GrB_NONBLOCKING)) ;

    // Determine which user-threading model is being used.
    GxB_Thread_Model thread_safety ;
    GxB_get (GxB_THREAD_SAFETY, &thread_safety) ;
    printf ("GraphBLAS is using ") ;
    switch (thread_safety)
    {
        case GxB_THREAD_POSIX :
            printf ("a POSIX pthread mutex\n") ;
            break ;
        case GxB_THREAD_WINDOWS :
            printf ("a Windows CriticalSection\n") ;
            break ;
        case GxB_THREAD_ANSI :
            printf ("an ANSI C11 mtx_lock\n") ;
            break ;
        case GxB_THREAD_OPENMP :
            printf ("an OpenMP critical section\n") ;
            break ;
        default : // GxB_THREAD_NONE
            #ifdef _OPENMP
            printf ("(nothing! This will fail!)\n") ;
            #else
            printf ("nothing (OK since user program is single-threaded)\n") ;
            #endif
            break ;
    }
    printf ("to synchronize user threads.\n") ;

    #ifdef _OPENMP
    printf ("User threads in this program are OpenMP threads.\n") ;
    #else
    printf ("This user program is single threaded.\n") ;
    #endif

    GrB_Matrix Aarray [NTHREADS] ;

    // create the threads
    #pragma omp parallel for num_threads(NTHREADS) 
    for (int id = 0 ; id < NTHREADS ; id++)
    {
        worker (&Aarray [id], id) ;
    }

    // the master thread prints them again, and frees them
    for (int id = 0 ; id < NTHREADS ; id++)
    {
        GrB_Matrix A = Aarray [id] ;
        printf ("\n---- Master prints matrix %d\n", id) ;
        OK (GxB_print (A, GxB_SHORT)) ;
        GrB_free (&A) ;
    }

    // print an error message
    printf ("\n\n---- Master thread prints an error message:\n") ;
    GrB_Matrix_new (NULL, GrB_FP64, 1, 1) ;
    printf ("master %d : Error: %s\n", id, GrB_error ( )) ;

    // finish GraphBLAS
    GrB_finalize ( ) ;

    // finish OpenMP
    exit (0) ;
}

