//------------------------------------------------------------------------------
// GxB_Global_Option_set: set a global default option for all future matrices
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// not parallel: this function does O(1) work and is already thread-safe.

#include "GB.h"

GrB_Info GxB_Global_Option_set      // set a global default option
(
    GxB_Option_Field field,         // option to change
    ...                             // value to change it to
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GB_WHERE ("GxB_Global_Option_set (field, value)") ;

    //--------------------------------------------------------------------------
    // set the global option
    //--------------------------------------------------------------------------

    va_list ap ;
    GxB_Format_Value format ;

    switch (field)
    {

        case GxB_HYPER : 

            va_start (ap, field) ;
            GB_Global.hyper_ratio = va_arg (ap, double) ;
            va_end (ap) ;
            break ;

        case GxB_FORMAT : 

            va_start (ap, field) ;
            format = va_arg (ap, GxB_Format_Value) ;
            va_end (ap) ;

            if (! (format == GxB_BY_ROW || format == GxB_BY_COL))
            { 
                return (GB_ERROR (GrB_INVALID_VALUE, (GB_LOG,
                        "unsupported format [%d], must be one of:\n"
                        "GxB_BY_ROW [%d] or GxB_BY_COL [%d]",
                        (int) format, (int) GxB_BY_ROW, (int) GxB_BY_COL))) ;
            }

            GB_Global.is_csc = (format != GxB_BY_ROW) ; 
            break ;

        case GxB_GLOBAL_NTHREADS :      // same as GxB_NTHREADS

            va_start (ap, field) ;
            int nthreads_max_new = va_arg (ap, int) ;
            va_end (ap) ;

            // if < 1, then treat it as if nthreads_max = 1
            nthreads_max_new = GB_IMAX (1, nthreads_max_new) ;
            if (nthreads_max_new > GxB_NTHREADS_MAX)
            {
                return (GB_ERROR (GrB_INVALID_VALUE, (GB_LOG,
                    "nthreads_max [%d] must be < GxB_NTHREADS_MAX [%d]\n"
                    "Recompile with a higher value of GxB_NTHREADS_MAX,\n"
                    "using -DGxB_NTHREADS_MAX=%d (or higher, as needed)",
                    nthreads_max_new, GxB_NTHREADS_MAX, nthreads_max_new))) ;
            }
            GB_Global.nthreads_max = nthreads_max_new ;
            break ;

        default : 

            return (GB_ERROR (GrB_INVALID_VALUE, (GB_LOG,
                    "invalid option field [%d], must be one of:\n"
                    "GxB_HYPER [%d], GxB_FORMAT [%d], or GxB_NTHREADS [%d]",
                    (int) field, (int) GxB_HYPER, (int) GxB_FORMAT,
                    (int) GxB_NTHREADS))) ;

    }

    return (GrB_SUCCESS) ;
}

