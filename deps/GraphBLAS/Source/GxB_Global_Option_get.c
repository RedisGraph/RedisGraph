//------------------------------------------------------------------------------
// GxB_Global_Option_get: get a global default option for all future matrices
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

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

    GB_WHERE ("GxB_Global_Option_get (field, &value)") ;

    //--------------------------------------------------------------------------
    // get the option
    //--------------------------------------------------------------------------

    va_list ap ;

    switch (field)
    {

        //----------------------------------------------------------------------
        // hyper_ratio
        //----------------------------------------------------------------------

        case GxB_HYPER : 

            {
                va_start (ap, field) ;
                double *hyper_ratio = va_arg (ap, double *) ;
                va_end (ap) ;
                GB_RETURN_IF_NULL (hyper_ratio) ;
                (*hyper_ratio) = GB_Global_hyper_ratio_get ( ) ;
            }
            break ;

        //----------------------------------------------------------------------
        // matrix format (CSR or CSC)
        //----------------------------------------------------------------------

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
        // threading model for synchronizing user threads
        //----------------------------------------------------------------------

        case GxB_THREAD_SAFETY : 

            {
                va_start (ap, field) ;
                GxB_Thread_Model *safety = va_arg (ap, GxB_Thread_Model *) ;
                va_end (ap) ;
                GB_RETURN_IF_NULL (safety) ;
                (*safety) = 
                    #if defined (USER_POSIX_THREADS)
                    GxB_THREAD_POSIX ;
                    #elif defined (USER_WINDOWS_THREADS)
                    GxB_THREAD_WINDOWS ;    // not yet supported
                    #elif defined (USER_ANSI_THREADS)
                    GxB_THREAD_ANSI ;       // not yet supported
                    #elif defined ( _OPENMP ) || defined (USER_OPENMP_THREADS)
                    GxB_THREAD_OPENMP ;
                    #else
                    GxB_THREAD_NONE ;       // GraphBLAS is not thread safe!
                    #endif
            }
            break ;

        //----------------------------------------------------------------------
        // internal parallel threading in GraphBLAS
        //----------------------------------------------------------------------

        case GxB_THREADING : 

            {
                va_start (ap, field) ;
                GxB_Thread_Model *threading = va_arg (ap, GxB_Thread_Model *) ;
                va_end (ap) ;
                GB_RETURN_IF_NULL (threading) ;
                #if defined ( _OPENMP )
                (*threading) = GxB_THREAD_OPENMP ;
                #else
                (*threading) = GxB_THREAD_NONE ;
                #endif
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
        // SuiteSparse:GraphBLAS version, etc
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
        // GraphBLAS API version, tec
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
        // controlling diagnostic output, for development only
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

        //----------------------------------------------------------------------
        // invalid option
        //----------------------------------------------------------------------

        default : 

            return (GB_ERROR (GrB_INVALID_VALUE, (GB_LOG,
                    "invalid option field [%d]\n", (int) field))) ;
    }

    return (GrB_SUCCESS) ;
}

