//------------------------------------------------------------------------------
// GB_unop__include.h: definitions for GB_unop__*.c
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

// This file has been automatically generated from Generator/GB_unop.h

// SPDX-License-Identifier: Apache-2.0
#if 0
GrB_Info GB (_unop_apply__(none))
(
    bool *Cx,
    const bool *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;
#endif

GrB_Info GB (_unop_tran__identity_bool_bool)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_bool_int8)
(
    bool *Cx,
    const int8_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_bool_int8)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_bool_int16)
(
    bool *Cx,
    const int16_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_bool_int16)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_bool_int32)
(
    bool *Cx,
    const int32_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_bool_int32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_bool_int64)
(
    bool *Cx,
    const int64_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_bool_int64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_bool_uint8)
(
    bool *Cx,
    const uint8_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_bool_uint8)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_bool_uint16)
(
    bool *Cx,
    const uint16_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_bool_uint16)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_bool_uint32)
(
    bool *Cx,
    const uint32_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_bool_uint32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_bool_uint64)
(
    bool *Cx,
    const uint64_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_bool_uint64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_bool_fp32)
(
    bool *Cx,
    const float *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_bool_fp32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_bool_fp64)
(
    bool *Cx,
    const double *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_bool_fp64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_bool_fc32)
(
    bool *Cx,
    const GxB_FC32_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_bool_fc32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_bool_fc64)
(
    bool *Cx,
    const GxB_FC64_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_bool_fc64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_int8_bool)
(
    int8_t *Cx,
    const bool *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_int8_bool)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0
#if 0
GrB_Info GB (_unop_apply__(none))
(
    int8_t *Cx,
    const int8_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;
#endif

GrB_Info GB (_unop_tran__identity_int8_int8)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_int8_int16)
(
    int8_t *Cx,
    const int16_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_int8_int16)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_int8_int32)
(
    int8_t *Cx,
    const int32_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_int8_int32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_int8_int64)
(
    int8_t *Cx,
    const int64_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_int8_int64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_int8_uint8)
(
    int8_t *Cx,
    const uint8_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_int8_uint8)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_int8_uint16)
(
    int8_t *Cx,
    const uint16_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_int8_uint16)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_int8_uint32)
(
    int8_t *Cx,
    const uint32_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_int8_uint32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_int8_uint64)
(
    int8_t *Cx,
    const uint64_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_int8_uint64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_int8_fp32)
(
    int8_t *Cx,
    const float *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_int8_fp32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_int8_fp64)
(
    int8_t *Cx,
    const double *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_int8_fp64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_int8_fc32)
(
    int8_t *Cx,
    const GxB_FC32_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_int8_fc32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_int8_fc64)
(
    int8_t *Cx,
    const GxB_FC64_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_int8_fc64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_int16_bool)
(
    int16_t *Cx,
    const bool *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_int16_bool)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_int16_int8)
(
    int16_t *Cx,
    const int8_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_int16_int8)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0
#if 0
GrB_Info GB (_unop_apply__(none))
(
    int16_t *Cx,
    const int16_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;
#endif

GrB_Info GB (_unop_tran__identity_int16_int16)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_int16_int32)
(
    int16_t *Cx,
    const int32_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_int16_int32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_int16_int64)
(
    int16_t *Cx,
    const int64_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_int16_int64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_int16_uint8)
(
    int16_t *Cx,
    const uint8_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_int16_uint8)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_int16_uint16)
(
    int16_t *Cx,
    const uint16_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_int16_uint16)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_int16_uint32)
(
    int16_t *Cx,
    const uint32_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_int16_uint32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_int16_uint64)
(
    int16_t *Cx,
    const uint64_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_int16_uint64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_int16_fp32)
(
    int16_t *Cx,
    const float *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_int16_fp32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_int16_fp64)
(
    int16_t *Cx,
    const double *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_int16_fp64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_int16_fc32)
(
    int16_t *Cx,
    const GxB_FC32_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_int16_fc32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_int16_fc64)
(
    int16_t *Cx,
    const GxB_FC64_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_int16_fc64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_int32_bool)
(
    int32_t *Cx,
    const bool *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_int32_bool)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_int32_int8)
(
    int32_t *Cx,
    const int8_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_int32_int8)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_int32_int16)
(
    int32_t *Cx,
    const int16_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_int32_int16)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0
#if 0
GrB_Info GB (_unop_apply__(none))
(
    int32_t *Cx,
    const int32_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;
#endif

GrB_Info GB (_unop_tran__identity_int32_int32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_int32_int64)
(
    int32_t *Cx,
    const int64_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_int32_int64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_int32_uint8)
(
    int32_t *Cx,
    const uint8_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_int32_uint8)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_int32_uint16)
(
    int32_t *Cx,
    const uint16_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_int32_uint16)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_int32_uint32)
(
    int32_t *Cx,
    const uint32_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_int32_uint32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_int32_uint64)
(
    int32_t *Cx,
    const uint64_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_int32_uint64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_int32_fp32)
