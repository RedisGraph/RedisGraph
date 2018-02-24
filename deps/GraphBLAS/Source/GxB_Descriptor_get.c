//------------------------------------------------------------------------------
// GxB_Descriptor_get: get a field in a descriptor
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB.h"

GrB_Info GxB_Descriptor_get     // get a parameter from a descriptor
(
    GrB_Desc_Value *val,        // value of the parameter
    const GrB_Descriptor desc,  // descriptor to query; NULL is ok
    const GrB_Desc_Field field  // parameter to query
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    WHERE ("GxB_Descriptor_get (&value, desc, field)") ;
    RETURN_IF_NULL (val) ;
    RETURN_IF_UNINITIALIZED (desc) ;

    //--------------------------------------------------------------------------
    // get the parameter
    //--------------------------------------------------------------------------

    switch (field)
    {
        case GrB_OUTP: (*val) = (desc==NULL) ? GxB_DEFAULT : desc->out  ; break;
        case GrB_MASK: (*val) = (desc==NULL) ? GxB_DEFAULT : desc->mask ; break;
        case GrB_INP0: (*val) = (desc==NULL) ? GxB_DEFAULT : desc->in0  ; break;
        case GrB_INP1: (*val) = (desc==NULL) ? GxB_DEFAULT : desc->in1  ; break;
        default:
            return (ERROR (GrB_INVALID_VALUE, (LOG,
                "invalid descriptor field"))) ;
    }

    return (REPORT_SUCCESS) ;
}

