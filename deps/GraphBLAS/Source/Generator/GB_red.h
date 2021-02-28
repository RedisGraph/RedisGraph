if_is_monoid

GrB_Info GB_red_scalar
(
    GB_atype *result,
    const GrB_Matrix A,
    GB_void *GB_RESTRICT W_space,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachvec
(
    GB_atype *GB_RESTRICT Tx,
    GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    GB_void *Wfirst_space,
    GB_void *Wlast_space,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachindex
(
    GrB_Matrix *Thandle,
    GrB_Type ttype,
    GrB_Matrix A,
    const int64_t *GB_RESTRICT pstart_slice,
    int nth,
    int nthreads,
    GB_Context Context
) ;

endif_is_monoid

GrB_Info GB_red_build
(
    GB_atype *GB_RESTRICT Tx,
    int64_t  *GB_RESTRICT Ti,
    const GB_atype *GB_RESTRICT S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *GB_RESTRICT I_work,
    const int64_t *GB_RESTRICT K_work,
    const int64_t *GB_RESTRICT tstart_slice,
    const int64_t *GB_RESTRICT tnz_slice,
    int nthreads
) ;

