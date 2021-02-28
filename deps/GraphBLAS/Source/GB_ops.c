//------------------------------------------------------------------------------
// GB_builtin.c: built-in types, functions, operators, and other externs
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// This file defines the predefined built-in types, descriptors, unary
// operators, binary operators, monoids, and semirings.

#include "GB.h"

//------------------------------------------------------------------------------
// built-in types
//------------------------------------------------------------------------------

// extern predefined type objects but opaque to the user
struct GB_Type_opaque
GB_opaque_GrB_BOOL   = {GB_MAGIC, sizeof(bool),     GB_BOOL_code  , "bool"    },
GB_opaque_GrB_INT8   = {GB_MAGIC, sizeof(int8_t),   GB_INT8_code  , "int8_t"  },
GB_opaque_GrB_UINT8  = {GB_MAGIC, sizeof(uint8_t),  GB_UINT8_code , "uint8_t" },
GB_opaque_GrB_INT16  = {GB_MAGIC, sizeof(int16_t),  GB_INT16_code , "int16_t" },
GB_opaque_GrB_UINT16 = {GB_MAGIC, sizeof(uint16_t), GB_UINT16_code, "uint16_t"},
GB_opaque_GrB_INT32  = {GB_MAGIC, sizeof(int32_t),  GB_INT32_code , "int32_t" },
GB_opaque_GrB_UINT32 = {GB_MAGIC, sizeof(uint32_t), GB_UINT32_code, "uint32_t"},
GB_opaque_GrB_INT64  = {GB_MAGIC, sizeof(int64_t),  GB_INT64_code , "int64_t" },
GB_opaque_GrB_UINT64 = {GB_MAGIC, sizeof(uint64_t), GB_UINT64_code, "uint64_t"},
GB_opaque_GrB_FP32   = {GB_MAGIC, sizeof(float),    GB_FP32_code  , "float"   },
GB_opaque_GrB_FP64   = {GB_MAGIC, sizeof(double),   GB_FP64_code  , "double"  };

// extern predefined types (handles to opaque types)
GrB_Type
    GrB_BOOL   = & GB_opaque_GrB_BOOL   ,
    GrB_INT8   = & GB_opaque_GrB_INT8   ,
    GrB_UINT8  = & GB_opaque_GrB_UINT8  ,
    GrB_INT16  = & GB_opaque_GrB_INT16  ,
    GrB_UINT16 = & GB_opaque_GrB_UINT16 ,
    GrB_INT32  = & GB_opaque_GrB_INT32  ,
    GrB_UINT32 = & GB_opaque_GrB_UINT32 ,
    GrB_INT64  = & GB_opaque_GrB_INT64  ,
    GrB_UINT64 = & GB_opaque_GrB_UINT64 ,
    GrB_FP32   = & GB_opaque_GrB_FP32   ,
    GrB_FP64   = & GB_opaque_GrB_FP64   ;

//------------------------------------------------------------------------------
// built-in descriptors
//------------------------------------------------------------------------------

#define o GxB_DEFAULT

#define GB_DESC(name,out,mask,in0,in1)                          \
struct GB_Descriptor_opaque GB_opaque_desc_ ## name =           \
{                                                               \
    GB_MAGIC,               /* initialized */                   \
    out, mask, in0, in1,    /* settings in the spec */          \
    o, o, o,                /* default: axb, #threads, chunk */ \
    true                    /* pre-defined */                   \
} ;                                                             \
GrB_Descriptor GrB_DESC_ ## name = & GB_opaque_desc_ ## name ;

//       name     outp         structure       comp      in0       in1

// GrB_NULL     , o          , o             + o       , o       , o
GB_DESC (T1     , o          , o             + o       , o       , GrB_TRAN )
GB_DESC (T0     , o          , o             + o       , GrB_TRAN, o        )
GB_DESC (T0T1   , o          , o             + o       , GrB_TRAN, GrB_TRAN )

GB_DESC (C      , o          , o             + GrB_COMP, o       , o        )
GB_DESC (CT1    , o          , o             + GrB_COMP, o       , GrB_TRAN )
GB_DESC (CT0    , o          , o             + GrB_COMP, GrB_TRAN, o        )
GB_DESC (CT0T1  , o          , o             + GrB_COMP, GrB_TRAN, GrB_TRAN )

