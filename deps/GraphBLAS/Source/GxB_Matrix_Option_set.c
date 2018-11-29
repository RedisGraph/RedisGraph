//------------------------------------------------------------------------------
// GxB_Matrix_option_set: set an option in a matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB.h"

GrB_Info GxB_Matrix_Option_set      // set an option in a matrix
(
    GrB_Matrix A,                   // descriptor to modify
    GxB_Option_Field field,         // option to change
    ...                             // value to change it to
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GB_WHERE ("GxB_Matrix_Option_set (A, field, value)") ;
    GB_RETURN_IF_NULL_OR_FAULTY (A) ;
    ASSERT_OK (GB_check (A, "A to set option", GB0)) ;

    GB_WAIT (A) ;

    //--------------------------------------------------------------------------
    // set the matrix option
    //--------------------------------------------------------------------------

    va_list ap ;
    GrB_Info info = GrB_SUCCESS ;
    bool new_csc ;
    double hyper_ratio ;
    GxB_Format_Value format ;

    switch (field)
    {

        case GxB_HYPER : 

            va_start (ap, field) ;
            hyper_ratio = va_arg (ap, double) ;
            va_end (ap) ;

            A->hyper_ratio = hyper_ratio ;

            // conform the matrix to its new desired hypersparsity
            info = GB_to_hyper_conform (A, Context) ;
            break ;

        case GxB_FORMAT : 

            va_start (ap, field) ;
            format = va_arg (ap, GxB_Format_Value) ;
            va_end (ap) ;

            // the value is normally GxB_BY_ROW (0) or GxB_BY_COL (1), but
            // any nonzero value results in GxB_BY_COL.
            new_csc = (format != GxB_BY_ROW) ;

            // conform the matrix to the new CSR/CSC format
            if (A->is_csc != new_csc)
            { 
                // A = A', done in place, and change to the new format.
                // transpose: no typecast, no op, in place of A
                info = GB_transpose (NULL, NULL, new_csc, A, NULL, Context) ;
                ASSERT (GB_IMPLIES (info == GrB_SUCCESS,
                    A->is_csc == new_csc)) ;
            }

            break ;

        default : 

            return (GB_ERROR (GrB_INVALID_VALUE, (GB_LOG,
                    "invalid option field [%d], must be one of:\n"
                    "GxB_HYPER [%d] or GxB_FORMAT [%d]",
                    (int) field, (int) GxB_HYPER, (int) GxB_FORMAT))) ;

    }

    return (info) ;
}

