//------------------------------------------------------------------------------
// GB_red__include.h: definitions for GB_red__*.c
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txargt for license.

// This file has been automatically generated from Generator/GB_red.h



GrB_Info GB_red_scalar__min_int8
(
    int8_t *result,
    const GrB_Matrix A,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachvec__min_int8
(
    int8_t *restrict Tx,
    GrB_Matrix A,
    const int64_t *restrict kfirst_slice,
    const int64_t *restrict klast_slice,
    const int64_t *restrict pstart_slice,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachindex__min_int8
(
    GrB_Matrix *Thandle,
    GrB_Type ttype,
    GrB_Matrix A,
    const int64_t *restrict pstart_slice,
    int nth,
    int nthreads,
    GB_Context Context
) ;



GrB_Info GB_red_build__min_int8
(
    int8_t *restrict Tx,
    int64_t  *restrict Ti,
    const int8_t *restrict S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *restrict I_work,
    const int64_t *restrict K_work,
    const int64_t *restrict tstart_slice,
    const int64_t *restrict tnz_slice,
    int nthreads
) ;



GrB_Info GB_red_scalar__min_int16
(
    int16_t *result,
    const GrB_Matrix A,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachvec__min_int16
(
    int16_t *restrict Tx,
    GrB_Matrix A,
    const int64_t *restrict kfirst_slice,
    const int64_t *restrict klast_slice,
    const int64_t *restrict pstart_slice,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachindex__min_int16
(
    GrB_Matrix *Thandle,
    GrB_Type ttype,
    GrB_Matrix A,
    const int64_t *restrict pstart_slice,
    int nth,
    int nthreads,
    GB_Context Context
) ;



GrB_Info GB_red_build__min_int16
(
    int16_t *restrict Tx,
    int64_t  *restrict Ti,
    const int16_t *restrict S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *restrict I_work,
    const int64_t *restrict K_work,
    const int64_t *restrict tstart_slice,
    const int64_t *restrict tnz_slice,
    int nthreads
) ;



GrB_Info GB_red_scalar__min_int32
(
    int32_t *result,
    const GrB_Matrix A,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachvec__min_int32
(
    int32_t *restrict Tx,
    GrB_Matrix A,
    const int64_t *restrict kfirst_slice,
    const int64_t *restrict klast_slice,
    const int64_t *restrict pstart_slice,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachindex__min_int32
(
    GrB_Matrix *Thandle,
    GrB_Type ttype,
    GrB_Matrix A,
    const int64_t *restrict pstart_slice,
    int nth,
    int nthreads,
    GB_Context Context
) ;



GrB_Info GB_red_build__min_int32
(
    int32_t *restrict Tx,
    int64_t  *restrict Ti,
    const int32_t *restrict S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *restrict I_work,
    const int64_t *restrict K_work,
    const int64_t *restrict tstart_slice,
    const int64_t *restrict tnz_slice,
    int nthreads
) ;



GrB_Info GB_red_scalar__min_int64
(
    int64_t *result,
    const GrB_Matrix A,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachvec__min_int64
(
    int64_t *restrict Tx,
    GrB_Matrix A,
    const int64_t *restrict kfirst_slice,
    const int64_t *restrict klast_slice,
    const int64_t *restrict pstart_slice,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachindex__min_int64
(
    GrB_Matrix *Thandle,
    GrB_Type ttype,
    GrB_Matrix A,
    const int64_t *restrict pstart_slice,
    int nth,
    int nthreads,
    GB_Context Context
) ;



GrB_Info GB_red_build__min_int64
(
    int64_t *restrict Tx,
    int64_t  *restrict Ti,
    const int64_t *restrict S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *restrict I_work,
    const int64_t *restrict K_work,
    const int64_t *restrict tstart_slice,
    const int64_t *restrict tnz_slice,
    int nthreads
) ;



GrB_Info GB_red_scalar__min_uint8
(
    uint8_t *result,
    const GrB_Matrix A,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachvec__min_uint8
(
    uint8_t *restrict Tx,
    GrB_Matrix A,
    const int64_t *restrict kfirst_slice,
    const int64_t *restrict klast_slice,
    const int64_t *restrict pstart_slice,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachindex__min_uint8
(
    GrB_Matrix *Thandle,
    GrB_Type ttype,
    GrB_Matrix A,
    const int64_t *restrict pstart_slice,
    int nth,
    int nthreads,
    GB_Context Context
) ;



GrB_Info GB_red_build__min_uint8
(
    uint8_t *restrict Tx,
    int64_t  *restrict Ti,
    const uint8_t *restrict S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *restrict I_work,
    const int64_t *restrict K_work,
    const int64_t *restrict tstart_slice,
    const int64_t *restrict tnz_slice,
    int nthreads
) ;



GrB_Info GB_red_scalar__min_uint16
(
    uint16_t *result,
    const GrB_Matrix A,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachvec__min_uint16
(
    uint16_t *restrict Tx,
    GrB_Matrix A,
    const int64_t *restrict kfirst_slice,
    const int64_t *restrict klast_slice,
    const int64_t *restrict pstart_slice,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachindex__min_uint16
(
    GrB_Matrix *Thandle,
    GrB_Type ttype,
    GrB_Matrix A,
    const int64_t *restrict pstart_slice,
    int nth,
    int nthreads,
    GB_Context Context
) ;



GrB_Info GB_red_build__min_uint16
(
    uint16_t *restrict Tx,
    int64_t  *restrict Ti,
    const uint16_t *restrict S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *restrict I_work,
    const int64_t *restrict K_work,
    const int64_t *restrict tstart_slice,
    const int64_t *restrict tnz_slice,
    int nthreads
) ;



GrB_Info GB_red_scalar__min_uint32
(
    uint32_t *result,
    const GrB_Matrix A,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachvec__min_uint32
(
    uint32_t *restrict Tx,
    GrB_Matrix A,
    const int64_t *restrict kfirst_slice,
    const int64_t *restrict klast_slice,
    const int64_t *restrict pstart_slice,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachindex__min_uint32
(
    GrB_Matrix *Thandle,
    GrB_Type ttype,
    GrB_Matrix A,
    const int64_t *restrict pstart_slice,
    int nth,
    int nthreads,
    GB_Context Context
) ;



GrB_Info GB_red_build__min_uint32
(
    uint32_t *restrict Tx,
    int64_t  *restrict Ti,
    const uint32_t *restrict S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *restrict I_work,
    const int64_t *restrict K_work,
    const int64_t *restrict tstart_slice,
    const int64_t *restrict tnz_slice,
    int nthreads
) ;



GrB_Info GB_red_scalar__min_uint64
(
    uint64_t *result,
    const GrB_Matrix A,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachvec__min_uint64
(
    uint64_t *restrict Tx,
    GrB_Matrix A,
    const int64_t *restrict kfirst_slice,
    const int64_t *restrict klast_slice,
    const int64_t *restrict pstart_slice,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachindex__min_uint64
(
    GrB_Matrix *Thandle,
    GrB_Type ttype,
    GrB_Matrix A,
    const int64_t *restrict pstart_slice,
    int nth,
    int nthreads,
    GB_Context Context
) ;



GrB_Info GB_red_build__min_uint64
(
    uint64_t *restrict Tx,
    int64_t  *restrict Ti,
    const uint64_t *restrict S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *restrict I_work,
    const int64_t *restrict K_work,
    const int64_t *restrict tstart_slice,
    const int64_t *restrict tnz_slice,
    int nthreads
) ;



GrB_Info GB_red_scalar__min_fp32
(
    float *result,
    const GrB_Matrix A,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachvec__min_fp32
(
    float *restrict Tx,
    GrB_Matrix A,
    const int64_t *restrict kfirst_slice,
    const int64_t *restrict klast_slice,
    const int64_t *restrict pstart_slice,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachindex__min_fp32
(
    GrB_Matrix *Thandle,
    GrB_Type ttype,
    GrB_Matrix A,
    const int64_t *restrict pstart_slice,
    int nth,
    int nthreads,
    GB_Context Context
) ;



GrB_Info GB_red_build__min_fp32
(
    float *restrict Tx,
    int64_t  *restrict Ti,
    const float *restrict S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *restrict I_work,
    const int64_t *restrict K_work,
    const int64_t *restrict tstart_slice,
    const int64_t *restrict tnz_slice,
    int nthreads
) ;



GrB_Info GB_red_scalar__min_fp64
(
    double *result,
    const GrB_Matrix A,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachvec__min_fp64
(
    double *restrict Tx,
    GrB_Matrix A,
    const int64_t *restrict kfirst_slice,
    const int64_t *restrict klast_slice,
    const int64_t *restrict pstart_slice,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachindex__min_fp64
(
    GrB_Matrix *Thandle,
    GrB_Type ttype,
    GrB_Matrix A,
    const int64_t *restrict pstart_slice,
    int nth,
    int nthreads,
    GB_Context Context
) ;



GrB_Info GB_red_build__min_fp64
(
    double *restrict Tx,
    int64_t  *restrict Ti,
    const double *restrict S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *restrict I_work,
    const int64_t *restrict K_work,
    const int64_t *restrict tstart_slice,
    const int64_t *restrict tnz_slice,
    int nthreads
) ;



GrB_Info GB_red_scalar__max_int8
(
    int8_t *result,
    const GrB_Matrix A,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachvec__max_int8
(
    int8_t *restrict Tx,
    GrB_Matrix A,
    const int64_t *restrict kfirst_slice,
    const int64_t *restrict klast_slice,
    const int64_t *restrict pstart_slice,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachindex__max_int8
(
    GrB_Matrix *Thandle,
    GrB_Type ttype,
    GrB_Matrix A,
    const int64_t *restrict pstart_slice,
    int nth,
    int nthreads,
    GB_Context Context
) ;



GrB_Info GB_red_build__max_int8
(
    int8_t *restrict Tx,
    int64_t  *restrict Ti,
    const int8_t *restrict S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *restrict I_work,
    const int64_t *restrict K_work,
    const int64_t *restrict tstart_slice,
    const int64_t *restrict tnz_slice,
    int nthreads
) ;



GrB_Info GB_red_scalar__max_int16
(
    int16_t *result,
    const GrB_Matrix A,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachvec__max_int16
(
    int16_t *restrict Tx,
    GrB_Matrix A,
    const int64_t *restrict kfirst_slice,
    const int64_t *restrict klast_slice,
    const int64_t *restrict pstart_slice,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachindex__max_int16
(
    GrB_Matrix *Thandle,
    GrB_Type ttype,
    GrB_Matrix A,
    const int64_t *restrict pstart_slice,
    int nth,
    int nthreads,
    GB_Context Context
) ;



GrB_Info GB_red_build__max_int16
(
    int16_t *restrict Tx,
    int64_t  *restrict Ti,
    const int16_t *restrict S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *restrict I_work,
    const int64_t *restrict K_work,
    const int64_t *restrict tstart_slice,
    const int64_t *restrict tnz_slice,
    int nthreads
) ;



GrB_Info GB_red_scalar__max_int32
(
    int32_t *result,
    const GrB_Matrix A,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachvec__max_int32
(
    int32_t *restrict Tx,
    GrB_Matrix A,
    const int64_t *restrict kfirst_slice,
    const int64_t *restrict klast_slice,
    const int64_t *restrict pstart_slice,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachindex__max_int32
(
    GrB_Matrix *Thandle,
    GrB_Type ttype,
    GrB_Matrix A,
    const int64_t *restrict pstart_slice,
    int nth,
    int nthreads,
    GB_Context Context
) ;



GrB_Info GB_red_build__max_int32
(
    int32_t *restrict Tx,
    int64_t  *restrict Ti,
    const int32_t *restrict S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *restrict I_work,
    const int64_t *restrict K_work,
    const int64_t *restrict tstart_slice,
    const int64_t *restrict tnz_slice,
    int nthreads
) ;



GrB_Info GB_red_scalar__max_int64
(
    int64_t *result,
    const GrB_Matrix A,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachvec__max_int64
(
    int64_t *restrict Tx,
    GrB_Matrix A,
    const int64_t *restrict kfirst_slice,
    const int64_t *restrict klast_slice,
    const int64_t *restrict pstart_slice,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachindex__max_int64
(
    GrB_Matrix *Thandle,
    GrB_Type ttype,
    GrB_Matrix A,
    const int64_t *restrict pstart_slice,
    int nth,
    int nthreads,
    GB_Context Context
) ;



GrB_Info GB_red_build__max_int64
(
    int64_t *restrict Tx,
    int64_t  *restrict Ti,
    const int64_t *restrict S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *restrict I_work,
    const int64_t *restrict K_work,
    const int64_t *restrict tstart_slice,
    const int64_t *restrict tnz_slice,
    int nthreads
) ;



GrB_Info GB_red_scalar__max_uint8
(
    uint8_t *result,
    const GrB_Matrix A,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachvec__max_uint8
(
    uint8_t *restrict Tx,
    GrB_Matrix A,
    const int64_t *restrict kfirst_slice,
    const int64_t *restrict klast_slice,
    const int64_t *restrict pstart_slice,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachindex__max_uint8
(
    GrB_Matrix *Thandle,
    GrB_Type ttype,
    GrB_Matrix A,
    const int64_t *restrict pstart_slice,
    int nth,
    int nthreads,
    GB_Context Context
) ;



GrB_Info GB_red_build__max_uint8
(
    uint8_t *restrict Tx,
    int64_t  *restrict Ti,
    const uint8_t *restrict S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *restrict I_work,
    const int64_t *restrict K_work,
    const int64_t *restrict tstart_slice,
    const int64_t *restrict tnz_slice,
    int nthreads
) ;



GrB_Info GB_red_scalar__max_uint16
(
    uint16_t *result,
    const GrB_Matrix A,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachvec__max_uint16
(
    uint16_t *restrict Tx,
    GrB_Matrix A,
    const int64_t *restrict kfirst_slice,
    const int64_t *restrict klast_slice,
    const int64_t *restrict pstart_slice,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachindex__max_uint16
(
    GrB_Matrix *Thandle,
    GrB_Type ttype,
    GrB_Matrix A,
    const int64_t *restrict pstart_slice,
    int nth,
    int nthreads,
    GB_Context Context
) ;



GrB_Info GB_red_build__max_uint16
(
    uint16_t *restrict Tx,
    int64_t  *restrict Ti,
    const uint16_t *restrict S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *restrict I_work,
    const int64_t *restrict K_work,
    const int64_t *restrict tstart_slice,
    const int64_t *restrict tnz_slice,
    int nthreads
) ;



GrB_Info GB_red_scalar__max_uint32
(
    uint32_t *result,
    const GrB_Matrix A,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachvec__max_uint32
(
    uint32_t *restrict Tx,
    GrB_Matrix A,
    const int64_t *restrict kfirst_slice,
    const int64_t *restrict klast_slice,
    const int64_t *restrict pstart_slice,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachindex__max_uint32
(
    GrB_Matrix *Thandle,
    GrB_Type ttype,
    GrB_Matrix A,
    const int64_t *restrict pstart_slice,
    int nth,
    int nthreads,
    GB_Context Context
) ;



GrB_Info GB_red_build__max_uint32
(
    uint32_t *restrict Tx,
    int64_t  *restrict Ti,
    const uint32_t *restrict S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *restrict I_work,
    const int64_t *restrict K_work,
    const int64_t *restrict tstart_slice,
    const int64_t *restrict tnz_slice,
    int nthreads
) ;



GrB_Info GB_red_scalar__max_uint64
(
    uint64_t *result,
    const GrB_Matrix A,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachvec__max_uint64
(
    uint64_t *restrict Tx,
    GrB_Matrix A,
    const int64_t *restrict kfirst_slice,
    const int64_t *restrict klast_slice,
    const int64_t *restrict pstart_slice,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachindex__max_uint64
(
    GrB_Matrix *Thandle,
    GrB_Type ttype,
    GrB_Matrix A,
    const int64_t *restrict pstart_slice,
    int nth,
    int nthreads,
    GB_Context Context
) ;



GrB_Info GB_red_build__max_uint64
(
    uint64_t *restrict Tx,
    int64_t  *restrict Ti,
    const uint64_t *restrict S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *restrict I_work,
    const int64_t *restrict K_work,
    const int64_t *restrict tstart_slice,
    const int64_t *restrict tnz_slice,
    int nthreads
) ;



GrB_Info GB_red_scalar__max_fp32
(
    float *result,
    const GrB_Matrix A,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachvec__max_fp32
(
    float *restrict Tx,
    GrB_Matrix A,
    const int64_t *restrict kfirst_slice,
    const int64_t *restrict klast_slice,
    const int64_t *restrict pstart_slice,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachindex__max_fp32
(
    GrB_Matrix *Thandle,
    GrB_Type ttype,
    GrB_Matrix A,
    const int64_t *restrict pstart_slice,
    int nth,
    int nthreads,
    GB_Context Context
) ;



GrB_Info GB_red_build__max_fp32
(
    float *restrict Tx,
    int64_t  *restrict Ti,
    const float *restrict S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *restrict I_work,
    const int64_t *restrict K_work,
    const int64_t *restrict tstart_slice,
    const int64_t *restrict tnz_slice,
    int nthreads
) ;



GrB_Info GB_red_scalar__max_fp64
(
    double *result,
    const GrB_Matrix A,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachvec__max_fp64
(
    double *restrict Tx,
    GrB_Matrix A,
    const int64_t *restrict kfirst_slice,
    const int64_t *restrict klast_slice,
    const int64_t *restrict pstart_slice,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachindex__max_fp64
(
    GrB_Matrix *Thandle,
    GrB_Type ttype,
    GrB_Matrix A,
    const int64_t *restrict pstart_slice,
    int nth,
    int nthreads,
    GB_Context Context
) ;



GrB_Info GB_red_build__max_fp64
(
    double *restrict Tx,
    int64_t  *restrict Ti,
    const double *restrict S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *restrict I_work,
    const int64_t *restrict K_work,
    const int64_t *restrict tstart_slice,
    const int64_t *restrict tnz_slice,
    int nthreads
) ;



GrB_Info GB_red_scalar__plus_int8
(
    int8_t *result,
    const GrB_Matrix A,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachvec__plus_int8
(
    int8_t *restrict Tx,
    GrB_Matrix A,
    const int64_t *restrict kfirst_slice,
    const int64_t *restrict klast_slice,
    const int64_t *restrict pstart_slice,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachindex__plus_int8
(
    GrB_Matrix *Thandle,
    GrB_Type ttype,
    GrB_Matrix A,
    const int64_t *restrict pstart_slice,
    int nth,
    int nthreads,
    GB_Context Context
) ;



GrB_Info GB_red_build__plus_int8
(
    int8_t *restrict Tx,
    int64_t  *restrict Ti,
    const int8_t *restrict S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *restrict I_work,
    const int64_t *restrict K_work,
    const int64_t *restrict tstart_slice,
    const int64_t *restrict tnz_slice,
    int nthreads
) ;



GrB_Info GB_red_scalar__plus_int16
(
    int16_t *result,
    const GrB_Matrix A,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachvec__plus_int16
(
    int16_t *restrict Tx,
    GrB_Matrix A,
    const int64_t *restrict kfirst_slice,
    const int64_t *restrict klast_slice,
    const int64_t *restrict pstart_slice,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachindex__plus_int16
(
    GrB_Matrix *Thandle,
    GrB_Type ttype,
    GrB_Matrix A,
    const int64_t *restrict pstart_slice,
    int nth,
    int nthreads,
    GB_Context Context
) ;



GrB_Info GB_red_build__plus_int16
(
    int16_t *restrict Tx,
    int64_t  *restrict Ti,
    const int16_t *restrict S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *restrict I_work,
    const int64_t *restrict K_work,
    const int64_t *restrict tstart_slice,
    const int64_t *restrict tnz_slice,
    int nthreads
) ;



GrB_Info GB_red_scalar__plus_int32
(
    int32_t *result,
    const GrB_Matrix A,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachvec__plus_int32
(
    int32_t *restrict Tx,
    GrB_Matrix A,
    const int64_t *restrict kfirst_slice,
    const int64_t *restrict klast_slice,
    const int64_t *restrict pstart_slice,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachindex__plus_int32
(
    GrB_Matrix *Thandle,
    GrB_Type ttype,
    GrB_Matrix A,
    const int64_t *restrict pstart_slice,
    int nth,
    int nthreads,
    GB_Context Context
) ;



GrB_Info GB_red_build__plus_int32
(
    int32_t *restrict Tx,
    int64_t  *restrict Ti,
    const int32_t *restrict S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *restrict I_work,
    const int64_t *restrict K_work,
    const int64_t *restrict tstart_slice,
    const int64_t *restrict tnz_slice,
    int nthreads
) ;



GrB_Info GB_red_scalar__plus_int64
(
    int64_t *result,
    const GrB_Matrix A,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachvec__plus_int64
(
    int64_t *restrict Tx,
    GrB_Matrix A,
    const int64_t *restrict kfirst_slice,
    const int64_t *restrict klast_slice,
    const int64_t *restrict pstart_slice,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachindex__plus_int64
(
    GrB_Matrix *Thandle,
    GrB_Type ttype,
    GrB_Matrix A,
    const int64_t *restrict pstart_slice,
    int nth,
    int nthreads,
    GB_Context Context
) ;



GrB_Info GB_red_build__plus_int64
(
    int64_t *restrict Tx,
    int64_t  *restrict Ti,
    const int64_t *restrict S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *restrict I_work,
    const int64_t *restrict K_work,
    const int64_t *restrict tstart_slice,
    const int64_t *restrict tnz_slice,
    int nthreads
) ;



GrB_Info GB_red_scalar__plus_uint8
(
    uint8_t *result,
    const GrB_Matrix A,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachvec__plus_uint8
(
    uint8_t *restrict Tx,
    GrB_Matrix A,
    const int64_t *restrict kfirst_slice,
    const int64_t *restrict klast_slice,
    const int64_t *restrict pstart_slice,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachindex__plus_uint8
(
    GrB_Matrix *Thandle,
    GrB_Type ttype,
    GrB_Matrix A,
    const int64_t *restrict pstart_slice,
    int nth,
    int nthreads,
    GB_Context Context
) ;



GrB_Info GB_red_build__plus_uint8
(
    uint8_t *restrict Tx,
    int64_t  *restrict Ti,
    const uint8_t *restrict S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *restrict I_work,
    const int64_t *restrict K_work,
    const int64_t *restrict tstart_slice,
    const int64_t *restrict tnz_slice,
    int nthreads
) ;



GrB_Info GB_red_scalar__plus_uint16
(
    uint16_t *result,
    const GrB_Matrix A,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachvec__plus_uint16
(
    uint16_t *restrict Tx,
    GrB_Matrix A,
    const int64_t *restrict kfirst_slice,
    const int64_t *restrict klast_slice,
    const int64_t *restrict pstart_slice,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachindex__plus_uint16
(
    GrB_Matrix *Thandle,
    GrB_Type ttype,
    GrB_Matrix A,
    const int64_t *restrict pstart_slice,
    int nth,
    int nthreads,
    GB_Context Context
) ;



GrB_Info GB_red_build__plus_uint16
(
    uint16_t *restrict Tx,
    int64_t  *restrict Ti,
    const uint16_t *restrict S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *restrict I_work,
    const int64_t *restrict K_work,
    const int64_t *restrict tstart_slice,
    const int64_t *restrict tnz_slice,
    int nthreads
) ;



GrB_Info GB_red_scalar__plus_uint32
(
    uint32_t *result,
    const GrB_Matrix A,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachvec__plus_uint32
(
    uint32_t *restrict Tx,
    GrB_Matrix A,
    const int64_t *restrict kfirst_slice,
    const int64_t *restrict klast_slice,
    const int64_t *restrict pstart_slice,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachindex__plus_uint32
(
    GrB_Matrix *Thandle,
    GrB_Type ttype,
    GrB_Matrix A,
    const int64_t *restrict pstart_slice,
    int nth,
    int nthreads,
    GB_Context Context
) ;



GrB_Info GB_red_build__plus_uint32
(
    uint32_t *restrict Tx,
    int64_t  *restrict Ti,
    const uint32_t *restrict S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *restrict I_work,
    const int64_t *restrict K_work,
    const int64_t *restrict tstart_slice,
    const int64_t *restrict tnz_slice,
    int nthreads
) ;



GrB_Info GB_red_scalar__plus_uint64
(
    uint64_t *result,
    const GrB_Matrix A,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachvec__plus_uint64
(
    uint64_t *restrict Tx,
    GrB_Matrix A,
    const int64_t *restrict kfirst_slice,
    const int64_t *restrict klast_slice,
    const int64_t *restrict pstart_slice,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachindex__plus_uint64
(
    GrB_Matrix *Thandle,
    GrB_Type ttype,
    GrB_Matrix A,
    const int64_t *restrict pstart_slice,
    int nth,
    int nthreads,
    GB_Context Context
) ;



GrB_Info GB_red_build__plus_uint64
(
    uint64_t *restrict Tx,
    int64_t  *restrict Ti,
    const uint64_t *restrict S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *restrict I_work,
    const int64_t *restrict K_work,
    const int64_t *restrict tstart_slice,
    const int64_t *restrict tnz_slice,
    int nthreads
) ;



GrB_Info GB_red_scalar__plus_fp32
(
    float *result,
    const GrB_Matrix A,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachvec__plus_fp32
(
    float *restrict Tx,
    GrB_Matrix A,
    const int64_t *restrict kfirst_slice,
    const int64_t *restrict klast_slice,
    const int64_t *restrict pstart_slice,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachindex__plus_fp32
(
    GrB_Matrix *Thandle,
    GrB_Type ttype,
    GrB_Matrix A,
    const int64_t *restrict pstart_slice,
    int nth,
    int nthreads,
    GB_Context Context
) ;



GrB_Info GB_red_build__plus_fp32
(
    float *restrict Tx,
    int64_t  *restrict Ti,
    const float *restrict S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *restrict I_work,
    const int64_t *restrict K_work,
    const int64_t *restrict tstart_slice,
    const int64_t *restrict tnz_slice,
    int nthreads
) ;



GrB_Info GB_red_scalar__plus_fp64
(
    double *result,
    const GrB_Matrix A,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachvec__plus_fp64
(
    double *restrict Tx,
    GrB_Matrix A,
    const int64_t *restrict kfirst_slice,
    const int64_t *restrict klast_slice,
    const int64_t *restrict pstart_slice,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachindex__plus_fp64
(
    GrB_Matrix *Thandle,
    GrB_Type ttype,
    GrB_Matrix A,
    const int64_t *restrict pstart_slice,
    int nth,
    int nthreads,
    GB_Context Context
) ;



GrB_Info GB_red_build__plus_fp64
(
    double *restrict Tx,
    int64_t  *restrict Ti,
    const double *restrict S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *restrict I_work,
    const int64_t *restrict K_work,
    const int64_t *restrict tstart_slice,
    const int64_t *restrict tnz_slice,
    int nthreads
) ;



GrB_Info GB_red_scalar__times_int8
(
    int8_t *result,
    const GrB_Matrix A,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachvec__times_int8
(
    int8_t *restrict Tx,
    GrB_Matrix A,
    const int64_t *restrict kfirst_slice,
    const int64_t *restrict klast_slice,
    const int64_t *restrict pstart_slice,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachindex__times_int8
(
    GrB_Matrix *Thandle,
    GrB_Type ttype,
    GrB_Matrix A,
    const int64_t *restrict pstart_slice,
    int nth,
    int nthreads,
    GB_Context Context
) ;



GrB_Info GB_red_build__times_int8
(
    int8_t *restrict Tx,
    int64_t  *restrict Ti,
    const int8_t *restrict S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *restrict I_work,
    const int64_t *restrict K_work,
    const int64_t *restrict tstart_slice,
    const int64_t *restrict tnz_slice,
    int nthreads
) ;



GrB_Info GB_red_scalar__times_int16
(
    int16_t *result,
    const GrB_Matrix A,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachvec__times_int16
(
    int16_t *restrict Tx,
    GrB_Matrix A,
    const int64_t *restrict kfirst_slice,
    const int64_t *restrict klast_slice,
    const int64_t *restrict pstart_slice,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachindex__times_int16
(
    GrB_Matrix *Thandle,
    GrB_Type ttype,
    GrB_Matrix A,
    const int64_t *restrict pstart_slice,
    int nth,
    int nthreads,
    GB_Context Context
) ;



GrB_Info GB_red_build__times_int16
(
    int16_t *restrict Tx,
    int64_t  *restrict Ti,
    const int16_t *restrict S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *restrict I_work,
    const int64_t *restrict K_work,
    const int64_t *restrict tstart_slice,
    const int64_t *restrict tnz_slice,
    int nthreads
) ;



GrB_Info GB_red_scalar__times_int32
(
    int32_t *result,
    const GrB_Matrix A,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachvec__times_int32
(
    int32_t *restrict Tx,
    GrB_Matrix A,
    const int64_t *restrict kfirst_slice,
    const int64_t *restrict klast_slice,
    const int64_t *restrict pstart_slice,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachindex__times_int32
(
    GrB_Matrix *Thandle,
    GrB_Type ttype,
    GrB_Matrix A,
    const int64_t *restrict pstart_slice,
    int nth,
    int nthreads,
    GB_Context Context
) ;



GrB_Info GB_red_build__times_int32
(
    int32_t *restrict Tx,
    int64_t  *restrict Ti,
    const int32_t *restrict S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *restrict I_work,
    const int64_t *restrict K_work,
    const int64_t *restrict tstart_slice,
    const int64_t *restrict tnz_slice,
    int nthreads
) ;



GrB_Info GB_red_scalar__times_int64
(
    int64_t *result,
    const GrB_Matrix A,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachvec__times_int64
(
    int64_t *restrict Tx,
    GrB_Matrix A,
    const int64_t *restrict kfirst_slice,
    const int64_t *restrict klast_slice,
    const int64_t *restrict pstart_slice,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachindex__times_int64
(
    GrB_Matrix *Thandle,
    GrB_Type ttype,
    GrB_Matrix A,
    const int64_t *restrict pstart_slice,
    int nth,
    int nthreads,
    GB_Context Context
) ;



GrB_Info GB_red_build__times_int64
(
    int64_t *restrict Tx,
    int64_t  *restrict Ti,
    const int64_t *restrict S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *restrict I_work,
    const int64_t *restrict K_work,
    const int64_t *restrict tstart_slice,
    const int64_t *restrict tnz_slice,
    int nthreads
) ;



GrB_Info GB_red_scalar__times_uint8
(
    uint8_t *result,
    const GrB_Matrix A,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachvec__times_uint8
(
    uint8_t *restrict Tx,
    GrB_Matrix A,
    const int64_t *restrict kfirst_slice,
    const int64_t *restrict klast_slice,
    const int64_t *restrict pstart_slice,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachindex__times_uint8
(
    GrB_Matrix *Thandle,
    GrB_Type ttype,
    GrB_Matrix A,
    const int64_t *restrict pstart_slice,
    int nth,
    int nthreads,
    GB_Context Context
) ;



GrB_Info GB_red_build__times_uint8
(
    uint8_t *restrict Tx,
    int64_t  *restrict Ti,
    const uint8_t *restrict S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *restrict I_work,
    const int64_t *restrict K_work,
    const int64_t *restrict tstart_slice,
    const int64_t *restrict tnz_slice,
    int nthreads
) ;



GrB_Info GB_red_scalar__times_uint16
(
    uint16_t *result,
    const GrB_Matrix A,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachvec__times_uint16
(
    uint16_t *restrict Tx,
    GrB_Matrix A,
    const int64_t *restrict kfirst_slice,
    const int64_t *restrict klast_slice,
    const int64_t *restrict pstart_slice,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachindex__times_uint16
(
    GrB_Matrix *Thandle,
    GrB_Type ttype,
    GrB_Matrix A,
    const int64_t *restrict pstart_slice,
    int nth,
    int nthreads,
    GB_Context Context
) ;



GrB_Info GB_red_build__times_uint16
(
    uint16_t *restrict Tx,
    int64_t  *restrict Ti,
    const uint16_t *restrict S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *restrict I_work,
    const int64_t *restrict K_work,
    const int64_t *restrict tstart_slice,
    const int64_t *restrict tnz_slice,
    int nthreads
) ;



GrB_Info GB_red_scalar__times_uint32
(
    uint32_t *result,
    const GrB_Matrix A,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachvec__times_uint32
(
    uint32_t *restrict Tx,
    GrB_Matrix A,
    const int64_t *restrict kfirst_slice,
    const int64_t *restrict klast_slice,
    const int64_t *restrict pstart_slice,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachindex__times_uint32
(
    GrB_Matrix *Thandle,
    GrB_Type ttype,
    GrB_Matrix A,
    const int64_t *restrict pstart_slice,
    int nth,
    int nthreads,
    GB_Context Context
) ;



GrB_Info GB_red_build__times_uint32
(
    uint32_t *restrict Tx,
    int64_t  *restrict Ti,
    const uint32_t *restrict S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *restrict I_work,
    const int64_t *restrict K_work,
    const int64_t *restrict tstart_slice,
    const int64_t *restrict tnz_slice,
    int nthreads
) ;



GrB_Info GB_red_scalar__times_uint64
(
    uint64_t *result,
    const GrB_Matrix A,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachvec__times_uint64
(
    uint64_t *restrict Tx,
    GrB_Matrix A,
    const int64_t *restrict kfirst_slice,
    const int64_t *restrict klast_slice,
    const int64_t *restrict pstart_slice,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachindex__times_uint64
(
    GrB_Matrix *Thandle,
    GrB_Type ttype,
    GrB_Matrix A,
    const int64_t *restrict pstart_slice,
    int nth,
    int nthreads,
    GB_Context Context
) ;



GrB_Info GB_red_build__times_uint64
(
    uint64_t *restrict Tx,
    int64_t  *restrict Ti,
    const uint64_t *restrict S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *restrict I_work,
    const int64_t *restrict K_work,
    const int64_t *restrict tstart_slice,
    const int64_t *restrict tnz_slice,
    int nthreads
) ;



GrB_Info GB_red_scalar__times_fp32
(
    float *result,
    const GrB_Matrix A,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachvec__times_fp32
(
    float *restrict Tx,
    GrB_Matrix A,
    const int64_t *restrict kfirst_slice,
    const int64_t *restrict klast_slice,
    const int64_t *restrict pstart_slice,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachindex__times_fp32
(
    GrB_Matrix *Thandle,
    GrB_Type ttype,
    GrB_Matrix A,
    const int64_t *restrict pstart_slice,
    int nth,
    int nthreads,
    GB_Context Context
) ;



GrB_Info GB_red_build__times_fp32
(
    float *restrict Tx,
    int64_t  *restrict Ti,
    const float *restrict S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *restrict I_work,
    const int64_t *restrict K_work,
    const int64_t *restrict tstart_slice,
    const int64_t *restrict tnz_slice,
    int nthreads
) ;



GrB_Info GB_red_scalar__times_fp64
(
    double *result,
    const GrB_Matrix A,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachvec__times_fp64
(
    double *restrict Tx,
    GrB_Matrix A,
    const int64_t *restrict kfirst_slice,
    const int64_t *restrict klast_slice,
    const int64_t *restrict pstart_slice,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachindex__times_fp64
(
    GrB_Matrix *Thandle,
    GrB_Type ttype,
    GrB_Matrix A,
    const int64_t *restrict pstart_slice,
    int nth,
    int nthreads,
    GB_Context Context
) ;



GrB_Info GB_red_build__times_fp64
(
    double *restrict Tx,
    int64_t  *restrict Ti,
    const double *restrict S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *restrict I_work,
    const int64_t *restrict K_work,
    const int64_t *restrict tstart_slice,
    const int64_t *restrict tnz_slice,
    int nthreads
) ;



GrB_Info GB_red_scalar__lor_bool
(
    bool *result,
    const GrB_Matrix A,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachvec__lor_bool
(
    bool *restrict Tx,
    GrB_Matrix A,
    const int64_t *restrict kfirst_slice,
    const int64_t *restrict klast_slice,
    const int64_t *restrict pstart_slice,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachindex__lor_bool
(
    GrB_Matrix *Thandle,
    GrB_Type ttype,
    GrB_Matrix A,
    const int64_t *restrict pstart_slice,
    int nth,
    int nthreads,
    GB_Context Context
) ;



GrB_Info GB_red_build__lor_bool
(
    bool *restrict Tx,
    int64_t  *restrict Ti,
    const bool *restrict S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *restrict I_work,
    const int64_t *restrict K_work,
    const int64_t *restrict tstart_slice,
    const int64_t *restrict tnz_slice,
    int nthreads
) ;



GrB_Info GB_red_scalar__land_bool
(
    bool *result,
    const GrB_Matrix A,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachvec__land_bool
(
    bool *restrict Tx,
    GrB_Matrix A,
    const int64_t *restrict kfirst_slice,
    const int64_t *restrict klast_slice,
    const int64_t *restrict pstart_slice,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachindex__land_bool
(
    GrB_Matrix *Thandle,
    GrB_Type ttype,
    GrB_Matrix A,
    const int64_t *restrict pstart_slice,
    int nth,
    int nthreads,
    GB_Context Context
) ;



GrB_Info GB_red_build__land_bool
(
    bool *restrict Tx,
    int64_t  *restrict Ti,
    const bool *restrict S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *restrict I_work,
    const int64_t *restrict K_work,
    const int64_t *restrict tstart_slice,
    const int64_t *restrict tnz_slice,
    int nthreads
) ;



GrB_Info GB_red_scalar__lxor_bool
(
    bool *result,
    const GrB_Matrix A,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachvec__lxor_bool
(
    bool *restrict Tx,
    GrB_Matrix A,
    const int64_t *restrict kfirst_slice,
    const int64_t *restrict klast_slice,
    const int64_t *restrict pstart_slice,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachindex__lxor_bool
(
    GrB_Matrix *Thandle,
    GrB_Type ttype,
    GrB_Matrix A,
    const int64_t *restrict pstart_slice,
    int nth,
    int nthreads,
    GB_Context Context
) ;



GrB_Info GB_red_build__lxor_bool
(
    bool *restrict Tx,
    int64_t  *restrict Ti,
    const bool *restrict S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *restrict I_work,
    const int64_t *restrict K_work,
    const int64_t *restrict tstart_slice,
    const int64_t *restrict tnz_slice,
    int nthreads
) ;



GrB_Info GB_red_scalar__eq_bool
(
    bool *result,
    const GrB_Matrix A,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachvec__eq_bool
(
    bool *restrict Tx,
    GrB_Matrix A,
    const int64_t *restrict kfirst_slice,
    const int64_t *restrict klast_slice,
    const int64_t *restrict pstart_slice,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachindex__eq_bool
(
    GrB_Matrix *Thandle,
    GrB_Type ttype,
    GrB_Matrix A,
    const int64_t *restrict pstart_slice,
    int nth,
    int nthreads,
    GB_Context Context
) ;



GrB_Info GB_red_build__eq_bool
(
    bool *restrict Tx,
    int64_t  *restrict Ti,
    const bool *restrict S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *restrict I_work,
    const int64_t *restrict K_work,
    const int64_t *restrict tstart_slice,
    const int64_t *restrict tnz_slice,
    int nthreads
) ;

#if 0

GrB_Info GB_red_scalar__(none)
(
    bool *result,
    const GrB_Matrix A,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachvec__(none)
(
    bool *restrict Tx,
    GrB_Matrix A,
    const int64_t *restrict kfirst_slice,
    const int64_t *restrict klast_slice,
    const int64_t *restrict pstart_slice,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachindex__(none)
(
    GrB_Matrix *Thandle,
    GrB_Type ttype,
    GrB_Matrix A,
    const int64_t *restrict pstart_slice,
    int nth,
    int nthreads,
    GB_Context Context
) ;

#endif

GrB_Info GB_red_build__first_bool
(
    bool *restrict Tx,
    int64_t  *restrict Ti,
    const bool *restrict S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *restrict I_work,
    const int64_t *restrict K_work,
    const int64_t *restrict tstart_slice,
    const int64_t *restrict tnz_slice,
    int nthreads
) ;

#if 0

GrB_Info GB_red_scalar__(none)
(
    int8_t *result,
    const GrB_Matrix A,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachvec__(none)
(
    int8_t *restrict Tx,
    GrB_Matrix A,
    const int64_t *restrict kfirst_slice,
    const int64_t *restrict klast_slice,
    const int64_t *restrict pstart_slice,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachindex__(none)
(
    GrB_Matrix *Thandle,
    GrB_Type ttype,
    GrB_Matrix A,
    const int64_t *restrict pstart_slice,
    int nth,
    int nthreads,
    GB_Context Context
) ;

#endif

GrB_Info GB_red_build__first_int8
(
    int8_t *restrict Tx,
    int64_t  *restrict Ti,
    const int8_t *restrict S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *restrict I_work,
    const int64_t *restrict K_work,
    const int64_t *restrict tstart_slice,
    const int64_t *restrict tnz_slice,
    int nthreads
) ;

#if 0

GrB_Info GB_red_scalar__(none)
(
    int16_t *result,
    const GrB_Matrix A,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachvec__(none)
(
    int16_t *restrict Tx,
    GrB_Matrix A,
    const int64_t *restrict kfirst_slice,
    const int64_t *restrict klast_slice,
    const int64_t *restrict pstart_slice,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachindex__(none)
(
    GrB_Matrix *Thandle,
    GrB_Type ttype,
    GrB_Matrix A,
    const int64_t *restrict pstart_slice,
    int nth,
    int nthreads,
    GB_Context Context
) ;

#endif

GrB_Info GB_red_build__first_int16
(
    int16_t *restrict Tx,
    int64_t  *restrict Ti,
    const int16_t *restrict S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *restrict I_work,
    const int64_t *restrict K_work,
    const int64_t *restrict tstart_slice,
    const int64_t *restrict tnz_slice,
    int nthreads
) ;

#if 0

GrB_Info GB_red_scalar__(none)
(
    int32_t *result,
    const GrB_Matrix A,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachvec__(none)
(
    int32_t *restrict Tx,
    GrB_Matrix A,
    const int64_t *restrict kfirst_slice,
    const int64_t *restrict klast_slice,
    const int64_t *restrict pstart_slice,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachindex__(none)
(
    GrB_Matrix *Thandle,
    GrB_Type ttype,
    GrB_Matrix A,
    const int64_t *restrict pstart_slice,
    int nth,
    int nthreads,
    GB_Context Context
) ;

#endif

GrB_Info GB_red_build__first_int32
(
    int32_t *restrict Tx,
    int64_t  *restrict Ti,
    const int32_t *restrict S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *restrict I_work,
    const int64_t *restrict K_work,
    const int64_t *restrict tstart_slice,
    const int64_t *restrict tnz_slice,
    int nthreads
) ;

#if 0

GrB_Info GB_red_scalar__(none)
(
    int64_t *result,
    const GrB_Matrix A,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachvec__(none)
(
    int64_t *restrict Tx,
    GrB_Matrix A,
    const int64_t *restrict kfirst_slice,
    const int64_t *restrict klast_slice,
    const int64_t *restrict pstart_slice,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachindex__(none)
(
    GrB_Matrix *Thandle,
    GrB_Type ttype,
    GrB_Matrix A,
    const int64_t *restrict pstart_slice,
    int nth,
    int nthreads,
    GB_Context Context
) ;

#endif

GrB_Info GB_red_build__first_int64
(
    int64_t *restrict Tx,
    int64_t  *restrict Ti,
    const int64_t *restrict S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *restrict I_work,
    const int64_t *restrict K_work,
    const int64_t *restrict tstart_slice,
    const int64_t *restrict tnz_slice,
    int nthreads
) ;

#if 0

GrB_Info GB_red_scalar__(none)
(
    uint8_t *result,
    const GrB_Matrix A,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachvec__(none)
(
    uint8_t *restrict Tx,
    GrB_Matrix A,
    const int64_t *restrict kfirst_slice,
    const int64_t *restrict klast_slice,
    const int64_t *restrict pstart_slice,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachindex__(none)
(
    GrB_Matrix *Thandle,
    GrB_Type ttype,
    GrB_Matrix A,
    const int64_t *restrict pstart_slice,
    int nth,
    int nthreads,
    GB_Context Context
) ;

#endif

GrB_Info GB_red_build__first_uint8
(
    uint8_t *restrict Tx,
    int64_t  *restrict Ti,
    const uint8_t *restrict S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *restrict I_work,
    const int64_t *restrict K_work,
    const int64_t *restrict tstart_slice,
    const int64_t *restrict tnz_slice,
    int nthreads
) ;

#if 0

GrB_Info GB_red_scalar__(none)
(
    uint16_t *result,
    const GrB_Matrix A,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachvec__(none)
(
    uint16_t *restrict Tx,
    GrB_Matrix A,
    const int64_t *restrict kfirst_slice,
    const int64_t *restrict klast_slice,
    const int64_t *restrict pstart_slice,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachindex__(none)
(
    GrB_Matrix *Thandle,
    GrB_Type ttype,
    GrB_Matrix A,
    const int64_t *restrict pstart_slice,
    int nth,
    int nthreads,
    GB_Context Context
) ;

#endif

GrB_Info GB_red_build__first_uint16
(
    uint16_t *restrict Tx,
    int64_t  *restrict Ti,
    const uint16_t *restrict S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *restrict I_work,
    const int64_t *restrict K_work,
    const int64_t *restrict tstart_slice,
    const int64_t *restrict tnz_slice,
    int nthreads
) ;

#if 0

GrB_Info GB_red_scalar__(none)
(
    uint32_t *result,
    const GrB_Matrix A,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachvec__(none)
(
    uint32_t *restrict Tx,
    GrB_Matrix A,
    const int64_t *restrict kfirst_slice,
    const int64_t *restrict klast_slice,
    const int64_t *restrict pstart_slice,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachindex__(none)
(
    GrB_Matrix *Thandle,
    GrB_Type ttype,
    GrB_Matrix A,
    const int64_t *restrict pstart_slice,
    int nth,
    int nthreads,
    GB_Context Context
) ;

#endif

GrB_Info GB_red_build__first_uint32
(
    uint32_t *restrict Tx,
    int64_t  *restrict Ti,
    const uint32_t *restrict S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *restrict I_work,
    const int64_t *restrict K_work,
    const int64_t *restrict tstart_slice,
    const int64_t *restrict tnz_slice,
    int nthreads
) ;

#if 0

GrB_Info GB_red_scalar__(none)
(
    uint64_t *result,
    const GrB_Matrix A,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachvec__(none)
(
    uint64_t *restrict Tx,
    GrB_Matrix A,
    const int64_t *restrict kfirst_slice,
    const int64_t *restrict klast_slice,
    const int64_t *restrict pstart_slice,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachindex__(none)
(
    GrB_Matrix *Thandle,
    GrB_Type ttype,
    GrB_Matrix A,
    const int64_t *restrict pstart_slice,
    int nth,
    int nthreads,
    GB_Context Context
) ;

#endif

GrB_Info GB_red_build__first_uint64
(
    uint64_t *restrict Tx,
    int64_t  *restrict Ti,
    const uint64_t *restrict S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *restrict I_work,
    const int64_t *restrict K_work,
    const int64_t *restrict tstart_slice,
    const int64_t *restrict tnz_slice,
    int nthreads
) ;

#if 0

GrB_Info GB_red_scalar__(none)
(
    float *result,
    const GrB_Matrix A,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachvec__(none)
(
    float *restrict Tx,
    GrB_Matrix A,
    const int64_t *restrict kfirst_slice,
    const int64_t *restrict klast_slice,
    const int64_t *restrict pstart_slice,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachindex__(none)
(
    GrB_Matrix *Thandle,
    GrB_Type ttype,
    GrB_Matrix A,
    const int64_t *restrict pstart_slice,
    int nth,
    int nthreads,
    GB_Context Context
) ;

#endif

GrB_Info GB_red_build__first_fp32
(
    float *restrict Tx,
    int64_t  *restrict Ti,
    const float *restrict S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *restrict I_work,
    const int64_t *restrict K_work,
    const int64_t *restrict tstart_slice,
    const int64_t *restrict tnz_slice,
    int nthreads
) ;

#if 0

GrB_Info GB_red_scalar__(none)
(
    double *result,
    const GrB_Matrix A,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachvec__(none)
(
    double *restrict Tx,
    GrB_Matrix A,
    const int64_t *restrict kfirst_slice,
    const int64_t *restrict klast_slice,
    const int64_t *restrict pstart_slice,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachindex__(none)
(
    GrB_Matrix *Thandle,
    GrB_Type ttype,
    GrB_Matrix A,
    const int64_t *restrict pstart_slice,
    int nth,
    int nthreads,
    GB_Context Context
) ;

#endif

GrB_Info GB_red_build__first_fp64
(
    double *restrict Tx,
    int64_t  *restrict Ti,
    const double *restrict S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *restrict I_work,
    const int64_t *restrict K_work,
    const int64_t *restrict tstart_slice,
    const int64_t *restrict tnz_slice,
    int nthreads
) ;

#if 0

GrB_Info GB_red_scalar__(none)
(
    bool *result,
    const GrB_Matrix A,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachvec__(none)
(
    bool *restrict Tx,
    GrB_Matrix A,
    const int64_t *restrict kfirst_slice,
    const int64_t *restrict klast_slice,
    const int64_t *restrict pstart_slice,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachindex__(none)
(
    GrB_Matrix *Thandle,
    GrB_Type ttype,
    GrB_Matrix A,
    const int64_t *restrict pstart_slice,
    int nth,
    int nthreads,
    GB_Context Context
) ;

#endif

GrB_Info GB_red_build__second_bool
(
    bool *restrict Tx,
    int64_t  *restrict Ti,
    const bool *restrict S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *restrict I_work,
    const int64_t *restrict K_work,
    const int64_t *restrict tstart_slice,
    const int64_t *restrict tnz_slice,
    int nthreads
) ;

#if 0

GrB_Info GB_red_scalar__(none)
(
    int8_t *result,
    const GrB_Matrix A,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachvec__(none)
(
    int8_t *restrict Tx,
    GrB_Matrix A,
    const int64_t *restrict kfirst_slice,
    const int64_t *restrict klast_slice,
    const int64_t *restrict pstart_slice,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachindex__(none)
(
    GrB_Matrix *Thandle,
    GrB_Type ttype,
    GrB_Matrix A,
    const int64_t *restrict pstart_slice,
    int nth,
    int nthreads,
    GB_Context Context
) ;

#endif

GrB_Info GB_red_build__second_int8
(
    int8_t *restrict Tx,
    int64_t  *restrict Ti,
    const int8_t *restrict S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *restrict I_work,
    const int64_t *restrict K_work,
    const int64_t *restrict tstart_slice,
    const int64_t *restrict tnz_slice,
    int nthreads
) ;

#if 0

GrB_Info GB_red_scalar__(none)
(
    int16_t *result,
    const GrB_Matrix A,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachvec__(none)
(
    int16_t *restrict Tx,
    GrB_Matrix A,
    const int64_t *restrict kfirst_slice,
    const int64_t *restrict klast_slice,
    const int64_t *restrict pstart_slice,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachindex__(none)
(
    GrB_Matrix *Thandle,
    GrB_Type ttype,
    GrB_Matrix A,
    const int64_t *restrict pstart_slice,
    int nth,
    int nthreads,
    GB_Context Context
) ;

#endif

GrB_Info GB_red_build__second_int16
(
    int16_t *restrict Tx,
    int64_t  *restrict Ti,
    const int16_t *restrict S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *restrict I_work,
    const int64_t *restrict K_work,
    const int64_t *restrict tstart_slice,
    const int64_t *restrict tnz_slice,
    int nthreads
) ;

#if 0

GrB_Info GB_red_scalar__(none)
(
    int32_t *result,
    const GrB_Matrix A,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachvec__(none)
(
    int32_t *restrict Tx,
    GrB_Matrix A,
    const int64_t *restrict kfirst_slice,
    const int64_t *restrict klast_slice,
    const int64_t *restrict pstart_slice,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachindex__(none)
(
    GrB_Matrix *Thandle,
    GrB_Type ttype,
    GrB_Matrix A,
    const int64_t *restrict pstart_slice,
    int nth,
    int nthreads,
    GB_Context Context
) ;

#endif

GrB_Info GB_red_build__second_int32
(
    int32_t *restrict Tx,
    int64_t  *restrict Ti,
    const int32_t *restrict S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *restrict I_work,
    const int64_t *restrict K_work,
    const int64_t *restrict tstart_slice,
    const int64_t *restrict tnz_slice,
    int nthreads
) ;

#if 0

GrB_Info GB_red_scalar__(none)
(
    int64_t *result,
    const GrB_Matrix A,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachvec__(none)
(
    int64_t *restrict Tx,
    GrB_Matrix A,
    const int64_t *restrict kfirst_slice,
    const int64_t *restrict klast_slice,
    const int64_t *restrict pstart_slice,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachindex__(none)
(
    GrB_Matrix *Thandle,
    GrB_Type ttype,
    GrB_Matrix A,
    const int64_t *restrict pstart_slice,
    int nth,
    int nthreads,
    GB_Context Context
) ;

#endif

GrB_Info GB_red_build__second_int64
(
    int64_t *restrict Tx,
    int64_t  *restrict Ti,
    const int64_t *restrict S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *restrict I_work,
    const int64_t *restrict K_work,
    const int64_t *restrict tstart_slice,
    const int64_t *restrict tnz_slice,
    int nthreads
) ;

#if 0

GrB_Info GB_red_scalar__(none)
(
    uint8_t *result,
    const GrB_Matrix A,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachvec__(none)
(
    uint8_t *restrict Tx,
    GrB_Matrix A,
    const int64_t *restrict kfirst_slice,
    const int64_t *restrict klast_slice,
    const int64_t *restrict pstart_slice,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachindex__(none)
(
    GrB_Matrix *Thandle,
    GrB_Type ttype,
    GrB_Matrix A,
    const int64_t *restrict pstart_slice,
    int nth,
    int nthreads,
    GB_Context Context
) ;

#endif

GrB_Info GB_red_build__second_uint8
(
    uint8_t *restrict Tx,
    int64_t  *restrict Ti,
    const uint8_t *restrict S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *restrict I_work,
    const int64_t *restrict K_work,
    const int64_t *restrict tstart_slice,
    const int64_t *restrict tnz_slice,
    int nthreads
) ;

#if 0

GrB_Info GB_red_scalar__(none)
(
    uint16_t *result,
    const GrB_Matrix A,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachvec__(none)
(
    uint16_t *restrict Tx,
    GrB_Matrix A,
    const int64_t *restrict kfirst_slice,
    const int64_t *restrict klast_slice,
    const int64_t *restrict pstart_slice,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachindex__(none)
(
    GrB_Matrix *Thandle,
    GrB_Type ttype,
    GrB_Matrix A,
    const int64_t *restrict pstart_slice,
    int nth,
    int nthreads,
    GB_Context Context
) ;

#endif

GrB_Info GB_red_build__second_uint16
(
    uint16_t *restrict Tx,
    int64_t  *restrict Ti,
    const uint16_t *restrict S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *restrict I_work,
    const int64_t *restrict K_work,
    const int64_t *restrict tstart_slice,
    const int64_t *restrict tnz_slice,
    int nthreads
) ;

#if 0

GrB_Info GB_red_scalar__(none)
(
    uint32_t *result,
    const GrB_Matrix A,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachvec__(none)
(
    uint32_t *restrict Tx,
    GrB_Matrix A,
    const int64_t *restrict kfirst_slice,
    const int64_t *restrict klast_slice,
    const int64_t *restrict pstart_slice,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachindex__(none)
(
    GrB_Matrix *Thandle,
    GrB_Type ttype,
    GrB_Matrix A,
    const int64_t *restrict pstart_slice,
    int nth,
    int nthreads,
    GB_Context Context
) ;

#endif

GrB_Info GB_red_build__second_uint32
(
    uint32_t *restrict Tx,
    int64_t  *restrict Ti,
    const uint32_t *restrict S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *restrict I_work,
    const int64_t *restrict K_work,
    const int64_t *restrict tstart_slice,
    const int64_t *restrict tnz_slice,
    int nthreads
) ;

#if 0

GrB_Info GB_red_scalar__(none)
(
    uint64_t *result,
    const GrB_Matrix A,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachvec__(none)
(
    uint64_t *restrict Tx,
    GrB_Matrix A,
    const int64_t *restrict kfirst_slice,
    const int64_t *restrict klast_slice,
    const int64_t *restrict pstart_slice,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachindex__(none)
(
    GrB_Matrix *Thandle,
    GrB_Type ttype,
    GrB_Matrix A,
    const int64_t *restrict pstart_slice,
    int nth,
    int nthreads,
    GB_Context Context
) ;

#endif

GrB_Info GB_red_build__second_uint64
(
    uint64_t *restrict Tx,
    int64_t  *restrict Ti,
    const uint64_t *restrict S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *restrict I_work,
    const int64_t *restrict K_work,
    const int64_t *restrict tstart_slice,
    const int64_t *restrict tnz_slice,
    int nthreads
) ;

#if 0

GrB_Info GB_red_scalar__(none)
(
    float *result,
    const GrB_Matrix A,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachvec__(none)
(
    float *restrict Tx,
    GrB_Matrix A,
    const int64_t *restrict kfirst_slice,
    const int64_t *restrict klast_slice,
    const int64_t *restrict pstart_slice,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachindex__(none)
(
    GrB_Matrix *Thandle,
    GrB_Type ttype,
    GrB_Matrix A,
    const int64_t *restrict pstart_slice,
    int nth,
    int nthreads,
    GB_Context Context
) ;

#endif

GrB_Info GB_red_build__second_fp32
(
    float *restrict Tx,
    int64_t  *restrict Ti,
    const float *restrict S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *restrict I_work,
    const int64_t *restrict K_work,
    const int64_t *restrict tstart_slice,
    const int64_t *restrict tnz_slice,
    int nthreads
) ;

#if 0

GrB_Info GB_red_scalar__(none)
(
    double *result,
    const GrB_Matrix A,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachvec__(none)
(
    double *restrict Tx,
    GrB_Matrix A,
    const int64_t *restrict kfirst_slice,
    const int64_t *restrict klast_slice,
    const int64_t *restrict pstart_slice,
    int ntasks,
    int nthreads
) ;

GrB_Info GB_red_eachindex__(none)
(
    GrB_Matrix *Thandle,
    GrB_Type ttype,
    GrB_Matrix A,
    const int64_t *restrict pstart_slice,
    int nth,
    int nthreads,
    GB_Context Context
) ;

#endif

GrB_Info GB_red_build__second_fp64
(
    double *restrict Tx,
    int64_t  *restrict Ti,
    const double *restrict S,
    int64_t nvals,
    int64_t ndupl,
    const int64_t *restrict I_work,
    const int64_t *restrict K_work,
    const int64_t *restrict tstart_slice,
    const int64_t *restrict tnz_slice,
    int nthreads
) ;

