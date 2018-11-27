GrB_Info GB_AgusB
(
    GrB_Matrix C,
    const GrB_Matrix M,
    const GrB_Matrix A,
    const GrB_Matrix B,
    bool flipxy,
    GB_Sauna Sauna,
    GB_Context Context
) ;

GrB_Info GB_AdotB
(
    GrB_Matrix *Chandle,
    const GrB_Matrix M,
    const GrB_Matrix A,
    const GrB_Matrix B,
    bool flipxy,
    GB_Context Context
) ;

GrB_Info GB_AheapB
(
    GrB_Matrix *Chandle,
    const GrB_Matrix M,
    const GrB_Matrix A,
    const GrB_Matrix B,
    bool flipxy,
    int64_t *restrict List,
    GB_pointer_pair *restrict pA_pair,
    GB_Element *restrict Heap,
    const int64_t bjnz_max,
    GB_Context Context
) ;


