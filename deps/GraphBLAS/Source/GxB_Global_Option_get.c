//------------------------------------------------------------------------------
// GxB_Global_Option_get: get a global default option for all future matrices
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
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

            va_start (ap, field) ;
            double *hyper_ratio = va_arg (ap, double *) ;
            va_end (ap) ;

            GB_RETURN_IF_NULL (hyper_ratio) ;
            (*hyper_ratio) = GB_Global.hyper_ratio ;
            break ;

        //----------------------------------------------------------------------
        // matrix format (CSR or CSC)
        //----------------------------------------------------------------------

        case GxB_FORMAT : 

            va_start (ap, field) ;
            GxB_Format_Value *format = va_arg (ap, GxB_Format_Value *) ;
            va_end (ap) ;

            GB_RETURN_IF_NULL (format) ;
            (*format) = (GB_Global.is_csc) ? GxB_BY_COL : GxB_BY_ROW ;
            break ;

        //----------------------------------------------------------------------
        // mode from GrB_init (blocking or non-blocking)
        //----------------------------------------------------------------------

        case GxB_MODE : 

            va_start (ap, field) ;
            GrB_Mode *mode = va_arg (ap, GrB_Mode *) ;
            va_end (ap) ;

            GB_RETURN_IF_NULL (mode) ;
            (*mode) = GB_Global.mode ;
            break ;

        //----------------------------------------------------------------------
        // threading model for synchronizing user threads
        //----------------------------------------------------------------------

        case GxB_THREAD_SAFETY : 

            va_start (ap, field) ;
            GxB_Thread_Model *thread_safety = va_arg (ap, GxB_Thread_Model *) ;
            va_end (ap) ;

            GB_RETURN_IF_NULL (thread_safety) ;
            (*thread_safety) = 

                #if defined (USER_POSIX_THREADS)
                GxB_THREAD_POSIX ;
                #elif defined (USER_WINDOWS_THREADS)
                GxB_THREAD_WINDOWS ;    // Windows threads not yet supported
                #elif defined (USER_ANSI_THREADS)
                GxB_THREAD_ANSI ;       // ANSI C11 threads not yet supported
                #elif defined (_OPENMP) || defined (USER_OPENMP_THREADS)
                GxB_THREAD_OPENMP ;
                #else
                GxB_THREAD_NONE ;       // GraphBLAS is not thread safe!
                #endif

            break ;

        //----------------------------------------------------------------------
        // internal parallel threading in GraphBLAS (currently none)
        //----------------------------------------------------------------------

        case GxB_THREADING : 

            va_start (ap, field) ;
            GxB_Thread_Model *threading = va_arg (ap, GxB_Thread_Model *) ;
            va_end (ap) ;

            GB_RETURN_IF_NULL (threading) ;
            (*threading) = GxB_THREAD_NONE ;
            break ;

        //----------------------------------------------------------------------
        // invalid option
        //----------------------------------------------------------------------

        default : 

            return (GB_ERROR (GrB_INVALID_VALUE, (GB_LOG,
                    "invalid option field [%d], must be one of:\n"
                    "GxB_HYPER [%d], GxB_FORMAT [%d], GxB_MODE [%d],"
                    "GxB_THREAD_SAFETY [%d], or GxB_THREADING [%d]",
                    (int) field, (int) GxB_HYPER, (int) GxB_FORMAT,
                    (int) GxB_MODE, (int) GxB_THREAD_SAFETY,
                    (int) GxB_THREADING))) ;

    }

    return (GrB_SUCCESS) ;
}

