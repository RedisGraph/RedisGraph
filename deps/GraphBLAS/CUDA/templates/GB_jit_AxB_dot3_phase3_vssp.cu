//------------------------------------------------------------------------------
// spGEMM_very_sparse_sparse.cu 
//------------------------------------------------------------------------------

// The spGEM_vssp CUDA kernel produces the semi-ring product of two
// sparse matrices of types T_A and T_B and common index space size n, to a  
// output matrix of type T_C. The matrices are sparse, with different numbers
// of non-zeros and different sparsity patterns. 
// ie. we want to produce C = A'*B in the sense of the given semi-ring.

// This version uses a binary-search algorithm, when the sizes nnzA and nnzB
// are far apart in size, neither is very spare nor dense, for any size of N.

// Both the grid and block are 1D, so blockDim.x is the # threads in a
// threadblock, and the # of threadblocks is grid.x

// Let b = blockIdx.x, and let s be blockDim.x. s= 32 with a variable number
// of active threads = min( min(nzA, nzB), 32) 

// Thus, each t in threadblock b owns a part of the set of pairs in the 
// sparse-sparse bucket of work. The job for each pair of vectors is to find 
// the intersection of the index sets Ai and Bi, perform the semi-ring dot 
// product on those items in the intersection, and finally
// on exit write it to Cx [pair].

//  int64_t start          <- start of vector pairs for this kernel
//  int64_t end            <- end of vector pairs for this kernel
//  int64_t *Bucket        <- array of pair indices for all kernels 
//  GrB_Matrix C         <- result matrix 
//  GrB_Matrix M         <- mask matrix
//  GrB_Matrix A         <- input matrix A
//  GrB_Matrix B         <- input matrix B

#include <limits>
#include <cstdint>
#include <cooperative_groups.h>
#include "mySemiRing.h"
#include "matrix.h"

// Using tile size fixed at compile time, we don't need shared memory
#define tile_sz 32 

using namespace cooperative_groups;

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

#define intersects_per_thread 8

template< typename T_C, typename T_A, typename T_B, typename T_X, typename T_Y, typename T_Z>
__global__ void AxB_dot3_phase3_vssp
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
   // Typed pointers to access data in A,B,C
   T_A *Ax = (T_A*)A->x;
   T_B *Bx = (T_B*)B->x;
   T_C *Cx = (T_C*)C->x;
   int64_t *Ci = C->i;
   int64_t *Mi = M->i;
   int64_t *Ai = A->i;
   int64_t *Bi = B->i;
   int64_t *Ap = A->p;
   int64_t *Bp = B->p;

   // sz = expected non-zeros per dot 
   int m = 256/sz;
   int nvecs = end - start;
   int dpt = nvecs/(gridDim.x*32);
   
   int dots = (nvecs +dpt -1)/dpt; 

   // zombie count
   int zc = 0;
   int64_t pair_id, im;

   // set thread ID
   unsigned int tid_global = threadIdx.x+ blockDim.x* blockIdx.x;
   unsigned int tid = threadIdx.x;

   unsigned long int b = blockIdx.x ;

   // Main loop over pairs 
   for (pair_id = start+ tid_global, im = 0; 
        pair_id < end && im < m;  
        pair_id += gridDim.x*blockDim.x, ++im){

        int64_t i = Mi[pair_id];
        int64_t j = Ci[pair_id] >> 4;

        if( j < 0) //Pre-zombie
        {
            zc++;
            continue;
        }

        int64_t pA      = Ap[i];
        int64_t pA_end  = Ap[i+1];
        int64_t nnzA = pA_end - pA;

        int64_t pB      = B->p[j]; 
        int64_t pB_end  = B->p[j+1]; 
        int64_t nnzB = pB_end - pB;

        //Search for each nonzero in the smaller vector to find intersection 
        bool cij_exists = false;

        T_A aki;
        T_B bkj;
        T_Z cij;

        if (nnzA <= nnzB) {
            //----------------------------------------------------------------------
            // A(:,i) is very sparse compared to B(:,j)
            //----------------------------------------------------------------------

            while (pA < pA_end && pB < pB_end)
            {
                int64_t ia = Ai [pA] ;
                int64_t ib = Bi [pB] ;
                if (ia < ib)
                { 
                    // A(ia,i) appears before B(ib,j)
                    pA++ ;
                }
                else if (ib < ia)
                { 
                    // B(ib,j) appears before A(ia,i)
                    // discard all entries B(ib:ia-1,j)
                    int64_t pleft = pB + 1 ;
                    int64_t pright = pB_end - 1 ;
                    GB_TRIM_BINARY_SEARCH (ia, Bi, pleft, pright) ;
                    //ASSERT (pleft > pB) ;
                    pB = pleft ;
                }
                else // ia == ib == k
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
            }
        }
        else {
            //----------------------------------------------------------------------
            // B(:,j) is very sparse compared to A(:,i)
            //----------------------------------------------------------------------

            while (pA < pA_end && pB < pB_end)
            {
                int64_t ia = Ai [pA] ;
                int64_t ib = Bi [pB] ;
                if (ia < ib)
                { 
                    // A(ia,i) appears before B(ib,j)
                    // discard all entries A(ia:ib-1,i)
                    int64_t pleft = pA + 1 ;
                    int64_t pright = pA_end - 1 ;
                    GB_TRIM_BINARY_SEARCH (ib, Ai, pleft, pright) ;
                    //ASSERT (pleft > pA) ;
                    pA = pleft ;
                }
                else if (ib < ia)
                { 
                    // B(ib,j) appears before A(ia,i)
                    pB++ ;
                }
                else // ia == ib == k
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
            }

        }
        if ( cij_exists){
           GB_PUTC ( Ci[pair_id]=i ) ;
           GB_PUTC ( Cx[pair_id]=(T_C)cij ) ;
        }
        else {
           zc++; 
           //printf(" %lld, %lld is zombie %d!\n",i,j,zc);
           GB_PUTC( Ci[pair_id] = GB_FLIP( i ) ) ;
        }


    }

    //--------------------------------------------------------------------------
    // reduce sum per-thread values to a single scalar
    //--------------------------------------------------------------------------
    thread_block_tile<tile_sz> tile = tiled_partition<tile_sz>( this_thread_block());
    zc = reduce_sum<int,tile_sz>(tile, zc);

    if( threadIdx.x ==0) {
      //printf("warp %d zombie count = %d\n", blockIdx.x, zc);
      atomicAdd( (unsigned long long int*)&(C->nzombies), (unsigned long long int)zc);
      //printf(" Czombie = %lld\n",C->nzombies);
    }

}

