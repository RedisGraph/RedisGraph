//------------------------------------------------------------------------------
// GB_compatible: check input and operators for type compatibility
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Check if the types for C<Mask> = accum (C,T) are all compatible,
// and (if present) make sure the size of C and Mask match.

#include "GB.h"

GrB_Info GB_compatible          // SUCCESS if all is OK, *_MISMATCH otherwise
(
    const GrB_Type ctype,       // the type of C (matrix or scalar)
    const GrB_Matrix C,         // the output matrix C; NULL if C is a scalar
    const GrB_Matrix Mask,      // optional Mask, NULL if no mask
    const GrB_BinaryOp accum,   // C<Mask> = accum(C,T) is computed
    const GrB_Type ttype,       // type of T
    GB_Context Context
)
{

    ASSERT (GB_ALIAS_OK (C, Mask)) ;

    GrB_Info info ;

    if (accum != NULL)
    {
        // Results T are accumlated via C<Mask>=accum(C,T)

        // For entries in C and T, c=z=accum(c,t) is computed, so C must
        // be compatible with both the ztype and xtype of accum, and T
        // must be compatible with the ytype of accum.

        // For entries in T but not C, c=t is assigned, so C and T must
        // be compatible.  This is the same as the condition below
        // when accum is NULL.

        info = GB_BinaryOp_compatible (accum, ctype, ctype, ttype, 0, Context) ;
        if (info != GrB_SUCCESS)
        { 
            return (info) ;
        }
    }

    // C<Mask> = T, so C and T must be compatible.
    // also C<Mask> = accum(C,T) for entries in T but not C
    if (!GB_Type_compatible (ctype, ttype))
    { 
        return (GB_ERROR (GrB_DOMAIN_MISMATCH, (GB_LOG,
            "result of computation of type [%s]\n"
            "cannot be typecast to final output of type [%s]",
            ttype->name, ctype->name))) ;
    }

    // check the mask
    return (GB_Mask_compatible (Mask, C, 1, 1, Context)) ;
}

