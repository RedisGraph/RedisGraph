//------------------------------------------------------------------------------
// GrB_Monoid_new:  create a new monoid
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Create a new monoid with binary operator, z=op(x.y).  The three types of x,
// y, and z must all be the same, and the identity value must also have the
// same type.  No typecasting is done for the identity value.

#include "GB.h"

#define GB_MONOID_NEW(type,T)                                               \
GrB_Info GrB_Monoid_new_ ## T       /* create a new monoid */               \
(                                                                           \
    GrB_Monoid *monoid,             /* handle of monoid to create    */     \
    GrB_BinaryOp op,                /* binary operator of the monoid */     \
    type identity                   /* identity value of the monoid  */     \
)                                                                           \
{                                                                           \
    GB_WHERE ("GrB_Monoid_new_" GB_STR(T) " (&monoid, op, identity)") ;     \
    type id = identity ;                                                    \
    return (GB_Monoid_new (monoid, op, &id, NULL, GB_ ## T ## _code, Context));\
}

GB_MONOID_NEW (bool     , BOOL   )
GB_MONOID_NEW (int8_t   , INT8   )
GB_MONOID_NEW (uint8_t  , UINT8  )
GB_MONOID_NEW (int16_t  , INT16  )
GB_MONOID_NEW (uint16_t , UINT16 )
GB_MONOID_NEW (int32_t  , INT32  )
GB_MONOID_NEW (uint32_t , UINT32 )
GB_MONOID_NEW (int64_t  , INT64  )
GB_MONOID_NEW (uint64_t , UINT64 )
GB_MONOID_NEW (float    , FP32   )
GB_MONOID_NEW (double   , FP64   )

GrB_Info GrB_Monoid_new_UDT         // create a monoid with a user-defined type
(
    GrB_Monoid *monoid,             // handle of monoid to create
    GrB_BinaryOp op,                // binary operator of the monoid
    void *identity                  // identity value of the monoid
)
{ 
    GB_WHERE ("GrB_Monoid_new_UDT (&monoid, op, identity)") ;
    GB_RETURN_IF_NULL (identity) ;
    return (GB_Monoid_new (monoid, op, identity, NULL, GB_UDT_code, Context)) ;
}

