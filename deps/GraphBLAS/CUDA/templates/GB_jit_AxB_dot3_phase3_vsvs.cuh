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

#pragma once
#define GB_CUDA_KERNEL
#include <limits>
#include <cstdint>
#include <cmath>
#include <stdio.h>
#include <cooperative_groups.h>
#include "GB_cuda_kernel.h"
#include "GB_hash.h"
#include "GB_hyper_hash_lookup.h"

using namespace cooperative_groups;

// FIXME: move this out into its own *.cuh
template< typename T, int tile_sz>
__inline__ __device__ 
T GB_warp_ReduceSumPlus( thread_block_tile<tile_sz> g, T val)
{
    // Each iteration halves the number of active threads
    // Each thread adds its partial sum[i] to sum[lane+i]
    /*
    #pragma unroll
    for (int i = tile_sz >> 1; i > 0; i >>= 1) {
        val +=  g.shfl_down( val, i);
    }
    */
    val +=  g.shfl_down( val, 16);
    val +=  g.shfl_down( val, 8);
    val +=  g.shfl_down( val, 4);
    val +=  g.shfl_down( val, 2);
    val +=  g.shfl_down( val, 1);
    return val; // note: only thread 0 will return full sum
}

template<typename T, int warpSize>
__inline__ __device__
T GB_block_ReduceSum(thread_block g, T val)
{
  static __shared__ T shared[warpSize]; // Shared mem for 32 partial sums

  int lane = threadIdx.x & 31 ; // % warpSize;
  int wid  = threadIdx.x >> 5 ; // / warpSize;
  thread_block_tile<warpSize> tile = tiled_partition<warpSize>( g );

  // Each warp performs partial reduction
  val = GB_warp_ReduceSumPlus<T, warpSize>( tile, val);    

  // Wait for all partial reductions
  if (lane==0) shared[wid]=val; // Write reduced value to shared memory
  g.sync();                     // Wait for all partial reductions

  //if (wid > 0 ) return val;

  //read from shared memory only if that warp existed
  val = (threadIdx.x <  (blockDim.x / warpSize ) ) ? shared[lane] : 0;

  if (wid==0) val = GB_warp_ReduceSumPlus<T, warpSize>( tile, val); //Final reduce within first warp

  return val;
}

//------------------------------------------------------------------------------

template<
    typename T_C, typename T_A, typename T_B,
    typename T_Z, typename T_X, typename T_Y, uint64_t srcode>
