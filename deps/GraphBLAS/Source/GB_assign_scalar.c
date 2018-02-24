//------------------------------------------------------------------------------
// GB_assign_scalar: scalar expansion: C<Mask>(I,J) = accum (C(I,J),x)
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Assigns a single scalar to a submatrix, C<Mask>(I,J) = accum (C(I,J),x)

// This function does the work for GrB_Matrix_assign_TYPE and
// GrB_Vector_assign_TYPE, where TYPE is one of the 11 types, or the
// type-generic macro suffix, "_scalar".

#include "GB.h"

GrB_Info GB_assign_scalar           // C<Mask>(I,J) = accum (C(I,J),x)
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Matrix Mask,          // optional mask for C(I,J), unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C(I,J),T)
    const void *scalar,             // scalar to assign to C(I,J)
    const GB_Type_code scalar_code, // type code of scalar to assign
    const GrB_Index *I,             // row indices
    const GrB_Index ni,             // number of row indices
    const GrB_Index *J,             // column indices
    const GrB_Index nj,             // number of column indices
    const GrB_Descriptor desc       // descriptor for C(I,J) and Mask
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    RETURN_IF_NULL (scalar) ;
    ASSERT (scalar_code <= GB_UDT_code) ;

    // get the descriptor
    GET_DESCRIPTOR (info, desc, C_replace, Mask_comp, ignore1, ignore2) ;

    //--------------------------------------------------------------------------
    // C<Mask>(I,J) = accum (C(I,J), A) and variations
    //--------------------------------------------------------------------------

    return (GB_assign (
        C,          C_replace,      // C matrix and its descriptor
        Mask,       Mask_comp,      // Mask matrix and its descriptor
        accum,                      // for accum (C(I,J),A)
        NULL,       false,          // no explicit matrix
        I, ni,                      // row indices
        J, nj,                      // column indices
        true,                       // do scalar expansion
        scalar,                     // scalar to assign, expands to become A
        scalar_code,                // type code of scalar to expand
        false, false)) ;            // not GrB_Col_assign nor GrB_row_assign
}

