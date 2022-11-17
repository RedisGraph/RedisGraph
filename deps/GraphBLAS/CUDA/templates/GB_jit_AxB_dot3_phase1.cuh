//------------------------------------------------------------------------------
// template/GB_jit_AxB_dot3_phase1.cuh: build nanobuckets, hunt for pre-zombies
//------------------------------------------------------------------------------

// dot3, phase1: symbolic load balancing and data partition
// to assign work to different 'buckets' for later compute

//  This kernel scans the non-zero pattern in A and B, takes into account the
//  mask and computes total work required to form C. Then it classifies each
//  dot product into a set of buckets for efficient compute. 

#pragma once

#define GB_CUDA_KERNEL
#include <limits>
#include "GB_cuda_kernel.h"
#include "GB_hash.h"
#include "GB_hyper_hash_lookup.h"
#include "GB_cuda_buckets.h"
#include <cub/block/block_scan.cuh>
#include <cooperative_groups.h>

// FIXME: use #include "GB_is.h"
// true if A is bitmap
#define GB_IS_BITMAP(A) ((A) != NULL && ((A)->b != NULL))

// true if A is full (but not bitmap)
#define GB_IS_FULL(A) \
    ((A) != NULL && (A)->h == NULL && (A)->p == NULL && (A)->i == NULL \
        && (A)->b == NULL)

// true if A is hypersparse
#define GB_IS_HYPERSPARSE(A) ((A) != NULL && ((A)->h != NULL))

// true if A is sparse (but not hypersparse)
#define GB_IS_SPARSE(A) ((A) != NULL && ((A)->h == NULL) && (A)->p != NULL)

using namespace cooperative_groups;

//------------------------------------------------------------------------------
// GB_jit_AxB_dot3_phase1: build nanobuckets, hunt for pre-zombies
//------------------------------------------------------------------------------

// GB_AxB_cuda_dot3_phase1 is a CUDA kernel that scans all entries in C and
// assigns them to each of the NBUCKETS buckets.  The output is a
// NBUCKETS-by-blockDim array of bucket counts, per threadblock (the nanobucket
// array).  Each of the blockDim.x threads has its own set of NBUCKETS bucket
// counts.  Each threadblock in this kernel then computes the first part of the
// cumulative sum of the nanobuckets, and writes it to global memory.

// The kernel also computes Ci, of size nnz(C), which contains the
// zombie assignment or bucket assignment for non-zombies in C.

// Assigns the dot product C(i,j) = A(:,i)'*B(:,j) to a specific bucket.  Both
// A(:,i) and B(:,j) are non-empty when this method is called.
// GB_BUCKET_ZOMBIE:    C(i,j) is a prezombie, either A(:,i) or B(:,j) are
//                      empty.
// GB_BUCKET_VSVS       both A(:,i) and B(:,j) are very sparse.
// GB_BUCKET_MERGEPATH  both A(:,i) and B(:,j) are sparse, but neither are
//                      very sparse

// FIXME: What if all entries are in one bucket;
// can we skip the bucket creation?

