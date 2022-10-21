//------------------------------------------------------------------------------
// AxB_dot3_phase3_spdn.cu 
//------------------------------------------------------------------------------

// This CUDA kernel produces the semi-ring product of two
// sparse matrices of types T_A and T_B and common index space size n, to a  
// output matrix of type T_C. The matrices are sparse, with different numbers
// of non-zeros and different sparsity patterns. 
// ie. we want to produce C = A'*B in the sense of the given semi-ring.

// This version uses an entire threadblock to compute each C(i,j) dot product.

// Both the grid and block are 1D, so blockDim.x is the # threads in a
// threadblock, and the # of threadblocks is grid.x

//  int64_t start          <- start of vector pairs for this kernel
//  int64_t end            <- end of vector pairs for this kernel
//  int64_t *Bucket        <- array of pair indices for all kernels 
//  matrix<T_C> *C         <- result matrix 
//  matrix<T_M> *M         <- mask matrix
//  matrix<T_A> *A         <- input matrix A
//  matrix<T_B> *B         <- input matrix B

#pragma once

#include <limits>
#include <cstdint>
#include <cooperative_groups.h>
#include "GB_cuda_kernel.h"
#include "GB_hash.h"
#include "GB_hyper_hash_lookup.h"

// Using tile size fixed at compile time, we don't need shared memory
#define tile_sz 32 

using namespace cooperative_groups;

// FIXME: for the ANY monoid, GB_reduce_sum becomes trivial.
// or, if terminal condition is hit.

// FIXME: move this out of here, to share it
template< typename T, int warp_sz>
__device__ __inline__ 
T GB_reduce_sum(thread_block_tile<warp_sz> g, T val)
{
    // Each iteration halves the number of active threads
    // Each thread adds its partial sum[i] to sum[lane+i]
    // Temporary T is necessary to handle arbirary ops
    #pragma unroll
    for (int i = warp_sz >> 1; i > 0; i >>= 1)
    {
        T next = g.shfl_down( val, i);
        GB_ADD( val, val, next ); 
    }
    return val;
}

template< typename T, int warp_sz>
__device__ __inline__ 
T reduce_plus(thread_block_tile<warp_sz> g, T val)
{
    // Each iteration halves the number of active threads
    // Each thread adds its partial sum[i] to sum[lane+i]
    #pragma unroll
    for (int i = warp_sz >> 1; i > 0; i >>= 1)
    {
        val += g.shfl_down( val, i) ;
    }
    return val; // note: only thread 0 will return full sum and flag value
} 

template<
    typename T_C, typename T_A, typename T_B,
    typename T_Z, typename T_X, typename T_Y,
    uint64_t srcode>
