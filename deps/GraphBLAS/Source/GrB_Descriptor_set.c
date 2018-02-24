//------------------------------------------------------------------------------
// GrB_Descriptor_set: set a field in a descriptor
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB.h"

GrB_Info GrB_Descriptor_set     // set a parameter in a descriptor
(
    GrB_Descriptor desc,        // descriptor to modify
    const GrB_Desc_Field field, // parameter to change
    const GrB_Desc_Value val    // value to change it to
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    WHERE ("GrB_Descriptor_set (desc, field, value)") ;
    RETURN_IF_NULL_OR_UNINITIALIZED (desc) ;
    ASSERT_OK (GB_check (desc, "desc to set", 0)) ;

    //--------------------------------------------------------------------------
    // set the parameter
    //--------------------------------------------------------------------------

    switch (field)
    {

        case GrB_OUTP:

            if (! (val == GxB_DEFAULT || val == GrB_REPLACE))
            {
                return (ERROR (GrB_INVALID_VALUE, (LOG,
                    "invalid descriptor value [%d] for GrB_OUTP field;\n"
                    "must be GxB_DEFAULT [%d] or GrB_REPLACE [%d]",
                    val, GxB_DEFAULT, GrB_REPLACE))) ;
            }
            desc->out  = val ;
            break ;

        case GrB_MASK:

            if (! (val == GxB_DEFAULT || val == GrB_SCMP))
            {
                return (ERROR (GrB_INVALID_VALUE, (LOG,
                    "invalid descriptor value [%d] for GrB_MASK field;\n"
                    "must be GxB_DEFAULT [%d] or GrB_SCMP [%d]",
                    val, GxB_DEFAULT, GrB_SCMP))) ;
            }
            desc->mask = val ;
            break ;

        case GrB_INP0:

            if (! (val == GxB_DEFAULT || val == GrB_TRAN))
            {
                return (ERROR (GrB_INVALID_VALUE, (LOG,
                    "invalid descriptor value [%d] for GrB_INP0 field;\n"
                    "must be GxB_DEFAULT [%d] or GrB_TRAN [%d]",
                    val, GxB_DEFAULT, GrB_TRAN))) ;
            }
            desc->in0  = val ;
            break ;

        case GrB_INP1:

            if (! (val == GxB_DEFAULT || val == GrB_TRAN))
            {
                return (ERROR (GrB_INVALID_VALUE, (LOG,
                    "invalid descriptor value [%d] for GrB_INP1 field;\n"
                    "must be GxB_DEFAULT [%d] or GrB_TRAN [%d]",
                    val, GxB_DEFAULT, GrB_TRAN))) ;
            }
            desc->in1  = val ;
            break ;

        default:

            return (ERROR (GrB_INVALID_VALUE, (LOG,
                "invalid descriptor field [%d], must be one of:\n"
                "GrB_OUTP [%d], GrB_MASK [%d], GrB_INP0 [%d], or GrB_INP1 [%d]",
                field, GrB_OUTP, GrB_MASK, GrB_INP0, GrB_INP1))) ;
    }

    return (REPORT_SUCCESS) ;
}

