//------------------------------------------------------------------------------
// GB_matlab_helper.c: helper functions for MATLAB interface
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// These functions are only used by the MATLAB interface for
// SuiteSparse:GraphBLAS.

#include "GB_matlab_helper.h"

// determine the number of threads to use
#define GB_NTHREADS(work)                                       \
    int nthreads_max = GB_Global_nthreads_max_get ( ) ;         \
    double chunk = GB_Global_chunk_get ( ) ;                    \
    int nthreads = GB_nthreads (work, chunk, nthreads_max) ;

//------------------------------------------------------------------------------
// GB_matlab_helper1: convert 0-based indices to 1-based
//------------------------------------------------------------------------------

void GB_matlab_helper1              // convert zero-based indices to one-based
(
    double *restrict I_double,      // output array
    const GrB_Index *restrict I,    // input array
    int64_t nvals                   // size of input and output arrays
)
{

    GB_NTHREADS (nvals) ;

    #pragma omp parallel for num_threads(nthreads) schedule(static)
    for (int64_t k = 0 ; k < nvals ; k++)
    {
        I_double [k] = (double) (I [k] + 1) ;
    }
}

//------------------------------------------------------------------------------
// GB_matlab_helper1i: convert 0-based indices to 1-based
//------------------------------------------------------------------------------

void GB_matlab_helper1i             // convert zero-based indices to one-based
(
    int64_t *restrict I,            // input/output array
    int64_t nvals                   // size of input/output array
)
{

    GB_NTHREADS (nvals) ;

    #pragma omp parallel for num_threads(nthreads) schedule(static)
    for (int64_t k = 0 ; k < nvals ; k++)
    {
        I [k] ++ ;
    }
}

//------------------------------------------------------------------------------
// GB_matlab_helper2: create structure for dense matrix
//------------------------------------------------------------------------------

void GB_matlab_helper2              // fill Xp and Xi for a dense matrix
(
    GrB_Index *restrict Xp,         // size ncols+1
    GrB_Index *restrict Xi,         // size nrows*ncols
    int64_t ncols,
    int64_t nrows
)
{

    GB_NTHREADS (ncols) ;

    #pragma omp parallel for num_threads(nthreads) schedule(static)
    for (int64_t j = 0 ; j <= ncols ; j++)
    {
        Xp [j] = j * nrows ;
    }

    double work = ((double) ncols) * ((double) nrows) ;
    nthreads = GB_nthreads (work, chunk, nthreads_max) ;

    #pragma omp parallel for num_threads(nthreads) schedule(static) \
        collapse(2)
    for (int64_t j = 0 ; j < ncols ; j++)
    {
        for (int64_t i = 0 ; i < nrows ; i++)
        {
            Xi [j * nrows + i] = i ;
        }
    }
}

//------------------------------------------------------------------------------
// GB_matlab_helper3: convert 1-based indices to 0-based
//------------------------------------------------------------------------------

bool GB_matlab_helper3              // return true if OK, false on error
(
    int64_t *restrict List,         // size len, output array
    const double *restrict List_double, // size len, input array
    int64_t len,
    int64_t *List_max               // also compute the max entry in the list
)
{

    GB_NTHREADS (len) ;

    bool ok = true ;
    int64_t listmax = -1 ;

    #pragma omp parallel for num_threads(nthreads) schedule(static) \
        reduction(&&:ok) reduction(max:listmax)
    for (int64_t k = 0 ; k < len ; k++)
    {
        double x = List_double [k] ;
        int64_t i = (int64_t) x ;
        ok = ok && (x == (double) i) ;
        listmax = GB_IMAX (listmax, i) ;
        List [k] = i - 1 ;
    }

    (*List_max) = listmax ;
    return (ok) ;
}

//------------------------------------------------------------------------------
// GB_matlab_helper3i: convert 1-based indices to 0-based
//------------------------------------------------------------------------------

void GB_matlab_helper3i
(
    int64_t *restrict List,         // size len, output array
    const int64_t *restrict List_int64, // size len, input array
    int64_t len,
    int64_t *List_max               // also compute the max entry in the list
)
{

    GB_NTHREADS (len) ;

    int64_t listmax = -1 ;

    #pragma omp parallel for num_threads(nthreads) schedule(static) \
        reduction(max:listmax)
    for (int64_t k = 0 ; k < len ; k++)
    {
        int64_t i = List_int64 [k] ;
        listmax = GB_IMAX (listmax, i) ;
        List [k] = i - 1 ;
    }

    (*List_max) = listmax ;
}

//------------------------------------------------------------------------------
// GB_matlab_helper4: find the max entry in an index list
//------------------------------------------------------------------------------

int64_t GB_matlab_helper4           // find max (I) + 1
(
    const GrB_Index *restrict I,    // array of size len
    const int64_t len
)
{

    GB_NTHREADS (len) ;

    GrB_Index imax = 0 ;
    #pragma omp parallel for num_threads(nthreads) schedule(static) \
        reduction(max:imax)
    for (int64_t k = 0 ; k < len ; k++)
    {
        imax = GB_IMAX (imax, I [k]) ;
    }
    if (len > 0) imax++ ;
    return (imax) ;
}

//------------------------------------------------------------------------------
// GB_matlab_helper5: construct pattern of S for gblogassign
//------------------------------------------------------------------------------

void GB_matlab_helper5              // construct pattern of S
(
    GrB_Index *restrict Si,         // array of size anz
    GrB_Index *restrict Sj,         // array of size anz
    const GrB_Index *restrict Mi,   // array of size mnz
    const GrB_Index *restrict Mj,   // array of size mnz
    GrB_Index *restrict Ai,         // array of size anz
    const GrB_Index anz
)
{

    GB_NTHREADS (anz) ;

    #pragma omp parallel for num_threads(nthreads) schedule(static)
    for (int64_t k = 0 ; k < anz ; k++)
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
    bool *restrict Gbool,           // array of size gnvals
    const GrB_Index gnvals
)
{

    GB_NTHREADS (gnvals) ;

    #pragma omp parallel for num_threads(nthreads) schedule(static)
    for (int64_t k = 0 ; k < gnvals ; k++)
    {
        Gbool [k] = true ;
    }
}

//------------------------------------------------------------------------------
// GB_matlab_helper7: Kx = uint64 (0:mnz-1), for gblogextract
//------------------------------------------------------------------------------

void GB_matlab_helper7              // Kx = uint64 (0:mnz-1)
(
    uint64_t *restrict Kx,          // array of size mnz
    const GrB_Index mnz
)
{

    GB_NTHREADS (mnz) ;

    #pragma omp parallel for num_threads(nthreads) schedule(static)
    for (int64_t k = 0 ; k < mnz ; k++)
    {
        Kx [k] = k ;
    }
}

//------------------------------------------------------------------------------
// GB_matlab_helper8: expand a scalar into an array
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

    #pragma omp parallel for num_threads(nthreads) schedule(static)
    for (int64_t k = 0 ; k < nvals ; k++)
    {
        // C [k] = A [0]
        memcpy (C + k * s, A, s) ;
    }
}

