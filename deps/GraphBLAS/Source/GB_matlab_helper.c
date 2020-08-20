//------------------------------------------------------------------------------
// GB_matlab_helper.c: helper functions for MATLAB interface
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// These functions are only used by the MATLAB interface for
// SuiteSparse:GraphBLAS.

#include "GB_matlab_helper.h"

//------------------------------------------------------------------------------
// GB_NTHREADS: determine the number of threads to use
//------------------------------------------------------------------------------

#define GB_NTHREADS(work)                                       \
    int nthreads_max = GB_Global_nthreads_max_get ( ) ;         \
    double chunk = GB_Global_chunk_get ( ) ;                    \
    int nthreads = GB_nthreads (work, chunk, nthreads_max) ;

//------------------------------------------------------------------------------
// GB_ALLOCATE_WORK: allocate per-thread workspace
//------------------------------------------------------------------------------

#define GB_ALLOCATE_WORK(work_type)                             \
    work_type *Work ;                                           \
    GB_MALLOC_MEMORY (Work, nthreads, sizeof (work_type)) ;     \
    if (Work == NULL) return (false) ;

//------------------------------------------------------------------------------
// GB_FREE_WORK: free per-thread workspace
//------------------------------------------------------------------------------

#define GB_FREE_WORK(work_type)                                 \
    GB_FREE_MEMORY (Work, nthreads, sizeof (work_type)) ;

//------------------------------------------------------------------------------
// GB_matlab_helper1: convert 0-based indices to 1-based for gbextracttuples
//------------------------------------------------------------------------------

void GB_matlab_helper1              // convert zero-based indices to one-based
(
    double *GB_RESTRICT I_double,   // output array
    const GrB_Index *GB_RESTRICT I, // input array
    int64_t nvals                   // size of input and output arrays
)
{

    GB_NTHREADS (nvals) ;

    int64_t k ;
    #pragma omp parallel for num_threads(nthreads) schedule(static)
    for (k = 0 ; k < nvals ; k++)
    {
        I_double [k] = (double) (I [k] + 1) ;
    }
}

//------------------------------------------------------------------------------
// GB_matlab_helper1i: convert 0-based indices to 1-based for gbextracttuples
//------------------------------------------------------------------------------

void GB_matlab_helper1i             // convert zero-based indices to one-based
(
    int64_t *GB_RESTRICT I,         // input/output array
    int64_t nvals                   // size of input/output array
)
{

    GB_NTHREADS (nvals) ;

    int64_t k ;
    #pragma omp parallel for num_threads(nthreads) schedule(static)
    for (k = 0 ; k < nvals ; k++)
    {
        I [k] ++ ;
    }
}

//------------------------------------------------------------------------------
// GB_matlab_helper2: create structure for dense matrix for gb_get_shallow
//------------------------------------------------------------------------------

void GB_matlab_helper2              // fill Xp and Xi for a dense matrix
(
    GrB_Index *GB_RESTRICT Xp,      // size ncols+1
    GrB_Index *GB_RESTRICT Xi,      // size nrows*ncols
    int64_t ncols,
    int64_t nrows
)
{

    GB_NTHREADS (ncols) ;

    int64_t j ;
    #pragma omp parallel for num_threads(nthreads) schedule(static)
    for (j = 0 ; j <= ncols ; j++)
    {
        Xp [j] = j * nrows ;
    }

    double work = ((double) ncols) * ((double) nrows) ;
    nthreads = GB_nthreads (work, chunk, nthreads_max) ;

    int64_t nel = nrows * ncols ;
    int64_t k ;
    #pragma omp parallel for num_threads(nthreads) schedule(static)
    for (k = 0 ; k < nel ; k++)
    {
        int64_t i = k % nrows ;
        Xi [k] = i ;
    }
}

//------------------------------------------------------------------------------
// GB_matlab_helper3: convert 1-based indices to 0-based for gb_mxarray_to_list
//------------------------------------------------------------------------------

