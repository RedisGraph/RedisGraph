//------------------------------------------------------------------------------
// GB_Global: global values in GraphBLAS
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// All Global storage is declared, initialized, and accessed here.  The
// contents of the GB_Global struct are only accessible to functions in this
// file.  Global storage is used to keep track of the GraphBLAS mode (blocking
// or non-blocking), for pointers to malloc/realloc/free functions,
// global matrix options, and other settings.

#include "GB_atomics.h"

//------------------------------------------------------------------------------
// Global storage: for all threads in a user application that uses GraphBLAS
//------------------------------------------------------------------------------

typedef struct
{

    //--------------------------------------------------------------------------
    // blocking/non-blocking mode, set by GrB_init
    //--------------------------------------------------------------------------

    GrB_Mode mode ;             // GrB_NONBLOCKING, GrB_BLOCKING
                                // GxB_NONBLOCKING_GPU, or GxB_BLOCKING_GPU
    bool GrB_init_called ;      // true if GrB_init already called

    //--------------------------------------------------------------------------
    // threading control
    //--------------------------------------------------------------------------

    int nthreads_max ;          // max number of threads to use
    double chunk ;              // chunk size for determining # threads to use

    //--------------------------------------------------------------------------
    // hypersparsity and CSR/CSC format control
    //--------------------------------------------------------------------------

    float bitmap_switch [GxB_NBITMAP_SWITCH] ; // default bitmap_switch
    float hyper_switch ;        // default hyper_switch for new matrices
    bool is_csc ;               // default CSR/CSC format for new matrices

    //--------------------------------------------------------------------------
    // abort function: only used for debugging
    //--------------------------------------------------------------------------

    void (* abort_function ) (void) ;

    //--------------------------------------------------------------------------
    // malloc/realloc/free: memory management functions
    //--------------------------------------------------------------------------

    // All threads must use the same malloc/realloc/free functions.
    // They default to the ANSI C11 functions, but can be defined by GxB_init.

    void * (* malloc_function  ) (size_t)         ;     // required
    void * (* realloc_function ) (void *, size_t) ;     // may be NULL
    void   (* free_function    ) (void *)         ;     // required
    bool malloc_is_thread_safe ;   // default is true

    //--------------------------------------------------------------------------
    // memory usage tracking: for testing and debugging only
    //--------------------------------------------------------------------------

    // malloc_tracking:  default is false.  There is no user-accessible API for
    // setting this to true.  If true, the following statistics are computed.
    // If false, all of the following are unused.

    // nmalloc:  To aid in searching for memory leaks, GraphBLAS keeps track of
    // the number of blocks of allocated that have not yet been freed.  The
    // count starts at zero.  GB_malloc_memory and GB_calloc_memory increment
    // this count, and free (of a non-NULL pointer) decrements it.  realloc
    // increments the count it if is allocating a new block, but it does this
    // by calling GB_malloc_memory.

    // malloc_debug: this is used for testing only (GraphBLAS/Tcov).  If true,
    // then use malloc_debug_count for testing memory allocation and
    // out-of-memory conditions.  If malloc_debug_count > 0, the value is
    // decremented after each allocation of memory.  If malloc_debug_count <=
    // 0, the GB_*_memory routines pretend to fail; returning NULL and not
    // allocating anything.

    bool malloc_tracking ;          // true if allocations are being tracked
    int64_t nmalloc ;               // number of blocks allocated but not freed
    bool malloc_debug ;             // if true, test memory handling
    int64_t malloc_debug_count ;    // for testing memory handling

    //--------------------------------------------------------------------------
    // for testing and development
    //--------------------------------------------------------------------------

    int64_t hack [2] ;              // settings for testing/development only

    //--------------------------------------------------------------------------
    // diagnostic output
    //--------------------------------------------------------------------------

    bool burble ;                   // controls GBURBLE output
    GB_printf_function_t printf_func ;  // pointer to printf
    GB_flush_function_t flush_func ;   // pointer to flush
    bool print_one_based ;          // if true, print 1-based indices
    bool print_mem_shallow ;        // if true, print # shallow bytes

    //--------------------------------------------------------------------------
    // timing: for code development only
    //--------------------------------------------------------------------------

    double timing [40] ;

    //--------------------------------------------------------------------------
    // for malloc debugging only
    //--------------------------------------------------------------------------

    #ifdef GB_DEBUG
    #define GB_MEMTABLE_SIZE 10000
    GB_void *memtable_p [GB_MEMTABLE_SIZE] ;
    size_t   memtable_s [GB_MEMTABLE_SIZE] ;
    #endif
    int nmemtable ;

    //--------------------------------------------------------------------------
    // internal memory pool
    //--------------------------------------------------------------------------

    // free_pool [k] is a pointer to a link list of freed blocks, all of size
    // exactly equal to 2^k.  The total number of blocks in the kth pool is
    // given by free_pool_nblocks [k], and the upper bound on this is given by
    // free_pool_limit [k].  If any additional blocks of size 2^k above that
    // limit are freed by GB_dealloc_memory, they are not placed in the pool,
    // but actually freed instead.

    void *free_pool [64] ;
    int64_t free_pool_nblocks [64] ;
    int64_t free_pool_limit [64] ;

    //--------------------------------------------------------------------------
    // CPU features
    //--------------------------------------------------------------------------

    bool cpu_features_avx2 ;        // x86_64 with AVX2
    bool cpu_features_avx512f ;     // x86_64 with AVX512f

    //--------------------------------------------------------------------------
    // CUDA (DRAFT: in progress)
    //--------------------------------------------------------------------------

    int gpu_count ;                 // # of GPUs in the system
    GrB_Desc_Value gpu_control ;    // always, never, or default
    double gpu_chunk ;              // min problem size for using a GPU
    // properties of each GPU:
    GB_cuda_device gpu_properties [GB_CUDA_MAX_GPUS] ;

}
GB_Global_struct ;

