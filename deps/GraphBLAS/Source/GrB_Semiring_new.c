//------------------------------------------------------------------------------
// GrB_Semiring_new: create a new semiring
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// A GraphBLAS Semiring consists of two components: "add" and "multiply".
// These components imply three domains: ztype, xtype, and ytype.

// The "add" is an associative and commutative monoid, which is a binary
// operator that works on a single type, ztype = add(ztype,ztype).  The add
// monoid also includes an identity value, called "zero", so that
// add(x,zero)=add(zero,x)=x.  For most algebras, this "zero" is a plain zero
// in the usual sense, but this is not the case for all algebras.  For example,
// for the max-plus algebra, the "add" operator is the function max(a,b), and
// the "zero" for this operator is -infinity since max(a,-inf)=max(-inf,a)=a.

// The "multiply" is a binary operator z = multiply(x,y).  It has no
// restrictions, except that the type of z must exactly match the ztype
// of the add monoid.  That is, the types for the multiply operator are
// ztype = multiply (xtype, ytype).  When the semiring is applied to two
// matrices A and B, where (A,B) appear in that order in the method, the
// multiply operator is always applied as z = multiply (A(i,j),B(i,j)).  The
// two input operands always appear in that order.  That is, the multiply
// operator is not assumed to be commutative.

#include "GB.h"

GrB_Info GrB_Semiring_new           // create a semiring
(
    GrB_Semiring *semiring,         // handle of semiring to create
    const GrB_Monoid add,           // additive monoid of the semiring
    const GrB_BinaryOp multiply     // multiply operator of the semiring
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    WHERE ("GrB_Semiring_new (&semiring, add, multiply)") ;
    RETURN_IF_NULL (semiring) ;
    (*semiring) = NULL ;
    RETURN_IF_NULL_OR_UNINITIALIZED (add) ;
    RETURN_IF_NULL_OR_UNINITIALIZED (multiply) ;

    ASSERT_OK (GB_check (add, "semiring->add", 0)) ;
    ASSERT_OK (GB_check (multiply, "semiring->multiply", 0)) ;

    // z = multiply(x,y); type of z must match monoid z = add(z,z)
    if (multiply->ztype != add->op->ztype)
    {
        (*semiring) = NULL ;
        return (ERROR (GrB_DOMAIN_MISMATCH, (LOG,
            "Semiring multiply output domain must match monoid domain"))) ;
    }

    //--------------------------------------------------------------------------
    // create the semiring
    //--------------------------------------------------------------------------

    // allocate the semiring
    GB_CALLOC_MEMORY (*semiring, 1, sizeof (GB_Semiring_opaque)) ;
    if (*semiring == NULL)
    {
        return (ERROR (GrB_OUT_OF_MEMORY, (LOG, "out of memory"))) ;
    }

    // initialize the semiring
    GrB_Semiring s = *semiring ;
    s->magic = MAGIC ;
    s->add = add ;
    s->multiply = multiply ;
    s->user_defined = true ;

    ASSERT_OK (GB_check (s, "new semiring", 0)) ;
    return (REPORT_SUCCESS) ;
}

