//------------------------------------------------------------------------------
// GB_iso_reduce_to_scalar: reduce an iso matrix to a scalar
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#include "GB_reduce.h"

void GB_iso_reduce_to_scalar        // s = reduce (A) where A is iso
(
    GB_void *restrict s,    // output scalar of type reduce->op->ztype
    GrB_Monoid reduce,      // monoid to use for the reduction
    GrB_Matrix A,           // matrix to reduce
    GB_Context Context
)
{ 

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (A->iso) ;
    ASSERT_MATRIX_OK (A, "A for iso_reduce_to_scalar", GB0) ;
    ASSERT_MONOID_OK (reduce, "monoid for iso_reduce_to_scalar", GB0) ;
    ASSERT (s != NULL) ;
    ASSERT (GB_ZOMBIES_OK (A)) ;
    ASSERT (GB_JUMBLED_OK (A)) ;
    ASSERT (!GB_PENDING (A)) ;

    //--------------------------------------------------------------------------
    // get input matrix and the monoid
    //--------------------------------------------------------------------------

    // A consists of n entries, all equal to Ax [0]
    uint64_t n = (uint64_t) (GB_nnz (A) - A->nzombies) ;
    ASSERT (n > 0) ;

    // get the monoid
    GxB_binary_function freduce = reduce->op->binop_function ;
    GrB_Type ztype = reduce->op->ztype ;
    size_t zsize = ztype->size ;
    GB_Type_code zcode = ztype->code ;

    // a = (ztype) Ax [0]
    GB_void a [GB_VLA(zsize)] ;
    GB_cast_scalar (a, zcode, A->x, A->type->code, zsize) ;

    //--------------------------------------------------------------------------
    // reduce n entries, all equal to a, to the scalar s, in O(log(n)) time
    //--------------------------------------------------------------------------

    GB_iso_reduce_worker (s, freduce, a, n, zsize) ;
}