GB_PUBLIC GB_Global_struct GB_Global ;

GB_Global_struct GB_Global =
{

    // GraphBLAS mode
    .mode = GrB_NONBLOCKING,    // default is nonblocking, no GPU

    // initialization flag
    .GrB_init_called = false,   // GrB_init has not yet been called

    // max number of threads and chunk size
    .nthreads_max = 1,
    .chunk = GB_CHUNK_DEFAULT,

    // min dimension                density
    #define GB_BITSWITCH_1          ((float) 0.04)
    #define GB_BITSWITCH_2          ((float) 0.05)
    #define GB_BITSWITCH_3_to_4     ((float) 0.06)
    #define GB_BITSWITCH_5_to_8     ((float) 0.08)
    #define GB_BITSWITCH_9_to_16    ((float) 0.10)
    #define GB_BITSWITCH_17_to_32   ((float) 0.20)
    #define GB_BITSWITCH_33_to_64   ((float) 0.30)
    #define GB_BITSWITCH_gt_than_64 ((float) 0.40)

    // default format
    .bitmap_switch = {
        GB_BITSWITCH_1,
        GB_BITSWITCH_2,
        GB_BITSWITCH_3_to_4,
        GB_BITSWITCH_5_to_8,
        GB_BITSWITCH_9_to_16,
        GB_BITSWITCH_17_to_32,
        GB_BITSWITCH_33_to_64,
        GB_BITSWITCH_gt_than_64 },
    .hyper_switch = GB_HYPER_SWITCH_DEFAULT,

    .is_csc = false,    // default is GxB_BY_ROW

    // abort function for debugging only
    .abort_function   = abort,

    // malloc/realloc/free functions: default to ANSI C11 functions
    .malloc_function  = malloc,
    .realloc_function = realloc,
    .free_function    = free,
    .malloc_is_thread_safe = true,

    // malloc tracking, for testing, statistics, and debugging only
    .malloc_tracking = false,
    .nmalloc = 0,                // memory block counter
    .malloc_debug = false,       // do not test memory handling
    .malloc_debug_count = 0,     // counter for testing memory handling

    // for testing and development only
    .hack = {0, 0},

    // diagnostics
    .burble = false,
    .printf_func = NULL,
    .flush_func = NULL,
    .print_one_based = false,   // if true, print 1-based indices
    .print_mem_shallow = false, // for @GrB interface only

    .timing = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },

    // for malloc debugging only
    .nmemtable = 0,     // memtable is empty

    // all free_pool lists start out empty
    .free_pool = {
        NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
        NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
        NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
        NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
        NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
        NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
        NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
        NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL },

    .free_pool_nblocks = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },

    // default limits on the number of free blocks in each list:
    .free_pool_limit =

    #ifdef _OPENMP
    {   0,      // size 2^0 = 1 byte   none
        0,      // size 2^1 = 2        none
        0,      // size 2^2 = 4        none

        16483,  // size 2^3 = 8        (2^14 blocks * 2^3  = 128 KB total)
        16483,  // size 2^4 = 16 bytes (2^14 blocks * 2^4  = 256 KB total)
        16483,  // size 2^5 = 32       (2^14 blocks * 2^5  = 512 KB total)
        16483,  // size 2^6 = 64       (2^14 blocks * 2^6  = 1 MB total)
        16483,  // size 2^7 = 128      (2^14 blocks * 2^7  = 2 MB total)

        16483,  // size 2^8 = 256      (2^14 blocks * 2^8  = 4 MB total)
        8192,   // size 2^9 = 512      (2^13 blocks * 2^9  = 4 MB total)
        4096,   // size 2^10 = 1 KB    (2^12 blocks * 2^10 = 4 MB total)
        2048,   // size 2^11 = 2 KB    (2^11 blocks * 2^11 = 4 MB total)

        1024,   // size 2^12 = 4 KB    (2^10 blocks * 2^12 = 4 MB total)
        512,    // size 2^13 = 8 KB    (2^9  blocks * 2^13 = 4 MB total)
        256,    // size 2^14 = 16 KB   (2^8  blocks * 2^14 = 4 MB total)
        128,    // size 2^15 = 32 KB   (2^7  blocks * 2^15 = 4 MB total)

        // maximum total size = about 36 MB
        // by default, no blocks larger than 32 KB are kept in the free_pool

        0,      // size 2^16 = 64 KB
        0,      // size 2^17 = 128 KB
        0,      // size 2^18 = 256 KB
        0,      // size 2^19 = 512 KB

        0,      // size 2^20 = 1 MB
        0,      // size 2^21
        0,      // size 2^22
        0,      // size 2^23
        0,      // size 2^24
        0,      // size 2^25
        0,      // size 2^26
        0,      // size 2^27
        0,      // size 2^28
        0,      // size 2^29

        0,      // size 2^30 (1 GB)
        0,      // size 2^31
        0,      // size 2^32
        0,      // size 2^33
        0,      // size 2^34
        0,      // size 2^35
        0,      // size 2^36
        0,      // size 2^37
        0,      // size 2^38
        0,      // size 2^39

        // These larger sizes are of course unlikely to appear, but adding all
        // 64 possibilities means that the free_pool does not need to check an
        // upper bound.

        0,      // size 2^40 (1 TB)
        0,      // size 2^41
        0,      // size 2^42
        0,      // size 2^43
        0,      // size 2^44
        0,      // size 2^45
        0,      // size 2^46
        0,      // size 2^47
        0,      // size 2^48
        0,      // size 2^49

        0,      // size 2^50 (1 PB)
        0,      // size 2^51
        0,      // size 2^52
        0,      // size 2^53
        0,      // size 2^54
        0,      // size 2^55
        0,      // size 2^56
        0,      // size 2^57
        0,      // size 2^58
        0,      // size 2^59

        0,      // size 2^60 (1 exabyte)
        0,      // size 2^61
        0,      // size 2^62
        0 },    // size 2^63 (4 exabytes!)