(
    int32_t *Cx,
    const float *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_int32_fp32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_int32_fp64)
(
    int32_t *Cx,
    const double *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_int32_fp64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_int32_fc32)
(
    int32_t *Cx,
    const GxB_FC32_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_int32_fc32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_int32_fc64)
(
    int32_t *Cx,
    const GxB_FC64_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_int32_fc64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_int64_bool)
(
    int64_t *Cx,
    const bool *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_int64_bool)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_int64_int8)
(
    int64_t *Cx,
    const int8_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_int64_int8)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_int64_int16)
(
    int64_t *Cx,
    const int16_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_int64_int16)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_int64_int32)
(
    int64_t *Cx,
    const int32_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_int64_int32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0
#if 0
GrB_Info GB (_unop_apply__(none))
(
    int64_t *Cx,
    const int64_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;
#endif

GrB_Info GB (_unop_tran__identity_int64_int64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_int64_uint8)
(
    int64_t *Cx,
    const uint8_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_int64_uint8)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_int64_uint16)
(
    int64_t *Cx,
    const uint16_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_int64_uint16)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_int64_uint32)
(
    int64_t *Cx,
    const uint32_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_int64_uint32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_int64_uint64)
(
    int64_t *Cx,
    const uint64_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_int64_uint64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_int64_fp32)
(
    int64_t *Cx,
    const float *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_int64_fp32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_int64_fp64)
(
    int64_t *Cx,
    const double *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_int64_fp64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_int64_fc32)
(
    int64_t *Cx,
    const GxB_FC32_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_int64_fc32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_int64_fc64)
(
    int64_t *Cx,
    const GxB_FC64_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_int64_fc64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_uint8_bool)
(
    uint8_t *Cx,
    const bool *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_uint8_bool)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_uint8_int8)
(
    uint8_t *Cx,
    const int8_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_uint8_int8)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_uint8_int16)
(
    uint8_t *Cx,
    const int16_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_uint8_int16)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_uint8_int32)
(
    uint8_t *Cx,
    const int32_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_uint8_int32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_uint8_int64)
(
    uint8_t *Cx,
    const int64_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_uint8_int64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0
#if 0
GrB_Info GB (_unop_apply__(none))
(
    uint8_t *Cx,
    const uint8_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;
#endif

GrB_Info GB (_unop_tran__identity_uint8_uint8)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_uint8_uint16)
(
    uint8_t *Cx,
    const uint16_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_uint8_uint16)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_uint8_uint32)
(
    uint8_t *Cx,
    const uint32_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_uint8_uint32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_uint8_uint64)
(
    uint8_t *Cx,
    const uint64_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_uint8_uint64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_uint8_fp32)
(
    uint8_t *Cx,
    const float *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_uint8_fp32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_uint8_fp64)
(
    uint8_t *Cx,
    const double *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_uint8_fp64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_uint8_fc32)
(
    uint8_t *Cx,
    const GxB_FC32_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_uint8_fc32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_uint8_fc64)
(
    uint8_t *Cx,
    const GxB_FC64_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_uint8_fc64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_uint16_bool)
(
    uint16_t *Cx,
    const bool *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_uint16_bool)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_uint16_int8)
(
    uint16_t *Cx,
    const int8_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_uint16_int8)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_uint16_int16)
(
    uint16_t *Cx,
    const int16_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_uint16_int16)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_uint16_int32)
(
    uint16_t *Cx,
    const int32_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_uint16_int32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_uint16_int64)
(
    uint16_t *Cx,
    const int64_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_uint16_int64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_uint16_uint8)
(
    uint16_t *Cx,
    const uint8_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_uint16_uint8)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0
#if 0
GrB_Info GB (_unop_apply__(none))
(
    uint16_t *Cx,
    const uint16_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;
#endif

GrB_Info GB (_unop_tran__identity_uint16_uint16)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_uint16_uint32)
(
    uint16_t *Cx,
    const uint32_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_uint16_uint32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_uint16_uint64)
(
    uint16_t *Cx,
    const uint64_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_uint16_uint64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_uint16_fp32)
(
    uint16_t *Cx,
    const float *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_uint16_fp32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_uint16_fp64)
(
    uint16_t *Cx,
    const double *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_uint16_fp64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_uint16_fc32)
(
    uint16_t *Cx,
    const GxB_FC32_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_uint16_fc32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_uint16_fc64)
(
    uint16_t *Cx,
    const GxB_FC64_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_uint16_fc64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_uint32_bool)
(
    uint32_t *Cx,
    const bool *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_uint32_bool)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_uint32_int8)
(
    uint32_t *Cx,
    const int8_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_uint32_int8)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_uint32_int16)
(
    uint32_t *Cx,
    const int16_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_uint32_int16)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_uint32_int32)
(
    uint32_t *Cx,
    const int32_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_uint32_int32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_uint32_int64)
(
    uint32_t *Cx,
    const int64_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_uint32_int64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_uint32_uint8)
(
    uint32_t *Cx,
    const uint8_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_uint32_uint8)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_uint32_uint16)