bool GB_matlab_helper3              // return true if OK, false on error
(
    int64_t *GB_RESTRICT List,      // size len, output array
    const double *GB_RESTRICT List_double, // size len, input array
    int64_t len,
    int64_t *List_max               // also compute the max entry in the list
)
{

    GB_NTHREADS (len) ;

    bool ok = true ;
    int64_t listmax = -1 ;

    GB_ALLOCATE_WORK (int64_t) ;

    int tid ;
    #pragma omp parallel for num_threads(nthreads) schedule(static)
    for (tid = 0 ; tid < nthreads ; tid++)
    {
        bool my_ok = true ;
        int64_t k1, k2, my_listmax = -1 ;
        GB_PARTITION (k1, k2, len, tid, nthreads) ;
        for (int64_t k = k1 ; k < k2 ; k++)
        {
            double x = List_double [k] ;
            int64_t i = (int64_t) x ;
            my_ok = my_ok && (x == (double) i) ;
            my_listmax = GB_IMAX (my_listmax, i) ;
            List [k] = i - 1 ;
        }
        // rather than create a separate per-thread boolean workspace, just
        // use a sentinal value of INT64_MIN if non-integer indices appear
        // in List_double.
        Work [tid] = my_ok ? my_listmax : INT64_MIN ;
    }

    // wrapup
    for (tid = 0 ; tid < nthreads ; tid++)
    {
        listmax = GB_IMAX (listmax, Work [tid]) ;
        ok = ok && (Work [tid] != INT64_MIN) ;
    }

    GB_FREE_WORK (int64_t) ;

    (*List_max) = listmax ;
    return (ok) ;
}

//------------------------------------------------------------------------------
// GB_matlab_helper3i: convert 1-based indices to 0-based for gb_mxarray_to_list
//------------------------------------------------------------------------------

bool GB_matlab_helper3i             // return true if OK, false on error
(
    int64_t *GB_RESTRICT List,      // size len, output array
    const int64_t *GB_RESTRICT List_int64, // size len, input array
    int64_t len,
    int64_t *List_max               // also compute the max entry in the list
)
{

    GB_NTHREADS (len) ;

    int64_t listmax = -1 ;

    GB_ALLOCATE_WORK (int64_t) ;

    int tid ;
    #pragma omp parallel for num_threads(nthreads) schedule(static)
    for (tid = 0 ; tid < nthreads ; tid++)
    {
        int64_t k1, k2, my_listmax = -1 ;
        GB_PARTITION (k1, k2, len, tid, nthreads) ;
        for (int64_t k = k1 ; k < k2 ; k++)
        {
            int64_t i = List_int64 [k] ;
            my_listmax = GB_IMAX (my_listmax, i) ;
            List [k] = i - 1 ;
        }
        Work [tid] = my_listmax ;
    }

    // wrapup
    for (tid = 0 ; tid < nthreads ; tid++)
    {
        listmax = GB_IMAX (listmax, Work [tid]) ;
    }

    GB_FREE_WORK (int64_t) ;

    (*List_max) = listmax ;
    return (true) ;
}

//------------------------------------------------------------------------------
// GB_matlab_helper4: find the max entry in an index list for gbbuild
//------------------------------------------------------------------------------

bool GB_matlab_helper4              // return true if OK, false on error
(
    const GrB_Index *GB_RESTRICT I, // array of size len
    const int64_t len,
    GrB_Index *List_max             // find max (I) + 1
)
{

    GB_NTHREADS (len) ;

    GrB_Index listmax = 0 ;

    GB_ALLOCATE_WORK (GrB_Index) ;

    int tid ;
    #pragma omp parallel for num_threads(nthreads) schedule(static)
    for (tid = 0 ; tid < nthreads ; tid++)
    {
        int64_t k1, k2 ;
        GrB_Index my_listmax = 0 ;
        GB_PARTITION (k1, k2, len, tid, nthreads) ;
        for (int64_t k = k1 ; k < k2 ; k++)
        {
            my_listmax = GB_IMAX (my_listmax, I [k]) ;
        }
        Work [tid] = my_listmax ;
    }

    // wrapup
    for (tid = 0 ; tid < nthreads ; tid++)
    {
        listmax = GB_IMAX (listmax, Work [tid]) ;
    }

    GB_FREE_WORK (GrB_Index) ;

    if (len > 0) listmax++ ;
    (*List_max) = listmax ;
    return (true) ;
}

