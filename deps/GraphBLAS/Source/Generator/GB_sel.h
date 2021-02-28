if_phase1

void GB_sel_phase1
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const GB_atype *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;

endif_phase1

void GB_sel_phase2
(
    int64_t *GB_RESTRICT Ci,
    GB_atype *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const GB_atype *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;

