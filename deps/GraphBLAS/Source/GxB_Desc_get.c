//------------------------------------------------------------------------------
// GxB_Desc_get: get a field in a descriptor
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// This is identical to GxB_Descriptor_get, just with a different order of the
// parameters.  The last argument is a pointer whose type depends on the
// field.

#include "GB.h"

GrB_Info GxB_Desc_get           // get a parameter from a descriptor
(
    GrB_Descriptor desc,        // descriptor to query; NULL is ok
    GrB_Desc_Field field,       // parameter to query
    ...                         // return value of the descriptor
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GB_WHERE ("GxB_Desc_get (desc, field, &value)") ;
    GB_RETURN_IF_FAULTY (desc) ;

    //--------------------------------------------------------------------------
    // get the parameter
    //--------------------------------------------------------------------------

    va_list ap ;
    GrB_Desc_Value *value ;

    switch (field)
    {
        case GrB_OUTP : 

            va_start (ap, field) ;
            value = va_arg (ap, GrB_Desc_Value *) ;
            va_end (ap) ;
            GB_RETURN_IF_NULL (value) ;
            (*value) = (desc == NULL) ? GxB_DEFAULT : desc->out ;
            break ;

        case GrB_MASK : 

            va_start (ap, field) ;
            value = va_arg (ap, GrB_Desc_Value *) ;
            va_end (ap) ;
            GB_RETURN_IF_NULL (value) ;
            (*value) = (desc == NULL) ? GxB_DEFAULT : desc->mask ;
            break ;

        case GrB_INP0 : 

            va_start (ap, field) ;
            value = va_arg (ap, GrB_Desc_Value *) ;
            va_end (ap) ;
            GB_RETURN_IF_NULL (value) ;
            (*value) = (desc == NULL) ? GxB_DEFAULT : desc->in0 ;
            break ;

        case GrB_INP1 : 

            va_start (ap, field) ;
            value = va_arg (ap, GrB_Desc_Value *) ;
            va_end (ap) ;
            GB_RETURN_IF_NULL (value) ;
            (*value) = (desc == NULL) ? GxB_DEFAULT : desc->in1 ;
            break ;

        case GxB_AxB_METHOD : 

            va_start (ap, field) ;
            value = va_arg (ap, GrB_Desc_Value *) ;
            va_end (ap) ;
            GB_RETURN_IF_NULL (value) ;
            (*value) = (desc == NULL) ? GxB_DEFAULT : desc->axb ;
            break ;

        default : 

            return (GB_ERROR (GrB_INVALID_VALUE, (GB_LOG,
                "invalid descriptor field [%d], must be one of:\n"
                "GrB_OUTP [%d], GrB_MASK [%d], GrB_INP0 [%d], GrB_INP1 [%d]"
                "or GxB_AxB_METHOD [%d]\n", (int) field,
                (int) GrB_OUTP, (int) GrB_MASK, (int) GrB_INP0,
                (int) GrB_INP1, (int) GxB_AxB_METHOD))) ;
    }

    return (GrB_SUCCESS) ;
}

