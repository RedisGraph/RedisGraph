//------------------------------------------------------------------------------
// SuiteSparse/GraphBLAS/Config/user_def2.m4: code to call user semirings
//------------------------------------------------------------------------------

GrB_Info GB_AxB_user
(
    const GrB_Desc_Value GB_AxB_method,
    const GrB_Semiring GB_s,

    GrB_Matrix *GB_Chandle,
    const GrB_Matrix GB_M,
    const GrB_Matrix GB_A,     // not used for dot method
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
    const GrB_Matrix *GB_Aslice,
    const bool GB_mask_comp,
    const int GB_dot_nthreads,
    const int GB_naslice,
    const int GB_nbslice,
    int64_t **GB_C_counts
)
{
    GrB_Info GB_info = GrB_SUCCESS ;
    GB_semirings()
    return (GB_info) ;
}

