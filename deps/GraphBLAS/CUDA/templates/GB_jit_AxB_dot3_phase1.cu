//------------------------------------------------------------------------------
// templates/GB_AxB_cuda_dot3_phase1: symbolic load balancing and data partition
// to assign work to different 'buckets' for later compute
//------------------------------------------------------------------------------

//  This kernel scans the non-zero pattern in A and B, takes into account the
//  mask and computes total work required to form C. Then it classifies each
//  dot product into a set of buckets for efficient compute. 

#define GB_KERNEL
#include <limits>
#include <cstdint>
#include "matrix.h"
#include "GB_cuda_buckets.h"
#include "local_cub/block/block_scan.cuh"
#include "mySemiRing.h"

//------------------------------------------------------------------------------
// GB_bucket_assignment
//------------------------------------------------------------------------------

// assign the dot product C(i,j) = A(:,i)'*B(:,j) to a specific bucket
__device__ static inline GB_bucket_code GB_bucket_assignment
(
    int64_t ainz,       // # of entries A(:,i), always > 0
    int64_t bjnz,       // # of entries B(:,j), always > 0
    int64_t vlen        // vector length of A(:,i) and B(:,j)
)
{

    int b = 0 ; // no bucket assigned yet

    // GB_BUCKET (condition,bucket) :  assigns an entry to a bucket,
    // if the condition holds, but without using any if statements.
    // An entry is assigned once and not reassigned.

    // If the bucket b has not assigned, it is b = 0.  The GB_BUCKET function
    // tests this case, and if the condition is also true, the expression
    // (b==0) * condition * (bucket+1) becomes equal to bucket+1.  This
    // value is added to b, which is zero, so the final result is that b
    // is set to bucket+1.

    // If the bucket b has been assigned already, we have b > 0.  Thus,
    // the expression ((b==0) * condition * (bucket+1)) becomes zero.
    // When added to b, the result is that b doesn't change, so the bucket
    // assignment b is unmodified.

    #define GB_BUCKET(condition,bucket) \
        b = (((b == 0) * (condition)) * (bucket+1)) + b ;

//  if (ia_last < ib_first || ib_last < ia_first)
    { 

        //----------------------------------------------------------------------
        // pattern of A(:,i) and B(:,j) do not overlap
        //----------------------------------------------------------------------

        // The patterns of A(:,i) and B(:,j) are always sorted.  If the last
        // entry in A(:,i) comes before the first entry in B(:,j), or visa
        // versa, then there is no work to do since C(i,j) must be a zombie.

        // GB_BUCKET (ia_last < ib_first || ib_last < ia_first, GB_BUCKET_ZOMBIE);

    }
//  else if (bjnz == vlen && ainz == vlen && vlen > 256)
    {

        //----------------------------------------------------------------------
        // both A(:,i) and B(:,j) are dense
        //----------------------------------------------------------------------

        // No search of A(:,i) or B(:,j) is needed.  Total work is O(vlen).
        // The intersection is non-empty, so C(i,j) cannot be a zombie.

        // CUDA kernel: templates/GB_jit_AxB_dot3_phase3_dndn.cu.jit

        GB_BUCKET (bjnz == vlen && ainz == vlen && vlen > 256, GB_BUCKET_DNDN) ;

    }
//  else if (ainz == vlen)
    {
 
        //----------------------------------------------------------------------
        // A(:,i) is dense and B(:,j) is sparse
        //----------------------------------------------------------------------
 
        // No search of A(:,i) is needed.  Total work is O(bjnz), via a linear
        // time scan of B(:,j).  Since A(:,i) is dense and B(:,j) is non-empty,
        // the intersection is non-empty, so C(i,j) cannot be a zombie.

        // CUDA kernel: templates/GB_jit_AxB_dot3_phase3_spdn.cu.jit
        // Two buckets are used, depending on bjnz.
        GB_BUCKET (ainz == vlen && bjnz <  256, GB_BUCKET_DNVS) ;
        GB_BUCKET (ainz == vlen && bjnz >= 256, GB_BUCKET_DNSP) ;
 
    }
//  else if (bjnz == vlen)
    {

        //----------------------------------------------------------------------
        // A(:,i) is sparse and B(:,j) is dense
        //----------------------------------------------------------------------

        // No search of B(:,j) is needed.  Total work is O(ainz), via a linear
        // time scan of A(:,i).  Since B(:,j) is dense and A(:,i) is non-empty,
        // the intersection is non-empty, so C(i,j) cannot be a zombie.

        // CUDA kernel: templates/GB_jit_AxB_dot3_phase3_spdn.cu.jit
        // Two buckets are used, depending on ainz.
        GB_BUCKET (bjnz == vlen && ainz <  256, GB_BUCKET_VSDN) ;
        GB_BUCKET (bjnz == vlen && ainz >= 256, GB_BUCKET_SPDN) ;

    }
//  else if ((ainz > 32 * bjnz && bjnz < 256)
//        || (bjnz > 32 * ainz && ainz < 256))
    {

        //----------------------------------------------------------------------
        // A(:,i) is very sparse compared to B(:,j), or visa versa
        //----------------------------------------------------------------------

        // Since B(:,j) is small, and much smaller than A(:,i), the efficient
        // way to compute C(i,j) is a linear scan of B(:,j).  For each B(k,j),
        // a binary search for the index A(k,i) is done.  The expected work to
        // compute C(i,j) is thus O(bjnz * log2 (ainz)).  If A(:,i) is very
        // sparse compared to B(:,j), the opposite is done inside the kernel.

        // CUDA kernel: templates/GB_jit_AxB_dot3_phase3_vssp.cu.jit

        GB_BUCKET ((ainz > 32 * bjnz && bjnz < 256)
                || (bjnz > 32 * ainz && ainz < 256), GB_BUCKET_VSSP) ;

    }
//  else if (ainz + bjnz <= 4)
    {

        //----------------------------------------------------------------------
        // both A(:,i) and B(:,j) are very tiny (total size 4 or less)
        //----------------------------------------------------------------------

        // CUDA kernel: templates/GB_jit_AxB_dot3_phase3_vsvs.cu.jit
        //GB_BUCKET (ainz + bjnz <= 4, GB_BUCKET_VSVS_4) ;

    }
//  else if (ainz + bjnz <= 16)
    {

        //----------------------------------------------------------------------
        // both A(:,i) and B(:,j) are tiny (total size 16 or less)
        //----------------------------------------------------------------------

        // CUDA kernel: templates/GB_jit_AxB_dot3_phase3_vsvs.cu.jit
        //GB_BUCKET (ainz + bjnz <= 16, GB_BUCKET_VSVS_16) ;

    }
//  else if (ainz + bjnz <= 64)
    {

        //----------------------------------------------------------------------
        // both A(:,i) and B(:,j) are small (total size 64 or less)
        //----------------------------------------------------------------------

        // CUDA kernel: templates/GB_jit_AxB_dot3_phase3_vsvs.cu.jit
        //GB_BUCKET (ainz + bjnz <= 64, GB_BUCKET_VSVS_64) ;

    }
//  else if (ainz + bjnz <= 256)
    {

        //----------------------------------------------------------------------
        // both A(:,i) and B(:,j) are modest in size (total size 256 or less)
        //----------------------------------------------------------------------

        // CUDA kernel: templates/GB_jit_AxB_dot3_phase3_vsvs.cu.jit
        GB_BUCKET (ainz + bjnz <= 256, GB_BUCKET_VSVS_256) ;

    }
//  else
    {

        //----------------------------------------------------------------------
        // default: use the merge-path method
        //----------------------------------------------------------------------

        // CUDA kernel: templates/GB_jit_AxB_dot3_phase3_mp.cu.jit
        GB_BUCKET (true, GB_BUCKET_MERGEPATH) ;
    }

    // subtract one to undo the "bucket+1" assignment in the
    // GB_BUCKET macro assignment expression.
    return (GB_bucket_code) (b-1) ;
}


