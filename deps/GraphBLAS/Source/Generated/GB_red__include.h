//------------------------------------------------------------------------------
// GB_red__include.h: definitions for GB_red__*.c
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

// This file has been automatically generated from Generator/GB_red.h

// SPDX-License-Identifier: Apache-2.0


GrB_Info GB_red_scalar__min_int8
(
    int8_t *result,
    const GrB_Matrix A,
    GB_void *GB_RESTRICT W_space,
    bool *GB_RESTRICT F,
    int ntasks,
    int nthreads
) ;



GrB_Info GB_red_build__min_int8
(
    int8_t *GB_RESTRICT Tx,
    int64_t  *GB_RESTRICT Ti,
    const int8_t *GB_RESTRICT S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *GB_RESTRICT I_work,
    const int64_t *GB_RESTRICT K_work,
    const int64_t *GB_RESTRICT tstart_slice,
    const int64_t *GB_RESTRICT tnz_slice,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0


GrB_Info GB_red_scalar__min_int16
(
    int16_t *result,
    const GrB_Matrix A,
    GB_void *GB_RESTRICT W_space,
    bool *GB_RESTRICT F,
    int ntasks,
    int nthreads
) ;



GrB_Info GB_red_build__min_int16
(
    int16_t *GB_RESTRICT Tx,
    int64_t  *GB_RESTRICT Ti,
    const int16_t *GB_RESTRICT S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *GB_RESTRICT I_work,
    const int64_t *GB_RESTRICT K_work,
    const int64_t *GB_RESTRICT tstart_slice,
    const int64_t *GB_RESTRICT tnz_slice,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0


GrB_Info GB_red_scalar__min_int32
(
    int32_t *result,
    const GrB_Matrix A,
    GB_void *GB_RESTRICT W_space,
    bool *GB_RESTRICT F,
    int ntasks,
    int nthreads
) ;



GrB_Info GB_red_build__min_int32
(
    int32_t *GB_RESTRICT Tx,
    int64_t  *GB_RESTRICT Ti,
    const int32_t *GB_RESTRICT S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *GB_RESTRICT I_work,
    const int64_t *GB_RESTRICT K_work,
    const int64_t *GB_RESTRICT tstart_slice,
    const int64_t *GB_RESTRICT tnz_slice,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0


GrB_Info GB_red_scalar__min_int64
(
    int64_t *result,
    const GrB_Matrix A,
    GB_void *GB_RESTRICT W_space,
    bool *GB_RESTRICT F,
    int ntasks,
    int nthreads
) ;



GrB_Info GB_red_build__min_int64
(
    int64_t *GB_RESTRICT Tx,
    int64_t  *GB_RESTRICT Ti,
    const int64_t *GB_RESTRICT S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *GB_RESTRICT I_work,
    const int64_t *GB_RESTRICT K_work,
    const int64_t *GB_RESTRICT tstart_slice,
    const int64_t *GB_RESTRICT tnz_slice,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0


GrB_Info GB_red_scalar__min_uint8
(
    uint8_t *result,
    const GrB_Matrix A,
    GB_void *GB_RESTRICT W_space,
    bool *GB_RESTRICT F,
    int ntasks,
    int nthreads
) ;



GrB_Info GB_red_build__min_uint8
(
    uint8_t *GB_RESTRICT Tx,
    int64_t  *GB_RESTRICT Ti,
    const uint8_t *GB_RESTRICT S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *GB_RESTRICT I_work,
    const int64_t *GB_RESTRICT K_work,
    const int64_t *GB_RESTRICT tstart_slice,
    const int64_t *GB_RESTRICT tnz_slice,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0


GrB_Info GB_red_scalar__min_uint16
(
    uint16_t *result,
    const GrB_Matrix A,
    GB_void *GB_RESTRICT W_space,
    bool *GB_RESTRICT F,
    int ntasks,
    int nthreads
) ;



GrB_Info GB_red_build__min_uint16
(
    uint16_t *GB_RESTRICT Tx,
    int64_t  *GB_RESTRICT Ti,
    const uint16_t *GB_RESTRICT S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *GB_RESTRICT I_work,
    const int64_t *GB_RESTRICT K_work,
    const int64_t *GB_RESTRICT tstart_slice,
    const int64_t *GB_RESTRICT tnz_slice,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0


GrB_Info GB_red_scalar__min_uint32
(
    uint32_t *result,
    const GrB_Matrix A,
    GB_void *GB_RESTRICT W_space,
    bool *GB_RESTRICT F,
    int ntasks,
    int nthreads
) ;



GrB_Info GB_red_build__min_uint32
(
    uint32_t *GB_RESTRICT Tx,
    int64_t  *GB_RESTRICT Ti,
    const uint32_t *GB_RESTRICT S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *GB_RESTRICT I_work,
    const int64_t *GB_RESTRICT K_work,
    const int64_t *GB_RESTRICT tstart_slice,
    const int64_t *GB_RESTRICT tnz_slice,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0


GrB_Info GB_red_scalar__min_uint64
(
    uint64_t *result,
    const GrB_Matrix A,
    GB_void *GB_RESTRICT W_space,
    bool *GB_RESTRICT F,
    int ntasks,
    int nthreads
) ;



GrB_Info GB_red_build__min_uint64
(
    uint64_t *GB_RESTRICT Tx,
    int64_t  *GB_RESTRICT Ti,
    const uint64_t *GB_RESTRICT S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *GB_RESTRICT I_work,
    const int64_t *GB_RESTRICT K_work,
    const int64_t *GB_RESTRICT tstart_slice,
    const int64_t *GB_RESTRICT tnz_slice,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0


GrB_Info GB_red_scalar__min_fp32
(
    float *result,
    const GrB_Matrix A,
    GB_void *GB_RESTRICT W_space,
    bool *GB_RESTRICT F,
    int ntasks,
    int nthreads
) ;



GrB_Info GB_red_build__min_fp32
(
    float *GB_RESTRICT Tx,
    int64_t  *GB_RESTRICT Ti,
    const float *GB_RESTRICT S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *GB_RESTRICT I_work,
    const int64_t *GB_RESTRICT K_work,
    const int64_t *GB_RESTRICT tstart_slice,
    const int64_t *GB_RESTRICT tnz_slice,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0


GrB_Info GB_red_scalar__min_fp64
(
    double *result,
    const GrB_Matrix A,
    GB_void *GB_RESTRICT W_space,
    bool *GB_RESTRICT F,
    int ntasks,
    int nthreads
) ;



GrB_Info GB_red_build__min_fp64
(
    double *GB_RESTRICT Tx,
    int64_t  *GB_RESTRICT Ti,
    const double *GB_RESTRICT S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *GB_RESTRICT I_work,
    const int64_t *GB_RESTRICT K_work,
    const int64_t *GB_RESTRICT tstart_slice,
    const int64_t *GB_RESTRICT tnz_slice,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0


GrB_Info GB_red_scalar__max_int8
(
    int8_t *result,
    const GrB_Matrix A,
    GB_void *GB_RESTRICT W_space,
    bool *GB_RESTRICT F,
    int ntasks,
    int nthreads
) ;



GrB_Info GB_red_build__max_int8
(
    int8_t *GB_RESTRICT Tx,
    int64_t  *GB_RESTRICT Ti,
    const int8_t *GB_RESTRICT S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *GB_RESTRICT I_work,
    const int64_t *GB_RESTRICT K_work,
    const int64_t *GB_RESTRICT tstart_slice,
    const int64_t *GB_RESTRICT tnz_slice,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0


GrB_Info GB_red_scalar__max_int16
(
    int16_t *result,
    const GrB_Matrix A,
    GB_void *GB_RESTRICT W_space,
    bool *GB_RESTRICT F,
    int ntasks,
    int nthreads
) ;



GrB_Info GB_red_build__max_int16
(
    int16_t *GB_RESTRICT Tx,
    int64_t  *GB_RESTRICT Ti,
    const int16_t *GB_RESTRICT S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *GB_RESTRICT I_work,
    const int64_t *GB_RESTRICT K_work,
    const int64_t *GB_RESTRICT tstart_slice,
    const int64_t *GB_RESTRICT tnz_slice,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0


GrB_Info GB_red_scalar__max_int32
(
    int32_t *result,
    const GrB_Matrix A,
    GB_void *GB_RESTRICT W_space,
    bool *GB_RESTRICT F,
    int ntasks,
    int nthreads
) ;



GrB_Info GB_red_build__max_int32
(
    int32_t *GB_RESTRICT Tx,
    int64_t  *GB_RESTRICT Ti,
    const int32_t *GB_RESTRICT S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *GB_RESTRICT I_work,
    const int64_t *GB_RESTRICT K_work,
    const int64_t *GB_RESTRICT tstart_slice,
    const int64_t *GB_RESTRICT tnz_slice,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0


GrB_Info GB_red_scalar__max_int64
(
    int64_t *result,
    const GrB_Matrix A,
    GB_void *GB_RESTRICT W_space,
    bool *GB_RESTRICT F,
    int ntasks,
    int nthreads
) ;



GrB_Info GB_red_build__max_int64
(
    int64_t *GB_RESTRICT Tx,
    int64_t  *GB_RESTRICT Ti,
    const int64_t *GB_RESTRICT S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *GB_RESTRICT I_work,
    const int64_t *GB_RESTRICT K_work,
    const int64_t *GB_RESTRICT tstart_slice,
    const int64_t *GB_RESTRICT tnz_slice,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0


GrB_Info GB_red_scalar__max_uint8
(
    uint8_t *result,
    const GrB_Matrix A,
    GB_void *GB_RESTRICT W_space,
    bool *GB_RESTRICT F,
    int ntasks,
    int nthreads
) ;



GrB_Info GB_red_build__max_uint8
(
    uint8_t *GB_RESTRICT Tx,
    int64_t  *GB_RESTRICT Ti,
    const uint8_t *GB_RESTRICT S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *GB_RESTRICT I_work,
    const int64_t *GB_RESTRICT K_work,
    const int64_t *GB_RESTRICT tstart_slice,
    const int64_t *GB_RESTRICT tnz_slice,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0


GrB_Info GB_red_scalar__max_uint16
(
    uint16_t *result,
    const GrB_Matrix A,
    GB_void *GB_RESTRICT W_space,
    bool *GB_RESTRICT F,
    int ntasks,
    int nthreads
) ;



GrB_Info GB_red_build__max_uint16
(
    uint16_t *GB_RESTRICT Tx,
    int64_t  *GB_RESTRICT Ti,
    const uint16_t *GB_RESTRICT S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *GB_RESTRICT I_work,
    const int64_t *GB_RESTRICT K_work,
    const int64_t *GB_RESTRICT tstart_slice,
    const int64_t *GB_RESTRICT tnz_slice,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0


GrB_Info GB_red_scalar__max_uint32
(
    uint32_t *result,
    const GrB_Matrix A,
    GB_void *GB_RESTRICT W_space,
    bool *GB_RESTRICT F,
    int ntasks,
    int nthreads
) ;



GrB_Info GB_red_build__max_uint32
(
    uint32_t *GB_RESTRICT Tx,
    int64_t  *GB_RESTRICT Ti,
    const uint32_t *GB_RESTRICT S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *GB_RESTRICT I_work,
    const int64_t *GB_RESTRICT K_work,
    const int64_t *GB_RESTRICT tstart_slice,
    const int64_t *GB_RESTRICT tnz_slice,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0


GrB_Info GB_red_scalar__max_uint64
(
    uint64_t *result,
    const GrB_Matrix A,
    GB_void *GB_RESTRICT W_space,
    bool *GB_RESTRICT F,
    int ntasks,
    int nthreads
) ;



GrB_Info GB_red_build__max_uint64
(
    uint64_t *GB_RESTRICT Tx,
    int64_t  *GB_RESTRICT Ti,
    const uint64_t *GB_RESTRICT S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *GB_RESTRICT I_work,
    const int64_t *GB_RESTRICT K_work,
    const int64_t *GB_RESTRICT tstart_slice,
    const int64_t *GB_RESTRICT tnz_slice,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0


GrB_Info GB_red_scalar__max_fp32
(
    float *result,
    const GrB_Matrix A,
    GB_void *GB_RESTRICT W_space,
    bool *GB_RESTRICT F,
    int ntasks,
    int nthreads
) ;



GrB_Info GB_red_build__max_fp32
(
    float *GB_RESTRICT Tx,
    int64_t  *GB_RESTRICT Ti,
    const float *GB_RESTRICT S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *GB_RESTRICT I_work,
    const int64_t *GB_RESTRICT K_work,
    const int64_t *GB_RESTRICT tstart_slice,
    const int64_t *GB_RESTRICT tnz_slice,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0


GrB_Info GB_red_scalar__max_fp64
(
    double *result,
    const GrB_Matrix A,
    GB_void *GB_RESTRICT W_space,
    bool *GB_RESTRICT F,
    int ntasks,
    int nthreads
) ;



GrB_Info GB_red_build__max_fp64
(
    double *GB_RESTRICT Tx,
    int64_t  *GB_RESTRICT Ti,
    const double *GB_RESTRICT S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *GB_RESTRICT I_work,
    const int64_t *GB_RESTRICT K_work,
    const int64_t *GB_RESTRICT tstart_slice,
    const int64_t *GB_RESTRICT tnz_slice,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0


GrB_Info GB_red_scalar__any_bool
(
    bool *result,
    const GrB_Matrix A,
    GB_void *GB_RESTRICT W_space,
    bool *GB_RESTRICT F,
    int ntasks,
    int nthreads
) ;



GrB_Info GB_red_build__any_bool
(
    bool *GB_RESTRICT Tx,
    int64_t  *GB_RESTRICT Ti,
    const bool *GB_RESTRICT S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *GB_RESTRICT I_work,
    const int64_t *GB_RESTRICT K_work,
    const int64_t *GB_RESTRICT tstart_slice,
    const int64_t *GB_RESTRICT tnz_slice,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0


GrB_Info GB_red_scalar__any_int8
(
    int8_t *result,
    const GrB_Matrix A,
    GB_void *GB_RESTRICT W_space,
    bool *GB_RESTRICT F,
    int ntasks,
    int nthreads
) ;



GrB_Info GB_red_build__any_int8
(
    int8_t *GB_RESTRICT Tx,
    int64_t  *GB_RESTRICT Ti,
    const int8_t *GB_RESTRICT S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *GB_RESTRICT I_work,
    const int64_t *GB_RESTRICT K_work,
    const int64_t *GB_RESTRICT tstart_slice,
    const int64_t *GB_RESTRICT tnz_slice,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0


GrB_Info GB_red_scalar__any_int16
(
    int16_t *result,
    const GrB_Matrix A,
    GB_void *GB_RESTRICT W_space,
    bool *GB_RESTRICT F,
    int ntasks,
    int nthreads
) ;



GrB_Info GB_red_build__any_int16
(
    int16_t *GB_RESTRICT Tx,
    int64_t  *GB_RESTRICT Ti,
    const int16_t *GB_RESTRICT S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *GB_RESTRICT I_work,
    const int64_t *GB_RESTRICT K_work,
    const int64_t *GB_RESTRICT tstart_slice,
    const int64_t *GB_RESTRICT tnz_slice,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0


GrB_Info GB_red_scalar__any_int32
(
    int32_t *result,
    const GrB_Matrix A,
    GB_void *GB_RESTRICT W_space,
    bool *GB_RESTRICT F,
    int ntasks,
    int nthreads
) ;



GrB_Info GB_red_build__any_int32
(
    int32_t *GB_RESTRICT Tx,
    int64_t  *GB_RESTRICT Ti,
    const int32_t *GB_RESTRICT S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *GB_RESTRICT I_work,
    const int64_t *GB_RESTRICT K_work,
    const int64_t *GB_RESTRICT tstart_slice,
    const int64_t *GB_RESTRICT tnz_slice,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0


GrB_Info GB_red_scalar__any_int64
(
    int64_t *result,
    const GrB_Matrix A,
    GB_void *GB_RESTRICT W_space,
    bool *GB_RESTRICT F,
    int ntasks,
    int nthreads
) ;



GrB_Info GB_red_build__any_int64
(
    int64_t *GB_RESTRICT Tx,
    int64_t  *GB_RESTRICT Ti,
    const int64_t *GB_RESTRICT S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *GB_RESTRICT I_work,
    const int64_t *GB_RESTRICT K_work,
    const int64_t *GB_RESTRICT tstart_slice,
    const int64_t *GB_RESTRICT tnz_slice,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0


GrB_Info GB_red_scalar__any_uint8
(
    uint8_t *result,
    const GrB_Matrix A,
    GB_void *GB_RESTRICT W_space,
    bool *GB_RESTRICT F,
    int ntasks,
    int nthreads
) ;



GrB_Info GB_red_build__any_uint8
(
    uint8_t *GB_RESTRICT Tx,
    int64_t  *GB_RESTRICT Ti,
    const uint8_t *GB_RESTRICT S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *GB_RESTRICT I_work,
    const int64_t *GB_RESTRICT K_work,
    const int64_t *GB_RESTRICT tstart_slice,
    const int64_t *GB_RESTRICT tnz_slice,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0


GrB_Info GB_red_scalar__any_uint16
(
    uint16_t *result,
    const GrB_Matrix A,
    GB_void *GB_RESTRICT W_space,
    bool *GB_RESTRICT F,
    int ntasks,
    int nthreads
) ;



GrB_Info GB_red_build__any_uint16
(
    uint16_t *GB_RESTRICT Tx,
    int64_t  *GB_RESTRICT Ti,
    const uint16_t *GB_RESTRICT S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *GB_RESTRICT I_work,
    const int64_t *GB_RESTRICT K_work,
    const int64_t *GB_RESTRICT tstart_slice,
    const int64_t *GB_RESTRICT tnz_slice,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0


GrB_Info GB_red_scalar__any_uint32
(
    uint32_t *result,
    const GrB_Matrix A,
    GB_void *GB_RESTRICT W_space,
    bool *GB_RESTRICT F,
    int ntasks,
    int nthreads
) ;



GrB_Info GB_red_build__any_uint32
(
    uint32_t *GB_RESTRICT Tx,
    int64_t  *GB_RESTRICT Ti,
    const uint32_t *GB_RESTRICT S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *GB_RESTRICT I_work,
    const int64_t *GB_RESTRICT K_work,
    const int64_t *GB_RESTRICT tstart_slice,
    const int64_t *GB_RESTRICT tnz_slice,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0


GrB_Info GB_red_scalar__any_uint64
(
    uint64_t *result,
    const GrB_Matrix A,
    GB_void *GB_RESTRICT W_space,
    bool *GB_RESTRICT F,
    int ntasks,
    int nthreads
) ;



GrB_Info GB_red_build__any_uint64
(
    uint64_t *GB_RESTRICT Tx,
    int64_t  *GB_RESTRICT Ti,
    const uint64_t *GB_RESTRICT S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *GB_RESTRICT I_work,
    const int64_t *GB_RESTRICT K_work,
    const int64_t *GB_RESTRICT tstart_slice,
    const int64_t *GB_RESTRICT tnz_slice,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0


GrB_Info GB_red_scalar__any_fp32
(
    float *result,
    const GrB_Matrix A,
    GB_void *GB_RESTRICT W_space,
    bool *GB_RESTRICT F,
    int ntasks,
    int nthreads
) ;



GrB_Info GB_red_build__any_fp32
(
    float *GB_RESTRICT Tx,
    int64_t  *GB_RESTRICT Ti,
    const float *GB_RESTRICT S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *GB_RESTRICT I_work,
    const int64_t *GB_RESTRICT K_work,
    const int64_t *GB_RESTRICT tstart_slice,
    const int64_t *GB_RESTRICT tnz_slice,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0


GrB_Info GB_red_scalar__any_fp64
(
    double *result,
    const GrB_Matrix A,
    GB_void *GB_RESTRICT W_space,
    bool *GB_RESTRICT F,
    int ntasks,
    int nthreads
) ;



GrB_Info GB_red_build__any_fp64
(
    double *GB_RESTRICT Tx,
    int64_t  *GB_RESTRICT Ti,
    const double *GB_RESTRICT S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *GB_RESTRICT I_work,
    const int64_t *GB_RESTRICT K_work,
    const int64_t *GB_RESTRICT tstart_slice,
    const int64_t *GB_RESTRICT tnz_slice,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0


GrB_Info GB_red_scalar__any_fc32
(
    GxB_FC32_t *result,
    const GrB_Matrix A,
    GB_void *GB_RESTRICT W_space,
    bool *GB_RESTRICT F,
    int ntasks,
    int nthreads
) ;



GrB_Info GB_red_build__any_fc32
(
    GxB_FC32_t *GB_RESTRICT Tx,
    int64_t  *GB_RESTRICT Ti,
    const GxB_FC32_t *GB_RESTRICT S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *GB_RESTRICT I_work,
    const int64_t *GB_RESTRICT K_work,
    const int64_t *GB_RESTRICT tstart_slice,
    const int64_t *GB_RESTRICT tnz_slice,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0


GrB_Info GB_red_scalar__any_fc64
(
    GxB_FC64_t *result,
    const GrB_Matrix A,
    GB_void *GB_RESTRICT W_space,
    bool *GB_RESTRICT F,
    int ntasks,
    int nthreads
) ;



GrB_Info GB_red_build__any_fc64
(
    GxB_FC64_t *GB_RESTRICT Tx,
    int64_t  *GB_RESTRICT Ti,
    const GxB_FC64_t *GB_RESTRICT S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *GB_RESTRICT I_work,
    const int64_t *GB_RESTRICT K_work,
    const int64_t *GB_RESTRICT tstart_slice,
    const int64_t *GB_RESTRICT tnz_slice,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0


GrB_Info GB_red_scalar__plus_int8
(
    int8_t *result,
    const GrB_Matrix A,
    GB_void *GB_RESTRICT W_space,
    bool *GB_RESTRICT F,
    int ntasks,
    int nthreads
) ;



GrB_Info GB_red_build__plus_int8
(
    int8_t *GB_RESTRICT Tx,
    int64_t  *GB_RESTRICT Ti,
    const int8_t *GB_RESTRICT S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *GB_RESTRICT I_work,
    const int64_t *GB_RESTRICT K_work,
    const int64_t *GB_RESTRICT tstart_slice,
    const int64_t *GB_RESTRICT tnz_slice,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0


GrB_Info GB_red_scalar__plus_int16
(
    int16_t *result,
    const GrB_Matrix A,
    GB_void *GB_RESTRICT W_space,
    bool *GB_RESTRICT F,
    int ntasks,
    int nthreads
) ;



GrB_Info GB_red_build__plus_int16
(
    int16_t *GB_RESTRICT Tx,
    int64_t  *GB_RESTRICT Ti,
    const int16_t *GB_RESTRICT S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *GB_RESTRICT I_work,
    const int64_t *GB_RESTRICT K_work,
    const int64_t *GB_RESTRICT tstart_slice,
    const int64_t *GB_RESTRICT tnz_slice,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0


GrB_Info GB_red_scalar__plus_int32
(
    int32_t *result,
    const GrB_Matrix A,
    GB_void *GB_RESTRICT W_space,
    bool *GB_RESTRICT F,
    int ntasks,
    int nthreads
) ;



GrB_Info GB_red_build__plus_int32
(
    int32_t *GB_RESTRICT Tx,
    int64_t  *GB_RESTRICT Ti,
    const int32_t *GB_RESTRICT S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *GB_RESTRICT I_work,
    const int64_t *GB_RESTRICT K_work,
    const int64_t *GB_RESTRICT tstart_slice,
    const int64_t *GB_RESTRICT tnz_slice,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0


GrB_Info GB_red_scalar__plus_int64
(
    int64_t *result,
    const GrB_Matrix A,
    GB_void *GB_RESTRICT W_space,
    bool *GB_RESTRICT F,
    int ntasks,
    int nthreads
) ;



GrB_Info GB_red_build__plus_int64
(
    int64_t *GB_RESTRICT Tx,
    int64_t  *GB_RESTRICT Ti,
    const int64_t *GB_RESTRICT S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *GB_RESTRICT I_work,
    const int64_t *GB_RESTRICT K_work,
    const int64_t *GB_RESTRICT tstart_slice,
    const int64_t *GB_RESTRICT tnz_slice,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0


GrB_Info GB_red_scalar__plus_uint8
(
    uint8_t *result,
    const GrB_Matrix A,
    GB_void *GB_RESTRICT W_space,
    bool *GB_RESTRICT F,
    int ntasks,
    int nthreads
) ;



GrB_Info GB_red_build__plus_uint8
(
    uint8_t *GB_RESTRICT Tx,
    int64_t  *GB_RESTRICT Ti,
    const uint8_t *GB_RESTRICT S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *GB_RESTRICT I_work,
    const int64_t *GB_RESTRICT K_work,
    const int64_t *GB_RESTRICT tstart_slice,
    const int64_t *GB_RESTRICT tnz_slice,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0


GrB_Info GB_red_scalar__plus_uint16
(
    uint16_t *result,
    const GrB_Matrix A,
    GB_void *GB_RESTRICT W_space,
    bool *GB_RESTRICT F,
    int ntasks,
    int nthreads
) ;



GrB_Info GB_red_build__plus_uint16
(
    uint16_t *GB_RESTRICT Tx,
    int64_t  *GB_RESTRICT Ti,
    const uint16_t *GB_RESTRICT S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *GB_RESTRICT I_work,
    const int64_t *GB_RESTRICT K_work,
    const int64_t *GB_RESTRICT tstart_slice,
    const int64_t *GB_RESTRICT tnz_slice,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0


GrB_Info GB_red_scalar__plus_uint32
(
    uint32_t *result,
    const GrB_Matrix A,
    GB_void *GB_RESTRICT W_space,
    bool *GB_RESTRICT F,
    int ntasks,
    int nthreads
) ;



GrB_Info GB_red_build__plus_uint32
(
    uint32_t *GB_RESTRICT Tx,
    int64_t  *GB_RESTRICT Ti,
    const uint32_t *GB_RESTRICT S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *GB_RESTRICT I_work,
    const int64_t *GB_RESTRICT K_work,
    const int64_t *GB_RESTRICT tstart_slice,
    const int64_t *GB_RESTRICT tnz_slice,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0


GrB_Info GB_red_scalar__plus_uint64
(
    uint64_t *result,
    const GrB_Matrix A,
    GB_void *GB_RESTRICT W_space,
    bool *GB_RESTRICT F,
    int ntasks,
    int nthreads
) ;



GrB_Info GB_red_build__plus_uint64
(
    uint64_t *GB_RESTRICT Tx,
    int64_t  *GB_RESTRICT Ti,
    const uint64_t *GB_RESTRICT S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *GB_RESTRICT I_work,
    const int64_t *GB_RESTRICT K_work,
    const int64_t *GB_RESTRICT tstart_slice,
    const int64_t *GB_RESTRICT tnz_slice,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0


GrB_Info GB_red_scalar__plus_fp32
(
    float *result,
    const GrB_Matrix A,
    GB_void *GB_RESTRICT W_space,
    bool *GB_RESTRICT F,
    int ntasks,
    int nthreads
) ;



GrB_Info GB_red_build__plus_fp32
(
    float *GB_RESTRICT Tx,
    int64_t  *GB_RESTRICT Ti,
    const float *GB_RESTRICT S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *GB_RESTRICT I_work,
    const int64_t *GB_RESTRICT K_work,
    const int64_t *GB_RESTRICT tstart_slice,
    const int64_t *GB_RESTRICT tnz_slice,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0


GrB_Info GB_red_scalar__plus_fp64
(
    double *result,
    const GrB_Matrix A,
    GB_void *GB_RESTRICT W_space,
    bool *GB_RESTRICT F,
    int ntasks,
    int nthreads
) ;



GrB_Info GB_red_build__plus_fp64
(
    double *GB_RESTRICT Tx,
    int64_t  *GB_RESTRICT Ti,
    const double *GB_RESTRICT S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *GB_RESTRICT I_work,
    const int64_t *GB_RESTRICT K_work,
    const int64_t *GB_RESTRICT tstart_slice,
    const int64_t *GB_RESTRICT tnz_slice,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0


GrB_Info GB_red_scalar__plus_fc32
(
    GxB_FC32_t *result,
    const GrB_Matrix A,
    GB_void *GB_RESTRICT W_space,
    bool *GB_RESTRICT F,
    int ntasks,
    int nthreads
) ;



GrB_Info GB_red_build__plus_fc32
(
    GxB_FC32_t *GB_RESTRICT Tx,
    int64_t  *GB_RESTRICT Ti,
    const GxB_FC32_t *GB_RESTRICT S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *GB_RESTRICT I_work,
    const int64_t *GB_RESTRICT K_work,
    const int64_t *GB_RESTRICT tstart_slice,
    const int64_t *GB_RESTRICT tnz_slice,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0


GrB_Info GB_red_scalar__plus_fc64
(
    GxB_FC64_t *result,
    const GrB_Matrix A,
    GB_void *GB_RESTRICT W_space,
    bool *GB_RESTRICT F,
    int ntasks,
    int nthreads
) ;



GrB_Info GB_red_build__plus_fc64
(
    GxB_FC64_t *GB_RESTRICT Tx,
    int64_t  *GB_RESTRICT Ti,
    const GxB_FC64_t *GB_RESTRICT S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *GB_RESTRICT I_work,
    const int64_t *GB_RESTRICT K_work,
    const int64_t *GB_RESTRICT tstart_slice,
    const int64_t *GB_RESTRICT tnz_slice,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0


GrB_Info GB_red_scalar__times_int8
(
    int8_t *result,
    const GrB_Matrix A,
    GB_void *GB_RESTRICT W_space,
    bool *GB_RESTRICT F,
    int ntasks,
    int nthreads
) ;



GrB_Info GB_red_build__times_int8
(
    int8_t *GB_RESTRICT Tx,
    int64_t  *GB_RESTRICT Ti,
    const int8_t *GB_RESTRICT S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *GB_RESTRICT I_work,
    const int64_t *GB_RESTRICT K_work,
    const int64_t *GB_RESTRICT tstart_slice,
    const int64_t *GB_RESTRICT tnz_slice,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0


GrB_Info GB_red_scalar__times_int16
(
    int16_t *result,
    const GrB_Matrix A,
    GB_void *GB_RESTRICT W_space,
    bool *GB_RESTRICT F,
    int ntasks,
    int nthreads
) ;



GrB_Info GB_red_build__times_int16
(
    int16_t *GB_RESTRICT Tx,
    int64_t  *GB_RESTRICT Ti,
    const int16_t *GB_RESTRICT S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *GB_RESTRICT I_work,
    const int64_t *GB_RESTRICT K_work,
    const int64_t *GB_RESTRICT tstart_slice,
    const int64_t *GB_RESTRICT tnz_slice,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0


GrB_Info GB_red_scalar__times_int32
(
    int32_t *result,
    const GrB_Matrix A,
    GB_void *GB_RESTRICT W_space,
    bool *GB_RESTRICT F,
    int ntasks,
    int nthreads
) ;



GrB_Info GB_red_build__times_int32
(
    int32_t *GB_RESTRICT Tx,
    int64_t  *GB_RESTRICT Ti,
    const int32_t *GB_RESTRICT S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *GB_RESTRICT I_work,
    const int64_t *GB_RESTRICT K_work,
    const int64_t *GB_RESTRICT tstart_slice,
    const int64_t *GB_RESTRICT tnz_slice,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0


GrB_Info GB_red_scalar__times_int64
(
    int64_t *result,
    const GrB_Matrix A,
    GB_void *GB_RESTRICT W_space,
    bool *GB_RESTRICT F,
    int ntasks,
    int nthreads
) ;



GrB_Info GB_red_build__times_int64
(
    int64_t *GB_RESTRICT Tx,
    int64_t  *GB_RESTRICT Ti,
    const int64_t *GB_RESTRICT S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *GB_RESTRICT I_work,
    const int64_t *GB_RESTRICT K_work,
    const int64_t *GB_RESTRICT tstart_slice,
    const int64_t *GB_RESTRICT tnz_slice,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0


GrB_Info GB_red_scalar__times_uint8
(
    uint8_t *result,
    const GrB_Matrix A,
    GB_void *GB_RESTRICT W_space,
    bool *GB_RESTRICT F,
    int ntasks,
    int nthreads
) ;



GrB_Info GB_red_build__times_uint8
(
    uint8_t *GB_RESTRICT Tx,
    int64_t  *GB_RESTRICT Ti,
    const uint8_t *GB_RESTRICT S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *GB_RESTRICT I_work,
    const int64_t *GB_RESTRICT K_work,
    const int64_t *GB_RESTRICT tstart_slice,
    const int64_t *GB_RESTRICT tnz_slice,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0


GrB_Info GB_red_scalar__times_uint16
(
    uint16_t *result,
    const GrB_Matrix A,
    GB_void *GB_RESTRICT W_space,
    bool *GB_RESTRICT F,
    int ntasks,
    int nthreads
) ;



GrB_Info GB_red_build__times_uint16
(
    uint16_t *GB_RESTRICT Tx,
    int64_t  *GB_RESTRICT Ti,
    const uint16_t *GB_RESTRICT S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *GB_RESTRICT I_work,
    const int64_t *GB_RESTRICT K_work,
    const int64_t *GB_RESTRICT tstart_slice,
    const int64_t *GB_RESTRICT tnz_slice,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0


GrB_Info GB_red_scalar__times_uint32
(
    uint32_t *result,
    const GrB_Matrix A,
    GB_void *GB_RESTRICT W_space,
    bool *GB_RESTRICT F,
    int ntasks,
    int nthreads
) ;



GrB_Info GB_red_build__times_uint32
(
    uint32_t *GB_RESTRICT Tx,
    int64_t  *GB_RESTRICT Ti,
    const uint32_t *GB_RESTRICT S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *GB_RESTRICT I_work,
    const int64_t *GB_RESTRICT K_work,
    const int64_t *GB_RESTRICT tstart_slice,
    const int64_t *GB_RESTRICT tnz_slice,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0


GrB_Info GB_red_scalar__times_uint64
(
    uint64_t *result,
    const GrB_Matrix A,
    GB_void *GB_RESTRICT W_space,
    bool *GB_RESTRICT F,
    int ntasks,
    int nthreads
) ;



GrB_Info GB_red_build__times_uint64
(
    uint64_t *GB_RESTRICT Tx,
    int64_t  *GB_RESTRICT Ti,
    const uint64_t *GB_RESTRICT S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *GB_RESTRICT I_work,
    const int64_t *GB_RESTRICT K_work,
    const int64_t *GB_RESTRICT tstart_slice,
    const int64_t *GB_RESTRICT tnz_slice,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0


GrB_Info GB_red_scalar__times_fp32
(
    float *result,
    const GrB_Matrix A,
    GB_void *GB_RESTRICT W_space,
    bool *GB_RESTRICT F,
    int ntasks,
    int nthreads
) ;



GrB_Info GB_red_build__times_fp32
(
    float *GB_RESTRICT Tx,
    int64_t  *GB_RESTRICT Ti,
    const float *GB_RESTRICT S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *GB_RESTRICT I_work,
    const int64_t *GB_RESTRICT K_work,
    const int64_t *GB_RESTRICT tstart_slice,
    const int64_t *GB_RESTRICT tnz_slice,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0


GrB_Info GB_red_scalar__times_fp64
(
    double *result,
    const GrB_Matrix A,
    GB_void *GB_RESTRICT W_space,
    bool *GB_RESTRICT F,
    int ntasks,
    int nthreads
) ;



GrB_Info GB_red_build__times_fp64
(
    double *GB_RESTRICT Tx,
    int64_t  *GB_RESTRICT Ti,
    const double *GB_RESTRICT S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *GB_RESTRICT I_work,
    const int64_t *GB_RESTRICT K_work,
    const int64_t *GB_RESTRICT tstart_slice,
    const int64_t *GB_RESTRICT tnz_slice,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0


GrB_Info GB_red_scalar__times_fc32
(
    GxB_FC32_t *result,
    const GrB_Matrix A,
    GB_void *GB_RESTRICT W_space,
    bool *GB_RESTRICT F,
    int ntasks,
    int nthreads
) ;



GrB_Info GB_red_build__times_fc32
(
    GxB_FC32_t *GB_RESTRICT Tx,
    int64_t  *GB_RESTRICT Ti,
    const GxB_FC32_t *GB_RESTRICT S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *GB_RESTRICT I_work,
    const int64_t *GB_RESTRICT K_work,
    const int64_t *GB_RESTRICT tstart_slice,
    const int64_t *GB_RESTRICT tnz_slice,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0


GrB_Info GB_red_scalar__times_fc64
(
    GxB_FC64_t *result,
    const GrB_Matrix A,
    GB_void *GB_RESTRICT W_space,
    bool *GB_RESTRICT F,
    int ntasks,
    int nthreads
) ;



GrB_Info GB_red_build__times_fc64
(
    GxB_FC64_t *GB_RESTRICT Tx,
    int64_t  *GB_RESTRICT Ti,
    const GxB_FC64_t *GB_RESTRICT S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *GB_RESTRICT I_work,
    const int64_t *GB_RESTRICT K_work,
    const int64_t *GB_RESTRICT tstart_slice,
    const int64_t *GB_RESTRICT tnz_slice,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0


GrB_Info GB_red_scalar__lor_bool
(
    bool *result,
    const GrB_Matrix A,
    GB_void *GB_RESTRICT W_space,
    bool *GB_RESTRICT F,
    int ntasks,
    int nthreads
) ;



GrB_Info GB_red_build__lor_bool
(
    bool *GB_RESTRICT Tx,
    int64_t  *GB_RESTRICT Ti,
    const bool *GB_RESTRICT S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *GB_RESTRICT I_work,
    const int64_t *GB_RESTRICT K_work,
    const int64_t *GB_RESTRICT tstart_slice,
    const int64_t *GB_RESTRICT tnz_slice,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0


GrB_Info GB_red_scalar__land_bool
(
    bool *result,
    const GrB_Matrix A,
    GB_void *GB_RESTRICT W_space,
    bool *GB_RESTRICT F,
    int ntasks,
    int nthreads
) ;



GrB_Info GB_red_build__land_bool
(
    bool *GB_RESTRICT Tx,
    int64_t  *GB_RESTRICT Ti,
    const bool *GB_RESTRICT S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *GB_RESTRICT I_work,
    const int64_t *GB_RESTRICT K_work,
    const int64_t *GB_RESTRICT tstart_slice,
    const int64_t *GB_RESTRICT tnz_slice,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0


GrB_Info GB_red_scalar__lxor_bool
(
    bool *result,
    const GrB_Matrix A,
    GB_void *GB_RESTRICT W_space,
    bool *GB_RESTRICT F,
    int ntasks,
    int nthreads
) ;



GrB_Info GB_red_build__lxor_bool
(
    bool *GB_RESTRICT Tx,
    int64_t  *GB_RESTRICT Ti,
    const bool *GB_RESTRICT S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *GB_RESTRICT I_work,
    const int64_t *GB_RESTRICT K_work,
    const int64_t *GB_RESTRICT tstart_slice,
    const int64_t *GB_RESTRICT tnz_slice,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0


GrB_Info GB_red_scalar__eq_bool
(
    bool *result,
    const GrB_Matrix A,
    GB_void *GB_RESTRICT W_space,
    bool *GB_RESTRICT F,
    int ntasks,
    int nthreads
) ;



GrB_Info GB_red_build__eq_bool
(
    bool *GB_RESTRICT Tx,
    int64_t  *GB_RESTRICT Ti,
    const bool *GB_RESTRICT S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *GB_RESTRICT I_work,
    const int64_t *GB_RESTRICT K_work,
    const int64_t *GB_RESTRICT tstart_slice,
    const int64_t *GB_RESTRICT tnz_slice,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0


GrB_Info GB_red_scalar__any_bool
(
    bool *result,
    const GrB_Matrix A,
    GB_void *GB_RESTRICT W_space,
    bool *GB_RESTRICT F,
    int ntasks,
    int nthreads
) ;



GrB_Info GB_red_build__any_bool
(
    bool *GB_RESTRICT Tx,
    int64_t  *GB_RESTRICT Ti,
    const bool *GB_RESTRICT S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *GB_RESTRICT I_work,
    const int64_t *GB_RESTRICT K_work,
    const int64_t *GB_RESTRICT tstart_slice,
    const int64_t *GB_RESTRICT tnz_slice,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0
#if 0

GrB_Info GB_red_scalar__(none)
(
    bool *result,
    const GrB_Matrix A,
    GB_void *GB_RESTRICT W_space,
    bool *GB_RESTRICT F,
    int ntasks,
    int nthreads
) ;

#endif

GrB_Info GB_red_build__first_bool
(
    bool *GB_RESTRICT Tx,
    int64_t  *GB_RESTRICT Ti,
    const bool *GB_RESTRICT S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *GB_RESTRICT I_work,
    const int64_t *GB_RESTRICT K_work,
    const int64_t *GB_RESTRICT tstart_slice,
    const int64_t *GB_RESTRICT tnz_slice,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0
#if 0

GrB_Info GB_red_scalar__(none)
(
    int8_t *result,
    const GrB_Matrix A,
    GB_void *GB_RESTRICT W_space,
    bool *GB_RESTRICT F,
    int ntasks,
    int nthreads
) ;

#endif

GrB_Info GB_red_build__first_int8
(
    int8_t *GB_RESTRICT Tx,
    int64_t  *GB_RESTRICT Ti,
    const int8_t *GB_RESTRICT S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *GB_RESTRICT I_work,
    const int64_t *GB_RESTRICT K_work,
    const int64_t *GB_RESTRICT tstart_slice,
    const int64_t *GB_RESTRICT tnz_slice,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0
#if 0

GrB_Info GB_red_scalar__(none)
(
    int16_t *result,
    const GrB_Matrix A,
    GB_void *GB_RESTRICT W_space,
    bool *GB_RESTRICT F,
    int ntasks,
    int nthreads
) ;

#endif

GrB_Info GB_red_build__first_int16
(
    int16_t *GB_RESTRICT Tx,
    int64_t  *GB_RESTRICT Ti,
    const int16_t *GB_RESTRICT S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *GB_RESTRICT I_work,
    const int64_t *GB_RESTRICT K_work,
    const int64_t *GB_RESTRICT tstart_slice,
    const int64_t *GB_RESTRICT tnz_slice,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0
#if 0

GrB_Info GB_red_scalar__(none)
(
    int32_t *result,
    const GrB_Matrix A,
    GB_void *GB_RESTRICT W_space,
    bool *GB_RESTRICT F,
    int ntasks,
    int nthreads
) ;

#endif

GrB_Info GB_red_build__first_int32
(
    int32_t *GB_RESTRICT Tx,
    int64_t  *GB_RESTRICT Ti,
    const int32_t *GB_RESTRICT S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *GB_RESTRICT I_work,
    const int64_t *GB_RESTRICT K_work,
    const int64_t *GB_RESTRICT tstart_slice,
    const int64_t *GB_RESTRICT tnz_slice,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0
#if 0

GrB_Info GB_red_scalar__(none)
(
    int64_t *result,
    const GrB_Matrix A,
    GB_void *GB_RESTRICT W_space,
    bool *GB_RESTRICT F,
    int ntasks,
    int nthreads
) ;

#endif

GrB_Info GB_red_build__first_int64
(
    int64_t *GB_RESTRICT Tx,
    int64_t  *GB_RESTRICT Ti,
    const int64_t *GB_RESTRICT S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *GB_RESTRICT I_work,
    const int64_t *GB_RESTRICT K_work,
    const int64_t *GB_RESTRICT tstart_slice,
    const int64_t *GB_RESTRICT tnz_slice,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0
#if 0

GrB_Info GB_red_scalar__(none)
(
    uint8_t *result,
    const GrB_Matrix A,
    GB_void *GB_RESTRICT W_space,
    bool *GB_RESTRICT F,
    int ntasks,
    int nthreads
) ;

#endif

GrB_Info GB_red_build__first_uint8
(
    uint8_t *GB_RESTRICT Tx,
    int64_t  *GB_RESTRICT Ti,
    const uint8_t *GB_RESTRICT S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *GB_RESTRICT I_work,
    const int64_t *GB_RESTRICT K_work,
    const int64_t *GB_RESTRICT tstart_slice,
    const int64_t *GB_RESTRICT tnz_slice,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0
#if 0

GrB_Info GB_red_scalar__(none)
(
    uint16_t *result,
    const GrB_Matrix A,
    GB_void *GB_RESTRICT W_space,
    bool *GB_RESTRICT F,
    int ntasks,
    int nthreads
) ;

#endif

GrB_Info GB_red_build__first_uint16
(
    uint16_t *GB_RESTRICT Tx,
    int64_t  *GB_RESTRICT Ti,
    const uint16_t *GB_RESTRICT S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *GB_RESTRICT I_work,
    const int64_t *GB_RESTRICT K_work,
    const int64_t *GB_RESTRICT tstart_slice,
    const int64_t *GB_RESTRICT tnz_slice,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0
#if 0

GrB_Info GB_red_scalar__(none)
(
    uint32_t *result,
    const GrB_Matrix A,
    GB_void *GB_RESTRICT W_space,
    bool *GB_RESTRICT F,
    int ntasks,
    int nthreads
) ;

#endif

GrB_Info GB_red_build__first_uint32
(
    uint32_t *GB_RESTRICT Tx,
    int64_t  *GB_RESTRICT Ti,
    const uint32_t *GB_RESTRICT S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *GB_RESTRICT I_work,
    const int64_t *GB_RESTRICT K_work,
    const int64_t *GB_RESTRICT tstart_slice,
    const int64_t *GB_RESTRICT tnz_slice,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0
#if 0

GrB_Info GB_red_scalar__(none)
(
    uint64_t *result,
    const GrB_Matrix A,
    GB_void *GB_RESTRICT W_space,
    bool *GB_RESTRICT F,
    int ntasks,
    int nthreads
) ;

#endif

GrB_Info GB_red_build__first_uint64
(
    uint64_t *GB_RESTRICT Tx,
    int64_t  *GB_RESTRICT Ti,
    const uint64_t *GB_RESTRICT S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *GB_RESTRICT I_work,
    const int64_t *GB_RESTRICT K_work,
    const int64_t *GB_RESTRICT tstart_slice,
    const int64_t *GB_RESTRICT tnz_slice,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0
#if 0

GrB_Info GB_red_scalar__(none)
(
    float *result,
    const GrB_Matrix A,
    GB_void *GB_RESTRICT W_space,
    bool *GB_RESTRICT F,
    int ntasks,
    int nthreads
) ;

#endif

GrB_Info GB_red_build__first_fp32
(
    float *GB_RESTRICT Tx,
    int64_t  *GB_RESTRICT Ti,
    const float *GB_RESTRICT S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *GB_RESTRICT I_work,
    const int64_t *GB_RESTRICT K_work,
    const int64_t *GB_RESTRICT tstart_slice,
    const int64_t *GB_RESTRICT tnz_slice,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0
#if 0

GrB_Info GB_red_scalar__(none)
(
    double *result,
    const GrB_Matrix A,
    GB_void *GB_RESTRICT W_space,
    bool *GB_RESTRICT F,
    int ntasks,
    int nthreads
) ;

#endif

GrB_Info GB_red_build__first_fp64
(
    double *GB_RESTRICT Tx,
    int64_t  *GB_RESTRICT Ti,
    const double *GB_RESTRICT S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *GB_RESTRICT I_work,
    const int64_t *GB_RESTRICT K_work,
    const int64_t *GB_RESTRICT tstart_slice,
    const int64_t *GB_RESTRICT tnz_slice,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0
#if 0

GrB_Info GB_red_scalar__(none)
(
    GxB_FC32_t *result,
    const GrB_Matrix A,
    GB_void *GB_RESTRICT W_space,
    bool *GB_RESTRICT F,
    int ntasks,
    int nthreads
) ;

#endif

GrB_Info GB_red_build__first_fc32
(
    GxB_FC32_t *GB_RESTRICT Tx,
    int64_t  *GB_RESTRICT Ti,
    const GxB_FC32_t *GB_RESTRICT S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *GB_RESTRICT I_work,
    const int64_t *GB_RESTRICT K_work,
    const int64_t *GB_RESTRICT tstart_slice,
    const int64_t *GB_RESTRICT tnz_slice,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0
#if 0

GrB_Info GB_red_scalar__(none)
(
    GxB_FC64_t *result,
    const GrB_Matrix A,
    GB_void *GB_RESTRICT W_space,
    bool *GB_RESTRICT F,
    int ntasks,
    int nthreads
) ;

#endif

GrB_Info GB_red_build__first_fc64
(
    GxB_FC64_t *GB_RESTRICT Tx,
    int64_t  *GB_RESTRICT Ti,
    const GxB_FC64_t *GB_RESTRICT S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *GB_RESTRICT I_work,
    const int64_t *GB_RESTRICT K_work,
    const int64_t *GB_RESTRICT tstart_slice,
    const int64_t *GB_RESTRICT tnz_slice,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0
#if 0

GrB_Info GB_red_scalar__(none)
(
    bool *result,
    const GrB_Matrix A,
    GB_void *GB_RESTRICT W_space,
    bool *GB_RESTRICT F,
    int ntasks,
    int nthreads
) ;

#endif

GrB_Info GB_red_build__second_bool
(
    bool *GB_RESTRICT Tx,
    int64_t  *GB_RESTRICT Ti,
    const bool *GB_RESTRICT S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *GB_RESTRICT I_work,
    const int64_t *GB_RESTRICT K_work,
    const int64_t *GB_RESTRICT tstart_slice,
    const int64_t *GB_RESTRICT tnz_slice,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0
#if 0

GrB_Info GB_red_scalar__(none)
(
    int8_t *result,
    const GrB_Matrix A,
    GB_void *GB_RESTRICT W_space,
    bool *GB_RESTRICT F,
    int ntasks,
    int nthreads
) ;

#endif

GrB_Info GB_red_build__second_int8
(
    int8_t *GB_RESTRICT Tx,
    int64_t  *GB_RESTRICT Ti,
    const int8_t *GB_RESTRICT S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *GB_RESTRICT I_work,
    const int64_t *GB_RESTRICT K_work,
    const int64_t *GB_RESTRICT tstart_slice,
    const int64_t *GB_RESTRICT tnz_slice,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0
#if 0

GrB_Info GB_red_scalar__(none)
(
    int16_t *result,
    const GrB_Matrix A,
    GB_void *GB_RESTRICT W_space,
    bool *GB_RESTRICT F,
    int ntasks,
    int nthreads
) ;

#endif

GrB_Info GB_red_build__second_int16
(
    int16_t *GB_RESTRICT Tx,
    int64_t  *GB_RESTRICT Ti,
    const int16_t *GB_RESTRICT S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *GB_RESTRICT I_work,
    const int64_t *GB_RESTRICT K_work,
    const int64_t *GB_RESTRICT tstart_slice,
    const int64_t *GB_RESTRICT tnz_slice,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0
#if 0

GrB_Info GB_red_scalar__(none)
(
    int32_t *result,
    const GrB_Matrix A,
    GB_void *GB_RESTRICT W_space,
    bool *GB_RESTRICT F,
    int ntasks,
    int nthreads
) ;

#endif

GrB_Info GB_red_build__second_int32
(
    int32_t *GB_RESTRICT Tx,
    int64_t  *GB_RESTRICT Ti,
    const int32_t *GB_RESTRICT S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *GB_RESTRICT I_work,
    const int64_t *GB_RESTRICT K_work,
    const int64_t *GB_RESTRICT tstart_slice,
    const int64_t *GB_RESTRICT tnz_slice,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0
#if 0

GrB_Info GB_red_scalar__(none)
(
    int64_t *result,
    const GrB_Matrix A,
    GB_void *GB_RESTRICT W_space,
    bool *GB_RESTRICT F,
    int ntasks,
    int nthreads
) ;

#endif

GrB_Info GB_red_build__second_int64
(
    int64_t *GB_RESTRICT Tx,
    int64_t  *GB_RESTRICT Ti,
    const int64_t *GB_RESTRICT S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *GB_RESTRICT I_work,
    const int64_t *GB_RESTRICT K_work,
    const int64_t *GB_RESTRICT tstart_slice,
    const int64_t *GB_RESTRICT tnz_slice,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0
#if 0

GrB_Info GB_red_scalar__(none)
(
    uint8_t *result,
    const GrB_Matrix A,
    GB_void *GB_RESTRICT W_space,
    bool *GB_RESTRICT F,
    int ntasks,
    int nthreads
) ;

#endif

GrB_Info GB_red_build__second_uint8
(
    uint8_t *GB_RESTRICT Tx,
    int64_t  *GB_RESTRICT Ti,
    const uint8_t *GB_RESTRICT S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *GB_RESTRICT I_work,
    const int64_t *GB_RESTRICT K_work,
    const int64_t *GB_RESTRICT tstart_slice,
    const int64_t *GB_RESTRICT tnz_slice,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0
#if 0

GrB_Info GB_red_scalar__(none)
(
    uint16_t *result,
    const GrB_Matrix A,
    GB_void *GB_RESTRICT W_space,
    bool *GB_RESTRICT F,
    int ntasks,
    int nthreads
) ;

#endif

GrB_Info GB_red_build__second_uint16
(
    uint16_t *GB_RESTRICT Tx,
    int64_t  *GB_RESTRICT Ti,
    const uint16_t *GB_RESTRICT S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *GB_RESTRICT I_work,
    const int64_t *GB_RESTRICT K_work,
    const int64_t *GB_RESTRICT tstart_slice,
    const int64_t *GB_RESTRICT tnz_slice,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0
#if 0

GrB_Info GB_red_scalar__(none)
(
    uint32_t *result,
    const GrB_Matrix A,
    GB_void *GB_RESTRICT W_space,
    bool *GB_RESTRICT F,
    int ntasks,
    int nthreads
) ;

#endif

GrB_Info GB_red_build__second_uint32
(
    uint32_t *GB_RESTRICT Tx,
    int64_t  *GB_RESTRICT Ti,
    const uint32_t *GB_RESTRICT S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *GB_RESTRICT I_work,
    const int64_t *GB_RESTRICT K_work,
    const int64_t *GB_RESTRICT tstart_slice,
    const int64_t *GB_RESTRICT tnz_slice,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0
#if 0

GrB_Info GB_red_scalar__(none)
(
    uint64_t *result,
    const GrB_Matrix A,
    GB_void *GB_RESTRICT W_space,
    bool *GB_RESTRICT F,
    int ntasks,
    int nthreads
) ;

#endif

GrB_Info GB_red_build__second_uint64
(
    uint64_t *GB_RESTRICT Tx,
    int64_t  *GB_RESTRICT Ti,
    const uint64_t *GB_RESTRICT S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *GB_RESTRICT I_work,
    const int64_t *GB_RESTRICT K_work,
    const int64_t *GB_RESTRICT tstart_slice,
    const int64_t *GB_RESTRICT tnz_slice,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0
#if 0

GrB_Info GB_red_scalar__(none)
(
    float *result,
    const GrB_Matrix A,
    GB_void *GB_RESTRICT W_space,
    bool *GB_RESTRICT F,
    int ntasks,
    int nthreads
) ;

#endif

GrB_Info GB_red_build__second_fp32
(
    float *GB_RESTRICT Tx,
    int64_t  *GB_RESTRICT Ti,
    const float *GB_RESTRICT S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *GB_RESTRICT I_work,
    const int64_t *GB_RESTRICT K_work,
    const int64_t *GB_RESTRICT tstart_slice,
    const int64_t *GB_RESTRICT tnz_slice,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0
#if 0

GrB_Info GB_red_scalar__(none)
(
    double *result,
    const GrB_Matrix A,
    GB_void *GB_RESTRICT W_space,
    bool *GB_RESTRICT F,
    int ntasks,
    int nthreads
) ;

#endif

GrB_Info GB_red_build__second_fp64
(
    double *GB_RESTRICT Tx,
    int64_t  *GB_RESTRICT Ti,
    const double *GB_RESTRICT S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *GB_RESTRICT I_work,
    const int64_t *GB_RESTRICT K_work,
    const int64_t *GB_RESTRICT tstart_slice,
    const int64_t *GB_RESTRICT tnz_slice,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0
#if 0

GrB_Info GB_red_scalar__(none)
(
    GxB_FC32_t *result,
    const GrB_Matrix A,
    GB_void *GB_RESTRICT W_space,
    bool *GB_RESTRICT F,
    int ntasks,
    int nthreads
) ;

#endif

GrB_Info GB_red_build__second_fc32
(
    GxB_FC32_t *GB_RESTRICT Tx,
    int64_t  *GB_RESTRICT Ti,
    const GxB_FC32_t *GB_RESTRICT S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *GB_RESTRICT I_work,
    const int64_t *GB_RESTRICT K_work,
    const int64_t *GB_RESTRICT tstart_slice,
    const int64_t *GB_RESTRICT tnz_slice,
    int nthreads
) ;

// SPDX-License-Identifier: Apache-2.0
#if 0

GrB_Info GB_red_scalar__(none)
(
    GxB_FC64_t *result,
    const GrB_Matrix A,
    GB_void *GB_RESTRICT W_space,
    bool *GB_RESTRICT F,
    int ntasks,
    int nthreads
) ;

#endif

GrB_Info GB_red_build__second_fc64
(
    GxB_FC64_t *GB_RESTRICT Tx,
    int64_t  *GB_RESTRICT Ti,
    const GxB_FC64_t *GB_RESTRICT S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *GB_RESTRICT I_work,
    const int64_t *GB_RESTRICT K_work,
    const int64_t *GB_RESTRICT tstart_slice,
    const int64_t *GB_RESTRICT tnz_slice,
    int nthreads
) ;

