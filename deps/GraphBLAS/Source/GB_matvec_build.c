//------------------------------------------------------------------------------
// GB_matvec_build: check inputs and build a matrix or vector
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// CALLED BY: GrB_Matrix_build_* and GrB_Vector_build_*
// CALLS:     GB_build

// This function implements GrB_Matrix_build_* and GrB_Vector_build_*.

#include "GB_build.h"

GrB_Info GB_matvec_build        // check inputs then build matrix or vector
(
    GrB_Matrix C,               // matrix or vector to build
    const GrB_Index *I,         // row indices of tuples
    const GrB_Index *J,         // col indices of tuples (NULL for vector)
    const void *X,              // array of values of tuples
    const GrB_Index nvals,      // number of tuples
    const GrB_BinaryOp dup,     // binary function to assemble duplicates
    const GB_Type_code scode,   // GB_Type_code of X array
    const bool is_matrix,       // true if C is a matrix, false if GrB_Vector
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT_MATRIX_OK (C, "C for GB_matvec_build", GB0) ;
    GB_RETURN_IF_NULL (I) ;
    if (I == GrB_ALL)
    { 
        return (GB_ERROR (GrB_INVALID_VALUE, (GB_LOG,
            "List of row indices cannot be GrB_ALL"))) ;
    }

    if (nvals == GxB_RANGE || nvals == GxB_STRIDE || nvals == GxB_BACKWARDS)
    { 
        return (GB_ERROR (GrB_INVALID_VALUE, (GB_LOG,
            "nvals cannot be GxB_RANGE, GxB_STRIDE, or GxB_BACKWARDS"))) ;
    }

    if (is_matrix)
    {
        GB_RETURN_IF_NULL (J) ;
        if (J == GrB_ALL)
        { 
            return (GB_ERROR (GrB_INVALID_VALUE, (GB_LOG,
                "List of column indices cannot be 'GrB_ALL'"))) ;
        }
    }
    else
    { 
        // only GrB_Vector_build calls this function with J == NULL
        ASSERT (J == NULL) ;
    }

    GB_RETURN_IF_NULL (X) ;
    GB_RETURN_IF_NULL_OR_FAULTY (dup) ;

    ASSERT_BINARYOP_OK (dup, "dup operator for assembling duplicates", GB0) ;
    ASSERT (scode <= GB_UDT_code) ;

    if (nvals > GB_INDEX_MAX)
    { 
        // problem too large
        return (GB_ERROR (GrB_INVALID_VALUE, (GB_LOG,
            "problem too large: nvals "GBu" exceeds "GBu,
            nvals, GB_INDEX_MAX))) ;
    }

    // check types of dup
    if (dup->xtype != dup->ztype || dup->ytype != dup->ztype)
    { 
        // all 3 types of z = dup (x,y) must be the same.  dup must also be
        // associative but there is no way to check this in general.
        return (GB_ERROR (GrB_DOMAIN_MISMATCH, (GB_LOG, "All domains of dup "
        "operator for assembling duplicates must be identical.\n"
        "operator is: [%s] = %s ([%s],[%s])",
        dup->ztype->name, dup->name, dup->xtype->name, dup->ytype->name))) ;
    }

    if (!GB_Type_compatible (C->type, dup->ztype))
    { 
        // the type of C and dup must be compatible
        return (GB_ERROR (GrB_DOMAIN_MISMATCH, (GB_LOG,
        "operator dup [%s] has type [%s]\n"
        "cannot be typecast to entries in output of type [%s]",
        dup->name, dup->ztype->name, C->type->name))) ;
    }

    // C and X must be compatible
    if (!GB_code_compatible (scode, dup->ztype->code))
    { 
        // All types must be compatible with each other: C, dup, and X.
        // User-defined types are only compatible with themselves; they are not
        // compatible with any built-in type nor any other user-defined type.
        // Thus, if C, dup, or X have any user-defined type, this
        // condition requires all three types to be identical: the same
        // user-defined type.  No casting will be done in this case.
        return (GB_ERROR (GrB_DOMAIN_MISMATCH, (GB_LOG,
        "numerical values of tuples of type [%s]\n"
        "cannot be typecast as input to the dup operator\n"
        "z=%s(x,y), whose input types are [%s]",
        GB_code_string (scode), dup->name, dup->ztype->name))) ;
    }

    if (!GB_EMPTY (C))
    { 
        // The matrix has existing entries.  This is required by the GraphBLAS
        // API specification to generate an error, so the test is made here.
        // However, any existing content is safely freed immediately below, so
        // this test is not required, except to conform to the spec.  Zombies
        // are excluded from this test.
        return (GB_ERROR (GrB_OUTPUT_NOT_EMPTY, (GB_LOG,
            "output already has existing entries"))) ;
    }

    //--------------------------------------------------------------------------
    // build the matrix
    //--------------------------------------------------------------------------

    // GB_build treats I, J, and X as read-only; they must not be modified

    return (GB_build (C, I, J, X, nvals, dup, scode, is_matrix, true, Context));
}

