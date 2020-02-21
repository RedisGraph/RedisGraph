
GrB_Info GB_Cdense_05d
(
    GrB_Matrix C,
    const GrB_Matrix M,
    const bool Mask_struct,
    const GB_void *p_cwork,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const int ntasks,
    const int nthreads
) ;

GrB_Info GB_Cdense_06d
(
    GrB_Matrix C,
    const GrB_Matrix A,
    const bool Mask_struct,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const int ntasks,
    const int nthreads
) ;

GrB_Info GB_Cdense_25
(
    GrB_Matrix C,
    const GrB_Matrix M,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const int ntasks,
    const int nthreads
) ;

