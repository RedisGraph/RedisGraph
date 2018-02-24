//------------------------------------------------------------------------------
// GB_builtin.c: built-in types, functions, operators, and other externs
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// This file defines the predefined built-in objects: 11 types, 45 unary
// operators, 256 binary operators, 44 monoids, and 960 semirings.

#include "GB.h"

//------------------------------------------------------------------------------
// built-in types
//------------------------------------------------------------------------------

// extern predefined type objects but opaque to the user
GB_Type_opaque
GB_opaque_BOOL   = { MAGIC, sizeof (bool),     GB_BOOL_code   , "bool"     },
GB_opaque_INT8   = { MAGIC, sizeof (int8_t),   GB_INT8_code   , "int8_t"   },
GB_opaque_UINT8  = { MAGIC, sizeof (uint8_t),  GB_UINT8_code  , "uint8_t"  },
GB_opaque_INT16  = { MAGIC, sizeof (int16_t),  GB_INT16_code  , "int16_t"  },
GB_opaque_UINT16 = { MAGIC, sizeof (uint16_t), GB_UINT16_code , "uint16_t" },
GB_opaque_INT32  = { MAGIC, sizeof (int32_t),  GB_INT32_code  , "int32_t"  },
GB_opaque_UINT32 = { MAGIC, sizeof (uint32_t), GB_UINT32_code , "uint32_t" },
GB_opaque_INT64  = { MAGIC, sizeof (int64_t),  GB_INT64_code  , "int64_t"  },
GB_opaque_UINT64 = { MAGIC, sizeof (uint64_t), GB_UINT64_code , "uint64_t" },
GB_opaque_FP32   = { MAGIC, sizeof (float),    GB_FP32_code   , "float"    },
GB_opaque_FP64   = { MAGIC, sizeof (double),   GB_FP64_code   , "double"   } ;

// extern predefined types (handles to opaque types)
GrB_Type
    GrB_BOOL   = &GB_opaque_BOOL,
    GrB_INT8   = &GB_opaque_INT8,
    GrB_UINT8  = &GB_opaque_UINT8,
    GrB_INT16  = &GB_opaque_INT16,
    GrB_UINT16 = &GB_opaque_UINT16,
    GrB_INT32  = &GB_opaque_INT32,
    GrB_UINT32 = &GB_opaque_UINT32,
    GrB_INT64  = &GB_opaque_INT64,
    GrB_UINT64 = &GB_opaque_UINT64,
    GrB_FP32   = &GB_opaque_FP32,
    GrB_FP64   = &GB_opaque_FP64 ;

//------------------------------------------------------------------------------
// built-in unary and binary operators
//------------------------------------------------------------------------------

