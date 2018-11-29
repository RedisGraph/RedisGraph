//------------------------------------------------------------------------------
// GxB_Global_Option_set: set a global default option for all future matrices
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

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

    switch (field)
    {

        case GxB_HYPER : 

            va_start (ap, field) ;
            GB_Global.hyper_ratio = va_arg (ap, double) ;
            va_end (ap) ;
            break ;

        case GxB_FORMAT : 

            va_start (ap, field) ;
            GB_Global.is_csc = (va_arg (ap, GxB_Format_Value) != GxB_BY_ROW) ; 
            va_end (ap) ;
            break ;

        default : 

            return (GB_ERROR (GrB_INVALID_VALUE, (GB_LOG,
                    "invalid option field [%d], must be one of:\n"
                    "GxB_HYPER [%d] or GxB_FORMAT [%d]",
                    (int) field, (int) GxB_HYPER, (int) GxB_FORMAT))) ;

    }

    return (GrB_SUCCESS) ;
}

