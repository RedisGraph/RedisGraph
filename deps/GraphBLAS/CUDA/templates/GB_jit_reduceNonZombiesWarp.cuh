//------------------------------------------------------------------------------
// template/GB_jit_reduceNonZombiesWarp.cu
//------------------------------------------------------------------------------

// The GB_jit_reduceNonZombiesWarp CUDA kernel reduces an array g_idata of size
// n, of any type T_A, to an array g_odata of size grid.x.  Each threadblock
// (blockIdx.x) reduces its portion of g_idata to a single scalar, g_odata
// [blockIdx.x].

// Both the grid and block are 1D, so blockDim.x is the # threads in a
// threadblock, and the # of threadblocks is grid.x

// Let b = blockIdx.x, and let s be blockDim.x.
// Each threadblock owns s*8 contiguous items in the input data.

// Thus, threadblock b owns g_idata [b*s*8 ... min(n,(b+1)*s*8-1)].  It's job
// is to reduce this data to a scalar, and write it to g_odata [b].

#define GB_CUDA_KERNEL
#include <limits>
#include <type_traits>
#include "GB_cuda_kernel.h"
#include "GB_cuda_atomics.cuh"
#include <cstdint>
#include <cooperative_groups.h>

using namespace cooperative_groups;

template< typename T_A, int tile_sz, int rcode>
__inline__ __device__ 
T_A warp_ReduceSum( thread_block_tile<tile_sz> g, T_A val)
{
    // Each iteration halves the number of active threads
    // Each thread adds its partial sum[i] to sum[lane+i]

    /*
    #pragma unroll
    for (int i = tile_sz >> 1; i > 0; i >>= 1) {
        T_A fold = g.shfl_down( val, i);
        GB_ADD( val, val, fold );
        //printf("thd%d   %d OP %d is %d\n", threadIdx.x, val, fold, OP( val, fold));
    }
    */
        T_A fold = g.shfl_down( val, 16);
        GB_ADD( val, val, fold );
        fold = g.shfl_down( val, 8);
        GB_ADD( val, val, fold );
        fold = g.shfl_down( val, 4);
        GB_ADD( val, val, fold );
        fold = g.shfl_down( val, 2);
        GB_ADD( val, val, fold );
        fold = g.shfl_down( val, 1);
        GB_ADD( val, val, fold );
    //if (threadIdx.x ==0) printf("thd%d single warp sum is %d\n", threadIdx.x,  val);
    return val; // note: only thread 0 will return full sum
}


template<typename T_A, int warpSize, int rcode>
__inline__ __device__
T_A block_ReduceSum(thread_block g, T_A val)
{
  static __shared__ T_A shared[warpSize]; // Shared mem for 32 partial sums
  int lane = threadIdx.x & 31 ; // % warpSize;
  int wid  = threadIdx.x >> 5 ; // / warpSize;
  thread_block_tile<warpSize> tile = tiled_partition<warpSize>( g );

    // TODO: Figure out how to use graphblas-specific INFINITY macro
    #ifndef INFINITY
    #define INFINITY std::numeric_limits<double>::max()
    #endif


    // Each warp performs partial reduction
  val = warp_ReduceSum<T_A, warpSize, rcode>( tile, val);

  // Wait for all partial reductions
  if (lane==0) { 
     //printf("thd%d warp%d sum is %d\n", threadIdx.x, wid, val);
     shared[wid] = val; // Write reduced value to shared memory
     //printf("thd%d stored warp%d sum %d\n", threadIdx.x, wid, val);
  }
  this_thread_block().sync();     // Wait for all partial reductions
    GB_DECLARE_MONOID_IDENTITY (identity) ;

  //if (wid > 0 ) return val;
  //read from shared memory only if that warp existed
  //else { 
    val = (threadIdx.x < (blockDim.x / warpSize) ) ? shared[lane] : identity ;
    //if (lane < (blockDim.x/ warpSize) ) printf("thd%d warp%d loaded val = %d\n", threadIdx.x, lane, val);
    val = warp_ReduceSum<T_A, warpSize, rcode>( tile, val); //Final reduce within first warp
  //}

  return val;
}


template< typename T_A, typename T_Z, int rcode, bool atomic_reduce = true>
__global__ void reduceNonZombiesWarp
(
    GrB_Matrix A,
    GrB_Scalar R,      // array of size grid.x if atomic_reduce==false and size 1 if atomic_reduce==true
    int64_t N,         // number of edges for sparse, size of x array for full/bitmap
    bool is_sparse
)
{

    // TODO: Figure out how to use graphblas-specific INFINITY macro
    #ifndef INFINITY
    #define INFINITY std::numeric_limits<T_A>::max()
    #endif

    // set thread ID
    int tid = threadIdx.x ;

    const int64_t *__restrict__ index = A->i;
    const T_A *__restrict__ g_idata = (T_A*) A->x;  // T_A or GB_A_TYPENAME
    T_Z *g_odata = (T_Z*) R->x;

    // each thread tid reduces its result into sum
//  T_Z sum = (T_Z) GB_IDENTITY;
    GB_DECLARE_MONOID_IDENTITY (sum) ;

    for(int i = blockIdx.x * blockDim.x + threadIdx.x; 
        i < N;
        i += blockDim.x * gridDim.x) {

        if (is_sparse && index[i] < 0) continue; // skip zombies
        T_A fold = g_idata[i];
        GB_ADD( sum, sum, fold );
    }
    this_thread_block().sync(); 

    //--------------------------------------------------------------------------
    // reduce work [0..s-1] to a single scalar
    //--------------------------------------------------------------------------
    // this assumes blockDim is a multiple of 32
    sum = block_ReduceSum< T_A, 32, rcode >( this_thread_block(), sum) ;
    this_thread_block().sync(); 

    // write result for this block to global mem
    if (tid == 0)
    {
        // TODO: Assuming sum for now (like the rest of the kernel)
        if(atomic_reduce) {
            atomic_add<T_Z>(g_odata, sum);
        } else {
            g_odata [blockIdx.x] = sum ;
        }
    }
}

