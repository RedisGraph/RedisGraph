//------------------------------------------------------------------------------
// GB_bitmap_selector:  select entries from a bitmap or full matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#include "GB_select.h"
#include "GB_sel__include.h"

#define GB_FREE_ALL ;

GrB_Info GB_bitmap_selector
(
    GrB_Matrix C,               // output matrix, static header
    const bool C_iso,           // if true, C is iso
    GB_Select_Opcode opcode,    // selector opcode
    const GxB_select_function user_select,      // user select function
    const bool flipij,          // if true, flip i and j for user operator
    GrB_Matrix A,               // input matrix
    const int64_t ithunk,       // (int64_t) Thunk, if Thunk is NULL
    const GB_void *restrict xthunk,
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GrB_Info info ;
    ASSERT_MATRIX_OK (A, "A for bitmap selector", GB0) ;
    ASSERT (GB_IS_BITMAP (A) || GB_as_if_full (A)) ;
    ASSERT (opcode != GB_RESIZE_opcode) ;
    ASSERT (opcode != GB_NONZOMBIE_opcode) ;
    ASSERT (C != NULL && C->static_header) ;

    //--------------------------------------------------------------------------
    // get A
    //--------------------------------------------------------------------------

    int64_t anz = GB_nnz_held (A) ;
    const size_t asize = A->type->size ;
    const GB_Type_code acode = A->type->code ;

    //--------------------------------------------------------------------------
    // allocate C
    //--------------------------------------------------------------------------

    // C->b and C->x are malloc'd, not calloc'd
    // set C->iso = C_iso   OK
    GB_OK (GB_new_bix (&C, true, // always bitmap, static header
        A->type, A->vlen, A->vdim, GB_Ap_calloc, true,
        GxB_BITMAP, false, A->hyper_switch, -1, anz, true, C_iso, Context)) ;
    int64_t cnvals ;

    //--------------------------------------------------------------------------
    // determine the number of threads to use
    //--------------------------------------------------------------------------

    GB_GET_NTHREADS_MAX (nthreads_max, chunk, Context) ;
    int nthreads = GB_nthreads (anz, chunk, nthreads_max) ;

    //--------------------------------------------------------------------------
    // set the iso value of C
    //--------------------------------------------------------------------------

    if (C_iso)
    { 
        GB_iso_select (C->x, opcode, xthunk, A->x, acode, asize) ;
    }

    //--------------------------------------------------------------------------
    // launch the switch factory to select the entries
    //--------------------------------------------------------------------------

    #define GB_BITMAP_SELECTOR
    #define GB_selbit(opname,aname) GB (_sel_bitmap_ ## opname ## aname)
    #define GB_SEL_WORKER(opname,aname,atype)                           \
    {                                                                   \
        GB_selbit (opname, aname) (C->b, (atype *) C->x, &cnvals, A,    \
            flipij, ithunk, (atype *) xthunk, user_select, nthreads) ;  \
    }                                                                   \
    break ;

    const GB_Type_code typecode = (A->iso) ? GB_ignore_code : acode ;
    #include "GB_select_factory.c"

    //--------------------------------------------------------------------------
    // return result
    //--------------------------------------------------------------------------

    C->nvals = cnvals ;
    C->magic = GB_MAGIC ;
    ASSERT_MATRIX_OK (C, "C from bitmap selector", GB0) ;
    return (GrB_SUCCESS) ;
}

