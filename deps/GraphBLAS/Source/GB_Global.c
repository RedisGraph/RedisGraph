//------------------------------------------------------------------------------
// GB_Global: global values in GraphBLAS
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB.h"

//------------------------------------------------------------------------------
// All Global storage is declared and initialized here
//------------------------------------------------------------------------------

// If the user creates threads that work on GraphBLAS matrices, then all of
// those threads must share the same matrix queue, and the same mode.

GB_Global_struct GB_Global =
{

    // user-level multithreading can be disabled for testing
    #ifdef USER_NO_THREADS
    .user_multithreaded = false,
    #else
    .user_multithreaded = true,
    #endif

    // queued matrices with work to do
    .queue_head = NULL,         // pointer to first queued matrix

    // GraphBLAS mode
    .mode = GrB_NONBLOCKING,    // default is nonblocking

    // initialization flag
    .GrB_init_called = false,   // GrB_init has not yet been called

    // max number of threads
    .nthreads_max = 1,          // max number of threads

    // default format
    .hyper_ratio = GB_HYPER_DEFAULT,
    .is_csc = (GB_FORMAT_DEFAULT != GxB_BY_ROW),    // default is GxB_BY_ROW

    // Sauna workspace for Gustavson's method (one per thread)
    .Saunas [0] = NULL,
    .Sauna_in_use [0] = false,

    // abort function for debugging only
    .abort_function   = abort,

    // malloc/calloc/realloc/free functions: default to ANSI C11 functions
    .malloc_function  = malloc,
    .calloc_function  = calloc,
    .realloc_function = realloc,
    .free_function    = free,

    // malloc tracking, for testing, statistics, and debugging only
    .malloc_tracking = false,
    .nmalloc = 0,                // memory block counter
    .malloc_debug = false,       // do not test memory handling
    .malloc_debug_count = 0,     // counter for testing memory handling
    .inuse = 0,                  // memory space current in use
    .maxused = 0                 // high water memory usage

} ;

//------------------------------------------------------------------------------
// GB_Global access functions
//------------------------------------------------------------------------------

int GB_Global_nthreads_max_get ( )
{
    return (GB_Global.nthreads_max) ;
}

int64_t GB_Global_nmalloc_get ( )
{
    return (GB_Global.nmalloc) ;
}

void GB_Global_nmalloc_clear ( )
{
    GB_Global.nmalloc = 0 ;
}

int64_t GB_Global_nmalloc_decrement ( )
{
    return (--(GB_Global.nmalloc)) ;
}

int64_t GB_Global_nmalloc_increment ( )
{
    return (++(GB_Global.nmalloc)) ;
}

void GB_Global_abort_function_set (void (* abort_function) (void))
{
    GB_Global.abort_function = abort_function ;
}

void GB_Global_abort_function_call ( )
{
    GB_Global.abort_function ( ) ;
}

void GB_Global_GrB_init_called_set (bool GrB_init_called)
{
    GB_Global.GrB_init_called = GrB_init_called ;
}

void GB_Global_malloc_tracking_set (bool malloc_tracking)
{
    GB_Global.malloc_tracking = malloc_tracking ;
}

bool GB_Global_malloc_tracking_get ( )
{
    return (GB_Global.malloc_tracking) ;
}

void GB_Global_malloc_debug_set (bool malloc_debug)
{
    GB_Global.malloc_debug = malloc_debug ;
}

bool GB_Global_malloc_debug_get ( )
{
    return (GB_Global.malloc_debug) ;
}

void GB_Global_malloc_debug_count_set (int64_t malloc_debug_count)
{
    GB_Global.malloc_debug_count = malloc_debug_count ;
}

int64_t GB_Global_inuse_get ( )
{
    return (GB_Global.inuse) ;
}

void GB_Global_inuse_clear ( )
{
    GB_Global.inuse = 0 ;
    GB_Global.maxused = 0 ;
}

void GB_Global_inuse_increment (int64_t s)
{
    GB_Global.inuse += s ;
    GB_Global.maxused = GB_IMAX (GB_Global.maxused, GB_Global.inuse) ;
}

void GB_Global_inuse_decrement (int64_t s)
{
    GB_Global.inuse -= s ;
}

int64_t GB_Global_maxused_get ( )
{
    return (GB_Global.maxused) ;
}

void *GB_Global_queue_head_get ( )
{
    return (GB_Global.queue_head) ;
}

void GB_Global_queue_head_set (void *p)
{
    GB_Global.queue_head = p ;
}

void GB_Global_mode_set (GrB_Mode mode)
{
    GB_Global.mode = mode ;
}

GB_Sauna GB_Global_Saunas_get (int id)
{
    return (GB_Global.Saunas [id]) ;
}

void GB_Global_user_multithreaded_set (bool user_multithreaded)
{
    GB_Global.user_multithreaded = user_multithreaded ;
}

