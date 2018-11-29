//------------------------------------------------------------------------------
// GrB_Descriptor_set: set a field in a descriptor
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB.h"

GrB_Info GrB_Descriptor_set     // set a parameter in a descriptor
(
    GrB_Descriptor desc,        // descriptor to modify
    const GrB_Desc_Field field, // parameter to change
    const GrB_Desc_Value value  // value to change it to
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GB_WHERE ("GrB_Descriptor_set (desc, field, value)") ;
    GB_RETURN_IF_NULL_OR_FAULTY (desc) ;
    ASSERT_OK (GB_check (desc, "desc to set", GB0)) ;

    //--------------------------------------------------------------------------
    // set the parameter
    //--------------------------------------------------------------------------

    switch (field)
    {

        case GrB_OUTP : 

            if (! (value == GxB_DEFAULT || value == GrB_REPLACE))
            { 
                return (GB_ERROR (GrB_INVALID_VALUE, (GB_LOG,
                    "invalid descriptor value [%d] for GrB_OUTP field;\n"
                    "must be GxB_DEFAULT [%d] or GrB_REPLACE [%d]",
                    (int) value, (int) GxB_DEFAULT, (int) GrB_REPLACE))) ;
            }
            desc->out  = value ;
            break ;

        case GrB_MASK : 

            if (! (value == GxB_DEFAULT || value == GrB_SCMP))
            { 
                return (GB_ERROR (GrB_INVALID_VALUE, (GB_LOG,
                    "invalid descriptor value [%d] for GrB_MASK field;\n"
                    "must be GxB_DEFAULT [%d] or GrB_SCMP [%d]",
                    (int) value, (int) GxB_DEFAULT, (int) GrB_SCMP))) ;
            }
            desc->mask = value ;
            break ;

        case GrB_INP0 : 

            if (! (value == GxB_DEFAULT || value == GrB_TRAN))
            { 
                return (GB_ERROR (GrB_INVALID_VALUE, (GB_LOG,
                    "invalid descriptor value [%d] for GrB_INP0 field;\n"
                    "must be GxB_DEFAULT [%d] or GrB_TRAN [%d]",
                    (int) value, (int) GxB_DEFAULT, (int) GrB_TRAN))) ;
            }
            desc->in0  = value ;
            break ;

        case GrB_INP1 : 

            if (! (value == GxB_DEFAULT || value == GrB_TRAN))
            { 
                return (GB_ERROR (GrB_INVALID_VALUE, (GB_LOG,
                    "invalid descriptor value [%d] for GrB_INP1 field;\n"
                    "must be GxB_DEFAULT [%d] or GrB_TRAN [%d]",
                    (int) value, (int) GxB_DEFAULT, (int) GrB_TRAN))) ;
            }
            desc->in1  = value ;
            break ;

        case GxB_AxB_METHOD : 

            if (! (value == GxB_DEFAULT  || value == GxB_AxB_GUSTAVSON
                || value == GxB_AxB_HEAP || value == GxB_AxB_DOT))
            { 
                return (GB_ERROR (GrB_INVALID_VALUE, (GB_LOG,
                    "invalid descriptor value [%d] for GrB_AxB_METHOD field;\n"
                    "must be GxB_DEFAULT [%d], GxB_AxB_GUSTAVSON [%d]\n"
                    "GxB_AxB_HEAP [%d] or GxB_AxB_DOT [%d]",
                    (int) value, (int) GxB_DEFAULT, (int) GxB_AxB_GUSTAVSON,
                    (int) GxB_AxB_HEAP, (int) GxB_AxB_DOT))) ;
            }
            desc->axb  = value ;
            break ;

        default : 

            return (GB_ERROR (GrB_INVALID_VALUE, (GB_LOG,
                "invalid descriptor field [%d], must be one of:\n"
                "GrB_OUTP [%d], GrB_MASK [%d], GrB_INP0 [%d], GrB_INP1 [%d]"
                "or GxB_AxB_METHOD [%d]", (int) field,
                (int) GrB_OUTP, (int) GrB_MASK, (int) GrB_INP0, (int) GrB_INP1,
                (int) GxB_AxB_METHOD))) ;
    }

    return (GrB_SUCCESS) ;
}

