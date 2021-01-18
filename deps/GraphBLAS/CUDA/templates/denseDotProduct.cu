//------------------------------------------------------------------------------
// denseDotProduct.cu 
//------------------------------------------------------------------------------

// The denseDotProduct CUDA kernel produces the semi-ring dot product of two
// vectors of types T1 and T2 and common size n, to a vector odata of type T3.
// ie. we want to produce dot(x,y) in the sense of the given semi-ring.

// Both the grid and block are 1D, so blockDim.x is the # threads in a
// threadblock, and the # of threadblocks is grid.x

// Let b = blockIdx.x, and let s be blockDim.x.
// Each threadblock owns s*8 contiguous items in the input data.

// Thus, threadblock b owns g_idata [b*s*8 ... min(n,(b+1)*s*8-1)].  It's job
// is to reduce this data to a scalar, and write it to g_odata [b].

#include <limits>
#include "mySemiRing.h"
#include <cooperative_groups.h>

using namespace cooperative_groups;

template< typename T3, int tile_sz>
__inline__ __device__ 
T3 warp_ReduceSum(thread_block_tile<tile_sz> g, T3 val)
{
    // Each iteration halves the number of active threads
    // Each thread adds its partial sum[i] to sum[lane+i]
    for (int i = g.size() / 2; i > 0; i /= 2)
    {
        T3 fold = g.shfl_down( val, i);
        val = ADD( val, fold );
    }
    return val; // note: only thread 0 will return full sum
}

template<typename T3, int warpSize>
__inline__ __device__
T3 block_ReduceSum(thread_block g, T3 val)
{
  static __shared__ T3 shared[warpSize]; // Shared mem for 32 partial sums
  int lane = threadIdx.x % warpSize;
  int wid = threadIdx.x / warpSize;
  thread_block_tile<warpSize> tile = tiled_partition<warpSize>(g);

  // Each warp performs partial reduction
  val = warp_ReduceSum<T3,warpSize>(tile, val);    

  if (lane==0) shared[wid]=val; // Write reduced value to shared memory

  __syncthreads();              // Wait for all partial reductions

  //read from shared memory only if that warp existed
  val = (threadIdx.x < blockDim.x / warpSize) ? shared[lane] : (T3)MONOID_IDENTITY3;

  
  if (wid==0) val = warp_ReduceSum<T3,warpSize>(tile,val); //Final reduce within first warp

  return val;
}

template< typename T1, typename T2, typename T3>
__global__ void denseDotProduct
(
    T1 *g_xdata,     // array of size n, type T1
    T2 *g_ydata,     // array of size n, type T2
    T3 *g_odata,       // array of size grid.x, type T3
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

    // each thread tid reduces its result into sum 
    T3 sum;

    // nothing to do
    if (block_start > block_end) { return ; }

    // convert global data pointer to the local pointer of this block
    T1 *xdata = g_xdata + block_start ;
    T2 *ydata = g_ydata + block_start ;

    T1 x0, x1, x2, x3, x4, x5, x6, x7 ;
    T2 y0, y1, y2, y3, y4, y5, y6, y7 ;

    if (block_end <= n)
    {
        // unrolling 8
        x0 = xdata [tid] ;
        x1 = xdata [tid +     s] ;
        x2 = xdata [tid + 2 * s] ;
        x3 = xdata [tid + 3 * s] ;
        x4 = xdata [tid + 4 * s] ;
        x5 = xdata [tid + 5 * s] ;
        x6 = xdata [tid + 6 * s] ;
        x7 = xdata [tid + 7 * s] ;

        y0 = ydata [tid] ;
        y1 = ydata [tid +     s] ;
        y2 = ydata [tid + 2 * s] ;
        y3 = ydata [tid + 3 * s] ;
        y4 = ydata [tid + 4 * s] ;
        y5 = ydata [tid + 5 * s] ;
        y6 = ydata [tid + 6 * s] ;
        y7 = ydata [tid + 7 * s] ;
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
        #define XDATA(i) ((i < lastblocksize) ? xdata [i] : MONOID_IDENTITY1)
        #define YDATA(i) ((i < lastblocksize) ? ydata [i] : MONOID_IDENTITY2)
        int lastblocksize = n - block_start ;
        x0 = XDATA (tid) ;
        x1 = XDATA (tid +     s) ;
        x2 = XDATA (tid + 2 * s) ;
        x3 = XDATA (tid + 3 * s) ;
        x4 = XDATA (tid + 4 * s) ;
        x5 = XDATA (tid + 5 * s) ;
        x6 = XDATA (tid + 6 * s) ;
        x7 = XDATA (tid + 7 * s) ;

        y0 = YDATA (tid) ;
        y1 = YDATA (tid +     s) ;
        y2 = YDATA (tid + 2 * s) ;
        y3 = YDATA (tid + 3 * s) ;
        y4 = YDATA (tid + 4 * s) ;
        y5 = YDATA (tid + 5 * s) ;
        y6 = YDATA (tid + 6 * s) ;
        y7 = YDATA (tid + 7 * s) ;
    }

    //work [tid] = mul(x0,y0) + mul(x1,y1) + mul(x2,y2) + mul(x3,y3)
    //               + mul(x4,y4) + mul(x5,y5) + mul(x6,y6)+ mul(x7,y7) ;
          sum  = ADD( MUL(x0,y0) , ADD( MUL(x1,y1) , ADD( MUL(x2,y2), 
                 ADD( MUL(x3,y3) , ADD( MUL(x4,y4) , ADD( MUL(x5,y5), 
                 ADD( MUL(x6,y6) , MUL(x7,y7)))))))) ;

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
            tid, sum,
            (double) (x0 + x1 + x2 + x3 + x4 + x5 + x6 + x7)) ;
        }
        */

    __syncthreads ( ) ;

    //--------------------------------------------------------------------------
    // reduce per-thread sums to a single scalar
    //--------------------------------------------------------------------------

    sum = block_ReduceSum<T3, 32>( this_thread_block(), sum); 

    // write result for this block to global mem
    if (tid == 0)
    {
        printf ("final %d : %g\n", b, (T3) sum) ;
        g_odata [b] = sum ;
    }
}

