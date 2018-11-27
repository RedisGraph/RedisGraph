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

    GB_semirings()

    if (GB_info == GrB_INVALID_OBJECT)
    {
        return (GB_ERROR (GrB_INVALID_OBJECT, (GB_LOG, "undefined semiring"))) ;
    }
    return (GB_info) ;
}

