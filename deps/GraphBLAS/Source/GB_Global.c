//------------------------------------------------------------------------------
// GB_Global: global values in GraphBLAS
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// All Global storage is declared, initialized, and accessed here.  The
// contents of the GB_Global struct are only accessible to functions in this
// file.  Global storage is used to keep track of the GraphBLAS mode (blocking
// or non-blocking), for pointers to malloc/calloc/realloc/free functions,
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

    GrB_Mode mode ;             // GrB_NONBLOCKING or GrB_BLOCKING
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
    // malloc/calloc/realloc/free: memory management functions
    //--------------------------------------------------------------------------

    // All threads must use the same malloc/calloc/realloc/free functions.
    // They default to the ANSI C11 functions, but can be defined by GxB_init.

    void * (* malloc_function  ) (size_t)         ;
    void * (* calloc_function  ) (size_t, size_t) ;
    void * (* realloc_function ) (void *, size_t) ;
    void   (* free_function    ) (void *)         ;
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

    int64_t hack ;                  // ad hoc setting (for draft versions only)

    //--------------------------------------------------------------------------
    // diagnostic output
    //--------------------------------------------------------------------------

    bool burble ;                   // controls GBURBLE output

    //--------------------------------------------------------------------------
    // for MATLAB interface only
    //--------------------------------------------------------------------------

    bool print_one_based ;          // if true, print 1-based indices

    //--------------------------------------------------------------------------
    // CUDA (DRAFT: in progress)
    //--------------------------------------------------------------------------

    int gpu_count ;                 // # of GPUs in the system
    GrB_Desc_Value gpu_control ;    // always, never, or default
    double gpu_chunk ;              // min problem size for using a GPU
    // properties of each GPU:
    GB_cuda_device gpu_properties [GB_CUDA_MAX_GPUS] ;

    //--------------------------------------------------------------------------
    // timing: for code development only
    //--------------------------------------------------------------------------

    double timing [20] ;

    // #include "GB_Global_struct_mkl_template.c"
}
GB_Global_struct ;

GB_PUBLIC GB_Global_struct GB_Global ;

GB_Global_struct GB_Global =
{

    // GraphBLAS mode
    .mode = GrB_NONBLOCKING,    // default is nonblocking

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
    .hyper_switch = GB_HYPER_SWITCH_DEFAULT,
    .bitmap_switch = {
        GB_BITSWITCH_1,
        GB_BITSWITCH_2,
        GB_BITSWITCH_3_to_4,
        GB_BITSWITCH_5_to_8,
        GB_BITSWITCH_9_to_16,
        GB_BITSWITCH_17_to_32,
        GB_BITSWITCH_33_to_64,
        GB_BITSWITCH_gt_than_64 },

    .is_csc = (GB_FORMAT_DEFAULT != GxB_BY_ROW),    // default is GxB_BY_ROW

    // abort function for debugging only
    .abort_function   = abort,

    // malloc/calloc/realloc/free functions: default to ANSI C11 functions
    .malloc_function  = malloc,
    .calloc_function  = calloc,
    .realloc_function = realloc,
    .free_function    = free,
    .malloc_is_thread_safe = true,

    // malloc tracking, for testing, statistics, and debugging only
    .malloc_tracking = false,
    .nmalloc = 0,                // memory block counter
    .malloc_debug = false,       // do not test memory handling
    .malloc_debug_count = 0,     // counter for testing memory handling

    // for testing and development only
    .hack = 0,

    // diagnostics
    .burble = false,

    // for MATLAB interface only
    .print_one_based = false,   // if true, print 1-based indices

    // #include "GB_Global_init_mkl_template.c'

    // CUDA environment (DRAFT: in progress)
    .gpu_count = 0,                     // # of GPUs in the system
    .gpu_control = GxB_DEFAULT,         // always, never, or default
    .gpu_chunk = GB_GPU_CHUNK_DEFAULT   // min problem size for using a GPU

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
// malloc_function
//------------------------------------------------------------------------------

void GB_Global_malloc_function_set (void * (* malloc_function) (size_t))
{ 
    GB_Global.malloc_function = malloc_function ;
}

void * GB_Global_malloc_function (size_t size)
{ 
    bool ok = true ;
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
    return (ok ? p : NULL) ;
}

//------------------------------------------------------------------------------
// calloc_function
//------------------------------------------------------------------------------

void GB_Global_calloc_function_set (void * (* calloc_function) (size_t, size_t))
{ 
    GB_Global.calloc_function = calloc_function ;
}

void * GB_Global_calloc_function (size_t count, size_t size)
{ 
    bool ok = true ;
    void *p = NULL ;
    if (GB_Global.malloc_is_thread_safe)
    {
        p = GB_Global.calloc_function (count, size) ;
    }
    else
    {
        #pragma omp critical(GB_malloc_protection)
        {
            p = GB_Global.calloc_function (count, size) ;
        }
    }
    return (ok ? p : NULL) ;
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
    bool ok = true ;
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
    return (ok ? pnew : NULL) ;
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

void GB_Global_nmalloc_increment (void)
{ 
    GB_ATOMIC_UPDATE
    GB_Global.nmalloc++ ;
}

GB_PUBLIC
void GB_Global_nmalloc_decrement (void)
{ 
    GB_ATOMIC_UPDATE
    GB_Global.nmalloc-- ;
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
// hack: for setting an internal value for development only
//------------------------------------------------------------------------------

GB_PUBLIC
void GB_Global_hack_set (int64_t hack)
{ 
    GB_Global.hack = hack ;
}

GB_PUBLIC
int64_t GB_Global_hack_get (void)
{ 
    return (GB_Global.hack) ;
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

//------------------------------------------------------------------------------
// for MATLAB interface only
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
    return (GB_Global.gpu_properties [device].number_of_sms)  ;
}

bool GB_Global_gpu_device_pool_size_set( int device, size_t size)
{
    GB_GPU_DEVICE_CHECK (0) ;       // zero if invalid GPU
    GB_Global.gpu_properties [device].pool_size = (int) size ;
    return( true); 
}

bool GB_Global_gpu_device_max_pool_size_set( int device, size_t size)
{
    GB_GPU_DEVICE_CHECK (0) ;       // zero if invalid GPU
    GB_Global.gpu_properties[device].max_pool_size = (int) size ;
    return( true); 
}

bool GB_Global_gpu_device_memory_resource_set( int device, void *resource)
{
    GB_GPU_DEVICE_CHECK (0) ;       // zero if invalid GPU
    GB_Global.gpu_properties[device].memory_resource = resource;
    return( true); 
}

void* GB_Global_gpu_device_memory_resource_get( int device )
{
    GB_GPU_DEVICE_CHECK (0) ;       // zero if invalid GPU
    return ( GB_Global.gpu_properties [device].memory_resource ) ;
    //NOTE: this returns a void*, needs to be cast to be used
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
    for (int k = 0 ; k < 20 ; k++)
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

// #include "GB_Global_mkl_template.c

