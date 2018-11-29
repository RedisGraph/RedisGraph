//------------------------------------------------------------------------------
// GB_Monoid_new: create a Monoid with a specific type of identity
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB.h"

GrB_Info GB_Monoid_new          // create a monoid
(
    GrB_Monoid *monoid,         // handle of monoid to create
    const GrB_BinaryOp op,      // binary operator of the monoid
    const void *identity,       // identity value
    const GB_Type_code idcode,  // identity code
    GB_Context Context
)
{ 

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GB_RETURN_IF_NULL (monoid) ;
    (*monoid) = NULL ;
    GB_RETURN_IF_NULL_OR_FAULTY (op) ;
    GB_RETURN_IF_NULL (identity) ;

    ASSERT_OK (GB_check (op, "op for monoid", GB0)) ;
    ASSERT (idcode <= GB_UDT_code) ;

    // check operator types; all must be identical
    if (op->xtype != op->ztype || op->ytype != op->ztype)
    { 
        return (GB_ERROR (GrB_DOMAIN_MISMATCH, (GB_LOG,
            "All domains of monoid operator must be identical;\n"
            "operator is: [%s] = %s ([%s],[%s])",
            op->ztype->name, op->name, op->xtype->name, op->ytype->name))) ;
    }

    // The idcode must match the monoid->op->ztype->code for built-in types,
    // and this can be rigourously checked.  For all user-defined types,
    // identity is a mere void * pointer, and its actual type cannot be
    // compared with the input op->ztype parameter.  Only the type code,
    // GB_UDT_code or GB_UCT_code, can be checked to see if it matches.  In
    // that case, all that is known is that identity is a void * pointer that
    // points to something, hopefully a scalar of the proper user-defined type.
    if (idcode != op->ztype->code)
    { 
        return (GB_ERROR (GrB_DOMAIN_MISMATCH, (GB_LOG,
            "Identity type [%s]\n"
            "must be identical to monoid operator z=%s(x,y) of type [%s]",
            GB_code_string (idcode), op->name, op->ztype->name))) ;
    }

    //--------------------------------------------------------------------------
    // create the monoid
    //--------------------------------------------------------------------------

    // allocate the monoid
    GB_CALLOC_MEMORY (*monoid, 1, sizeof (struct GB_Monoid_opaque)) ;
    if (*monoid == NULL)
    { 
        return (GB_NO_MEMORY) ;
    }

    // initialize the monoid
    GrB_Monoid mon = *monoid ;
    mon->magic = GB_MAGIC ;
    mon->op = op ;
    mon->object_kind = GB_USER_RUNTIME ;
    mon->op_ztype_size = op->ztype->size ;
    GB_CALLOC_MEMORY (mon->identity, 1, op->ztype->size) ;
    if (mon->identity == NULL)
    { 
        GB_FREE_MEMORY (*monoid, 1, sizeof (struct GB_Monoid_opaque)) ;
        return (GB_NO_MEMORY) ;
    }

    // copy the identity into the monoid.  No typecasting needed.
    memcpy (mon->identity, identity, op->ztype->size) ;

    ASSERT_OK (GB_check (mon, "new monoid", GB0)) ;
    return (GrB_SUCCESS) ;
}

