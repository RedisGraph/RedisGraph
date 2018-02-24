//------------------------------------------------------------------------------
// GB_Mask_compatible: check input and operators for type compatibility
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// check the type and dimenions of the mask

#include "GB.h"

GrB_Info GB_Mask_compatible     // check type and dimensions of mask
(
    const GrB_Matrix Mask,      // mask to check
    const GrB_Matrix C,         // C<Mask>= ...
    const GrB_Index nrows,      // size of output if C is NULL
    const GrB_Index ncols
)
{

    if (Mask != NULL)
    {

        // Mask is typecast to boolean
        if (!GB_Type_compatible (Mask->type, GrB_BOOL))
        {
            return (ERROR (GrB_DOMAIN_MISMATCH, (LOG,
                "Mask of type [%s] cannot be typecast to boolean",
                Mask->type->name))) ;
        }

        // check the Mask dimensions
        GrB_Index Mask_nrows = (C == NULL) ? nrows : C->nrows ;
        GrB_Index Mask_ncols = (C == NULL) ? ncols : C->ncols ;
        if (Mask->nrows != Mask_nrows || Mask->ncols != Mask_ncols)
        {
            return (ERROR (GrB_DIMENSION_MISMATCH, (LOG,
                "Mask is "GBd"-by-"GBd"; "
                "does not match output dimensions ("GBd"-by-"GBd")",
                Mask->nrows, Mask->ncols, Mask_nrows, Mask_ncols))) ;
        }
    }

    return (REPORT_SUCCESS) ;
}