(
    uint32_t *Cx,
    const uint16_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_uint32_uint16)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0
#if 0
GrB_Info GB (_unop_apply__(none))
(
    uint32_t *Cx,
    const uint32_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;
#endif

GrB_Info GB (_unop_tran__identity_uint32_uint32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_uint32_uint64)
(
    uint32_t *Cx,
    const uint64_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_uint32_uint64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_uint32_fp32)
(
    uint32_t *Cx,
    const float *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_uint32_fp32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_uint32_fp64)
(
    uint32_t *Cx,
    const double *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_uint32_fp64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_uint32_fc32)
(
    uint32_t *Cx,
    const GxB_FC32_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_uint32_fc32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_uint32_fc64)
(
    uint32_t *Cx,
    const GxB_FC64_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_uint32_fc64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_uint64_bool)
(
    uint64_t *Cx,
    const bool *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_uint64_bool)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_uint64_int8)
(
    uint64_t *Cx,
    const int8_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_uint64_int8)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_uint64_int16)
(
    uint64_t *Cx,
    const int16_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_uint64_int16)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_uint64_int32)
(
    uint64_t *Cx,
    const int32_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_uint64_int32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_uint64_int64)
(
    uint64_t *Cx,
    const int64_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_uint64_int64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_uint64_uint8)
(
    uint64_t *Cx,
    const uint8_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_uint64_uint8)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_uint64_uint16)
(
    uint64_t *Cx,
    const uint16_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_uint64_uint16)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_uint64_uint32)
(
    uint64_t *Cx,
    const uint32_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_uint64_uint32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0
#if 0
GrB_Info GB (_unop_apply__(none))
(
    uint64_t *Cx,
    const uint64_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;
#endif

GrB_Info GB (_unop_tran__identity_uint64_uint64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_uint64_fp32)
(
    uint64_t *Cx,
    const float *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_uint64_fp32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_uint64_fp64)
(
    uint64_t *Cx,
    const double *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_uint64_fp64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_uint64_fc32)
(
    uint64_t *Cx,
    const GxB_FC32_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_uint64_fc32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_uint64_fc64)
(
    uint64_t *Cx,
    const GxB_FC64_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_uint64_fc64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_fp32_bool)
(
    float *Cx,
    const bool *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_fp32_bool)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_fp32_int8)
(
    float *Cx,
    const int8_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_fp32_int8)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_fp32_int16)
(
    float *Cx,
    const int16_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_fp32_int16)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_fp32_int32)
(
    float *Cx,
    const int32_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_fp32_int32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_fp32_int64)
(
    float *Cx,
    const int64_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_fp32_int64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_fp32_uint8)
(
    float *Cx,
    const uint8_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_fp32_uint8)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_fp32_uint16)
(
    float *Cx,
    const uint16_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_fp32_uint16)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_fp32_uint32)
(
    float *Cx,
    const uint32_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_fp32_uint32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_fp32_uint64)
(
    float *Cx,
    const uint64_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_fp32_uint64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0
#if 0
GrB_Info GB (_unop_apply__(none))
(
    float *Cx,
    const float *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;
#endif

GrB_Info GB (_unop_tran__identity_fp32_fp32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_fp32_fp64)
(
    float *Cx,
    const double *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_fp32_fp64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_fp32_fc32)
(
    float *Cx,
    const GxB_FC32_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_fp32_fc32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_fp32_fc64)
(
    float *Cx,
    const GxB_FC64_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_fp32_fc64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_fp64_bool)
(
    double *Cx,
    const bool *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_fp64_bool)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_fp64_int8)
(
    double *Cx,
    const int8_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_fp64_int8)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_fp64_int16)
(
    double *Cx,
    const int16_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_fp64_int16)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_fp64_int32)
(
    double *Cx,
    const int32_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_fp64_int32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_fp64_int64)
(
    double *Cx,
    const int64_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_fp64_int64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_fp64_uint8)
(
    double *Cx,
    const uint8_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_fp64_uint8)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_fp64_uint16)
(
    double *Cx,
    const uint16_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_fp64_uint16)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_fp64_uint32)
(
    double *Cx,
    const uint32_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_fp64_uint32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_fp64_uint64)
(
    double *Cx,
    const uint64_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_fp64_uint64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_fp64_fp32)
(
    double *Cx,
    const float *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_fp64_fp32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0
#if 0
GrB_Info GB (_unop_apply__(none))
(
    double *Cx,
    const double *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;
#endif

GrB_Info GB (_unop_tran__identity_fp64_fp64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_fp64_fc32)
(
    double *Cx,
    const GxB_FC32_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_fp64_fc32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_fp64_fc64)
(
    double *Cx,
    const GxB_FC64_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_fp64_fc64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_fc32_bool)
(
    GxB_FC32_t *Cx,
    const bool *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_fc32_bool)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_fc32_int8)
(
    GxB_FC32_t *Cx,
    const int8_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_fc32_int8)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_fc32_int16)
(
    GxB_FC32_t *Cx,
    const int16_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_fc32_int16)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_fc32_int32)
