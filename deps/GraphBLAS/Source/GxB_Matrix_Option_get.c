//------------------------------------------------------------------------------
// GxB_Matrix_option_get: get an option in a matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB.h"

GrB_Info GxB_Matrix_Option_get      // gets the current option of a matrix
(
    GrB_Matrix A,                   // matrix to query
    GxB_Option_Field field,         // option to query
    ...                             // return value of the matrix option
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GB_WHERE ("GxB_Matrix_Option_get (A, field, &value)") ;
    GB_RETURN_IF_NULL_OR_FAULTY (A) ;
    ASSERT_OK (GB_check (A, "A to get option", GB0)) ;

    //--------------------------------------------------------------------------
    // get the option
    //--------------------------------------------------------------------------

    va_list ap ;
    double *hyper_ratio, hyper ;
    GxB_Format_Value *format ;
    bool is_csc ;

    hyper  = A->hyper_ratio ;
    is_csc = A->is_csc ;

    switch (field)
    {

        case GxB_HYPER : 

            va_start (ap, field) ;
            hyper_ratio = va_arg (ap, double *) ;
            va_end (ap) ;

            GB_RETURN_IF_NULL (hyper_ratio) ;
            (*hyper_ratio) = hyper ;
            break ;

        case GxB_FORMAT : 

            va_start (ap, field) ;
            format = va_arg (ap, GxB_Format_Value *) ;
            va_end (ap) ;

            GB_RETURN_IF_NULL (format) ;
            (*format) = (is_csc) ? GxB_BY_COL : GxB_BY_ROW ;
            break ;

        default : 

            return (GB_ERROR (GrB_INVALID_VALUE, (GB_LOG,
                    "invalid option field [%d], must be one of:\n"
                    "GxB_HYPER [%d] or GxB_FORMAT [%d]",
                    (int) field, (int) GxB_HYPER, (int) GxB_FORMAT))) ;

    }
    return (GrB_SUCCESS) ;
}
