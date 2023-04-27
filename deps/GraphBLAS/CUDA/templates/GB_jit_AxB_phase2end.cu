//------------------------------------------------------------------------------
// templates/GB_AxB_cuda_dot3_phase2: fill the global buckets
//------------------------------------------------------------------------------

// TODO describe me

#define GB_CUDA_KERNEL


//#include <cstdint>
#include "GB_cuda_buckets.h"
#include "matrix.h"
#include <cooperative_groups.h>
#include "local_cub/block/block_scan.cuh"

using namespace cooperative_groups;

__global__
void AxB_phase2end
        (
                // input, not modified:
                int64_t *__restrict__ nanobuckets,    // array of size 12-blockDim.x-by-nblocks
                const int64_t *__restrict__ blockbucket,    // global bucket count, of size 12*nblocks
                // output:
                const int64_t *__restrict__ bucketp,        // global bucket cumsum, of size 13
                int64_t *__restrict__ bucket,         // global buckets, of size cnz (== mnz)
                const int64_t *__restrict__ offset,         // global offsets, for each bucket
                // inputs, not modified:
                const GrB_Matrix C,            // output matrix
                const int64_t cnz        // number of entries in C and M
        )
{

    //--------------------------------------------------------------------------
    // get C and M
    //--------------------------------------------------------------------------

    // Ci [p] for an entry C(i,j) contains either GB_FLIP(i) if C(i,j) is a
    // zombie, or (k << 4) + bucket otherwise, where C(:,j) is the kth vector
    // of C (j = Ch [k] if hypersparse or j = k if standard sparse), and
    // where bucket is the bucket assignment for C(i,j).  This phase does not
    // need k, just the bucket for each entry C(i,j).

    int64_t *__restrict__ Ci = C->i ;       // for zombies, or bucket assignment
    int64_t *__restrict__ Mp = C->p ;       // for offset calculations
    int64_t mnvec = C->nvec;

    //--------------------------------------------------------------------------
    // load and shift the nanobuckets for this thread block
    //--------------------------------------------------------------------------

    // The taskbucket for this threadblock is an array of size
    // 12-by-blockDim.x, held by row.  It forms a 2D array within the 3D
    // nanobuckets array.
    int64_t *__restrict__ taskbucket = nanobuckets + blockIdx.x * (NBUCKETS * blockDim.x) ;

    //printf("block%d thd%d blockbucket= %ld\n", blockIdx.x, threadIdx.x,
    //                                           blockbucket[blockIdx.x*gridDim.x+blockIdx.x]);

    // Each thread in this threadblock owns one column of this taskbucket, for
    // its set of 12 nanobuckets.  The nanobuckets are a column of length 12,
    // with stride equal to blockDim.x.
    int64_t *__restrict__ nanobucket = taskbucket + threadIdx.x;

    // Each thread loads its 12 nanobucket values into registers.
#define LOAD_NANOBUCKET(bucket)                     \
        int64_t my_bucket_ ## bucket =                  \
            nanobucket [bucket * blockDim.x]        \
         + blockbucket [bucket * gridDim.x + blockIdx.x]\
         + bucketp [bucket] ; 

    LOAD_NANOBUCKET (0) ;
    LOAD_NANOBUCKET (1) ;
    LOAD_NANOBUCKET (2) ;
    LOAD_NANOBUCKET (3) ;
    LOAD_NANOBUCKET (4) ;
    LOAD_NANOBUCKET (5) ;
    LOAD_NANOBUCKET (6) ;
    LOAD_NANOBUCKET (7) ;
    LOAD_NANOBUCKET (8) ;
    LOAD_NANOBUCKET (9) ;
    LOAD_NANOBUCKET (10) ;
    LOAD_NANOBUCKET (11) ;

    // Now each thread has an index into the global set of 12 buckets,
    // held in bucket, of where to place its own entries.

    //--------------------------------------------------------------------------
    // construct the global buckets
    //--------------------------------------------------------------------------

    // The slice for task blockIdx.x contains entries pfirst:plast-1 of M and
    // C, which is the part of C operated on by this threadblock.
    int64_t pfirst, plast ;

    /*
    for ( int tid_global = threadIdx.x + blockIdx.x * blockDim.x ;
              tid_global < (mnvec+7)/8 ;
              tid_global += blockDim.x * gridDim.x)
    */
    int chunk_max= (cnz + chunksize -1)/chunksize;
    for ( int chunk = blockIdx.x;
          chunk < chunk_max;
          chunk += gridDim.x )
    {

        //GB_PARTITION (pfirst, plast, cnz, tid_global, (mnvec+7)/8 ) ;
        pfirst = chunksize * chunk ;
        plast  = GB_IMIN( chunksize * (chunk+1), cnz ) ;

        int chunk_end;
        if ( cnz > chunksize) chunk_end = GB_IMIN(  chunksize,
                                                    cnz - chunksize*(chunk) );
        else chunk_end = cnz;

        // find the first vector of the slice for task blockIdx.x: the
        // vector that owns the entry Ai [pfirst] and Ax [pfirst].
        //kfirst = GB_search_for_vector_device (pfirst, Mp, 0, mnvec) ;

        // find the last vector of the slice for task blockIdx.x: the
        // vector that owns the entry Ai [plast-1] and Ax [plast-1].
        //klast = GB_search_for_vector_device (plast-1, Mp, kfirst, mnvec) ;


        for ( int p = pfirst + threadIdx.x;
              p < pfirst + chunk_end;
              p += blockDim.x )
        {
            // get the entry C(i,j), and extract its bucket.  Then
            // place the entry C(i,j) in the global bucket it belongs to.

            // TODO: these writes to global are not coalesced.  Instead: each
            // threadblock could buffer its writes to 12 buffers and when the
            // buffers are full they can be written to global.
            int ibucket = Ci[p] & 0xF;
            //printf(" thd: %d p,Ci[p] = %ld,%ld,%d\n", threadIdx.x, p, Ci[p], irow );
            switch (ibucket)
            {
                case  0: bucket [my_bucket_0++ ] = p ; Ci[p] = Ci[p] >>4; break ; //unshift zombies
                case  1: bucket [my_bucket_1++ ] = p ; break ;
                case  2: bucket [my_bucket_2++ ] = p ; break ;
                case  3: bucket [my_bucket_3++ ] = p ; break ;
                case  4: bucket [my_bucket_4++ ] = p ; break ;
                case  5: bucket [my_bucket_5++ ] = p ; break ;
                case  6: bucket [my_bucket_6++ ] = p ; break ;
                case  7: bucket [my_bucket_7++ ] = p ; break ;
                case  8: bucket [my_bucket_8++ ] = p ; break ;
                case  9: bucket [my_bucket_9++ ] = p ; break ;
                case 10: bucket [my_bucket_10++] = p ; break ;
                case 11: bucket [my_bucket_11++] = p ; break ;
                default: break;
            }

        }
        //__syncthreads();
    }
}

