//------------------------------------------------------------------------------
// GB_msort_3b.h: definitions for GB_msort_3b.c
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// A parallel mergesort of an array of 3-by-n integers.  Each key consists
// of three integers.

#include "GB_sort.h"

//------------------------------------------------------------------------------
// prototypes only needed for GB_msort_3b
//------------------------------------------------------------------------------

void GB_msort_3b_create_merge_tasks
(
    // output:
    int64_t *GB_RESTRICT L_task,        // L_task [t0...t0+ntasks-1] computed
    int64_t *GB_RESTRICT L_len,         // L_len  [t0...t0+ntasks-1] computed
    int64_t *GB_RESTRICT R_task,        // R_task [t0...t0+ntasks-1] computed
    int64_t *GB_RESTRICT R_len,         // R_len  [t0...t0+ntasks-1] computed
    int64_t *GB_RESTRICT S_task,        // S_task [t0...t0+ntasks-1] computed
    // input:
    const int t0,                       // first task tid to create
    const int ntasks,                   // # of tasks to create
    const int64_t pS_start,             // merge into S [pS_start...]
    const int64_t *GB_RESTRICT L_0,     // Left = L [pL_start...pL_end-1]
    const int64_t *GB_RESTRICT L_1,
    const int64_t *GB_RESTRICT L_2,
    const int64_t pL_start,
    const int64_t pL_end,
    const int64_t *GB_RESTRICT R_0,     // Right = R [pR_start...pR_end-1]
    const int64_t *GB_RESTRICT R_1,
    const int64_t *GB_RESTRICT R_2,
    const int64_t pR_start,
    const int64_t pR_end
) ;