(
    GxB_FC32_t *Cx,
    const int32_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_fc32_int32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_fc32_int64)
(
    GxB_FC32_t *Cx,
    const int64_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_fc32_int64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_fc32_uint8)
(
    GxB_FC32_t *Cx,
    const uint8_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_fc32_uint8)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_fc32_uint16)
(
    GxB_FC32_t *Cx,
    const uint16_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_fc32_uint16)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_fc32_uint32)
(
    GxB_FC32_t *Cx,
    const uint32_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_fc32_uint32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_fc32_uint64)
(
    GxB_FC32_t *Cx,
    const uint64_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_fc32_uint64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_fc32_fp32)
(
    GxB_FC32_t *Cx,
    const float *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_fc32_fp32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_fc32_fp64)
(
    GxB_FC32_t *Cx,
    const double *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_fc32_fp64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0
#if 0
GrB_Info GB (_unop_apply__(none))
(
    GxB_FC32_t *Cx,
    const GxB_FC32_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;
#endif

GrB_Info GB (_unop_tran__identity_fc32_fc32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_fc32_fc64)
(
    GxB_FC32_t *Cx,
    const GxB_FC64_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_fc32_fc64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_fc64_bool)
(
    GxB_FC64_t *Cx,
    const bool *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_fc64_bool)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_fc64_int8)
(
    GxB_FC64_t *Cx,
    const int8_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_fc64_int8)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_fc64_int16)
(
    GxB_FC64_t *Cx,
    const int16_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_fc64_int16)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_fc64_int32)
(
    GxB_FC64_t *Cx,
    const int32_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_fc64_int32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_fc64_int64)
(
    GxB_FC64_t *Cx,
    const int64_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_fc64_int64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_fc64_uint8)
(
    GxB_FC64_t *Cx,
    const uint8_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_fc64_uint8)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_fc64_uint16)
(
    GxB_FC64_t *Cx,
    const uint16_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_fc64_uint16)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_fc64_uint32)
(
    GxB_FC64_t *Cx,
    const uint32_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_fc64_uint32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_fc64_uint64)
(
    GxB_FC64_t *Cx,
    const uint64_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_fc64_uint64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_fc64_fp32)
(
    GxB_FC64_t *Cx,
    const float *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_fc64_fp32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_fc64_fp64)
(
    GxB_FC64_t *Cx,
    const double *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_fc64_fp64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__identity_fc64_fc32)
(
    GxB_FC64_t *Cx,
    const GxB_FC32_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__identity_fc64_fc32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0
#if 0
GrB_Info GB (_unop_apply__(none))
(
    GxB_FC64_t *Cx,
    const GxB_FC64_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;
#endif

GrB_Info GB (_unop_tran__identity_fc64_fc64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__ainv_bool_bool)
(
    bool *Cx,
    const bool *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__ainv_bool_bool)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__ainv_int8_int8)
(
    int8_t *Cx,
    const int8_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__ainv_int8_int8)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__ainv_int16_int16)
(
    int16_t *Cx,
    const int16_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__ainv_int16_int16)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__ainv_int32_int32)
(
    int32_t *Cx,
    const int32_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__ainv_int32_int32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__ainv_int64_int64)
(
    int64_t *Cx,
    const int64_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__ainv_int64_int64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__ainv_uint8_uint8)
(
    uint8_t *Cx,
    const uint8_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__ainv_uint8_uint8)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__ainv_uint16_uint16)
(
    uint16_t *Cx,
    const uint16_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__ainv_uint16_uint16)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__ainv_uint32_uint32)
(
    uint32_t *Cx,
    const uint32_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__ainv_uint32_uint32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__ainv_uint64_uint64)
(
    uint64_t *Cx,
    const uint64_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__ainv_uint64_uint64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__ainv_fp32_fp32)
(
    float *Cx,
    const float *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__ainv_fp32_fp32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__ainv_fp64_fp64)
(
    double *Cx,
    const double *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__ainv_fp64_fp64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__ainv_fc32_fc32)
(
    GxB_FC32_t *Cx,
    const GxB_FC32_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__ainv_fc32_fc32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__ainv_fc64_fc64)
(
    GxB_FC64_t *Cx,
    const GxB_FC64_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__ainv_fc64_fc64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__abs_bool_bool)
(
    bool *Cx,
    const bool *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__abs_bool_bool)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__abs_int8_int8)
(
    int8_t *Cx,
    const int8_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__abs_int8_int8)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__abs_int16_int16)
(
    int16_t *Cx,
    const int16_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__abs_int16_int16)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__abs_int32_int32)
(
    int32_t *Cx,
    const int32_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__abs_int32_int32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__abs_int64_int64)
(
    int64_t *Cx,
    const int64_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__abs_int64_int64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__abs_uint8_uint8)
(
    uint8_t *Cx,
    const uint8_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__abs_uint8_uint8)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__abs_uint16_uint16)
(
    uint16_t *Cx,
    const uint16_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__abs_uint16_uint16)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__abs_uint32_uint32)
