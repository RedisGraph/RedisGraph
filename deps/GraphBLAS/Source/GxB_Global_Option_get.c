//------------------------------------------------------------------------------
// GxB_Global_Option_get: get a global default option for all future matrices
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#include "GB.h"

GrB_Info GxB_Global_Option_get      // gets the current global option
(
    GxB_Option_Field field,         // option to query
    ...                             // return value of the global option
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GB_WHERE1 ("GxB_Global_Option_get (field, &value)") ;

    //--------------------------------------------------------------------------
    // get the option
    //--------------------------------------------------------------------------

    va_list ap ;

    switch (field)
    {

        //----------------------------------------------------------------------
        // matrix format
        //----------------------------------------------------------------------

        case GxB_HYPER_SWITCH : 

            {
                va_start (ap, field) ;
                double *hyper_switch = va_arg (ap, double *) ;
                va_end (ap) ;
                GB_RETURN_IF_NULL (hyper_switch) ;
                (*hyper_switch) = (double) GB_Global_hyper_switch_get ( ) ;
            }
            break ;

        case GxB_BITMAP_SWITCH : 

            {
                va_start (ap, field) ;
                double *bitmap_switch = va_arg (ap, double *) ;
                va_end (ap) ;
                GB_RETURN_IF_NULL (bitmap_switch) ;
                for (int k = 0 ; k < GxB_NBITMAP_SWITCH ; k++)
                {
                    double b = (double) GB_Global_bitmap_switch_get (k) ;
                    bitmap_switch [k] = b ;
                }
            }
            break ;

        case GxB_FORMAT : 

            {
                va_start (ap, field) ;
                GxB_Format_Value *format = va_arg (ap, GxB_Format_Value *) ;
                va_end (ap) ;
                GB_RETURN_IF_NULL (format) ;
                (*format) = (GB_Global_is_csc_get ( )) ?
                    GxB_BY_COL : GxB_BY_ROW ;
            }
            break ;

        //----------------------------------------------------------------------
        // mode from GrB_init (blocking or non-blocking)
        //----------------------------------------------------------------------

        case GxB_MODE : 

            {
                va_start (ap, field) ;
                GrB_Mode *mode = va_arg (ap, GrB_Mode *) ;
                va_end (ap) ;
                GB_RETURN_IF_NULL (mode) ;
                (*mode) = GB_Global_mode_get ( )  ;
            }
            break ;

        //----------------------------------------------------------------------
        // default number of threads
        //----------------------------------------------------------------------

        case GxB_GLOBAL_NTHREADS :      // same as GxB_NTHREADS

            {
                va_start (ap, field) ;
                int *nthreads_max = va_arg (ap, int *) ;
                va_end (ap) ;
                GB_RETURN_IF_NULL (nthreads_max) ;
                (*nthreads_max) = GB_Global_nthreads_max_get ( ) ;
            }
            break ;

        //----------------------------------------------------------------------
        // default chunk size
        //----------------------------------------------------------------------

        case GxB_GLOBAL_CHUNK :         // same as GxB_CHUNK

            {
                va_start (ap, field) ;
                double *chunk = va_arg (ap, double *) ;
                va_end (ap) ;
                GB_RETURN_IF_NULL (chunk) ;
                (*chunk) = GB_Global_chunk_get ( ) ;
            }
            break ;

        //----------------------------------------------------------------------
        // memory pool control
        //----------------------------------------------------------------------

        case GxB_MEMORY_POOL : 

            {
                va_start (ap, field) ;
                int64_t *free_pool_limit = va_arg (ap, int64_t *) ;
                va_end (ap) ;
                GB_RETURN_IF_NULL (free_pool_limit) ;
                for (int k = 0 ; k < 64 ; k++)
                { 
                    free_pool_limit [k] = GB_Global_free_pool_limit_get (k) ;
                }
            }
            break ;

        //----------------------------------------------------------------------
        // SuiteSparse:GraphBLAS version, date, license, etc
        //----------------------------------------------------------------------

        case GxB_LIBRARY_NAME : 

            {
                va_start (ap, field) ;
                char **name = va_arg (ap, char **) ;
                va_end (ap) ;
                GB_RETURN_IF_NULL (name) ;
                (*name) = GxB_IMPLEMENTATION_NAME ;
            }
            break ;

        case GxB_LIBRARY_VERSION : 

            {
                va_start (ap, field) ;
                int *version = va_arg (ap, int *) ;
                va_end (ap) ;
                GB_RETURN_IF_NULL (version) ;
                version [0] = GxB_IMPLEMENTATION_MAJOR ;
                version [1] = GxB_IMPLEMENTATION_MINOR ;
                version [2] = GxB_IMPLEMENTATION_SUB ;
            }
            break ;

        case GxB_LIBRARY_DATE : 

            {
                va_start (ap, field) ;
                char **date = va_arg (ap, char **) ;
                va_end (ap) ;
                GB_RETURN_IF_NULL (date) ;
                (*date) = GxB_IMPLEMENTATION_DATE ;
            }
            break ;

        case GxB_LIBRARY_ABOUT : 

            {
                va_start (ap, field) ;
                char **about = va_arg (ap, char **) ;
                va_end (ap) ;
                GB_RETURN_IF_NULL (about) ;
                (*about) = GxB_IMPLEMENTATION_ABOUT ;
            }
            break ;

        case GxB_LIBRARY_LICENSE : 

            {
                va_start (ap, field) ;
                char **license = va_arg (ap, char **) ;
                va_end (ap) ;
                GB_RETURN_IF_NULL (license) ;
                (*license) = GxB_IMPLEMENTATION_LICENSE ;
            }
            break ;

        case GxB_LIBRARY_COMPILE_DATE : 

            {
                va_start (ap, field) ;
                char **compile_date = va_arg (ap, char **) ;
                va_end (ap) ;
                GB_RETURN_IF_NULL (compile_date) ;
                (*compile_date) = __DATE__ ;
            }
            break ;

        case GxB_LIBRARY_COMPILE_TIME : 

            {
                va_start (ap, field) ;
                char **compile_time = va_arg (ap, char **) ;
                va_end (ap) ;
                GB_RETURN_IF_NULL (compile_time) ;
                (*compile_time) = __TIME__ ;
            }
            break ;

        case GxB_LIBRARY_URL : 

            {
                va_start (ap, field) ;
                char **url = va_arg (ap, char **) ;
                va_end (ap) ;
                GB_RETURN_IF_NULL (url) ;
                (*url) = "http://faculty.cse.tamu.edu/davis/GraphBLAS" ;
            }
            break ;

        //----------------------------------------------------------------------
        // GraphBLAS API version, date, etc
        //----------------------------------------------------------------------

        case GxB_API_VERSION : 

            {
                va_start (ap, field) ;
                int *api_version = va_arg (ap, int *) ;
                va_end (ap) ;
                GB_RETURN_IF_NULL (api_version) ;
                api_version [0] = GxB_SPEC_MAJOR ; 
                api_version [1] = GxB_SPEC_MINOR ;
                api_version [2] = GxB_SPEC_SUB ;
            }
            break ;

        case GxB_API_DATE : 

            {
                va_start (ap, field) ;
                char **api_date = va_arg (ap, char **) ;
                va_end (ap) ;
                GB_RETURN_IF_NULL (api_date) ;
                (*api_date) = GxB_SPEC_DATE ;
            }
            break ;

        case GxB_API_ABOUT : 

            {
                va_start (ap, field) ;
                char **api_about = va_arg (ap, char **) ;
                va_end (ap) ;
                GB_RETURN_IF_NULL (api_about) ;
                (*api_about) = GxB_SPEC_ABOUT ;
            }
            break ;

        case GxB_API_URL : 

            {
                va_start (ap, field) ;
                char **api_url = va_arg (ap, char **) ;
                va_end (ap) ;
                GB_RETURN_IF_NULL (api_url) ;
                (*api_url) = "http://graphblas.org" ;
            }
            break ;

        //----------------------------------------------------------------------
        // compiler used to compile GraphBLAS
        //----------------------------------------------------------------------

        case GxB_COMPILER_VERSION : 

            {
                va_start (ap, field) ;
                int *compiler_version = va_arg (ap, int *) ;
                va_end (ap) ;
                GB_RETURN_IF_NULL (compiler_version) ;
                compiler_version [0] = GB_COMPILER_MAJOR ; 
                compiler_version [1] = GB_COMPILER_MINOR ;
                compiler_version [2] = GB_COMPILER_SUB ;
            }
            break ;

        case GxB_COMPILER_NAME : 

            {
                va_start (ap, field) ;
                char **compiler_name = va_arg (ap, char **) ;
                va_end (ap) ;
                GB_RETURN_IF_NULL (compiler_name) ;
                (*compiler_name) = GB_COMPILER_NAME ;
            }
            break ;

        //----------------------------------------------------------------------
        // controlling diagnostic output
        //----------------------------------------------------------------------

        case GxB_BURBLE : 

            {
                va_start (ap, field) ;
                bool *burble = va_arg (ap, bool *) ;
                va_end (ap) ;
                GB_RETURN_IF_NULL (burble) ;
                (*burble) = GB_Global_burble_get ( ) ;
            }
            break ;

        case GxB_PRINTF : 

            {
                va_start (ap, field) ;
                void **printf_func = va_arg (ap, void **) ;
                va_end (ap) ;
                GB_RETURN_IF_NULL (printf_func) ;
                (*printf_func) = (void *) GB_Global_printf_get ( ) ;
            }
            break ;

        case GxB_FLUSH : 

            {
                va_start (ap, field) ;
                void **flush_func = va_arg (ap, void **) ;
                va_end (ap) ;
                GB_RETURN_IF_NULL (flush_func) ;
                (*flush_func) = (void *) GB_Global_flush_get ( ) ;
            }
            break ;

        case GxB_PRINT_1BASED : 

            {
                va_start (ap, field) ;
                bool *onebased = va_arg (ap, bool *) ;
                va_end (ap) ;
                GB_RETURN_IF_NULL (onebased) ;
                (*onebased) = GB_Global_print_one_based_get ( ) ;
            }
            break ;

        //----------------------------------------------------------------------
        // CUDA (DRAFT: in progress, do not use)
        //----------------------------------------------------------------------

        case GxB_GLOBAL_GPU_CONTROL :       // same as GxB_GPU_CONTROL

            {
                va_start (ap, field) ;
                GrB_Desc_Value *gpu_control = va_arg (ap, GrB_Desc_Value *) ;
                va_end (ap) ;
                GB_RETURN_IF_NULL (gpu_control) ;
                (*gpu_control) = GB_Global_gpu_control_get ( ) ;
            }
            break ;

        case GxB_GLOBAL_GPU_CHUNK :         // same as GxB_GPU_CHUNK

            {
                va_start (ap, field) ;
                double *gpu_chunk = va_arg (ap, double *) ;
                va_end (ap) ;
                GB_RETURN_IF_NULL (gpu_chunk) ;
                (*gpu_chunk) = GB_Global_gpu_chunk_get ( ) ;
            }
            break ;

        default : 

            return (GrB_INVALID_VALUE) ;
    }

    #pragma omp flush
    return (GrB_SUCCESS) ;
}

