//------------------------------------------------------------------------------
// SuiteSparse/GraphBLAS/Source/all_user_objects.c
//------------------------------------------------------------------------------

// This file is constructed automatically by cmake and m4 when GraphBLAS is
// compiled, from the Config/user_def*.m4 and User/*.m4 files.  Do not edit
// this file directly.  It contains references to internally-defined functions
// and objects inside GraphBLAS, which are not user-callable.

#include "GB.h"

//------------------------------------------------------------------------------
// SuiteSparse/GraphBLAS/Config/user_def1.m4: define user-defined objects
//------------------------------------------------------------------------------
















//------------------------------------------------------------------------------
// SuiteSparse/GraphBLAS/Config/user_def2.m4: code to call user semirings
//------------------------------------------------------------------------------

GrB_Info GB_AxB_user
(
    const GrB_Desc_Value GB_AxB_method,
    const GrB_Semiring GB_s,

    GrB_Matrix *GB_Chandle,
    const GrB_Matrix GB_M,
    const GrB_Matrix GB_A,
    const GrB_Matrix GB_B,
    bool GB_flipxy,                // if true, A and B have been swapped

    // for heap method only:
    int64_t *restrict GB_List,
    GB_pointer_pair *restrict GB_pA_pair,
    GB_Element *restrict GB_Heap,
    const int64_t GB_bjnz_max,

    // for Gustavson method only:
    GB_Sauna GB_C_Sauna,

    GB_Context Context
)
{
    GrB_Info GB_info = GrB_INVALID_OBJECT ;

    if (0)
    {
        ;
    }

    if (GB_info == GrB_INVALID_OBJECT)
    {
        return (GB_ERROR (GrB_INVALID_OBJECT, (GB_LOG, "undefined semiring"))) ;
    }
    return (GB_info) ;
}

