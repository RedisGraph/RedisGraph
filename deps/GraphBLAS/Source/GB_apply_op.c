//------------------------------------------------------------------------------
// GB_apply_op: typecast and apply a unary operator to an array
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Cx = op ((xtype) Ax)

// Compare with GB_transpose_op.c

#include "GB_apply.h"
#ifndef GBCOMPACT
#include "GB_iterator.h"
#include "GB_unaryop__include.h"
#endif

void GB_apply_op            // apply a unary operator, Cx = op ((xtype) Ax)
(
    GB_void *restrict Cx,           // output array, of type op->ztype
    const GrB_UnaryOp op,           // operator to apply
    const GB_void *restrict Ax,     // input array, of type Atype
    const GrB_Type Atype,           // type of Ax
    const int64_t anz,              // size of Ax and Cx
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (Cx != NULL) ;
    ASSERT (Ax != NULL) ;
    ASSERT (anz >= 0) ;
    ASSERT (Atype != NULL) ;
    ASSERT (op != NULL) ;

    //--------------------------------------------------------------------------
    // determine the number of threads to use
    //--------------------------------------------------------------------------

    GB_GET_NTHREADS_MAX (nthreads_max, chunk, Context) ;
    int nthreads = GB_nthreads (anz, chunk, nthreads_max) ;

    //--------------------------------------------------------------------------
    // define the worker for the switch factory
    //--------------------------------------------------------------------------

    // FUTURE:: these operators could be renamed:
    // GrB_AINV_BOOL and GxB_ABS_BOOL to GrB_IDENTITY_BOOL.
    // GrB_MINV_BOOL to GxB_ONE_BOOL.
    // GxB_ABS_UINT* to GrB_IDENTITY_UINT*.
    // and then these workers would not need to be created.

    #define GB_unop(op,zname,aname) GB_unop_ ## op ## zname ## aname

    #define GB_WORKER(op,zname,ztype,aname,atype)                           \
    {                                                                       \
        GrB_Info info = GB_unop (op,zname,aname) ((ztype *) Cx,             \
            (const atype *) Ax, anz, nthreads) ;                            \
        if (info == GrB_SUCCESS) return ;                                   \
    }                                                                       \
    break ;

    //--------------------------------------------------------------------------
    // launch the switch factory
    //--------------------------------------------------------------------------

    #ifndef GBCOMPACT
    #include "GB_unaryop_factory.c"
    #endif

    //--------------------------------------------------------------------------
    // generic worker: typecast and apply an operator
    //--------------------------------------------------------------------------

    size_t asize = Atype->size ;
    size_t zsize = op->ztype->size ;
    size_t xsize = op->xtype->size ;
    GB_cast_function
        cast_A_to_X = GB_cast_factory (op->xtype->code, Atype->code) ;
    GxB_unary_function fop = op->function ;

    #pragma omp parallel for num_threads(nthreads) schedule(static)
    for (int64_t p = 0 ; p < anz ; p++)
    { 
        // xwork = (xtype) Ax [p]
        GB_void xwork [xsize] ;
        cast_A_to_X (xwork, Ax +(p*asize), asize) ;
        // Cx [p] = fop (xwork)
        fop (Cx +(p*zsize), xwork) ;
    }
}

