//------------------------------------------------------------------------------
// GxB_Matrix_Option_get: get an option in a matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
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
    ASSERT_MATRIX_OK (A, "A to get option", GB0) ;

    //--------------------------------------------------------------------------
    // get the option
    //--------------------------------------------------------------------------

    va_list ap ;

    switch (field)
    {

        case GxB_HYPER : 

            {
                va_start (ap, field) ;
                double *hyper_ratio = va_arg (ap, double *) ;
                va_end (ap) ;
                GB_RETURN_IF_NULL (hyper_ratio) ;
                (*hyper_ratio) = A->hyper_ratio ;
            }
            break ;

        case GxB_FORMAT : 

            {
                va_start (ap, field) ;
                GxB_Format_Value *format = va_arg (ap, GxB_Format_Value *) ;
                va_end (ap) ;
                GB_RETURN_IF_NULL (format) ;
                (*format) = (A->is_csc) ? GxB_BY_COL : GxB_BY_ROW ;
            }
            break ;

        case GxB_IS_HYPER : 

            {
                va_start (ap, field) ;
                bool *is_hyper = va_arg (ap, bool *) ;
                va_end (ap) ;
                GB_RETURN_IF_NULL (is_hyper) ;
                (*is_hyper) = A->is_hyper ;
            }
            break ;

        default : 

            return (GB_ERROR (GrB_INVALID_VALUE, (GB_LOG,
                    "invalid option field [%d], must be one of:\n"
                    "GxB_HYPER [%d], GxB_FORMAT [%d], or GxB_IS_HYPER [%d]",
                    (int) field, (int) GxB_HYPER, (int) GxB_FORMAT,
                    (int) GxB_IS_HYPER))) ;

    }
    return (GrB_SUCCESS) ;
}
