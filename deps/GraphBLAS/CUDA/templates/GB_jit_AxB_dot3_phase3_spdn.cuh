//******************************************************************************
//  Sparse dot products in batch form, sparse - dense case. 
//  Each thread in this kernel is responsible for m vector-pairs(x,y), 
//  m = 256/sz, where sz is in {4, 16, 64, 256}
//  We know each non-zero on the sparse side will hit a dense value.
//  Template on <T_C, T_A, T_B, T_X, T_Y, T_Z >
//  Parameters:

//  matrix<T_C> *C         <- C result matrix 
//  matrix<T_C> *M         <- Mask matrix 
//  matrix<T_A> *A         <- A matrix to multiply, sparse 
//  matrix<T_B> *B         <- B matrix to multiply, dense in sparse format? 
//******************************************************************************

/* fixme: This kernel needs to be split into 4 methods.  Perhaps a single
    file with #ifdef's could be used to keep the code size down.

        (A sparse or hypersparse) * (B bitmap)
        (A sparse or hypersparse) * (B full)
        (A bitmap) * (B sparse or hypersparse)
        (A full) * (B sparse or hypersparse)

    The buckets are not needed, unless the sparse matrix needs to be
    split into "very sparse vectors" (one thread per dot) and "longer
    sparse vectors" (one warp or threadblock cooperates on a single dot).
    Then only 2 buckets are needed ... or the work could be done in a single
    pass, and the test for these 2 cases could be done on the fly.

    The buckets are entirely different from the general case when both A and
    B are sparse.

    C and M would still be sparse or hypersparse.
*/

#pragma once

#include <limits>
#include <cstdint>
#include <stdio.h>
#include "GB_cuda_kernel.h"
#include "GB_hash.h"
#include "GB_hyper_hash_lookup.h"

#include <cooperative_groups.h>

#define tile_sz 32

//#include "local_cub/block/block_reduce.cuh"


using namespace cooperative_groups;

// TODO: Put this in a shared location
template< typename T, int warpSize >
__device__ T reduce_sum(thread_block_tile<warpSize> g, T val)
{
    // Each iteration halves the number of active threads
    // Each thread adds its partial sum[i] to sum[lane+i]
    for (int i = g.size() / 2; i > 0; i /= 2)
    {
        val += g.shfl_down(val,i) ;
    }
    return val; // note: only thread 0 will return full sum
}


template<
    typename T_C, typename T_A, typename T_B,
    typename T_Z, typename T_X, typename T_Y,
    uint64_t srcode>
