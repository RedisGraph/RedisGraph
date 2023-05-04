//------------------------------------------------------------------------------
// GB_Global.h: definitions for global data
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// These defintions are not visible to the user.  They are used only inside
// GraphBLAS itself.  Note that the GB_Global struct does not appear here.
// It is accessible only by the functions in GB_Global.c.

#ifndef GB_GLOBAL_H
#define GB_GLOBAL_H

void     GB_Global_cpu_features_query (void) ;
bool     GB_Global_cpu_features_avx2 (void) ;
bool     GB_Global_cpu_features_avx512f (void) ;

void     GB_Global_mode_set (GrB_Mode mode) ;
GrB_Mode GB_Global_mode_get (void) ;

void     GB_Global_sort_set (int sort) ;
int      GB_Global_sort_get (void) ;

void     GB_Global_GrB_init_called_set (bool GrB_init_called) ;
bool     GB_Global_GrB_init_called_get (void) ;

void     GB_Global_nthreads_max_set (int nthreads_max) ;
int      GB_Global_nthreads_max_get (void) ;

int      GB_Global_omp_get_max_threads (void) ;

void     GB_Global_chunk_set (double chunk) ;
double   GB_Global_chunk_get (void) ;

void     GB_Global_hyper_switch_set (float hyper_switch) ;
float    GB_Global_hyper_switch_get (void) ;

void     GB_Global_bitmap_switch_set (int k, float b) ;
float    GB_Global_bitmap_switch_get (int k) ;
float    GB_Global_bitmap_switch_matrix_get
                        (int64_t vlen, int64_t vdim) ;
void     GB_Global_bitmap_switch_default (void) ;

void     GB_Global_is_csc_set (bool is_csc) ;
bool     GB_Global_is_csc_get (void) ;

void     GB_Global_abort_function_set
                        (void (* abort_function) (void)) ;
void     GB_Global_abort_function (void) ;

void     GB_Global_malloc_function_set (void * (* malloc_function) (size_t)) ;
void  *  GB_Global_malloc_function (size_t size) ;
void     GB_Global_realloc_function_set
            (void * (* realloc_function) (void *, size_t)) ;
void  *  GB_Global_realloc_function (void *p, size_t size) ;
bool     GB_Global_have_realloc_function (void) ;
void     GB_Global_free_function_set (void (* free_function) (void *)) ;
void     GB_Global_free_function (void *p) ;

void     GB_Global_malloc_is_thread_safe_set (bool malloc_is_thread_safe) ;
bool     GB_Global_malloc_is_thread_safe_get (void) ;

void     GB_Global_malloc_tracking_set (bool malloc_tracking) ;
bool     GB_Global_malloc_tracking_get (void) ;

void     GB_Global_nmalloc_clear (void) ;
int64_t  GB_Global_nmalloc_get (void) ;

void     GB_Global_malloc_debug_set (bool malloc_debug) ;
bool     GB_Global_malloc_debug_get (void) ;

void     GB_Global_malloc_debug_count_set (int64_t malloc_debug_count) ;
bool     GB_Global_malloc_debug_count_decrement (void) ;

void     GB_Global_hack_set (int k, int64_t hack) ;
int64_t  GB_Global_hack_get (int k) ;

void     GB_Global_burble_set (bool burble) ;
bool     GB_Global_burble_get (void) ;

void     GB_Global_print_one_based_set (bool onebased) ;
bool     GB_Global_print_one_based_get (void) ;

void     GB_Global_print_mem_shallow_set (bool mem_shallow) ;
bool     GB_Global_print_mem_shallow_get (void) ;

void     GB_Global_gpu_control_set (GrB_Desc_Value value) ;
GrB_Desc_Value GB_Global_gpu_control_get (void);
void     GB_Global_gpu_chunk_set (double gpu_chunk) ;
double   GB_Global_gpu_chunk_get (void) ;
bool     GB_Global_gpu_count_set (bool enable_cuda) ;
int      GB_Global_gpu_count_get (void) ;
size_t   GB_Global_gpu_memorysize_get (int device) ;
int      GB_Global_gpu_sm_get (int device) ;
bool     GB_Global_gpu_device_pool_size_set (int device, size_t size) ;
bool     GB_Global_gpu_device_max_pool_size_set (int device, size_t size) ;
bool     GB_Global_gpu_device_memory_resource_set (int device, void *resource) ;
void*    GB_Global_gpu_device_memory_resource_get (int device) ;
bool     GB_Global_gpu_device_properties_get (int device) ;

void     GB_Global_timing_clear_all (void) ;
void     GB_Global_timing_clear (int k) ;
void     GB_Global_timing_set (int k, double t) ;
void     GB_Global_timing_add (int k, double t) ;
double   GB_Global_timing_get (int k) ;

int      GB_Global_memtable_n (void) ;
void     GB_Global_memtable_dump (void) ;
void     GB_Global_memtable_clear (void) ;
void     GB_Global_memtable_add (void *p, size_t size) ;
size_t   GB_Global_memtable_size (void *p) ;
void     GB_Global_memtable_remove (void *p) ;
bool     GB_Global_memtable_find (void *p) ;

void     GB_Global_free_pool_init (bool clear) ;
void    *GB_Global_free_pool_get (int k) ;
bool     GB_Global_free_pool_put (void *p, int k) ;
void     GB_Global_free_pool_dump (int pr) ;
int64_t  GB_Global_free_pool_limit_get (int k) ;
void     GB_Global_free_pool_limit_set (int64_t *limit) ;
int64_t  GB_Global_free_pool_nblocks_total (void) ;

typedef int (* GB_flush_function_t) (void) ;
typedef int (* GB_printf_function_t) (const char *restrict format, ...) ;

GB_printf_function_t GB_Global_printf_get (void) ;
void     GB_Global_printf_set (GB_printf_function_t p) ;

GB_flush_function_t GB_Global_flush_get (void) ;
void     GB_Global_flush_set (GB_flush_function_t p) ;

double   GB_Global_get_wtime (void) ;
#endif