(
    uint32_t *Cx,
    const uint32_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__abs_uint32_uint32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__abs_uint64_uint64)
(
    uint64_t *Cx,
    const uint64_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__abs_uint64_uint64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__abs_fp32_fp32)
(
    float *Cx,
    const float *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__abs_fp32_fp32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__abs_fp64_fp64)
(
    double *Cx,
    const double *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__abs_fp64_fp64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__minv_bool_bool)
(
    bool *Cx,
    const bool *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__minv_bool_bool)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__minv_int8_int8)
(
    int8_t *Cx,
    const int8_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__minv_int8_int8)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__minv_int16_int16)
(
    int16_t *Cx,
    const int16_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__minv_int16_int16)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__minv_int32_int32)
(
    int32_t *Cx,
    const int32_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__minv_int32_int32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__minv_int64_int64)
(
    int64_t *Cx,
    const int64_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__minv_int64_int64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__minv_uint8_uint8)
(
    uint8_t *Cx,
    const uint8_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__minv_uint8_uint8)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__minv_uint16_uint16)
(
    uint16_t *Cx,
    const uint16_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__minv_uint16_uint16)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__minv_uint32_uint32)
(
    uint32_t *Cx,
    const uint32_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__minv_uint32_uint32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__minv_uint64_uint64)
(
    uint64_t *Cx,
    const uint64_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__minv_uint64_uint64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__minv_fp32_fp32)
(
    float *Cx,
    const float *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__minv_fp32_fp32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__minv_fp64_fp64)
(
    double *Cx,
    const double *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__minv_fp64_fp64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__minv_fc32_fc32)
(
    GxB_FC32_t *Cx,
    const GxB_FC32_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__minv_fc32_fc32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__minv_fc64_fc64)
(
    GxB_FC64_t *Cx,
    const GxB_FC64_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__minv_fc64_fc64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__lnot_bool_bool)
(
    bool *Cx,
    const bool *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__lnot_bool_bool)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__lnot_int8_int8)
(
    int8_t *Cx,
    const int8_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__lnot_int8_int8)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__lnot_int16_int16)
(
    int16_t *Cx,
    const int16_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__lnot_int16_int16)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__lnot_int32_int32)
(
    int32_t *Cx,
    const int32_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__lnot_int32_int32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__lnot_int64_int64)
(
    int64_t *Cx,
    const int64_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__lnot_int64_int64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__lnot_uint8_uint8)
(
    uint8_t *Cx,
    const uint8_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__lnot_uint8_uint8)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__lnot_uint16_uint16)
(
    uint16_t *Cx,
    const uint16_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__lnot_uint16_uint16)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__lnot_uint32_uint32)
(
    uint32_t *Cx,
    const uint32_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__lnot_uint32_uint32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__lnot_uint64_uint64)
(
    uint64_t *Cx,
    const uint64_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__lnot_uint64_uint64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__lnot_fp32_fp32)
(
    float *Cx,
    const float *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__lnot_fp32_fp32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__lnot_fp64_fp64)
(
    double *Cx,
    const double *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__lnot_fp64_fp64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__bnot_int8_int8)
(
    int8_t *Cx,
    const int8_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__bnot_int8_int8)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__bnot_int16_int16)
(
    int16_t *Cx,
    const int16_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__bnot_int16_int16)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__bnot_int32_int32)
(
    int32_t *Cx,
    const int32_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__bnot_int32_int32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__bnot_int64_int64)
(
    int64_t *Cx,
    const int64_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__bnot_int64_int64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__bnot_uint8_uint8)
(
    uint8_t *Cx,
    const uint8_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__bnot_uint8_uint8)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__bnot_uint16_uint16)
(
    uint16_t *Cx,
    const uint16_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__bnot_uint16_uint16)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__bnot_uint32_uint32)
(
    uint32_t *Cx,
    const uint32_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__bnot_uint32_uint32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__bnot_uint64_uint64)
(
    uint64_t *Cx,
    const uint64_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__bnot_uint64_uint64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__sqrt_fp32_fp32)
(
    float *Cx,
    const float *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__sqrt_fp32_fp32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__sqrt_fp64_fp64)
(
    double *Cx,
    const double *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__sqrt_fp64_fp64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__sqrt_fc32_fc32)
(
    GxB_FC32_t *Cx,
    const GxB_FC32_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__sqrt_fc32_fc32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__sqrt_fc64_fc64)
(
    GxB_FC64_t *Cx,
    const GxB_FC64_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__sqrt_fc64_fc64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__log_fp32_fp32)
(
    float *Cx,
    const float *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__log_fp32_fp32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__log_fp64_fp64)
(
    double *Cx,
    const double *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__log_fp64_fp64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__log_fc32_fc32)
(
    GxB_FC32_t *Cx,
    const GxB_FC32_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__log_fc32_fc32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__log_fc64_fc64)
(
    GxB_FC64_t *Cx,
    const GxB_FC64_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__log_fc64_fc64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__exp_fp32_fp32)
(
    float *Cx,
    const float *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__exp_fp32_fp32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__exp_fp64_fp64)
