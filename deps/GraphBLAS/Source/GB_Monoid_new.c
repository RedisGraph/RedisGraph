//------------------------------------------------------------------------------
// GB_Monoid_new: create a GrB_Monoid
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Create a user-defined monoid with an operator, identity value, and
// (optionally) a terminal value.  If using a built-in operator, a duplicate
// boolean operators is first replaced with its unique equivalent.  If the
// operator is built-in and corresponds to a known monoid, then the identity
// value and terminal value provided on input are ignored, and the known values
// are used instead.  This is to allow the use of the hard-coded functions for
// built-in monoids.

#include "GB.h"

GrB_Info GB_Monoid_new          // create a monoid
(
    GrB_Monoid *monoid,         // handle of monoid to create
    GrB_BinaryOp op,            // binary operator of the monoid
    const void *identity,       // identity value
    const void *terminal,       // terminal value, if any (may be NULL)
    GB_Type_code idcode,        // identity and terminal type code
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GB_RETURN_IF_NULL (monoid) ;
    (*monoid) = NULL ;
    GB_RETURN_IF_NULL_OR_FAULTY (op) ;
    ASSERT (identity != NULL) ;

    ASSERT_BINARYOP_OK (op, "op for monoid", GB0) ;
    ASSERT (idcode <= GB_UDT_code) ;

    //--------------------------------------------------------------------------
    // rename built-in binary operators
    //--------------------------------------------------------------------------

    // If the user requests the creation of a monoid based on a duplicate
    // built-in binary operator, the unique boolean operator is used instead.
    // See also GB_boolean_rename, which does this for opcodes, not operators.
    // This is done before the operator is checked, so that any error messages
    // reflect the renaming.

    if (op == GrB_DIV_BOOL)
    { 
        // FIRST and DIV are the same for boolean:
        op = GrB_FIRST_BOOL ;
    }
    else if (op == GxB_RDIV_BOOL)
    { 
        // SECOND and RDIV are the same for boolean:
        op = GrB_SECOND_BOOL ;
    }
    else if (op == GrB_MIN_BOOL || op == GrB_TIMES_BOOL)
    { 
        // MIN, TIMES, and LAND are the same for boolean:
        op = GrB_LAND ;
    }
    else if (op == GrB_MAX_BOOL || op == GrB_PLUS_BOOL)
    { 
        // MAX, PLUS, and OR are the same for boolean:
        op = GrB_LOR ;
    }
    else if (op == GxB_ISNE_BOOL || op == GrB_NE_BOOL || op == GrB_MINUS_BOOL
        || op == GxB_RMINUS_BOOL)
    { 
        // ISNE, NE, MINUS, RMINUS, and XOR are the same for boolean:
        op = GrB_LXOR ;
    }
    else if (op == GxB_ISEQ_BOOL)
    { 
        // ISEQ, EQ are the same for boolean:
        op = GrB_EQ_BOOL ;
    }
    else if (op == GxB_ISGT_BOOL)
    { 
        // ISGT, GT are the same for boolean:
        op = GrB_GT_BOOL ;
    }
    else if (op == GxB_ISLT_BOOL)
    { 
        // ISLT, LT are the same for boolean:
        op = GrB_LT_BOOL ;
    }
    else if (op == GxB_ISGE_BOOL)
    { 
        // ISGE, GE are the same for boolean:
        op = GrB_GE_BOOL ;
    }
    else if (op == GxB_ISLE_BOOL)
    { 
        // ISLE, LE are the same for boolean:
        op = GrB_LE_BOOL ;
    }

    //--------------------------------------------------------------------------
    // continue checking inputs
    //--------------------------------------------------------------------------

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
    // GB_UDT_code, can be checked to see if it matches.  In
    // that case, all that is known is that identity is a void * pointer that
    // points to something, hopefully a scalar of the proper user-defined type.

    GB_Type_code zcode = op->ztype->code ;
    if (idcode != zcode)
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
        // out of memory
        return (GB_OUT_OF_MEMORY) ;
    }

    // initialize the monoid
    GrB_Monoid mon = *monoid ;
    mon->magic = GB_MAGIC ;
    mon->op = op ;
    mon->object_kind = GB_USER_RUNTIME ;
    size_t zsize = op->ztype->size ;
    mon->op_ztype_size = zsize ;
    mon->identity = NULL ;                  // defined below
    mon->terminal = NULL ;                  // defined below (if present)

    //--------------------------------------------------------------------------
    // allocation of idenity and terminal values
    //--------------------------------------------------------------------------

    // allocate both the identity and terminal value
    #define GB_ALLOC_IDENTITY_AND_TERMINAL                                  \
    {                                                                       \
        GB_CALLOC_MEMORY (mon->identity, 1, zsize) ;                        \
        GB_CALLOC_MEMORY (mon->terminal, 1, zsize) ;                        \
        if (mon->identity == NULL || mon->terminal == NULL)                 \
        {                                                                   \
            /* out of memory */                                             \
            GB_FREE_MEMORY (mon->identity, 1, zsize) ;                      \
            GB_FREE_MEMORY (mon->terminal, 1, zsize) ;                      \
            GB_FREE_MEMORY (*monoid, 1, sizeof (struct GB_Monoid_opaque)) ; \
            return (GB_OUT_OF_MEMORY) ;                                     \
        }                                                                   \
    }

    // allocate just the identity, not the terminal
    #define GB_ALLOC_JUST_IDENTITY                                          \
    {                                                                       \
        GB_CALLOC_MEMORY (mon->identity, 1, zsize) ;                        \
        if (mon->identity == NULL)                                          \
        {                                                                   \
            /* out of memory */                                             \
            GB_FREE_MEMORY (*monoid, 1, sizeof (struct GB_Monoid_opaque)) ; \
            return (GB_OUT_OF_MEMORY) ;                                     \
        }                                                                   \
    }

    //--------------------------------------------------------------------------
    // create a user-defined monoid based on a known built-in monoid
    //--------------------------------------------------------------------------

    bool done = false ;

    // If the user requests the creation of a monoid based on a built-in
    // operator that corresponds to known monoid, the identity and terminal
    // values provided by the user to GrB_Monoid_new are ignored, since these
    // are all handled by the same hard-coded functions as the built-in monoids
    // based on the same operator.

    // create a monoid with both an identity and a terminal value,
    // based on a built-in operator that is a known monoid
    #define GB_IT(ztype,identity_value,terminal_value)                      \
    {                                                                       \
        GB_ALLOC_IDENTITY_AND_TERMINAL ;                                    \
        ztype *identity = mon->identity ;                                   \
        ztype *terminal = mon->terminal ;                                   \
        (*identity) = identity_value ;                                      \
        (*terminal) = terminal_value ;                                      \
        done = true ;                                                       \
    }                                                                       \
    break ;

    // create a monoid with just an identity but no terminal value,
    // based on a built-in operator that is a known monoid
    #define GB_IN(ztype,identity_value)                                     \
    {                                                                       \
        GB_ALLOC_JUST_IDENTITY ;                                            \
        ztype *identity = mon->identity ;                                   \
        (*identity) = identity_value ;                                      \
        done = true ;                                                       \
    }                                                                       \
    break ;

    switch (op->opcode)
    {
        case GB_MIN_opcode :

            // MIN monoid:  identity is +inf, terminal is -inf
            switch (zcode)
            {
                case GB_INT8_code   : GB_IT (int8_t  , INT8_MAX  , INT8_MIN  )
                case GB_INT16_code  : GB_IT (int16_t , INT16_MAX , INT16_MIN )
                case GB_INT32_code  : GB_IT (int32_t , INT32_MAX , INT32_MIN )
                case GB_INT64_code  : GB_IT (int64_t , INT64_MAX , INT64_MIN )
                case GB_UINT8_code  : GB_IT (uint8_t , UINT8_MAX , 0         )
                case GB_UINT16_code : GB_IT (uint16_t, UINT16_MAX, 0         )
                case GB_UINT32_code : GB_IT (uint32_t, UINT32_MAX, 0         )
                case GB_UINT64_code : GB_IT (uint64_t, UINT64_MAX, 0         )
                case GB_FP32_code   : GB_IT (float   , INFINITY  , -INFINITY )
                case GB_FP64_code   : GB_IT (double  , ((double) INFINITY)  ,
                                                       ((double) -INFINITY) )
                default: ;
            }
            break ;

        case GB_MAX_opcode :

            // MAX monoid:  identity is -inf, terminal is +inf
            switch (zcode)
            {
                case GB_INT8_code   : GB_IT (int8_t  , INT8_MIN  , INT8_MAX  )
                case GB_INT16_code  : GB_IT (int16_t , INT16_MIN , INT16_MAX )
                case GB_INT32_code  : GB_IT (int32_t , INT32_MIN , INT32_MAX )
                case GB_INT64_code  : GB_IT (int64_t , INT64_MIN , INT64_MAX )
                case GB_UINT8_code  : GB_IT (uint8_t , 0         , UINT8_MAX )
                case GB_UINT16_code : GB_IT (uint16_t, 0         , UINT16_MAX)
                case GB_UINT32_code : GB_IT (uint32_t, 0         , UINT32_MAX)
                case GB_UINT64_code : GB_IT (uint64_t, 0         , UINT64_MAX)
                case GB_FP32_code   : GB_IT (float   , -INFINITY , INFINITY  )
                case GB_FP64_code   : GB_IT (double  , ((double) -INFINITY)  ,
                                                       ((double) INFINITY) )
                default: ;
            }
            break ;

        case GB_PLUS_opcode :

            // PLUS monoid:  identity is zero, no terminal value
            switch (zcode)
            {
                case GB_INT8_code   : GB_IN (int8_t  , 0 )
                case GB_INT16_code  : GB_IN (int16_t , 0 )
                case GB_INT32_code  : GB_IN (int32_t , 0 )
                case GB_INT64_code  : GB_IN (int64_t , 0 )
                case GB_UINT8_code  : GB_IN (uint8_t , 0 )
                case GB_UINT16_code : GB_IN (uint16_t, 0 )
                case GB_UINT32_code : GB_IN (uint32_t, 0 )
                case GB_UINT64_code : GB_IN (uint64_t, 0 )
                case GB_FP32_code   : GB_IN (float   , 0 )
                case GB_FP64_code   : GB_IN (double  , 0 )
                default: ;
            }
            break ;

        case GB_TIMES_opcode :

            // TIMES monoid:  identity is 1, no terminal value
            switch (zcode)
            {
                case GB_INT8_code   : GB_IN (int8_t  , 1 )
                case GB_INT16_code  : GB_IN (int16_t , 1 )
                case GB_INT32_code  : GB_IN (int32_t , 1 )
                case GB_INT64_code  : GB_IN (int64_t , 1 )
                case GB_UINT8_code  : GB_IN (uint8_t , 1 )
                case GB_UINT16_code : GB_IN (uint16_t, 1 )
                case GB_UINT32_code : GB_IN (uint32_t, 1 )
                case GB_UINT64_code : GB_IN (uint64_t, 1 )
                case GB_FP32_code   : GB_IN (float   , 1 )
                case GB_FP64_code   : GB_IN (double  , 1 )
                default: ;
            }
            break ;

        case GB_LOR_opcode :

            // boolean OR monoid:  identity is false, terminal is true
            if (zcode == GB_BOOL_code) GB_IT (bool, false, true)

        case GB_LAND_opcode :

            // boolean AND monoid:  identity is true, terminal is false
            if (zcode == GB_BOOL_code) GB_IT (bool, true, false)

        case GB_LXOR_opcode :

            // boolean XOR monoid:  identity is false, no terminal value
            if (zcode == GB_BOOL_code) GB_IN (bool, false)

        case GB_EQ_opcode :

            // boolean EQ monoid:  identity is true, no terminal value
            if (zcode == GB_BOOL_code) GB_IN (bool, true)

        default : ;

            // monoid identity and terminal value defined below
    }

    //--------------------------------------------------------------------------
    // user-defined operators or unknown monoids
    //--------------------------------------------------------------------------

    // The monoid is based on a user-defined operator, or on a built-in
    // operator that is not a known monoid.  Use the identity and terminal
    // values provided on input.

    if (!done)
    {
        if (terminal == NULL)
        { 
            // create a monoid with just an identity but no terminal value
            GB_ALLOC_JUST_IDENTITY ;
            memcpy (mon->identity, identity, zsize) ;
        }
        else
        { 
            // create a monoid with both an identity and a terminal value
            GB_ALLOC_IDENTITY_AND_TERMINAL ;
            memcpy (mon->identity, identity, zsize) ;
            memcpy (mon->terminal, terminal, zsize) ;
        }
    }

    ASSERT_MONOID_OK (mon, "new monoid", GB0) ;
    return (GrB_SUCCESS) ;
}

