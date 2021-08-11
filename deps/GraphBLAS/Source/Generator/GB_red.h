// SPDX-License-Identifier: Apache-2.0
if_is_monoid

GrB_Info GB (_red_scalar)
(
    GB_atype *result,
    const GrB_Matrix A,
    GB_void *restrict W_space,
    bool *restrict F,
    int ntasks,
    int nthreads
) ;

endif_is_monoid

GrB_Info GB (_red_build)
(
    GB_atype *restrict Tx,
    int64_t  *restrict Ti,
    const GB_atype *restrict Sx,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *restrict I_work,
    const int64_t *restrict K_work,
    const int64_t *restrict tstart_slice,
    const int64_t *restrict tnz_slice,
    int nthreads
) ;

