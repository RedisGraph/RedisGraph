//------------------------------------------------------------------------------
// GB_AxB__include1.h: definitions for Generated1/GB_AxB__*.c
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

// This file has been automatically generated from Generator/GB_AxB.h

// SPDX-License-Identifier: Apache-2.0
GrB_Info GB (_Adot2B__any_pair_iso)
(
    GrB_Matrix C,
    const GrB_Matrix M, const bool Mask_comp, const bool Mask_struct,
    const GrB_Matrix A, bool A_is_pattern, int64_t *restrict A_slice,
    const GrB_Matrix B, bool B_is_pattern, int64_t *restrict B_slice,
    int nthreads, int naslice, int nbslice
) ;

GrB_Info GB (_Adot3B__any_pair_iso)
(
    GrB_Matrix C,
    const GrB_Matrix M, const bool Mask_struct,
    const GrB_Matrix A, bool A_is_pattern,
    const GrB_Matrix B, bool B_is_pattern,
    const GB_task_struct *restrict TaskList,
    const int ntasks,
    const int nthreads
) ;

#if 0
GrB_Info GB (_Adot4B__any_pair_iso)
(
    GrB_Matrix C, const bool C_in_iso, const GB_void *cinput_void,
    const GrB_Matrix A, bool A_is_pattern,
    int64_t *restrict A_slice, int naslice,
    const GrB_Matrix B, bool B_is_pattern,
    int64_t *restrict B_slice, int nbslice,
    const int nthreads
) ;
#endif

GrB_Info GB (_Asaxpy3B__any_pair_iso)
(
    GrB_Matrix C,   // C<any M>=A*B, C sparse or hypersparse
    const GrB_Matrix M, const bool Mask_comp, const bool Mask_struct,
    const bool M_in_place,
    const GrB_Matrix A, bool A_is_pattern,
    const GrB_Matrix B, bool B_is_pattern,
    GB_saxpy3task_struct *restrict SaxpyTasks,
    const int ntasks, const int nfine, const int nthreads, const int do_sort,
    GB_Context Context
) ;

GrB_Info GB (_Asaxpy3B_noM__any_pair_iso)
(
    GrB_Matrix C,   // C=A*B, C sparse or hypersparse
    const GrB_Matrix A, bool A_is_pattern,
    const GrB_Matrix B, bool B_is_pattern,
    GB_saxpy3task_struct *restrict SaxpyTasks,
    const int ntasks, const int nfine, const int nthreads,
    const int do_sort,
    GB_Context Context
) ;

GrB_Info GB (_Asaxpy3B_M__any_pair_iso)
(
    GrB_Matrix C,   // C<M>=A*B, C sparse or hypersparse
    const GrB_Matrix M, const bool Mask_struct, const bool M_in_place,
    const GrB_Matrix A, bool A_is_pattern,
    const GrB_Matrix B, bool B_is_pattern,
    GB_saxpy3task_struct *restrict SaxpyTasks,
    const int ntasks, const int nfine, const int nthreads,
    const int do_sort,
    GB_Context Context
) ;

GrB_Info GB (_Asaxpy3B_notM__any_pair_iso)
(
    GrB_Matrix C,   // C<!M>=A*B, C sparse or hypersparse
    const GrB_Matrix M, const bool Mask_struct, const bool M_in_place,
    const GrB_Matrix A, bool A_is_pattern,
    const GrB_Matrix B, bool B_is_pattern,
    GB_saxpy3task_struct *restrict SaxpyTasks,
    const int ntasks, const int nfine, const int nthreads,
    const int do_sort,
    GB_Context Context
) ;

GrB_Info GB (_AsaxbitB__any_pair_iso)
(
    GrB_Matrix C,   // C<any M>=A*B, C bitmap or full
    const GrB_Matrix M, const bool Mask_comp, const bool Mask_struct,
    const GrB_Matrix A, bool A_is_pattern,
    const GrB_Matrix B, bool B_is_pattern,
    GB_Context Context
) ;

