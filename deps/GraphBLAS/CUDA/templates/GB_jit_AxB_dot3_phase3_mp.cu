//------------------------------------------------------------------------------
// AxB_dot3_phase3_mp.cu 
//------------------------------------------------------------------------------

// This CUDA kernel produces the semi-ring product of two
// sparse matrices of types T_A and T_B and common index space size n, to a  
// output matrix of type T_C. The matrices are sparse, with different numbers
// of non-zeros and different sparsity patterns. 
// ie. we want to produce C = A'*B in the sense of the given semi-ring.

// This version uses a merge-path algorithm, when the sizes nnzA and nnzB are 
// relatively close in size, neither is very spare nor dense, for any size of N.
// Handles arbitrary sparsity patterns with guaranteed load balance.

// Both the grid and block are 1D, so blockDim.x is the # threads in a
// threadblock, and the # of threadblocks is grid.x

// Let b = blockIdx.x, and let s be blockDim.x. s= 32 with a variable number
// of active threads = min( min(g_xnz, g_ynz), 32) 

// Thus, threadblock b owns a part of the index set spanned by g_xi and g_yi.  Its job
// is to find the intersection of the index sets g_xi and g_yi, perform the semi-ring dot
// product on those items in the intersection, and finally reduce this data to a scalar, 
// on exit write it to g_odata [b].

//  int64_t start          <- start of vector pairs for this kernel
//  int64_t end            <- end of vector pairs for this kernel
//  int64_t *Bucket        <- array of pair indices for all kernels 
//  matrix<T_C> *C         <- result matrix 
//  matrix<T_M> *M         <- mask matrix
//  matrix<T_A> *A         <- input matrix A
//  matrix<T_B> *B         <- input matrix B
#include <limits>
#include <cstdint>
#include <cooperative_groups.h>
#include "mySemiRing.h"
#include "matrix.h"

// Using tile size fixed at compile time, we don't need shared memory
#define tile_sz 32 

using namespace cooperative_groups;

template< typename T, int warp_sz>
__device__ __inline__ 
T GB_reduce_sum(thread_block_tile<warp_sz> g, T val)
{
    // Each iteration halves the number of active threads
    // Each thread adds its partial sum[i] to sum[lane+i]
    for (int i = g.size() / 2; i > 0; i /= 2)
    {
        T next = g.shfl_down( val, i);
        val = GB_ADD( val, next ) ;
    }
    return val;
}

template< typename T, int warp_sz>
__device__ __inline__ 
T reduce_plus(thread_block_tile<warp_sz> g, T val)
{
    // Each iteration halves the number of active threads
    // Each thread adds its partial sum[i] to sum[lane+i]
    for (int i = g.size() / 2; i > 0; i /= 2)
    {
        val += g.shfl_down( val, i) ;
    }
    return val; // note: only thread 0 will return full sum and flag value
}

#define intersects_per_thread 8