#else
    // the free pool requires an OpenMP critical section,
    // so disable it if OpenMP is not available.
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
#endif

    // CPU features
    .cpu_features_avx2 = false,         // x86_64 with AVX2
    .cpu_features_avx512f = false,      // x86_64 with AVX512f

    // CUDA environment (DRAFT: in progress)
    .gpu_count = 0,                     // # of GPUs in the system
    .gpu_control = GxB_DEFAULT,         // always, never, or default
    .gpu_chunk = GB_GPU_CHUNK_DEFAULT,  // min problem size for using a GPU

} ;

//==============================================================================
// GB_Global access functions
//==============================================================================

//------------------------------------------------------------------------------
// mode
//------------------------------------------------------------------------------

void GB_Global_mode_set (GrB_Mode mode)
{ 
    GB_Global.mode = mode ;
}

GrB_Mode GB_Global_mode_get (void)
{ 
    return (GB_Global.mode) ;
}

//------------------------------------------------------------------------------
// GrB_init_called
//------------------------------------------------------------------------------

GB_PUBLIC
void GB_Global_GrB_init_called_set (bool GrB_init_called)
{ 
    GB_Global.GrB_init_called = GrB_init_called ;
}

GB_PUBLIC
bool GB_Global_GrB_init_called_get (void)
{ 
    return (GB_Global.GrB_init_called) ;
}

//------------------------------------------------------------------------------
// cpu features
//------------------------------------------------------------------------------

// GB_Global_cpu_features_query is used just once, by GrB_init or GxB_init,
// to determine at run-time whether or not AVX2 and/or AVX512F is available.
// Once these two flags are set, they are saved in the GB_Global struct, and
// can then be queried later by GB_Global_cpu_features_avx*.

GB_PUBLIC
void GB_Global_cpu_features_query (void)
{ 
    #if GBX86
    {

        //----------------------------------------------------------------------
        // x86_64 architecture: see if AVX2 and/or AVX512F are supported
        //----------------------------------------------------------------------

        #if !defined ( GBNCPUFEAT )
        {
            // Google's cpu_features package is available: use run-time tests
            X86Features features = GetX86Info ( ).features ;
            GB_Global.cpu_features_avx2 = (bool) (features.avx2) ;
            GB_Global.cpu_features_avx512f = (bool) (features.avx512f) ;
        }
        #else
        {
            // cpu_features package not available; use compile-time tests
            #if defined ( GBAVX2 )
            {
                // the build system asserts whether or not AVX2 is available
                GB_Global.cpu_features_avx2 = (bool) (GBAVX2) ;
            }
            #else
            {
                // AVX2 not available
                GB_Global.cpu_features_avx2 = false ;
            }
            #endif
            #if defined ( GBAVX512F )
            {
                // the build system asserts whether or not AVX512F is available
                GB_Global.cpu_features_avx512f = (bool) (GBAVX512F) ;
            }
            #else
            {
                // AVX512F not available
                GB_Global.cpu_features_avx512f = false ;
            }
            #endif
        }
        #endif

    }
    #else
    {

        //----------------------------------------------------------------------
        // not on the x86_64 architecture, so no AVX2 or AVX512F acceleration
        //----------------------------------------------------------------------

        GB_Global.cpu_features_avx2 = false ;
        GB_Global.cpu_features_avx512f = false ;

    }
    #endif
}

GB_PUBLIC
bool GB_Global_cpu_features_avx2 (void)
{ 
    return (GB_Global.cpu_features_avx2) ;
}

GB_PUBLIC
bool GB_Global_cpu_features_avx512f (void)
{ 
    return (GB_Global.cpu_features_avx512f) ;
}

//------------------------------------------------------------------------------
// nthreads_max
//------------------------------------------------------------------------------

GB_PUBLIC
void GB_Global_nthreads_max_set (int nthreads_max)
{ 
    GB_Global.nthreads_max = GB_IMAX (nthreads_max, 1) ;
}

GB_PUBLIC
int GB_Global_nthreads_max_get (void)
{ 
    return (GB_Global.nthreads_max) ;
}

//------------------------------------------------------------------------------
// OpenMP max_threads
//------------------------------------------------------------------------------

GB_PUBLIC
int GB_Global_omp_get_max_threads (void)
{ 
    return (GB_OPENMP_MAX_THREADS) ;
}

//------------------------------------------------------------------------------
// chunk
//------------------------------------------------------------------------------

