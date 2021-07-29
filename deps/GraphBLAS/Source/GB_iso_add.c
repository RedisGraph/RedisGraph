//------------------------------------------------------------------------------
// GB_iso_add: apply a binary op and check for iso result for C=A+B
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// Compute c = op(a,b) for two matrices A and B, and return true if C=A+B
// results in an iso matrix C.  If true, the output scalar c is the iso value
// for the matrix C.

#include "GB_add.h"
#include "GB_emult.h"

bool GB_iso_add             // c = op(a,b), return true if C is iso
(
    // output
    GB_void *restrict c,    // output scalar of iso array
    // input
    GrB_Type ctype,         // type of c
    GrB_Matrix A,           // input matrix
    GrB_Matrix B,           // input matrix
    GrB_BinaryOp op         // binary operator, if present
)
{

    //--------------------------------------------------------------------------
    // get inputs
    //--------------------------------------------------------------------------

    ASSERT_MATRIX_OK (A, "A for GB_iso_add", GB0) ;
    ASSERT_MATRIX_OK (B, "B for GB_iso_add", GB0) ;
    ASSERT_TYPE_OK (ctype, "ctype for GB_iso_add", GB0) ;
    ASSERT_BINARYOP_OK_OR_NULL (op, "op for GB_iso_add", GB0) ;
    ASSERT (c != NULL) ;

    //--------------------------------------------------------------------------
    // special case if both A and B are full (or as-if-full) 
    //--------------------------------------------------------------------------

    if (GB_as_if_full (A) && GB_as_if_full (B))
    {
        // A and B are both full (or as-if-full), and both eWiseMult and
        // eWiseAdd would compute the same thing.  GB_emult detects this
        // condition and calls GB_add, so that GB_emult doesn't have to handle
        // this case.  GB_add tests the iso condition as if it were computing
        // C=A.*B.
        return (GB_iso_emult (c, ctype, A, B, op)) ;
    }

    //--------------------------------------------------------------------------
    // quick return if A or B are not iso or if the op is positional
    //--------------------------------------------------------------------------

    if (!A->iso || !B->iso || GB_OP_IS_POSITIONAL (op))
    { 
        // C is not iso
        return (false) ;
    }

    //--------------------------------------------------------------------------
    // compare the iso values of A and B, typecasted to C
    //--------------------------------------------------------------------------

    const size_t csize = ctype->size ;
    const size_t asize = A->type->size ;
    const size_t bsize = B->type->size ;

    const GB_Type_code ccode = ctype->code ;
    const GB_Type_code acode = A->type->code ;
    const GB_Type_code bcode = B->type->code ;

    // c = zero
    memset (c, 0, csize) ;

    // a = (ctype) Ax [0]
    GB_void a [GB_VLA(csize)] ;
    GB_cast_scalar (a, ccode, A->x, acode, asize) ;

    // b = (ctype) Bx [0]
    GB_void b [GB_VLA(csize)] ;
    GB_cast_scalar (b, ccode, B->x, bcode, bsize) ;

    if (memcmp (a, b, csize) != 0)
    {
        // the iso values of A and B differ, when typecasted to C
        return (false) ;
    }

    //--------------------------------------------------------------------------
    // compute the C iso value and compare with A and B
    //--------------------------------------------------------------------------

    if (op == NULL)
    { 

        // For GB_wait, the pattern of A and B are known to be disjoint, so no
        // operator is used, and op is NULL.  No typecasting is done.
        ASSERT (ctype == A->type) ;
        memcpy (c, a, csize) ;
        return (true) ;

    }
    else
    {

        // get the binary operator
        const GxB_binary_function fadd = op->function ;

        const GrB_Type xtype = op->xtype ;
        const GrB_Type ytype = op->ytype ;
        const GrB_Type ztype = op->ztype ;

        const GB_Type_code xcode = xtype->code ;
        const GB_Type_code ycode = ytype->code ;
        const GB_Type_code zcode = ztype->code ;

        const size_t xsize = xtype->size ;
        const size_t ysize = ytype->size ;
        const size_t zsize = ztype->size ;

        // x = (xtype) Ax [0]
        GB_void x [GB_VLA(xsize)] ;
        GB_cast_scalar (x, xcode, A->x, acode, asize) ;

        // y = (ytype) Bx [0]
        GB_void y [GB_VLA(ysize)] ;
        GB_cast_scalar (y, ycode, B->x, bcode, bsize) ;

        // z = op (x,y)
        GB_void z [GB_VLA(zsize)] ;
        fadd (z, x, y) ;

        // c = (ctype) z
        GB_cast_scalar (c, ccode, z, zcode, zsize) ;

        if (memcmp (c, a, csize) != 0)
        { 
            // the iso values of C and A differ
            return (false) ;
        }

        return (true) ;
    }
}

