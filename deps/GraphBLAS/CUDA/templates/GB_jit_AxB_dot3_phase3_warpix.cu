//------------------------------------------------------------------------------
// AxB_dot3_phase3_warpix.cu 
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
#define GB_KERNEL
#include <limits>
#include <cstdint>
#include "matrix.h"
#include <cooperative_groups.h>
#include "mySemiRing.h"

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
__global__ void AxB_dot3_phase3_warpix
(
    int64_t start,
    int64_t end,
    int64_t *__restrict__ Bucket,
    GrB_Matrix C,
    GrB_Matrix M,
    GrB_Matrix A,
    GrB_Matrix B,
    int sz
)
{

    T_A *__restrict__ Ax = (T_A*)A->x;
    T_B *__restrict__ Bx = (T_B*)B->x;
    T_C *__restrict__ Cx = (T_C*)C->x;
    int64_t *__restrict__ Ci = C->i;
    int64_t *__restrict__ Mi = M->i;
    int64_t *__restrict__ Mp = M->p;
    int64_t *__restrict__ Ai = A->i;
    int64_t *__restrict__ Bi = B->i;
    int64_t *__restrict__ Ap = A->p;
    int64_t *__restrict__ Bp = B->p;

    int64_t mnvec = M->nvec;

    // zombie count
    int zc;

    int64_t pair_id;

    // set thread ID
    int tid_global = threadIdx.x+ blockDim.x* blockIdx.x;
    int tid = threadIdx.x;
    int b = blockIdx.x ;

    // total items to be inspected
    int64_t nnzA = 0;
    int64_t nnzB = 0;

    thread_block_tile<tile_sz> tile = tiled_partition<tile_sz>( this_thread_block());

    //int parts = gridDim.x; //Each warp is a part

    //Find our part of the work bucket
    int64_t pfirst, plast, kfirst, klast ;
    GB_PARTITION (pfirst, plast, end-start, b, gridDim.x ) ;
    /* 
    if( tid ==0 ) {
       printf("block%d is alive, pf,pl=%ld,%ld \n", b, pfirst, plast);
    }
    __syncthreads();
    */
    
    
    __shared__ int64_t As[256];
    __shared__ int64_t Bs[256];
    __shared__ T_A Axs[256]; 
    __shared__ T_B Bxs[256]; 

   /* 
    int Bpl[9]; // local offsets into shared for multiple vectors of B
    int shr_vec[8] ; //columns of B we see in this task

    pair_id = Bucket[pfirst];
    int64_t i = Mi[pair_id] ;
    int vecs = 1 ;
    int last_vec = i;
    shr_vec[0] = i;
    for (int id =1; id< plast-pfirst; id++)
    {
         pair_id = Bucket[pfirst+id];
         i = Mi[pair_id];
         if (i == last_vec) continue;
         vecs++;
         shr_vec[vecs] = i;
         last_vec = i;
    }
    int all_loaded = 0;

    Bpl[0] = 0;
    for ( int k = 0; k < vecs; k++)
    {   
        int64_t pA       = Ap[ shr_vec[k] ]; 
        int64_t pA_end   = Ap[ shr_vec[k] +1]; 
        nnzA = pA_end - pA;
        Bpl[k+1] = Bpl[k] + nnzA;
        for (int i = tid ; i < nnzA; i+= blockDim.x)
        {
           As[ Bpl[k] +i ] = Ai[ pA + i ] ; 
        }
        __syncthreads();
    }

    //pre-load columns of B, which will be reused, to shared memory
    //Due to loading a contigious block with stride 1 this is fast
        
    all_loaded = (Bpl[vecs] < 256 );
    if( tid == 0 ) {
       printf("block%d loaded %d vals from B, vecs=%d, all_loaded=%d\n",
                 b, Bpl[vecs], vecs, all_loaded );
    }
    __syncthreads();


    // reset counter
    */
    // Main loop over pairs 
    for (int id = start + pfirst; // loop on pairs 
         id < start+ plast;  
         id ++ )
    {
         int64_t pair_id = Bucket[id];
          
         int64_t i = Mi[pair_id];
         int64_t j = Ci[pair_id] >> 4;

         int64_t pA       = Ap[i];
         int64_t pA_end   = Ap[i+1];
         nnzA = pA_end - pA;

         int64_t pB       = Bp[j]; 
         int64_t pB_end   = Bp[j+1]; 
         nnzB = pB_end - pB;

         zc = 0 ;
         int j_last = -1 ;
         
         
    // No search, this warp does all the work

    int tx_start = pA;
    int tx_end   = pA_end;
    int ty_start = pB;
    int ty_end   = pB_end;

    for ( int i = tid; i < nnzA ; i+= blockDim.x)
    {
       As [i] = Ai[ pA + i];
       Axs[i] = Ax[ pA + i];
    }
    __syncthreads();

    if ( j != j_last) { 
        for ( int i = tid; i < nnzB ; i+= blockDim.x)
        {
           Bs [i] = Bi[ pB + i];
           Bxs[i] = Bx[ pB + i];
        }
        __syncthreads();
        j_last = j;
    }
    

    /*     
    if ( tid==0 ) {
      //printf("block %d dot %lld i,j= %lld,%lld\n", blockIdx.x, pair_id, i, j);
      printf("block%d dot %ld(i,j)=(%ld,%ld) xs,xe= %d,%d ys,ye = %d,%d \n", 
               b, pair_id, i, j, tx_start,tx_end, ty_start, ty_end);
      //for(int a = 0; a < nnzA; a++) printf(" As[%d]:%ld ",a, As[j]);
    }
    tile.sync();
    */
    
    

    // Warp intersection: balanced by design, no idle threads. 
    // Each 32 thread warp will handle 32 comparisons per loop.
    // Either A or B takes stride 4, other takes stride 8
    // For this version A strides 4, B strides 8
    T_A aki;
    T_B bkj;
    T_Z cij = GB_IDENTITY ;
    int Astride = nnzA > nnzB ? 8 : 4;
    int Ashift  = nnzA > nnzB ? 3 : 2;
    int Amask   = nnzA > nnzB ? 7 : 3;
    int Bstride = nnzB >= nnzA ? 8 : 4;
    //printf(" Astride = %d, Bstride = %d\n", Astride, Bstride);

    // TODO PLUS_PAIR_INT64, FP32, FP64: no need for cij_exists.
    // just check if cij > 0

    int cij_exists  = 0 ;

    //Warp intersection dot product
    int bitty_row = tid &  Amask ;
    int bitty_col = tid >> Ashift ;

    int k = tx_start + bitty_row ;
    int l = ty_start + bitty_col ;

    //Ai[k] = As[ k -pA ];  for lookup
    //Bi[l] = Bs[ l -pB ]; 


    int inc_k,inc_l;

    int active = ( ( k < tx_end) && (l < ty_end ) );
       
    /*    
    printf("block%d tid%d  Ai,As=%ld,%ld Bi,Bs=%ld,%ld  k,l =%d,%d active:%d\n",
                    b,tid, Ai[k], As[k -pA], Bi[l], Bs[l -pB],
                    k, l,  active );
    */
                    
    
    while ( tile.any(active) )
    {
       inc_k = 0;
       inc_l = 0;
       int kp = k-pA;
       int lp = l-pB;
       if ( active )
       { 
          coalesced_group g = coalesced_threads();
          if ( g.thread_rank() == g.size()-1)
          {
             inc_k = ( As[kp] <= Bs[lp] ) ;
             inc_l = ( Bs[lp] <= As[kp] ) ;
             // printf("block%d tid%d inc_k= %d inc_l = %d\n",b, tid, inc_k, inc_l );
          }
          //tile.sync();

          if ( As [kp] == Bs [lp] )
          {
              //Axs[kp] = Ax[k];
              //Bxs[lp] = Bx[l];

              GB_GETA ( aki=(T_Z)Axs[kp] ) ;
              GB_GETB ( bkj=(T_Z)Bxs[lp] ) ;
              if (cij_exists)
              {
                T_Z t = GB_MULT( (T_Z) aki, (T_Z) bkj);
                GB_ADD_F( cij, t ) ;
                //printf("block%d  thd%d ix at %ld(%ld)  cij += %d * %d\n",b, tid, Ai[k], As[kp], aki, bkj);
              }
              else
              {
                cij_exists = 1 ;
                cij = GB_MULT ( (T_Z) aki, (T_Z) bkj) ;
                //printf("  thd%d ix at %ld(%ld)  cij = %d * %d \n", tid, Ai[k], Ais[kp], aki, bkj);
              }
          }
          // TODO check terminal condition
          //printf(" block%u work value = %d, exists = %d\n", b, cij, cij_exists);
          //printf("block%d tid%d k,l = %d,%d Ai,Bi = %ld,%ld \n", b, tid, k, l, Ai[k], Bi[l] );
       }
       //tile.sync();
       //inc_k = tile.shfl_down( inc_k, 31-tid);
       if( tile.any(inc_k) ) {
          k =1+ tile.shfl_down(k,31-tid) + bitty_row ; // tid%Astride;
          //Ais [k-pA] = As[k-pA];
          //Axs [bitty_row] = Ax[k];
       }
       if( tile.any(inc_l) ) {
          l =1+ tile.shfl_down(l,31-tid) + bitty_col ; // tid/Astride;
          //Bis [l-pB] = Bs[l-pB];
          //Bxs [bitty_col] = Bx[l];
       }
       active = ( ( k < tx_end) && (l < ty_end ) );
       //printf("block%d tid = %d k = %d l= %d active=%d\n", b, tid, k, l,active);
    }
    tile.sync();

    //--------------------------------------------------------------------------
    // reduce sum per-thread values to a single scalar, get OR of flag
    //--------------------------------------------------------------------------

    // Do vote here for control.
    cij_exists  = tile.any( cij_exists);
    tile.sync();

    if (cij_exists)
    {
       cij = GB_reduce_sum<T_Z, tile_sz>( tile, cij );
    }
    tile.sync();
    

    // Atomic write result for this block to global mem
    if (tid == 0)
    {
        //printf ("final %d : %d exists = %d\n", b,  cij, cij_exists) ;
        if (cij_exists)
        {
           //printf("block%d i,j =%ld,%ld cij = %d\n",b, i, j, cij);
           GB_PUTC( Cx[pair_id] = (T_C) cij ) ;
           GB_PUTC ( Ci[pair_id] = i ) ;
           
        }
        else
        {
            //printf(" dot %d is a zombie\n", pair_id);
            zc++;
            GB_PUTC ( Ci[pair_id] = GB_FLIP (i) ) ;
        }
    
    //__syncthreads(); 
  

       if( zc > 0)
       {
          //printf("warp %d zombie count = %d\n", blockIdx.x, zc);
          atomicAdd( (unsigned long long int*)&(C->nzombies), (unsigned long long int)zc);
          //printf("blk:%d Czombie = %lld\n",blockIdx.x,C->zombies);
       }

    }
    tile.sync();
    /*
    */
  }
}