__global__ void AxB_dot3_phase3_vsvs
( 
  int64_t start,
  int64_t end,
  int64_t *Bucket,  // do the work in Bucket [start:end-1]
  GrB_Matrix C,
  GrB_Matrix M,
  GrB_Matrix A,
  GrB_Matrix B,
  int sz            // unused
)
{

    // TODO: Figure out how to use graphblas-specific INFINITY macro
    #ifndef INFINITY
    #define INFINITY std::numeric_limits<T_C>::max()
    #endif

    int64_t dots = end - start;
   // sz = expected non-zeros per dot
//   /*
//   int m = (gridDim.x*blockDim.x)*256/sz;
//   int dpt = (nvecs+ gridDim.x*blockDim.x -1)/(gridDim.x*blockDim.x);
//   m = dpt < m ? dpt : m;
//
//   int dots = (nvecs +m -1)/m;
//   */
    const T_A *__restrict__ Ax = (T_A *)A->x  ;
    const T_B *__restrict__ Bx = (T_B *)B->x  ;
          T_C *__restrict__ Cx = (T_C *)C->x  ;
          int64_t *__restrict__ Ci = C->i ;
    const int64_t *__restrict__ Mi = M->i ;
    #if GB_M_IS_HYPER
    const int64_t *__restrict__ Mh = M->h ;
    #endif

    #if GB_A_IS_HYPER || GB_A_IS_SPARSE
    const int64_t *__restrict__ Ai = A->i ;
    const int64_t *__restrict__ Ap = A->p ;
    #endif

    #if GB_B_IS_HYPER || GB_B_IS_SPARSE
    const int64_t *__restrict__ Bi = B->i ;
    const int64_t *__restrict__ Bp = B->p ;
    #endif

    #if GB_A_IS_HYPER
    const int64_t *__restrict__ A_Yp = A->Y->p ;
    const int64_t *__restrict__ A_Yi = A->Y->i ;
    const int64_t *__restrict__ A_Yx = (int64_t *) A->Y->x ;
    const int64_t A_hash_bits = A->Y->vdim - 1 ;
    #endif

    #if GB_B_IS_HYPER
    const int64_t *__restrict__ B_Yp = B->Y->p ;
    const int64_t *__restrict__ B_Yi = B->Y->i ;
    const int64_t *__restrict__ B_Yx = (int64_t *) B->Y->x ;
    const int64_t B_hash_bits = B->Y->vdim - 1 ;
    #endif

    //int64_t pfirst, plast;

    //GB_PARTITION (pfirst, plast, dots, blockIdx.x, gridDim.x ) ;

    int64_t my_nzombies = 0 ;

    int all_in_one = ( (end - start) == (M->p)[(M->nvec)] ) ;

  //for ( int64_t kk = pfirst+ threadIdx.x ;
  //              kk < plast;
  //              kk += blockDim.x )
    for ( int64_t kk = start+ threadIdx.x +blockDim.x*blockIdx.x ;
                  kk < end;
                  kk += blockDim.x*gridDim.x )
    {
        int64_t pair_id = all_in_one ? kk : Bucket[ kk ];

        int64_t i = Mi [pair_id] ;
        int64_t k = Ci [pair_id]>>4 ;

        // j = k or j = Mh [k] if C and M are hypersparse
        #if GB_M_IS_HYPER
        int64_t j = Mh [k] ;
        #else
        int64_t j = k ;
        #endif

        // find A(:,i):  A is always sparse or hypersparse
        int64_t pA, pA_end ;
        #if GB_A_IS_HYPER
        GB_hyper_hash_lookup (Ap, A_Yp, A_Yi, A_Yx, A_hash_bits,
           i, &pA, &pA_end) ;
        #else
        pA       = Ap[i] ;
        pA_end   = Ap[i+1] ;
        #endif

        // find B(:,j):  B is always sparse or hypersparse
        int64_t pB, pB_end ;
        #if GB_B_IS_HYPER
        GB_hyper_hash_lookup (Bp, B_Yp, B_Yi, B_Yx, B_hash_bits,
           j, &pB, &pB_end) ;
        #else
        pB       = Bp[j] ;
        pB_end   = Bp[j+1] ;
        #endif

        GB_DECLAREA (aki) ;
        GB_DECLAREB (bkj) ;
        #if !GB_C_ISO
//      T_Z cij = GB_IDENTITY ;
        GB_DECLARE_MONOID_IDENTITY (cij) ;
        #endif

        bool cij_exists = false;

        while (pA < pA_end && pB < pB_end )
        {
            int64_t ia = Ai [pA] ;
            int64_t ib = Bi [pB] ;
            #if GB_IS_PLUS_PAIR_REAL_SEMIRING && GB_ZTYPE_IGNORE_OVERFLOW
                cij += (ia == ib) ;
            #else
                if (ia == ib)
                { 
                    // A(k,i) and B(k,j) are the next entries to merge
                    GB_DOT_MERGE (pA, pB) ;
                    GB_DOT_TERMINAL (cij) ;   // break if cij == terminal
                }
            #endif
            pA += ( ia <= ib);  // incr pA if A(ia,i) at or before B(ib,j)
            pB += ( ib <= ia);  // incr pB if B(ib,j) at or before A(ia,i)
        }

        GB_CIJ_EXIST_POSTCHECK ;
        if (cij_exists)
        {
            Ci[pair_id] = i ;
            GB_PUTC ( Cx[pair_id] = (T_C)cij ) ;
        }
        else
        {
            my_nzombies++;
            Ci[pair_id] = GB_FLIP( i ) ;
        }
    }

    // FIXME: use this in spdn and vsdn:
    this_thread_block().sync(); 

    my_nzombies = GB_block_ReduceSum<int64_t , 32>( this_thread_block(), my_nzombies);
    this_thread_block().sync(); 

    if( threadIdx.x == 0 && my_nzombies > 0)
    {
        atomicAdd( (unsigned long long int*)&(C->nzombies), (unsigned long long int)my_nzombies);
    }
}

