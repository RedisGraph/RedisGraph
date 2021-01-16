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
    GrB_Matrix *Chandle,        // output matrix, never NULL
    GB_Select_Opcode opcode,    // selector opcode
    const GxB_select_function user_select,      // user select function
    const bool flipij,          // if true, flip i and j for user operator
    GrB_Matrix A,               // input matrix
    const int64_t ithunk,       // (int64_t) Thunk, if Thunk is NULL
    const GB_void *GB_RESTRICT xthunk,
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GrB_Info info ;
    ASSERT_MATRIX_OK (A, "A for bitmap selector", GB0) ;
    ASSERT (GB_is_packed (A)) ;
    ASSERT (opcode != GB_RESIZE_opcode) ;
    ASSERT (opcode != GB_NONZOMBIE_opcode) ;

    // Only GB_Matrix_wait and GB_resize pass in Chandle as NULL, and they
    // do not operate on bitmap matrices.  So for the bitmap case, Chandle
    // is never NULL.
    ASSERT (Chandle != NULL) ;

    //--------------------------------------------------------------------------
    // get A
    //--------------------------------------------------------------------------

    int64_t anz = GB_NNZ_HELD (A) ;
    const GB_Type_code typecode = A->type->code ;

    //--------------------------------------------------------------------------
    // allocate C
    //--------------------------------------------------------------------------

    // C->b and C->x are malloc'd, not calloc'd
    GrB_Matrix C = NULL ;
    GB_OK (GB_new_bix (&C, // always bitmap, new header
        A->type, A->vlen, A->vdim, GB_Ap_calloc, true,
        GxB_BITMAP, false, A->hyper_switch, -1, anz, true, Context)) ;
    int64_t cnvals ;

    //--------------------------------------------------------------------------
    // determine the number of threads to use
    //--------------------------------------------------------------------------

    GB_GET_NTHREADS_MAX (nthreads_max, chunk, Context) ;
    int nthreads = GB_nthreads (anz, chunk, nthreads_max) ;

    //--------------------------------------------------------------------------
    // clear C for the EQ_ZERO opcode
    //--------------------------------------------------------------------------

    // All other opcodes set C->x in the worker below
    if (opcode == GB_EQ_ZERO_opcode)
    { 
        GB_memset (C->x, 0, anz * A->type->size, nthreads_max) ;
    }

    //--------------------------------------------------------------------------
    // launch the switch factory to select the entries
    //--------------------------------------------------------------------------

    #define GB_BITMAP_SELECTOR
    #define GB_selbit(opname,aname) GB_sel_bitmap_ ## opname ## aname
    #define GB_SEL_WORKER(opname,aname,atype)                           \
    {                                                                   \
        GB_selbit (opname, aname) (C->b, C->x, &cnvals, A, flipij,      \
            ithunk, (atype *) xthunk, user_select, nthreads) ;          \
    }                                                                   \
    break ;
    #include "GB_select_factory.c"

    //--------------------------------------------------------------------------
    // return result
    //--------------------------------------------------------------------------

    (*Chandle) = C ;
    C->nvals = cnvals ;
    C->magic = GB_MAGIC ;
    ASSERT_MATRIX_OK (C, "C from bitmap selector", GB0) ;
    return (GrB_SUCCESS) ;
}

