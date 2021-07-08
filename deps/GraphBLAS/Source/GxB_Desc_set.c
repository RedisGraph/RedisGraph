//------------------------------------------------------------------------------
// GxB_Desc_set: set a field in a descriptor
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// This is identical to GrB_Descriptor_set, except that the last argument is a
// pointer whose type depends on the field.  For the four descriptor fields
// in the spec, the type is the same as GrB_Descriptor_set (a scalar of
// type GrB_Desc_Value).

#include "GB.h"

GrB_Info GxB_Desc_set           // set a parameter in a descriptor
(
    GrB_Descriptor desc,        // descriptor to modify
    GrB_Desc_Field field,       // parameter to change
    ...                         // value to change it to
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    if (desc != NULL && desc->header_size == 0)
    { 
        // built-in descriptors may not be modified
        return (GrB_INVALID_VALUE) ;
    }

    GB_WHERE (desc, "GxB_Desc_set (desc, field, value)") ;
    GB_RETURN_IF_NULL_OR_FAULTY (desc) ;
    ASSERT_DESCRIPTOR_OK (desc, "desc to set", GB0) ;

    //--------------------------------------------------------------------------
    // set the parameter
    //--------------------------------------------------------------------------

    va_list ap ;

    switch (field)
    {

        case GrB_OUTP : 

            {
                va_start (ap, field) ;
                int value = va_arg (ap, int) ;
                va_end (ap) ;
                if (! (value == GxB_DEFAULT || value == GrB_REPLACE))
                { 
                    GB_ERROR (GrB_INVALID_VALUE,
                        "invalid descriptor value [%d] for GrB_OUTP field;\n"
                        "must be GxB_DEFAULT [%d] or GrB_REPLACE [%d]",
                        value, (int) GxB_DEFAULT, (int) GrB_REPLACE) ;
                }
                desc->out = (GrB_Desc_Value) value ;
            }
            break ;

        case GrB_MASK : 

            {
                va_start (ap, field) ;
                int value = va_arg (ap, int) ;
                va_end (ap) ;
                if (! (value == GxB_DEFAULT ||
                       value == GrB_COMP ||
                       value == GrB_STRUCTURE ||
                       value == (GrB_COMP + GrB_STRUCTURE)))
                { 
                    GB_ERROR (GrB_INVALID_VALUE,
                        "invalid descriptor value [%d] for GrB_MASK field;\n"
                        "must be GxB_DEFAULT [%d], GrB_COMP [%d],\n"
                        "GrB_STRUCTURE [%d], or GrB_COMP+GrB_STRUCTURE [%d]",
                        value, (int) GxB_DEFAULT, (int) GrB_COMP,
                        (int) GrB_STRUCTURE,
                        (int) (GrB_COMP + GrB_STRUCTURE)) ;
                }
                int mask = (int) desc->mask ;
                switch (value)
                {
                    case GrB_COMP      : mask |= GrB_COMP ;      break ;
                    case GrB_STRUCTURE : mask |= GrB_STRUCTURE ; break ;
                    default            : mask = value ;          break ;
                }
                desc->mask = (GrB_Desc_Value) mask ;
            }
            break ;

        case GrB_INP0 : 

            {
                va_start (ap, field) ;
                int value = va_arg (ap, int) ;
                va_end (ap) ;
                if (! (value == GxB_DEFAULT || value == GrB_TRAN))
                { 
                    GB_ERROR (GrB_INVALID_VALUE,
                        "invalid descriptor value [%d] for GrB_INP0 field;\n"
                        "must be GxB_DEFAULT [%d] or GrB_TRAN [%d]",
                        value, (int) GxB_DEFAULT, (int) GrB_TRAN) ;
                }
                desc->in0 = (GrB_Desc_Value) value ;
            }
            break ;

        case GrB_INP1 : 

            {
                va_start (ap, field) ;
                int value = va_arg (ap, int) ;
                va_end (ap) ;
                if (! (value == GxB_DEFAULT || value == GrB_TRAN))
                { 
                    GB_ERROR (GrB_INVALID_VALUE,
                        "invalid descriptor value [%d] for GrB_INP1 field;\n"
                        "must be GxB_DEFAULT [%d] or GrB_TRAN [%d]",
                        value, (int) GxB_DEFAULT, (int) GrB_TRAN) ;
                }
                desc->in1 = (GrB_Desc_Value) value ;
            }
            break ;

        case GxB_DESCRIPTOR_NTHREADS :      // same as GxB_NTHREADS

            {
                va_start (ap, field) ;
                desc->nthreads_max = va_arg (ap, int) ;
                va_end (ap) ;
            }
            break ;

        case GxB_DESCRIPTOR_CHUNK :         // same as GxB_CHUNK

            {
                va_start (ap, field) ;
                desc->chunk = va_arg (ap, double) ;
                va_end (ap) ;
            }
            break ;

        case GxB_AxB_METHOD : 

            {
                va_start (ap, field) ;
                int value = va_arg (ap, int) ;
                va_end (ap) ;
                if (! (value == GxB_DEFAULT  || value == GxB_AxB_GUSTAVSON
                    || value == GxB_AxB_DOT
                    || value == GxB_AxB_HASH || value == GxB_AxB_SAXPY))
                { 
                    GB_ERROR (GrB_INVALID_VALUE,
                        "invalid descriptor value [%d] for GrB_AxB_METHOD"
                        " field;\nmust be GxB_DEFAULT [%d], GxB_AxB_GUSTAVSON"
                        " [%d]\nGxB_AxB_DOT [%d]"
                        " GxB_AxB_HASH [%d] or GxB_AxB_SAXPY [%d]",
                        value, (int) GxB_DEFAULT, (int) GxB_AxB_GUSTAVSON,
                        (int) GxB_AxB_DOT,
                        (int) GxB_AxB_HASH, (int) GxB_AxB_SAXPY) ;
                }
                desc->axb = (GrB_Desc_Value) value ;
            }
            break ;

        case GxB_SORT :

            {
                va_start (ap, field) ;
                desc->do_sort = va_arg (ap, int) ;
                va_end (ap) ;
            }
            break ;

        default : 

            GB_ERROR (GrB_INVALID_VALUE,
                "invalid descriptor field [%d], must be one of:\n"
                "GrB_OUTP [%d], GrB_MASK [%d], GrB_INP0 [%d], GrB_INP1 [%d]\n"
                "GxB_NTHREADS [%d], GxB_CHUNK [%d], GxB_AxB_METHOD [%d]\n"
                "or GxB_SORT [%d]\n",
                (int) field, (int) GrB_OUTP, (int) GrB_MASK, (int) GrB_INP0,
                (int) GrB_INP1, (int) GxB_NTHREADS, (int) GxB_CHUNK,
                (int) GxB_AxB_METHOD, (int) GxB_SORT) ;
    }

    return (GrB_SUCCESS) ;
}

