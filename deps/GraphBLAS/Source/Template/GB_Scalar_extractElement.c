//------------------------------------------------------------------------------
// GB_Scalar_extractElement_template: x = S
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// Extract the value of single scalar, x = S, typecasting from the
// type of S to the type of x, as needed.

// Returns GrB_SUCCESS if the GxB_Scalar entry is present, and sets x to its
// value.  Returns GrB_NO_VALUE if the GxB_Scalar is not present, and x is
// unmodified.

// This template constructs GxB_Scalar_extractElement_[TYPE] for each of the
// 13 built-in types, and the _UDT method for all user-defined types.

GrB_Info GB_EXTRACT_ELEMENT     // extract a single entry from S
(
    GB_XTYPE *x,                // scalar to extract, not modified if not found
    const GxB_Scalar S          // GxB_Scalar to extract a scalar from
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GB_RETURN_IF_NULL_OR_FAULTY (S) ;
    GB_RETURN_IF_NULL (x) ;

    // delete any lingering zombies, assemble any pending tuples, and unjumble
    if (GB_ANY_PENDING_WORK (S))
    { 
        // extract scalar with pending tuples or zombies.  It cannot be
        // actually jumbled, but S->jumbled might true anyway.
        GrB_Info info ;
        GB_WHERE1 (GB_WHERE_STRING) ;
        GB_BURBLE_START ("GxB_Scalar_extractElement") ;
        GB_OK (GB_Matrix_wait ((GrB_Matrix) S, Context)) ;
        GB_BURBLE_END ;
    }

    ASSERT (!GB_ANY_PENDING_WORK (S)) ;

    // GB_XCODE and S must be compatible
    GB_Type_code scode = S->type->code ;
    if (!GB_code_compatible (GB_XCODE, scode))
    { 
        return (GrB_DOMAIN_MISMATCH) ;
    }

    if ((S->nzmax == 0)                         // empty
        || (S->p != NULL && S->p [1] == 0)      // sparse/hyper with no entry
        || (S->b != NULL && S->b [0] == 0))     // bitmap with no entry
    { 
        // quick return
        return (GrB_NO_VALUE) ;
    }

    //--------------------------------------------------------------------------
    // extract the scalar
    //--------------------------------------------------------------------------

    #if !defined ( GB_UDT_EXTRACT )
    if (GB_XCODE == scode)
    { 
        // copy the value from S into x, no typecasting, for built-in
        // types only.
        GB_XTYPE *GB_RESTRICT Sx = ((GB_XTYPE *) (S->x)) ;
        (*x) = Sx [0] ;
    }
    else
    #endif
    { 
        // typecast the value from S into x
        GB_cast_array ((GB_void *) x, GB_XCODE,
            ((GB_void *) S->x), scode, NULL, S->type->size, 1, 1) ;
    }
    return (GrB_SUCCESS) ;
}

#undef GB_UDT_EXTRACT
#undef GB_EXTRACT_ELEMENT
#undef GB_XTYPE
#undef GB_XCODE

