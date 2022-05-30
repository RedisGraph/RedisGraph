//------------------------------------------------------------------------------
// GB_Vector_extractElement: x = V(i)
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// Extract the value of single scalar, x = V(i), typecasting from the
// type of V to the type of x, as needed.

// Returns GrB_SUCCESS if V(i) is present, and sets x to its value.
// Returns GrB_NO_VALUE if V(i) is not present, and x is unmodified.

// This template constructs GrB_Vector_extractElement_[TYPE], for each of the
// 13 built-in types, and the _UDT method for all user-defined types.

// FUTURE: tolerate zombies

GrB_Info GB_EXTRACT_ELEMENT     // extract a single entry, x = V(i)
(
    GB_XTYPE *x,                // scalar to extract, not modified if not found
    const GrB_Vector V,         // vector to extract a scalar from
    GrB_Index i                 // index
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GB_RETURN_IF_NULL_OR_FAULTY (V) ;
    GB_RETURN_IF_NULL (x) ;

    // delete any lingering zombies, assemble any pending tuples, and unjumble
    if (GB_ANY_PENDING_WORK (V))
    { 
        GrB_Info info ;
        GB_WHERE1 (GB_WHERE_STRING) ;
        GB_BURBLE_START ("GrB_Vector_extractElement") ;
        GB_OK (GB_wait ((GrB_Matrix) V, "v", Context)) ;
        GB_BURBLE_END ;
    }

    ASSERT (!GB_ANY_PENDING_WORK (V)) ;

    // check index
    if (i >= V->vlen)
    { 
        return (GrB_INVALID_INDEX) ;
    }

    // GB_XCODE and V must be compatible
    GB_Type_code vcode = V->type->code ;
    if (!GB_code_compatible (GB_XCODE, vcode))
    { 
        return (GrB_DOMAIN_MISMATCH) ;
    }

    if (GB_nnz ((GrB_Matrix) V) == 0)
    { 
        // quick return
        return (GrB_NO_VALUE) ;
    }

    //--------------------------------------------------------------------------
    // find the entry V(i)
    //--------------------------------------------------------------------------

    int64_t pleft ;
    bool found ;
    const int64_t *restrict Vp = V->p ;

    if (Vp != NULL)
    { 
        // V is sparse
        const int64_t *restrict Vi = V->i ;

        pleft = 0 ;
        int64_t pright = Vp [1] - 1 ;

        // binary search for index i
        // Time taken for this step is at most O(log(nnz(V))).
        GB_BINARY_SEARCH (i, Vi, pleft, pright, found) ;
    }
    else
    {
        // V is bitmap or full
        pleft = i ;
        const int8_t *restrict Vb = V->b ;
        if (Vb != NULL)
        { 
            // V is bitmap
            found = (Vb [pleft] == 1) ;
        }
        else
        { 
            // V is full
            found = true ;
        }
    }

    //--------------------------------------------------------------------------
    // extract the element
    //--------------------------------------------------------------------------

    if (found)
    {
        #if !defined ( GB_UDT_EXTRACT )
        if (GB_XCODE == vcode)
        { 
            // copy the value from V [...] into the scalar x, no typecasting,
            // for built-in types only.
            GB_XTYPE *restrict Vx = ((GB_XTYPE *) (V->x)) ;
            (*x) = Vx [V->iso ? 0:pleft] ;
        }
        else
        #endif
        { 
            // typecast the value from V [...] into the scalar x
            size_t vsize = V->type->size ;
            void *vx = ((GB_void *) V->x) + (V->iso ? 0 : (pleft*vsize)) ;
            GB_cast_scalar (x, GB_XCODE, vx, vcode, vsize) ;
        }
        // TODO: do not flush if extracting to GrB_Scalar
        #pragma omp flush
        return (GrB_SUCCESS) ;
    }
    else
    { 
        // Entry not found.
        return (GrB_NO_VALUE) ;
    }
}

#undef GB_UDT_EXTRACT
#undef GB_EXTRACT_ELEMENT
#undef GB_XTYPE
#undef GB_XCODE

