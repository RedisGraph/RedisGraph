//------------------------------------------------------------------------------
// GrB_Monoid_new:  create a new monoid
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Create a new monoid with binary operator, z=op(x.y).  The three types of x,
// y, and z must all be the same, and the identity value must also have the
// same type.  No typecasting is done for the identity value.

#include "GB.h"

#undef MONOID
#define MONOID(type,T)                                                  \
GrB_Info GrB_Monoid_ ## T ##_new    /* create a new boolean monoid   */ \
(                                                                       \
    GrB_Monoid *monoid,             /* handle of monoid to create    */ \
    const GrB_BinaryOp op,          /* binary operator of the monoid */ \
    const type identity             /* identity value of the monoid  */ \
)                                                                       \
{                                                                       \
    WHERE ("GrB_Monoid_" GB_STR(T) "_new (&monoid, op, identity)") ;    \
    type id = identity ;                                                \
    return (GB_Monoid_new (monoid, op, &id, GB_ ## T ## _code)) ;       \
}

MONOID (bool     , BOOL   ) ;
MONOID (int8_t   , INT8   ) ;
MONOID (uint8_t  , UINT8  ) ;
MONOID (int16_t  , INT16  ) ;
MONOID (uint16_t , UINT16 ) ;
MONOID (int32_t  , INT32  ) ;
MONOID (uint32_t , UINT32 ) ;
MONOID (int64_t  , INT64  ) ;
MONOID (uint64_t , UINT64 ) ;
MONOID (float    , FP32   ) ;
MONOID (double   , FP64   ) ;

GrB_Info GrB_Monoid_UDT_new         // create a monoid with a user-defined type
(
    GrB_Monoid *monoid,             // handle of monoid to create
    const GrB_BinaryOp op,          // binary operator of the monoid
    const void *identity            // identity value of the monoid
)
{
    WHERE ("GrB_Monoid_UDT_new (&monoid, op, identity)") ;
    return (GB_Monoid_new (monoid, op, identity, GB_UDT_code)) ;
}

#undef MONOID

