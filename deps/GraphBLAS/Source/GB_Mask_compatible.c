//------------------------------------------------------------------------------
// GB_Mask_compatible: check input and operators for type compatibility
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// check the type and dimenions of the mask

#include "GB.h"

GrB_Info GB_Mask_compatible     // check type and dimensions of mask
(
    const GrB_Matrix Mask,      // mask to check
    const GrB_Matrix C,         // C<Mask>= ...
    const GrB_Index nrows,      // size of output if C is NULL (see GB*assign)
    const GrB_Index ncols,
    GB_Context Context
)
{ 

    ASSERT (GB_ALIAS_OK (C, Mask)) ;

    if (Mask != NULL)
    { 

        // Mask is typecast to boolean
        if (!GB_Type_compatible (Mask->type, GrB_BOOL))
        { 
            return (GB_ERROR (GrB_DOMAIN_MISMATCH, (GB_LOG,
                "Mask of type [%s] cannot be typecast to boolean",
                Mask->type->name))) ;
        }

        // check the Mask dimensions
        GrB_Index cnrows = (C == NULL) ? nrows : GB_NROWS (C) ;
        GrB_Index cncols = (C == NULL) ? ncols : GB_NCOLS (C) ;
        if (GB_NROWS (Mask) != cnrows || GB_NCOLS (Mask) != cncols)
        { 
            return (GB_ERROR (GrB_DIMENSION_MISMATCH, (GB_LOG,
                "Mask is "GBd"-by-"GBd"; "
                "does not match output dimensions ("GBu"-by-"GBu")",
                GB_NROWS (Mask), GB_NCOLS (Mask), cnrows, cncols))) ;
        }
    }

    return (GrB_SUCCESS) ;
}

