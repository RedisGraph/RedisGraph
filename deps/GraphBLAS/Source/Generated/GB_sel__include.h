//------------------------------------------------------------------------------
// GB_sel__include.h: definitions for GB_sel__*.c
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txargt for license.

// This file has been automatically generated from Generator/GB_sel.h



void GB_sel_phase1__user_any
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const GB_void *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__user_any
(
    int64_t *GB_RESTRICT Ci,
    GB_void *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const GB_void *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__tril_any
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const GB_void *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__tril_any
(
    int64_t *GB_RESTRICT Ci,
    GB_void *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const GB_void *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__triu_any
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const GB_void *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__triu_any
(
    int64_t *GB_RESTRICT Ci,
    GB_void *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const GB_void *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__diag_any
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const GB_void *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__diag_any
(
    int64_t *GB_RESTRICT Ci,
    GB_void *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const GB_void *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__offdiag_any
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const GB_void *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__offdiag_any
(
    int64_t *GB_RESTRICT Ci,
    GB_void *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const GB_void *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__resize_any
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const GB_void *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__resize_any
(
    int64_t *GB_RESTRICT Ci,
    GB_void *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const GB_void *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;

#if 0

void GB_sel_phase1__(none)
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const bool *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;

#endif

void GB_sel_phase2__nonzombie_bool
(
    int64_t *GB_RESTRICT Ci,
    bool *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const bool *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;

#if 0

void GB_sel_phase1__(none)
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const int8_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;

#endif

void GB_sel_phase2__nonzombie_int8
(
    int64_t *GB_RESTRICT Ci,
    int8_t *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const int8_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;

#if 0

void GB_sel_phase1__(none)
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const int16_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;

#endif

void GB_sel_phase2__nonzombie_int16
(
    int64_t *GB_RESTRICT Ci,
    int16_t *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const int16_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;

#if 0

void GB_sel_phase1__(none)
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const int32_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;

#endif

void GB_sel_phase2__nonzombie_int32
(
    int64_t *GB_RESTRICT Ci,
    int32_t *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const int32_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;

#if 0

void GB_sel_phase1__(none)
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const int64_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;

#endif

void GB_sel_phase2__nonzombie_int64
(
    int64_t *GB_RESTRICT Ci,
    int64_t *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const int64_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;

#if 0

void GB_sel_phase1__(none)
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const uint8_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;

#endif

void GB_sel_phase2__nonzombie_uint8
(
    int64_t *GB_RESTRICT Ci,
    uint8_t *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const uint8_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;

#if 0

void GB_sel_phase1__(none)
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const uint16_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;

#endif

void GB_sel_phase2__nonzombie_uint16
(
    int64_t *GB_RESTRICT Ci,
    uint16_t *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const uint16_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;

#if 0

void GB_sel_phase1__(none)
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const uint32_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;

#endif

void GB_sel_phase2__nonzombie_uint32
(
    int64_t *GB_RESTRICT Ci,
    uint32_t *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const uint32_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;

#if 0

void GB_sel_phase1__(none)
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const uint64_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;

#endif

void GB_sel_phase2__nonzombie_uint64
(
    int64_t *GB_RESTRICT Ci,
    uint64_t *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const uint64_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;

#if 0

void GB_sel_phase1__(none)
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const float *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;

#endif

void GB_sel_phase2__nonzombie_fp32
(
    int64_t *GB_RESTRICT Ci,
    float *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const float *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;

#if 0

void GB_sel_phase1__(none)
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const double *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;

#endif

void GB_sel_phase2__nonzombie_fp64
(
    int64_t *GB_RESTRICT Ci,
    double *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const double *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__nonzombie_any
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const GB_void *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__nonzombie_any
(
    int64_t *GB_RESTRICT Ci,
    GB_void *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const GB_void *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__nonzero_bool
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const bool *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__nonzero_bool
(
    int64_t *GB_RESTRICT Ci,
    bool *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const bool *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__nonzero_int8
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const int8_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__nonzero_int8
(
    int64_t *GB_RESTRICT Ci,
    int8_t *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const int8_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__nonzero_int16
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const int16_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__nonzero_int16
(
    int64_t *GB_RESTRICT Ci,
    int16_t *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const int16_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__nonzero_int32
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const int32_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__nonzero_int32
(
    int64_t *GB_RESTRICT Ci,
    int32_t *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const int32_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__nonzero_int64
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const int64_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__nonzero_int64
(
    int64_t *GB_RESTRICT Ci,
    int64_t *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const int64_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__nonzero_uint8
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const uint8_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__nonzero_uint8
(
    int64_t *GB_RESTRICT Ci,
    uint8_t *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const uint8_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__nonzero_uint16
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const uint16_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__nonzero_uint16
(
    int64_t *GB_RESTRICT Ci,
    uint16_t *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const uint16_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__nonzero_uint32
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const uint32_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__nonzero_uint32
(
    int64_t *GB_RESTRICT Ci,
    uint32_t *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const uint32_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__nonzero_uint64
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const uint64_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__nonzero_uint64
(
    int64_t *GB_RESTRICT Ci,
    uint64_t *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const uint64_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__nonzero_fp32
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const float *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__nonzero_fp32
(
    int64_t *GB_RESTRICT Ci,
    float *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const float *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__nonzero_fp64
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const double *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__nonzero_fp64
(
    int64_t *GB_RESTRICT Ci,
    double *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const double *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__nonzero_any
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const GB_void *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__nonzero_any
(
    int64_t *GB_RESTRICT Ci,
    GB_void *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const GB_void *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__eq_zero_bool
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const bool *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__eq_zero_bool
(
    int64_t *GB_RESTRICT Ci,
    bool *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const bool *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__eq_zero_int8
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const int8_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__eq_zero_int8
(
    int64_t *GB_RESTRICT Ci,
    int8_t *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const int8_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__eq_zero_int16
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const int16_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__eq_zero_int16
(
    int64_t *GB_RESTRICT Ci,
    int16_t *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const int16_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__eq_zero_int32
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const int32_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__eq_zero_int32
(
    int64_t *GB_RESTRICT Ci,
    int32_t *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const int32_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__eq_zero_int64
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const int64_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__eq_zero_int64
(
    int64_t *GB_RESTRICT Ci,
    int64_t *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const int64_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__eq_zero_uint8
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const uint8_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__eq_zero_uint8
(
    int64_t *GB_RESTRICT Ci,
    uint8_t *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const uint8_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__eq_zero_uint16
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const uint16_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__eq_zero_uint16
(
    int64_t *GB_RESTRICT Ci,
    uint16_t *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const uint16_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__eq_zero_uint32
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const uint32_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__eq_zero_uint32
(
    int64_t *GB_RESTRICT Ci,
    uint32_t *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const uint32_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__eq_zero_uint64
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const uint64_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__eq_zero_uint64
(
    int64_t *GB_RESTRICT Ci,
    uint64_t *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const uint64_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__eq_zero_fp32
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const float *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__eq_zero_fp32
(
    int64_t *GB_RESTRICT Ci,
    float *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const float *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__eq_zero_fp64
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const double *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__eq_zero_fp64
(
    int64_t *GB_RESTRICT Ci,
    double *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const double *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__eq_zero_any
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const GB_void *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__eq_zero_any
(
    int64_t *GB_RESTRICT Ci,
    GB_void *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const GB_void *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__gt_zero_int8
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const int8_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__gt_zero_int8
(
    int64_t *GB_RESTRICT Ci,
    int8_t *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const int8_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__gt_zero_int16
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const int16_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__gt_zero_int16
(
    int64_t *GB_RESTRICT Ci,
    int16_t *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const int16_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__gt_zero_int32
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const int32_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__gt_zero_int32
(
    int64_t *GB_RESTRICT Ci,
    int32_t *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const int32_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__gt_zero_int64
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const int64_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__gt_zero_int64
(
    int64_t *GB_RESTRICT Ci,
    int64_t *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const int64_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__gt_zero_fp32
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const float *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__gt_zero_fp32
(
    int64_t *GB_RESTRICT Ci,
    float *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const float *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__gt_zero_fp64
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const double *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__gt_zero_fp64
(
    int64_t *GB_RESTRICT Ci,
    double *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const double *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__ge_zero_int8
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const int8_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__ge_zero_int8
(
    int64_t *GB_RESTRICT Ci,
    int8_t *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const int8_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__ge_zero_int16
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const int16_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__ge_zero_int16
(
    int64_t *GB_RESTRICT Ci,
    int16_t *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const int16_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__ge_zero_int32
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const int32_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__ge_zero_int32
(
    int64_t *GB_RESTRICT Ci,
    int32_t *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const int32_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__ge_zero_int64
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const int64_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__ge_zero_int64
(
    int64_t *GB_RESTRICT Ci,
    int64_t *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const int64_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__ge_zero_fp32
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const float *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__ge_zero_fp32
(
    int64_t *GB_RESTRICT Ci,
    float *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const float *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__ge_zero_fp64
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const double *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__ge_zero_fp64
(
    int64_t *GB_RESTRICT Ci,
    double *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const double *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__lt_zero_int8
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const int8_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__lt_zero_int8
(
    int64_t *GB_RESTRICT Ci,
    int8_t *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const int8_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__lt_zero_int16
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const int16_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__lt_zero_int16
(
    int64_t *GB_RESTRICT Ci,
    int16_t *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const int16_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__lt_zero_int32
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const int32_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__lt_zero_int32
(
    int64_t *GB_RESTRICT Ci,
    int32_t *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const int32_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__lt_zero_int64
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const int64_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__lt_zero_int64
(
    int64_t *GB_RESTRICT Ci,
    int64_t *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const int64_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__lt_zero_fp32
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const float *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__lt_zero_fp32
(
    int64_t *GB_RESTRICT Ci,
    float *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const float *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__lt_zero_fp64
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const double *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__lt_zero_fp64
(
    int64_t *GB_RESTRICT Ci,
    double *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const double *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__le_zero_int8
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const int8_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__le_zero_int8
(
    int64_t *GB_RESTRICT Ci,
    int8_t *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const int8_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__le_zero_int16
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const int16_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__le_zero_int16
(
    int64_t *GB_RESTRICT Ci,
    int16_t *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const int16_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__le_zero_int32
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const int32_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__le_zero_int32
(
    int64_t *GB_RESTRICT Ci,
    int32_t *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const int32_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__le_zero_int64
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const int64_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__le_zero_int64
(
    int64_t *GB_RESTRICT Ci,
    int64_t *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const int64_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__le_zero_fp32
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const float *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__le_zero_fp32
(
    int64_t *GB_RESTRICT Ci,
    float *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const float *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__le_zero_fp64
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const double *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__le_zero_fp64
(
    int64_t *GB_RESTRICT Ci,
    double *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const double *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__ne_thunk_int8
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const int8_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__ne_thunk_int8
(
    int64_t *GB_RESTRICT Ci,
    int8_t *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const int8_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__ne_thunk_int16
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const int16_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__ne_thunk_int16
(
    int64_t *GB_RESTRICT Ci,
    int16_t *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const int16_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__ne_thunk_int32
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const int32_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__ne_thunk_int32
(
    int64_t *GB_RESTRICT Ci,
    int32_t *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const int32_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__ne_thunk_int64
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const int64_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__ne_thunk_int64
(
    int64_t *GB_RESTRICT Ci,
    int64_t *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const int64_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__ne_thunk_uint8
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const uint8_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__ne_thunk_uint8
(
    int64_t *GB_RESTRICT Ci,
    uint8_t *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const uint8_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__ne_thunk_uint16
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const uint16_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__ne_thunk_uint16
(
    int64_t *GB_RESTRICT Ci,
    uint16_t *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const uint16_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__ne_thunk_uint32
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const uint32_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__ne_thunk_uint32
(
    int64_t *GB_RESTRICT Ci,
    uint32_t *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const uint32_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__ne_thunk_uint64
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const uint64_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__ne_thunk_uint64
(
    int64_t *GB_RESTRICT Ci,
    uint64_t *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const uint64_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__ne_thunk_fp32
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const float *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__ne_thunk_fp32
(
    int64_t *GB_RESTRICT Ci,
    float *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const float *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__ne_thunk_fp64
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const double *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__ne_thunk_fp64
(
    int64_t *GB_RESTRICT Ci,
    double *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const double *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__ne_thunk_any
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const GB_void *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__ne_thunk_any
(
    int64_t *GB_RESTRICT Ci,
    GB_void *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const GB_void *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__eq_thunk_int8
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const int8_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__eq_thunk_int8
(
    int64_t *GB_RESTRICT Ci,
    int8_t *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const int8_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__eq_thunk_int16
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const int16_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__eq_thunk_int16
(
    int64_t *GB_RESTRICT Ci,
    int16_t *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const int16_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__eq_thunk_int32
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const int32_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__eq_thunk_int32
(
    int64_t *GB_RESTRICT Ci,
    int32_t *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const int32_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__eq_thunk_int64
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const int64_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__eq_thunk_int64
(
    int64_t *GB_RESTRICT Ci,
    int64_t *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const int64_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__eq_thunk_uint8
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const uint8_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__eq_thunk_uint8
(
    int64_t *GB_RESTRICT Ci,
    uint8_t *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const uint8_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__eq_thunk_uint16
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const uint16_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__eq_thunk_uint16
(
    int64_t *GB_RESTRICT Ci,
    uint16_t *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const uint16_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__eq_thunk_uint32
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const uint32_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__eq_thunk_uint32
(
    int64_t *GB_RESTRICT Ci,
    uint32_t *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const uint32_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__eq_thunk_uint64
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const uint64_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__eq_thunk_uint64
(
    int64_t *GB_RESTRICT Ci,
    uint64_t *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const uint64_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__eq_thunk_fp32
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const float *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__eq_thunk_fp32
(
    int64_t *GB_RESTRICT Ci,
    float *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const float *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__eq_thunk_fp64
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const double *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__eq_thunk_fp64
(
    int64_t *GB_RESTRICT Ci,
    double *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const double *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__eq_thunk_any
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const GB_void *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__eq_thunk_any
(
    int64_t *GB_RESTRICT Ci,
    GB_void *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const GB_void *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__gt_thunk_int8
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const int8_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__gt_thunk_int8
(
    int64_t *GB_RESTRICT Ci,
    int8_t *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const int8_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__gt_thunk_int16
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const int16_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__gt_thunk_int16
(
    int64_t *GB_RESTRICT Ci,
    int16_t *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const int16_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__gt_thunk_int32
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const int32_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__gt_thunk_int32
(
    int64_t *GB_RESTRICT Ci,
    int32_t *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const int32_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__gt_thunk_int64
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const int64_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__gt_thunk_int64
(
    int64_t *GB_RESTRICT Ci,
    int64_t *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const int64_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__gt_thunk_uint8
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const uint8_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__gt_thunk_uint8
(
    int64_t *GB_RESTRICT Ci,
    uint8_t *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const uint8_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__gt_thunk_uint16
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const uint16_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__gt_thunk_uint16
(
    int64_t *GB_RESTRICT Ci,
    uint16_t *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const uint16_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__gt_thunk_uint32
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const uint32_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__gt_thunk_uint32
(
    int64_t *GB_RESTRICT Ci,
    uint32_t *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const uint32_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__gt_thunk_uint64
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const uint64_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__gt_thunk_uint64
(
    int64_t *GB_RESTRICT Ci,
    uint64_t *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const uint64_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__gt_thunk_fp32
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const float *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__gt_thunk_fp32
(
    int64_t *GB_RESTRICT Ci,
    float *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const float *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__gt_thunk_fp64
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const double *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__gt_thunk_fp64
(
    int64_t *GB_RESTRICT Ci,
    double *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const double *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__ge_thunk_int8
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const int8_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__ge_thunk_int8
(
    int64_t *GB_RESTRICT Ci,
    int8_t *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const int8_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__ge_thunk_int16
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const int16_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__ge_thunk_int16
(
    int64_t *GB_RESTRICT Ci,
    int16_t *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const int16_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__ge_thunk_int32
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const int32_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__ge_thunk_int32
(
    int64_t *GB_RESTRICT Ci,
    int32_t *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const int32_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__ge_thunk_int64
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const int64_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__ge_thunk_int64
(
    int64_t *GB_RESTRICT Ci,
    int64_t *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const int64_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__ge_thunk_uint8
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const uint8_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__ge_thunk_uint8
(
    int64_t *GB_RESTRICT Ci,
    uint8_t *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const uint8_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__ge_thunk_uint16
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const uint16_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__ge_thunk_uint16
(
    int64_t *GB_RESTRICT Ci,
    uint16_t *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const uint16_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__ge_thunk_uint32
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const uint32_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__ge_thunk_uint32
(
    int64_t *GB_RESTRICT Ci,
    uint32_t *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const uint32_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__ge_thunk_uint64
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const uint64_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__ge_thunk_uint64
(
    int64_t *GB_RESTRICT Ci,
    uint64_t *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const uint64_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__ge_thunk_fp32
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const float *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__ge_thunk_fp32
(
    int64_t *GB_RESTRICT Ci,
    float *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const float *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__ge_thunk_fp64
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const double *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__ge_thunk_fp64
(
    int64_t *GB_RESTRICT Ci,
    double *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const double *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__lt_thunk_int8
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const int8_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__lt_thunk_int8
(
    int64_t *GB_RESTRICT Ci,
    int8_t *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const int8_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__lt_thunk_int16
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const int16_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__lt_thunk_int16
(
    int64_t *GB_RESTRICT Ci,
    int16_t *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const int16_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__lt_thunk_int32
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const int32_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__lt_thunk_int32
(
    int64_t *GB_RESTRICT Ci,
    int32_t *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const int32_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__lt_thunk_int64
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const int64_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__lt_thunk_int64
(
    int64_t *GB_RESTRICT Ci,
    int64_t *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const int64_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__lt_thunk_uint8
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const uint8_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__lt_thunk_uint8
(
    int64_t *GB_RESTRICT Ci,
    uint8_t *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const uint8_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__lt_thunk_uint16
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const uint16_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__lt_thunk_uint16
(
    int64_t *GB_RESTRICT Ci,
    uint16_t *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const uint16_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__lt_thunk_uint32
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const uint32_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__lt_thunk_uint32
(
    int64_t *GB_RESTRICT Ci,
    uint32_t *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const uint32_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__lt_thunk_uint64
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const uint64_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__lt_thunk_uint64
(
    int64_t *GB_RESTRICT Ci,
    uint64_t *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const uint64_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__lt_thunk_fp32
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const float *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__lt_thunk_fp32
(
    int64_t *GB_RESTRICT Ci,
    float *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const float *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__lt_thunk_fp64
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const double *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__lt_thunk_fp64
(
    int64_t *GB_RESTRICT Ci,
    double *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const double *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__le_thunk_int8
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const int8_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__le_thunk_int8
(
    int64_t *GB_RESTRICT Ci,
    int8_t *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const int8_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__le_thunk_int16
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const int16_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__le_thunk_int16
(
    int64_t *GB_RESTRICT Ci,
    int16_t *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const int16_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__le_thunk_int32
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const int32_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__le_thunk_int32
(
    int64_t *GB_RESTRICT Ci,
    int32_t *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const int32_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__le_thunk_int64
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const int64_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__le_thunk_int64
(
    int64_t *GB_RESTRICT Ci,
    int64_t *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const int64_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__le_thunk_uint8
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const uint8_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__le_thunk_uint8
(
    int64_t *GB_RESTRICT Ci,
    uint8_t *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const uint8_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__le_thunk_uint16
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const uint16_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__le_thunk_uint16
(
    int64_t *GB_RESTRICT Ci,
    uint16_t *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const uint16_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__le_thunk_uint32
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const uint32_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__le_thunk_uint32
(
    int64_t *GB_RESTRICT Ci,
    uint32_t *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const uint32_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__le_thunk_uint64
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const uint64_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__le_thunk_uint64
(
    int64_t *GB_RESTRICT Ci,
    uint64_t *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const uint64_t *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__le_thunk_fp32
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const float *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__le_thunk_fp32
(
    int64_t *GB_RESTRICT Ci,
    float *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const float *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase1__le_thunk_fp64
(
    int64_t *GB_RESTRICT Zp,
    int64_t *GB_RESTRICT Cp,
    GB_void *GB_RESTRICT Wfirst_space,
    GB_void *GB_RESTRICT Wlast_space,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const double *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;



void GB_sel_phase2__le_thunk_fp64
(
    int64_t *GB_RESTRICT Ci,
    double *GB_RESTRICT Cx,
    const int64_t *GB_RESTRICT Zp,
    const int64_t *GB_RESTRICT Cp,
    const int64_t *GB_RESTRICT C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *GB_RESTRICT kfirst_slice,
    const int64_t *GB_RESTRICT klast_slice,
    const int64_t *GB_RESTRICT pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const double *GB_RESTRICT xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
) ;

