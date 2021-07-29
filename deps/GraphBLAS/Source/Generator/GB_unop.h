// SPDX-License-Identifier: Apache-2.0
if_unop_apply_enabled
GrB_Info GB (_unop_apply)
(
    GB_ctype *Cx,
    const GB_atype *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;
endif_unop_apply_enabled

GrB_Info GB (_unop_tran)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

