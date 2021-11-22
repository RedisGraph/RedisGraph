// SPDX-License-Identifier: Apache-2.0
if_phase1
void GB (_sel_phase1)
(
    int64_t *restrict Zp,
    int64_t *restrict Cp,
    int64_t *restrict Wfirst,
    int64_t *restrict Wlast,
    const GrB_Matrix A,
    const bool flipij,
    const int64_t ithunk,
    const GB_atype *restrict athunk,
    const GB_void *restrict ythunk,
    const GB_Operator op,
    const int64_t *A_ek_slicing, const int A_ntasks, const int A_nthreads
) ;
endif_phase1

if_phase2
void GB (_sel_phase2)
(
    int64_t *restrict Ci,
    GB_atype *restrict Cx,
    const int64_t *restrict Zp,
    const int64_t *restrict Cp,
    const int64_t *restrict Cp_kfirst,
    const GrB_Matrix A,
    const bool flipij,
    const int64_t ithunk,
    const GB_atype *restrict athunk,
    const GB_void *restrict ythunk,
    const GB_Operator op,
    const int64_t *A_ek_slicing, const int A_ntasks, const int A_nthreads
) ;
endif_phase2

if_bitmap
void GB (_sel_bitmap)
(
    int8_t *Cb,
    GB_atype *restrict Cx,
    int64_t *cnvals_handle,
    GrB_Matrix A,
    const bool flipij,
    const int64_t ithunk,
    const GB_atype *restrict athunk,
    const GB_void *restrict ythunk,
    const GB_Operator op,
    const int nthreads
) ;
endif_bitmap