GB_PUBLIC
void GB_Global_chunk_set (double chunk)
{ 
    if (chunk <= GxB_DEFAULT) chunk = GB_CHUNK_DEFAULT ;
    GB_Global.chunk = fmax (chunk, 1) ;
}

GB_PUBLIC
double GB_Global_chunk_get (void)
{ 
    return (GB_Global.chunk) ;
}

//------------------------------------------------------------------------------
// hyper_switch
//------------------------------------------------------------------------------

GB_PUBLIC
void GB_Global_hyper_switch_set (float hyper_switch)
{ 
    GB_Global.hyper_switch = hyper_switch ;
}

GB_PUBLIC
float GB_Global_hyper_switch_get (void)
{ 
    return (GB_Global.hyper_switch) ;
}

//------------------------------------------------------------------------------
// bitmap_switch
//------------------------------------------------------------------------------

GB_PUBLIC
void GB_Global_bitmap_switch_set (int k, float b)
{ 
    k = GB_IMAX (k, 0) ;
    k = GB_IMIN (k, 7) ;
    GB_Global.bitmap_switch [k] = b ;
}

GB_PUBLIC
float GB_Global_bitmap_switch_get (int k)
{ 
    k = GB_IMAX (k, 0) ;
    k = GB_IMIN (k, 7) ;
    return (GB_Global.bitmap_switch [k]) ;
}

GB_PUBLIC
float GB_Global_bitmap_switch_matrix_get (int64_t vlen, int64_t vdim)
{ 
    int64_t d = GB_IMIN (vlen, vdim) ;
    if (d <=  1) return (GB_Global.bitmap_switch [0]) ;
    if (d <=  2) return (GB_Global.bitmap_switch [1]) ;
    if (d <=  4) return (GB_Global.bitmap_switch [2]) ;
    if (d <=  8) return (GB_Global.bitmap_switch [3]) ;
    if (d <= 16) return (GB_Global.bitmap_switch [4]) ;
    if (d <= 32) return (GB_Global.bitmap_switch [5]) ;
    if (d <= 64) return (GB_Global.bitmap_switch [6]) ;
    return (GB_Global.bitmap_switch [7]) ;
}

GB_PUBLIC
void GB_Global_bitmap_switch_default (void)
{ 
    GB_Global.bitmap_switch [0] = GB_BITSWITCH_1 ;
    GB_Global.bitmap_switch [1] = GB_BITSWITCH_2 ;
    GB_Global.bitmap_switch [2] = GB_BITSWITCH_3_to_4 ;
    GB_Global.bitmap_switch [3] = GB_BITSWITCH_5_to_8 ;
    GB_Global.bitmap_switch [4] = GB_BITSWITCH_9_to_16 ;
    GB_Global.bitmap_switch [5] = GB_BITSWITCH_17_to_32 ;
    GB_Global.bitmap_switch [6] = GB_BITSWITCH_33_to_64 ;
    GB_Global.bitmap_switch [7] = GB_BITSWITCH_gt_than_64 ;
}

//------------------------------------------------------------------------------
// is_csc
//------------------------------------------------------------------------------

void GB_Global_is_csc_set (bool is_csc)
{ 
    GB_Global.is_csc = is_csc ;
}

bool GB_Global_is_csc_get (void)
{ 
    return (GB_Global.is_csc) ;
}

//------------------------------------------------------------------------------
// abort_function
//------------------------------------------------------------------------------

GB_PUBLIC
void GB_Global_abort_function_set (void (* abort_function) (void))
{ 
    GB_Global.abort_function = abort_function ;
}

GB_PUBLIC
void GB_Global_abort_function (void)
{
    GB_Global.abort_function ( ) ;
}

//------------------------------------------------------------------------------
// malloc debuging
//------------------------------------------------------------------------------

// These functions keep a separate record of the pointers to all allocated
// blocks of memory and their sizes, just for sanity checks.

GB_PUBLIC
void GB_Global_memtable_dump (void)
{
    #ifdef GB_DEBUG
    printf ("\nmemtable dump: %d nmalloc " GBd "\n", GB_Global.nmemtable,
        GB_Global.nmalloc) ;
    for (int k = 0 ; k < GB_Global.nmemtable ; k++)
    {
        printf ("  %4d: %12p : %ld\n", k,
            GB_Global.memtable_p [k],
            GB_Global.memtable_s [k]) ;
    }
    #endif
}

GB_PUBLIC
int GB_Global_memtable_n (void)
{
    return (GB_Global.nmemtable) ;
}

GB_PUBLIC
void GB_Global_memtable_clear (void)
{
    GB_Global.nmemtable = 0 ;
}

// add a pointer to the table of malloc'd blocks
GB_PUBLIC
void GB_Global_memtable_add (void *p, size_t size)
{
    if (p == NULL) return ;
    if (GB_Global.malloc_tracking)
    {
        GB_ATOMIC_UPDATE
        GB_Global.nmalloc++ ;
    }
    #ifdef GB_DEBUG
    bool fail = false ;
    #ifdef GB_MEMDUMP
    printf ("memtable add %p size %ld\n", p, size) ;
    #endif
    #pragma omp critical(GB_memtable)
    {
        int n = GB_Global.nmemtable ;
        fail = (n > GB_MEMTABLE_SIZE) ;
        if (!fail)
        {
            for (int i = 0 ; i < n ; i++)
            {
                if (p == GB_Global.memtable_p [i])
                {
                    printf ("\nadd duplicate %p size %ld\n", p, size) ;
                    GB_Global_memtable_dump ( ) ;
                    printf ("Hey %d %p\n", i,p) ;
                    fail = true ;
                    break ;
                }
            }
        }
        if (!fail && p != NULL)
        {
            GB_Global.memtable_p [n] = p ;
            GB_Global.memtable_s [n] = size ;
            GB_Global.nmemtable++ ;
        }
    }
    ASSERT (!fail) ;
    #ifdef GB_MEMDUMP
    GB_Global_memtable_dump ( ) ;
    #endif
    #endif
}

