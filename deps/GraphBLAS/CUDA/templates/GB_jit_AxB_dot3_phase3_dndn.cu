
//------------------------------------------------------------------------------
// AxB_dot3_phase3_dndn.cu 
//------------------------------------------------------------------------------

// This CUDA kernel produces the semi-ring product of two
// sparse matrices of types T_A and T_B and common index space size n, to a  
// output matrix of type T_C. The matrices are sparse, with different numbers
// of non-zeros and different sparsity patterns. 
// ie. we want to produce C = A'*B in the sense of the given semi-ring.

// This version uses a simple warp-based dense dot product algorithm, when the
// vectors coming from both A and B are dense, for any size of N.

// Both the grid and block are 1D, so blockDim.x is the # threads in a
// threadblock, and the # of threadblocks is grid.x

// Let b = blockIdx.x, and let s be blockDim.x. s= 32 with a variable number
// of active threads = min( min(nzA, nzB), 32) 

// Thus, threadblock b owns a semi-ring dot product on a pair of vectors. 
// The work is to load the data, do the multiply and add work and finally 
// reduce this data to a scalar, and write it to Cx[pair].

//  int64_t start          <- start of vector pairs for this kernel
//  int64_t end            <- end of vector pairs for this kernel
//  int64_t *Bucket        <- array of pair indices for all kernels 
//  GrB_Matrix C           <- result matrix 
//  GrB_Matrix M           <- mask matrix
//  GrB_Matrix A           <- input matrix A
//  GrB_Matrix B           <- input matrix B
//  int sz                 <- size parameter (not used) 

#include <limits>
#include <cstdint>
#include <cooperative_groups.h>
#include "matrix.h"
#include "mySemiRing.h"

// Using tile size fixed at compile time, we don't need shared memory
#define tile_sz 32 

using namespace cooperative_groups;

template< typename T, int warp_sz>
__inline__ __device__ T warp_ReduceSum(thread_block_tile<warp_sz> g, T val)
{
    // Each iteration halves the number of active threads
    // Each thread adds its partial sum[i] to sum[lane+i]
    for (int i = g.size() / 2; i > 0; i /= 2)
    {
        T next = g.shfl_down( val, i) ;
        val = GB_ADD( val, next ); 
    }
    return val; // note: only thread 0 will return full sum
}

template<typename T, int warpSize >
__inline__ __device__
T block_ReduceSum(thread_block g, T val, T Ident)
{
  static __shared__ T shared[warpSize]; // Shared mem for 32 partial sums
  int lane = threadIdx.x % warpSize;
  int wid = threadIdx.x / warpSize;
  thread_block_tile<warpSize> tile = tiled_partition<warpSize>(g);

  // Each warp performs partial reduction
  val = warp_ReduceSum< T, warpSize>(tile, val);    

  if (lane==0) shared[wid] = val; // Write reduced value to shared memory

  //tile.sync();                    // Wait for all partial reductions

  if (wid > 0 || gridDim.x == 1 ) return val;

  //read from shared memory only if that warp existed
  val = (threadIdx.x < blockDim.x / warpSize) ? shared[lane] :  Ident  ;

  if (wid==0) val = warp_ReduceSum< T, warpSize>(tile,val); //Final reduce within first warp

  return val;
}


template< typename T_C, typename T_A, typename T_B, typename T_X, typename T_Y, typename T_Z>
__global__ void AxB_dot3_phase3_dndn 
(
    int64_t start,
    int64_t end,
    int64_t *Bucket,
    GrB_Matrix C,
    GrB_Matrix M,
    GrB_Matrix A,
    GrB_Matrix B,
    int sz
)
{

    T_A *Ax = (T_A*)A->x;
    T_B *Bx = (T_B*)B->x;
    T_C *Cx = (T_C*)C->x;
    int64_t *Mi = M->i;
    int64_t *Ci = C->i;
    int64_t *Ap = A->p;
    int64_t *Bp = B->p;

    // zombie count
    int zc = 0;
    int64_t pair_id;

    // total items to be inspected
    int64_t nnzA = 0;
    int64_t nnzB = 0;
    int s = blockDim.x;

    // Main loop over pairs 
    for (pair_id = start + blockIdx.x; //warp per pair 
         pair_id < end;  
         pair_id += gridDim.x ){

         int64_t i = Mi[pair_id];
         int64_t j = Ci[pair_id] >> 4;

         int64_t pA = Ap[i];
         int64_t xend   = Ap[i+1];
         nnzA = xend - pA;

         int64_t pB = Bp[j]; 
         int64_t yend   = Bp[j+1]; 
         nnzB = yend - pB;

    /*
    if (threadIdx.x == 0 ){
        printf(" i,j = %d,%d  nnz= %d xstart,end = %d,%d  ystart,end = %d,%d\n",
            (int)i,(int)j,  (int)nnzA, (int)xstart,(int)xend, (int)ystart, (int)yend);
    }
    __syncthreads();                                          
    */

    
    // convert global data pointer to the local pointer of this block
    T_A  aki; // *xdata = &Ax[xstart]; 
    T_B  bkj; // *ydata = &Bx[ystart];
    T_Z  cij;

    GB_GETA ( aki=(T_Z)Ax[pA+threadIdx.x] ) ;             // aki = A(0,i)
    GB_GETB ( bkj=(T_Z)Bx[pB+threadIdx.x] ) ;             // bkj = B(0,j)
    GB_C_MULT ( cij, aki, bkj ) ;                        // cij = aki * bkj

    for ( int tid = threadIdx.x + s; tid < nnzA; tid+= s) { 
          // cij += A(k,i) * B(k,j)
          // GB_DOT_TERMINAL ( cij ) ;             // break if cij == terminal
          GB_GETA ( aki=(T_Z)Ax[pA+tid] ) ;         // aki = A(k,i)
          GB_GETB ( bkj=(T_Z)Bx[pB+tid] ) ;        // bkj = B(k,j)
          GB_MULTADD ( cij, aki, bkj ) ;        // cij += aki * bkj
    }


    //--------------------------------------------------------------------------
    // reduce per-thread sums to a single scalar
    //--------------------------------------------------------------------------
    thread_block_tile<32> tile = tiled_partition<32>( this_thread_block() );
    cij = warp_ReduceSum<T_Z, 32> ( tile, cij);

    // write result for this block to global mem
    if (threadIdx.x == 0)
    {
       //printf("tid: %d final sum after reduce = %d\n", threadIdx.x, sum);
       GB_PUTC( Cx[pair_id]=(T_C)cij ) ;
       GB_PUTC( Ci[pair_id]=i ) ;
    }
    //__syncthreads ( ) ;
  }

}

