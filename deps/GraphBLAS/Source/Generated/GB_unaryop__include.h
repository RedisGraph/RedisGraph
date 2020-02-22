//------------------------------------------------------------------------------
// GB_unaryop__include.h: definitions for GB_unaryop__*.c
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txargt for license.

// This file has been automatically generated from Generator/GB_unaryop.h

GrB_Info GB_unop__one_bool_bool
(
    bool *Cx,
    bool *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__one_bool_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__one_int8_int8
(
    int8_t *Cx,
    int8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__one_int8_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__one_int16_int16
(
    int16_t *Cx,
    int16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__one_int16_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__one_int32_int32
(
    int32_t *Cx,
    int32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__one_int32_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__one_int64_int64
(
    int64_t *Cx,
    int64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__one_int64_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__one_uint8_uint8
(
    uint8_t *Cx,
    uint8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__one_uint8_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__one_uint16_uint16
(
    uint16_t *Cx,
    uint16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__one_uint16_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__one_uint32_uint32
(
    uint32_t *Cx,
    uint32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__one_uint32_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__one_uint64_uint64
(
    uint64_t *Cx,
    uint64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__one_uint64_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__one_fp32_fp32
(
    float *Cx,
    float *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__one_fp32_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__one_fp64_fp64
(
    double *Cx,
    double *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__one_fp64_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_bool_bool
(
    bool *Cx,
    bool *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_bool_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_bool_int8
(
    bool *Cx,
    int8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_bool_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_bool_int16
(
    bool *Cx,
    int16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_bool_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_bool_int32
(
    bool *Cx,
    int32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_bool_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_bool_int64
(
    bool *Cx,
    int64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_bool_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_bool_uint8
(
    bool *Cx,
    uint8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_bool_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_bool_uint16
(
    bool *Cx,
    uint16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_bool_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_bool_uint32
(
    bool *Cx,
    uint32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_bool_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_bool_uint64
(
    bool *Cx,
    uint64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_bool_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_bool_fp32
(
    bool *Cx,
    float *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_bool_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_bool_fp64
(
    bool *Cx,
    double *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_bool_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_int8_bool
(
    int8_t *Cx,
    bool *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_int8_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_int8_int8
(
    int8_t *Cx,
    int8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_int8_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_int8_int16
(
    int8_t *Cx,
    int16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_int8_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_int8_int32
(
    int8_t *Cx,
    int32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_int8_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_int8_int64
(
    int8_t *Cx,
    int64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_int8_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_int8_uint8
(
    int8_t *Cx,
    uint8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_int8_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_int8_uint16
(
    int8_t *Cx,
    uint16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_int8_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_int8_uint32
(
    int8_t *Cx,
    uint32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_int8_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_int8_uint64
(
    int8_t *Cx,
    uint64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_int8_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_int8_fp32
(
    int8_t *Cx,
    float *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_int8_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_int8_fp64
(
    int8_t *Cx,
    double *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_int8_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_int16_bool
(
    int16_t *Cx,
    bool *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_int16_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_int16_int8
(
    int16_t *Cx,
    int8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_int16_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_int16_int16
(
    int16_t *Cx,
    int16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_int16_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_int16_int32
(
    int16_t *Cx,
    int32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_int16_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_int16_int64
(
    int16_t *Cx,
    int64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_int16_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_int16_uint8
(
    int16_t *Cx,
    uint8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_int16_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_int16_uint16
(
    int16_t *Cx,
    uint16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_int16_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_int16_uint32
(
    int16_t *Cx,
    uint32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_int16_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_int16_uint64
(
    int16_t *Cx,
    uint64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_int16_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_int16_fp32
(
    int16_t *Cx,
    float *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_int16_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_int16_fp64
(
    int16_t *Cx,
    double *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_int16_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_int32_bool
(
    int32_t *Cx,
    bool *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_int32_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_int32_int8
(
    int32_t *Cx,
    int8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_int32_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_int32_int16
(
    int32_t *Cx,
    int16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_int32_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_int32_int32
(
    int32_t *Cx,
    int32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_int32_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_int32_int64
(
    int32_t *Cx,
    int64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_int32_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_int32_uint8
(
    int32_t *Cx,
    uint8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_int32_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_int32_uint16
(
    int32_t *Cx,
    uint16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_int32_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_int32_uint32
(
    int32_t *Cx,
    uint32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_int32_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_int32_uint64
(
    int32_t *Cx,
    uint64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_int32_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_int32_fp32
(
    int32_t *Cx,
    float *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_int32_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_int32_fp64
(
    int32_t *Cx,
    double *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_int32_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_int64_bool
(
    int64_t *Cx,
    bool *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_int64_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_int64_int8
(
    int64_t *Cx,
    int8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_int64_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_int64_int16
(
    int64_t *Cx,
    int16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_int64_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_int64_int32
(
    int64_t *Cx,
    int32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_int64_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_int64_int64
(
    int64_t *Cx,
    int64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_int64_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_int64_uint8
(
    int64_t *Cx,
    uint8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_int64_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_int64_uint16
(
    int64_t *Cx,
    uint16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_int64_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_int64_uint32
(
    int64_t *Cx,
    uint32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_int64_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_int64_uint64
(
    int64_t *Cx,
    uint64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_int64_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_int64_fp32
(
    int64_t *Cx,
    float *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_int64_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_int64_fp64
(
    int64_t *Cx,
    double *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_int64_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_uint8_bool
(
    uint8_t *Cx,
    bool *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_uint8_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_uint8_int8
(
    uint8_t *Cx,
    int8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_uint8_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_uint8_int16
(
    uint8_t *Cx,
    int16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_uint8_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_uint8_int32
(
    uint8_t *Cx,
    int32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_uint8_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_uint8_int64
(
    uint8_t *Cx,
    int64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_uint8_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_uint8_uint8
(
    uint8_t *Cx,
    uint8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_uint8_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_uint8_uint16
(
    uint8_t *Cx,
    uint16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_uint8_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_uint8_uint32
(
    uint8_t *Cx,
    uint32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_uint8_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_uint8_uint64
(
    uint8_t *Cx,
    uint64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_uint8_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_uint8_fp32
(
    uint8_t *Cx,
    float *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_uint8_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_uint8_fp64
(
    uint8_t *Cx,
    double *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_uint8_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_uint16_bool
(
    uint16_t *Cx,
    bool *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_uint16_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_uint16_int8
(
    uint16_t *Cx,
    int8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_uint16_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_uint16_int16
(
    uint16_t *Cx,
    int16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_uint16_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_uint16_int32
(
    uint16_t *Cx,
    int32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_uint16_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_uint16_int64
(
    uint16_t *Cx,
    int64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_uint16_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_uint16_uint8
(
    uint16_t *Cx,
    uint8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_uint16_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_uint16_uint16
(
    uint16_t *Cx,
    uint16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_uint16_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_uint16_uint32
(
    uint16_t *Cx,
    uint32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_uint16_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_uint16_uint64
(
    uint16_t *Cx,
    uint64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_uint16_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_uint16_fp32
(
    uint16_t *Cx,
    float *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_uint16_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_uint16_fp64
(
    uint16_t *Cx,
    double *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_uint16_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_uint32_bool
(
    uint32_t *Cx,
    bool *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_uint32_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_uint32_int8
(
    uint32_t *Cx,
    int8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_uint32_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_uint32_int16
(
    uint32_t *Cx,
    int16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_uint32_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_uint32_int32
(
    uint32_t *Cx,
    int32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_uint32_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_uint32_int64
(
    uint32_t *Cx,
    int64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_uint32_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_uint32_uint8
(
    uint32_t *Cx,
    uint8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_uint32_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_uint32_uint16
(
    uint32_t *Cx,
    uint16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_uint32_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_uint32_uint32
(
    uint32_t *Cx,
    uint32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_uint32_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_uint32_uint64
(
    uint32_t *Cx,
    uint64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_uint32_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_uint32_fp32
(
    uint32_t *Cx,
    float *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_uint32_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_uint32_fp64
(
    uint32_t *Cx,
    double *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_uint32_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_uint64_bool
(
    uint64_t *Cx,
    bool *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_uint64_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_uint64_int8
(
    uint64_t *Cx,
    int8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_uint64_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_uint64_int16
(
    uint64_t *Cx,
    int16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_uint64_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_uint64_int32
(
    uint64_t *Cx,
    int32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_uint64_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_uint64_int64
(
    uint64_t *Cx,
    int64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_uint64_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_uint64_uint8
(
    uint64_t *Cx,
    uint8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_uint64_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_uint64_uint16
(
    uint64_t *Cx,
    uint16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_uint64_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_uint64_uint32
(
    uint64_t *Cx,
    uint32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_uint64_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_uint64_uint64
(
    uint64_t *Cx,
    uint64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_uint64_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_uint64_fp32
(
    uint64_t *Cx,
    float *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_uint64_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_uint64_fp64
(
    uint64_t *Cx,
    double *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_uint64_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_fp32_bool
(
    float *Cx,
    bool *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_fp32_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_fp32_int8
(
    float *Cx,
    int8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_fp32_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_fp32_int16
(
    float *Cx,
    int16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_fp32_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_fp32_int32
(
    float *Cx,
    int32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_fp32_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_fp32_int64
(
    float *Cx,
    int64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_fp32_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_fp32_uint8
(
    float *Cx,
    uint8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_fp32_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_fp32_uint16
(
    float *Cx,
    uint16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_fp32_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_fp32_uint32
(
    float *Cx,
    uint32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_fp32_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_fp32_uint64
(
    float *Cx,
    uint64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_fp32_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_fp32_fp32
(
    float *Cx,
    float *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_fp32_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_fp32_fp64
(
    float *Cx,
    double *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_fp32_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_fp64_bool
(
    double *Cx,
    bool *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_fp64_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_fp64_int8
(
    double *Cx,
    int8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_fp64_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_fp64_int16
(
    double *Cx,
    int16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_fp64_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_fp64_int32
(
    double *Cx,
    int32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_fp64_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_fp64_int64
(
    double *Cx,
    int64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_fp64_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_fp64_uint8
(
    double *Cx,
    uint8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_fp64_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_fp64_uint16
(
    double *Cx,
    uint16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_fp64_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_fp64_uint32
(
    double *Cx,
    uint32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_fp64_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_fp64_uint64
(
    double *Cx,
    uint64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_fp64_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_fp64_fp32
(
    double *Cx,
    float *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_fp64_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__identity_fp64_fp64
(
    double *Cx,
    double *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__identity_fp64_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_bool_bool
(
    bool *Cx,
    bool *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_bool_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_bool_int8
(
    bool *Cx,
    int8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_bool_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_bool_int16
(
    bool *Cx,
    int16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_bool_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_bool_int32
(
    bool *Cx,
    int32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_bool_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_bool_int64
(
    bool *Cx,
    int64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_bool_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_bool_uint8
(
    bool *Cx,
    uint8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_bool_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_bool_uint16
(
    bool *Cx,
    uint16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_bool_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_bool_uint32
(
    bool *Cx,
    uint32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_bool_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_bool_uint64
(
    bool *Cx,
    uint64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_bool_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_bool_fp32
(
    bool *Cx,
    float *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_bool_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_bool_fp64
(
    bool *Cx,
    double *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_bool_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_int8_bool
(
    int8_t *Cx,
    bool *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_int8_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_int8_int8
(
    int8_t *Cx,
    int8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_int8_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_int8_int16
(
    int8_t *Cx,
    int16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_int8_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_int8_int32
(
    int8_t *Cx,
    int32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_int8_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_int8_int64
(
    int8_t *Cx,
    int64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_int8_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_int8_uint8
(
    int8_t *Cx,
    uint8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_int8_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_int8_uint16
(
    int8_t *Cx,
    uint16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_int8_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_int8_uint32
(
    int8_t *Cx,
    uint32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_int8_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_int8_uint64
(
    int8_t *Cx,
    uint64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_int8_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_int8_fp32
(
    int8_t *Cx,
    float *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_int8_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_int8_fp64
(
    int8_t *Cx,
    double *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_int8_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_int16_bool
(
    int16_t *Cx,
    bool *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_int16_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_int16_int8
(
    int16_t *Cx,
    int8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_int16_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_int16_int16
(
    int16_t *Cx,
    int16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_int16_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_int16_int32
(
    int16_t *Cx,
    int32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_int16_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_int16_int64
(
    int16_t *Cx,
    int64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_int16_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_int16_uint8
(
    int16_t *Cx,
    uint8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_int16_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_int16_uint16
(
    int16_t *Cx,
    uint16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_int16_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_int16_uint32
(
    int16_t *Cx,
    uint32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_int16_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_int16_uint64
(
    int16_t *Cx,
    uint64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_int16_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_int16_fp32
(
    int16_t *Cx,
    float *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_int16_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_int16_fp64
(
    int16_t *Cx,
    double *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_int16_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_int32_bool
(
    int32_t *Cx,
    bool *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_int32_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_int32_int8
(
    int32_t *Cx,
    int8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_int32_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_int32_int16
(
    int32_t *Cx,
    int16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_int32_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_int32_int32
(
    int32_t *Cx,
    int32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_int32_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_int32_int64
(
    int32_t *Cx,
    int64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_int32_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_int32_uint8
(
    int32_t *Cx,
    uint8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_int32_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_int32_uint16
(
    int32_t *Cx,
    uint16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_int32_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_int32_uint32
(
    int32_t *Cx,
    uint32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_int32_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_int32_uint64
(
    int32_t *Cx,
    uint64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_int32_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_int32_fp32
(
    int32_t *Cx,
    float *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_int32_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_int32_fp64
(
    int32_t *Cx,
    double *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_int32_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_int64_bool
(
    int64_t *Cx,
    bool *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_int64_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_int64_int8
(
    int64_t *Cx,
    int8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_int64_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_int64_int16
(
    int64_t *Cx,
    int16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_int64_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_int64_int32
(
    int64_t *Cx,
    int32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_int64_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_int64_int64
(
    int64_t *Cx,
    int64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_int64_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_int64_uint8
(
    int64_t *Cx,
    uint8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_int64_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_int64_uint16
(
    int64_t *Cx,
    uint16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_int64_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_int64_uint32
(
    int64_t *Cx,
    uint32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_int64_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_int64_uint64
(
    int64_t *Cx,
    uint64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_int64_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_int64_fp32
(
    int64_t *Cx,
    float *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_int64_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_int64_fp64
(
    int64_t *Cx,
    double *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_int64_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_uint8_bool
(
    uint8_t *Cx,
    bool *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_uint8_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_uint8_int8
(
    uint8_t *Cx,
    int8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_uint8_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_uint8_int16
(
    uint8_t *Cx,
    int16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_uint8_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_uint8_int32
(
    uint8_t *Cx,
    int32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_uint8_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_uint8_int64
(
    uint8_t *Cx,
    int64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_uint8_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_uint8_uint8
(
    uint8_t *Cx,
    uint8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_uint8_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_uint8_uint16
(
    uint8_t *Cx,
    uint16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_uint8_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_uint8_uint32
(
    uint8_t *Cx,
    uint32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_uint8_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_uint8_uint64
(
    uint8_t *Cx,
    uint64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_uint8_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_uint8_fp32
(
    uint8_t *Cx,
    float *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_uint8_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_uint8_fp64
(
    uint8_t *Cx,
    double *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_uint8_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_uint16_bool
(
    uint16_t *Cx,
    bool *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_uint16_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_uint16_int8
(
    uint16_t *Cx,
    int8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_uint16_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_uint16_int16
(
    uint16_t *Cx,
    int16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_uint16_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_uint16_int32
(
    uint16_t *Cx,
    int32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_uint16_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_uint16_int64
(
    uint16_t *Cx,
    int64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_uint16_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_uint16_uint8
(
    uint16_t *Cx,
    uint8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_uint16_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_uint16_uint16
(
    uint16_t *Cx,
    uint16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_uint16_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_uint16_uint32
(
    uint16_t *Cx,
    uint32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_uint16_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_uint16_uint64
(
    uint16_t *Cx,
    uint64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_uint16_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_uint16_fp32
(
    uint16_t *Cx,
    float *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_uint16_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_uint16_fp64
(
    uint16_t *Cx,
    double *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_uint16_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_uint32_bool
(
    uint32_t *Cx,
    bool *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_uint32_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_uint32_int8
(
    uint32_t *Cx,
    int8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_uint32_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_uint32_int16
(
    uint32_t *Cx,
    int16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_uint32_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_uint32_int32
(
    uint32_t *Cx,
    int32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_uint32_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_uint32_int64
(
    uint32_t *Cx,
    int64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_uint32_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_uint32_uint8
(
    uint32_t *Cx,
    uint8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_uint32_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_uint32_uint16
(
    uint32_t *Cx,
    uint16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_uint32_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_uint32_uint32
(
    uint32_t *Cx,
    uint32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_uint32_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_uint32_uint64
(
    uint32_t *Cx,
    uint64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_uint32_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_uint32_fp32
(
    uint32_t *Cx,
    float *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_uint32_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_uint32_fp64
(
    uint32_t *Cx,
    double *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_uint32_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_uint64_bool
(
    uint64_t *Cx,
    bool *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_uint64_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_uint64_int8
(
    uint64_t *Cx,
    int8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_uint64_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_uint64_int16
(
    uint64_t *Cx,
    int16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_uint64_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_uint64_int32
(
    uint64_t *Cx,
    int32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_uint64_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_uint64_int64
(
    uint64_t *Cx,
    int64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_uint64_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_uint64_uint8
(
    uint64_t *Cx,
    uint8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_uint64_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_uint64_uint16
(
    uint64_t *Cx,
    uint16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_uint64_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_uint64_uint32
(
    uint64_t *Cx,
    uint32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_uint64_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_uint64_uint64
(
    uint64_t *Cx,
    uint64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_uint64_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_uint64_fp32
(
    uint64_t *Cx,
    float *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_uint64_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_uint64_fp64
(
    uint64_t *Cx,
    double *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_uint64_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_fp32_bool
(
    float *Cx,
    bool *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_fp32_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_fp32_int8
(
    float *Cx,
    int8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_fp32_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_fp32_int16
(
    float *Cx,
    int16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_fp32_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_fp32_int32
(
    float *Cx,
    int32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_fp32_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_fp32_int64
(
    float *Cx,
    int64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_fp32_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_fp32_uint8
(
    float *Cx,
    uint8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_fp32_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_fp32_uint16
(
    float *Cx,
    uint16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_fp32_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_fp32_uint32
(
    float *Cx,
    uint32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_fp32_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_fp32_uint64
(
    float *Cx,
    uint64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_fp32_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_fp32_fp32
(
    float *Cx,
    float *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_fp32_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_fp32_fp64
(
    float *Cx,
    double *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_fp32_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_fp64_bool
(
    double *Cx,
    bool *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_fp64_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_fp64_int8
(
    double *Cx,
    int8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_fp64_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_fp64_int16
(
    double *Cx,
    int16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_fp64_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_fp64_int32
(
    double *Cx,
    int32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_fp64_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_fp64_int64
(
    double *Cx,
    int64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_fp64_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_fp64_uint8
(
    double *Cx,
    uint8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_fp64_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_fp64_uint16
(
    double *Cx,
    uint16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_fp64_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_fp64_uint32
(
    double *Cx,
    uint32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_fp64_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_fp64_uint64
(
    double *Cx,
    uint64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_fp64_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_fp64_fp32
(
    double *Cx,
    float *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_fp64_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__ainv_fp64_fp64
(
    double *Cx,
    double *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__ainv_fp64_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_bool_bool
(
    bool *Cx,
    bool *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_bool_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_bool_int8
(
    bool *Cx,
    int8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_bool_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_bool_int16
(
    bool *Cx,
    int16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_bool_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_bool_int32
(
    bool *Cx,
    int32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_bool_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_bool_int64
(
    bool *Cx,
    int64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_bool_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_bool_uint8
(
    bool *Cx,
    uint8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_bool_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_bool_uint16
(
    bool *Cx,
    uint16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_bool_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_bool_uint32
(
    bool *Cx,
    uint32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_bool_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_bool_uint64
(
    bool *Cx,
    uint64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_bool_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_bool_fp32
(
    bool *Cx,
    float *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_bool_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_bool_fp64
(
    bool *Cx,
    double *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_bool_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_int8_bool
(
    int8_t *Cx,
    bool *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_int8_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_int8_int8
(
    int8_t *Cx,
    int8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_int8_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_int8_int16
(
    int8_t *Cx,
    int16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_int8_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_int8_int32
(
    int8_t *Cx,
    int32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_int8_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_int8_int64
(
    int8_t *Cx,
    int64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_int8_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_int8_uint8
(
    int8_t *Cx,
    uint8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_int8_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_int8_uint16
(
    int8_t *Cx,
    uint16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_int8_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_int8_uint32
(
    int8_t *Cx,
    uint32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_int8_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_int8_uint64
(
    int8_t *Cx,
    uint64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_int8_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_int8_fp32
(
    int8_t *Cx,
    float *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_int8_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_int8_fp64
(
    int8_t *Cx,
    double *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_int8_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_int16_bool
(
    int16_t *Cx,
    bool *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_int16_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_int16_int8
(
    int16_t *Cx,
    int8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_int16_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_int16_int16
(
    int16_t *Cx,
    int16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_int16_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_int16_int32
(
    int16_t *Cx,
    int32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_int16_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_int16_int64
(
    int16_t *Cx,
    int64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_int16_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_int16_uint8
(
    int16_t *Cx,
    uint8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_int16_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_int16_uint16
(
    int16_t *Cx,
    uint16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_int16_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_int16_uint32
(
    int16_t *Cx,
    uint32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_int16_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_int16_uint64
(
    int16_t *Cx,
    uint64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_int16_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_int16_fp32
(
    int16_t *Cx,
    float *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_int16_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_int16_fp64
(
    int16_t *Cx,
    double *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_int16_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_int32_bool
(
    int32_t *Cx,
    bool *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_int32_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_int32_int8
(
    int32_t *Cx,
    int8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_int32_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_int32_int16
(
    int32_t *Cx,
    int16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_int32_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_int32_int32
(
    int32_t *Cx,
    int32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_int32_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_int32_int64
(
    int32_t *Cx,
    int64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_int32_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_int32_uint8
(
    int32_t *Cx,
    uint8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_int32_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_int32_uint16
(
    int32_t *Cx,
    uint16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_int32_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_int32_uint32
(
    int32_t *Cx,
    uint32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_int32_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_int32_uint64
(
    int32_t *Cx,
    uint64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_int32_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_int32_fp32
(
    int32_t *Cx,
    float *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_int32_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_int32_fp64
(
    int32_t *Cx,
    double *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_int32_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_int64_bool
(
    int64_t *Cx,
    bool *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_int64_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_int64_int8
(
    int64_t *Cx,
    int8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_int64_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_int64_int16
(
    int64_t *Cx,
    int16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_int64_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_int64_int32
(
    int64_t *Cx,
    int32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_int64_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_int64_int64
(
    int64_t *Cx,
    int64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_int64_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_int64_uint8
(
    int64_t *Cx,
    uint8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_int64_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_int64_uint16
(
    int64_t *Cx,
    uint16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_int64_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_int64_uint32
(
    int64_t *Cx,
    uint32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_int64_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_int64_uint64
(
    int64_t *Cx,
    uint64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_int64_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_int64_fp32
(
    int64_t *Cx,
    float *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_int64_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_int64_fp64
(
    int64_t *Cx,
    double *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_int64_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_uint8_bool
(
    uint8_t *Cx,
    bool *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_uint8_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_uint8_int8
(
    uint8_t *Cx,
    int8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_uint8_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_uint8_int16
(
    uint8_t *Cx,
    int16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_uint8_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_uint8_int32
(
    uint8_t *Cx,
    int32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_uint8_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_uint8_int64
(
    uint8_t *Cx,
    int64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_uint8_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_uint8_uint8
(
    uint8_t *Cx,
    uint8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_uint8_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_uint8_uint16
(
    uint8_t *Cx,
    uint16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_uint8_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_uint8_uint32
(
    uint8_t *Cx,
    uint32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_uint8_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_uint8_uint64
(
    uint8_t *Cx,
    uint64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_uint8_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_uint8_fp32
(
    uint8_t *Cx,
    float *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_uint8_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_uint8_fp64
(
    uint8_t *Cx,
    double *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_uint8_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_uint16_bool
(
    uint16_t *Cx,
    bool *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_uint16_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_uint16_int8
(
    uint16_t *Cx,
    int8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_uint16_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_uint16_int16
(
    uint16_t *Cx,
    int16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_uint16_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_uint16_int32
(
    uint16_t *Cx,
    int32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_uint16_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_uint16_int64
(
    uint16_t *Cx,
    int64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_uint16_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_uint16_uint8
(
    uint16_t *Cx,
    uint8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_uint16_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_uint16_uint16
(
    uint16_t *Cx,
    uint16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_uint16_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_uint16_uint32
(
    uint16_t *Cx,
    uint32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_uint16_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_uint16_uint64
(
    uint16_t *Cx,
    uint64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_uint16_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_uint16_fp32
(
    uint16_t *Cx,
    float *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_uint16_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_uint16_fp64
(
    uint16_t *Cx,
    double *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_uint16_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_uint32_bool
(
    uint32_t *Cx,
    bool *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_uint32_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_uint32_int8
(
    uint32_t *Cx,
    int8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_uint32_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_uint32_int16
(
    uint32_t *Cx,
    int16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_uint32_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_uint32_int32
(
    uint32_t *Cx,
    int32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_uint32_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_uint32_int64
(
    uint32_t *Cx,
    int64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_uint32_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_uint32_uint8
(
    uint32_t *Cx,
    uint8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_uint32_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_uint32_uint16
(
    uint32_t *Cx,
    uint16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_uint32_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_uint32_uint32
(
    uint32_t *Cx,
    uint32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_uint32_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_uint32_uint64
(
    uint32_t *Cx,
    uint64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_uint32_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_uint32_fp32
(
    uint32_t *Cx,
    float *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_uint32_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_uint32_fp64
(
    uint32_t *Cx,
    double *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_uint32_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_uint64_bool
(
    uint64_t *Cx,
    bool *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_uint64_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_uint64_int8
(
    uint64_t *Cx,
    int8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_uint64_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_uint64_int16
(
    uint64_t *Cx,
    int16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_uint64_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_uint64_int32
(
    uint64_t *Cx,
    int32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_uint64_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_uint64_int64
(
    uint64_t *Cx,
    int64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_uint64_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_uint64_uint8
(
    uint64_t *Cx,
    uint8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_uint64_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_uint64_uint16
(
    uint64_t *Cx,
    uint16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_uint64_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_uint64_uint32
(
    uint64_t *Cx,
    uint32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_uint64_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_uint64_uint64
(
    uint64_t *Cx,
    uint64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_uint64_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_uint64_fp32
(
    uint64_t *Cx,
    float *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_uint64_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_uint64_fp64
(
    uint64_t *Cx,
    double *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_uint64_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_fp32_bool
(
    float *Cx,
    bool *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_fp32_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_fp32_int8
(
    float *Cx,
    int8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_fp32_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_fp32_int16
(
    float *Cx,
    int16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_fp32_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_fp32_int32
(
    float *Cx,
    int32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_fp32_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_fp32_int64
(
    float *Cx,
    int64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_fp32_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_fp32_uint8
(
    float *Cx,
    uint8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_fp32_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_fp32_uint16
(
    float *Cx,
    uint16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_fp32_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_fp32_uint32
(
    float *Cx,
    uint32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_fp32_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_fp32_uint64
(
    float *Cx,
    uint64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_fp32_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_fp32_fp32
(
    float *Cx,
    float *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_fp32_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_fp32_fp64
(
    float *Cx,
    double *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_fp32_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_fp64_bool
(
    double *Cx,
    bool *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_fp64_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_fp64_int8
(
    double *Cx,
    int8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_fp64_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_fp64_int16
(
    double *Cx,
    int16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_fp64_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_fp64_int32
(
    double *Cx,
    int32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_fp64_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_fp64_int64
(
    double *Cx,
    int64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_fp64_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_fp64_uint8
(
    double *Cx,
    uint8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_fp64_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_fp64_uint16
(
    double *Cx,
    uint16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_fp64_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_fp64_uint32
(
    double *Cx,
    uint32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_fp64_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_fp64_uint64
(
    double *Cx,
    uint64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_fp64_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_fp64_fp32
(
    double *Cx,
    float *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_fp64_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__abs_fp64_fp64
(
    double *Cx,
    double *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__abs_fp64_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_bool_bool
(
    bool *Cx,
    bool *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_bool_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_bool_int8
(
    bool *Cx,
    int8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_bool_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_bool_int16
(
    bool *Cx,
    int16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_bool_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_bool_int32
(
    bool *Cx,
    int32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_bool_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_bool_int64
(
    bool *Cx,
    int64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_bool_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_bool_uint8
(
    bool *Cx,
    uint8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_bool_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_bool_uint16
(
    bool *Cx,
    uint16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_bool_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_bool_uint32
(
    bool *Cx,
    uint32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_bool_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_bool_uint64
(
    bool *Cx,
    uint64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_bool_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_bool_fp32
(
    bool *Cx,
    float *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_bool_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_bool_fp64
(
    bool *Cx,
    double *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_bool_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_int8_bool
(
    int8_t *Cx,
    bool *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_int8_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_int8_int8
(
    int8_t *Cx,
    int8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_int8_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_int8_int16
(
    int8_t *Cx,
    int16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_int8_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_int8_int32
(
    int8_t *Cx,
    int32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_int8_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_int8_int64
(
    int8_t *Cx,
    int64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_int8_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_int8_uint8
(
    int8_t *Cx,
    uint8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_int8_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_int8_uint16
(
    int8_t *Cx,
    uint16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_int8_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_int8_uint32
(
    int8_t *Cx,
    uint32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_int8_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_int8_uint64
(
    int8_t *Cx,
    uint64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_int8_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_int8_fp32
(
    int8_t *Cx,
    float *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_int8_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_int8_fp64
(
    int8_t *Cx,
    double *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_int8_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_int16_bool
(
    int16_t *Cx,
    bool *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_int16_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_int16_int8
(
    int16_t *Cx,
    int8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_int16_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_int16_int16
(
    int16_t *Cx,
    int16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_int16_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_int16_int32
(
    int16_t *Cx,
    int32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_int16_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_int16_int64
(
    int16_t *Cx,
    int64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_int16_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_int16_uint8
(
    int16_t *Cx,
    uint8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_int16_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_int16_uint16
(
    int16_t *Cx,
    uint16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_int16_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_int16_uint32
(
    int16_t *Cx,
    uint32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_int16_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_int16_uint64
(
    int16_t *Cx,
    uint64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_int16_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_int16_fp32
(
    int16_t *Cx,
    float *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_int16_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_int16_fp64
(
    int16_t *Cx,
    double *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_int16_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_int32_bool
(
    int32_t *Cx,
    bool *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_int32_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_int32_int8
(
    int32_t *Cx,
    int8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_int32_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_int32_int16
(
    int32_t *Cx,
    int16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_int32_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_int32_int32
(
    int32_t *Cx,
    int32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_int32_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_int32_int64
(
    int32_t *Cx,
    int64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_int32_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_int32_uint8
(
    int32_t *Cx,
    uint8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_int32_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_int32_uint16
(
    int32_t *Cx,
    uint16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_int32_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_int32_uint32
(
    int32_t *Cx,
    uint32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_int32_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_int32_uint64
(
    int32_t *Cx,
    uint64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_int32_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_int32_fp32
(
    int32_t *Cx,
    float *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_int32_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_int32_fp64
(
    int32_t *Cx,
    double *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_int32_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_int64_bool
(
    int64_t *Cx,
    bool *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_int64_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_int64_int8
(
    int64_t *Cx,
    int8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_int64_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_int64_int16
(
    int64_t *Cx,
    int16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_int64_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_int64_int32
(
    int64_t *Cx,
    int32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_int64_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_int64_int64
(
    int64_t *Cx,
    int64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_int64_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_int64_uint8
(
    int64_t *Cx,
    uint8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_int64_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_int64_uint16
(
    int64_t *Cx,
    uint16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_int64_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_int64_uint32
(
    int64_t *Cx,
    uint32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_int64_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_int64_uint64
(
    int64_t *Cx,
    uint64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_int64_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_int64_fp32
(
    int64_t *Cx,
    float *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_int64_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_int64_fp64
(
    int64_t *Cx,
    double *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_int64_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_uint8_bool
(
    uint8_t *Cx,
    bool *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_uint8_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_uint8_int8
(
    uint8_t *Cx,
    int8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_uint8_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_uint8_int16
(
    uint8_t *Cx,
    int16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_uint8_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_uint8_int32
(
    uint8_t *Cx,
    int32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_uint8_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_uint8_int64
(
    uint8_t *Cx,
    int64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_uint8_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_uint8_uint8
(
    uint8_t *Cx,
    uint8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_uint8_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_uint8_uint16
(
    uint8_t *Cx,
    uint16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_uint8_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_uint8_uint32
(
    uint8_t *Cx,
    uint32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_uint8_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_uint8_uint64
(
    uint8_t *Cx,
    uint64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_uint8_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_uint8_fp32
(
    uint8_t *Cx,
    float *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_uint8_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_uint8_fp64
(
    uint8_t *Cx,
    double *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_uint8_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_uint16_bool
(
    uint16_t *Cx,
    bool *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_uint16_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_uint16_int8
(
    uint16_t *Cx,
    int8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_uint16_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_uint16_int16
(
    uint16_t *Cx,
    int16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_uint16_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_uint16_int32
(
    uint16_t *Cx,
    int32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_uint16_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_uint16_int64
(
    uint16_t *Cx,
    int64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_uint16_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_uint16_uint8
(
    uint16_t *Cx,
    uint8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_uint16_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_uint16_uint16
(
    uint16_t *Cx,
    uint16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_uint16_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_uint16_uint32
(
    uint16_t *Cx,
    uint32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_uint16_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_uint16_uint64
(
    uint16_t *Cx,
    uint64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_uint16_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_uint16_fp32
(
    uint16_t *Cx,
    float *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_uint16_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_uint16_fp64
(
    uint16_t *Cx,
    double *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_uint16_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_uint32_bool
(
    uint32_t *Cx,
    bool *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_uint32_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_uint32_int8
(
    uint32_t *Cx,
    int8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_uint32_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_uint32_int16
(
    uint32_t *Cx,
    int16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_uint32_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_uint32_int32
(
    uint32_t *Cx,
    int32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_uint32_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_uint32_int64
(
    uint32_t *Cx,
    int64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_uint32_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_uint32_uint8
(
    uint32_t *Cx,
    uint8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_uint32_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_uint32_uint16
(
    uint32_t *Cx,
    uint16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_uint32_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_uint32_uint32
(
    uint32_t *Cx,
    uint32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_uint32_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_uint32_uint64
(
    uint32_t *Cx,
    uint64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_uint32_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_uint32_fp32
(
    uint32_t *Cx,
    float *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_uint32_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_uint32_fp64
(
    uint32_t *Cx,
    double *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_uint32_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_uint64_bool
(
    uint64_t *Cx,
    bool *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_uint64_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_uint64_int8
(
    uint64_t *Cx,
    int8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_uint64_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_uint64_int16
(
    uint64_t *Cx,
    int16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_uint64_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_uint64_int32
(
    uint64_t *Cx,
    int32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_uint64_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_uint64_int64
(
    uint64_t *Cx,
    int64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_uint64_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_uint64_uint8
(
    uint64_t *Cx,
    uint8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_uint64_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_uint64_uint16
(
    uint64_t *Cx,
    uint16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_uint64_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_uint64_uint32
(
    uint64_t *Cx,
    uint32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_uint64_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_uint64_uint64
(
    uint64_t *Cx,
    uint64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_uint64_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_uint64_fp32
(
    uint64_t *Cx,
    float *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_uint64_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_uint64_fp64
(
    uint64_t *Cx,
    double *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_uint64_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_fp32_bool
(
    float *Cx,
    bool *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_fp32_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_fp32_int8
(
    float *Cx,
    int8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_fp32_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_fp32_int16
(
    float *Cx,
    int16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_fp32_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_fp32_int32
(
    float *Cx,
    int32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_fp32_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_fp32_int64
(
    float *Cx,
    int64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_fp32_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_fp32_uint8
(
    float *Cx,
    uint8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_fp32_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_fp32_uint16
(
    float *Cx,
    uint16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_fp32_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_fp32_uint32
(
    float *Cx,
    uint32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_fp32_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_fp32_uint64
(
    float *Cx,
    uint64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_fp32_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_fp32_fp32
(
    float *Cx,
    float *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_fp32_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_fp32_fp64
(
    float *Cx,
    double *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_fp32_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_fp64_bool
(
    double *Cx,
    bool *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_fp64_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_fp64_int8
(
    double *Cx,
    int8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_fp64_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_fp64_int16
(
    double *Cx,
    int16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_fp64_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_fp64_int32
(
    double *Cx,
    int32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_fp64_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_fp64_int64
(
    double *Cx,
    int64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_fp64_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_fp64_uint8
(
    double *Cx,
    uint8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_fp64_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_fp64_uint16
(
    double *Cx,
    uint16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_fp64_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_fp64_uint32
(
    double *Cx,
    uint32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_fp64_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_fp64_uint64
(
    double *Cx,
    uint64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_fp64_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_fp64_fp32
(
    double *Cx,
    float *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_fp64_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__minv_fp64_fp64
(
    double *Cx,
    double *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__minv_fp64_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_bool_bool
(
    bool *Cx,
    bool *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_bool_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_bool_int8
(
    bool *Cx,
    int8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_bool_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_bool_int16
(
    bool *Cx,
    int16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_bool_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_bool_int32
(
    bool *Cx,
    int32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_bool_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_bool_int64
(
    bool *Cx,
    int64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_bool_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_bool_uint8
(
    bool *Cx,
    uint8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_bool_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_bool_uint16
(
    bool *Cx,
    uint16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_bool_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_bool_uint32
(
    bool *Cx,
    uint32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_bool_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_bool_uint64
(
    bool *Cx,
    uint64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_bool_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_bool_fp32
(
    bool *Cx,
    float *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_bool_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_bool_fp64
(
    bool *Cx,
    double *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_bool_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_int8_bool
(
    int8_t *Cx,
    bool *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_int8_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_int8_int8
(
    int8_t *Cx,
    int8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_int8_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_int8_int16
(
    int8_t *Cx,
    int16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_int8_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_int8_int32
(
    int8_t *Cx,
    int32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_int8_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_int8_int64
(
    int8_t *Cx,
    int64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_int8_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_int8_uint8
(
    int8_t *Cx,
    uint8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_int8_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_int8_uint16
(
    int8_t *Cx,
    uint16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_int8_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_int8_uint32
(
    int8_t *Cx,
    uint32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_int8_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_int8_uint64
(
    int8_t *Cx,
    uint64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_int8_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_int8_fp32
(
    int8_t *Cx,
    float *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_int8_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_int8_fp64
(
    int8_t *Cx,
    double *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_int8_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_int16_bool
(
    int16_t *Cx,
    bool *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_int16_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_int16_int8
(
    int16_t *Cx,
    int8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_int16_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_int16_int16
(
    int16_t *Cx,
    int16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_int16_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_int16_int32
(
    int16_t *Cx,
    int32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_int16_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_int16_int64
(
    int16_t *Cx,
    int64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_int16_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_int16_uint8
(
    int16_t *Cx,
    uint8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_int16_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_int16_uint16
(
    int16_t *Cx,
    uint16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_int16_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_int16_uint32
(
    int16_t *Cx,
    uint32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_int16_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_int16_uint64
(
    int16_t *Cx,
    uint64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_int16_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_int16_fp32
(
    int16_t *Cx,
    float *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_int16_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_int16_fp64
(
    int16_t *Cx,
    double *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_int16_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_int32_bool
(
    int32_t *Cx,
    bool *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_int32_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_int32_int8
(
    int32_t *Cx,
    int8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_int32_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_int32_int16
(
    int32_t *Cx,
    int16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_int32_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_int32_int32
(
    int32_t *Cx,
    int32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_int32_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_int32_int64
(
    int32_t *Cx,
    int64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_int32_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_int32_uint8
(
    int32_t *Cx,
    uint8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_int32_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_int32_uint16
(
    int32_t *Cx,
    uint16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_int32_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_int32_uint32
(
    int32_t *Cx,
    uint32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_int32_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_int32_uint64
(
    int32_t *Cx,
    uint64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_int32_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_int32_fp32
(
    int32_t *Cx,
    float *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_int32_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_int32_fp64
(
    int32_t *Cx,
    double *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_int32_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_int64_bool
(
    int64_t *Cx,
    bool *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_int64_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_int64_int8
(
    int64_t *Cx,
    int8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_int64_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_int64_int16
(
    int64_t *Cx,
    int16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_int64_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_int64_int32
(
    int64_t *Cx,
    int32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_int64_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_int64_int64
(
    int64_t *Cx,
    int64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_int64_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_int64_uint8
(
    int64_t *Cx,
    uint8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_int64_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_int64_uint16
(
    int64_t *Cx,
    uint16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_int64_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_int64_uint32
(
    int64_t *Cx,
    uint32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_int64_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_int64_uint64
(
    int64_t *Cx,
    uint64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_int64_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_int64_fp32
(
    int64_t *Cx,
    float *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_int64_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_int64_fp64
(
    int64_t *Cx,
    double *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_int64_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_uint8_bool
(
    uint8_t *Cx,
    bool *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_uint8_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_uint8_int8
(
    uint8_t *Cx,
    int8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_uint8_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_uint8_int16
(
    uint8_t *Cx,
    int16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_uint8_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_uint8_int32
(
    uint8_t *Cx,
    int32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_uint8_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_uint8_int64
(
    uint8_t *Cx,
    int64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_uint8_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_uint8_uint8
(
    uint8_t *Cx,
    uint8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_uint8_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_uint8_uint16
(
    uint8_t *Cx,
    uint16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_uint8_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_uint8_uint32
(
    uint8_t *Cx,
    uint32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_uint8_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_uint8_uint64
(
    uint8_t *Cx,
    uint64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_uint8_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_uint8_fp32
(
    uint8_t *Cx,
    float *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_uint8_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_uint8_fp64
(
    uint8_t *Cx,
    double *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_uint8_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_uint16_bool
(
    uint16_t *Cx,
    bool *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_uint16_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_uint16_int8
(
    uint16_t *Cx,
    int8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_uint16_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_uint16_int16
(
    uint16_t *Cx,
    int16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_uint16_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_uint16_int32
(
    uint16_t *Cx,
    int32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_uint16_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_uint16_int64
(
    uint16_t *Cx,
    int64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_uint16_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_uint16_uint8
(
    uint16_t *Cx,
    uint8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_uint16_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_uint16_uint16
(
    uint16_t *Cx,
    uint16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_uint16_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_uint16_uint32
(
    uint16_t *Cx,
    uint32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_uint16_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_uint16_uint64
(
    uint16_t *Cx,
    uint64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_uint16_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_uint16_fp32
(
    uint16_t *Cx,
    float *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_uint16_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_uint16_fp64
(
    uint16_t *Cx,
    double *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_uint16_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_uint32_bool
(
    uint32_t *Cx,
    bool *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_uint32_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_uint32_int8
(
    uint32_t *Cx,
    int8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_uint32_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_uint32_int16
(
    uint32_t *Cx,
    int16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_uint32_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_uint32_int32
(
    uint32_t *Cx,
    int32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_uint32_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_uint32_int64
(
    uint32_t *Cx,
    int64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_uint32_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_uint32_uint8
(
    uint32_t *Cx,
    uint8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_uint32_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_uint32_uint16
(
    uint32_t *Cx,
    uint16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_uint32_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_uint32_uint32
(
    uint32_t *Cx,
    uint32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_uint32_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_uint32_uint64
(
    uint32_t *Cx,
    uint64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_uint32_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_uint32_fp32
(
    uint32_t *Cx,
    float *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_uint32_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_uint32_fp64
(
    uint32_t *Cx,
    double *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_uint32_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_uint64_bool
(
    uint64_t *Cx,
    bool *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_uint64_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_uint64_int8
(
    uint64_t *Cx,
    int8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_uint64_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_uint64_int16
(
    uint64_t *Cx,
    int16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_uint64_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_uint64_int32
(
    uint64_t *Cx,
    int32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_uint64_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_uint64_int64
(
    uint64_t *Cx,
    int64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_uint64_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_uint64_uint8
(
    uint64_t *Cx,
    uint8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_uint64_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_uint64_uint16
(
    uint64_t *Cx,
    uint16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_uint64_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_uint64_uint32
(
    uint64_t *Cx,
    uint32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_uint64_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_uint64_uint64
(
    uint64_t *Cx,
    uint64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_uint64_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_uint64_fp32
(
    uint64_t *Cx,
    float *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_uint64_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_uint64_fp64
(
    uint64_t *Cx,
    double *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_uint64_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_fp32_bool
(
    float *Cx,
    bool *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_fp32_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_fp32_int8
(
    float *Cx,
    int8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_fp32_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_fp32_int16
(
    float *Cx,
    int16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_fp32_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_fp32_int32
(
    float *Cx,
    int32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_fp32_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_fp32_int64
(
    float *Cx,
    int64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_fp32_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_fp32_uint8
(
    float *Cx,
    uint8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_fp32_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_fp32_uint16
(
    float *Cx,
    uint16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_fp32_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_fp32_uint32
(
    float *Cx,
    uint32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_fp32_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_fp32_uint64
(
    float *Cx,
    uint64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_fp32_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_fp32_fp32
(
    float *Cx,
    float *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_fp32_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_fp32_fp64
(
    float *Cx,
    double *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_fp32_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_fp64_bool
(
    double *Cx,
    bool *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_fp64_bool
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_fp64_int8
(
    double *Cx,
    int8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_fp64_int8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_fp64_int16
(
    double *Cx,
    int16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_fp64_int16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_fp64_int32
(
    double *Cx,
    int32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_fp64_int32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_fp64_int64
(
    double *Cx,
    int64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_fp64_int64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_fp64_uint8
(
    double *Cx,
    uint8_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_fp64_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_fp64_uint16
(
    double *Cx,
    uint16_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_fp64_uint16
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_fp64_uint32
(
    double *Cx,
    uint32_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_fp64_uint32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_fp64_uint64
(
    double *Cx,
    uint64_t *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_fp64_uint64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_fp64_fp32
(
    double *Cx,
    float *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_fp64_fp32
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

GrB_Info GB_unop__lnot_fp64_fp64
(
    double *Cx,
    double *Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran__lnot_fp64_fp64
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t *GB_RESTRICT *Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *GB_RESTRICT A_slice,
    int naslice
) ;

