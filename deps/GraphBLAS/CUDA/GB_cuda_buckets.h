// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------
// GB_cuda_buckets.h: definitions for buckets using for dot3 
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// This file is #include'd only in the GraphBLAS/CUDA/GB_cuda*.cu source files.

#ifndef GB_CUDA_BUCKETS_H
#define GB_CUDA_BUCKETS_H

#define NBUCKETS 12
// 12 buckets: computed by up to 11 kernel launches (zombies need no work...),
// using 5 different kernels (with different configurations depending on the
// bucket).
typedef enum
{
    // bring out your dead:
    GB_BUCKET_ZOMBIE = 0,              // C(i,j) is a zombie (not a bucket)

// dot3:  C<M>=A'B, M is sparse or hyper, C is sparse or hyper
// 32 kernels A,B: (hyper,sparse,bitmap,full)^2 x M is (sparse/hyper)

// a full/full kernel:
    // CUDA kernel: dndn, handles a single bucket:
    // both A(:,i) and B(:,j) are dense
    GB_BUCKET_DNDN = 1,

// two full/(sparse,hyper) kernels:
    // CUDA kernel: spdn, handles 4 buckets:
    // A(:,i) is dense and B(:,j) is very sparse (< 256 entries)
    GB_BUCKET_DNVS = 2,
    // A(:,i) is dense and B(:,j) is sparse (>= 256 entries)
    GB_BUCKET_DNSP = 3,

// a sparse/full kernel
    // A(:,i) is very sparse (< 256 entries) and B(:,j) is dense
    GB_BUCKET_VSDN = 4,
    // A(:,i) is sparse (>= 256 entries) and B(:,j) is dense
    GB_BUCKET_SPDN = 5,

// a sparse/bitmap kernel
// a bitmap/bitmap kernel
// a bitmap/sparse kernel
// ...


// sparse/sparse:
    // CUDA kernel: vssp, handles 1 bucket, uses binary search:
    // A(:,i) is very sparse compared to B(:,j), or visa versa
    GB_BUCKET_VSSP = 6,

    // CUDA kernel: vsvs, handles 4 buckets:
    // let len = nnz (A (:,i) + nnz (B (:,j)), then:
    GB_BUCKET_VSVS_4 = 7,       // len <= 4
    GB_BUCKET_VSVS_16 = 8,      // len <= 16
    GB_BUCKET_VSVS_64 = 9,      // len <= 64
    GB_BUCKET_VSVS_256 = 10,     // len <= 256

    // CUDA kernel: mp, use the merge-path method:
    GB_BUCKET_MERGEPATH = 11,

    // CUDA kernel: warpix, use the warp-intersect method, unused so far:
    GB_BUCKET_WARP_IX = 12
}
GB_bucket_code ;

#endif
