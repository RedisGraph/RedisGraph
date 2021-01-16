// SPDX-License-Identifier: Apache-2.0
if_is_monoid

GrB_Info GB_red_scalar
(
    GB_atype *result,
    const GrB_Matrix A,
    GB_void *GB_RESTRICT W_space,
    bool *GB_RESTRICT F,
    int ntasks,
    int nthreads
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

