//------------------------------------------------------------------------------
// templates/GB_AxB_cuda_dot3_phase2: fill the global buckets
//------------------------------------------------------------------------------

// TODO describe me

#define GB_KERNEL
#include <cstdint>
#include "GB_cuda_buckets.h"
#include "matrix.h"
#include <cooperative_groups.h>
#include "local_cub/block/block_scan.cuh"

using namespace cooperative_groups;

// A stateful callback functor that maintains a running prefix to be applied
// during consecutive scan operations.
struct BlockPrefixCallbackOp
{
   // Running prefix
   int64_t running_total;
   // Constructor
   __device__ BlockPrefixCallbackOp(int64_t running_total) : running_total(running_total) {}

   // Callback operator to be entered by the first warp of threads in the block.
   // Thread-0 is responsible for returning a value for seeding the block-wide scan.
   __device__ int64_t operator()(int64_t block_aggregate)
   {
     int64_t old_prefix = running_total;
     running_total += block_aggregate;
     return old_prefix;
   }
};

__inline__ 
__device__ void blockBucketExclusiveSum(int bucketId, int64_t *d_data, int nblocks)
{
   #define blocksize  32

   // Specialize BlockScan for a 1D block of 32 threads
   typedef cub::BlockScan<int64_t, 32, cub::BLOCK_SCAN_WARP_SCANS> BlockScan; 

   // Allocate shared memory for BlockScan
   __shared__ typename BlockScan::TempStorage temp_storage;

   // Initialize running total
   BlockPrefixCallbackOp prefix_op(0);

   // Have the block iterate over segments of items
   int64_t data=0;

   int64_t *blockbucket= d_data;

   for (int block_id = 0; block_id < nblocks; block_id += blocksize)
   {
    // Load a segment of consecutive items that are blocked across threads

    //printf("block %d entering sum\n",blockIdx.x);
      int loc = block_id + threadIdx.x;
      if ( loc < nblocks)
      { 
        //printf("block %di loading tid=%d\n",block_id,tid);
        data  = blockbucket[bucketId*nblocks    +loc ] ; 
      }
      __syncthreads();

      //printf("bb%d_%d s0 before prefix= %ld \n", block_id,bucketId, 
      //                     blockbucket[bucketId*nblocks + block_id+threadIdx.x] )  ; 
      // Collectively compute the block-wide exclusive prefix sum
      BlockScan(temp_storage).ExclusiveSum( data, data, prefix_op);
      __syncthreads();

      if ( loc < nblocks)
      { 
        blockbucket[bucketId*nblocks   +loc ]  = data  ; 
      }
      __syncthreads();

        //printf("bb%d_%d = %ld \n", block_id, bucketId, blockbucket[bucketId*nblocks+block_id+threadIdx.x] )  ; 
      
      data = 0;
   }
}


template< typename T, int tile_sz>
__inline__ __device__ 
T warp_ReduceSumPlus( thread_block_tile<tile_sz> tile, T val)
{
    // Each iteration halves the number of active threads
    // Each thread adds its partial sum[i] to sum[lane+i]
    for (int i = tile.size() / 2; i > 0; i /= 2) {
        val +=  tile.shfl_down( val, i);
    }
    return val; // note: only thread 0 will return full sum
}

template<typename T, int warpSize>
__inline__ __device__
T block_ReduceSum(thread_block g, T val)
{
  static __shared__ T shared[warpSize]; // Shared mem for 32 partial sums
  int lane = threadIdx.x % warpSize;
  int wid = threadIdx.x / warpSize;
  thread_block_tile<warpSize> tile = tiled_partition<warpSize>( g );

  // Each warp performs partial reduction
  val = warp_ReduceSumPlus<T, warpSize>( tile, val);    

  // Wait for all partial reductions
  if (lane==0) { 
     //printf("thd%d warp%d sum is %d\n", threadIdx.x, wid, val);
     shared[wid]=val; // Write reduced value to shared memory
     //printf("thd%d stored warp %d sum %d\n", threadIdx.x, wid, val);
  }
  __syncthreads();              // Wait for all partial reductions

  if (wid > 0 ) return val ;
  //Final reduce within first warp
  if (wid==0) val = warp_ReduceSumPlus<T, warpSize>( tile, val) ; 

  return val;
}

// GB_AxB_cuda_dot3_phase2 is a CUDA kernel that takes as input the
// nanobuckets and blockbucket arrays computed by the first phase kernel,
// GB_AxB_cuda_dot3_phase1.  The launch geometry of this kernel must match the
// GB_AxB_cuda_dot3_phase1 kernel, with the same # of threads and threadblocks.

