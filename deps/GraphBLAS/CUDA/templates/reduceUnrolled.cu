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

#include "myOp.h"
#include <cooperative_groups.h>
#include "GB_cuda.h"

GrB_Matrix Stuff ;  // hack hack hack

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
  g.sync();              // Wait for all partial reductions

  if (wid > 0 || gridDim.x == 1 ) return val;
  //read from shared memory only if that warp existed
  val = (threadIdx.x < blockDim.x / warpSize) ? shared[lane] : MONOID_IDENTITY;
  //printf("thd%d warp loaded val = %d\n", threadIdx.x, lane, val);

  
  if (wid==0) val = warp_ReduceSum<T, warpSize>( tile, val); //Final reduce within first warp

  return val;
}

template< typename T>
__global__ void reduceUnrolled
(
    T *g_idata,     // array of size n
    T *g_odata,     // array of size grid.x
    unsigned int n
)
{
    // set thread ID
    unsigned int tid = threadIdx.x ;

    // this threadblock b owns g_idata [block_start ... block_end-1]
    unsigned long int s = blockDim.x ;
    unsigned long int b = blockIdx.x ;
    unsigned long int block_start = b * s * 8 ;
    unsigned long int block_end   = (b + 1) * s * 8 ;

    /*
    if (tid == 0)
    {
        printf ("block %d: [%lu ... %ld]\n", b, block_start, block_end-1) ;
    }
    */

    /*
    if (tid == 0 && b == 0)
    {
        printf ("type is size %d\n", sizeof (T)) ;
        for (int k = 0 ; k < n ; k++) printf ("%4d: %g\n", k, (double) g_idata [k]) ;
        printf ("\n") ;
    }
    */

    // nothing to do
    if (block_start > block_end) { if (tid == 0) printf ("bye!\n") ; return ; }

    // convert global data pointer to the local pointer of this block
    T *idata = g_idata + block_start ;

    T x0, x1, x2, x3, x4, x5, x6, x7 ;

    if (block_end <= n)
    {
        // unrolling 8
        x0 = idata [tid] ;
        x1 = idata [tid +     s] ;
        x2 = idata [tid + 2 * s] ;
        x3 = idata [tid + 3 * s] ;
        x4 = idata [tid + 4 * s] ;
        x5 = idata [tid + 5 * s] ;
        x6 = idata [tid + 6 * s] ;
        x7 = idata [tid + 7 * s] ;

        /*
        if (b == 0)
        {
            printf ("block zero: here is tid %2d : %g %g %g %g %g %g %g %g \n", tid,
                (double) x0, (double) x1, (double) x2, (double) x3,
                (double) x4, (double) x5, (double) x6, (double) x7) ;
        }
        */

    }
    else
    {
        // the last block has size less than 8*s
        #define IDATA(i) ((i < lastblocksize) ? idata [i] : MONOID_IDENTITY)
        int lastblocksize = n - block_start ;
        x0 = IDATA (tid) ;
        x1 = IDATA (tid +     s) ;
        x2 = IDATA (tid + 2 * s) ;
        x3 = IDATA (tid + 3 * s) ;
        x4 = IDATA (tid + 4 * s) ;
        x5 = IDATA (tid + 5 * s) ;
        x6 = IDATA (tid + 6 * s) ;
        x7 = IDATA (tid + 7 * s) ;
    }
    T sum;
    //work [tid] = x0 + x1 + x2 + x3 + x4 + x5 + x6 + x7 ;
    sum = OP( x0 ,OP( x1, OP( x2, OP( x3,
                 OP( x4, OP( x5 , OP( x6 , x7))))))) ;

        /*
        if (b == 0)
        {
            printf ("block zero: still is tid %2d : %g %g %g %g %g %g %g %g \n", tid,
                (double) x0, (double) x1, (double) x2, (double) x3,
                (double) x4, (double) x5, (double) x6, (double) x7) ;
        }

        if (b == 0)
        {
            printf ("block zero: here is tid %d result %g  is %g\n",
            tid, (double) work [tid],
            (double) (x0 + x1 + x2 + x3 + x4 + x5 + x6 + x7)) ;
        }
        */

    __syncthreads ( ) ;

    //--------------------------------------------------------------------------
    // reduce work [0..s-1] to a single scalar
    //--------------------------------------------------------------------------

    // This assumes that s is a power of 2 and <= 1024, and at least 32
    // This assumes blockDim is a multiple of 32
    sum = block_ReduceSum<T , 32>( this_thread_block(), sum); 

    // write result for this block to global mem
    if (tid == 0)
    {
        // printf ("final %d : %g\n", b, (double) work [0]) ;
        g_odata [b] = sum ;
    }
}

