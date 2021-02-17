//------------------------------------------------------------------------------
// GB_convert_bitmap_worker: construct triplets or CSC/CSR from bitmap
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// TODO allow this function to do typecasting.  Create 169 different versions
// for all 13x13 versions.  Use this as part of Method 24, C=A assignment.

#include "GB.h"
#include "GB_partition.h"

#define GB_FREE_ALL     \
{                       \
    GB_FREE (W) ;       \
}

GrB_Info GB_convert_bitmap_worker   // extract CSC/CSR or triplets from bitmap
(
    // outputs:
    int64_t *GB_RESTRICT Ap,        // vector pointers for CSC/CSR form
    int64_t *GB_RESTRICT Ai,        // indices for CSC/CSR or triplet form
    int64_t *GB_RESTRICT Aj,        // vector indices for triplet form
    GB_void *GB_RESTRICT Ax_new,    // values for CSC/CSR or triplet form
    int64_t *anvec_nonempty,        // # of non-empty vectors
    // inputs: not modified
    const GrB_Matrix A,             // matrix to extract; not modified
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (GB_IS_BITMAP (A)) ;
    ASSERT (Ap != NULL) ;           // must be provided on input, size avdim+1

    int64_t *GB_RESTRICT W = NULL ;
    const int64_t avdim = A->vdim ;
    const int64_t avlen = A->vlen ;
    const size_t asize = A->type->size ;

    //--------------------------------------------------------------------------
    // count the entries in each vector
    //--------------------------------------------------------------------------

    const int8_t *GB_RESTRICT Ab = A->b ;

    GB_GET_NTHREADS_MAX (nthreads_max, chunk, Context) ;
    int nthreads = GB_nthreads (avlen*avdim, chunk, nthreads_max) ;
    bool by_vector = (nthreads <= avdim) ;

    if (by_vector)
    {

        //----------------------------------------------------------------------
        // compute all vectors in parallel (no workspace)
        //----------------------------------------------------------------------

        int64_t j ;
        #pragma omp parallel for num_threads(nthreads) schedule(static)
        for (j = 0 ; j < avdim ; j++)
        {
            // ajnz = nnz (A (:,j))
            int64_t ajnz = 0 ;
            int64_t pA_start = j * avlen ;
            for (int64_t i = 0 ; i < avlen ; i++)
            { 
                // see if A(i,j) is present in the bitmap
                int64_t p = i + pA_start ;
                ajnz += Ab [p] ;
                ASSERT (Ab [p] == 0 || Ab [p] == 1) ;
            }
            Ap [j] = ajnz ;
        }

    }
    else
    {

        //----------------------------------------------------------------------
        // compute blocks of rows in parallel
        //----------------------------------------------------------------------

        // allocate one row of W per thread, each row of length avdim
        W = GB_MALLOC (nthreads * avdim, int64_t) ;
        if (W == NULL)
        {
            GB_FREE_ALL ;
            return (GrB_OUT_OF_MEMORY) ;
        }

        int taskid ;
        #pragma omp parallel for num_threads(nthreads) schedule(static)
        for (taskid = 0 ; taskid < nthreads ; taskid++)
        {
            int64_t *GB_RESTRICT Wtask = W + taskid * avdim ;
            int64_t istart, iend ;
            GB_PARTITION (istart, iend, avlen, taskid, nthreads) ;
            for (int64_t j = 0 ; j < avdim ; j++)
            {
                // ajnz = nnz (A (istart:iend-1,j))
                int64_t ajnz = 0 ;
                int64_t pA_start = j * avlen ;
                for (int64_t i = istart ; i < iend ; i++)
                { 
                    // see if A(i,j) is present in the bitmap
                    int64_t p = i + pA_start ;
                    ajnz += Ab [p] ;
                    ASSERT (Ab [p] == 0 || Ab [p] == 1) ;
                }
                Wtask [j] = ajnz ;
            }
        }

        // cumulative sum to compute nnz(A(:,j)) for each vector j
        int64_t j ;
        #pragma omp parallel for num_threads(nthreads) schedule(static)
        for (j = 0 ; j < avdim ; j++)
        {
            int64_t ajnz = 0 ;
            for (int taskid = 0 ; taskid < nthreads ; taskid++)
            { 
                int64_t *GB_RESTRICT Wtask = W + taskid * avdim ;
                int64_t c = Wtask [j] ;
                Wtask [j] = ajnz ;
                ajnz += c ;
            }
            Ap [j] = ajnz ;
        }
    }

    //--------------------------------------------------------------------------
    // cumulative sum of Ap 
    //--------------------------------------------------------------------------

    int nth = GB_nthreads (avdim, chunk, nthreads_max) ;
    GB_cumsum (Ap, avdim, anvec_nonempty, nth) ;
    int64_t anz = Ap [avdim] ;
    ASSERT (anz == A->nvals) ;

    //--------------------------------------------------------------------------
    // gather the pattern and values from the bitmap
    //--------------------------------------------------------------------------

    // TODO: add type-specific versions for built-in types

    const GB_void *GB_RESTRICT Ax = A->x ;

    if (by_vector)
    {

        //----------------------------------------------------------------------
        // construct all vectors in parallel (no workspace)
        //----------------------------------------------------------------------

        int64_t j ;
        #pragma omp parallel for num_threads(nthreads) schedule(static)
        for (j = 0 ; j < avdim ; j++)
        {
            // gather from the bitmap into the new A (:,j)
            int64_t pnew = Ap [j] ;
            int64_t pA_start = j * avlen ;
            for (int64_t i = 0 ; i < avlen ; i++)
            {
                int64_t p = i + pA_start ;
                if (Ab [p])
                {
                    // A(i,j) is in the bitmap
                    if (Ai != NULL) Ai [pnew] = i ;
                    if (Aj != NULL) Aj [pnew] = j ;
                    if (Ax_new != NULL)
                    { 
                        // Ax_new [pnew] = Ax [p])
                        memcpy (Ax_new +(pnew)*asize, Ax +(p)*asize, asize) ;
                    }
                    pnew++ ;
                }
            }
            ASSERT (pnew == Ap [j+1]) ;
        }

    }
    else
    {

        //----------------------------------------------------------------------
        // compute blocks of rows in parallel
        //----------------------------------------------------------------------

        int taskid ;
        #pragma omp parallel for num_threads(nthreads) schedule(static)
        for (taskid = 0 ; taskid < nthreads ; taskid++)
        {
            int64_t *GB_RESTRICT Wtask = W + taskid * avdim ;
            int64_t istart, iend ;
            GB_PARTITION (istart, iend, avlen, taskid, nthreads) ;
            for (int64_t j = 0 ; j < avdim ; j++)
            {
                // gather from the bitmap into the new A (:,j)
                int64_t pnew = Ap [j] + Wtask [j] ;
                int64_t pA_start = j * avlen ;
                for (int64_t i = istart ; i < iend ; i++)
                {
                    // see if A(i,j) is present in the bitmap
                    int64_t p = i + pA_start ;
                    if (Ab [p])
                    {
                        // A(i,j) is in the bitmap
                        if (Ai != NULL) Ai [pnew] = i ;
                        if (Aj != NULL) Aj [pnew] = j ;
                        if (Ax_new != NULL)
                        { 
                            // Ax_new [pnew] = Ax [p] ;
                            memcpy (Ax_new +(pnew)*asize, Ax +(p)*asize, asize);
                        }
                        pnew++ ;
                    }
                }
            }
        }
    }

    //--------------------------------------------------------------------------
    // free workspace return result
    //--------------------------------------------------------------------------

    GB_FREE (W) ;
    return (GrB_SUCCESS) ;
}

