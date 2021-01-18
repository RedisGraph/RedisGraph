//******************************************************************************
//  Sparse dot version of Matrix-Matrix multiply with mask 
//  Each thread in this kernel is responsible for m vector-pairs(x,y), 
//  finding intersections and producting the final dot product for each
//  using a serial merge algorithm on the sparse vectors. 
//  m = 256/sz, where sz is in {4, 16, 64, 256}
//  For a vector-pair, sz = xnz + ynz 
//  Template on <T_C, T_M, T_A, T_B>
//  Parameters:

//  int64_t start          <- start of vector pairs for this kernel
//  int64_t end            <- end of vector pairs for this kernel
//  int64_t *Bucket        <- array of pair indices for all kernels 
//  matrix<T_C> *C         <- result matrix 
//  matrix<T_M> *M         <- mask matrix
//  matrix<T_A> *A         <- input matrix A
//  matrix<T_B> *B         <- input matrix B
//  int sz                 <- nnz of very sparse vectors

//  Blocksize is 1024, uses warp and block reductions to count zombies produced.
//******************************************************************************
#define GB_KERNEL
#include <limits>
#include <cstdint>
#include <stdio.h>
#include <cooperative_groups.h>
#include "matrix.h"
#include "mySemiRing.h"

using namespace cooperative_groups;

template< typename T, int tile_sz>
__inline__ __device__ 
T warp_ReduceSumPlus( thread_block_tile<tile_sz> g, T val)
{
    // Each iteration halves the number of active threads
    // Each thread adds its partial sum[i] to sum[lane+i]
    for (int i = g.size() / 2; i > 0; i /= 2) {
        //printf("thd%d   %d OP %d is %d\n", threadIdx.x, val, fold, OP( val, fold));
        val +=  g.shfl_down( val, i);
    }
    return val; // note: only thread 0 will return full sum
}

template< typename T, int tile_sz>
__inline__ __device__ 
T warp_Reduce( thread_block_tile<tile_sz> g, T val)
{
    // Each iteration halves the number of active threads
    // Each thread adds its partial sum[i] to sum[lane+i]
    for (int i = g.size() / 2; i > 0; i /= 2) {
        T next = g.shfl_down( val, i) ;
        val = GB_ADD( sum, next ) ; 
    }
    return val; // note: only thread 0 will return full sum
}

template<typename T, int warpSize>
__inline__ __device__
T block_ReduceSum(thread_block g, T val)
{
  static __shared__ T shared[warpSize]; // Shared mem for 32 partial sums
  int lane = threadIdx.x & 31 ; // % warpSize;
  int wid  = threadIdx.x >> 5 ; // / warpSize;
  thread_block_tile<warpSize> tile = tiled_partition<warpSize>( g );

  // Each warp performs partial reduction
  val = warp_ReduceSumPlus<T, warpSize>( tile, val);    

  // Wait for all partial reductions
  if (lane==0) shared[wid]=val; // Write reduced value to shared memory
  __syncthreads();              // Wait for all partial reductions

  if (wid > 0 || gridDim.x == 1 ) return val;

  //read from shared memory only if that warp existed
  val = (threadIdx.x <  (blockDim.x / warpSize ) ) ? shared[lane] : 0;
  //printf("thd%d warp loaded val = %d\n", threadIdx.x, lane, val);

  if (wid==0) val = warp_ReduceSumPlus<T, warpSize>( tile, val); //Final reduce within first warp

  return val;
}