// helper macro to define unary operators; z and x have the same type
#define UNARY(PREFIX,OPERATOR,OPNAME)                                       \
GB_UnaryOp_opaque GB (OPERATOR ## _opaque) =                                \
{                                                                           \
    MAGIC,                                                                  \
    & GB (opaque),                                                          \
    & GB (opaque),                                                          \
    & GB (OPERATOR ## _f),                                                  \
    OPNAME,                                                                 \
    GB_ ## OPERATOR ## _opcode                                              \
} ;                                                                         \
GrB_UnaryOp GRB_NAME (PREFIX,OPERATOR) = & GB (OPERATOR ## _opaque) ;


// helper macro to define binary operators: all x,y,z types the same
#define BINARY(PREFIX,OPERATOR,OPNAME)                                      \
GB_BinaryOp_opaque GB (OPERATOR ## _opaque) =                               \
{                                                                           \
    MAGIC,                                                                  \
    & GB (opaque),                                                          \
    & GB (opaque),                                                          \
    & GB (opaque),                                                          \
    & GB (OPERATOR ## _f),                                                  \
    OPNAME,                                                                 \
    GB_ ## OPERATOR ## _opcode                                              \
} ;                                                                         \
GrB_BinaryOp GRB_NAME (PREFIX,OPERATOR) = & GB (OPERATOR ## _opaque) ;

// helper macro to define binary operators that return BOOL
#define BINARY_BOOL(PREFIX,OPERATOR,OPNAME)                                 \
GB_BinaryOp_opaque GB (OPERATOR ## _opaque) =                               \
{                                                                           \
    MAGIC,                                                                  \
    & GB (opaque),                                                          \
    & GB (opaque),                                                          \
    & GB_opaque_BOOL,                                                       \
    & GB (OPERATOR ## _f),                                                  \
    OPNAME,                                                                 \
    GB_ ## OPERATOR ## _opcode                                              \
} ;                                                                         \
GrB_BinaryOp GRB_NAME (PREFIX,OPERATOR) = & GB (OPERATOR ## _opaque) ;

#define TYPE            bool
#define BOOLEAN
#define GRB_NAME(g,x)   g  ## x ## _BOOL
#define GB(x)           GB_ ## x ## _BOOL
#define CAST_NAME(x)    GB_cast_bool_ ## x
#include "GB_ops_template.c"

#define TYPE            int8_t
#define GRB_NAME(g,x)   g ## x ## _INT8
#define GB(x)           GB_ ## x ## _INT8
#define CAST_NAME(x)    GB_cast_int8_t_ ## x
#include "GB_ops_template.c"

#define TYPE            uint8_t
#define UNSIGNED
#define GRB_NAME(g,x)   g ## x ## _UINT8
#define GB(x)           GB_ ## x ## _UINT8
#define CAST_NAME(x)    GB_cast_uint8_t_ ## x
#include "GB_ops_template.c"

#define TYPE            int16_t
#define GRB_NAME(g,x)   g ## x ## _INT16
#define GB(x)           GB_ ## x ## _INT16
#define CAST_NAME(x)    GB_cast_int16_t_ ## x
#include "GB_ops_template.c"

#define TYPE            uint16_t
#define UNSIGNED
#define GRB_NAME(g,x)   g ## x ## _UINT16
#define GB(x)           GB_ ## x ## _UINT16
#define CAST_NAME(x)    GB_cast_uint16_t_ ## x
#include "GB_ops_template.c"

#define TYPE            int32_t
#define GRB_NAME(g,x)   g ## x ## _INT32
#define GB(x)           GB_ ## x ## _INT32
#define CAST_NAME(x)    GB_cast_int32_t_ ## x
#include "GB_ops_template.c"

#define TYPE            uint32_t
#define UNSIGNED
#define GRB_NAME(g,x)   g ## x ## _UINT32
#define GB(x)           GB_ ## x ## _UINT32
#define CAST_NAME(x)    GB_cast_uint32_t_ ## x
#include "GB_ops_template.c"

#define TYPE            int64_t
#define GRB_NAME(g,x)   g ## x ## _INT64
#define GB(x)           GB_ ## x ## _INT64
#define CAST_NAME(x)    GB_cast_int64_t_ ## x
#include "GB_ops_template.c"

#define TYPE            uint64_t
#define UNSIGNED
#define GRB_NAME(g,x)   g ## x ## _UINT64
#define GB(x)           GB_ ## x ## _UINT64
#define CAST_NAME(x)    GB_cast_uint64_t_ ## x
#include "GB_ops_template.c"

#define TYPE            float
#define FLOATING_POINT
#define GRB_NAME(g,x)   g ## x ## _FP32
#define GB(x)           GB_ ## x ## _FP32
#define CAST_NAME(x)    GB_cast_float_ ## x
#include "GB_ops_template.c"

#define TYPE            double
#define FLOATING_POINT
#define GRB_NAME(g,x)   g ## x ## _FP64
#define GB(x)           GB_ ## x ## _FP64
#define CAST_NAME(x)    GB_cast_double_ ## x
#include "GB_ops_template.c"

//------------------------------------------------------------------------------
// special cases for functions and operators
//------------------------------------------------------------------------------

void GB_copy_user_user (void *z, const void *x, size_t s)
{
    memcpy (z, x, s) ;
}

// 4 special cases:
// purely boolean operators: these do not have _BOOL in their name
// They are not created by the templates above.
GrB_UnaryOp  GrB_LNOT = & GB_LNOT_opaque_BOOL ;
GrB_BinaryOp GrB_LOR  = & GB_LOR_opaque_BOOL ;
GrB_BinaryOp GrB_LAND = & GB_LAND_opaque_BOOL ;
GrB_BinaryOp GrB_LXOR = & GB_LXOR_opaque_BOOL ;

//------------------------------------------------------------------------------
// built-in select operators
//------------------------------------------------------------------------------

GB_SelectOp_opaque GB_TRIL_opaque =
{
    MAGIC, NULL, NULL, "tril", GB_TRIL_opcode
} ;

GB_SelectOp_opaque GB_TRIU_opaque =
{
    MAGIC, NULL, NULL, "triu", GB_TRIU_opcode
} ;

GB_SelectOp_opaque GB_DIAG_opaque =
{
    MAGIC, NULL, NULL, "diag", GB_DIAG_opcode
} ;

GB_SelectOp_opaque GB_OFFDIAG_opaque =
{
    MAGIC, NULL, NULL, "offdiag", GB_OFFDIAG_opcode
} ;

GB_SelectOp_opaque GB_NONZERO_opaque =
{
    MAGIC, NULL, NULL, "nonzero", GB_NONZERO_opcode
} ;

GxB_SelectOp GxB_TRIL    = & GB_TRIL_opaque ;
GxB_SelectOp GxB_TRIU    = & GB_TRIU_opaque ;
GxB_SelectOp GxB_DIAG    = & GB_DIAG_opaque ;
GxB_SelectOp GxB_OFFDIAG = & GB_OFFDIAG_opaque ;
GxB_SelectOp GxB_NONZERO = & GB_NONZERO_opaque ;

//------------------------------------------------------------------------------
// GrB_ALL for methods that take lists of indices
//------------------------------------------------------------------------------

// The pointer is never deferenced.  It is passed in as an argument to indicate
// that all indices are to be used, as in the colon in C = A(:,j).

GrB_Index GB_ALL_opaque = 0 ;
const GrB_Index *GrB_ALL = & GB_ALL_opaque ;

//------------------------------------------------------------------------------
// predefined built-in monoids
//------------------------------------------------------------------------------

// helper macro to define built-in monoids
#define MONOID(OP,T,CTYPE,IDENTITY,IDZERO)                                  \
CTYPE GB_identity_ ## OP ## opaque_ ## T = IDENTITY ;                       \
GB_Monoid_opaque GB_ ## OP ## T ## _MONOID_opaque =                         \
{                                                                           \
    MAGIC,                                                                  \
    & GB_ ## OP ## opaque_ ## T,                                            \
    & GB_identity_ ## OP ## opaque_ ## T,                                   \
    IDZERO,                                                                 \
    false,                                                                  \
} ;                                                                         \
GrB_Monoid GxB_ ## OP ## T ## _MONOID  = & GB_ ## OP ## T ## _MONOID_opaque ;

// MIN monoids:
MONOID ( MIN_   , INT8   , int8_t   , INT8_MAX   , false ) ;
MONOID ( MIN_   , UINT8  , uint8_t  , UINT8_MAX  , false ) ;
MONOID ( MIN_   , INT16  , int16_t  , INT16_MAX  , false ) ;
MONOID ( MIN_   , UINT16 , uint16_t , UINT16_MAX , false ) ;
MONOID ( MIN_   , INT32  , int32_t  , INT32_MAX  , false ) ;
MONOID ( MIN_   , UINT32 , uint32_t , UINT32_MAX , false ) ;
MONOID ( MIN_   , INT64  , int64_t  , INT64_MAX  , false ) ;
MONOID ( MIN_   , UINT64 , uint64_t , UINT64_MAX , false ) ;
MONOID ( MIN_   , FP32   , float    , INFINITY   , false ) ;
MONOID ( MIN_   , FP64   , double   , INFINITY   , false ) ;

// MAX monoids:
MONOID ( MAX_   , INT8   , int8_t   , INT8_MIN   , false ) ;
MONOID ( MAX_   , UINT8  , uint8_t  , 0          , true  ) ;
MONOID ( MAX_   , INT16  , int16_t  , INT16_MIN  , false ) ;
MONOID ( MAX_   , UINT16 , uint16_t , 0          , true  ) ;
MONOID ( MAX_   , INT32  , int32_t  , INT32_MIN  , false ) ;
MONOID ( MAX_   , UINT32 , uint32_t , 0          , true  ) ;
MONOID ( MAX_   , INT64  , int64_t  , INT64_MIN  , false ) ;
MONOID ( MAX_   , UINT64 , uint64_t , 0          , true  ) ;
MONOID ( MAX_   , FP32   , float    , -INFINITY  , false ) ;
MONOID ( MAX_   , FP64   , double   , -INFINITY  , false ) ;

// PLUS monoids:
MONOID ( PLUS_  , INT8   , int8_t   , 0          , true  ) ;
MONOID ( PLUS_  , UINT8  , uint8_t  , 0          , true  ) ;
MONOID ( PLUS_  , INT16  , int16_t  , 0          , true  ) ;
MONOID ( PLUS_  , UINT16 , uint16_t , 0          , true  ) ;
MONOID ( PLUS_  , INT32  , int32_t  , 0          , true  ) ;
MONOID ( PLUS_  , UINT32 , uint32_t , 0          , true  ) ;
MONOID ( PLUS_  , INT64  , int64_t  , 0          , true  ) ;
MONOID ( PLUS_  , UINT64 , uint64_t , 0          , true  ) ;
MONOID ( PLUS_  , FP32   , float    , 0          , true  ) ;
MONOID ( PLUS_  , FP64   , double   , 0          , true  ) ;

// TIMES monoids:
MONOID ( TIMES_ , INT8   , int8_t   , 1          , false ) ;
MONOID ( TIMES_ , UINT8  , uint8_t  , 1          , false ) ;
MONOID ( TIMES_ , INT16  , int16_t  , 1          , false ) ;
MONOID ( TIMES_ , UINT16 , uint16_t , 1          , false ) ;
MONOID ( TIMES_ , INT32  , int32_t  , 1          , false ) ;
MONOID ( TIMES_ , UINT32 , uint32_t , 1          , false ) ;
MONOID ( TIMES_ , INT64  , int64_t  , 1          , false ) ;
MONOID ( TIMES_ , UINT64 , uint64_t , 1          , false ) ;
MONOID ( TIMES_ , FP32   , float    , 1          , false ) ;
MONOID ( TIMES_ , FP64   , double   , 1          , false ) ;

// Boolean monoids:
MONOID ( LOR_   , BOOL   , bool     , false      , false ) ;
MONOID ( LAND_  , BOOL   , bool     , true       , true  ) ;
MONOID ( LXOR_  , BOOL   , bool     , false      , false ) ;
MONOID ( EQ_    , BOOL   , bool     , true       , true  ) ;

//------------------------------------------------------------------------------
// predefined built-in semirings
//------------------------------------------------------------------------------

// helper macro to define semirings: all x,y,z types the same
#define SEMIRING(ADD,MULT)                                                  \
GB_Semiring_opaque GB (ADD ## _ ## MULT ## _opaque) =                       \
{                                                                           \
    MAGIC,                                                                  \
    & GM (ADD),                                                             \
    & GB (MULT ## _opaque),                                                 \
    false,                                                                  \
} ;                                                                         \
GrB_Semiring GRB (ADD ## _ ## MULT) = & GB (ADD ## _ ## MULT ## _opaque) ;

// helper macro to define semirings: x,y types the same, z boolean
#define SEMIRING_COMPARE(ADD,MULT)                                          \
GB_Semiring_opaque GB (ADD ## _ ## MULT ## _opaque) =                       \
{                                                                           \
    MAGIC,                                                                  \
    & GMBOOL (ADD),                                                         \
    & GB (MULT ## _opaque),                                                 \
    false,                                                                  \
} ;                                                                         \
GrB_Semiring GRB (ADD ## _ ## MULT) = & GB (ADD ## _ ## MULT ## _opaque) ;

#define GMBOOL(x)     GB_ ## x ## _BOOL_MONOID_opaque

#define BOOLEAN
#define GRB(x)        GxB_ ## x ## _BOOL
#define GB(x)         GB_ ## x ## _BOOL
#define GM(x)         GB_ ## x ## _BOOL ## _MONOID_opaque
#include "GB_semiring_template.c"

#define GRB(x)        GxB_ ## x ## _INT8
#define GB(x)         GB_ ## x ## _INT8
#define GM(x)         GB_ ## x ## _INT8 ## _MONOID_opaque
#include "GB_semiring_template.c"

#define GRB(x)        GxB_ ## x ## _UINT8
#define GB(x)         GB_ ## x ## _UINT8
#define GM(x)         GB_ ## x ## _UINT8 ## _MONOID_opaque
#include "GB_semiring_template.c"

#define GRB(x)        GxB_ ## x ## _INT16
#define GB(x)         GB_ ## x ## _INT16
#define GM(x)         GB_ ## x ## _INT16 ## _MONOID_opaque
#include "GB_semiring_template.c"

#define GRB(x)        GxB_ ## x ## _UINT16
#define GB(x)         GB_ ## x ## _UINT16
#define GM(x)         GB_ ## x ## _UINT16 ## _MONOID_opaque
#include "GB_semiring_template.c"

#define GRB(x)        GxB_ ## x ## _INT32
#define GB(x)         GB_ ## x ## _INT32
#define GM(x)         GB_ ## x ## _INT32 ## _MONOID_opaque
#include "GB_semiring_template.c"

#define GRB(x)        GxB_ ## x ## _UINT32
#define GB(x)         GB_ ## x ## _UINT32
#define GM(x)         GB_ ## x ## _UINT32 ## _MONOID_opaque
#include "GB_semiring_template.c"

#define GRB(x)        GxB_ ## x ## _INT64
#define GB(x)         GB_ ## x ## _INT64
#define GM(x)         GB_ ## x ## _INT64 ## _MONOID_opaque
#include "GB_semiring_template.c"

#define GRB(x)        GxB_ ## x ## _UINT64
#define GB(x)         GB_ ## x ## _UINT64
#define GM(x)         GB_ ## x ## _UINT64 ## _MONOID_opaque
#include "GB_semiring_template.c"

#define GRB(x)        GxB_ ## x ## _FP32
#define GB(x)         GB_ ## x ## _FP32
#define GM(x)         GB_ ## x ## _FP32 ## _MONOID_opaque
#include "GB_semiring_template.c"

#define GRB(x)        GxB_ ## x ## _FP64
#define GB(x)         GB_ ## x ## _FP64
#define GM(x)         GB_ ## x ## _FP64 ## _MONOID_opaque
#include "GB_semiring_template.c"

#undef GMBOOL

#undef UNARY
#undef BINARY
#undef BINARY_BOOL
#undef MONOID
#undef SEMIRING
#undef SEMIRING_COMPARE