//--------------------------------------------------------------------------
// GB_AxB_cuda_dot3_phase1: build nanobuckets, hunt for pre-zombies
//--------------------------------------------------------------------------

// GB_AxB_cuda_dot3_phase1 is a CUDA kernel that scans all entries in C and
// assigns them to each of the 12 buckets.  The output is a 12-by-blockDim array of
// bucket counts, per threadblock (the nanobucket array).  Each of the blockDim.x 
// threads has its own set of 12 bucket counts.  Each threadblock in this
// kernel then computes the first part of the cumulative sum of the
// nanobuckets, and writes it to global memory.

// The kernel also computes Ci, of size nnz(C), which contains the
// zombie assignment or bucket assignment for non-zombies in C.

template<typename Type_M> 
__global__ void GB_AxB_cuda_dot3_phase1
(
    // outputs, preallocated in global memory:
    int64_t *nanobuckets,       // array of size 12-blockDim.x-by-gridDim.x
    int64_t *blockbucket,       // bucket counts, of size 12-by-gridDim.x
    // input/output:
    GrB_Matrix C,               // final output matrix
    // inputs, not modified:
    const GrB_Matrix M,         // mask matrix
    const GrB_Matrix A,         // input matrix
    const GrB_Matrix B          // input matrix
)
{

    //--------------------------------------------------------------------------
    // get C, M, A, and B
    //--------------------------------------------------------------------------
    
    const int64_t *__restrict__ Mh = M->h ;
    const int64_t *__restrict__ Mp = M->p ;
    const int64_t *__restrict__ Mi = M->i ;
    const Type_M *__restrict__ Mx = (Type_M*)M->x ;    // not accessed if M is structural
    const int64_t mnvec = M->nvec ;
    const int64_t mnz =  GB_NNZ(M) ;
    const bool M_is_hyper = M->is_hyper ;

    const int64_t *__restrict__ Ah = A->h ;
    const int64_t *__restrict__ Ap = A->p ;
    const int64_t *__restrict__ Ai = A->i ;
    const int64_t avlen = A->vlen ;
    const int64_t anz = GB_NNZ(A) ;
    const bool A_is_hyper = A->is_hyper ;

    const int64_t *__restrict__ Bh = B->h ;
    const int64_t *__restrict__ Bp = B->p ;
    const int64_t *__restrict__ Bi = B->i ;
    const int64_t bvlen = B->vlen ;
    const int64_t bnz = GB_NNZ(B);
    const bool B_is_hyper = B->is_hyper ;

    // int64_t *restrict Cp = C->p ;    // copy of Mp
    // int64_t *restrict Ch = C->h ;    // copy of Mh
    int64_t *__restrict__ Ci = C->i ;       // for zombies, or bucket assignment

    // Ci [p] for an entry C(i,j) contains either GB_FLIP(i) if C(i,j) is a
    // zombie, or (k << 4) + bucket otherwise, where C(:,j) is the kth vector
    // of C (j = Ch [k] if hypersparse or j = k if standard sparse), and
    // where bucket is the bucket assignment for C(i,j). 
    // bucket can be recovered from Ci by bucket = Ci & 0xF

    //--------------------------------------------------------------------------
    // clear the bucket counters
    //--------------------------------------------------------------------------

    //ASSERT (mnz > 0) ;
    //ASSERT (gridDim.x <= mnz) ;

    // each thread uses 12 bucket counters, held in register
    int64_t my_bucket_0  = 0 ;
    int64_t my_bucket_1  = 0 ;
    int64_t my_bucket_2  = 0 ;
    int64_t my_bucket_3  = 0 ;
    int64_t my_bucket_4  = 0 ;
    int64_t my_bucket_5  = 0 ;
    int64_t my_bucket_6  = 0 ;
    int64_t my_bucket_7  = 0 ;
    int64_t my_bucket_8  = 0 ;
    int64_t my_bucket_9  = 0 ;
    int64_t my_bucket_10 = 0 ;
    int64_t my_bucket_11 = 0 ;

    // Registers cannot be indexed (!) so this macro is used instead.
    // The bucket registers are indexed by the GB_bucket_code enum.
    #define GB_BUCKET_COUNT(bucket)                 \
    {                                               \
        switch (bucket)                             \
        {                                           \
            case  0: my_bucket_0++  ; break ;       \
            case  1: my_bucket_1++  ; break ;       \
            case  2: my_bucket_2++  ; break ;       \
            case  3: my_bucket_3++  ; break ;       \
            case  4: my_bucket_4++  ; break ;       \
            case  5: my_bucket_5++  ; break ;       \
            case  6: my_bucket_6++  ; break ;       \
            case  7: my_bucket_7++  ; break ;       \
            case  8: my_bucket_8++  ; break ;       \
            case  9: my_bucket_9++  ; break ;       \
            case 10: my_bucket_10++ ; break ;       \
            case 11: my_bucket_11++ ; break ;       \
        }                                           \
    }
     /*
    if(threadIdx.x==0 ) {
       printf(" in phase1 kernel, mnz,anz,bnz= %ld,%ld,%ld\n",mnz,anz,bnz); 
    }
    __syncthreads();
     */
     #define pointerchunk 256

     __shared__ int64_t Mps[pointerchunk];
     __shared__ int64_t ks [chunksize];

    //--------------------------------------------------------------------------
    // compute the task descriptor
    //--------------------------------------------------------------------------

    // all threads in this block will compute the same values for these:
    int32_t pfirst, plast, kfirst, klast ;
    /*
    for ( int tid_global = threadIdx.x + blockIdx.x * blockDim.x ; 
              tid_global < (mnvec+ 7)/8 ;
              tid_global += blockDim.x*gridDim.x) 
              */
    int chunk_max= (mnz + chunksize -1)/chunksize;
    for ( int chunk = blockIdx.x;
              chunk < chunk_max;
              chunk += gridDim.x ) 
    {

      // The slice for each task contains entries pfirst:plast-1 of M and C.
      //GB_PARTITION (pfirst, plast, mnz, chunk, (mnz+1023)/1024 ) ;
      pfirst = chunksize * chunk ; 
      plast  = GB_IMIN( chunksize * (chunk+1), mnz ) ;

      int chunk_end;
      if ( mnz > chunksize) chunk_end = GB_IMIN(  chunksize, 
                                                  mnz - chunksize*(chunk) ) ; 
      else chunk_end = mnz;

      // find the first vector of the slice for task tid_global: the
      // vector that owns the entry Ai [pfirst] and Ax [pfirst].
      kfirst = GB_search_for_vector_device (pfirst, Mp, 0, mnvec) -1 ;
      //if( pfirst ==0) kfirst = 0;

      // find the last vector of the slice for task blockIdx.x: the
      // vector that owns the entry Ai [plast-1] and Ax [plast-1].
      klast = GB_search_for_vector_device (plast-1, Mp, kfirst, mnvec) ;

      int k_end = GB_IMIN(  pointerchunk ,  klast - kfirst +2 ) ;
       /* 
      if( threadIdx.x ==0) 
      {
         printf("chunk%d pfirst,plast,ch_end =%d,%d,%d kfirst,klast,kend = %d,%d,%d\n",
                 chunk, pfirst, plast, chunk_end, kfirst, klast, k_end ) ;
      }
      __syncthreads();
      */
      
     
      // load pointer values for this chunk
      for ( int i = threadIdx.x; i< k_end; i+= blockDim.x)
      {
          Mps[i] = Mp[i + kfirst];
      }
      __syncthreads();

      // search for k values for each entry
      float slope = (float)(mnvec)/(float)(mnz* chunksize) ;
      for ( int i =  threadIdx.x; i< chunk_end; i+= blockDim.x)
      {   
          ks[i] = kfirst + slope*( float )(i);
          while ( Mps[ ks[i] - kfirst + 1 ] <= (i+pfirst) )
             ks[i]++;
          while ( Mps[ ks[i] - kfirst     ] >  (i+pfirst) )
             ks[i]--;
      }
      __syncthreads();


    //ASSERT (0 <= kfirst && kfirst <= klast && klast < mnvec) ;
    /*
    if (threadIdx.x ==0 ) {
       printf ("threadblock %d  after ksearch pfirst %ld plast %ld kfirst %ld klast %ld\n",
                blockIdx.x, pfirst, plast, kfirst, klast) ;
    }
    __syncthreads();
    */
    
    

    //--------------------------------------------------------------------------
    // assign entries in C(i,j) to the buckets
    //--------------------------------------------------------------------------

    // if B is hypersparse, bpleft ... TODO describe
    // int64_t bpleft = 0 ;
    
        //----------------------------------------------------------------------
        // no binary search variant
        //----------------------------------------------------------------------

        //printf ("no binary search\n") ;

        //int32_t pM_start, pM_end ;
        //for (int64_t pM = pfirst + threadIdx.x ; pM < plast ; pM += blockDim.x)
        int32_t i,j;
        int32_t k = kfirst ;
            
        //for (int64_t pM = pfirst; pM < plast; pM++ ) 
        for ( int pM = pfirst + threadIdx.x;
                  pM < pfirst + chunk_end;
                  pM += blockDim.x )
        {
            GB_bucket_code bucket = GB_BUCKET_ZOMBIE ;
            k = ks[ pM - pfirst ] ;
            //k += ( pM == Mp[k+1] ) ;
            //printf ("tid%d  k %ld pM %ld\n", tid_global, k, pM;
            i = Mi [ pM ] ;

            if ( MX ( pM ) )
            { 

            // do a binary search for k (and j) that has this entry M(i,j)
            //k = GB_search_for_vector_device (pM, Mp, k, klast) ;

// HACK
j = k ;
//          int64_t j = (Mh == NULL) ? k : Mh [k] ;

            //--------------------------------------------------------------
            // get B(:,j)
            //--------------------------------------------------------------

            int64_t pB, pB_end ;
// HACK: for sparse only, not hypersparse
pB = Bp [j] ;
pB_end = Bp [j+1] ;
//              GB_lookup_device (B_is_hyper, Bh, Bp, &bpleft, bnvec-1, j,
//                  &pB, &pB_end) ;
                int64_t bjnz = pB_end - pB ;
                if (bjnz > 0)
                {
                 //   int64_t ib_first = Bi [pB] ;
                 //   int64_t ib_last  = Bi [pB_end-1] ;

                    //----------------------------------------------------------
                    // get A(:,i)
                    //----------------------------------------------------------

                    int64_t pA, pA_end ;
                    //int64_t apleft = 0 ;
// HACK: for sparse only, not hypersparse
pA = Ap [i] ;
pA_end = Ap [i+1] ;
//                  GB_lookup_device (A_is_hyper, Ah, Ap, &apleft, anvec-1, i,
//                      &pA, &pA_end) ;
                    int64_t ainz = pA_end - pA ;
                    if (ainz > 0)
                    {
                     //   int64_t ia_first = Ai [pA] ;
                     //   int64_t ia_last  = Ai [pA_end-1] ;

                        //------------------------------------------------------
                        // determine the bucket for C(i,j)
                        //------------------------------------------------------

                        //bucket = GB_BUCKET_MERGEPATH ;
                         bucket= GB_bucket_assignment ( ainz, bjnz, bvlen) ;
                    }
                }
            }

            if (bucket == GB_BUCKET_ZOMBIE)
            {
                // mark C(i,j) is a zombie
                //printf ("tid%d pM=%d %d,%d prezombie\n",threadIdx.x,pM,i,j) ;
                Ci [pM] = GB_FLIP (i) << 4 ;
                // GB_BUCKET_COUNT (GB_BUCKET_ZOMBIE) ;
                my_bucket_0++ ; //0 is the zombie bucket
            }
            else
            {
                // place C(i,j) in its bucket
                Ci [pM] = (k << 4) + bucket ;
                GB_BUCKET_COUNT (bucket) ;
                //printf ("tid%d pM=%d %d,%d b=%d\n",threadIdx.x, pM, i,j, (int)bucket) ;
            }
         }
            
        
    
    }
    __syncthreads();

    //--------------------------------------------------------------------------
    // cumulative sum of each bucket
    //--------------------------------------------------------------------------

    typedef cub::BlockScan<int64_t, 32, cub::BLOCK_SCAN_WARP_SCANS> BlockCumSum; 
    __shared__ typename BlockCumSum::TempStorage temp_storage;

    // The taskbucket for this thread block is an array of size
    // 12-by-blockDim.x, held by row.  Each thread owns one column of this
    // taskbucket, the nanobucket.  The nanobucket is a column of length 12,
    // with stride equal to blockDim.x.
    int64_t *nanobucket =
        nanobuckets + blockIdx.x * (12 * blockDim.x) + threadIdx.x ;

    #define CUMSUM_AND_STORE_NANOBUCKET(bucket)                             \
        if( threadIdx.x == blockDim.x-1)                                    \
            blockbucket [blockIdx.x + bucket * gridDim.x] =                 \
            my_bucket_ ## bucket ;                                          \
        BlockCumSum(temp_storage).ExclusiveSum                              \
            ( my_bucket_ ## bucket, my_bucket_ ## bucket) ;                 \
            __syncthreads();                                                \
        nanobucket [bucket * blockDim.x] = my_bucket_ ## bucket ;

    CUMSUM_AND_STORE_NANOBUCKET (0) ;
    CUMSUM_AND_STORE_NANOBUCKET (1) ;
    CUMSUM_AND_STORE_NANOBUCKET (2) ;
    CUMSUM_AND_STORE_NANOBUCKET (3) ;
    CUMSUM_AND_STORE_NANOBUCKET (4) ;
    CUMSUM_AND_STORE_NANOBUCKET (5) ;
    CUMSUM_AND_STORE_NANOBUCKET (6) ;
    CUMSUM_AND_STORE_NANOBUCKET (7) ;
    CUMSUM_AND_STORE_NANOBUCKET (8) ;
    CUMSUM_AND_STORE_NANOBUCKET (9) ;
    CUMSUM_AND_STORE_NANOBUCKET (10) ;
    CUMSUM_AND_STORE_NANOBUCKET (11) ;

    /*    
    if(threadIdx.x +blockIdx.x*blockDim.x <= mnvec) //blockDim.x -1){ 
    {
       printf("thd %d blk%d nbucket0 has %ld prev\n",threadIdx.x, blockIdx.x, nanobucket[0]);
       printf("thd %d blk%d nbucket1 has %ld prev\n",threadIdx.x, blockIdx.x, nanobucket[1*blockDim.x]);
       printf("thd %d blk%d nbucket2 has %ld prev\n",threadIdx.x, blockIdx.x, nanobucket[2*blockDim.x]);
       printf("thd %d blk%d nbucket3 has %ld prev\n",threadIdx.x, blockIdx.x, nanobucket[3*blockDim.x]);
       printf("thd %d blk%d nbucket4 has %ld prev\n",threadIdx.x, blockIdx.x, nanobucket[4*blockDim.x]);
       printf("thd %d blk%d nbucket5 has %ld prev\n",threadIdx.x, blockIdx.x, nanobucket[5*blockDim.x]);
       printf("thd %d blk%d nbucket6 has %ld prev\n",threadIdx.x, blockIdx.x, nanobucket[6*blockDim.x]);
       printf("thd %d blk%d nbucket7 has %ld prev\n",threadIdx.x, blockIdx.x, nanobucket[7*blockDim.x]);
       printf("thd %d blk%d nbucket8 has %ld prev\n",threadIdx.x, blockIdx.x, nanobucket[8*blockDim.x]);
       printf("thd %d blk%d nbucket9 has %ld prev\n",threadIdx.x, blockIdx.x, nanobucket[9*blockDim.x]);
       printf("thd %d blk%d nbucket10 has %ld prev\n",threadIdx.x, blockIdx.x, nanobucket[10*blockDim.x]);
       printf("thd %d blk%d nbucket11 has %ld prev\n",threadIdx.x, blockIdx.x, nanobucket[11*blockDim.x]);

    }
    __syncthreads();
    */
        

    // The last thread now has the sum of all nanobuckets, which is then saved
    // to the global bucket counts.   blockbucket is an array of size
    // 12-by-gridDim.x, held by row, with one column per thread block.
    // The last thread saves its result in the column of this thread block.
    // Note that this write to global memory is not coalesced.

    #define STORE_GLOBAL_BUCKET_COUNT(bucket)                    \
        blockbucket [blockIdx.x + bucket * gridDim.x] +=         \
            my_bucket_ ## bucket ;

    if (threadIdx.x == blockDim.x - 1 ) 
    {
        STORE_GLOBAL_BUCKET_COUNT (0) ;
        STORE_GLOBAL_BUCKET_COUNT (1) ;
        STORE_GLOBAL_BUCKET_COUNT (2) ;
        STORE_GLOBAL_BUCKET_COUNT (3) ;
        STORE_GLOBAL_BUCKET_COUNT (4) ;
        STORE_GLOBAL_BUCKET_COUNT (5) ;
        STORE_GLOBAL_BUCKET_COUNT (6) ;
        STORE_GLOBAL_BUCKET_COUNT (7) ;
        STORE_GLOBAL_BUCKET_COUNT (8) ;
        STORE_GLOBAL_BUCKET_COUNT (9) ;
        STORE_GLOBAL_BUCKET_COUNT (10) ;
        STORE_GLOBAL_BUCKET_COUNT (11) ;
    }
    
    /* 
    if(threadIdx.x == blockDim.x -1){ 

       printf("block%d bbucket0 has %ld entries\n",blockIdx.x, blockbucket[0*gridDim.x+blockIdx.x]);
       printf("block%d bbucket1 has %ld entries\n",blockIdx.x, blockbucket[1*gridDim.x+blockIdx.x]);
       printf("block%d bbucket2 has %ld entries\n",blockIdx.x, blockbucket[2*gridDim.x+blockIdx.x]);
       printf("block%d bbucket3 has %ld entries\n",blockIdx.x, blockbucket[3*gridDim.x+blockIdx.x]);
       printf("block%d bbucket4 has %ld entries\n",blockIdx.x, blockbucket[4*gridDim.x+blockIdx.x]);
       printf("block%d bbucket5 has %ld entries\n",blockIdx.x, blockbucket[5*gridDim.x+blockIdx.x]);
       printf("block%d bbucket6 has %ld entries\n",blockIdx.x, blockbucket[6*gridDim.x+blockIdx.x]);
       printf("block%d bbucket7 has %ld entries\n",blockIdx.x, blockbucket[7*gridDim.x+blockIdx.x]);
       printf("block%d bbucket8 has %ld entries\n",blockIdx.x, blockbucket[8*gridDim.x+blockIdx.x]);
       printf("block%d bbucket9 has %ld entries\n",blockIdx.x, blockbucket[9*gridDim.x+blockIdx.x]);
       printf("block%d bbucket10 has %ld entries\n",blockIdx.x, blockbucket[10*gridDim.x+blockIdx.x]);
       printf("block%d bbucket11 has %ld entries\n",blockIdx.x, blockbucket[11*gridDim.x+blockIdx.x]);

    }
    __syncthreads();
    */
    
}