//------------------------------------------------------------------------------
// GB_matlab_helper5: construct pattern of S for gblogassign
//------------------------------------------------------------------------------

void GB_matlab_helper5              // construct pattern of S
(
    GrB_Index *GB_RESTRICT Si,         // array of size anz
    GrB_Index *GB_RESTRICT Sj,         // array of size anz
    const GrB_Index *GB_RESTRICT Mi,   // array of size mnz
    const GrB_Index *GB_RESTRICT Mj,   // array of size mnz
    GrB_Index *GB_RESTRICT Ai,         // array of size anz
    const GrB_Index anz
)
{

    GB_NTHREADS (anz) ;

    int64_t k ;
    #pragma omp parallel for num_threads(nthreads) schedule(static)
    for (k = 0 ; k < anz ; k++)
    {
        Si [k] = Mi [Ai [k]] ;
        Sj [k] = Mj [Ai [k]] ;
    }
}

//------------------------------------------------------------------------------
// GB_matlab_helper6: set bool array to all true gblogextract
//------------------------------------------------------------------------------

void GB_matlab_helper6              // set Gbool to all true
(
    bool *GB_RESTRICT Gbool,        // array of size gnvals
    const GrB_Index gnvals
)
{

    GB_NTHREADS (gnvals) ;

    int64_t k ;
    #pragma omp parallel for num_threads(nthreads) schedule(static)
    for (k = 0 ; k < gnvals ; k++)
    {
        Gbool [k] = true ;
    }
}

//------------------------------------------------------------------------------
// GB_matlab_helper7: Kx = uint64 (0:mnz-1), for gblogextract
//------------------------------------------------------------------------------

void GB_matlab_helper7              // Kx = uint64 (0:mnz-1)
(
    uint64_t *GB_RESTRICT Kx,       // array of size mnz
    const GrB_Index mnz
)
{

    GB_NTHREADS (mnz) ;

    int64_t k ;
    #pragma omp parallel for num_threads(nthreads) schedule(static)
    for (k = 0 ; k < mnz ; k++)
    {
        Kx [k] = k ;
    }
}

//------------------------------------------------------------------------------
// GB_matlab_helper8: expand a scalar into an array for gbbuild
//------------------------------------------------------------------------------

void GB_matlab_helper8
(
    GB_void *C,         // output array of size nvals * s
    GB_void *A,         // input scalar of size s
    GrB_Index nvals,    // size of C
    size_t s            // size of each scalar
)
{

    GB_NTHREADS (nvals) ;

    int64_t k ;
    #pragma omp parallel for num_threads(nthreads) schedule(static)
    for (k = 0 ; k < nvals ; k++)
    {
        // C [k] = A [0]
        memcpy (C + k * s, A, s) ;
    }
}

//------------------------------------------------------------------------------
// GB_matlab_helper9: compute the degree of each vector
//------------------------------------------------------------------------------

bool GB_matlab_helper9  // true if successful, false if out of memory
(
    GrB_Matrix A,       // input matrix
    int64_t **degree,   // degree of each vector, size nvec
    GrB_Index **list,   // list of non-empty vectors
    GrB_Index *nvec     // # of non-empty vectors
)
{
    int64_t anvec = A->nvec ;
    GB_NTHREADS (anvec) ;

    uint64_t *List = NULL ;
    int64_t  *Degree = NULL ;
    GB_MALLOC_MEMORY (List,   anvec, sizeof (int64_t)) ;
    GB_MALLOC_MEMORY (Degree, anvec, sizeof (int64_t)) ;

    if (List == NULL || Degree == NULL)
    {
        GB_FREE_MEMORY (List,   anvec, sizeof (int64_t)) ;
        GB_FREE_MEMORY (Degree, anvec, sizeof (int64_t)) ;
        return (false) ;
    }

    int64_t *Ah = A->h ;
    int64_t *Ap = A->p ;

    int64_t k ;
    #pragma omp parallel for num_threads(nthreads) schedule(static)
    for (k = 0 ; k < anvec ; k++)
    {
        List [k] = (Ah == NULL) ? k : Ah [k] ;
        Degree [k] = Ap [k+1] - Ap [k] ;
    }

    // return result
    (*degree) = Degree ;
    (*list) = List ;
    (*nvec) = anvec ;
    return (true) ;
}

