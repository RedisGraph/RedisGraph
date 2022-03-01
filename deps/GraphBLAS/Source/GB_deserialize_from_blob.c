//------------------------------------------------------------------------------
// GB_deserialize_from_blob: uncompress a set of blocks from the blob
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// Decompress a single array from a set of compressed blocks in the blob.  If
// the input data is mangled, this method is still safe, since it performs the
// bare minimum sanity checks to ensure no out-of-bounds indexing of arrays.
// However, the contents of output array are not fully checked.  This step is
// done by GB_deserialize, if requested.

#include "GB.h"
#include "GB_serialize.h"
#include "GB_lz4.h"

#define GB_FREE_ALL         \
{                           \
    GB_FREE (&X, X_size) ;  \
}

GrB_Info GB_deserialize_from_blob
(
    // output:
    GB_void **X_handle,         // uncompressed output array
    size_t *X_size_handle,      // size of X as allocated
    // input:
    int64_t X_len,              // size of X in bytes
    const GB_void *blob,        // serialized blob of size blob_size
    size_t blob_size,
    int64_t *Sblocks,           // array of size nblocks
    int32_t nblocks,            // # of compressed blocks for this array
    int32_t method,             // compression method used for each block
    // input/output:
    size_t *s_handle,           // where to read from the blob
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GrB_Info info ;
    ASSERT (blob != NULL) ;
    ASSERT (s_handle != NULL) ;
    ASSERT (X_handle != NULL) ;
    ASSERT (X_size_handle != NULL) ;
    (*X_handle) = NULL ;
    (*X_size_handle) = 0 ;

    //--------------------------------------------------------------------------
    // parse the method
    //--------------------------------------------------------------------------

    int32_t algo, level ;
    GB_serialize_method (&algo, &level, method) ;

    //--------------------------------------------------------------------------
    // allocate the output array
    //--------------------------------------------------------------------------

    size_t X_size = 0 ;
    GB_void *X = GB_MALLOC (X_len, GB_void, &X_size) ;  // OK
    if (X == NULL)
    { 
        // out of memory
        return (GrB_OUT_OF_MEMORY) ;
    }

    //--------------------------------------------------------------------------
    // determine the number of threads to use
    //--------------------------------------------------------------------------

    GB_GET_NTHREADS_MAX (nthreads_max, chunk, Context) ;

    //--------------------------------------------------------------------------
    // decompress the blocks from the blob
    //--------------------------------------------------------------------------

    size_t s = (*s_handle) ;
    bool ok = true ;

    if (algo == GxB_COMPRESSION_NONE)
    {

        //----------------------------------------------------------------------
        // no compression; the array is held in a single block
        //----------------------------------------------------------------------

        if (nblocks > 1 || Sblocks [0] != X_len || s + X_len > blob_size)
        {
            // blob is invalid: guard against an unsafe memcpy
            ok = false ;
        }
        else
        { 
            // copy the blob into the array X.  This is now safe and secure.
            // The contents of X are not yet checked, however.
            GB_memcpy (X, blob + s, X_len, nthreads_max) ;
        }

    }
    else if (algo == GxB_COMPRESSION_LZ4 || algo == GxB_COMPRESSION_LZ4HC)
    {

        //----------------------------------------------------------------------
        // LZ4 / LZ4HC compression
        //----------------------------------------------------------------------

        int nthreads = GB_IMIN (nthreads_max, nblocks) ;
        int32_t blockid ;
        #pragma omp parallel for num_threads(nthreads) schedule(dynamic) \
            reduction(&&:ok)
        for (blockid = 0 ; blockid < nblocks ; blockid++)
        {
            // get the start and end of the compressed and uncompressed blocks
            int64_t kstart, kend ;
            GB_PARTITION (kstart, kend, X_len, blockid, nblocks) ;
            int64_t s_start = (blockid == 0) ? 0 : Sblocks [blockid-1] ;
            int64_t s_end   = Sblocks [blockid] ;
            size_t  s_size  = s_end - s_start ;
            size_t  d_size  = kend - kstart ;
            // ensure s_start, s_end, kstart, and kend are all valid,
            // to avoid accessing arrays out of bounds, if input is corrupted.
            if (kstart < 0 || kend < 0 || s_start < 0 || s_end < 0 ||
                kstart >= kend || s_start >= s_end || s_size > INT32_MAX ||
                s + s_start > blob_size || s + s_end > blob_size ||
                kstart > X_len || kend > X_len || d_size > INT32_MAX)
            {
                // blob is invalid
                ok = false ;
            }
            else
            { 
                // uncompress the compressed block of size s_size
                // from blob [s + s_start:s_end-1] into X [kstart:kend-1].
                // This is safe and secure so far.  The contents of X are
                // not yet checked, however.  That step is done in
                // GB_deserialize, if requested.
                const char *src = (const char *) (blob + s + s_start) ;
                char *dst = (char *) (X + kstart) ;
                int src_size = (int) s_size ;
                int dst_size = (int) d_size ;
                int u = LZ4_decompress_safe (src, dst, src_size, dst_size) ;
                if (u != dst_size)
                {
                    // blob is invalid
                    ok = false ;
                }
            }
        }
    }
    else
    {
        // unknown compression method
        ok = false ;
    }

    if (!ok)
    {
        // decompression failure; blob is invalid
        GB_FREE_ALL ;
        return (GrB_INVALID_OBJECT) ;
    }

    //--------------------------------------------------------------------------
    // return result: X, its size, and updated index into the blob
    //--------------------------------------------------------------------------

    (*X_handle) = X ;
    (*X_size_handle) = X_size ;
    if (nblocks > 0)
    {
        s += Sblocks [nblocks-1] ;
    }
    (*s_handle) = s ;
    return (GrB_SUCCESS) ;
}

