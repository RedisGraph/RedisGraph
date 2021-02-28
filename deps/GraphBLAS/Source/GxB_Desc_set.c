//------------------------------------------------------------------------------
// GxB_Desc_set: set a field in a descriptor
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

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

    GB_WHERE ("GxB_Desc_set (desc, field, value)") ;
    GB_RETURN_IF_NULL_OR_FAULTY (desc) ;
    ASSERT_DESCRIPTOR_OK (desc, "desc to set", GB0) ;

    if (desc->predefined)
    { 
        return (GB_ERROR (GrB_INVALID_VALUE, (GB_LOG,
            "predefined descriptors may not be modified"))) ;
    }

    //--------------------------------------------------------------------------
    // set the parameter
    //--------------------------------------------------------------------------

    va_list ap ;

    switch (field)
    {

        case GrB_OUTP : 

            {
                va_start (ap, field) ;
                GrB_Desc_Value value = va_arg (ap, GrB_Desc_Value) ;
                va_end (ap) ;
                if (! (value == GxB_DEFAULT || value == GrB_REPLACE))
                { 
                    return (GB_ERROR (GrB_INVALID_VALUE, (GB_LOG,
                        "invalid descriptor value [%d] for GrB_OUTP field;\n"
                        "must be GxB_DEFAULT [%d] or GrB_REPLACE [%d]",
                        (int) value, (int) GxB_DEFAULT, (int) GrB_REPLACE))) ;
                }
                desc->out  = value ;
            }
            break ;

        case GrB_MASK : 

            {
                va_start (ap, field) ;
                GrB_Desc_Value value = va_arg (ap, GrB_Desc_Value) ;
                va_end (ap) ;
                if (! (value == GxB_DEFAULT ||
                       value == GrB_COMP ||
                       value == GrB_STRUCTURE ||
                       value == (GrB_COMP + GrB_STRUCTURE)))
                { 
                    return (GB_ERROR (GrB_INVALID_VALUE, (GB_LOG,
                        "invalid descriptor value [%d] for GrB_MASK field;\n"
                        "must be GxB_DEFAULT [%d], GrB_COMP [%d],\n"
                        "GrB_STRUCTURE [%d], or GrB_COMP+GrB_STRUCTURE [%d]",
                        (int) value, (int) GxB_DEFAULT, (int) GrB_COMP,
                        (int) GrB_STRUCTURE,
                        (int) (GrB_COMP + GrB_STRUCTURE)))) ;
                }
                switch (value)
                {
                    case GrB_COMP:      desc->mask |= GrB_COMP ;      break ;
                    case GrB_STRUCTURE: desc->mask |= GrB_STRUCTURE ; break ;
                    default:            desc->mask = value ;          break ;
                }
            }
            break ;

        case GrB_INP0 : 

            {
                va_start (ap, field) ;
                GrB_Desc_Value value = va_arg (ap, GrB_Desc_Value) ;
                va_end (ap) ;
                if (! (value == GxB_DEFAULT || value == GrB_TRAN))
                { 
                    return (GB_ERROR (GrB_INVALID_VALUE, (GB_LOG,
                        "invalid descriptor value [%d] for GrB_INP0 field;\n"
                        "must be GxB_DEFAULT [%d] or GrB_TRAN [%d]",
                        (int) value, (int) GxB_DEFAULT, (int) GrB_TRAN))) ;
                }
                desc->in0  = value ;
            }
            break ;

        case GrB_INP1 : 

            {
                va_start (ap, field) ;
                GrB_Desc_Value value = va_arg (ap, GrB_Desc_Value) ;
                va_end (ap) ;
                if (! (value == GxB_DEFAULT || value == GrB_TRAN))
                { 
                    return (GB_ERROR (GrB_INVALID_VALUE, (GB_LOG,
                        "invalid descriptor value [%d] for GrB_INP1 field;\n"
                        "must be GxB_DEFAULT [%d] or GrB_TRAN [%d]",
                        (int) value, (int) GxB_DEFAULT, (int) GrB_TRAN))) ;
                }
                desc->in1  = value ;
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
                GrB_Desc_Value value = va_arg (ap, GrB_Desc_Value) ;
                va_end (ap) ;
                if (! (value == GxB_DEFAULT  || value == GxB_AxB_GUSTAVSON
                    || value == GxB_AxB_HEAP || value == GxB_AxB_DOT
                    || value == GxB_AxB_HASH || value == GxB_AxB_SAXPY))
                { 
                    return (GB_ERROR (GrB_INVALID_VALUE, (GB_LOG,
                        "invalid descriptor value [%d] for GrB_AxB_METHOD"
                        " field;\nmust be GxB_DEFAULT [%d], GxB_AxB_GUSTAVSON"
                        " [%d]\nGxB_AxB_HEAP [%d], GxB_AxB_DOT [%d]\n"
                        " GxB_AxB_HASH [%d] or GxB_AxB_SAXPY [%d]",
                        (int) value, (int) GxB_DEFAULT, (int) GxB_AxB_GUSTAVSON,
                        (int) GxB_AxB_HEAP, (int) GxB_AxB_DOT,
                        (int) GxB_AxB_HASH, (int) GxB_AxB_SAXPY))) ;
                }
                desc->axb  = value ;
            }
            break ;

        default : 

            return (GB_ERROR (GrB_INVALID_VALUE, (GB_LOG,
                "invalid descriptor field [%d], must be one of:\n"
                "GrB_OUTP [%d], GrB_MASK [%d], GrB_INP0 [%d], GrB_INP1 [%d]\n"
                "GxB_NTHREADS [%d], GxB_CHUNK [%d] or GxB_AxB_METHOD [%d]",
                (int) field, (int) GrB_OUTP, (int) GrB_MASK, (int) GrB_INP0,
                (int) GrB_INP1, (int) GxB_NTHREADS, (int) GxB_CHUNK,
                (int) GxB_AxB_METHOD))) ;
    }

    return (GrB_SUCCESS) ;
}

