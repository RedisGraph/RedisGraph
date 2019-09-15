//------------------------------------------------------------------------------
// SuiteSparse/GraphBLAS/Source/all_user_objects.c
//------------------------------------------------------------------------------

// This file is constructed automatically by cmake and m4 when GraphBLAS is
// compiled, from the Config/user_def*.m4 and *.m4 files in User/.  Do not edit
// this file directly.  It contains references to internally-defined functions
// and objects inside GraphBLAS, which are not user-callable.

#include "GB_mxm.h"
#include "GB_user.h"

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
    const GrB_Matrix GB_A,          // not used for dot2 method
    const GrB_Matrix GB_B,
    bool GB_flipxy,

    // for heap method only:
    int64_t *restrict GB_List,
    GB_pointer_pair *restrict GB_pA_pair,
    GB_Element *restrict GB_Heap,
    const int64_t GB_bjnz_max,

    // for Gustavson's method only:
    GB_Sauna GB_C_Sauna,

    // for dot method only:
    const GrB_Matrix *GB_Aslice,    // for dot2 only
    const int GB_dot_nthreads,      // for dot2 and dot3
    const int GB_naslice,           // for dot2 only
    const int GB_nbslice,           // for dot2 only
    int64_t **GB_C_counts,          // for dot2 only

    // for dot3 method only:
    const GB_task_struct *restrict GB_TaskList,
    const int GB_ntasks
)
{
    GrB_Info GB_info = GrB_SUCCESS ;
    if (0)
    {
        ;
    }
    return (GB_info) ;
}