// get the size of a malloc'd block
GB_PUBLIC
size_t GB_Global_memtable_size (void *p)
{
    size_t size = 0 ;
    #ifdef GB_DEBUG
    if (p == NULL) return (0) ;
    bool found = false ;
    #pragma omp critical(GB_memtable)
    {
        int n = GB_Global.nmemtable ;
        for (int i = 0 ; i < n ; i++)
        {
            if (p == GB_Global.memtable_p [i])
            {
                size = GB_Global.memtable_s [i] ;
                found = true ;
                break ;
            }
        }
    }
    if (!found)
    {
        printf ("\nFAIL: %p not found\n", p) ;
        GB_Global_memtable_dump ( ) ;
        ASSERT (0) ;
    }
    #endif
    return (size) ;
}

// test if a malloc'd block is in the table
GB_PUBLIC
bool GB_Global_memtable_find (void *p)
{
    bool found = false ;
    #ifdef GB_DEBUG
    if (p == NULL) return (false) ;
    #pragma omp critical(GB_memtable)
    {
        int n = GB_Global.nmemtable ;
        for (int i = 0 ; i < n ; i++)
        {
            if (p == GB_Global.memtable_p [i])
            {
                found = true ;
                break ;
            }
        }
    }
    #endif
    return (found) ;
}

// remove a pointer from the table of malloc'd blocks
GB_PUBLIC
void GB_Global_memtable_remove (void *p)
{
    if (p == NULL) return ;
    if (GB_Global.malloc_tracking)
    {
        GB_ATOMIC_UPDATE
        GB_Global.nmalloc-- ;
    }
    #ifdef GB_DEBUG
    bool found = false ;
    #ifdef GB_MEMDUMP
    printf ("memtable remove %p ", p) ;
    #endif
    #pragma omp critical(GB_memtable)
    {
        int n = GB_Global.nmemtable ;
        for (int i = 0 ; i < n ; i++)
        {
            if (p == GB_Global.memtable_p [i])
            {
                // found p in the table; remove it
                GB_Global.memtable_p [i] = GB_Global.memtable_p [n-1] ;
                GB_Global.memtable_s [i] = GB_Global.memtable_s [n-1] ;
                GB_Global.nmemtable -- ;
                found = true ;
                break ;
            }
        }
    }
    if (!found)
    {
        printf ("remove %p NOT FOUND\n", p) ;
        GB_Global_memtable_dump ( ) ;
    }
    ASSERT (found) ;
    #ifdef GB_MEMDUMP
    GB_Global_memtable_dump ( ) ;
    #endif
    #endif
}

//------------------------------------------------------------------------------
// malloc_function
//------------------------------------------------------------------------------

void GB_Global_malloc_function_set (void * (* malloc_function) (size_t))
{ 
    GB_Global.malloc_function = malloc_function ;
}

void * GB_Global_malloc_function (size_t size)
{ 
    void *p = NULL ;
    if (GB_Global.malloc_is_thread_safe)
    {
        p = GB_Global.malloc_function (size) ;
    }
    else
    {
        #pragma omp critical(GB_malloc_protection)
        {
            p = GB_Global.malloc_function (size) ;
        }
    }
    GB_Global_memtable_add (p, size) ;
    return (p) ;
}

//------------------------------------------------------------------------------
// realloc_function
//------------------------------------------------------------------------------

void GB_Global_realloc_function_set
(
    void * (* realloc_function) (void *, size_t)
)
{ 
    GB_Global.realloc_function = realloc_function ;
}

bool GB_Global_have_realloc_function (void)
{ 
    return (GB_Global.realloc_function != NULL) ;
}

void * GB_Global_realloc_function (void *p, size_t size)
{ 
    void *pnew = NULL ;
    if (GB_Global.malloc_is_thread_safe)
    {
        pnew = GB_Global.realloc_function (p, size) ;
    }
    else
    {
        #pragma omp critical(GB_malloc_protection)
        {
            pnew = GB_Global.realloc_function (p, size) ;
        }
    }
    if (pnew != NULL)
    {
        GB_Global_memtable_remove (p) ;
        GB_Global_memtable_add (pnew, size) ;
    }
    return (pnew) ;
}

//------------------------------------------------------------------------------
// free_function
//------------------------------------------------------------------------------

void GB_Global_free_function_set (void (* free_function) (void *))
{ 
    GB_Global.free_function = free_function ;
}

void GB_Global_free_function (void *p)
{ 
    if (GB_Global.malloc_is_thread_safe)
    {
        GB_Global.free_function (p) ;
    }
    else
    {
        #pragma omp critical(GB_malloc_protection)
        {
            GB_Global.free_function (p) ;
        }
    }
    GB_Global_memtable_remove (p) ;
}

//------------------------------------------------------------------------------
// malloc_is_thread_safe
//------------------------------------------------------------------------------

GB_PUBLIC
void GB_Global_malloc_is_thread_safe_set (bool malloc_is_thread_safe)
{ 
    GB_Global.malloc_is_thread_safe = malloc_is_thread_safe ;
}