__global__ void AxB_dot3_phase3_spdn
(
    int64_t start,
    int64_t end,
    int64_t *Bucket,    // do the work in Bucket [start:end-1]
    GrB_Matrix C,
    GrB_Matrix M,
    GrB_Matrix A,
    GrB_Matrix B,
    int sz              // FIXME: unused
)
{

    // TODO: Figure out how to use graphblas-specific INFINITY macro
    #ifndef INFINITY
    #define INFINITY std::numeric_limits<T_C>::max()
    #endif

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

    #if GB_A_IS_BITMAP
    const int8_t *__restrict__ Ab = A->b ;
    #endif

    #if GB_B_IS_HYPER || GB_B_IS_SPARSE
    const int64_t *__restrict__ Bi = B->i ;
    const int64_t *__restrict__ Bp = B->p ;
    #endif

    #if GB_B_IS_BITMAP
    const int8_t *__restrict__ Bb = B->b ;
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

    // zombie count
    int64_t zc = 0;

    int64_t pair_id;

    thread_block_tile<tile_sz> tile = tiled_partition<tile_sz>( this_thread_block());
    int all_in_one = ( (end - start) == (M->p)[(M->nvec)] ) ;

    // Main loop over pairs 
    int64_t kk ;
    for (kk = start+ blockIdx.x; // warp per C(i,j)=A(:,i)'*B(:,j) dot product
         kk < end;  
         kk += gridDim.x )
    {

        pair_id = all_in_one ? kk : Bucket [kk] ;
        int64_t i = Mi[pair_id];
        int64_t k = Ci[pair_id] >> 4;

        // j = k or j = Mh [k] if C and M are hypersparse
        #if GB_M_IS_HYPER
        int64_t j = Mh [k] ;
        #else
        int64_t j = k ;
        #endif

        // find A(:,i)
        int64_t pA, pA_end ;
        #if GB_A_IS_HYPER
        GB_hyper_hash_lookup (Ap, A_Yp, A_Yi, A_Yx, A_hash_bits,
            i, &pA, &pA_end) ;
        #elif GB_A_IS_SPARSE
        pA = Ap[i] ;
        pA_end   = Ap[i+1] ;
        #else
        // A is bitmap or full
        pA = A->vlen * i ;
        pA_end = pA + i ;
        #endif

        GB_DECLAREA (aki) ;
        GB_DECLAREB (bkj) ;
        #if !GB_C_ISO
//      T_Z cij = GB_IDENTITY ;
        GB_DECLARE_MONOID_IDENTITY (cij) ;
        #endif

        int cij_exists = 0 ;       // FIXME: make a bool

        // find B(:,j)
        int64_t pB, pB_end ;
        #if GB_B_IS_HYPER
        GB_hyper_hash_lookup (Bp, B_Yp, B_Yi, B_Yx, B_hash_bits,
           j, &pB, &pB_end) ;
        #elif GB_B_IS_SPARSE
        pB     = Bp[j] ;
        pB_end = Bp[j+1] ;
        #else
        // B is bitmap or full
        pB     = B->vlen * j ;
        pB_end = pB + j ;
        #endif

        //----------------------------------------------------------------------
        // compute C(i,j) = A(:,i)'*B(:,j) using the entire threadblock
        //----------------------------------------------------------------------

        #if ( GB_A_IS_FULL )
        {
//          int64_t bjnz = pB_end - pB ;    // bjnz = nnz (B (:,j))
//          if (bjnz > 0)                   // will always be >= 128
            {

                //--------------------------------------------------------------
                // A is full and B is sparse/hyper
                //--------------------------------------------------------------

                cij_exists = true ;
                for (int64_t p = pB + threadIdx.x ; p < pB_end ; p += blockDim.x)
                {
                    int64_t k = Bi [p] ;        // next row index of B(:,j)
                    // cij += A(k,i) * B(k,j)
                    GB_GETA ( aki, Ax, pA+k ) ;           // aki = A(k,i)
                    GB_GETB ( bkj, Bx, p ) ;              // bkj = B(k,j)
                    GB_MULTADD ( cij, aki, bkj, i, k, j ) ;        // cij += aki * bkj
                    GB_DOT_TERMINAL (cij) ;     // break if cij == terminal
                }
            }
        }
        #elif ( GB_A_IS_BITMAP )
        {
            //------------------------------------------------------------------
            // A is bitmap and B is sparse/hyper
            //------------------------------------------------------------------

            for (int64_t p = pB + threadIdx.x ; p < pB_end ; p += blockDim.x)
            {
                int64_t k = Bi [p] ;        // next row index of B(:,j)
                if (Ab [pA+k])              // check if A(k,i) exists
                {
                    // cij += A(k,i) * B(k,j)
                    GB_DOT_MERGE (pA+k, p) ;
                    GB_DOT_TERMINAL (cij) ;     // break if cij == terminal
                }
            }
        }
        #elif ( GB_B_IS_FULL )
        {
//          int64_t ainz = pA_end - pA ;    // ainz = nnz (A (:,i))
//          if (ainz > 0)                   // will always be >= 128
            {

                //--------------------------------------------------------------
                // A is sparse/hyper and B is full
                //--------------------------------------------------------------

                cij_exists = true ;
                for (int64_t p = pA + threadIdx.x ; p < pA_end ; p += blockDim.x)
                {
                    int64_t k = Ai [p] ;        // next row index of A(:,i)
                    // cij += A(k,i) * B(k,j)
                    GB_GETA ( aki, Ax, p ) ;              // aki = A(i,k)
                    GB_GETB ( bkj, Bx, pB+k) ;            // bkj = B(j,k)
                    GB_MULTADD ( cij, aki, bkj, i, k, j) ;         // cij += aik * bjk
                    GB_DOT_TERMINAL (cij) ;     // break if cij == terminal
                }
            }
        }
        #elif ( GB_B_IS_BITMAP )
        {

            //------------------------------------------------------------------
            // A is sparse/hyper and B is bitmap
            //------------------------------------------------------------------

            for (int64_t p = pA + threadIdx.x ; p < pA_end ; p += blockDim.x)
            {
                int64_t k = Ai [p] ;        // next row index of A(:,i)
                if (Bb [pB+k])              // check if B(k,j) exists
                {
                    // cij += A(k,i) * B(k,j)
                    GB_DOT_MERGE (p, pB+k) ;
                    GB_DOT_TERMINAL (cij) ;     // break if cij == terminal
                }
            }
        }
        #endif

        GB_CIJ_EXIST_POSTCHECK

        //----------------------------------------------------------------------
        // reduce sum per-thread values to a single scalar, get OR of flag
        //----------------------------------------------------------------------

        /*
        if (threadIdx.x == 0)
        {
            printf ("reduce %d : %d exists = %d\n", b,  cij, cij_exists) ;
        }
        __syncthreads();
        */

        // Do vote here for control.
        cij_exists = tile.any (cij_exists) ;
        tile.sync ( ) ;

        #if !GB_C_ISO
        if (cij_exists)
        {
           cij = GB_reduce_sum<T_Z, tile_sz>( tile, cij );
        }
        #endif

        // write result for this block to global mem
        if (threadIdx.x == 0)
        {
            if (cij_exists)
            {
               GB_PUTC ( Cx[pair_id]=(T_C)cij ) ;
               Ci[pair_id] = i ;
            }
            else
            {
               zc++;
               Ci[pair_id]=GB_FLIP (i) ;
            }
        }
        //__syncthreads(); 
    }

    //--------------------------------------------------------------------------

    if(threadIdx.x ==0 && zc > 0)
    {
        // printf("warp %d zombie count = %d, nzombies = %d\n", blockIdx.x, zc, C->nzombies);
        atomicAdd( (unsigned long long int*)&(C->nzombies), (unsigned long long int)zc);
        // printf(" Czombie = %lld\n",C->nzombies);
    }

  //__syncthreads();
}