__global__ 
void GB_AxB_dot3_phase2
(
    // input, not modified:
    int64_t *__restrict__ nanobuckets,    // array of size 12-blockDim.x-by-nblocks
    int64_t *__restrict__ blockbucket,    // global bucket count, of size 12*nblocks
    // output:
    int64_t *__restrict__ bucketp,        // global bucket cumsum, of size 13 
    int64_t *__restrict__ bucket,         // global buckets, of size cnz (== mnz)
    int64_t *__restrict__ offset,         // global offsets, for each bucket
    // inputs, not modified:
    GrB_Matrix C,             // output matrix
    const int64_t cnz,        // number of entries in C and M 
    const int nblocks         // input number of blocks to reduce
)
{

    //--------------------------------------------------------------------------
    // get C and M
    //--------------------------------------------------------------------------

    //int64_t *Ci = C->i ;       // for zombies, or bucket assignment

    // Ci [p] for an entry C(i,j) contains either GB_FLIP(i) if C(i,j) is a
    // zombie, or (k << 4) + bucket otherwise, where C(:,j) is the kth vector
    // of C (j = Ch [k] if hypersparse or j = k if standard sparse), and
    // where bucket is the bucket assignment for C(i,j).  This phase does not
    // need k, just the bucket for each entry C(i,j).

    //--------------------------------------------------------------------------
    // sum up the bucket counts of prior threadblocks
    //--------------------------------------------------------------------------

    // blockbucket is an array of size 12-by-nblocks, held by row.  The
    // entry blockbucket [bucket * nblocks + t] holds the # of entries
    // in the bucket (in range 0 to 11) found by threadblock t.


    //__shared__ uint64_t offset [12] ;
    uint64_t s_0=0;
    uint64_t s_1=0;
    uint64_t s_2=0;
    uint64_t s_3=0;
    uint64_t s_4=0;
    uint64_t s_5=0;
    uint64_t s_6=0;
    uint64_t s_7=0;
    uint64_t s_8=0;
    uint64_t s_9=0;
    uint64_t s_10=0;
    uint64_t s_11=0;

    thread_block_tile<32> tile = tiled_partition<32>(this_thread_block() );

    //printf("block %d entering sum\n",blockIdx.x);
    int tid = threadIdx.x  + blockIdx.x*blockDim.x;
    #define reduceBucket( B )    \
     for( tid = threadIdx.x + blockIdx.x*blockDim.x; \
          tid < nblocks;  \
          tid += blockDim.x*gridDim.x) \
     {                           \
        s_ ## B  += blockbucket[  B *nblocks +tid] ;  \
     } \
     __syncthreads(); \
     s_ ## B  = warp_ReduceSumPlus<uint64_t , 32>( tile, s_ ## B); 

     reduceBucket( 0 )
     reduceBucket( 1 )
     reduceBucket( 2 )
     reduceBucket( 3 )
     reduceBucket( 4 )
     reduceBucket( 5 )
     reduceBucket( 6 )
     reduceBucket( 7 )
     reduceBucket( 8 )
     reduceBucket( 9 )
     reduceBucket( 10 )
     reduceBucket( 11 )


        //printf("summing blk,tid=%d,%d\n",blockIdx.x,threadIdx.x);
       if (threadIdx.x ==0 )
       {
          atomicAdd( (unsigned long long int*)&(offset[0]), s_0);
          atomicAdd( (unsigned long long int*)&(offset[1]), s_1);
          atomicAdd( (unsigned long long int*)&(offset[2]), s_2);
          atomicAdd( (unsigned long long int*)&(offset[3]), s_3);
          atomicAdd( (unsigned long long int*)&(offset[4]), s_4);
          atomicAdd( (unsigned long long int*)&(offset[5]), s_5);
          atomicAdd( (unsigned long long int*)&(offset[6]), s_6);
          atomicAdd( (unsigned long long int*)&(offset[7]), s_7);
          atomicAdd( (unsigned long long int*)&(offset[8]), s_8);
          atomicAdd( (unsigned long long int*)&(offset[9]), s_9);
          atomicAdd( (unsigned long long int*)&(offset[10]),s_10);
          atomicAdd( (unsigned long long int*)&(offset[11]),s_11);
       }
       __syncthreads();
       


    if( gridDim.x >= 12)
    {
        // Cumulative sum across blocks for each bucket 
        if (blockIdx.x <12)
           blockBucketExclusiveSum( blockIdx.x, blockbucket, nblocks ) ;
    }
    else
    {
        if (blockIdx.x == 0)
        {
           blockBucketExclusiveSum( 0, blockbucket, nblocks ) ;
           blockBucketExclusiveSum( 1, blockbucket, nblocks ) ;
           blockBucketExclusiveSum( 2, blockbucket, nblocks ) ;
           blockBucketExclusiveSum( 3, blockbucket, nblocks ) ;
           blockBucketExclusiveSum( 4, blockbucket, nblocks ) ;
           blockBucketExclusiveSum( 5, blockbucket, nblocks ) ;
           blockBucketExclusiveSum( 6, blockbucket, nblocks ) ;
           blockBucketExclusiveSum( 7, blockbucket, nblocks ) ;
           blockBucketExclusiveSum( 8, blockbucket, nblocks ) ;
           blockBucketExclusiveSum( 9, blockbucket, nblocks ) ;
           blockBucketExclusiveSum( 10, blockbucket, nblocks) ;
           blockBucketExclusiveSum( 11, blockbucket, nblocks) ;
        }
    }
    
    
    

    //--------------------------------------------------------------------------
    // last threadblock saves the cumsum of the 12 global buckets
    //--------------------------------------------------------------------------
    /* do on cpu
    if (blockIdx.x == 0) // gridDim.x - 1)
    {

        // the last threadblock: compute all 12 global bucket sizes, and its
        // cumulative sum
        if (threadIdx.x == 0)
        {
            // the work in this last threadblock is single-threaded
            uint64_t s = 0;
            for (int bucket = 0 ; bucket < 12 ; bucket++)
            {
                // write the global cumsum of all buckets to the final global
                // bucketp.  bucketp [bucket] is the starting position in
                // the bucket.
                bucketp [bucket] = s ;
                
                // bucket_size is the total # of entries in this bucket, for
                // all threadblocks.  It has nearly been computed already,
                // since offset [bucket] = sum (blockbucket (bucket,0:blockDim.x-1)).
                // All that is left is to add the counts for the last threadblock.`
                //int64_t global_bucket_size = offset [bucket];   
                     // + blockbucket [bucket * gridDim.x + blockIdx.x] ;

                //printf("bucketp[%d]= %ld\n",bucket, s);
                // s is a cumulative sum of the global bucket sizes
                s += offset[bucket]; // global_bucket_size ;
            }
            // The kth global bucket (for k = 0 to 11) appears in:
            // bucket [bucketp [k]... bucketp [k+1]-1],
            // so the end of the last bucket needs bucketp [12].
            bucketp [12] = (int64_t)s;
                //printf("bucketp[12]= %ld\n", s);
            // all entries in C now appear in the buckets.
            // ASSERT (s == cnz) ;
        }
        __syncthreads ( ) ;
    }
    */

} // phase2 