template<typename T_M, uint64_t srcode, int chunk_size = 128>
__global__ void GB_jit_AxB_dot3_phase1
(
    // outputs, preallocated in global memory:
    int64_t *nanobuckets,   // array of size NBUCKETS-blockDim.x-by-gridDim.x
    int64_t *blockbucket,   // bucket counts, of size NBUCKETS-by-gridDim.x
    // input/output:
    GrB_Matrix C,           // final output matrix
    // inputs, not modified:
    const GrB_Matrix M,     // mask matrix
    const GrB_Matrix A,     // input matrix
    const GrB_Matrix B      // input matrix
)
{

    //--------------------------------------------------------------------------
    // get C, M, A, and B
    //--------------------------------------------------------------------------

    const int64_t *__restrict__ Mh = M->h ;
    const int64_t *__restrict__ Mp = M->p ;
    const int64_t *__restrict__ Mi = M->i ;
    #if !GB_MASK_STRUCT
    const T_M *__restrict__ Mx = (T_M*) M->x ; // not accessed if M structural
    #endif
    const int64_t mnvec = M->nvec ;
    const int64_t mvlen = M->vlen ;
    const int64_t mnz = GB_nnz(M) ;
    const bool M_is_hyper = M->h != NULL ;
    ASSERT (GB_IS_SPARSE (M) || GB_IS_HYPERSPARSE (M)) ;

    const int64_t *__restrict__ Ah = A->h ;
    const int64_t *__restrict__ Ap = A->p ;
    const int64_t *__restrict__ Ai = A->i ;
    const int64_t avlen = A->vlen ;
    const int64_t anz = GB_nnz(A) ;

//  printf ("\non the GPU: A is %d %d %d %d\n",
//  GB_IS_SPARSE (A), GB_IS_HYPERSPARSE (A),
//  GB_IS_BITMAP (A), GB_IS_FULL (A)) ;

//  printf ("\non the GPU: B is %d %d %d %d\n",
//  GB_IS_SPARSE (B), GB_IS_HYPERSPARSE (B),
//  GB_IS_BITMAP (B), GB_IS_FULL (B)) ;

    const int64_t *__restrict__ Bh = B->h ;
    const int64_t *__restrict__ Bp = B->p ;
    const int64_t *__restrict__ Bi = B->i ;
    const int64_t bvlen = B->vlen ;
    const int64_t bnz = GB_nnz(B);

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

    // int64_t *restrict Cp = C->p ;    // copy of Mp
    // int64_t *restrict Ch = C->h ;    // copy of Mh
    int64_t *__restrict__ Ci = C->i ;   // for zombies, or bucket assignment

    // Ci [p] for an entry C(i,j) contains either GB_FLIP(i) if C(i,j) is a
    // zombie, or (k << 4) + bucket otherwise, where C(:,j) is the kth vector
    // of C (j = Ch [k] if hypersparse or j = k if standard sparse), and
    // where bucket is the bucket assignment for C(i,j).
    // bucket can be recovered from Ci by bucket = Ci & 0xF

    //--------------------------------------------------------------------------
    // clear the bucket counters
    //--------------------------------------------------------------------------
    int64_t my_bucket[NBUCKETS];

    // ASSERT (mnz > 0) ;
    // ASSERT (gridDim.x <= mnz) ;

    // each thread uses NBUCKETS bucket counters, held in register
    #pragma unroll
    for(int b = 0; b < NBUCKETS; ++b) {
        my_bucket[b] = 0;
    }

    __shared__ int64_t ks [chunk_size] ;


    //--------------------------------------------------------------------------
    // assign all entries of C to the buckets
    //--------------------------------------------------------------------------

    // all threads in this block will compute the same values for these:
    int64_t pfirst, plast, kfirst, klast ;

    int64_t chunk_max = GB_ICEIL (mnz, chunk_size) ;
    //      (mnz + chunk_size -1)/chunk_size;
    for ( int64_t chunk = blockIdx.x;
                  chunk < chunk_max;
                  chunk += gridDim.x )
    {

        //----------------------------------------------------------------------
        // determine the work done by this iteration, "chunk"
        //----------------------------------------------------------------------

        // The slice for each task contains entries pfirst:plast-1 of M and C.
        // This iteration "chunk" computes Ci and Cx [pfirst...plast-1], using
        // Mi and Mx [pfirst:plast-1].  All threads in the thread block are
        // used for this "chunk".
        pfirst = chunk_size * chunk ;
        plast  = pfirst + chunk_size ;
        // plast = GB_IMIN (plast, mnz) ;
        if (plast > mnz) plast = mnz ;
        int64_t my_chunk_size = plast - pfirst ;

        // find the first vector of the slice for this chunk: the
        // vector that owns the entry Mi [pfirst] and Mx [pfirst].
        kfirst = GB_search_for_vector_device (pfirst, Mp, 0, mnvec, mvlen) ;

        // find the last vector of the slice for task blockIdx.x: the
        // vector that owns the entry Mi [plast-1] and Mx [plast-1].
        klast = GB_search_for_vector_device (plast-1, Mp, kfirst, mnvec, mvlen);

        // number of vectors in C and M for this "chunk" iteration, where
        // Mp [kfirst:klast] will be operated on.
        int64_t nk = klast - kfirst + 1 ;

        //----------------------------------------------------------------------
        // fill ks to find all indices
        //----------------------------------------------------------------------

        // search for k values for each entry pfirst:plast-1
        float slope = ((float) nk) / ((float) my_chunk_size) ;
        int64_t mnvec1 = mnvec - 1 ;
        for (int64_t kk = threadIdx.x ; kk < my_chunk_size ; kk += blockDim.x)
        {
            // get a rough estimate of k for the kkth entry in ks
            int64_t k = kfirst + (int64_t) (slope * ((float) kk)) ;
            // k cannot be smaller than kfirst, but might be bigger than
            // mnvec-1, so ensure it is in the valid range, kfirst to mnvec-1
            // k = GB_IMIN (k, mnvec-1) ;
            if (k > mnvec1) k = mnvec1 ; 
            // look for p in Mp, where p is in range pfirst:plast-1
            // where pfirst >= 0 and plast < mnz
            int64_t p = kk + pfirst ;
            // linear-time search for the k value of the pth entry
            while ( Mp [ k + 1 ] <= p ) k++ ;
            while ( Mp [ k     ] >  p ) k-- ;
            ks [kk] = k ;
        }
        this_thread_block().sync();

        //----------------------------------------------------------------------
        // assign entries in C(i,j) to the buckets
        //----------------------------------------------------------------------

        for ( int64_t pM = pfirst + threadIdx.x;
                      pM < pfirst + my_chunk_size;
                      pM += blockDim.x )
        {
            GB_bucket_code bucket = GB_BUCKET_ZOMBIE ;
            int64_t k = ks [pM - pfirst] ;  // get the k value of Mi,Mx [pM].
            int64_t i = Mi [ pM ] ;
            #if GB_M_IS_HYPER
            int64_t j = Mh [k] ;        // Note that Ch and Mh are the same
            #else
            int64_t j = k ;
            #endif
            if ( MX ( pM ) )
            {

                //--------------------------------------------------------------
                // get B(:,j)
                //--------------------------------------------------------------

                int64_t pB, pB_end ;
                #if GB_B_IS_HYPER
                GB_hyper_hash_lookup (Bp, B_Yp, B_Yi, B_Yx, B_hash_bits,
                    j, &pB, &pB_end) ;
                #elif GB_B_IS_SPARSE
                pB       = Bp[j] ;
                pB_end   = Bp[j+1] ;
                #else
                // B is bitmap or full
                pB       = bvlen * j ;
                pB_end   = pB + j ;
                #endif

                int64_t bjnz = pB_end - pB ;
                if (bjnz > 0)
                {

                    //----------------------------------------------------------
                    // get A(:,i)
                    //----------------------------------------------------------

                    int64_t pA, pA_end ;
                    #if GB_A_IS_HYPER
                    GB_hyper_hash_lookup (Ap, A_Yp, A_Yi, A_Yx, A_hash_bits,
                        i, &pA, &pA_end) ;
                    #elif GB_A_IS_SPARSE
                    pA       = Ap[i] ;
                    pA_end   = Ap[i+1] ;
                    #else
                    // A is bitmap or full
                    pA       = avlen * i ;
                    pA_end   = pA + i ;
                    #endif

                    int64_t ainz = pA_end - pA ;
                    if (ainz > 0)
                    {
                        // determine the bucket for C(i,j)
                        #if (GB_A_IS_SPARSE || GB_A_IS_HYPER) && \
                            (GB_B_IS_SPARSE || GB_B_IS_HYPER)
                        // A and B are both sparse/hyper
                        bool vsvs = (ainz + bjnz <= 128) ;
                        bucket = (GB_bucket_code)
                           (  ((int) ( vsvs)) * ((int) GB_BUCKET_VSVS)
                            + ((int) (!vsvs)) * ((int) GB_BUCKET_MERGEPATH)) ;
                        #elif (GB_A_IS_SPARSE || GB_A_IS_HYPER) && \
                              (GB_B_IS_BITMAP || GB_B_IS_FULL)
                        // A is sparse/hyper, B is bitmap/full
                        bool vsvs = (ainz <= 128) ;
                        bucket = (GB_bucket_code)
                           (  ((int) ( vsvs)) * ((int) GB_BUCKET_VSDN)
                            + ((int) (!vsvs)) * ((int) GB_BUCKET_SPDN)) ;
                        #else
                        // A is bitmap/full, B is sparse/hyper
                        bool vsvs = (bjnz <= 128) ;
                        bucket = (GB_bucket_code)
                           (  ((int) ( vsvs)) * ((int) GB_BUCKET_VSDN)
                            + ((int) (!vsvs)) * ((int) GB_BUCKET_SPDN)) ;
                        #endif
                    }
                }
            }

            Ci[pM] = (bucket == GB_BUCKET_ZOMBIE) * ( GB_FLIP(i) << 4)
                   + (bucket != GB_BUCKET_ZOMBIE) * ((k<<4) + bucket) ;
            my_bucket[bucket]++;
        }
    }
    this_thread_block().sync();

    //--------------------------------------------------------------------------
    // cumulative sum of each bucket
    //--------------------------------------------------------------------------

    typedef cub::BlockScan<int64_t, 32, cub::BLOCK_SCAN_WARP_SCANS> BlockCumSum;
    __shared__ typename BlockCumSum::TempStorage temp_storage ;

    // The taskbucket for this thread block is an array of size
    // NBUCKETS-by-blockDim.x, held by row.  Each thread owns one column of
    // this taskbucket, the nanobucket.  The nanobucket is a column of length
    // NBUCKETS, with stride equal to blockDim.x.
    int64_t *nanobucket =
        nanobuckets + blockIdx.x * (NBUCKETS * blockDim.x) + threadIdx.x ;

    #pragma unroll
    for (int b = 0; b < NBUCKETS; ++b)
    {
        if ( threadIdx.x == blockDim.x-1)
        {
            blockbucket [blockIdx.x + b * gridDim.x] = my_bucket[b] ;
        }
        this_thread_block().sync();

        BlockCumSum(temp_storage).ExclusiveSum( my_bucket[b], my_bucket[b]) ;

        this_thread_block().sync();

        nanobucket [b * blockDim.x] = my_bucket[b] ;
    }

    // The last thread now has the sum of all nanobuckets, which is then saved
    // to the global bucket counts.   blockbucket is an array of size
    // NBUCKETS-by-gridDim.x, held by row, with one column per thread block.
    // The last thread saves its result in the column of this thread block.
    // Note that this write to global memory is not coalesced.

    if (threadIdx.x == blockDim.x - 1 )
    {
        #pragma unroll
        for(int b = 0; b < NBUCKETS; ++b)
        {
            blockbucket [b * gridDim.x + blockIdx.x] += my_bucket[b];
        }
    }
}