__global__ void AxB_dot3_phase3_spdn
( 
  GrB_Matrix C, 
  GrB_Matrix M, 
  GrB_Matrix A, 
  GrB_Matrix B
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

//   typedef cub::BlockReduce<int, 32> BlockReduce;
//   __shared__ typename BlockReduce::TempStorage temp_storage;

//   if( threadIdx.x ==0)
//      printf("thd:%d %d dots/thrd, nvec = %d blockDim=%d\n",threadIdx.x, sz, nvec, blockDim.x);
//   __syncthreads();

    int64_t pair_id; 

    int64_t start = 0;
    int64_t end = M->p[M->nvec];
//       if (threadIdx.x ==0)
//         printf("thd%u pi=%lld\n",tid, start+threadIdx.x);
//       __syncthreads();

    for (int64_t kk = start +threadIdx.x +blockIdx.x*blockDim.x; 
                 kk < end ;  
                 kk += gridDim.x*blockDim.x  )
    {

        pair_id =  kk ;
        int64_t i = Mi[pair_id];  // cols from mask
        int64_t k = Ci[pair_id] >> 4;  // vector of C previously encoded in phase1

        // j = k or j = Mh [k] if C and M are hypersparse
        #if GB_M_IS_HYPER
        int64_t j = Mh [k] ;
        #else
        int64_t j = k ;
        #endif

        //printf("tid=%d, i=%lu, j=%lu\n", threadIdx.x, i, j);
        //      if (threadIdx.x ==0)
        //         printf("thd%u i,j=%lld,%lld\n",tid, i,j);
        //      __syncthreads();

        // Prep row offsets for both A and B

        // find A(:,i)
        int64_t pA, pA_end ;
        #if GB_A_IS_HYPER
        GB_hyper_hash_lookup (Ap, A_Yp, A_Yi, A_Yx, A_hash_bits,
            i, &pA, &pA_end) ;
        #elif GB_A_IS_SPARSE
        pA     = Ap[i] ;
        pA_end = Ap[i+1] ;
        #else
        // A is bitmap or full
        pA     = (A->vlen)*i;
        pA_end = pA +(A->vlen);
        #endif

        int64_t nnzA   = pA_end - pA;

        // find B(:,j)
        int64_t pB, pB_end ;
        #if GB_B_IS_HYPER
        GB_hyper_hash_lookup (Bp, B_Yp, B_Yi, B_Yx, B_hash_bits,
            j, &pB, &pB_end) ;
        #elif GB_B_IS_SPARSE
        pB       = Bp[j];   // col of C
        pB_end   = Bp[j+1];
        #else
        // B is bitmap or full
        pB   = (B->vlen)*j;
        pB_end = pB +(B->vlen);
        #endif

        int64_t nnzB = pB_end - pB;

        GB_DECLAREA (aki) ;
        GB_DECLAREB (bkj) ;
        T_Z cij = GB_IDENTITY ;

        int zombie_count = 0;

        if (nnzA == 0 || nnzB == 0)
        {
            i = GB_FLIP (i) ;
            zombie_count +=1;
        }
        else if( nnzA == A->vlen) // A is dense
        {
            /**
            * A is dense, iterate over columns of B, applying monoid and binary op to current
            */
            int64_t k = Bi [pB] ;               // first row index of B(:,j)
            // cij = A(k,i) * B(k,j)
            GB_GETA ( aki, Ax, pA+k ) ;         // aki = A(k,i)
            GB_GETB ( bkj, Bx, pB ) ;           // bkj = B(k,j)

            // TODO: Check tha GB_C_MULT applies the identity automatically since cij has not been initialized
            GB_C_MULT ( cij, aki, bkj ) ;           // cij = aki * bkj

            printf("A_dense: tid=%d, pair_id=%d, i=%lu, j=%lu, nnzA=%lu, nnzB=%lu, k[B]=%lu, aki=%d, bkj=%d, cij=%d\n", threadIdx.x, pair_id, i, j, nnzA, nnzB, k, aki, bkj, cij);

            for (int64_t p = pB+1 ; p < pB_end ; ++p)
            {
                //GB_DOT_TERMINAL (cij) ;             // break if cij == terminal
                int64_t k = Bi [p] ;                // next row index of B(:,j)
                // cij += A(k,i) * B(k,j)
                GB_GETA ( aki, Ax, pA+k ) ;           // aki = A(k,i)
                GB_GETB ( bkj, Bx, p ) ;              // bkj = B(k,j)
                GB_MULTADD ( cij, aki, bkj ) ;        // cij += aki * bkj
                //printf("in_loop: tid=%d, pair_id=%d, i=%lu, j=%lu, nnzA=%lu, nnzB=%lu, k[B]=%lu, aki=%d, bkj=%d, cij=%d\n", threadIdx.x, pair_id, i, j, nnzA, nnzB, k, aki, bkj, cij);
            }

        }
        else if( nnzB == B->vlen) // B is dense
        {
            int64_t k = Ai [pA] ;               // first col index of A(i, :)
            // cij = A(i,k) * B(j,k)
            GB_GETA ( aki, Ax, pA ) ;           // aki = A(i,k)

            // Jump straight to position in B vector (since we know it's dense)
            GB_GETB ( bkj, Bx, pB+k ) ;           // bkj = B(k,j)

            GB_C_MULT ( cij, aki, bkj) ;           // cij = aki * bkj
            //printf("B_dense: tid=%d, pair_id=%d, i=%lu, j=%lu, nnzA=%lu, nnzB=%lu, k[B]=%lu, aki=%d, bkj=%d, cij=%d\n", threadIdx.x, pair_id, i, j, nnzA, nnzB, k, aki, bkj, cij);

            for (int64_t p = pA+1 ; p < pA_end ; ++p)
            {
                //GB_DOT_TERMINAL (cij) ;             // break if cij == terminal
                int64_t k = Ai [p] ;                // next row index of A(:,i)
                // cij += A(k,i) * B(k,j)
                GB_GETA ( aki, Ax, p ) ;              // aki = A(i,k)
                GB_GETB ( bkj, Bx, pB+k) ;            // bkj = B(j,k)
                GB_MULTADD ( cij, aki, bkj) ;         // cij += aik * bjk
                //printf("in_loop: tid=%d, pair_id=%d, i=%lu, j=%lu, nnzA=%lu, nnzB=%lu, k[B]=%lu, aki=%d, bkj=%d, cij=%d\n", threadIdx.x, pair_id, i, j, nnzA, nnzB, k, aki, bkj, cij);
            }
        }

        // C(i,j) = A(:,i) * B(:,j)
        //        /**
        //         * If A is bitmap, we need to look up offset and nnz of B
        //         * and treat Ax as fully dense (size=n*k).
        //         */
        //
        //        // TODO: We probably want to pull this into a separate and
        //        //  much smaller kernel just for these formats (e.g. spdn w/ B=bitmap, A=sparse)
        //        #if ( GB_A_IS_BITMAP ) // A is dense
        //        {
        //             int64_t pB = Bp[i];
        //             int64_t pB_end   = Bp[i+1];
        //             int64_t nnzB   = pB_end - pB;
        //             int64_t k = Bi [pB] ;               // first row index of B(:,j)
        //            // cij = A(k,i) * B(k,j)
        //
        ////             printf("tid=%d, A is dense, k=%ld, i=%ld\n", threadIdx.x, k, i);
        //            GB_GETA ( aki, Ax,pA + i ) ;           // aki = A(k,i)
        //            GB_GETB ( bkj, Bx,pB ) ;           // bkj = B(k,j)
        //            cij = GB_MULT(aki, bkj ) ;           // cij = aki * bkj
        //
        //        }
        //
        //        //TODO: We probably want to pull this into a separate
        //        // much smaller kernel just for these formats (e.g. spdn w/ B=full, A=sparse)
        //        /**
        //         * If A is full, we need to look up offset and nzz of B
        //         * and treat Ax as fully dense (size=n*k)
        //         */
        //        #elif ( GB_A_IS_FULL ) // A is dense
        //        {
        //             int64_t pB = Bp[i];
        //             int64_t pB_end   = Bp[i+1];
        //             int64_t nnzB   = pB_end - pB;
        //
        //            int64_t k = Bi [pB] ;               // first row index of B(:,j)
        //            // cij = A(k,i) * B(k,j)
        //
        ////             printf("tid=%d, A is dense, k=%ld, i=%ld\n", threadIdx.x, k, i);
        //            GB_GETA ( aki, Ax,pA + i ) ;           // aki = A(k,i)
        //            GB_GETB ( bkj, Bx,pB ) ;           // bkj = B(k,j)
        //            cij = GB_MULT(aki, bkj ) ;           // cij = aki * bkj
        //
        //            for (int64_t p = pB+1 ; p < pB_end ; p++)
        //            {
        //                //GB_DOT_TERMINAL (cij) ;           // break if cij == terminal
        //                int64_t k = Bi [p] ;                // next row index of B(:,j)
        //                // cij += A(k,i) * B(k,j)
        //                GB_GETA ( aki, Ax,A->vlen * i + k ) ;      // aki = A(k,i)
        //                GB_GETB ( bkj, Bx,p ) ;                    // bkj = B(k,j)
        //                cij = GB_ADD ( cij, GB_MULT(aki, bkj ) ) ;      // cij += aki * bkj
        //            }
        //        }
        //
        //        /**
        //         * If A is sparse but current row of A is dense, we need to look up
        //         * offset of B and offset of A
        //         */
        //        #elif (GB_B_IS_BITMAP)
        //        {
        //
        //        }
        //
        //        #elif (GB_B_IS_FULL)
        //        {
        //
        //        }
        //
        //        /**
        //         * If
        //         */
        //        #else
        //        {
        //
        //
        //             int64_t pA = Ap[i];
        //             int64_t pA_end   = Ap[i+1];
        //             int64_t nnzA   = pA_end - pA;
        //
        //            int64_t k = Ai [pA] ;               // first row index of A(:,i)
        ////             printf("tid=%d, B is dense, k=%ld, j=%ld\n", threadIdx.x, k, j);
        //            // cij = A(k,i) * B(k,j)
        //            GB_GETA ( aki, Ax, pA  ) ;           // aki = A(k,i)
        //            GB_GETB ( bkj, Bx, B->vlen*k+j ) ;           // bkj = B(k,j)
        //
        //            cij =  GB_MULT(aki, bkj) ;           // cij = aki * bkj
        ////             printf("aki=%d, bkj=%d, cij=%d\n", aki, bkj, cij);
        //
        //            for (int64_t p = pA+1 ; p < pA_end ; p++)
        //            {
        //                //GB_DOT_TERMINAL (cij) ;             // break if cij == terminal
        //                int64_t k = Ai [p] ;                // next row index of A(:,i)
        //                // cij += A(k,i) * B(k,j)
        //                GB_GETA ( aki,Ax, p  ) ;           // aki = A(k,i)
        //                GB_GETB ( bkj,Bx, B->vlen*k+j ) ;           // bkj = B(k,j)
        //                cij = GB_ADD ( cij, GB_MULT(aki, bkj) );        // cij += aki * bkj
        ////                printf("aki=%d, bkj=%d, cij=%d\n", aki, bkj, cij);
        //            }
        //         } else {
        //             if(threadIdx.x == 0 && blockIdx.x == 0) {
        //                 printf("ERROR: At least one side must be dense.\n");
        //                 break;
        //             }
        //         }

        Ci[pair_id]=i ;
        GB_PUTC( Cx[pair_id]=(T_C) cij ) ;

        //         int zc = BlockReduce(temp_storage).Sum(zombie_count);
        thread_block_tile<tile_sz> tile = tiled_partition<tile_sz>( this_thread_block());
        int zc = reduce_sum<int,tile_sz>(tile, zombie_count);

        if(threadIdx.x == 0 && zc > 0)
        atomicAdd(&(C->nzombies), zc);
    }
}
