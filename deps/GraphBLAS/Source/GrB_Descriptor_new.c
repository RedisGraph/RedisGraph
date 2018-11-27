//------------------------------------------------------------------------------
// GrB_Descriptor_new: create a new descriptor
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Default values are set to GxB_DEFAULT

#include "GB.h"

GrB_Info GrB_Descriptor_new     // create a new descriptor
(
    GrB_Descriptor *descriptor  // handle of descriptor to create
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GB_WHERE ("GrB_Descriptor_new (&descriptor)") ;
    GB_RETURN_IF_NULL (descriptor) ;
    (*descriptor) = NULL ;

    //--------------------------------------------------------------------------
    // create the descriptor
    //--------------------------------------------------------------------------

    // allocate the descriptor
    GB_CALLOC_MEMORY (*descriptor, 1, sizeof (struct GB_Descriptor_opaque)) ;
    if (*descriptor == NULL)
    { 
        return (GB_NO_MEMORY) ;
    }

    // initialize the descriptor
    GrB_Descriptor desc = *descriptor ;
    desc->magic = GB_MAGIC ;
    desc->out  = GxB_DEFAULT ;     // descriptor for output
    desc->mask = GxB_DEFAULT ;     // descriptor for the mask input
    desc->in0  = GxB_DEFAULT ;     // descriptor for the first input
    desc->in1  = GxB_DEFAULT ;     // descriptor for the second input
    desc->axb  = GxB_DEFAULT ;     // descriptor for C=A*B
    return (GrB_SUCCESS) ;
}