GB_DESC (S      , o          , GrB_STRUCTURE + o       , o       , o        )
GB_DESC (ST1    , o          , GrB_STRUCTURE + o       , o       , GrB_TRAN )
GB_DESC (ST0    , o          , GrB_STRUCTURE + o       , GrB_TRAN, o        )
GB_DESC (ST0T1  , o          , GrB_STRUCTURE + o       , GrB_TRAN, GrB_TRAN )

GB_DESC (SC     , o          , GrB_STRUCTURE + GrB_COMP, o       , o        )
GB_DESC (SCT1   , o          , GrB_STRUCTURE + GrB_COMP, o       , GrB_TRAN )
GB_DESC (SCT0   , o          , GrB_STRUCTURE + GrB_COMP, GrB_TRAN, o        )
GB_DESC (SCT0T1 , o          , GrB_STRUCTURE + GrB_COMP, GrB_TRAN, GrB_TRAN )

GB_DESC (R      , GrB_REPLACE, o             + o       , o       , o        )
GB_DESC (RT1    , GrB_REPLACE, o             + o       , o       , GrB_TRAN )
GB_DESC (RT0    , GrB_REPLACE, o             + o       , GrB_TRAN, o        )
GB_DESC (RT0T1  , GrB_REPLACE, o             + o       , GrB_TRAN, GrB_TRAN )

GB_DESC (RC     , GrB_REPLACE, o             + GrB_COMP, o       , o        )
GB_DESC (RCT1   , GrB_REPLACE, o             + GrB_COMP, o       , GrB_TRAN )
GB_DESC (RCT0   , GrB_REPLACE, o             + GrB_COMP, GrB_TRAN, o        )
GB_DESC (RCT0T1 , GrB_REPLACE, o             + GrB_COMP, GrB_TRAN, GrB_TRAN )

GB_DESC (RS     , GrB_REPLACE, GrB_STRUCTURE + o       , o       , o        )
GB_DESC (RST1   , GrB_REPLACE, GrB_STRUCTURE + o       , o       , GrB_TRAN )
GB_DESC (RST0   , GrB_REPLACE, GrB_STRUCTURE + o       , GrB_TRAN, o        )
GB_DESC (RST0T1 , GrB_REPLACE, GrB_STRUCTURE + o       , GrB_TRAN, GrB_TRAN )

GB_DESC (RSC    , GrB_REPLACE, GrB_STRUCTURE + GrB_COMP, o       , o        )
GB_DESC (RSCT1  , GrB_REPLACE, GrB_STRUCTURE + GrB_COMP, o       , GrB_TRAN )
GB_DESC (RSCT0  , GrB_REPLACE, GrB_STRUCTURE + GrB_COMP, GrB_TRAN, o        )
GB_DESC (RSCT0T1, GrB_REPLACE, GrB_STRUCTURE + GrB_COMP, GrB_TRAN, GrB_TRAN )

#undef o

//------------------------------------------------------------------------------
// built-in unary and binary operators
//------------------------------------------------------------------------------

#if defined __INTEL_COMPILER
// disable icc warnings
//  144:  initialize with incompatible pointer
#pragma warning (disable: 144 )
#elif defined __GNUC__
#pragma GCC diagnostic ignored "-Wincompatible-pointer-types"
#endif

