//------------------------------------------------------------------------------
// GB_Global.h: definitions for global variables
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// These defintions are not visible to the user.  They are used only inside
// GraphBLAS itself.  Note that the GB_Global struct does not appear here.
// It is accessible only by the functions in GB_Global.c.

#ifndef GB_GLOBAL_H
#define GB_GLOBAL_H

void     GB_Global_queue_head_set (void *p) ;
void  *  GB_Global_queue_head_get (void) ;

void     GB_Global_mode_set (GrB_Mode mode) ;
GrB_Mode GB_Global_mode_get (void) ;

void     GB_Global_GrB_init_called_set (bool GrB_init_called) ;
bool     GB_Global_GrB_init_called_get (void) ;

void     GB_Global_nthreads_max_set (int nthreads_max) ;
int      GB_Global_nthreads_max_get (void) ;

int      GB_Global_omp_get_max_threads (void) ;

void     GB_Global_chunk_set (double chunk) ;
double   GB_Global_chunk_get (void) ;

void     GB_Global_hyper_ratio_set (double hyper_ratio) ;
double   GB_Global_hyper_ratio_get (void) ;

void     GB_Global_is_csc_set (bool is_csc) ;
double   GB_Global_is_csc_get (void) ;

void     GB_Global_Saunas_set (int id, GB_Sauna Sauna) ;
GB_Sauna GB_Global_Saunas_get (int id) ;

bool     GB_Global_Sauna_in_use_get (int id) ;
void     GB_Global_Sauna_in_use_set (int id, bool in_use) ;

void     GB_Global_abort_function_set (void (* abort_function) (void)) ;
void     GB_Global_abort_function (void) ;

void     GB_Global_malloc_function_set
         (
             void * (* malloc_function) (size_t)
         ) ;
void  *  GB_Global_malloc_function (size_t size) ;

void     GB_Global_calloc_function_set
         (
             void * (* calloc_function) (size_t, size_t)
         ) ;
void  *  GB_Global_calloc_function (size_t count, size_t size) ;

void     GB_Global_realloc_function_set
         (
             void * (* realloc_function) (void *, size_t)
         ) ;
void  *  GB_Global_realloc_function (void *p, size_t size) ;

void     GB_Global_free_function_set (void (* free_function) (void *)) ;
void     GB_Global_free_function (void *p) ;

void     GB_Global_malloc_is_thread_safe_set
         (
            bool malloc_is_thread_safe
         ) ;
bool     GB_Global_malloc_is_thread_safe_get (void) ;

void     GB_Global_malloc_tracking_set (bool malloc_tracking) ;
bool     GB_Global_malloc_tracking_get (void) ;

void     GB_Global_nmalloc_clear (void) ;
int64_t  GB_Global_nmalloc_get (void) ;
int64_t  GB_Global_nmalloc_increment (void) ;
int64_t  GB_Global_nmalloc_decrement (void) ;

void     GB_Global_malloc_debug_set (bool malloc_debug) ;
bool     GB_Global_malloc_debug_get (void) ;

void     GB_Global_malloc_debug_count_set (int64_t malloc_debug_count) ;
bool     GB_Global_malloc_debug_count_decrement (void) ;

void     GB_Global_inuse_clear (void) ;
void     GB_Global_inuse_increment (int64_t s) ;
void     GB_Global_inuse_decrement (int64_t s) ;
int64_t  GB_Global_inuse_get (void) ;
int64_t  GB_Global_maxused_get (void) ;

void     GB_Global_hack_set (int64_t hack) ;
int64_t  GB_Global_hack_get (void) ;

#endif