//------------------------------------------------------------------------------
// GB_matlab_helper10: compute norm (x-y,p) of two dense FP32 or FP64 vectors
//------------------------------------------------------------------------------

// p can be:

//      0 or 2:     2-norm, sqrt (sum ((x-y).^2))
//      1:          1-norm, sum (abs (x-y))
//      INT64_MAX   inf-norm, max (abs (x-y))
//      INT64_MIN   (-inf)-norm, min (abs (x-y))
//      other:      p-norm not yet computed

double GB_matlab_helper10       // norm (x-y,p), or -1 on error
(
    GB_void *x_arg,             // float or double, depending on type parameter
    GB_void *y_arg,             // same type as x, treat as zero if NULL
    GrB_Type type,              // GrB_FP32 or GrB_FP64
    int64_t p,                  // 0, 1, 2, INT64_MIN, or INT64_MAX
    GrB_Index n
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    if (!(type == GrB_FP32 || type == GrB_FP64))
    {
        // type of x and y must be GrB_FP32 or GrB_FP64
        return ((double) -1) ;
    }

    if (n == 0)
    {
        return ((double) 0) ;
    }

    //--------------------------------------------------------------------------
    // allocate workspace and determine # of threads to use
    //--------------------------------------------------------------------------

    GB_NTHREADS (n) ;
    GB_ALLOCATE_WORK (double) ;

    //--------------------------------------------------------------------------
    // each thread computes its partial norm
    //--------------------------------------------------------------------------

    int tid ;
    #pragma omp parallel for num_threads(nthreads) schedule(static)
    for (tid = 0 ; tid < nthreads ; tid++)
    {
        int64_t k1, k2 ;
        GB_PARTITION (k1, k2, n, tid, nthreads) ;

        if (type == GrB_FP32)
        {

            //------------------------------------------------------------------
            // FP32 case
            //------------------------------------------------------------------

            float my_s = 0 ;
            const float *x = (float *) x_arg ;
            const float *y = (float *) y_arg ;
            switch (p)
            {
                case 0:     // Frobenius norm
                case 2:     // 2-norm: sqrt of sum of (x-y).^2
                {
                    if (y == NULL)
                    {
                        for (int64_t k = k1 ; k < k2 ; k++)
                        {
                            float t = x [k] ;
                            my_s += (t*t) ;
                        }
                    }
                    else
                    {
                        for (int64_t k = k1 ; k < k2 ; k++)
                        {
                            float t = (x [k] - y [k]) ;
                            my_s += (t*t) ;
                        }
                    }
                }
                break ;

                case 1:     // 1-norm: sum (abs (x-y))
                {
                    if (y == NULL)
                    {
                        for (int64_t k = k1 ; k < k2 ; k++)
                        {
                            my_s += fabsf (x [k]) ;
                        }
                    }
                    else
                    {
                        for (int64_t k = k1 ; k < k2 ; k++)
                        {
                            my_s += fabsf (x [k] - y [k]) ;
                        }
                    }
                }
                break ;

                case INT64_MAX:     // inf-norm: max (abs (x-y))
                {
                    if (y == NULL)
                    {
                        for (int64_t k = k1 ; k < k2 ; k++)
                        {
                            my_s = fmaxf (my_s, fabsf (x [k])) ;
                        }
                    }
                    else
                    {
                        for (int64_t k = k1 ; k < k2 ; k++)
                        {
                            my_s = fmaxf (my_s, fabsf (x [k] - y [k])) ;
                        }
                    }
                }
                break ;

                case INT64_MIN:     // (-inf)-norm: min (abs (x-y))
                {
                    my_s = INFINITY ;
                    if (y == NULL)
                    {
                        for (int64_t k = k1 ; k < k2 ; k++)
                        {
                            my_s = fminf (my_s, fabsf (x [k])) ;
                        }
                    }
                    else
                    {
                        for (int64_t k = k1 ; k < k2 ; k++)
                        {
                            my_s = fminf (my_s, fabsf (x [k] - y [k])) ;
                        }
                    }
                }
                break ;

                default: ;  // p-norm not yet supported
            }
            Work [tid] = (double) my_s ;

        }
        else
        {

            //------------------------------------------------------------------
            // FP64 case
            //------------------------------------------------------------------

            double my_s = 0 ;
            const double *x = (double *) x_arg ;
            const double *y = (double *) y_arg ;
            switch (p)
            {
                case 0:     // Frobenius norm
                case 2:     // 2-norm: sqrt of sum of (x-y).^2
                {
                    if (y == NULL)
                    {
                        for (int64_t k = k1 ; k < k2 ; k++)
                        {
                            double t = x [k] ;
                            my_s += (t*t) ;
                        }
                    }
                    else
                    {
                        for (int64_t k = k1 ; k < k2 ; k++)
                        {
                            double t = (x [k] - y [k]) ;
                            my_s += (t*t) ;
                        }
                    }
                }
                break ;

                case 1:     // 1-norm: sum (abs (x-y))
                {
                    if (y == NULL)
                    {
                        for (int64_t k = k1 ; k < k2 ; k++)
                        {
                            my_s += fabs (x [k]) ;
                        }
                    }
                    else
                    {
                        for (int64_t k = k1 ; k < k2 ; k++)
                        {
                            my_s += fabs (x [k] - y [k]) ;
                        }
                    }
                }
                break ;

                case INT64_MAX:     // inf-norm: max (abs (x-y))
                {
                    if (y == NULL)
                    {
                        for (int64_t k = k1 ; k < k2 ; k++)
                        {
                            my_s = fmax (my_s, fabs (x [k])) ;
                        }
                    }
                    else
                    {
                        for (int64_t k = k1 ; k < k2 ; k++)
                        {
                            my_s = fmax (my_s, fabs (x [k] - y [k])) ;
                        }
                    }
                }
                break ;

                case INT64_MIN:     // (-inf)-norm: min (abs (x-y))
                {
                    my_s = INFINITY ;
                    if (y == NULL)
                    {
                        for (int64_t k = k1 ; k < k2 ; k++)
                        {
                            my_s = fmin (my_s, fabs (x [k])) ;
                        }
                    }
                    else
                    {
                        for (int64_t k = k1 ; k < k2 ; k++)
                        {
                            my_s = fmin (my_s, fabs (x [k] - y [k])) ;
                        }
                    }
                }
                break ;

                default: ;  // p-norm not yet supported
            }

            Work [tid] = my_s ;
        }
    }

    //--------------------------------------------------------------------------
    // combine results of each thread
    //--------------------------------------------------------------------------

    double s = 0 ;
    switch (p)
    {
        case 0:     // Frobenius norm
        case 2:     // 2-norm: sqrt of sum of (x-y).^2
        {
            for (int64_t tid = 0 ; tid < nthreads ; tid++)
            {
                s += Work [tid] ;
            }
            s = sqrt (s) ;
        }
        break ;

        case 1:     // 1-norm: sum (abs (x-y))
        {
            for (int64_t tid = 0 ; tid < nthreads ; tid++)
            {
                s += Work [tid] ;
            }
        }
        break ;

        case INT64_MAX:     // inf-norm: max (abs (x-y))
        {
            for (int64_t tid = 0 ; tid < nthreads ; tid++)
            {
                s = fmax (s, Work [tid]) ;
            }
        }
        break ;

        case INT64_MIN:     // (-inf)-norm: min (abs (x-y))
        {
            s = Work [0] ;
            for (int64_t tid = 1 ; tid < nthreads ; tid++)
            {
                s = fmin (s, Work [tid]) ;
            }
        }
        break ;

        default:    // p-norm not yet supported
            s = -1 ;
    }

    //--------------------------------------------------------------------------
    // free workspace and return result
    //--------------------------------------------------------------------------

    GB_FREE_WORK (double) ;
    return (s) ;
}

