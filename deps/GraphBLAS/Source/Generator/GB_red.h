if_is_monoid

GrB_Info GB_red_scalar
(
    GB_atype *result,
    const GrB_Matrix A,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachvec
(
    GB_atype *restrict Tx,
    GrB_Matrix A,
    const int64_t *restrict kfirst_slice,
    const int64_t *restrict klast_slice,
    const int64_t *restrict pstart_slice,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachindex
(
    GrB_Matrix *Thandle,
    GrB_Type ttype,
    GrB_Matrix A,
    const int64_t *restrict pstart_slice,
    int nth,
    int nthreads,
    GB_Context Context
) ;

endif_is_monoid

GrB_Info GB_red_build
(
    GB_atype *restrict Tx,
    int64_t  *restrict Ti,
    const GB_atype *restrict S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *restrict I_work,
    const int64_t *restrict K_work,
    const int64_t *restrict tstart_slice,
    const int64_t *restrict tnz_slice,
    int nthreads
) ;

