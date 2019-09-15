//------------------------------------------------------------------------------
// GB_unaryop__include.h: definitions for GB_unaryop__*.c
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txargt for license.

// This file has been automatically generated from Generator/GB_unaryop.h

GrB_Info GB_unop__one_bool_bool
(
    bool *restrict Cx,
    const bool *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__one_bool_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__one_int8_int8
(
    int8_t *restrict Cx,
    const int8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__one_int8_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__one_int16_int16
(
    int16_t *restrict Cx,
    const int16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__one_int16_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__one_int32_int32
(
    int32_t *restrict Cx,
    const int32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__one_int32_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__one_int64_int64
(
    int64_t *restrict Cx,
    const int64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__one_int64_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__one_uint8_uint8
(
    uint8_t *restrict Cx,
    const uint8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__one_uint8_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__one_uint16_uint16
(
    uint16_t *restrict Cx,
    const uint16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__one_uint16_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__one_uint32_uint32
(
    uint32_t *restrict Cx,
    const uint32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__one_uint32_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__one_uint64_uint64
(
    uint64_t *restrict Cx,
    const uint64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__one_uint64_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__one_fp32_fp32
(
    float *restrict Cx,
    const float *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__one_fp32_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__one_fp64_fp64
(
    double *restrict Cx,
    const double *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__one_fp64_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_bool_bool
(
    bool *restrict Cx,
    const bool *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_bool_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_bool_int8
(
    bool *restrict Cx,
    const int8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_bool_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_bool_int16
(
    bool *restrict Cx,
    const int16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_bool_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_bool_int32
(
    bool *restrict Cx,
    const int32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_bool_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_bool_int64
(
    bool *restrict Cx,
    const int64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_bool_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_bool_uint8
(
    bool *restrict Cx,
    const uint8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_bool_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_bool_uint16
(
    bool *restrict Cx,
    const uint16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_bool_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_bool_uint32
(
    bool *restrict Cx,
    const uint32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_bool_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_bool_uint64
(
    bool *restrict Cx,
    const uint64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_bool_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_bool_fp32
(
    bool *restrict Cx,
    const float *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_bool_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_bool_fp64
(
    bool *restrict Cx,
    const double *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_bool_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_int8_bool
(
    int8_t *restrict Cx,
    const bool *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_int8_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_int8_int8
(
    int8_t *restrict Cx,
    const int8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_int8_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_int8_int16
(
    int8_t *restrict Cx,
    const int16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_int8_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_int8_int32
(
    int8_t *restrict Cx,
    const int32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_int8_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_int8_int64
(
    int8_t *restrict Cx,
    const int64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_int8_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_int8_uint8
(
    int8_t *restrict Cx,
    const uint8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_int8_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_int8_uint16
(
    int8_t *restrict Cx,
    const uint16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_int8_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_int8_uint32
(
    int8_t *restrict Cx,
    const uint32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_int8_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_int8_uint64
(
    int8_t *restrict Cx,
    const uint64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_int8_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_int8_fp32
(
    int8_t *restrict Cx,
    const float *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_int8_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_int8_fp64
(
    int8_t *restrict Cx,
    const double *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_int8_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_int16_bool
(
    int16_t *restrict Cx,
    const bool *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_int16_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_int16_int8
(
    int16_t *restrict Cx,
    const int8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_int16_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_int16_int16
(
    int16_t *restrict Cx,
    const int16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_int16_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_int16_int32
(
    int16_t *restrict Cx,
    const int32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_int16_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_int16_int64
(
    int16_t *restrict Cx,
    const int64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_int16_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_int16_uint8
(
    int16_t *restrict Cx,
    const uint8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_int16_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_int16_uint16
(
    int16_t *restrict Cx,
    const uint16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_int16_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_int16_uint32
(
    int16_t *restrict Cx,
    const uint32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_int16_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_int16_uint64
(
    int16_t *restrict Cx,
    const uint64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_int16_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_int16_fp32
(
    int16_t *restrict Cx,
    const float *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_int16_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_int16_fp64
(
    int16_t *restrict Cx,
    const double *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_int16_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_int32_bool
(
    int32_t *restrict Cx,
    const bool *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_int32_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_int32_int8
(
    int32_t *restrict Cx,
    const int8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_int32_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_int32_int16
(
    int32_t *restrict Cx,
    const int16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_int32_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_int32_int32
(
    int32_t *restrict Cx,
    const int32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_int32_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_int32_int64
(
    int32_t *restrict Cx,
    const int64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_int32_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_int32_uint8
(
    int32_t *restrict Cx,
    const uint8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_int32_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_int32_uint16
(
    int32_t *restrict Cx,
    const uint16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_int32_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_int32_uint32
(
    int32_t *restrict Cx,
    const uint32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_int32_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_int32_uint64
(
    int32_t *restrict Cx,
    const uint64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_int32_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_int32_fp32
(
    int32_t *restrict Cx,
    const float *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_int32_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_int32_fp64
(
    int32_t *restrict Cx,
    const double *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_int32_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_int64_bool
(
    int64_t *restrict Cx,
    const bool *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_int64_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_int64_int8
(
    int64_t *restrict Cx,
    const int8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_int64_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_int64_int16
(
    int64_t *restrict Cx,
    const int16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_int64_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_int64_int32
(
    int64_t *restrict Cx,
    const int32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_int64_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_int64_int64
(
    int64_t *restrict Cx,
    const int64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_int64_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_int64_uint8
(
    int64_t *restrict Cx,
    const uint8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_int64_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_int64_uint16
(
    int64_t *restrict Cx,
    const uint16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_int64_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_int64_uint32
(
    int64_t *restrict Cx,
    const uint32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_int64_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_int64_uint64
(
    int64_t *restrict Cx,
    const uint64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_int64_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_int64_fp32
(
    int64_t *restrict Cx,
    const float *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_int64_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_int64_fp64
(
    int64_t *restrict Cx,
    const double *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_int64_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_uint8_bool
(
    uint8_t *restrict Cx,
    const bool *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_uint8_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_uint8_int8
(
    uint8_t *restrict Cx,
    const int8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_uint8_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_uint8_int16
(
    uint8_t *restrict Cx,
    const int16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_uint8_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_uint8_int32
(
    uint8_t *restrict Cx,
    const int32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_uint8_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_uint8_int64
(
    uint8_t *restrict Cx,
    const int64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_uint8_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_uint8_uint8
(
    uint8_t *restrict Cx,
    const uint8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_uint8_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_uint8_uint16
(
    uint8_t *restrict Cx,
    const uint16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_uint8_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_uint8_uint32
(
    uint8_t *restrict Cx,
    const uint32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_uint8_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_uint8_uint64
(
    uint8_t *restrict Cx,
    const uint64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_uint8_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_uint8_fp32
(
    uint8_t *restrict Cx,
    const float *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_uint8_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_uint8_fp64
(
    uint8_t *restrict Cx,
    const double *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_uint8_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_uint16_bool
(
    uint16_t *restrict Cx,
    const bool *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_uint16_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_uint16_int8
(
    uint16_t *restrict Cx,
    const int8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_uint16_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_uint16_int16
(
    uint16_t *restrict Cx,
    const int16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_uint16_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_uint16_int32
(
    uint16_t *restrict Cx,
    const int32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_uint16_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_uint16_int64
(
    uint16_t *restrict Cx,
    const int64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_uint16_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_uint16_uint8
(
    uint16_t *restrict Cx,
    const uint8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_uint16_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_uint16_uint16
(
    uint16_t *restrict Cx,
    const uint16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_uint16_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_uint16_uint32
(
    uint16_t *restrict Cx,
    const uint32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_uint16_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_uint16_uint64
(
    uint16_t *restrict Cx,
    const uint64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_uint16_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_uint16_fp32
(
    uint16_t *restrict Cx,
    const float *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_uint16_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_uint16_fp64
(
    uint16_t *restrict Cx,
    const double *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_uint16_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_uint32_bool
(
    uint32_t *restrict Cx,
    const bool *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_uint32_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_uint32_int8
(
    uint32_t *restrict Cx,
    const int8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_uint32_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_uint32_int16
(
    uint32_t *restrict Cx,
    const int16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_uint32_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_uint32_int32
(
    uint32_t *restrict Cx,
    const int32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_uint32_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_uint32_int64
(
    uint32_t *restrict Cx,
    const int64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_uint32_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_uint32_uint8
(
    uint32_t *restrict Cx,
    const uint8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_uint32_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_uint32_uint16
(
    uint32_t *restrict Cx,
    const uint16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_uint32_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_uint32_uint32
(
    uint32_t *restrict Cx,
    const uint32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_uint32_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_uint32_uint64
(
    uint32_t *restrict Cx,
    const uint64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_uint32_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_uint32_fp32
(
    uint32_t *restrict Cx,
    const float *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_uint32_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_uint32_fp64
(
    uint32_t *restrict Cx,
    const double *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_uint32_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_uint64_bool
(
    uint64_t *restrict Cx,
    const bool *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_uint64_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_uint64_int8
(
    uint64_t *restrict Cx,
    const int8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_uint64_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_uint64_int16
(
    uint64_t *restrict Cx,
    const int16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_uint64_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_uint64_int32
(
    uint64_t *restrict Cx,
    const int32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_uint64_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_uint64_int64
(
    uint64_t *restrict Cx,
    const int64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_uint64_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_uint64_uint8
(
    uint64_t *restrict Cx,
    const uint8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_uint64_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_uint64_uint16
(
    uint64_t *restrict Cx,
    const uint16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_uint64_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_uint64_uint32
(
    uint64_t *restrict Cx,
    const uint32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_uint64_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_uint64_uint64
(
    uint64_t *restrict Cx,
    const uint64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_uint64_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_uint64_fp32
(
    uint64_t *restrict Cx,
    const float *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_uint64_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_uint64_fp64
(
    uint64_t *restrict Cx,
    const double *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_uint64_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_fp32_bool
(
    float *restrict Cx,
    const bool *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_fp32_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_fp32_int8
(
    float *restrict Cx,
    const int8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_fp32_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_fp32_int16
(
    float *restrict Cx,
    const int16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_fp32_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_fp32_int32
(
    float *restrict Cx,
    const int32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_fp32_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_fp32_int64
(
    float *restrict Cx,
    const int64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_fp32_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_fp32_uint8
(
    float *restrict Cx,
    const uint8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_fp32_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_fp32_uint16
(
    float *restrict Cx,
    const uint16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_fp32_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_fp32_uint32
(
    float *restrict Cx,
    const uint32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_fp32_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_fp32_uint64
(
    float *restrict Cx,
    const uint64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_fp32_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_fp32_fp32
(
    float *restrict Cx,
    const float *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_fp32_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_fp32_fp64
(
    float *restrict Cx,
    const double *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_fp32_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_fp64_bool
(
    double *restrict Cx,
    const bool *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_fp64_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_fp64_int8
(
    double *restrict Cx,
    const int8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_fp64_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_fp64_int16
(
    double *restrict Cx,
    const int16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_fp64_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_fp64_int32
(
    double *restrict Cx,
    const int32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_fp64_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_fp64_int64
(
    double *restrict Cx,
    const int64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_fp64_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_fp64_uint8
(
    double *restrict Cx,
    const uint8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_fp64_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_fp64_uint16
(
    double *restrict Cx,
    const uint16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_fp64_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_fp64_uint32
(
    double *restrict Cx,
    const uint32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_fp64_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_fp64_uint64
(
    double *restrict Cx,
    const uint64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_fp64_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_fp64_fp32
(
    double *restrict Cx,
    const float *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_fp64_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_fp64_fp64
(
    double *restrict Cx,
    const double *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_fp64_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_bool_bool
(
    bool *restrict Cx,
    const bool *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_bool_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_bool_int8
(
    bool *restrict Cx,
    const int8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_bool_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_bool_int16
(
    bool *restrict Cx,
    const int16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_bool_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_bool_int32
(
    bool *restrict Cx,
    const int32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_bool_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_bool_int64
(
    bool *restrict Cx,
    const int64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_bool_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_bool_uint8
(
    bool *restrict Cx,
    const uint8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_bool_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_bool_uint16
(
    bool *restrict Cx,
    const uint16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_bool_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_bool_uint32
(
    bool *restrict Cx,
    const uint32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_bool_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_bool_uint64
(
    bool *restrict Cx,
    const uint64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_bool_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_bool_fp32
(
    bool *restrict Cx,
    const float *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_bool_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_bool_fp64
(
    bool *restrict Cx,
    const double *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_bool_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_int8_bool
(
    int8_t *restrict Cx,
    const bool *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_int8_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_int8_int8
(
    int8_t *restrict Cx,
    const int8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_int8_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_int8_int16
(
    int8_t *restrict Cx,
    const int16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_int8_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_int8_int32
(
    int8_t *restrict Cx,
    const int32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_int8_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_int8_int64
(
    int8_t *restrict Cx,
    const int64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_int8_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_int8_uint8
(
    int8_t *restrict Cx,
    const uint8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_int8_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_int8_uint16
(
    int8_t *restrict Cx,
    const uint16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_int8_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_int8_uint32
(
    int8_t *restrict Cx,
    const uint32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_int8_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_int8_uint64
(
    int8_t *restrict Cx,
    const uint64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_int8_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_int8_fp32
(
    int8_t *restrict Cx,
    const float *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_int8_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_int8_fp64
(
    int8_t *restrict Cx,
    const double *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_int8_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_int16_bool
(
    int16_t *restrict Cx,
    const bool *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_int16_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_int16_int8
(
    int16_t *restrict Cx,
    const int8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_int16_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_int16_int16
(
    int16_t *restrict Cx,
    const int16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_int16_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_int16_int32
(
    int16_t *restrict Cx,
    const int32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_int16_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_int16_int64
(
    int16_t *restrict Cx,
    const int64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_int16_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_int16_uint8
(
    int16_t *restrict Cx,
    const uint8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_int16_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_int16_uint16
(
    int16_t *restrict Cx,
    const uint16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_int16_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_int16_uint32
(
    int16_t *restrict Cx,
    const uint32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_int16_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_int16_uint64
(
    int16_t *restrict Cx,
    const uint64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_int16_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_int16_fp32
(
    int16_t *restrict Cx,
    const float *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_int16_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_int16_fp64
(
    int16_t *restrict Cx,
    const double *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_int16_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_int32_bool
(
    int32_t *restrict Cx,
    const bool *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_int32_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_int32_int8
(
    int32_t *restrict Cx,
    const int8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_int32_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_int32_int16
(
    int32_t *restrict Cx,
    const int16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_int32_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_int32_int32
(
    int32_t *restrict Cx,
    const int32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_int32_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_int32_int64
(
    int32_t *restrict Cx,
    const int64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_int32_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_int32_uint8
(
    int32_t *restrict Cx,
    const uint8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_int32_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_int32_uint16
(
    int32_t *restrict Cx,
    const uint16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_int32_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_int32_uint32
(
    int32_t *restrict Cx,
    const uint32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_int32_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_int32_uint64
(
    int32_t *restrict Cx,
    const uint64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_int32_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_int32_fp32
(
    int32_t *restrict Cx,
    const float *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_int32_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_int32_fp64
(
    int32_t *restrict Cx,
    const double *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_int32_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_int64_bool
(
    int64_t *restrict Cx,
    const bool *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_int64_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_int64_int8
(
    int64_t *restrict Cx,
    const int8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_int64_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_int64_int16
(
    int64_t *restrict Cx,
    const int16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_int64_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_int64_int32
(
    int64_t *restrict Cx,
    const int32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_int64_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_int64_int64
(
    int64_t *restrict Cx,
    const int64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_int64_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_int64_uint8
(
    int64_t *restrict Cx,
    const uint8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_int64_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_int64_uint16
(
    int64_t *restrict Cx,
    const uint16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_int64_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_int64_uint32
(
    int64_t *restrict Cx,
    const uint32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_int64_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_int64_uint64
(
    int64_t *restrict Cx,
    const uint64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_int64_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_int64_fp32
(
    int64_t *restrict Cx,
    const float *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_int64_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_int64_fp64
(
    int64_t *restrict Cx,
    const double *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_int64_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_uint8_bool
(
    uint8_t *restrict Cx,
    const bool *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_uint8_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_uint8_int8
(
    uint8_t *restrict Cx,
    const int8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_uint8_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_uint8_int16
(
    uint8_t *restrict Cx,
    const int16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_uint8_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_uint8_int32
(
    uint8_t *restrict Cx,
    const int32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_uint8_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_uint8_int64
(
    uint8_t *restrict Cx,
    const int64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_uint8_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_uint8_uint8
(
    uint8_t *restrict Cx,
    const uint8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_uint8_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_uint8_uint16
(
    uint8_t *restrict Cx,
    const uint16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_uint8_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_uint8_uint32
(
    uint8_t *restrict Cx,
    const uint32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_uint8_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_uint8_uint64
(
    uint8_t *restrict Cx,
    const uint64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_uint8_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_uint8_fp32
(
    uint8_t *restrict Cx,
    const float *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_uint8_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_uint8_fp64
(
    uint8_t *restrict Cx,
    const double *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_uint8_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_uint16_bool
(
    uint16_t *restrict Cx,
    const bool *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_uint16_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_uint16_int8
(
    uint16_t *restrict Cx,
    const int8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_uint16_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_uint16_int16
(
    uint16_t *restrict Cx,
    const int16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_uint16_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_uint16_int32
(
    uint16_t *restrict Cx,
    const int32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_uint16_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_uint16_int64
(
    uint16_t *restrict Cx,
    const int64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_uint16_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_uint16_uint8
(
    uint16_t *restrict Cx,
    const uint8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_uint16_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_uint16_uint16
(
    uint16_t *restrict Cx,
    const uint16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_uint16_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_uint16_uint32
(
    uint16_t *restrict Cx,
    const uint32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_uint16_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_uint16_uint64
(
    uint16_t *restrict Cx,
    const uint64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_uint16_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_uint16_fp32
(
    uint16_t *restrict Cx,
    const float *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_uint16_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_uint16_fp64
(
    uint16_t *restrict Cx,
    const double *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_uint16_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_uint32_bool
(
    uint32_t *restrict Cx,
    const bool *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_uint32_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_uint32_int8
(
    uint32_t *restrict Cx,
    const int8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_uint32_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_uint32_int16
(
    uint32_t *restrict Cx,
    const int16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_uint32_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_uint32_int32
(
    uint32_t *restrict Cx,
    const int32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_uint32_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_uint32_int64
(
    uint32_t *restrict Cx,
    const int64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_uint32_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_uint32_uint8
(
    uint32_t *restrict Cx,
    const uint8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_uint32_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_uint32_uint16
(
    uint32_t *restrict Cx,
    const uint16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_uint32_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_uint32_uint32
(
    uint32_t *restrict Cx,
    const uint32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_uint32_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_uint32_uint64
(
    uint32_t *restrict Cx,
    const uint64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_uint32_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_uint32_fp32
(
    uint32_t *restrict Cx,
    const float *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_uint32_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_uint32_fp64
(
    uint32_t *restrict Cx,
    const double *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_uint32_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_uint64_bool
(
    uint64_t *restrict Cx,
    const bool *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_uint64_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_uint64_int8
(
    uint64_t *restrict Cx,
    const int8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_uint64_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_uint64_int16
(
    uint64_t *restrict Cx,
    const int16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_uint64_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_uint64_int32
(
    uint64_t *restrict Cx,
    const int32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_uint64_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_uint64_int64
(
    uint64_t *restrict Cx,
    const int64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_uint64_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_uint64_uint8
(
    uint64_t *restrict Cx,
    const uint8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_uint64_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_uint64_uint16
(
    uint64_t *restrict Cx,
    const uint16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_uint64_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_uint64_uint32
(
    uint64_t *restrict Cx,
    const uint32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_uint64_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_uint64_uint64
(
    uint64_t *restrict Cx,
    const uint64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_uint64_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_uint64_fp32
(
    uint64_t *restrict Cx,
    const float *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_uint64_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_uint64_fp64
(
    uint64_t *restrict Cx,
    const double *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_uint64_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_fp32_bool
(
    float *restrict Cx,
    const bool *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_fp32_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_fp32_int8
(
    float *restrict Cx,
    const int8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_fp32_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_fp32_int16
(
    float *restrict Cx,
    const int16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_fp32_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_fp32_int32
(
    float *restrict Cx,
    const int32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_fp32_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_fp32_int64
(
    float *restrict Cx,
    const int64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_fp32_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_fp32_uint8
(
    float *restrict Cx,
    const uint8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_fp32_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_fp32_uint16
(
    float *restrict Cx,
    const uint16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_fp32_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_fp32_uint32
(
    float *restrict Cx,
    const uint32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_fp32_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_fp32_uint64
(
    float *restrict Cx,
    const uint64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_fp32_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_fp32_fp32
(
    float *restrict Cx,
    const float *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_fp32_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_fp32_fp64
(
    float *restrict Cx,
    const double *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_fp32_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_fp64_bool
(
    double *restrict Cx,
    const bool *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_fp64_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_fp64_int8
(
    double *restrict Cx,
    const int8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_fp64_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_fp64_int16
(
    double *restrict Cx,
    const int16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_fp64_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_fp64_int32
(
    double *restrict Cx,
    const int32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_fp64_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_fp64_int64
(
    double *restrict Cx,
    const int64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_fp64_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_fp64_uint8
(
    double *restrict Cx,
    const uint8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_fp64_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_fp64_uint16
(
    double *restrict Cx,
    const uint16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_fp64_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_fp64_uint32
(
    double *restrict Cx,
    const uint32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_fp64_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_fp64_uint64
(
    double *restrict Cx,
    const uint64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_fp64_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_fp64_fp32
(
    double *restrict Cx,
    const float *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_fp64_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_fp64_fp64
(
    double *restrict Cx,
    const double *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_fp64_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_bool_bool
(
    bool *restrict Cx,
    const bool *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_bool_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_bool_int8
(
    bool *restrict Cx,
    const int8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_bool_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_bool_int16
(
    bool *restrict Cx,
    const int16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_bool_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_bool_int32
(
    bool *restrict Cx,
    const int32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_bool_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_bool_int64
(
    bool *restrict Cx,
    const int64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_bool_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_bool_uint8
(
    bool *restrict Cx,
    const uint8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_bool_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_bool_uint16
(
    bool *restrict Cx,
    const uint16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_bool_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_bool_uint32
(
    bool *restrict Cx,
    const uint32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_bool_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_bool_uint64
(
    bool *restrict Cx,
    const uint64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_bool_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_bool_fp32
(
    bool *restrict Cx,
    const float *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_bool_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_bool_fp64
(
    bool *restrict Cx,
    const double *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_bool_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_int8_bool
(
    int8_t *restrict Cx,
    const bool *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_int8_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_int8_int8
(
    int8_t *restrict Cx,
    const int8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_int8_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_int8_int16
(
    int8_t *restrict Cx,
    const int16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_int8_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_int8_int32
(
    int8_t *restrict Cx,
    const int32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_int8_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_int8_int64
(
    int8_t *restrict Cx,
    const int64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_int8_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_int8_uint8
(
    int8_t *restrict Cx,
    const uint8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_int8_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_int8_uint16
(
    int8_t *restrict Cx,
    const uint16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_int8_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_int8_uint32
(
    int8_t *restrict Cx,
    const uint32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_int8_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_int8_uint64
(
    int8_t *restrict Cx,
    const uint64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_int8_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_int8_fp32
(
    int8_t *restrict Cx,
    const float *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_int8_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_int8_fp64
(
    int8_t *restrict Cx,
    const double *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_int8_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_int16_bool
(
    int16_t *restrict Cx,
    const bool *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_int16_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_int16_int8
(
    int16_t *restrict Cx,
    const int8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_int16_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_int16_int16
(
    int16_t *restrict Cx,
    const int16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_int16_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_int16_int32
(
    int16_t *restrict Cx,
    const int32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_int16_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_int16_int64
(
    int16_t *restrict Cx,
    const int64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_int16_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_int16_uint8
(
    int16_t *restrict Cx,
    const uint8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_int16_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_int16_uint16
(
    int16_t *restrict Cx,
    const uint16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_int16_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_int16_uint32
(
    int16_t *restrict Cx,
    const uint32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_int16_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_int16_uint64
(
    int16_t *restrict Cx,
    const uint64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_int16_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_int16_fp32
(
    int16_t *restrict Cx,
    const float *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_int16_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_int16_fp64
(
    int16_t *restrict Cx,
    const double *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_int16_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_int32_bool
(
    int32_t *restrict Cx,
    const bool *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_int32_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_int32_int8
(
    int32_t *restrict Cx,
    const int8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_int32_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_int32_int16
(
    int32_t *restrict Cx,
    const int16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_int32_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_int32_int32
(
    int32_t *restrict Cx,
    const int32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_int32_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_int32_int64
(
    int32_t *restrict Cx,
    const int64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_int32_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_int32_uint8
(
    int32_t *restrict Cx,
    const uint8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_int32_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_int32_uint16
(
    int32_t *restrict Cx,
    const uint16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_int32_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_int32_uint32
(
    int32_t *restrict Cx,
    const uint32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_int32_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_int32_uint64
(
    int32_t *restrict Cx,
    const uint64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_int32_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_int32_fp32
(
    int32_t *restrict Cx,
    const float *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_int32_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_int32_fp64
(
    int32_t *restrict Cx,
    const double *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_int32_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_int64_bool
(
    int64_t *restrict Cx,
    const bool *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_int64_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_int64_int8
(
    int64_t *restrict Cx,
    const int8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_int64_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_int64_int16
(
    int64_t *restrict Cx,
    const int16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_int64_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_int64_int32
(
    int64_t *restrict Cx,
    const int32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_int64_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_int64_int64
(
    int64_t *restrict Cx,
    const int64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_int64_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_int64_uint8
(
    int64_t *restrict Cx,
    const uint8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_int64_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_int64_uint16
(
    int64_t *restrict Cx,
    const uint16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_int64_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_int64_uint32
(
    int64_t *restrict Cx,
    const uint32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_int64_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_int64_uint64
(
    int64_t *restrict Cx,
    const uint64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_int64_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_int64_fp32
(
    int64_t *restrict Cx,
    const float *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_int64_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_int64_fp64
(
    int64_t *restrict Cx,
    const double *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_int64_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_uint8_bool
(
    uint8_t *restrict Cx,
    const bool *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_uint8_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_uint8_int8
(
    uint8_t *restrict Cx,
    const int8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_uint8_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_uint8_int16
(
    uint8_t *restrict Cx,
    const int16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_uint8_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_uint8_int32
(
    uint8_t *restrict Cx,
    const int32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_uint8_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_uint8_int64
(
    uint8_t *restrict Cx,
    const int64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_uint8_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_uint8_uint8
(
    uint8_t *restrict Cx,
    const uint8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_uint8_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_uint8_uint16
(
    uint8_t *restrict Cx,
    const uint16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_uint8_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_uint8_uint32
(
    uint8_t *restrict Cx,
    const uint32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_uint8_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_uint8_uint64
(
    uint8_t *restrict Cx,
    const uint64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_uint8_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_uint8_fp32
(
    uint8_t *restrict Cx,
    const float *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_uint8_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_uint8_fp64
(
    uint8_t *restrict Cx,
    const double *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_uint8_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_uint16_bool
(
    uint16_t *restrict Cx,
    const bool *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_uint16_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_uint16_int8
(
    uint16_t *restrict Cx,
    const int8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_uint16_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_uint16_int16
(
    uint16_t *restrict Cx,
    const int16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_uint16_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_uint16_int32
(
    uint16_t *restrict Cx,
    const int32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_uint16_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_uint16_int64
(
    uint16_t *restrict Cx,
    const int64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_uint16_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_uint16_uint8
(
    uint16_t *restrict Cx,
    const uint8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_uint16_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_uint16_uint16
(
    uint16_t *restrict Cx,
    const uint16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_uint16_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_uint16_uint32
(
    uint16_t *restrict Cx,
    const uint32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_uint16_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_uint16_uint64
(
    uint16_t *restrict Cx,
    const uint64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_uint16_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_uint16_fp32
(
    uint16_t *restrict Cx,
    const float *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_uint16_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_uint16_fp64
(
    uint16_t *restrict Cx,
    const double *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_uint16_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_uint32_bool
(
    uint32_t *restrict Cx,
    const bool *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_uint32_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_uint32_int8
(
    uint32_t *restrict Cx,
    const int8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_uint32_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_uint32_int16
(
    uint32_t *restrict Cx,
    const int16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_uint32_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_uint32_int32
(
    uint32_t *restrict Cx,
    const int32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_uint32_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_uint32_int64
(
    uint32_t *restrict Cx,
    const int64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_uint32_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_uint32_uint8
(
    uint32_t *restrict Cx,
    const uint8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_uint32_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_uint32_uint16
(
    uint32_t *restrict Cx,
    const uint16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_uint32_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_uint32_uint32
(
    uint32_t *restrict Cx,
    const uint32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_uint32_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_uint32_uint64
(
    uint32_t *restrict Cx,
    const uint64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_uint32_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_uint32_fp32
(
    uint32_t *restrict Cx,
    const float *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_uint32_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_uint32_fp64
(
    uint32_t *restrict Cx,
    const double *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_uint32_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_uint64_bool
(
    uint64_t *restrict Cx,
    const bool *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_uint64_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_uint64_int8
(
    uint64_t *restrict Cx,
    const int8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_uint64_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_uint64_int16
(
    uint64_t *restrict Cx,
    const int16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_uint64_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_uint64_int32
(
    uint64_t *restrict Cx,
    const int32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_uint64_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_uint64_int64
(
    uint64_t *restrict Cx,
    const int64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_uint64_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_uint64_uint8
(
    uint64_t *restrict Cx,
    const uint8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_uint64_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_uint64_uint16
(
    uint64_t *restrict Cx,
    const uint16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_uint64_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_uint64_uint32
(
    uint64_t *restrict Cx,
    const uint32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_uint64_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_uint64_uint64
(
    uint64_t *restrict Cx,
    const uint64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_uint64_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_uint64_fp32
(
    uint64_t *restrict Cx,
    const float *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_uint64_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_uint64_fp64
(
    uint64_t *restrict Cx,
    const double *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_uint64_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_fp32_bool
(
    float *restrict Cx,
    const bool *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_fp32_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_fp32_int8
(
    float *restrict Cx,
    const int8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_fp32_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_fp32_int16
(
    float *restrict Cx,
    const int16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_fp32_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_fp32_int32
(
    float *restrict Cx,
    const int32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_fp32_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_fp32_int64
(
    float *restrict Cx,
    const int64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_fp32_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_fp32_uint8
(
    float *restrict Cx,
    const uint8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_fp32_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_fp32_uint16
(
    float *restrict Cx,
    const uint16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_fp32_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_fp32_uint32
(
    float *restrict Cx,
    const uint32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_fp32_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_fp32_uint64
(
    float *restrict Cx,
    const uint64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_fp32_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_fp32_fp32
(
    float *restrict Cx,
    const float *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_fp32_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_fp32_fp64
(
    float *restrict Cx,
    const double *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_fp32_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_fp64_bool
(
    double *restrict Cx,
    const bool *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_fp64_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_fp64_int8
(
    double *restrict Cx,
    const int8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_fp64_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_fp64_int16
(
    double *restrict Cx,
    const int16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_fp64_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_fp64_int32
(
    double *restrict Cx,
    const int32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_fp64_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_fp64_int64
(
    double *restrict Cx,
    const int64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_fp64_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_fp64_uint8
(
    double *restrict Cx,
    const uint8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_fp64_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_fp64_uint16
(
    double *restrict Cx,
    const uint16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_fp64_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_fp64_uint32
(
    double *restrict Cx,
    const uint32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_fp64_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_fp64_uint64
(
    double *restrict Cx,
    const uint64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_fp64_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_fp64_fp32
(
    double *restrict Cx,
    const float *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_fp64_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_fp64_fp64
(
    double *restrict Cx,
    const double *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_fp64_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_bool_bool
(
    bool *restrict Cx,
    const bool *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_bool_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_bool_int8
(
    bool *restrict Cx,
    const int8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_bool_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_bool_int16
(
    bool *restrict Cx,
    const int16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_bool_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_bool_int32
(
    bool *restrict Cx,
    const int32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_bool_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_bool_int64
(
    bool *restrict Cx,
    const int64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_bool_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_bool_uint8
(
    bool *restrict Cx,
    const uint8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_bool_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_bool_uint16
(
    bool *restrict Cx,
    const uint16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_bool_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_bool_uint32
(
    bool *restrict Cx,
    const uint32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_bool_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_bool_uint64
(
    bool *restrict Cx,
    const uint64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_bool_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_bool_fp32
(
    bool *restrict Cx,
    const float *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_bool_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_bool_fp64
(
    bool *restrict Cx,
    const double *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_bool_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_int8_bool
(
    int8_t *restrict Cx,
    const bool *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_int8_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_int8_int8
(
    int8_t *restrict Cx,
    const int8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_int8_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_int8_int16
(
    int8_t *restrict Cx,
    const int16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_int8_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_int8_int32
(
    int8_t *restrict Cx,
    const int32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_int8_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_int8_int64
(
    int8_t *restrict Cx,
    const int64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_int8_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_int8_uint8
(
    int8_t *restrict Cx,
    const uint8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_int8_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_int8_uint16
(
    int8_t *restrict Cx,
    const uint16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_int8_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_int8_uint32
(
    int8_t *restrict Cx,
    const uint32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_int8_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_int8_uint64
(
    int8_t *restrict Cx,
    const uint64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_int8_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_int8_fp32
(
    int8_t *restrict Cx,
    const float *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_int8_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_int8_fp64
(
    int8_t *restrict Cx,
    const double *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_int8_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_int16_bool
(
    int16_t *restrict Cx,
    const bool *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_int16_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_int16_int8
(
    int16_t *restrict Cx,
    const int8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_int16_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_int16_int16
(
    int16_t *restrict Cx,
    const int16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_int16_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_int16_int32
(
    int16_t *restrict Cx,
    const int32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_int16_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_int16_int64
(
    int16_t *restrict Cx,
    const int64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_int16_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_int16_uint8
(
    int16_t *restrict Cx,
    const uint8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_int16_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_int16_uint16
(
    int16_t *restrict Cx,
    const uint16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_int16_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_int16_uint32
(
    int16_t *restrict Cx,
    const uint32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_int16_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_int16_uint64
(
    int16_t *restrict Cx,
    const uint64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_int16_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_int16_fp32
(
    int16_t *restrict Cx,
    const float *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_int16_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_int16_fp64
(
    int16_t *restrict Cx,
    const double *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_int16_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_int32_bool
(
    int32_t *restrict Cx,
    const bool *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_int32_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_int32_int8
(
    int32_t *restrict Cx,
    const int8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_int32_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_int32_int16
(
    int32_t *restrict Cx,
    const int16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_int32_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_int32_int32
(
    int32_t *restrict Cx,
    const int32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_int32_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_int32_int64
(
    int32_t *restrict Cx,
    const int64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_int32_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_int32_uint8
(
    int32_t *restrict Cx,
    const uint8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_int32_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_int32_uint16
(
    int32_t *restrict Cx,
    const uint16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_int32_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_int32_uint32
(
    int32_t *restrict Cx,
    const uint32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_int32_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_int32_uint64
(
    int32_t *restrict Cx,
    const uint64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_int32_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_int32_fp32
(
    int32_t *restrict Cx,
    const float *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_int32_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_int32_fp64
(
    int32_t *restrict Cx,
    const double *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_int32_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_int64_bool
(
    int64_t *restrict Cx,
    const bool *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_int64_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_int64_int8
(
    int64_t *restrict Cx,
    const int8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_int64_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_int64_int16
(
    int64_t *restrict Cx,
    const int16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_int64_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_int64_int32
(
    int64_t *restrict Cx,
    const int32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_int64_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_int64_int64
(
    int64_t *restrict Cx,
    const int64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_int64_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_int64_uint8
(
    int64_t *restrict Cx,
    const uint8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_int64_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_int64_uint16
(
    int64_t *restrict Cx,
    const uint16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_int64_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_int64_uint32
(
    int64_t *restrict Cx,
    const uint32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_int64_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_int64_uint64
(
    int64_t *restrict Cx,
    const uint64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_int64_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_int64_fp32
(
    int64_t *restrict Cx,
    const float *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_int64_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_int64_fp64
(
    int64_t *restrict Cx,
    const double *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_int64_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_uint8_bool
(
    uint8_t *restrict Cx,
    const bool *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_uint8_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_uint8_int8
(
    uint8_t *restrict Cx,
    const int8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_uint8_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_uint8_int16
(
    uint8_t *restrict Cx,
    const int16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_uint8_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_uint8_int32
(
    uint8_t *restrict Cx,
    const int32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_uint8_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_uint8_int64
(
    uint8_t *restrict Cx,
    const int64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_uint8_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_uint8_uint8
(
    uint8_t *restrict Cx,
    const uint8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_uint8_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_uint8_uint16
(
    uint8_t *restrict Cx,
    const uint16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_uint8_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_uint8_uint32
(
    uint8_t *restrict Cx,
    const uint32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_uint8_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_uint8_uint64
(
    uint8_t *restrict Cx,
    const uint64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_uint8_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_uint8_fp32
(
    uint8_t *restrict Cx,
    const float *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_uint8_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_uint8_fp64
(
    uint8_t *restrict Cx,
    const double *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_uint8_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_uint16_bool
(
    uint16_t *restrict Cx,
    const bool *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_uint16_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_uint16_int8
(
    uint16_t *restrict Cx,
    const int8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_uint16_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_uint16_int16
(
    uint16_t *restrict Cx,
    const int16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_uint16_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_uint16_int32
(
    uint16_t *restrict Cx,
    const int32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_uint16_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_uint16_int64
(
    uint16_t *restrict Cx,
    const int64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_uint16_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_uint16_uint8
(
    uint16_t *restrict Cx,
    const uint8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_uint16_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_uint16_uint16
(
    uint16_t *restrict Cx,
    const uint16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_uint16_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_uint16_uint32
(
    uint16_t *restrict Cx,
    const uint32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_uint16_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_uint16_uint64
(
    uint16_t *restrict Cx,
    const uint64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_uint16_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_uint16_fp32
(
    uint16_t *restrict Cx,
    const float *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_uint16_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_uint16_fp64
(
    uint16_t *restrict Cx,
    const double *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_uint16_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_uint32_bool
(
    uint32_t *restrict Cx,
    const bool *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_uint32_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_uint32_int8
(
    uint32_t *restrict Cx,
    const int8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_uint32_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_uint32_int16
(
    uint32_t *restrict Cx,
    const int16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_uint32_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_uint32_int32
(
    uint32_t *restrict Cx,
    const int32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_uint32_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_uint32_int64
(
    uint32_t *restrict Cx,
    const int64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_uint32_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_uint32_uint8
(
    uint32_t *restrict Cx,
    const uint8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_uint32_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_uint32_uint16
(
    uint32_t *restrict Cx,
    const uint16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_uint32_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_uint32_uint32
(
    uint32_t *restrict Cx,
    const uint32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_uint32_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_uint32_uint64
(
    uint32_t *restrict Cx,
    const uint64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_uint32_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_uint32_fp32
(
    uint32_t *restrict Cx,
    const float *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_uint32_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_uint32_fp64
(
    uint32_t *restrict Cx,
    const double *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_uint32_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_uint64_bool
(
    uint64_t *restrict Cx,
    const bool *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_uint64_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_uint64_int8
(
    uint64_t *restrict Cx,
    const int8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_uint64_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_uint64_int16
(
    uint64_t *restrict Cx,
    const int16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_uint64_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_uint64_int32
(
    uint64_t *restrict Cx,
    const int32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_uint64_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_uint64_int64
(
    uint64_t *restrict Cx,
    const int64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_uint64_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_uint64_uint8
(
    uint64_t *restrict Cx,
    const uint8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_uint64_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_uint64_uint16
(
    uint64_t *restrict Cx,
    const uint16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_uint64_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_uint64_uint32
(
    uint64_t *restrict Cx,
    const uint32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_uint64_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_uint64_uint64
(
    uint64_t *restrict Cx,
    const uint64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_uint64_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_uint64_fp32
(
    uint64_t *restrict Cx,
    const float *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_uint64_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_uint64_fp64
(
    uint64_t *restrict Cx,
    const double *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_uint64_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_fp32_bool
(
    float *restrict Cx,
    const bool *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_fp32_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_fp32_int8
(
    float *restrict Cx,
    const int8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_fp32_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_fp32_int16
(
    float *restrict Cx,
    const int16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_fp32_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_fp32_int32
(
    float *restrict Cx,
    const int32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_fp32_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_fp32_int64
(
    float *restrict Cx,
    const int64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_fp32_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_fp32_uint8
(
    float *restrict Cx,
    const uint8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_fp32_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_fp32_uint16
(
    float *restrict Cx,
    const uint16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_fp32_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_fp32_uint32
(
    float *restrict Cx,
    const uint32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_fp32_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_fp32_uint64
(
    float *restrict Cx,
    const uint64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_fp32_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_fp32_fp32
(
    float *restrict Cx,
    const float *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_fp32_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_fp32_fp64
(
    float *restrict Cx,
    const double *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_fp32_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_fp64_bool
(
    double *restrict Cx,
    const bool *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_fp64_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_fp64_int8
(
    double *restrict Cx,
    const int8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_fp64_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_fp64_int16
(
    double *restrict Cx,
    const int16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_fp64_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_fp64_int32
(
    double *restrict Cx,
    const int32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_fp64_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_fp64_int64
(
    double *restrict Cx,
    const int64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_fp64_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_fp64_uint8
(
    double *restrict Cx,
    const uint8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_fp64_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_fp64_uint16
(
    double *restrict Cx,
    const uint16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_fp64_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_fp64_uint32
(
    double *restrict Cx,
    const uint32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_fp64_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_fp64_uint64
(
    double *restrict Cx,
    const uint64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_fp64_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_fp64_fp32
(
    double *restrict Cx,
    const float *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_fp64_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_fp64_fp64
(
    double *restrict Cx,
    const double *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_fp64_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_bool_bool
(
    bool *restrict Cx,
    const bool *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_bool_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_bool_int8
(
    bool *restrict Cx,
    const int8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_bool_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_bool_int16
(
    bool *restrict Cx,
    const int16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_bool_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_bool_int32
(
    bool *restrict Cx,
    const int32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_bool_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_bool_int64
(
    bool *restrict Cx,
    const int64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_bool_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_bool_uint8
(
    bool *restrict Cx,
    const uint8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_bool_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_bool_uint16
(
    bool *restrict Cx,
    const uint16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_bool_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_bool_uint32
(
    bool *restrict Cx,
    const uint32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_bool_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_bool_uint64
(
    bool *restrict Cx,
    const uint64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_bool_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_bool_fp32
(
    bool *restrict Cx,
    const float *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_bool_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_bool_fp64
(
    bool *restrict Cx,
    const double *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_bool_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_int8_bool
(
    int8_t *restrict Cx,
    const bool *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_int8_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_int8_int8
(
    int8_t *restrict Cx,
    const int8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_int8_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_int8_int16
(
    int8_t *restrict Cx,
    const int16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_int8_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_int8_int32
(
    int8_t *restrict Cx,
    const int32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_int8_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_int8_int64
(
    int8_t *restrict Cx,
    const int64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_int8_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_int8_uint8
(
    int8_t *restrict Cx,
    const uint8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_int8_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_int8_uint16
(
    int8_t *restrict Cx,
    const uint16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_int8_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_int8_uint32
(
    int8_t *restrict Cx,
    const uint32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_int8_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_int8_uint64
(
    int8_t *restrict Cx,
    const uint64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_int8_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_int8_fp32
(
    int8_t *restrict Cx,
    const float *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_int8_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_int8_fp64
(
    int8_t *restrict Cx,
    const double *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_int8_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_int16_bool
(
    int16_t *restrict Cx,
    const bool *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_int16_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_int16_int8
(
    int16_t *restrict Cx,
    const int8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_int16_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_int16_int16
(
    int16_t *restrict Cx,
    const int16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_int16_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_int16_int32
(
    int16_t *restrict Cx,
    const int32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_int16_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_int16_int64
(
    int16_t *restrict Cx,
    const int64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_int16_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_int16_uint8
(
    int16_t *restrict Cx,
    const uint8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_int16_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_int16_uint16
(
    int16_t *restrict Cx,
    const uint16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_int16_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_int16_uint32
(
    int16_t *restrict Cx,
    const uint32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_int16_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_int16_uint64
(
    int16_t *restrict Cx,
    const uint64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_int16_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_int16_fp32
(
    int16_t *restrict Cx,
    const float *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_int16_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_int16_fp64
(
    int16_t *restrict Cx,
    const double *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_int16_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_int32_bool
(
    int32_t *restrict Cx,
    const bool *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_int32_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_int32_int8
(
    int32_t *restrict Cx,
    const int8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_int32_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_int32_int16
(
    int32_t *restrict Cx,
    const int16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_int32_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_int32_int32
(
    int32_t *restrict Cx,
    const int32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_int32_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_int32_int64
(
    int32_t *restrict Cx,
    const int64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_int32_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_int32_uint8
(
    int32_t *restrict Cx,
    const uint8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_int32_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_int32_uint16
(
    int32_t *restrict Cx,
    const uint16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_int32_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_int32_uint32
(
    int32_t *restrict Cx,
    const uint32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_int32_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_int32_uint64
(
    int32_t *restrict Cx,
    const uint64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_int32_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_int32_fp32
(
    int32_t *restrict Cx,
    const float *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_int32_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_int32_fp64
(
    int32_t *restrict Cx,
    const double *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_int32_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_int64_bool
(
    int64_t *restrict Cx,
    const bool *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_int64_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_int64_int8
(
    int64_t *restrict Cx,
    const int8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_int64_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_int64_int16
(
    int64_t *restrict Cx,
    const int16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_int64_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_int64_int32
(
    int64_t *restrict Cx,
    const int32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_int64_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_int64_int64
(
    int64_t *restrict Cx,
    const int64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_int64_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_int64_uint8
(
    int64_t *restrict Cx,
    const uint8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_int64_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_int64_uint16
(
    int64_t *restrict Cx,
    const uint16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_int64_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_int64_uint32
(
    int64_t *restrict Cx,
    const uint32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_int64_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_int64_uint64
(
    int64_t *restrict Cx,
    const uint64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_int64_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_int64_fp32
(
    int64_t *restrict Cx,
    const float *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_int64_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_int64_fp64
(
    int64_t *restrict Cx,
    const double *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_int64_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_uint8_bool
(
    uint8_t *restrict Cx,
    const bool *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_uint8_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_uint8_int8
(
    uint8_t *restrict Cx,
    const int8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_uint8_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_uint8_int16
(
    uint8_t *restrict Cx,
    const int16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_uint8_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_uint8_int32
(
    uint8_t *restrict Cx,
    const int32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_uint8_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_uint8_int64
(
    uint8_t *restrict Cx,
    const int64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_uint8_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_uint8_uint8
(
    uint8_t *restrict Cx,
    const uint8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_uint8_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_uint8_uint16
(
    uint8_t *restrict Cx,
    const uint16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_uint8_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_uint8_uint32
(
    uint8_t *restrict Cx,
    const uint32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_uint8_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_uint8_uint64
(
    uint8_t *restrict Cx,
    const uint64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_uint8_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_uint8_fp32
(
    uint8_t *restrict Cx,
    const float *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_uint8_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_uint8_fp64
(
    uint8_t *restrict Cx,
    const double *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_uint8_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_uint16_bool
(
    uint16_t *restrict Cx,
    const bool *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_uint16_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_uint16_int8
(
    uint16_t *restrict Cx,
    const int8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_uint16_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_uint16_int16
(
    uint16_t *restrict Cx,
    const int16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_uint16_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_uint16_int32
(
    uint16_t *restrict Cx,
    const int32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_uint16_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_uint16_int64
(
    uint16_t *restrict Cx,
    const int64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_uint16_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_uint16_uint8
(
    uint16_t *restrict Cx,
    const uint8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_uint16_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_uint16_uint16
(
    uint16_t *restrict Cx,
    const uint16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_uint16_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_uint16_uint32
(
    uint16_t *restrict Cx,
    const uint32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_uint16_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_uint16_uint64
(
    uint16_t *restrict Cx,
    const uint64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_uint16_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_uint16_fp32
(
    uint16_t *restrict Cx,
    const float *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_uint16_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_uint16_fp64
(
    uint16_t *restrict Cx,
    const double *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_uint16_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_uint32_bool
(
    uint32_t *restrict Cx,
    const bool *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_uint32_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_uint32_int8
(
    uint32_t *restrict Cx,
    const int8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_uint32_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_uint32_int16
(
    uint32_t *restrict Cx,
    const int16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_uint32_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_uint32_int32
(
    uint32_t *restrict Cx,
    const int32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_uint32_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_uint32_int64
(
    uint32_t *restrict Cx,
    const int64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_uint32_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_uint32_uint8
(
    uint32_t *restrict Cx,
    const uint8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_uint32_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_uint32_uint16
(
    uint32_t *restrict Cx,
    const uint16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_uint32_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_uint32_uint32
(
    uint32_t *restrict Cx,
    const uint32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_uint32_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_uint32_uint64
(
    uint32_t *restrict Cx,
    const uint64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_uint32_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_uint32_fp32
(
    uint32_t *restrict Cx,
    const float *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_uint32_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_uint32_fp64
(
    uint32_t *restrict Cx,
    const double *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_uint32_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_uint64_bool
(
    uint64_t *restrict Cx,
    const bool *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_uint64_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_uint64_int8
(
    uint64_t *restrict Cx,
    const int8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_uint64_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_uint64_int16
(
    uint64_t *restrict Cx,
    const int16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_uint64_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_uint64_int32
(
    uint64_t *restrict Cx,
    const int32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_uint64_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_uint64_int64
(
    uint64_t *restrict Cx,
    const int64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_uint64_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_uint64_uint8
(
    uint64_t *restrict Cx,
    const uint8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_uint64_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_uint64_uint16
(
    uint64_t *restrict Cx,
    const uint16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_uint64_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_uint64_uint32
(
    uint64_t *restrict Cx,
    const uint32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_uint64_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_uint64_uint64
(
    uint64_t *restrict Cx,
    const uint64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_uint64_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_uint64_fp32
(
    uint64_t *restrict Cx,
    const float *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_uint64_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_uint64_fp64
(
    uint64_t *restrict Cx,
    const double *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_uint64_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_fp32_bool
(
    float *restrict Cx,
    const bool *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_fp32_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_fp32_int8
(
    float *restrict Cx,
    const int8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_fp32_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_fp32_int16
(
    float *restrict Cx,
    const int16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_fp32_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_fp32_int32
(
    float *restrict Cx,
    const int32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_fp32_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_fp32_int64
(
    float *restrict Cx,
    const int64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_fp32_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_fp32_uint8
(
    float *restrict Cx,
    const uint8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_fp32_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_fp32_uint16
(
    float *restrict Cx,
    const uint16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_fp32_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_fp32_uint32
(
    float *restrict Cx,
    const uint32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_fp32_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_fp32_uint64
(
    float *restrict Cx,
    const uint64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_fp32_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_fp32_fp32
(
    float *restrict Cx,
    const float *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_fp32_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_fp32_fp64
(
    float *restrict Cx,
    const double *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_fp32_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_fp64_bool
(
    double *restrict Cx,
    const bool *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_fp64_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_fp64_int8
(
    double *restrict Cx,
    const int8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_fp64_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_fp64_int16
(
    double *restrict Cx,
    const int16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_fp64_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_fp64_int32
(
    double *restrict Cx,
    const int32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_fp64_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_fp64_int64
(
    double *restrict Cx,
    const int64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_fp64_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_fp64_uint8
(
    double *restrict Cx,
    const uint8_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_fp64_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_fp64_uint16
(
    double *restrict Cx,
    const uint16_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_fp64_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_fp64_uint32
(
    double *restrict Cx,
    const uint32_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_fp64_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_fp64_uint64
(
    double *restrict Cx,
    const uint64_t *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_fp64_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_fp64_fp32
(
    double *restrict Cx,
    const float *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_fp64_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_fp64_fp64
(
    double *restrict Cx,
    const double *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_fp64_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;