// helper macro to define unary operators; z and x have the same type
#define GB_UNARY_OP_DEFINE(PREFIX,OPERATOR,OPNAME)                          \
struct GB_UnaryOp_opaque GB (opaque_ ## PREFIX ## OPERATOR) =               \
{                                                                           \
    GB_MAGIC,                                                               \
    & GB (opaque_GrB),                                                      \
    & GB (opaque_GrB),                                                      \
    & GB (OPERATOR ## _f),                                                  \
    OPNAME,                                                                 \
    GB_ ## OPERATOR ## _opcode                                              \
} ;                                                                         \
GrB_UnaryOp GrB_NAME (PREFIX,OPERATOR) = & GB (opaque_ ## PREFIX ## OPERATOR) ;

// helper macro to define binary operators: all x,y,z types the same
#define GB_BINARY_OP_DEFINE(PREFIX,OPERATOR,OPNAME)                         \
struct GB_BinaryOp_opaque GB (opaque_ ## PREFIX ## OPERATOR) =              \
{                                                                           \
    GB_MAGIC,                                                               \
    & GB (opaque_GrB),                                                      \
    & GB (opaque_GrB),                                                      \
    & GB (opaque_GrB),                                                      \
    & GB (OPERATOR ## _f),                                                  \
    OPNAME,                                                                 \
    GB_ ## OPERATOR ## _opcode                                              \
} ;                                                                         \
GrB_BinaryOp GrB_NAME (PREFIX,OPERATOR) = & GB (opaque_ ## PREFIX ## OPERATOR) ;

// helper macro to define binary operators that return BOOL
#define GB_BINARY_BOOL_OP_DEFINE(PREFIX,OPERATOR,OPNAME)                    \
struct GB_BinaryOp_opaque GB (opaque_ ## PREFIX ## OPERATOR) =              \
{                                                                           \
    GB_MAGIC,                                                               \
    & GB (opaque_GrB),                                                      \
    & GB (opaque_GrB),                                                      \
    & GB_opaque_GrB_BOOL,                                                   \
    & GB (OPERATOR ## _f),                                                  \
    OPNAME,                                                                 \
    GB_ ## OPERATOR ## _opcode                                              \
} ;                                                                         \
GrB_BinaryOp GrB_NAME (PREFIX,OPERATOR) = & GB (opaque_ ## PREFIX ## OPERATOR) ;

#define GB_TYPE            bool
#define GB_BOOLEAN
#define GrB_NAME(g,x)   g  ## x ## _BOOL
#define GB(x)           GB_ ## x ## _BOOL
#define GB_CAST_NAME(x)    GB_cast_bool_ ## x
#include "GB_ops_template.c"

#define GB_TYPE            int8_t
#define GrB_NAME(g,x)   g ## x ## _INT8
#define GB(x)           GB_ ## x ## _INT8
#define GB_CAST_NAME(x)    GB_cast_int8_t_ ## x
#include "GB_ops_template.c"

#define GB_TYPE            uint8_t
#define GB_UNSIGNED
#define GrB_NAME(g,x)   g ## x ## _UINT8
#define GB(x)           GB_ ## x ## _UINT8
#define GB_CAST_NAME(x)    GB_cast_uint8_t_ ## x
#include "GB_ops_template.c"

#define GB_TYPE            int16_t
#define GrB_NAME(g,x)   g ## x ## _INT16
#define GB(x)           GB_ ## x ## _INT16
#define GB_CAST_NAME(x)    GB_cast_int16_t_ ## x
#include "GB_ops_template.c"

#define GB_TYPE            uint16_t
#define GB_UNSIGNED
#define GrB_NAME(g,x)   g ## x ## _UINT16
#define GB(x)           GB_ ## x ## _UINT16
#define GB_CAST_NAME(x)    GB_cast_uint16_t_ ## x
#include "GB_ops_template.c"

#define GB_TYPE            int32_t
#define GrB_NAME(g,x)   g ## x ## _INT32
#define GB(x)           GB_ ## x ## _INT32
#define GB_CAST_NAME(x)    GB_cast_int32_t_ ## x
#include "GB_ops_template.c"

#define GB_TYPE            uint32_t
#define GB_UNSIGNED
#define GrB_NAME(g,x)   g ## x ## _UINT32
#define GB(x)           GB_ ## x ## _UINT32
#define GB_CAST_NAME(x)    GB_cast_uint32_t_ ## x
#include "GB_ops_template.c"

#define GB_TYPE            int64_t
#define GrB_NAME(g,x)   g ## x ## _INT64
#define GB(x)           GB_ ## x ## _INT64
#define GB_CAST_NAME(x)    GB_cast_int64_t_ ## x
#include "GB_ops_template.c"

#define GB_TYPE            uint64_t
#define GB_UNSIGNED
#define GrB_NAME(g,x)   g ## x ## _UINT64
#define GB(x)           GB_ ## x ## _UINT64
#define GB_CAST_NAME(x)    GB_cast_uint64_t_ ## x
#include "GB_ops_template.c"

#define GB_TYPE            float
#define GB_FLOATING_POINT
#define GrB_NAME(g,x)   g ## x ## _FP32
#define GB(x)           GB_ ## x ## _FP32
#define GB_CAST_NAME(x)    GB_cast_float_ ## x
#include "GB_ops_template.c"

#define GB_TYPE            double
#define GB_FLOATING_POINT
#define GrB_NAME(g,x)   g ## x ## _FP64
#define GB(x)           GB_ ## x ## _FP64
#define GB_CAST_NAME(x)    GB_cast_double_ ## x
#include "GB_ops_template.c"

//------------------------------------------------------------------------------
// special cases for functions and operators
//------------------------------------------------------------------------------

// 4 special cases:
// purely boolean operators: these do not have _BOOL in their name
// They are not created by the templates above.
GrB_UnaryOp  GrB_LNOT = & GB_opaque_GxB_LNOT_BOOL ;
GrB_BinaryOp GrB_LOR  = & GB_opaque_GxB_LOR_BOOL ;
GrB_BinaryOp GrB_LAND = & GB_opaque_GxB_LAND_BOOL ;
GrB_BinaryOp GrB_LXOR = & GB_opaque_GxB_LXOR_BOOL ;

//------------------------------------------------------------------------------
// built-in select operators
//------------------------------------------------------------------------------

struct GB_SelectOp_opaque GB_opaque_GxB_TRIL =
{
    GB_MAGIC, NULL, NULL, NULL, "tril", GB_TRIL_opcode
} ;

struct GB_SelectOp_opaque GB_opaque_GxB_TRIU =
{
    GB_MAGIC, NULL, NULL, NULL, "triu", GB_TRIU_opcode
} ;

struct GB_SelectOp_opaque GB_opaque_GxB_DIAG =
{
    GB_MAGIC, NULL, NULL, NULL, "diag", GB_DIAG_opcode
} ;

struct GB_SelectOp_opaque GB_opaque_GxB_OFFDIAG =
{
    GB_MAGIC, NULL, NULL, NULL, "offdiag", GB_OFFDIAG_opcode
} ;

struct GB_SelectOp_opaque GB_opaque_GxB_NONZERO =
{
    GB_MAGIC, NULL, NULL, NULL, "nonzero", GB_NONZERO_opcode
} ;

struct GB_SelectOp_opaque GB_opaque_GxB_EQ_ZERO =
{
    GB_MAGIC, NULL, NULL, NULL, "eq_zero", GB_EQ_ZERO_opcode
} ;

struct GB_SelectOp_opaque GB_opaque_GxB_GT_ZERO =
{
    GB_MAGIC, NULL, NULL, NULL, "gt_zero", GB_GT_ZERO_opcode
} ;

struct GB_SelectOp_opaque GB_opaque_GxB_GE_ZERO =
{
    GB_MAGIC, NULL, NULL, NULL, "ge_zero", GB_GE_ZERO_opcode
} ;

struct GB_SelectOp_opaque GB_opaque_GxB_LT_ZERO =
{
    GB_MAGIC, NULL, NULL, NULL, "lt_zero", GB_LT_ZERO_opcode
} ;

struct GB_SelectOp_opaque GB_opaque_GxB_LE_ZERO =
{
    GB_MAGIC, NULL, NULL, NULL, "le_zero", GB_LE_ZERO_opcode
} ;

struct GB_SelectOp_opaque GB_opaque_GxB_NE_THUNK =
{
    GB_MAGIC, NULL, NULL, NULL, "ne_thunk", GB_NE_THUNK_opcode
} ;

struct GB_SelectOp_opaque GB_opaque_GxB_EQ_THUNK =
{
    GB_MAGIC, NULL, NULL, NULL, "eq_thunk", GB_EQ_THUNK_opcode
} ;

struct GB_SelectOp_opaque GB_opaque_GxB_GT_THUNK =
{
    GB_MAGIC, NULL, NULL, NULL, "gt_thunk", GB_GT_THUNK_opcode
} ;

struct GB_SelectOp_opaque GB_opaque_GxB_GE_THUNK =
{
    GB_MAGIC, NULL, NULL, NULL, "ge_thunk", GB_GE_THUNK_opcode
} ;

struct GB_SelectOp_opaque GB_opaque_GxB_LT_THUNK =
{
    GB_MAGIC, NULL, NULL, NULL, "lt_thunk", GB_LT_THUNK_opcode
} ;

struct GB_SelectOp_opaque GB_opaque_GxB_LE_THUNK =
{
    GB_MAGIC, NULL, NULL, NULL, "le_thunk", GB_LE_THUNK_opcode
} ;

GxB_SelectOp GxB_TRIL     = & GB_opaque_GxB_TRIL ;
GxB_SelectOp GxB_TRIU     = & GB_opaque_GxB_TRIU ;
GxB_SelectOp GxB_DIAG     = & GB_opaque_GxB_DIAG ;
GxB_SelectOp GxB_OFFDIAG  = & GB_opaque_GxB_OFFDIAG ;

GxB_SelectOp GxB_NONZERO  = & GB_opaque_GxB_NONZERO ;
GxB_SelectOp GxB_EQ_ZERO  = & GB_opaque_GxB_EQ_ZERO ;
GxB_SelectOp GxB_GT_ZERO  = & GB_opaque_GxB_GT_ZERO ;
GxB_SelectOp GxB_GE_ZERO  = & GB_opaque_GxB_GE_ZERO ;
GxB_SelectOp GxB_LT_ZERO  = & GB_opaque_GxB_LT_ZERO ;
GxB_SelectOp GxB_LE_ZERO  = & GB_opaque_GxB_LE_ZERO ;

GxB_SelectOp GxB_NE_THUNK = & GB_opaque_GxB_NE_THUNK ;
GxB_SelectOp GxB_EQ_THUNK = & GB_opaque_GxB_EQ_THUNK ;
GxB_SelectOp GxB_GT_THUNK = & GB_opaque_GxB_GT_THUNK ;
GxB_SelectOp GxB_GE_THUNK = & GB_opaque_GxB_GE_THUNK ;
GxB_SelectOp GxB_LT_THUNK = & GB_opaque_GxB_LT_THUNK ;
GxB_SelectOp GxB_LE_THUNK = & GB_opaque_GxB_LE_THUNK ;

//------------------------------------------------------------------------------
// GrB_ALL
//------------------------------------------------------------------------------

// The GrB_ALL pointer is never deferenced.  It is passed in as an argument to
// indicate that all indices are to be used, as in the colon in C = A(:,j).

GrB_Index GB_opaque_GrB_ALL = 0 ;
const GrB_Index *GrB_ALL = & GB_opaque_GrB_ALL ;

// the default hypersparsity ratio is (1/16)
const double GxB_HYPER_DEFAULT = GB_HYPER_DEFAULT ;

// set GxB_HYPER to either of these to ensure matrix is always, or never,
// stored in hypersparse format, respectively.
const double GxB_ALWAYS_HYPER = GB_ALWAYS_HYPER ;
const double GxB_NEVER_HYPER  = GB_NEVER_HYPER ;
const GxB_Format_Value GxB_FORMAT_DEFAULT = GB_FORMAT_DEFAULT ;

//------------------------------------------------------------------------------
// predefined built-in monoids
//------------------------------------------------------------------------------

// helper macro to define built-in monoids (no terminal value)
#define GB_MONOID_DEFINE(PREFIX,OP,CTYPE,IDENTITY)                          \
CTYPE GB_opaque_identity_ ## OP = IDENTITY ;                                \
struct GB_Monoid_opaque GB_opaque_GxB_ ## OP ## _MONOID =                   \
{                                                                           \
    GB_MAGIC,                                                               \
    & GB_opaque_ ## PREFIX ## OP,                                           \
    & GB_opaque_identity_ ## OP,                                            \
    sizeof (CTYPE),                                                         \
    GB_BUILTIN,                                                             \
    NULL                                                                    \
} ;                                                                         \
GrB_Monoid GxB_ ## OP ## _MONOID = & GB_opaque_GxB_ ## OP ## _MONOID ;

// helper macro to define built-in monoids (with terminal value)
#define GB_MONOID_DEFINE_TERM(PREFIX,OP,CTYPE,IDENTITY,TERMINAL)            \
CTYPE GB_opaque_identity_ ## OP = IDENTITY ;                                \
CTYPE GB_opaque_terminal_ ## OP = TERMINAL ;                                \
struct GB_Monoid_opaque GB_opaque_GxB_ ## OP ## _MONOID =                   \
{                                                                           \
    GB_MAGIC,                                                               \
    & GB_opaque_ ## PREFIX ## OP,                                           \
    & GB_opaque_identity_ ## OP,                                            \
    sizeof (CTYPE),                                                         \
    GB_BUILTIN,                                                             \
    & GB_opaque_terminal_ ## OP                                             \
} ;                                                                         \
GrB_Monoid GxB_ ## OP ## _MONOID = & GB_opaque_GxB_ ## OP ## _MONOID ;

// MIN monoids:
GB_MONOID_DEFINE_TERM ( GrB_, MIN_INT8     , int8_t   , INT8_MAX   , INT8_MIN  )
GB_MONOID_DEFINE_TERM ( GrB_, MIN_INT16    , int16_t  , INT16_MAX  , INT16_MIN )
GB_MONOID_DEFINE_TERM ( GrB_, MIN_INT32    , int32_t  , INT32_MAX  , INT32_MIN )
GB_MONOID_DEFINE_TERM ( GrB_, MIN_INT64    , int64_t  , INT64_MAX  , INT64_MIN )
GB_MONOID_DEFINE_TERM ( GrB_, MIN_UINT8    , uint8_t  , UINT8_MAX  , 0         )
GB_MONOID_DEFINE_TERM ( GrB_, MIN_UINT16   , uint16_t , UINT16_MAX , 0         )
GB_MONOID_DEFINE_TERM ( GrB_, MIN_UINT32   , uint32_t , UINT32_MAX , 0         )
GB_MONOID_DEFINE_TERM ( GrB_, MIN_UINT64   , uint64_t , UINT64_MAX , 0         )
GB_MONOID_DEFINE_TERM ( GrB_, MIN_FP32     , float    , INFINITY   , -INFINITY )
GB_MONOID_DEFINE_TERM ( GrB_, MIN_FP64     , double   ,
    ((double) INFINITY)   , ((double) -INFINITY) )

// MAX monoids:
GB_MONOID_DEFINE_TERM ( GrB_, MAX_INT8     , int8_t   , INT8_MIN   , INT8_MAX  )
GB_MONOID_DEFINE_TERM ( GrB_, MAX_INT16    , int16_t  , INT16_MIN  , INT16_MAX )
GB_MONOID_DEFINE_TERM ( GrB_, MAX_INT32    , int32_t  , INT32_MIN  , INT32_MAX )
GB_MONOID_DEFINE_TERM ( GrB_, MAX_INT64    , int64_t  , INT64_MIN  , INT64_MAX )
GB_MONOID_DEFINE_TERM ( GrB_, MAX_UINT8    , uint8_t  , 0          , UINT8_MAX )
GB_MONOID_DEFINE_TERM ( GrB_, MAX_UINT16   , uint16_t , 0          , UINT16_MAX)
GB_MONOID_DEFINE_TERM ( GrB_, MAX_UINT32   , uint32_t , 0          , UINT32_MAX)
GB_MONOID_DEFINE_TERM ( GrB_, MAX_UINT64   , uint64_t , 0          , UINT64_MAX)
GB_MONOID_DEFINE_TERM ( GrB_, MAX_FP32     , float    , -INFINITY  , INFINITY  )
GB_MONOID_DEFINE_TERM ( GrB_, MAX_FP64     , double   ,
    ((double) -INFINITY)  , ((double) INFINITY)  )

// PLUS monoids:
GB_MONOID_DEFINE      ( GrB_, PLUS_INT8    , int8_t   , 0          )
GB_MONOID_DEFINE      ( GrB_, PLUS_INT16   , int16_t  , 0          )
GB_MONOID_DEFINE      ( GrB_, PLUS_INT32   , int32_t  , 0          )
GB_MONOID_DEFINE      ( GrB_, PLUS_INT64   , int64_t  , 0          )
GB_MONOID_DEFINE      ( GrB_, PLUS_UINT8   , uint8_t  , 0          )
GB_MONOID_DEFINE      ( GrB_, PLUS_UINT16  , uint16_t , 0          )
GB_MONOID_DEFINE      ( GrB_, PLUS_UINT32  , uint32_t , 0          )
GB_MONOID_DEFINE      ( GrB_, PLUS_UINT64  , uint64_t , 0          )
GB_MONOID_DEFINE      ( GrB_, PLUS_FP32    , float    , 0          )
GB_MONOID_DEFINE      ( GrB_, PLUS_FP64    , double   , 0          )

// TIMES monoids:
GB_MONOID_DEFINE_TERM ( GrB_, TIMES_INT8   , int8_t   , 1          , 0)
GB_MONOID_DEFINE_TERM ( GrB_, TIMES_INT16  , int16_t  , 1          , 0)
GB_MONOID_DEFINE_TERM ( GrB_, TIMES_INT32  , int32_t  , 1          , 0)
GB_MONOID_DEFINE_TERM ( GrB_, TIMES_INT64  , int64_t  , 1          , 0)
GB_MONOID_DEFINE_TERM ( GrB_, TIMES_UINT8  , uint8_t  , 1          , 0)
GB_MONOID_DEFINE_TERM ( GrB_, TIMES_UINT16 , uint16_t , 1          , 0)
GB_MONOID_DEFINE_TERM ( GrB_, TIMES_UINT32 , uint32_t , 1          , 0)
GB_MONOID_DEFINE_TERM ( GrB_, TIMES_UINT64 , uint64_t , 1          , 0)
GB_MONOID_DEFINE      ( GrB_, TIMES_FP32   , float    , 1          )
GB_MONOID_DEFINE      ( GrB_, TIMES_FP64   , double   , 1          )

// ANY monoids:
GB_MONOID_DEFINE_TERM ( GxB_, ANY_INT8     , int8_t   , 0, 0)
GB_MONOID_DEFINE_TERM ( GxB_, ANY_INT16    , int16_t  , 0, 0)
GB_MONOID_DEFINE_TERM ( GxB_, ANY_INT32    , int32_t  , 0, 0)
GB_MONOID_DEFINE_TERM ( GxB_, ANY_INT64    , int64_t  , 0, 0)
GB_MONOID_DEFINE_TERM ( GxB_, ANY_UINT8    , uint8_t  , 0, 0)
GB_MONOID_DEFINE_TERM ( GxB_, ANY_UINT16   , uint16_t , 0, 0)
GB_MONOID_DEFINE_TERM ( GxB_, ANY_UINT32   , uint32_t , 0, 0)
GB_MONOID_DEFINE_TERM ( GxB_, ANY_UINT64   , uint64_t , 0, 0)
GB_MONOID_DEFINE_TERM ( GxB_, ANY_FP32     , float    , 0, 0)
GB_MONOID_DEFINE_TERM ( GxB_, ANY_FP64     , double   , 0, 0)
GB_MONOID_DEFINE_TERM ( GxB_, ANY_BOOL     , bool     , 0, 0)

// Boolean monoids:
GB_MONOID_DEFINE_TERM ( GxB_, LOR_BOOL     , bool     , false      , true )
GB_MONOID_DEFINE_TERM ( GxB_, LAND_BOOL    , bool     , true       , false)
GB_MONOID_DEFINE      ( GxB_, LXOR_BOOL    , bool     , false      )
GB_MONOID_DEFINE      ( GrB_, EQ_BOOL      , bool     , true       )

//------------------------------------------------------------------------------
// predefined built-in semirings
//------------------------------------------------------------------------------

// helper macro to define semirings: all x,y,z types the same
#define GB_SEMIRING_DEFINE(ADD,PREFIX,MULT)                                 \
struct GB_Semiring_opaque GB (GxB_ ## ADD ## _ ## MULT) =                   \
{                                                                           \
    GB_MAGIC,                                                               \
    & GB_MONOID (ADD),                                                      \
    & GB (PREFIX ## MULT),                                                  \
    GB_BUILTIN                                                              \
} ;                                                                         \
GrB_Semiring GxB_NAME (ADD ## _ ## MULT) = & GB (GxB_ ## ADD ## _ ## MULT) ;

// helper macro to define semirings: x,y types the same, z boolean
#define GB_SEMIRING_COMPARE_DEFINE(ADD,MULT)                                \
struct GB_Semiring_opaque GB (GxB_ ## ADD ## _ ## MULT) =                   \
{                                                                           \
    GB_MAGIC,                                                               \
    & GB_BOOL_MONOID (ADD),                                                 \
    & GB (GrB_ ## MULT),                                                    \
    GB_BUILTIN                                                              \
} ;                                                                         \
GrB_Semiring GxB_NAME (ADD ## _ ## MULT) = & GB (GxB_ ## ADD ## _ ## MULT) ;

#define GB_BOOL_MONOID(x)     GB_opaque_GxB_ ## x ## _BOOL_MONOID

#define GB_BOOLEAN
#define GxB_NAME(x)   GxB_ ## x ## _BOOL
#define GB(x)         GB_opaque_ ## x ## _BOOL
#define GB_MONOID(x)  GB_opaque_GxB_ ## x ## _BOOL_MONOID
#include "GB_semiring_template.c"

#define GxB_NAME(x)   GxB_ ## x ## _INT8
#define GB(x)         GB_opaque_ ## x ## _INT8
#define GB_MONOID(x)  GB_opaque_GxB_ ## x ## _INT8_MONOID
#include "GB_semiring_template.c"

#define GxB_NAME(x)   GxB_ ## x ## _UINT8
#define GB(x)         GB_opaque_ ## x ## _UINT8
#define GB_MONOID(x)  GB_opaque_GxB_ ## x ## _UINT8_MONOID
#include "GB_semiring_template.c"

#define GxB_NAME(x)   GxB_ ## x ## _INT16
#define GB(x)         GB_opaque_ ## x ## _INT16
#define GB_MONOID(x)  GB_opaque_GxB_ ## x ## _INT16_MONOID
#include "GB_semiring_template.c"

#define GxB_NAME(x)   GxB_ ## x ## _UINT16
#define GB(x)         GB_opaque_ ## x ## _UINT16
#define GB_MONOID(x)  GB_opaque_GxB_ ## x ## _UINT16_MONOID
#include "GB_semiring_template.c"

#define GxB_NAME(x)   GxB_ ## x ## _INT32
#define GB(x)         GB_opaque_ ## x ## _INT32
#define GB_MONOID(x)  GB_opaque_GxB_ ## x ## _INT32_MONOID
#include "GB_semiring_template.c"

#define GxB_NAME(x)   GxB_ ## x ## _UINT32
#define GB(x)         GB_opaque_ ## x ## _UINT32
#define GB_MONOID(x)  GB_opaque_GxB_ ## x ## _UINT32_MONOID
#include "GB_semiring_template.c"

#define GxB_NAME(x)   GxB_ ## x ## _INT64
#define GB(x)         GB_opaque_ ## x ## _INT64
#define GB_MONOID(x)  GB_opaque_GxB_ ## x ## _INT64_MONOID
#include "GB_semiring_template.c"

#define GxB_NAME(x)   GxB_ ## x ## _UINT64
#define GB(x)         GB_opaque_ ## x ## _UINT64
#define GB_MONOID(x)  GB_opaque_GxB_ ## x ## _UINT64_MONOID
#include "GB_semiring_template.c"

#define GxB_NAME(x)   GxB_ ## x ## _FP32
#define GB(x)         GB_opaque_ ## x ## _FP32
#define GB_MONOID(x)  GB_opaque_GxB_ ## x ## _FP32_MONOID
#include "GB_semiring_template.c"

#define GxB_NAME(x)   GxB_ ## x ## _FP64
#define GB(x)         GB_opaque_ ## x ## _FP64
#define GB_MONOID(x)  GB_opaque_GxB_ ## x ## _FP64_MONOID
#include "GB_semiring_template.c"