GB_PUBLIC
bool GB_Global_malloc_is_thread_safe_get (void)
{ 
    return (GB_Global.malloc_is_thread_safe) ;
}

//------------------------------------------------------------------------------
// malloc_tracking
//------------------------------------------------------------------------------

GB_PUBLIC
void GB_Global_malloc_tracking_set (bool malloc_tracking)
{ 
    GB_Global.malloc_tracking = malloc_tracking ;
}

bool GB_Global_malloc_tracking_get (void)
{ 
    return (GB_Global.malloc_tracking) ;
}

//------------------------------------------------------------------------------
// nmalloc
//------------------------------------------------------------------------------

void GB_Global_nmalloc_clear (void)
{ 
    GB_ATOMIC_WRITE
    GB_Global.nmalloc = 0 ;
}

GB_PUBLIC
int64_t GB_Global_nmalloc_get (void)
{ 
    int64_t nmalloc ;
    GB_ATOMIC_READ
    nmalloc = GB_Global.nmalloc ;
    return (nmalloc) ;
}

//------------------------------------------------------------------------------
// malloc_debug
//------------------------------------------------------------------------------

GB_PUBLIC
void GB_Global_malloc_debug_set (bool malloc_debug)
{ 
    GB_ATOMIC_WRITE
    GB_Global.malloc_debug = malloc_debug ;
}

bool GB_Global_malloc_debug_get (void)
{ 
    bool malloc_debug ;
    GB_ATOMIC_READ
    malloc_debug = GB_Global.malloc_debug ;
    return (malloc_debug) ;
}

//------------------------------------------------------------------------------
// malloc_debug_count
//------------------------------------------------------------------------------

GB_PUBLIC
void GB_Global_malloc_debug_count_set (int64_t malloc_debug_count)
{ 
    GB_ATOMIC_WRITE
    GB_Global.malloc_debug_count = malloc_debug_count ;
}

bool GB_Global_malloc_debug_count_decrement (void)
{ 
    GB_ATOMIC_UPDATE
    GB_Global.malloc_debug_count-- ;

    int64_t malloc_debug_count ;
    GB_ATOMIC_READ
    malloc_debug_count = GB_Global.malloc_debug_count ;
    return (malloc_debug_count <= 0) ;
}

//------------------------------------------------------------------------------
// hack: for setting an internal flag for testing and development only
//------------------------------------------------------------------------------

GB_PUBLIC
void GB_Global_hack_set (int k, int64_t hack)
{ 
    GB_Global.hack [k] = hack ;
}

GB_PUBLIC
int64_t GB_Global_hack_get (int k)
{ 
    return (GB_Global.hack [k]) ;
}

//------------------------------------------------------------------------------
// burble: for controlling the burble output
//------------------------------------------------------------------------------

void GB_Global_burble_set (bool burble)
{ 
    GB_Global.burble = burble ;
}

GB_PUBLIC
bool GB_Global_burble_get (void)
{ 
    return (GB_Global.burble) ;
}

GB_PUBLIC
GB_printf_function_t GB_Global_printf_get (void)
{ 
    return (GB_Global.printf_func) ;
}

GB_PUBLIC
GB_flush_function_t GB_Global_flush_get (void)
{ 
    return (GB_Global.flush_func) ;
}

GB_PUBLIC
void GB_Global_printf_set (GB_printf_function_t pr_func)
{ 
    GB_Global.printf_func = pr_func ;
}

GB_PUBLIC
void GB_Global_flush_set (GB_flush_function_t fl_func)
{ 
    GB_Global.flush_func = fl_func ;
}

//------------------------------------------------------------------------------
// for printing matrices in 1-based index notation (@GrB and Julia)
//------------------------------------------------------------------------------

GB_PUBLIC
void GB_Global_print_one_based_set (bool onebased)
{ 
    GB_Global.print_one_based = onebased ;
}

GB_PUBLIC
bool GB_Global_print_one_based_get (void)
{ 
    return (GB_Global.print_one_based) ;
}

//------------------------------------------------------------------------------
// for printing matrix in @GrB interface
//------------------------------------------------------------------------------

GB_PUBLIC
void GB_Global_print_mem_shallow_set (bool mem_shallow)
{ 
    GB_Global.print_mem_shallow = mem_shallow ;
}

GB_PUBLIC
bool GB_Global_print_mem_shallow_get (void)
{ 
    return (GB_Global.print_mem_shallow) ;
}

//------------------------------------------------------------------------------
// CUDA (DRAFT: in progress)
//------------------------------------------------------------------------------

void GB_Global_gpu_control_set (GrB_Desc_Value gpu_control)
{ 
    // set the GPU control to always, never, or default
    if (GB_Global.gpu_count > 0)
    {
        // one or more GPUs are available: set gpu_control to
        // always, never, or default.
        if (gpu_control == GxB_GPU_ALWAYS || gpu_control == GxB_GPU_NEVER)
        {
            GB_Global.gpu_control = gpu_control ;
        }
        else
        {
            GB_Global.gpu_control = GxB_DEFAULT ;
        }
    }
    else
    {
        // no GPUs available: never use a GPU
        GB_Global.gpu_control = GxB_GPU_NEVER ;
    }
}

GrB_Desc_Value GB_Global_gpu_control_get (void)
{ 
    // get the GPU control parameter
    return (GB_Global.gpu_control) ;
}