(
    double *Cx,
    const double *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__exp_fp64_fp64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__exp_fc32_fc32)
(
    GxB_FC32_t *Cx,
    const GxB_FC32_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__exp_fc32_fc32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__exp_fc64_fc64)
(
    GxB_FC64_t *Cx,
    const GxB_FC64_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__exp_fc64_fc64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__sin_fp32_fp32)
(
    float *Cx,
    const float *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__sin_fp32_fp32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__sin_fp64_fp64)
(
    double *Cx,
    const double *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__sin_fp64_fp64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__sin_fc32_fc32)
(
    GxB_FC32_t *Cx,
    const GxB_FC32_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__sin_fc32_fc32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__sin_fc64_fc64)
(
    GxB_FC64_t *Cx,
    const GxB_FC64_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__sin_fc64_fc64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__cos_fp32_fp32)
(
    float *Cx,
    const float *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__cos_fp32_fp32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__cos_fp64_fp64)
(
    double *Cx,
    const double *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__cos_fp64_fp64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__cos_fc32_fc32)
(
    GxB_FC32_t *Cx,
    const GxB_FC32_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__cos_fc32_fc32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__cos_fc64_fc64)
(
    GxB_FC64_t *Cx,
    const GxB_FC64_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__cos_fc64_fc64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__tan_fp32_fp32)
(
    float *Cx,
    const float *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__tan_fp32_fp32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__tan_fp64_fp64)
(
    double *Cx,
    const double *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__tan_fp64_fp64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__tan_fc32_fc32)
(
    GxB_FC32_t *Cx,
    const GxB_FC32_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__tan_fc32_fc32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__tan_fc64_fc64)
(
    GxB_FC64_t *Cx,
    const GxB_FC64_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__tan_fc64_fc64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__asin_fp32_fp32)
(
    float *Cx,
    const float *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__asin_fp32_fp32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__asin_fp64_fp64)
(
    double *Cx,
    const double *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__asin_fp64_fp64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__asin_fc32_fc32)
(
    GxB_FC32_t *Cx,
    const GxB_FC32_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__asin_fc32_fc32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__asin_fc64_fc64)
(
    GxB_FC64_t *Cx,
    const GxB_FC64_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__asin_fc64_fc64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__acos_fp32_fp32)
(
    float *Cx,
    const float *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__acos_fp32_fp32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__acos_fp64_fp64)
(
    double *Cx,
    const double *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__acos_fp64_fp64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__acos_fc32_fc32)
(
    GxB_FC32_t *Cx,
    const GxB_FC32_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__acos_fc32_fc32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__acos_fc64_fc64)
(
    GxB_FC64_t *Cx,
    const GxB_FC64_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__acos_fc64_fc64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__atan_fp32_fp32)
(
    float *Cx,
    const float *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__atan_fp32_fp32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__atan_fp64_fp64)
(
    double *Cx,
    const double *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__atan_fp64_fp64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__atan_fc32_fc32)
(
    GxB_FC32_t *Cx,
    const GxB_FC32_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__atan_fc32_fc32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__atan_fc64_fc64)
(
    GxB_FC64_t *Cx,
    const GxB_FC64_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__atan_fc64_fc64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__sinh_fp32_fp32)
(
    float *Cx,
    const float *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__sinh_fp32_fp32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__sinh_fp64_fp64)
(
    double *Cx,
    const double *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__sinh_fp64_fp64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__sinh_fc32_fc32)
(
    GxB_FC32_t *Cx,
    const GxB_FC32_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__sinh_fc32_fc32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__sinh_fc64_fc64)
(
    GxB_FC64_t *Cx,
    const GxB_FC64_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__sinh_fc64_fc64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__cosh_fp32_fp32)
(
    float *Cx,
    const float *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__cosh_fp32_fp32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__cosh_fp64_fp64)
(
    double *Cx,
    const double *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__cosh_fp64_fp64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__cosh_fc32_fc32)
(
    GxB_FC32_t *Cx,
    const GxB_FC32_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__cosh_fc32_fc32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__cosh_fc64_fc64)
(
    GxB_FC64_t *Cx,
    const GxB_FC64_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__cosh_fc64_fc64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__tanh_fp32_fp32)
(
    float *Cx,
    const float *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__tanh_fp32_fp32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__tanh_fp64_fp64)
(
    double *Cx,
    const double *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__tanh_fp64_fp64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__tanh_fc32_fc32)
(
    GxB_FC32_t *Cx,
    const GxB_FC32_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__tanh_fc32_fc32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__tanh_fc64_fc64)
(
    GxB_FC64_t *Cx,
    const GxB_FC64_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__tanh_fc64_fc64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__asinh_fp32_fp32)
(
    float *Cx,
    const float *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__asinh_fp32_fp32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__asinh_fp64_fp64)
(
    double *Cx,
    const double *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__asinh_fp64_fp64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__asinh_fc32_fc32)
(
    GxB_FC32_t *Cx,
    const GxB_FC32_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__asinh_fc32_fc32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__asinh_fc64_fc64)
