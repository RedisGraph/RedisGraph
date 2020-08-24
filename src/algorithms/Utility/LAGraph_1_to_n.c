#include "LAGraph_internal.h"

#define LAGRAPH_FREE_ALL    \
{                           \
    GrB_free (&v) ;         \
    LAGRAPH_FREE (I) ;      \
    LAGRAPH_FREE (X) ;      \
}

GrB_Info LAGraph_1_to_n     // create an integer vector v = 1:n
(
    GrB_Vector *v_handle,   // vector to create
    GrB_Index n             // size of vector to create
)
{

    GrB_Info info ;
    GrB_Vector v = NULL ;
    int nthreads = LAGraph_get_nthreads ( ) ;
    nthreads = LAGRAPH_MIN (n / 4096, nthreads) ;
    nthreads = LAGRAPH_MAX (nthreads, 1) ;

    // allocate workspace
    GrB_Index *I = LAGraph_malloc (n, sizeof (GrB_Index)) ;

    // create a 32-bit or 64-bit integer vector 1:n
    if (n > INT32_MAX)
    {
        int64_t *X = LAGraph_malloc (n, sizeof (int64_t)) ;
        if (I == NULL || X == NULL)
        {
            LAGRAPH_FREE_ALL ;
            return (GrB_OUT_OF_MEMORY) ;
        }
        #pragma omp parallel for num_threads(nthreads) schedule(static)
        for (int64_t k = 0 ; k < n ; k++)
        {
            I [k] = k ;
            X [k] = k+1 ;
        }
        LAGRAPH_OK (GrB_Vector_new (&v, GrB_INT64, n)) ;
        LAGRAPH_OK (GrB_Vector_build (v, I, X, n, GrB_PLUS_INT64)) ;
        LAGRAPH_FREE (X) ;
    }
    else
    {
        int32_t *X = LAGraph_malloc (n, sizeof (int32_t)) ;
        if (I == NULL || X == NULL)
        {
            LAGRAPH_FREE_ALL ;
            return (GrB_OUT_OF_MEMORY) ;
        }
        #pragma omp parallel for num_threads(nthreads) schedule(static)
        for (int64_t k = 0 ; k < n ; k++)
        {
            I [k] = k ;
            X [k] = k+1 ;
        }
        LAGRAPH_OK (GrB_Vector_new (&v, GrB_INT32, n)) ;
        LAGRAPH_OK (GrB_Vector_build (v, I, X, n, GrB_PLUS_INT32)) ;
        LAGRAPH_FREE (X) ;
    }
    LAGRAPH_FREE (I) ;

    // return result
    (*v_handle) = v ;
    return (GrB_SUCCESS) ;
}