void GB_Global_gpu_chunk_set (double gpu_chunk)
{ 
    // set the GPU chunk factor
    if (gpu_chunk < 1) gpu_chunk = GB_GPU_CHUNK_DEFAULT ;
    GB_Global.gpu_chunk = gpu_chunk ;
}

double GB_Global_gpu_chunk_get (void)
{ 
    // get the GPU chunk factor
    return (GB_Global.gpu_chunk) ;
}

bool GB_Global_gpu_count_set (bool enable_cuda)
{ 
    // set the # of GPUs in the system;
    // this function is only called once, by GB_init.
    #if defined ( GBCUDA )
    if (enable_cuda)
    {
        return (GB_cuda_get_device_count (&GB_Global.gpu_count)) ;
    }
    else
    #endif
    {
        // no GPUs available, or available but not requested
        GB_Global.gpu_count = 0 ;
        return (true) ;
    }
}

int GB_Global_gpu_count_get (void)
{ 
    // get the # of GPUs in the system
    return (GB_Global.gpu_count) ;
}

#define GB_GPU_DEVICE_CHECK(error) \
    if (device < 0 || device >= GB_Global.gpu_count) return (error) ;

size_t GB_Global_gpu_memorysize_get (int device)
{
    // get the memory size of a specific GPU
    GB_GPU_DEVICE_CHECK (0) ;       // memory size zero if invalid GPU
    return (GB_Global.gpu_properties [device].total_global_memory) ;
}

int GB_Global_gpu_sm_get (int device)
{
    // get the # of SMs in a specific GPU
    GB_GPU_DEVICE_CHECK (0) ;       // zero if invalid GPU
    return (GB_Global.gpu_properties [device].number_of_sms) ;
}

bool GB_Global_gpu_device_pool_size_set (int device, size_t size)
{
    GB_GPU_DEVICE_CHECK (false) ;   // fail if invalid GPU
    GB_Global.gpu_properties [device].pool_size = size ;
    return (true) ; 
}

bool GB_Global_gpu_device_max_pool_size_set (int device, size_t size)
{
    GB_GPU_DEVICE_CHECK (false) ;   // fail if invalid GPU
    GB_Global.gpu_properties[device].max_pool_size = size ;
    return (true) ; 
}

bool GB_Global_gpu_device_memory_resource_set (int device, void *resource)
{
    GB_GPU_DEVICE_CHECK (false) ;   // fail if invalid GPU
    GB_Global.gpu_properties[device].memory_resource = resource;
    return (true) ; 
}

void* GB_Global_gpu_device_memory_resource_get (int device)
{
    GB_GPU_DEVICE_CHECK (false) ;   // fail if invalid GPU
    return  (GB_Global.gpu_properties [device].memory_resource) ;
    // NOTE: this returns a void*, needs to be cast to be used
}

bool GB_Global_gpu_device_properties_get (int device)
{
    // get all properties of a specific GPU;
    // this function is only called once per GPU, by GB_init.
    GB_GPU_DEVICE_CHECK (false) ;   // fail if invalid GPU
    #if defined ( GBCUDA )
    return (GB_cuda_get_device_properties (device,
        &(GB_Global.gpu_properties [device]))) ;
    #else
    // if no GPUs exist, they cannot be queried
    return (false) ;
    #endif
}

//------------------------------------------------------------------------------
// timing: for code development only
//------------------------------------------------------------------------------

GB_PUBLIC
void GB_Global_timing_clear_all (void)
{
    for (int k = 0 ; k < 40 ; k++)
    {
        GB_Global.timing [k] = 0 ;
    }
}

GB_PUBLIC
void GB_Global_timing_clear (int k)
{
    GB_Global.timing [k] = 0 ;
}

GB_PUBLIC
void GB_Global_timing_set (int k, double t)
{
    GB_Global.timing [k] = t ;
}

GB_PUBLIC
void GB_Global_timing_add (int k, double t)
{
    GB_Global.timing [k] += t ;
}

GB_PUBLIC
double GB_Global_timing_get (int k)
{
    return (GB_Global.timing [k]) ;
}

//------------------------------------------------------------------------------
// free_pool: fast access to free memory blocks
//------------------------------------------------------------------------------

// each free block contains a pointer to the next free block.  This requires
// the free block to be at least 8 bytes in size.
#define GB_NEXT(p) ((void **) p) [0]

// free_pool_init: initialize the free_pool
GB_PUBLIC
void GB_Global_free_pool_init (bool clear)
{ 
    #ifdef _OPENMP
        #pragma omp critical(GB_free_pool)
        {
            if (clear)
            {
                // clear the free pool
                for (int k = 0 ; k < 64 ; k++)
                {
                    GB_Global.free_pool [k] = NULL ;
                    GB_Global.free_pool_nblocks [k] = 0 ;
                }
            }
            // set the default free_pool_limit
            for (int k = 0 ; k < 64 ; k++)
            {
                GB_Global.free_pool_limit [k] = 0 ;
            }
            int64_t n = 16384 ;
            for (int k = 3 ; k <= 8 ; k++)
            {
                GB_Global.free_pool_limit [k] = n ;
            }
            for (int k = 9 ; k <= 19 ; k++)
            {
                n = n/2 ;
                GB_Global.free_pool_limit [k] = n ;
            }
        }
    #else
        // OpenMP not available: disable the free pool
        for (int k = 0 ; k < 64 ; k++)
        {
            GB_Global.free_pool [k] = NULL ;
            GB_Global.free_pool_nblocks [k] = 0 ;
        }
    #endif
}