template< typename T_C, typename T_A, typename T_B, typename T_X, typename T_Y, typename T_Z>
__global__ void AxB_dot3_phase3_vsvs
( 
  const int64_t start, 
  const int64_t end,
  const int64_t *__restrict__ Bucket, 
  const GrB_Matrix C, 
  const GrB_Matrix M, 
  const GrB_Matrix A, 
  const GrB_Matrix B,
  const int sz 
)
{
   int dots = end - start;
   // sz = expected non-zeros per dot 
   /*
   int m = (gridDim.x*blockDim.x)*256/sz;
   int dpt = (nvecs+ gridDim.x*blockDim.x -1)/(gridDim.x*blockDim.x);
   m = dpt < m ? dpt : m;
   
   int dots = (nvecs +m -1)/m; 
   */
   const T_A *__restrict__ Ax = (T_A *)A->x  ;
   const T_B *__restrict__ Bx = (T_B *)B->x  ;
   T_C *__restrict__ Cx = (T_C *)C->x  ;
   int64_t *__restrict__ Ci = C->i ;
   const int64_t *__restrict__ Mi = M->i ;
   const int64_t *__restrict__ Ai = A->i ;
   const int64_t *__restrict__ Bi = B->i ;
   const int64_t *__restrict__ Ap = A->p ;
   const int64_t *__restrict__ Bp = B->p ;

   int pfirst, plast;

   GB_PARTITION (pfirst, plast, dots, blockIdx.x, gridDim.x ) ;
   /* 
   if( threadIdx.x ==0 )
   {
      printf("block%d %d dots/thrd, start,end = %ld,%ld pf,pl=%d,%d blockDim=%d\n",
               blockIdx.x, (dots + blockDim.x*gridDim.x -1)/(blockDim.x*gridDim.x), 
               start, end, pfirst, plast, blockDim.x);
   }
   __syncthreads();
   */
   

   int zc = 0 ;
     
   int64_t pair_id;

   //for ( int tid= threadIdx.x +blockDim.x*blockIdx.x;
   //          tid < dots;
   //          tid += blockDim.x * gridDim.x)
   for ( int tid = pfirst+ threadIdx.x ;
             tid < plast;
             tid += blockDim.x )
   {

         pair_id = Bucket[ start + tid ]; 

         int64_t i = Mi [pair_id] ;
         int64_t j = Ci [pair_id]>>4 ; 

         int64_t pA       = Ap[i] ;
         int64_t pA_end   = Ap[i+1] ;
         int64_t pB       = Bp[j] ; 
         int64_t pB_end   = Bp[j+1] ; 

         T_A aki;
         T_B bkj;
         T_Z cij ;

         bool cij_exists = false;

         while (pA < pA_end && pB < pB_end)
         {
            int64_t ia = Ai [pA] ;
            int64_t ib = Bi [pB] ;
            if( ia == ib)
            { 
                // A(k,i) and B(k,j) are the next entries to merge
                #if defined ( GB_PHASE_1_OF_2 )
                cij_exists = true ;
                break ;
                #else
                GB_DOT_MERGE ;
                //GB_DOT_TERMINAL (cij) ;         // break if cij == terminal
                pA++ ;
                pB++ ;
                #endif
            }
            else 
            {
                // A(ia,i) appears before B(ib,j)
                pA += ( ia < ib);
                // B(ib,j) appears before A(ia,i)
                pB += ( ib < ia);
            }
         }
         if (cij_exists){
            GB_PUTC ( Ci[pair_id] = i ) ;
            GB_PUTC ( Cx[pair_id] = (T_C)cij ) ;
         }
         else{
            //printf(" %lld, %lld is zombie %d!\n",i,j,zc);
            zc++; 
            GB_PUTC( Ci[pair_id] = GB_FLIP( i ) ) ;
         }
   }
  
   __syncthreads();

   //printf("thd%d zombie count = %d\n",threadIdx.x,zc);
   zc = block_ReduceSum<int , 32>( this_thread_block(), zc); 
   __syncthreads();

   if( threadIdx.x == 0 && zc > 0) {
      //printf("block%d zombie count = %d\n", blockIdx.x, zc);
      atomicAdd( (unsigned long long int*)&(C->nzombies), (unsigned long long int)zc);
      //C->nzombies += (unsigned long long int)zc;
      //printf("blk:%d Czombie = %lld\n", blockIdx.x,C->nzombies);
   }
   
}