__global__ 
void GB_AxB_dot3_phase2end
(
    // input, not modified:
    int64_t *__restrict__ nanobuckets,    // array of size 12-blockDim.x-by-nblocks
    const int64_t *__restrict__ blockbucket,    // global bucket count, of size 12*nblocks
    // output:
    const int64_t *__restrict__ bucketp,        // global bucket cumsum, of size 13 
    int64_t *__restrict__ bucket,         // global buckets, of size cnz (== mnz)
    const int64_t *__restrict__ offset,        // global offsets, for each bucket
    // inputs, not modified:
    const GrB_Matrix C,            // output matrix
    const int64_t cnz        // number of entries in C and M 
)
{


    int64_t *__restrict__ Ci = C->i ;       // for zombies, or bucket assignment
    int64_t *__restrict__ Mp = C->p ;       // for offset calculations 
    int64_t mnvec = C->nvec;

    //--------------------------------------------------------------------------
    // load and shift the nanobuckets for this thread block
    //--------------------------------------------------------------------------

    // The taskbucket for this threadblock is an array of size
    // 12-by-blockDim.x, held by row.  It forms a 2D array within the 3D
    // nanobuckets array.
    int64_t *__restrict__ taskbucket = nanobuckets + blockIdx.x * (12 * blockDim.x) ;

    //printf("block%d thd%d blockbucket= %ld\n", blockIdx.x, threadIdx.x, 
    //                                           blockbucket[blockIdx.x*gridDim.x+blockIdx.x]);

    // Each thread in this threadblock owns one column of this taskbucket, for
    // its set of 12 nanobuckets.  The nanobuckets are a column of length 12,
    // with stride equal to blockDim.x.
    int64_t *__restrict__ nanobucket = taskbucket + threadIdx.x;

    // Each thread loads its 12 nanobucket values into registers.
    #define LOAD_NANOBUCKET(bucket)                     \
        int64_t my_bucket_ ## bucket =                  \
            nanobucket [bucket * blockDim.x]            \
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