template< typename T_C, typename T_A, typename T_B, typename T_X, typename T_Y, typename T_Z>  
__global__ void AxB_dot3_phase3_mp
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
    int64_t *Ci = C->i;
    int64_t *Mi = M->i;
    int64_t *Ai = A->i;
    int64_t *Bi = B->i;
    int64_t *Ap = A->p;
    int64_t *Bp = B->p;


    // zombie count
    int zc = 0;

    int64_t pair_id;

    // set thread ID
    int tid_global = threadIdx.x+ blockDim.x* blockIdx.x;
    int tid = threadIdx.x;

    int b = blockIdx.x ;

    // total items to be inspected
    int64_t nnzA = 0;
    int64_t nnzB = 0;
    int64_t n_intersect = 0;

    thread_block_tile<tile_sz> tile = tiled_partition<tile_sz>( this_thread_block());

    int parts = blockDim.x; //(n_intersect+ intersects_per_thread -1)/ intersects_per_thread; 

    // int has_zombies = 0 ;

    // Main loop over pairs 
    for (pair_id = start+ blockIdx.x; //warp per pair 
         pair_id < end;  
         pair_id += gridDim.x )
    {

         int64_t i = Mi[pair_id];
         int64_t j = Ci[pair_id] >> 4;

         int64_t xstart = Ap[i];
         int64_t xend   = Ap[i+1];
         nnzA = xend - xstart;

         int64_t ystart = Bp[j]; 
         int64_t yend   = Bp[j+1]; 
         nnzB = yend - ystart;

         n_intersect = GB_IMIN( xend -xstart, yend -ystart); 
    /* 
    if (threadIdx.x ==0 ) {
      printf("block %d  doing dot %lld  i,j= %lld,%lld\n", blockIdx.x, pair_id, i, j);
    }
    */
    //we want more than one intersection per thread
    int64_t nxy = nnzA + nnzB;

    int work_per_thread = (nxy +parts -1)/parts;
    int diag = GB_IMIN( work_per_thread*tid, nxy);
    int diag_end = GB_IMIN( diag + work_per_thread, nxy);
    //printf(" thd%d parts = %u wpt = %u diag, diag_end  = %u,%u\n",tid, parts, work_per_thread, diag, diag_end); 

    int x_min = GB_IMAX( (int)(diag - nnzB), 0);
    int x_max = GB_IMIN( diag, nnzA);

    //printf("start thd%u x_min = %u x_max = %u\n", tid_global, x_min,x_max);
    while ( x_min < x_max) { //binary search for correct diag break
      int pivot = (x_min +x_max)/2;
      if ( Ai[pivot + xstart] < Bi[ diag -pivot -1 + ystart]) {
         x_min = pivot +1;
      }
      else {
         x_max = pivot;
      }
    }
    int xcoord = x_min;
    int ycoord = diag -x_min -1;
    if (( diag > 0) &&(diag < (nnzA+nnzB)) && (Ai[xcoord+xstart] == Bi[ycoord+ystart]) ) { 
       diag--; //adjust for intersection incrementing both pointers 
    }
    // two start points are known now
    int tx_start = xcoord +xstart;
    int ty_start = diag -xcoord +ystart; 

    //if (x_start != y_start)
    //   printf("start thd%u  xs,ys = %i,%i\n", tid_global, x_start, y_start);

    x_min = GB_IMAX( (int)(diag_end - nnzB), 0);
    x_max = GB_IMIN( diag_end, nnzA);

    while ( x_min < x_max) {
       int pivot = (x_min +x_max)/2;
       //printf("thd%u pre_sw piv=%u diag_e = %u  xmin,xmax=%u,%u\n", tid_global, pivot, diag_end,x_min, x_max);
       if ( Ai[pivot+ xstart] < Bi[ diag_end -pivot -1 +ystart]) {
          x_min = pivot +1;
       }
       else {
          x_max = pivot;
       }
       //printf("thd%u piv=%u xmin,xmax = %u,%u\n", tid_global, pivot, x_min, x_max);
    }
    xcoord = x_min;
    ycoord = diag_end -x_min -1;
    if ( (diag_end < (nnzA +nnzB)) && (Ai[xcoord +xstart] == Bi[ycoord + ystart]) ) { 
        diag--; //adjust for intersection incrementing both pointers  
    }
    // two end points are known now
    int tx_end = xcoord +xstart; 
    int ty_end = diag_end - xcoord + ystart; 

    T_A aki;
    T_B bkj;
    T_Z cij = GB_IDENTITY ;

    // TODO PLUS_PAIR_INT64, FP32, FP64: no need for cij_exists.
    // just check if cij > 0

    int cij_exists  = 0 ;
    //printf(" thd%u has init value %f\n",tid, cij);

    //merge-path dot product
    int k = tx_start;
    int l = ty_start;
    while ( k < tx_end && l < ty_end )
    {
       if (Ai [k] == Bi [l])
       {
          GB_GETA ( aki=(T_Z)Ax[k] ) ;
          GB_GETB ( bkj=(T_Z)Bx[l] ) ;
          if (cij_exists)
          {
            T_Z t = GB_MULT( (T_Z)aki, (T_Z)bkj );
            GB_ADD_F (cij, t ) ;
          //printf("  thd%d ix at %lld   cij += %d * %d \n", tid_global, Ai[k], aki, bkj);
          }
          else
          {
            cij_exists = 1 ;
            cij = GB_MULT ( (T_Z)aki, (T_Z)bkj ) ;
          //printf("  thd%d ix at %lld   cij = %d * %d \n", tid_global, Ai[k], Ax[k], Bx[l]);
          }
          // TODO check terminal condition
          k+= 1;
          l+= 1;
          //printf(" block%u work value = %d, exists = %d\n", b, cij, cij_exists);
       }
       else
       {
            k += ( Ai[k] < Bi[l] ) ;
            l += ( Ai[k] > Bi[l] ) ;
       }
    }

    //tile.sync( ) ;
    //--------------------------------------------------------------------------
    // reduce sum per-thread values to a single scalar, get OR of flag
    //--------------------------------------------------------------------------
    /*
    if (tid == 0)
    {
        printf ("reduce %d : %d exists = %d\n", b,  cij, cij_exists) ;
    }
    __syncthreads();
    */

    // Do vote here for control.
    cij_exists  = tile.any( cij_exists);
    //tile.sync();

    if (cij_exists)
    {
       cij = GB_reduce_sum<T_Z, tile_sz>( tile, cij );
       
    }
    // else has_zombies = 1;


    //__syncthreads();
    //tile.sync( );
    // write result for this block to global mem
    if (tid == 0)
    {
        //printf ("final %d : %d exists = %d\n", b,  cij, cij_exists) ;
        if (cij_exists)
        {
           //printf(" cij = %d\n", cij);
           GB_PUTC ( Cx[pair_id]=(T_C)cij ) ;
           GB_PUTC ( Ci[pair_id]=i ) ;
        }
        else
        {
           //printf(" dot %d is a zombie\n", pair_id);
           zc++;
           GB_PUTC ( Ci[pair_id]=GB_FLIP (i) ) ;
        }
    }
    //__syncthreads(); 
  }

//--------------------------------------------------------------------------

  if( tid ==0 && zc > 0)
  {
      //printf("warp %d zombie count = %d\n", blockIdx.x, zc);
      atomicAdd( (unsigned long long int*)&(C->nzombies), (unsigned long long int)zc);
      //printf(" Czombie = %lld\n",C->nzombies);
  }

  //__syncthreads();

}