(
    GxB_FC64_t *Cx,
    const GxB_FC64_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__asinh_fc64_fc64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__acosh_fp32_fp32)
(
    float *Cx,
    const float *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__acosh_fp32_fp32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__acosh_fp64_fp64)
(
    double *Cx,
    const double *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__acosh_fp64_fp64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__acosh_fc32_fc32)
(
    GxB_FC32_t *Cx,
    const GxB_FC32_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__acosh_fc32_fc32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__acosh_fc64_fc64)
(
    GxB_FC64_t *Cx,
    const GxB_FC64_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__acosh_fc64_fc64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__atanh_fp32_fp32)
(
    float *Cx,
    const float *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__atanh_fp32_fp32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__atanh_fp64_fp64)
(
    double *Cx,
    const double *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__atanh_fp64_fp64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__atanh_fc32_fc32)
(
    GxB_FC32_t *Cx,
    const GxB_FC32_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__atanh_fc32_fc32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__atanh_fc64_fc64)
(
    GxB_FC64_t *Cx,
    const GxB_FC64_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__atanh_fc64_fc64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__signum_fp32_fp32)
(
    float *Cx,
    const float *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__signum_fp32_fp32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__signum_fp64_fp64)
(
    double *Cx,
    const double *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__signum_fp64_fp64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__signum_fc32_fc32)
(
    GxB_FC32_t *Cx,
    const GxB_FC32_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__signum_fc32_fc32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__signum_fc64_fc64)
(
    GxB_FC64_t *Cx,
    const GxB_FC64_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__signum_fc64_fc64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__ceil_fp32_fp32)
(
    float *Cx,
    const float *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__ceil_fp32_fp32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__ceil_fp64_fp64)
(
    double *Cx,
    const double *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__ceil_fp64_fp64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__ceil_fc32_fc32)
(
    GxB_FC32_t *Cx,
    const GxB_FC32_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__ceil_fc32_fc32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__ceil_fc64_fc64)
(
    GxB_FC64_t *Cx,
    const GxB_FC64_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__ceil_fc64_fc64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__floor_fp32_fp32)
(
    float *Cx,
    const float *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__floor_fp32_fp32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__floor_fp64_fp64)
(
    double *Cx,
    const double *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__floor_fp64_fp64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__floor_fc32_fc32)
(
    GxB_FC32_t *Cx,
    const GxB_FC32_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__floor_fc32_fc32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__floor_fc64_fc64)
(
    GxB_FC64_t *Cx,
    const GxB_FC64_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__floor_fc64_fc64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__round_fp32_fp32)
(
    float *Cx,
    const float *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__round_fp32_fp32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__round_fp64_fp64)
(
    double *Cx,
    const double *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__round_fp64_fp64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__round_fc32_fc32)
(
    GxB_FC32_t *Cx,
    const GxB_FC32_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__round_fc32_fc32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__round_fc64_fc64)
(
    GxB_FC64_t *Cx,
    const GxB_FC64_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__round_fc64_fc64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__trunc_fp32_fp32)
(
    float *Cx,
    const float *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__trunc_fp32_fp32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__trunc_fp64_fp64)
(
    double *Cx,
    const double *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__trunc_fp64_fp64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__trunc_fc32_fc32)
(
    GxB_FC32_t *Cx,
    const GxB_FC32_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__trunc_fc32_fc32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__trunc_fc64_fc64)
(
    GxB_FC64_t *Cx,
    const GxB_FC64_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__trunc_fc64_fc64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__exp2_fp32_fp32)
(
    float *Cx,
    const float *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__exp2_fp32_fp32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__exp2_fp64_fp64)
(
    double *Cx,
    const double *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__exp2_fp64_fp64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__exp2_fc32_fc32)
(
    GxB_FC32_t *Cx,
    const GxB_FC32_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__exp2_fc32_fc32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__exp2_fc64_fc64)
(
    GxB_FC64_t *Cx,
    const GxB_FC64_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__exp2_fc64_fc64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__expm1_fp32_fp32)
(
    float *Cx,
    const float *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__expm1_fp32_fp32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__expm1_fp64_fp64)
(
    double *Cx,
    const double *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__expm1_fp64_fp64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__expm1_fc32_fc32)
(
    GxB_FC32_t *Cx,
    const GxB_FC32_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__expm1_fc32_fc32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__expm1_fc64_fc64)
(
    GxB_FC64_t *Cx,
    const GxB_FC64_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__expm1_fc64_fc64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__log10_fp32_fp32)
(
    float *Cx,
    const float *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__log10_fp32_fp32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__log10_fp64_fp64)
(
    double *Cx,
    const double *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__log10_fp64_fp64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__log10_fc32_fc32)
(
    GxB_FC32_t *Cx,
    const GxB_FC32_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__log10_fc32_fc32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__log10_fc64_fc64)
(
    GxB_FC64_t *Cx,
    const GxB_FC64_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__log10_fc64_fc64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__log1p_fp32_fp32)
(
    float *Cx,
    const float *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__log1p_fp32_fp32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__log1p_fp64_fp64)