#ifdef GB_DEBUG
// check if a block is valid
static inline void GB_Global_free_pool_check (void *p, int k, char *where)
{
    // check the size of the block
    ASSERT (k >= 3 && k < 64) ;
    ASSERT (p != NULL) ;
    size_t size = GB_Global_memtable_size (p) ;
    ASSERT (size == ((size_t) 1) << k) ;
}
#endif

// free_pool_get: get a block from the free_pool, or return NULL if none
GB_PUBLIC
void *GB_Global_free_pool_get (int k)
{ 
    #ifdef _OPENMP
        void *p = NULL ;
        ASSERT (k >= 3 && k < 64) ;
        #pragma omp critical(GB_free_pool)
        {
            p = GB_Global.free_pool [k] ;
            if (p != NULL)
            {
                // remove the block from the kth free_pool
                GB_Global.free_pool_nblocks [k]-- ;
                GB_Global.free_pool [k] = GB_NEXT (p) ;
            }
        }
        if (p != NULL)
        {
            // clear the next pointer inside the block, since the block needs
            // to be all zero
            #ifdef GB_DEBUG
            GB_Global_free_pool_check (p, k, "get") ;
            #endif
        }
        return (p) ;
    #else
        // OpenMP not available: free pool not in use
        return (NULL) ;
    #endif
}

// free_pool_put: put a block in the free_pool, unless it is full
GB_PUBLIC
bool GB_Global_free_pool_put (void *p, int k)
{ 
    #ifdef _OPENMP
        #ifdef GB_DEBUG
        GB_Global_free_pool_check (p, k, "put") ;
        #endif
        bool returned_to_pool = false ;
        #pragma omp critical(GB_free_pool)
        {
            returned_to_pool =
                (GB_Global.free_pool_nblocks [k] <
                 GB_Global.free_pool_limit [k]) ;
            if (returned_to_pool)
            {
                // add the block to the head of the free_pool list
                GB_Global.free_pool_nblocks [k]++ ;
                GB_NEXT (p) = GB_Global.free_pool [k] ;
                GB_Global.free_pool [k] = p ;
            }
        }
        return (returned_to_pool) ;
    #else
        return (false) ;
    #endif
}

// free_pool_dump: check the validity of the free_pool
GB_PUBLIC
void GB_Global_free_pool_dump (int pr)
{
    #ifdef _OPENMP
    #ifdef GB_DEBUG
    bool fail = false ;
    #pragma omp critical(GB_free_pool)
    {
        for (int k = 0 ; k < 64 && !fail ; k++)
        {
            int64_t nblocks = GB_Global.free_pool_nblocks [k] ;
            int64_t limit   = GB_Global.free_pool_limit [k] ;
            if (nblocks != 0 && pr > 0)
            {
                printf ("pool %2d: " GBd " blocks, " GBd " limit\n",
                    k, nblocks, limit) ;
            }
            int64_t nblocks_actual = 0 ;
            void *p = GB_Global.free_pool [k] ;
            for ( ; p != NULL && !fail ; p = GB_NEXT (p))
            {
                if (pr > 1) printf ("  %16p ", p) ;
                size_t size = GB_Global_memtable_size (p) ;
                if (pr > 1) printf ("size: %ld\n", size) ;
                nblocks_actual++ ;
                fail = fail || (size != ((size_t) 1) << k) ;
                if (fail && pr > 0) printf ("    fail\n") ;
                fail = fail || (nblocks_actual > nblocks) ;
            }
            if (nblocks_actual != nblocks)
            {
                if (pr > 0) printf ("fail: # blocks " GBd " " GBd " \n",
                    nblocks_actual, nblocks) ;
                fail = true ;
            }
        }
    }
    ASSERT (!fail) ;
    #endif
    #endif
}

// free_pool_limit_get: get the limit on the # of blocks in the kth pool
GB_PUBLIC
int64_t GB_Global_free_pool_limit_get (int k)
{ 
    #ifdef _OPENMP
        int64_t nblocks = 0 ;
        if (k >= 3 && k < 64)
        {
            #pragma omp critical(GB_free_pool)
            {
                nblocks = GB_Global.free_pool_limit [k] ;
            }
        }
        return (nblocks) ;
    #else
        return (0) ;
    #endif
}

// free_pool_limit_set: set the limit on the # of blocks in the kth pool
GB_PUBLIC
void GB_Global_free_pool_limit_set (int k, int64_t nblocks)
{ 
    if (k >= 3 && k < 64)
    {
        #ifdef _OPENMP
            #pragma omp critical(GB_free_pool)
            {
                GB_Global.free_pool_limit [k] = nblocks ;
            }
        #else
            {
                GB_Global.free_pool_limit [k] = 0 ;
            }
        #endif
    }
}

// free_pool_nblocks_total:  total # of blocks in free_pool (for debug only)
GB_PUBLIC
int64_t GB_Global_free_pool_nblocks_total (void)
{
    int64_t nblocks = 0 ;
    #ifdef _OPENMP
    #pragma omp critical(GB_free_pool)
    {
        for (int k = 0 ; k < 64 ; k++)
        {
            nblocks += GB_Global.free_pool_nblocks [k] ;
        }
    }
    #endif
    return (nblocks) ;
}

//------------------------------------------------------------------------------
// get_wtime: return current wallclock time
//------------------------------------------------------------------------------

GB_PUBLIC
double GB_Global_get_wtime (void)
{ 
    return (GB_OPENMP_GET_WTIME) ;
}

