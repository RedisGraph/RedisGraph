//------------------------------------------------------------------------------
// reduceUnrolled.cu
//------------------------------------------------------------------------------

// The reduceUnrolled CUDA kernel reduces an array g_idata of size n, of any
// type T, to an array g_odata of size grid.x.  Each threadblock (blockIdx.x)
// reduces its portion of g_idata to a single scalar, g_odata [blockIdx.x].

// Both the grid and block are 1D, so blockDim.x is the # threads in a
// threadblock, and the # of threadblocks is grid.x

// Let b = blockIdx.x, and let s be blockDim.x.
// Each threadblock owns s*8 contiguous items in the input data.

// Thus, threadblock b owns g_idata [b*s*8 ... min(n,(b+1)*s*8-1)].  It's job
// is to reduce this data to a scalar, and write it to g_odata [b].

#include "mySemiRing.h"
#include <cooperative_groups.h>

using namespace cooperative_groups;

template< typename T, int tile_sz>
__inline__ __device__ 
T warp_ReduceSum( thread_block_tile<tile_sz> g, T val)
{
    // Each iteration halves the number of active threads
    // Each thread adds its partial sum[i] to sum[lane+i]
    for (int i = g.size() / 2; i > 0; i /= 2) {
        T fold = g.shfl_down( val, i);
        //printf("thd%d   %d OP %d is %d\n", threadIdx.x, val, fold, OP( val, fold));
        val = OP( val, fold );
    }
    //if (threadIdx.x ==0) printf("thd%d single warp sum is %d\n", threadIdx.x,  val);
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
  val = warp_ReduceSum<T, warpSize>( tile, val);    

  // Wait for all partial reductions
  if (lane==0) { 
     //printf("thd%d warp%d sum is %d\n", threadIdx.x, wid, val);
     shared[wid]=val; // Write reduced value to shared memory
     //printf("thd%d stored warp %d sum %d\n", threadIdx.x, wid, val);
  }
  __syncthreads();              // Wait for all partial reductions

  if (wid > 0 || gridDim.x == 1 ) return val;
  //read from shared memory only if that warp existed
  val = (threadIdx.x < blockDim.x / warpSize) ? shared[lane] : MONOID_IDENTITY;
  //printf("thd%d warp loaded val = %d\n", threadIdx.x, lane, val);

  
  if (wid==0) val = warp_ReduceSum<T, warpSize>( tile, val); //Final reduce within first warp

  return val;
}

template< typename T>
__global__ void reduceWarp
(
    T *g_idata,     // array of size n
    T *g_odata,     // array of size grid.x
    unsigned int N
)
{
    // set thread ID
    unsigned int tid = threadIdx.x ;

    // each thread tid reduces its result into sum
    T sum = (T) MONOID_IDENTITY;

    for(int i = blockIdx.x * blockDim.x + threadIdx.x; 
        i < N; 
        i += blockDim.x * gridDim.x) {
        sum = OP( sum, g_idata[i]);
    }
    //printf("thd%d  sum is %d\n", threadIdx.x + blockDim.x*blockIdx.x, sum);
    __syncthreads();
    //--------------------------------------------------------------------------
    // reduce work [0..s-1] to a single scalar
    //--------------------------------------------------------------------------
    // this assumes blockDim is a multiple of 32
    sum = block_ReduceSum<T , 32>( this_thread_block(), sum); 

    // write result for this block to global mem
    if (tid == 0)
    {
        // printf ("final %d : %g\n", b, (double) work [0]) ;
        g_odata [blockIdx.x] = sum ;
    }
}