(
    double *Cx,
    const double *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__log1p_fp64_fp64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__log1p_fc32_fc32)
(
    GxB_FC32_t *Cx,
    const GxB_FC32_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__log1p_fc32_fc32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__log1p_fc64_fc64)
(
    GxB_FC64_t *Cx,
    const GxB_FC64_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__log1p_fc64_fc64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__log2_fp32_fp32)
(
    float *Cx,
    const float *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__log2_fp32_fp32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__log2_fp64_fp64)
(
    double *Cx,
    const double *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__log2_fp64_fp64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__log2_fc32_fc32)
(
    GxB_FC32_t *Cx,
    const GxB_FC32_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__log2_fc32_fc32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__log2_fc64_fc64)
(
    GxB_FC64_t *Cx,
    const GxB_FC64_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__log2_fc64_fc64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__frexpx_fp32_fp32)
(
    float *Cx,
    const float *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__frexpx_fp32_fp32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__frexpx_fp64_fp64)
(
    double *Cx,
    const double *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__frexpx_fp64_fp64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__frexpe_fp32_fp32)
(
    float *Cx,
    const float *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__frexpe_fp32_fp32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__frexpe_fp64_fp64)
(
    double *Cx,
    const double *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__frexpe_fp64_fp64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__lgamma_fp32_fp32)
(
    float *Cx,
    const float *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__lgamma_fp32_fp32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__lgamma_fp64_fp64)
(
    double *Cx,
    const double *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__lgamma_fp64_fp64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__tgamma_fp32_fp32)
(
    float *Cx,
    const float *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__tgamma_fp32_fp32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__tgamma_fp64_fp64)
(
    double *Cx,
    const double *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__tgamma_fp64_fp64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__erf_fp32_fp32)
(
    float *Cx,
    const float *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__erf_fp32_fp32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__erf_fp64_fp64)
(
    double *Cx,
    const double *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__erf_fp64_fp64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__erfc_fp32_fp32)
(
    float *Cx,
    const float *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__erfc_fp32_fp32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__erfc_fp64_fp64)
(
    double *Cx,
    const double *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__erfc_fp64_fp64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__conj_fc32_fc32)
(
    GxB_FC32_t *Cx,
    const GxB_FC32_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__conj_fc32_fc32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__conj_fc64_fc64)
(
    GxB_FC64_t *Cx,
    const GxB_FC64_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__conj_fc64_fc64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__abs_fp32_fc32)
(
    float *Cx,
    const GxB_FC32_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__abs_fp32_fc32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__abs_fp64_fc64)
(
    double *Cx,
    const GxB_FC64_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__abs_fp64_fc64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__creal_fp32_fc32)
(
    float *Cx,
    const GxB_FC32_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__creal_fp32_fc32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__creal_fp64_fc64)
(
    double *Cx,
    const GxB_FC64_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__creal_fp64_fc64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__cimag_fp32_fc32)
(
    float *Cx,
    const GxB_FC32_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__cimag_fp32_fc32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__cimag_fp64_fc64)
(
    double *Cx,
    const GxB_FC64_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__cimag_fp64_fc64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__carg_fp32_fc32)
(
    float *Cx,
    const GxB_FC32_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__carg_fp32_fc32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__carg_fp64_fc64)
(
    double *Cx,
    const GxB_FC64_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__carg_fp64_fc64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__isinf_bool_fp32)
(
    bool *Cx,
    const float *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__isinf_bool_fp32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__isinf_bool_fp64)
(
    bool *Cx,
    const double *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__isinf_bool_fp64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__isinf_bool_fc32)
(
    bool *Cx,
    const GxB_FC32_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__isinf_bool_fc32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__isinf_bool_fc64)
(
    bool *Cx,
    const GxB_FC64_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__isinf_bool_fc64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__isnan_bool_fp32)
(
    bool *Cx,
    const float *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__isnan_bool_fp32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__isnan_bool_fp64)
(
    bool *Cx,
    const double *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__isnan_bool_fp64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__isnan_bool_fc32)
(
    bool *Cx,
    const GxB_FC32_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__isnan_bool_fc32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__isnan_bool_fc64)
(
    bool *Cx,
    const GxB_FC64_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__isnan_bool_fc64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__isfinite_bool_fp32)
(
    bool *Cx,
    const float *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__isfinite_bool_fp32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__isfinite_bool_fp64)
(
    bool *Cx,
    const double *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__isfinite_bool_fp64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__isfinite_bool_fc32)
(
    bool *Cx,
    const GxB_FC32_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__isfinite_bool_fc32)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0

GrB_Info GB (_unop_apply__isfinite_bool_fc64)
(
    bool *Cx,
    const GxB_FC64_t *Ax,
    const int8_t *restrict Ab,
    int64_t anz,
    int nthreads
) ;


GrB_Info GB (_unop_tran__isfinite_bool_fc64)
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *restrict *Workspaces,
    const int64_t *restrict A_slice,
    int nworkspaces,
    int nthreads
) ;

