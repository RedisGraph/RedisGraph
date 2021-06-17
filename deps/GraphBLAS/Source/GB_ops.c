//------------------------------------------------------------------------------
// GB_ops.c: built-in types, functions, operators, and other externs
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

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
GB_opaque_GrB_FP64   = {GB_MAGIC, sizeof(double),   GB_FP64_code  , "double"  },
GB_opaque_GxB_FC32   =
    {GB_MAGIC, sizeof (GxB_FC32_t), GB_FC32_code, "float complex" },
GB_opaque_GxB_FC64   =
    {GB_MAGIC, sizeof (GxB_FC64_t), GB_FC64_code, "double complex"} ;

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
    GrB_FP64   = & GB_opaque_GrB_FP64   ,
    GxB_FC32   = & GB_opaque_GxB_FC32   ,
    GxB_FC64   = & GB_opaque_GxB_FC64   ;

//------------------------------------------------------------------------------
// built-in descriptors
//------------------------------------------------------------------------------

#define o ((GrB_Desc_Value) GxB_DEFAULT)

#define GB_DESC(name,out,mask,in0,in1)                          \
struct GB_Descriptor_opaque GB_opaque_desc_ ## name =           \
{                                                               \
    GB_MAGIC,               /* initialized */                   \
    "",                                                         \
    (GrB_Desc_Value) (out),                                     \
    (GrB_Desc_Value) (mask),                                    \
    (GrB_Desc_Value) (in0),                                     \
    (GrB_Desc_Value) (in1),                                     \
    o, o, o,                /* default: axb, #threads, chunk */ \
    true                    /* pre-defined */                   \
} ;                                                             \
GrB_Descriptor GrB_DESC_ ## name = & GB_opaque_desc_ ## name ;

//       name     outp         structure     complement  in0       in1

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
#if !defined ( __cplusplus )
#pragma GCC diagnostic ignored "-Wincompatible-pointer-types"
#endif
#endif

#if ( _MSC_VER && !__INTEL_COMPILER )
// disable MS Visual Studio warnings
GB_PRAGMA (warning (disable : 4146 ))
#endif

// convert a GrB_object into the name of its opaque struct
#define GB_opaque(x) GB_opaque_ ## x

// helper macros to define unary operators
#define GB_OP1zx(prefix,op,str,z_t,ztype,x_t,xtype)                         \
    extern void GB (op ## _f) (z_t *z, const x_t *x) ;                      \
    struct GB_UnaryOp_opaque GB (opaque_ ## prefix ## op) =                 \
    {                                                                       \
        GB_MAGIC,                                                           \
        & GB_opaque (xtype),                                                \
        & GB_opaque (ztype),                                                \
        (GxB_unary_function) (& GB (op ## _f)),                             \
        str,                                                                \
        GB_ ## op ## _opcode                                                \
    } ;                                                                     \
    GrB_UnaryOp GrB_NAME (prefix,op) = & GB (opaque_ ## prefix ## op) ;
#define GB_OP1z(prefix,op,str,z_t,ztype)                                    \
        GB_OP1zx (prefix, op, str, z_t, ztype, GB_TYPE, GB_XTYPE)
#define GB_OP1(prefix,op,str) GB_OP1z (prefix, op, str, GB_TYPE, GB_XTYPE)

#define GB_OP1_RENAME(prefix1,prefix2,op) \
    GrB_UnaryOp GrB_NAME (prefix1,op) = & GB (opaque_ ## prefix2 ## op) ;

// helper macros to define binary operators
#define GB_OP2zxy(prefix,op,str,z_t,ztype,x_t,xtype,y_t,ytype)              \
    extern void GB (op ## _f) (z_t *z, const x_t *x, const y_t *y) ;        \
    struct GB_BinaryOp_opaque GB (opaque_ ## prefix ## op) =                \
    {                                                                       \
        GB_MAGIC,                                                           \
        & GB_opaque (xtype),                                                \
        & GB_opaque (ytype),                                                \
        & GB_opaque (ztype),                                                \
        (GxB_binary_function) (& GB (op ## _f)),                            \
        str,                                                                \
        GB_ ## op ## _opcode                                                \
    } ;                                                                     \
    GrB_BinaryOp GrB_NAME (prefix,op) = & GB (opaque_ ## prefix ## op) ;
#define GB_OP2z(prefix,op,str,z_t,ztype)                                    \
        GB_OP2zxy (prefix, op, str, z_t, ztype,                             \
            GB_TYPE, GB_XTYPE, GB_TYPE, GB_XTYPE)
#define GB_OP2(prefix,op,str) GB_OP2z (prefix, op, str, GB_TYPE, GB_XTYPE)
#define GB_OP2shift(prefix,op,str) \
    GB_OP2zxy (prefix, op, str, GB_TYPE, GB_XTYPE, GB_TYPE, GB_XTYPE,       \
        int8_t, GrB_INT8)

#define GB_TYPE             bool
#define GB_XTYPE            GrB_BOOL
#define GrB_NAME(g,x)       g  ## x ## _BOOL
#define GB(x)               GB_ ## x ## _BOOL
#include "GB_ops_template.c"

#define GB_TYPE             int8_t
#define GB_SIGNED_INT
#define GB_XTYPE            GrB_INT8
#define GrB_NAME(g,x)       g ## x ## _INT8
#define GB(x)               GB_ ## x ## _INT8
#include "GB_ops_template.c"

#define GB_TYPE             uint8_t
#define GB_UNSIGNED_INT
#define GB_XTYPE            GrB_UINT8
#define GrB_NAME(g,x)       g ## x ## _UINT8
#define GB(x)               GB_ ## x ## _UINT8
#include "GB_ops_template.c"

#define GB_TYPE             int16_t
#define GB_SIGNED_INT
#define GB_XTYPE            GrB_INT16
#define GrB_NAME(g,x)       g ## x ## _INT16
#define GB(x)               GB_ ## x ## _INT16
#include "GB_ops_template.c"

#define GB_TYPE             uint16_t
#define GB_UNSIGNED_INT
#define GB_XTYPE            GrB_UINT16
#define GrB_NAME(g,x)       g ## x ## _UINT16
#define GB(x)               GB_ ## x ## _UINT16
#include "GB_ops_template.c"

#define GB_TYPE             int32_t
#define GB_SIGNED_INT
#define GB_XTYPE            GrB_INT32
#define GrB_NAME(g,x)       g ## x ## _INT32
#define GB(x)               GB_ ## x ## _INT32
#include "GB_ops_template.c"

#define GB_TYPE             uint32_t
#define GB_UNSIGNED_INT
#define GB_XTYPE            GrB_UINT32
#define GrB_NAME(g,x)       g ## x ## _UINT32
#define GB(x)               GB_ ## x ## _UINT32
#include "GB_ops_template.c"

#define GB_TYPE             int64_t
#define GB_SIGNED_INT
#define GB_XTYPE            GrB_INT64
#define GrB_NAME(g,x)       g ## x ## _INT64
#define GB(x)               GB_ ## x ## _INT64
#include "GB_ops_template.c"

#define GB_TYPE             uint64_t
#define GB_UNSIGNED_INT
#define GB_XTYPE            GrB_UINT64
#define GrB_NAME(g,x)       g ## x ## _UINT64
#define GB(x)               GB_ ## x ## _UINT64
#include "GB_ops_template.c"

#define GB_TYPE             float
#define GB_XTYPE            GrB_FP32
#define GB_FLOAT
#define GB_FLOATING_POINT
#define GrB_NAME(g,x)       g ## x ## _FP32
#define GB(x)               GB_ ## x ## _FP32
#include "GB_ops_template.c"

#define GB_TYPE             double
#define GB_XTYPE            GrB_FP64
#define GB_DOUBLE
#define GB_FLOATING_POINT
#define GrB_NAME(g,x)       g ## x ## _FP64
#define GB(x)               GB_ ## x ## _FP64
#include "GB_ops_template.c"

#define GB_TYPE             GxB_FC32_t
#define GB_XTYPE            GxB_FC32
#define GB_FLOAT_COMPLEX
#define GB_COMPLEX
#define GB_FLOATING_POINT
#define GrB_NAME(g,x)       g ## x ## _FC32
#define GB(x)               GB_ ## x ## _FC32
#include "GB_ops_template.c"

#define GB_TYPE             GxB_FC64_t
#define GB_XTYPE            GxB_FC64
#define GB_DOUBLE_COMPLEX
#define GB_COMPLEX
#define GB_FLOATING_POINT
#define GrB_NAME(g,x)       g ## x ## _FC64
#define GB(x)               GB_ ## x ## _FC64
#include "GB_ops_template.c"

//------------------------------------------------------------------------------
// special cases for functions and operators
//------------------------------------------------------------------------------

// 5 special cases:
// purely boolean operators: these do not have _BOOL in their name
// They are not created by the templates above.
GrB_UnaryOp  GrB_LNOT  = & GB_opaque_GxB_LNOT_BOOL ;
GrB_BinaryOp GrB_LOR   = & GB_opaque_GxB_LOR_BOOL ;
GrB_BinaryOp GrB_LAND  = & GB_opaque_GxB_LAND_BOOL ;
GrB_BinaryOp GrB_LXOR  = & GB_opaque_GxB_LXOR_BOOL ;
GrB_BinaryOp GrB_LXNOR = & GB_opaque_GrB_EQ_BOOL ;

//------------------------------------------------------------------------------
// positional unary and binary operators
//------------------------------------------------------------------------------

// The function pointer inside a positional operator cannot be called directly,
// since it does not depend on the values of its two arguments.  The operator
// can only be implemented via its opcode.

// helper macros to define positional unary operators
#define GB_OP1_POS(op,str,type)                                             \
    struct GB_UnaryOp_opaque GB_opaque_GxB_ ## op ## type =                 \
    {                                                                       \
        GB_MAGIC,                                                           \
        & GB_opaque (GrB ## type),                                          \
        & GB_opaque (GrB ## type),                                          \
        NULL,  /* op->function is NULL; it cannot be called */              \
        str,                                                                \
        GB_ ## op ## _opcode                                                \
    } ;                                                                     \
    GrB_UnaryOp GxB_ ## op ## type = & GB_opaque_GxB_ ## op ## type ;

// helper macros to define positional binary operators
#define GB_OP2_POS(op,str,type)                                             \
    struct GB_BinaryOp_opaque GB_opaque_GxB_ ## op ## type =                \
    {                                                                       \
        GB_MAGIC,                                                           \
        & GB_opaque (GrB ## type),                                          \
        & GB_opaque (GrB ## type),                                          \
        & GB_opaque (GrB ## type),                                          \
        NULL,  /* op->function is NULL; it cannot be called */              \
        str,                                                                \
        GB_ ## op ## _opcode                                                \
    } ;                                                                     \
    GrB_BinaryOp GxB_ ## op ## type = & GB_opaque_GxB_ ## op ## type ;

GB_OP1_POS (POSITIONI , "positioni" , _INT32) ;
GB_OP1_POS (POSITIONI , "positioni" , _INT64) ;
GB_OP1_POS (POSITIONI1, "positioni1", _INT32) ;
GB_OP1_POS (POSITIONI1, "positioni1", _INT64) ;
GB_OP1_POS (POSITIONJ , "positionj" , _INT32) ;
GB_OP1_POS (POSITIONJ , "positionj" , _INT64) ;
GB_OP1_POS (POSITIONJ1, "positionj1", _INT32) ;
GB_OP1_POS (POSITIONJ1, "positionj1", _INT64) ;

GB_OP2_POS (FIRSTI    , "firsti"    , _INT32) ;
GB_OP2_POS (FIRSTI    , "firsti"    , _INT64) ;
GB_OP2_POS (FIRSTI1   , "firsti1"   , _INT32) ;
GB_OP2_POS (FIRSTI1   , "firsti1"   , _INT64) ;
GB_OP2_POS (FIRSTJ    , "firstj"    , _INT32) ;
GB_OP2_POS (FIRSTJ    , "firstj"    , _INT64) ;
GB_OP2_POS (FIRSTJ1   , "firstj1"   , _INT32) ;
GB_OP2_POS (FIRSTJ1   , "firstj1"   , _INT64) ;

GB_OP2_POS (SECONDI   , "secondi"   , _INT32) ;
GB_OP2_POS (SECONDI   , "secondi"   , _INT64) ;
GB_OP2_POS (SECONDI1  , "secondi1"  , _INT32) ;
GB_OP2_POS (SECONDI1  , "secondi1"  , _INT64) ;
GB_OP2_POS (SECONDJ   , "secondj"   , _INT32) ;
GB_OP2_POS (SECONDJ   , "secondj"   , _INT64) ;
GB_OP2_POS (SECONDJ1  , "secondj1"  , _INT32) ;
GB_OP2_POS (SECONDJ1  , "secondj1"  , _INT64) ;

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

// the default hyper_switch is defined in GB_defaults.h
const double GxB_HYPER_DEFAULT = GB_HYPER_SWITCH_DEFAULT ;

// set GxB_HYPER_SWITCH to either of these to ensure matrix is always, or never,
// stored in hypersparse format, respectively.
const double GxB_ALWAYS_HYPER = GB_ALWAYS_HYPER ;
const double GxB_NEVER_HYPER  = GB_NEVER_HYPER ;
const GxB_Format_Value GxB_FORMAT_DEFAULT = GB_FORMAT_DEFAULT ;

//------------------------------------------------------------------------------
// predefined built-in monoids
//------------------------------------------------------------------------------

#if ( _MSC_VER && !__INTEL_COMPILER )
#define GB_FC32_ONE  {1.0f, 0.0f}
#define GB_FC64_ONE  {1.0 , 0.0 }
#define GB_FC32_ZERO {0.0f, 0.0f}
#define GB_FC64_ZERO {0.0 , 0.0 }
#else
#define GB_FC32_ONE  GxB_CMPLXF (1,0)
#define GB_FC64_ONE  GxB_CMPLX  (1,0)
#define GB_FC32_ZERO GxB_CMPLXF (0,0)
#define GB_FC64_ZERO GxB_CMPLX  (0,0)
#endif

// helper macro to define built-in monoids (no terminal value)
#define GB_MONOID_DEF(prefix,op,ztype,identity)                             \
ztype GB_opaque_identity_ ## op = identity ;                                \
struct GB_Monoid_opaque GB_opaque_GxB_ ## op ## _MONOID =                   \
{                                                                           \
    GB_MAGIC,                                                               \
    & GB_opaque_ ## prefix ## op,                                           \
    & GB_opaque_identity_ ## op,                                            \
    NULL,                                                                   \
    true                                                                    \
} ;                                                                         \
GrB_Monoid GxB_ ## op ## _MONOID = & GB_opaque_GxB_ ## op ## _MONOID ;

// helper macro to define built-in monoids (with terminal value)
#define GB_MONOID_DEFT(prefix,op,ztype,identity,terminal)                   \
ztype GB_opaque_identity_ ## op = identity ;                                \
ztype GB_opaque_terminal_ ## op = terminal ;                                \
struct GB_Monoid_opaque GB_opaque_GxB_ ## op ## _MONOID =                   \
{                                                                           \
    GB_MAGIC,                                                               \
    & GB_opaque_ ## prefix ## op,                                           \
    & GB_opaque_identity_ ## op,                                            \
    & GB_opaque_terminal_ ## op,                                            \
    true                                                                    \
} ;                                                                         \
GrB_Monoid GxB_ ## op ## _MONOID = & GB_opaque_GxB_ ## op ## _MONOID ;

// macro to construct GrB_* monoids in the updated specification
#define GB_MONOID_GRB(op,type)                                              \
GrB_Monoid GrB_ ## op ## _MONOID_ ## type =                                 \
    & GB_opaque_GxB_ ## op ## _ ## type ## _MONOID ;

// MIN monoids:
GB_MONOID_DEFT ( GrB_, MIN_INT8     , int8_t    , INT8_MAX    , INT8_MIN  )
GB_MONOID_DEFT ( GrB_, MIN_INT16    , int16_t   , INT16_MAX   , INT16_MIN )
GB_MONOID_DEFT ( GrB_, MIN_INT32    , int32_t   , INT32_MAX   , INT32_MIN )
GB_MONOID_DEFT ( GrB_, MIN_INT64    , int64_t   , INT64_MAX   , INT64_MIN )
GB_MONOID_DEFT ( GrB_, MIN_UINT8    , uint8_t   , UINT8_MAX   , 0         )
GB_MONOID_DEFT ( GrB_, MIN_UINT16   , uint16_t  , UINT16_MAX  , 0         )
GB_MONOID_DEFT ( GrB_, MIN_UINT32   , uint32_t  , UINT32_MAX  , 0         )
GB_MONOID_DEFT ( GrB_, MIN_UINT64   , uint64_t  , UINT64_MAX  , 0         )
GB_MONOID_DEFT ( GrB_, MIN_FP32     , float     , INFINITY    , -INFINITY )
GB_MONOID_DEFT ( GrB_, MIN_FP64     , double    , INFINITY    , -INFINITY )

GB_MONOID_GRB  ( MIN, INT8     )
GB_MONOID_GRB  ( MIN, INT16    )
GB_MONOID_GRB  ( MIN, INT32    )
GB_MONOID_GRB  ( MIN, INT64    )
GB_MONOID_GRB  ( MIN, UINT8    )
GB_MONOID_GRB  ( MIN, UINT16   )
GB_MONOID_GRB  ( MIN, UINT32   )
GB_MONOID_GRB  ( MIN, UINT64   )
GB_MONOID_GRB  ( MIN, FP32     )
GB_MONOID_GRB  ( MIN, FP64     )

// MAX monoids:
GB_MONOID_DEFT ( GrB_, MAX_INT8     , int8_t    , INT8_MIN    , INT8_MAX  )
GB_MONOID_DEFT ( GrB_, MAX_INT16    , int16_t   , INT16_MIN   , INT16_MAX )
GB_MONOID_DEFT ( GrB_, MAX_INT32    , int32_t   , INT32_MIN   , INT32_MAX )
GB_MONOID_DEFT ( GrB_, MAX_INT64    , int64_t   , INT64_MIN   , INT64_MAX )
GB_MONOID_DEFT ( GrB_, MAX_UINT8    , uint8_t   , 0           , UINT8_MAX )
GB_MONOID_DEFT ( GrB_, MAX_UINT16   , uint16_t  , 0           , UINT16_MAX)
GB_MONOID_DEFT ( GrB_, MAX_UINT32   , uint32_t  , 0           , UINT32_MAX)
GB_MONOID_DEFT ( GrB_, MAX_UINT64   , uint64_t  , 0           , UINT64_MAX)
GB_MONOID_DEFT ( GrB_, MAX_FP32     , float     , -INFINITY   , INFINITY  )
GB_MONOID_DEFT ( GrB_, MAX_FP64     , double    , -INFINITY   , INFINITY  )

GB_MONOID_GRB  ( MAX, INT8     )
GB_MONOID_GRB  ( MAX, INT16    )
GB_MONOID_GRB  ( MAX, INT32    )
GB_MONOID_GRB  ( MAX, INT64    )
GB_MONOID_GRB  ( MAX, UINT8    )
GB_MONOID_GRB  ( MAX, UINT16   )
GB_MONOID_GRB  ( MAX, UINT32   )
GB_MONOID_GRB  ( MAX, UINT64   )
GB_MONOID_GRB  ( MAX, FP32     )
GB_MONOID_GRB  ( MAX, FP64     )

// PLUS monoids:
GB_MONOID_DEF  ( GrB_, PLUS_INT8    , int8_t    , 0           )
GB_MONOID_DEF  ( GrB_, PLUS_INT16   , int16_t   , 0           )
GB_MONOID_DEF  ( GrB_, PLUS_INT32   , int32_t   , 0           )
GB_MONOID_DEF  ( GrB_, PLUS_INT64   , int64_t   , 0           )
GB_MONOID_DEF  ( GrB_, PLUS_UINT8   , uint8_t   , 0           )
GB_MONOID_DEF  ( GrB_, PLUS_UINT16  , uint16_t  , 0           )
GB_MONOID_DEF  ( GrB_, PLUS_UINT32  , uint32_t  , 0           )
GB_MONOID_DEF  ( GrB_, PLUS_UINT64  , uint64_t  , 0           )
GB_MONOID_DEF  ( GrB_, PLUS_FP32    , float     , 0           )
GB_MONOID_DEF  ( GrB_, PLUS_FP64    , double    , 0           )
GB_MONOID_DEF  ( GxB_, PLUS_FC32    , GxB_FC32_t, GB_FC32_ZERO)
GB_MONOID_DEF  ( GxB_, PLUS_FC64    , GxB_FC64_t, GB_FC64_ZERO)

GB_MONOID_GRB  ( PLUS, INT8     )
GB_MONOID_GRB  ( PLUS, INT16    )
GB_MONOID_GRB  ( PLUS, INT32    )
GB_MONOID_GRB  ( PLUS, INT64    )
GB_MONOID_GRB  ( PLUS, UINT8    )
GB_MONOID_GRB  ( PLUS, UINT16   )
GB_MONOID_GRB  ( PLUS, UINT32   )
GB_MONOID_GRB  ( PLUS, UINT64   )
GB_MONOID_GRB  ( PLUS, FP32     )
GB_MONOID_GRB  ( PLUS, FP64     )

// TIMES monoids:
GB_MONOID_DEFT ( GrB_, TIMES_INT8   , int8_t    , 1           , 0)
GB_MONOID_DEFT ( GrB_, TIMES_INT16  , int16_t   , 1           , 0)
GB_MONOID_DEFT ( GrB_, TIMES_INT32  , int32_t   , 1           , 0)
GB_MONOID_DEFT ( GrB_, TIMES_INT64  , int64_t   , 1           , 0)
GB_MONOID_DEFT ( GrB_, TIMES_UINT8  , uint8_t   , 1           , 0)
GB_MONOID_DEFT ( GrB_, TIMES_UINT16 , uint16_t  , 1           , 0)
GB_MONOID_DEFT ( GrB_, TIMES_UINT32 , uint32_t  , 1           , 0)
GB_MONOID_DEFT ( GrB_, TIMES_UINT64 , uint64_t  , 1           , 0)
GB_MONOID_DEF  ( GrB_, TIMES_FP32   , float     , 1           )
GB_MONOID_DEF  ( GrB_, TIMES_FP64   , double    , 1           )
GB_MONOID_DEF  ( GxB_, TIMES_FC32   , GxB_FC32_t, GB_FC32_ONE)
GB_MONOID_DEF  ( GxB_, TIMES_FC64   , GxB_FC64_t, GB_FC64_ONE)

GB_MONOID_GRB  ( TIMES, INT8     )
GB_MONOID_GRB  ( TIMES, INT16    )
GB_MONOID_GRB  ( TIMES, INT32    )
GB_MONOID_GRB  ( TIMES, INT64    )
GB_MONOID_GRB  ( TIMES, UINT8    )
GB_MONOID_GRB  ( TIMES, UINT16   )
GB_MONOID_GRB  ( TIMES, UINT32   )
GB_MONOID_GRB  ( TIMES, UINT64   )
GB_MONOID_GRB  ( TIMES, FP32     )
GB_MONOID_GRB  ( TIMES, FP64     )

// ANY monoids:
GB_MONOID_DEFT ( GxB_, ANY_INT8     , int8_t    , 0           , 0)
GB_MONOID_DEFT ( GxB_, ANY_INT16    , int16_t   , 0           , 0)
GB_MONOID_DEFT ( GxB_, ANY_INT32    , int32_t   , 0           , 0)
GB_MONOID_DEFT ( GxB_, ANY_INT64    , int64_t   , 0           , 0)
GB_MONOID_DEFT ( GxB_, ANY_UINT8    , uint8_t   , 0           , 0)
GB_MONOID_DEFT ( GxB_, ANY_UINT16   , uint16_t  , 0           , 0)
GB_MONOID_DEFT ( GxB_, ANY_UINT32   , uint32_t  , 0           , 0)
GB_MONOID_DEFT ( GxB_, ANY_UINT64   , uint64_t  , 0           , 0)
GB_MONOID_DEFT ( GxB_, ANY_FP32     , float     , 0           , 0)
GB_MONOID_DEFT ( GxB_, ANY_FP64     , double    , 0           , 0)
GB_MONOID_DEFT ( GxB_, ANY_FC32     , GxB_FC32_t, GB_FC32_ZERO, GB_FC32_ZERO)
GB_MONOID_DEFT ( GxB_, ANY_FC64     , GxB_FC64_t, GB_FC64_ZERO, GB_FC64_ZERO)

// Boolean monoids:
GB_MONOID_DEFT ( GxB_, ANY_BOOL     , bool      , false       , false)
GB_MONOID_DEFT ( GxB_, LOR_BOOL     , bool      , false       , true )
GB_MONOID_DEFT ( GxB_, LAND_BOOL    , bool      , true        , false)
GB_MONOID_DEF  ( GxB_, LXOR_BOOL    , bool      , false       )
GB_MONOID_DEF  ( GrB_, EQ_BOOL      , bool      , true        )
// GrB_LXNOR_BOOL_MONIOD is the same as GrB_EQ_BOOL_MONIOD:
GrB_Monoid GxB_LXNOR_BOOL_MONOID = & GB_opaque_GxB_EQ_BOOL_MONOID ;

GB_MONOID_GRB  ( LOR  , BOOL     )
GB_MONOID_GRB  ( LAND , BOOL     )
GB_MONOID_GRB  ( LXOR , BOOL     )
GrB_Monoid GrB_LXNOR_MONOID_BOOL = & GB_opaque_GxB_EQ_BOOL_MONOID ;

// BOR monoids (bitwise or):
GB_MONOID_DEFT ( GrB_, BOR_UINT8    , uint8_t   , 0, 0xFF               )
GB_MONOID_DEFT ( GrB_, BOR_UINT16   , uint16_t  , 0, 0xFFFF             )
GB_MONOID_DEFT ( GrB_, BOR_UINT32   , uint32_t  , 0, 0xFFFFFFFF         )
GB_MONOID_DEFT ( GrB_, BOR_UINT64   , uint64_t  , 0, 0xFFFFFFFFFFFFFFFF )

// BAND monoids (bitwise and):
GB_MONOID_DEFT ( GrB_, BAND_UINT8   , uint8_t   , 0xFF              , 0 )
GB_MONOID_DEFT ( GrB_, BAND_UINT16  , uint16_t  , 0xFFFF            , 0 )
GB_MONOID_DEFT ( GrB_, BAND_UINT32  , uint32_t  , 0xFFFFFFFF        , 0 )
GB_MONOID_DEFT ( GrB_, BAND_UINT64  , uint64_t  , 0xFFFFFFFFFFFFFFFF, 0 )

// BXOR monoids (bitwise xor):
GB_MONOID_DEF  ( GrB_, BXOR_UINT8   , uint8_t   , 0)
GB_MONOID_DEF  ( GrB_, BXOR_UINT16  , uint16_t  , 0)
GB_MONOID_DEF  ( GrB_, BXOR_UINT32  , uint32_t  , 0)
GB_MONOID_DEF  ( GrB_, BXOR_UINT64  , uint64_t  , 0)

// BXNOR monoids (bitwise xnor):
GB_MONOID_DEF  ( GrB_, BXNOR_UINT8  , uint8_t   , 0xFF               )
GB_MONOID_DEF  ( GrB_, BXNOR_UINT16 , uint16_t  , 0xFFFF             )
GB_MONOID_DEF  ( GrB_, BXNOR_UINT32 , uint32_t  , 0xFFFFFFFF         )
GB_MONOID_DEF  ( GrB_, BXNOR_UINT64 , uint64_t  , 0xFFFFFFFFFFFFFFFF )

//------------------------------------------------------------------------------
// predefined built-in semirings
//------------------------------------------------------------------------------

// helper macro to define semirings: all x,y,z types the same
#define GB_SEMIRING_DEFINE(add,prefix,mult)                                 \
struct GB_Semiring_opaque GB (GxB_ ## add ## _ ## mult) =                   \
{                                                                           \
    GB_MAGIC,                                                               \
    & GB_MONOID (add),                                                      \
    & GB (prefix ## mult),                                                  \
    true                                                                    \
} ;                                                                         \
GrB_Semiring GxB_NAME (add ## _ ## mult) = & GB (GxB_ ## add ## _ ## mult) ;

// helper macro to define semirings: x,y types the same, z boolean
#define GB_SEMIRING_COMPARE_DEFINE(add,mult)                                \
struct GB_Semiring_opaque GB (GxB_ ## add ## _ ## mult) =                   \
{                                                                           \
    GB_MAGIC,                                                               \
    & GB_BOOL_MONOID (add),                                                 \
    & GB (GrB_ ## mult),                                                    \
    true                                                                    \
} ;                                                                         \
GrB_Semiring GxB_NAME (add ## _ ## mult) = & GB (GxB_ ## add ## _ ## mult) ;

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

#define GB_UNSIGNED_INT
#define GxB_NAME(x)   GxB_ ## x ## _UINT8
#define GB(x)         GB_opaque_ ## x ## _UINT8
#define GB_MONOID(x)  GB_opaque_GxB_ ## x ## _UINT8_MONOID
#include "GB_semiring_template.c"

#define GxB_NAME(x)   GxB_ ## x ## _INT16
#define GB(x)         GB_opaque_ ## x ## _INT16
#define GB_MONOID(x)  GB_opaque_GxB_ ## x ## _INT16_MONOID
#include "GB_semiring_template.c"

#define GB_UNSIGNED_INT
#define GxB_NAME(x)   GxB_ ## x ## _UINT16
#define GB(x)         GB_opaque_ ## x ## _UINT16
#define GB_MONOID(x)  GB_opaque_GxB_ ## x ## _UINT16_MONOID
#include "GB_semiring_template.c"

#define GxB_NAME(x)   GxB_ ## x ## _INT32
#define GB(x)         GB_opaque_ ## x ## _INT32
#define GB_MONOID(x)  GB_opaque_GxB_ ## x ## _INT32_MONOID
    // 40 positional semirings for INT64 types:
    GB_SEMIRING_DEFINE ( MIN   , GxB_, FIRSTI   )
    GB_SEMIRING_DEFINE ( MIN   , GxB_, FIRSTI1  )
    GB_SEMIRING_DEFINE ( MIN   , GxB_, FIRSTJ   )
    GB_SEMIRING_DEFINE ( MIN   , GxB_, FIRSTJ1  )
    GB_SEMIRING_DEFINE ( MIN   , GxB_, SECONDI  )
    GB_SEMIRING_DEFINE ( MIN   , GxB_, SECONDI1 )
    GB_SEMIRING_DEFINE ( MIN   , GxB_, SECONDJ  )
    GB_SEMIRING_DEFINE ( MIN   , GxB_, SECONDJ1 )
    GB_SEMIRING_DEFINE ( MAX   , GxB_, FIRSTI   )
    GB_SEMIRING_DEFINE ( MAX   , GxB_, FIRSTI1  )
    GB_SEMIRING_DEFINE ( MAX   , GxB_, FIRSTJ   )
    GB_SEMIRING_DEFINE ( MAX   , GxB_, FIRSTJ1  )
    GB_SEMIRING_DEFINE ( MAX   , GxB_, SECONDI  )
    GB_SEMIRING_DEFINE ( MAX   , GxB_, SECONDI1 )
    GB_SEMIRING_DEFINE ( MAX   , GxB_, SECONDJ  )
    GB_SEMIRING_DEFINE ( MAX   , GxB_, SECONDJ1 )
    GB_SEMIRING_DEFINE ( ANY   , GxB_, FIRSTI   )
    GB_SEMIRING_DEFINE ( ANY   , GxB_, FIRSTI1  )
    GB_SEMIRING_DEFINE ( ANY   , GxB_, FIRSTJ   )
    GB_SEMIRING_DEFINE ( ANY   , GxB_, FIRSTJ1  )
    GB_SEMIRING_DEFINE ( ANY   , GxB_, SECONDI  )
    GB_SEMIRING_DEFINE ( ANY   , GxB_, SECONDI1 )
    GB_SEMIRING_DEFINE ( ANY   , GxB_, SECONDJ  )
    GB_SEMIRING_DEFINE ( ANY   , GxB_, SECONDJ1 )
    GB_SEMIRING_DEFINE ( PLUS  , GxB_, FIRSTI   )
    GB_SEMIRING_DEFINE ( PLUS  , GxB_, FIRSTI1  )
    GB_SEMIRING_DEFINE ( PLUS  , GxB_, FIRSTJ   )
    GB_SEMIRING_DEFINE ( PLUS  , GxB_, FIRSTJ1  )
    GB_SEMIRING_DEFINE ( PLUS  , GxB_, SECONDI  )
    GB_SEMIRING_DEFINE ( PLUS  , GxB_, SECONDI1 )
    GB_SEMIRING_DEFINE ( PLUS  , GxB_, SECONDJ  )
    GB_SEMIRING_DEFINE ( PLUS  , GxB_, SECONDJ1 )
    GB_SEMIRING_DEFINE ( TIMES , GxB_, FIRSTI   )
    GB_SEMIRING_DEFINE ( TIMES , GxB_, FIRSTI1  )
    GB_SEMIRING_DEFINE ( TIMES , GxB_, FIRSTJ   )
    GB_SEMIRING_DEFINE ( TIMES , GxB_, FIRSTJ1  )
    GB_SEMIRING_DEFINE ( TIMES , GxB_, SECONDI  )
    GB_SEMIRING_DEFINE ( TIMES , GxB_, SECONDI1 )
    GB_SEMIRING_DEFINE ( TIMES , GxB_, SECONDJ  )
    GB_SEMIRING_DEFINE ( TIMES , GxB_, SECONDJ1 )
#include "GB_semiring_template.c"

#define GB_UNSIGNED_INT
#define GxB_NAME(x)   GxB_ ## x ## _UINT32
#define GB(x)         GB_opaque_ ## x ## _UINT32
#define GB_MONOID(x)  GB_opaque_GxB_ ## x ## _UINT32_MONOID
#include "GB_semiring_template.c"

#define GxB_NAME(x)   GxB_ ## x ## _INT64
#define GB(x)         GB_opaque_ ## x ## _INT64
#define GB_MONOID(x)  GB_opaque_GxB_ ## x ## _INT64_MONOID
    // 40 positional semirings for INT64 types:
    GB_SEMIRING_DEFINE ( MIN   , GxB_, FIRSTI   )
    GB_SEMIRING_DEFINE ( MIN   , GxB_, FIRSTI1  )
    GB_SEMIRING_DEFINE ( MIN   , GxB_, FIRSTJ   )
    GB_SEMIRING_DEFINE ( MIN   , GxB_, FIRSTJ1  )
    GB_SEMIRING_DEFINE ( MIN   , GxB_, SECONDI  )
    GB_SEMIRING_DEFINE ( MIN   , GxB_, SECONDI1 )
    GB_SEMIRING_DEFINE ( MIN   , GxB_, SECONDJ  )
    GB_SEMIRING_DEFINE ( MIN   , GxB_, SECONDJ1 )
    GB_SEMIRING_DEFINE ( MAX   , GxB_, FIRSTI   )
    GB_SEMIRING_DEFINE ( MAX   , GxB_, FIRSTI1  )
    GB_SEMIRING_DEFINE ( MAX   , GxB_, FIRSTJ   )
    GB_SEMIRING_DEFINE ( MAX   , GxB_, FIRSTJ1  )
    GB_SEMIRING_DEFINE ( MAX   , GxB_, SECONDI  )
    GB_SEMIRING_DEFINE ( MAX   , GxB_, SECONDI1 )
    GB_SEMIRING_DEFINE ( MAX   , GxB_, SECONDJ  )
    GB_SEMIRING_DEFINE ( MAX   , GxB_, SECONDJ1 )
    GB_SEMIRING_DEFINE ( ANY   , GxB_, FIRSTI   )
    GB_SEMIRING_DEFINE ( ANY   , GxB_, FIRSTI1  )
    GB_SEMIRING_DEFINE ( ANY   , GxB_, FIRSTJ   )
    GB_SEMIRING_DEFINE ( ANY   , GxB_, FIRSTJ1  )
    GB_SEMIRING_DEFINE ( ANY   , GxB_, SECONDI  )
    GB_SEMIRING_DEFINE ( ANY   , GxB_, SECONDI1 )
    GB_SEMIRING_DEFINE ( ANY   , GxB_, SECONDJ  )
    GB_SEMIRING_DEFINE ( ANY   , GxB_, SECONDJ1 )
    GB_SEMIRING_DEFINE ( PLUS  , GxB_, FIRSTI   )
    GB_SEMIRING_DEFINE ( PLUS  , GxB_, FIRSTI1  )
    GB_SEMIRING_DEFINE ( PLUS  , GxB_, FIRSTJ   )
    GB_SEMIRING_DEFINE ( PLUS  , GxB_, FIRSTJ1  )
    GB_SEMIRING_DEFINE ( PLUS  , GxB_, SECONDI  )
    GB_SEMIRING_DEFINE ( PLUS  , GxB_, SECONDI1 )
    GB_SEMIRING_DEFINE ( PLUS  , GxB_, SECONDJ  )
    GB_SEMIRING_DEFINE ( PLUS  , GxB_, SECONDJ1 )
    GB_SEMIRING_DEFINE ( TIMES , GxB_, FIRSTI   )
    GB_SEMIRING_DEFINE ( TIMES , GxB_, FIRSTI1  )
    GB_SEMIRING_DEFINE ( TIMES , GxB_, FIRSTJ   )
    GB_SEMIRING_DEFINE ( TIMES , GxB_, FIRSTJ1  )
    GB_SEMIRING_DEFINE ( TIMES , GxB_, SECONDI  )
    GB_SEMIRING_DEFINE ( TIMES , GxB_, SECONDI1 )
    GB_SEMIRING_DEFINE ( TIMES , GxB_, SECONDJ  )
    GB_SEMIRING_DEFINE ( TIMES , GxB_, SECONDJ1 )
#include "GB_semiring_template.c"

#define GB_UNSIGNED_INT
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

#define GB_COMPLEX
#define GxB_NAME(x)   GxB_ ## x ## _FC32
#define GB(x)         GB_opaque_ ## x ## _FC32
#define GB_MONOID(x)  GB_opaque_GxB_ ## x ## _FC32_MONOID
#include "GB_semiring_template.c"

#define GB_COMPLEX
#define GxB_NAME(x)   GxB_ ## x ## _FC64
#define GB(x)         GB_opaque_ ## x ## _FC64
#define GB_MONOID(x)  GB_opaque_GxB_ ## x ## _FC64_MONOID
#include "GB_semiring_template.c"

//------------------------------------------------------------------------------
// 124 predefined built-in semirings in the v1.3 C API
//------------------------------------------------------------------------------

// These predefined semirings have been added to the spec, as of v1.3.
// They are identical to the GxB* semirings of the same name, except for
// GrB_LXNOR_LOR_SEMIRING_BOOL, which is identical to GxB_EQ_LOR_BOOL.

#define GB_SEMIRING_GRB(add,mult,type)                              \
GrB_Semiring GrB_ ## add ## _ ## mult ## _SEMIRING_ ## type =       \
    & GB_opaque_GxB_ ## add ## _ ## mult ## _ ## type ;

    //--------------------------------------------------------------------------
    // 4 boolean semirings
    //--------------------------------------------------------------------------

    GB_SEMIRING_GRB (LOR, LAND, BOOL)
    GB_SEMIRING_GRB (LAND, LOR, BOOL)
    GB_SEMIRING_GRB (LXOR, LAND, BOOL)
    GrB_Semiring GrB_LXNOR_LOR_SEMIRING_BOOL = & GB_opaque_GxB_EQ_LOR_BOOL ;

    //--------------------------------------------------------------------------
    // 20 semirings with PLUS monoids
    //--------------------------------------------------------------------------

    GB_SEMIRING_GRB (PLUS, TIMES, INT8)
    GB_SEMIRING_GRB (PLUS, TIMES, INT16)
    GB_SEMIRING_GRB (PLUS, TIMES, INT32)
    GB_SEMIRING_GRB (PLUS, TIMES, INT64)
    GB_SEMIRING_GRB (PLUS, TIMES, UINT8)
    GB_SEMIRING_GRB (PLUS, TIMES, UINT16)
    GB_SEMIRING_GRB (PLUS, TIMES, UINT32)
    GB_SEMIRING_GRB (PLUS, TIMES, UINT64)
    GB_SEMIRING_GRB (PLUS, TIMES, FP32)
    GB_SEMIRING_GRB (PLUS, TIMES, FP64)

    GB_SEMIRING_GRB (PLUS, MIN, INT8)
    GB_SEMIRING_GRB (PLUS, MIN, INT16)
    GB_SEMIRING_GRB (PLUS, MIN, INT32)
    GB_SEMIRING_GRB (PLUS, MIN, INT64)
    GB_SEMIRING_GRB (PLUS, MIN, UINT8)
    GB_SEMIRING_GRB (PLUS, MIN, UINT16)
    GB_SEMIRING_GRB (PLUS, MIN, UINT32)
    GB_SEMIRING_GRB (PLUS, MIN, UINT64)
    GB_SEMIRING_GRB (PLUS, MIN, FP32)
    GB_SEMIRING_GRB (PLUS, MIN, FP64)

    //--------------------------------------------------------------------------
    // 50 semirings with MIN monoids
    //--------------------------------------------------------------------------

    GB_SEMIRING_GRB (MIN, PLUS, INT8)
    GB_SEMIRING_GRB (MIN, PLUS, INT16)
    GB_SEMIRING_GRB (MIN, PLUS, INT32)
    GB_SEMIRING_GRB (MIN, PLUS, INT64)
    GB_SEMIRING_GRB (MIN, PLUS, UINT8)
    GB_SEMIRING_GRB (MIN, PLUS, UINT16)
    GB_SEMIRING_GRB (MIN, PLUS, UINT32)
    GB_SEMIRING_GRB (MIN, PLUS, UINT64)
    GB_SEMIRING_GRB (MIN, PLUS, FP32)
    GB_SEMIRING_GRB (MIN, PLUS, FP64)

    GB_SEMIRING_GRB (MIN, TIMES, INT8)
    GB_SEMIRING_GRB (MIN, TIMES, INT16)
    GB_SEMIRING_GRB (MIN, TIMES, INT32)
    GB_SEMIRING_GRB (MIN, TIMES, INT64)
    GB_SEMIRING_GRB (MIN, TIMES, UINT8)
    GB_SEMIRING_GRB (MIN, TIMES, UINT16)
    GB_SEMIRING_GRB (MIN, TIMES, UINT32)
    GB_SEMIRING_GRB (MIN, TIMES, UINT64)
    GB_SEMIRING_GRB (MIN, TIMES, FP32)
    GB_SEMIRING_GRB (MIN, TIMES, FP64)

    GB_SEMIRING_GRB (MIN, FIRST, INT8)
    GB_SEMIRING_GRB (MIN, FIRST, INT16)
    GB_SEMIRING_GRB (MIN, FIRST, INT32)
    GB_SEMIRING_GRB (MIN, FIRST, INT64)
    GB_SEMIRING_GRB (MIN, FIRST, UINT8)
    GB_SEMIRING_GRB (MIN, FIRST, UINT16)
    GB_SEMIRING_GRB (MIN, FIRST, UINT32)
    GB_SEMIRING_GRB (MIN, FIRST, UINT64)
    GB_SEMIRING_GRB (MIN, FIRST, FP32)
    GB_SEMIRING_GRB (MIN, FIRST, FP64)

    GB_SEMIRING_GRB (MIN, SECOND, INT8)
    GB_SEMIRING_GRB (MIN, SECOND, INT16)
    GB_SEMIRING_GRB (MIN, SECOND, INT32)
    GB_SEMIRING_GRB (MIN, SECOND, INT64)
    GB_SEMIRING_GRB (MIN, SECOND, UINT8)
    GB_SEMIRING_GRB (MIN, SECOND, UINT16)
    GB_SEMIRING_GRB (MIN, SECOND, UINT32)
    GB_SEMIRING_GRB (MIN, SECOND, UINT64)
    GB_SEMIRING_GRB (MIN, SECOND, FP32)
    GB_SEMIRING_GRB (MIN, SECOND, FP64)

    GB_SEMIRING_GRB (MIN, MAX, INT8)
    GB_SEMIRING_GRB (MIN, MAX, INT16)
    GB_SEMIRING_GRB (MIN, MAX, INT32)
    GB_SEMIRING_GRB (MIN, MAX, INT64)
    GB_SEMIRING_GRB (MIN, MAX, UINT8)
    GB_SEMIRING_GRB (MIN, MAX, UINT16)
    GB_SEMIRING_GRB (MIN, MAX, UINT32)
    GB_SEMIRING_GRB (MIN, MAX, UINT64)
    GB_SEMIRING_GRB (MIN, MAX, FP32)
    GB_SEMIRING_GRB (MIN, MAX, FP64)

    //--------------------------------------------------------------------------
    // 50 semirings with MAX monoids
    //--------------------------------------------------------------------------

    GB_SEMIRING_GRB (MAX, PLUS, INT8)
    GB_SEMIRING_GRB (MAX, PLUS, INT16)
    GB_SEMIRING_GRB (MAX, PLUS, INT32)
    GB_SEMIRING_GRB (MAX, PLUS, INT64)
    GB_SEMIRING_GRB (MAX, PLUS, UINT8)
    GB_SEMIRING_GRB (MAX, PLUS, UINT16)
    GB_SEMIRING_GRB (MAX, PLUS, UINT32)
    GB_SEMIRING_GRB (MAX, PLUS, UINT64)
    GB_SEMIRING_GRB (MAX, PLUS, FP32)
    GB_SEMIRING_GRB (MAX, PLUS, FP64)

    GB_SEMIRING_GRB (MAX, TIMES, INT8)
    GB_SEMIRING_GRB (MAX, TIMES, INT16)
    GB_SEMIRING_GRB (MAX, TIMES, INT32)
    GB_SEMIRING_GRB (MAX, TIMES, INT64)
    GB_SEMIRING_GRB (MAX, TIMES, UINT8)
    GB_SEMIRING_GRB (MAX, TIMES, UINT16)
    GB_SEMIRING_GRB (MAX, TIMES, UINT32)
    GB_SEMIRING_GRB (MAX, TIMES, UINT64)
    GB_SEMIRING_GRB (MAX, TIMES, FP32)
    GB_SEMIRING_GRB (MAX, TIMES, FP64)

    GB_SEMIRING_GRB (MAX, FIRST, INT8)
    GB_SEMIRING_GRB (MAX, FIRST, INT16)
    GB_SEMIRING_GRB (MAX, FIRST, INT32)
    GB_SEMIRING_GRB (MAX, FIRST, INT64)
    GB_SEMIRING_GRB (MAX, FIRST, UINT8)
    GB_SEMIRING_GRB (MAX, FIRST, UINT16)
    GB_SEMIRING_GRB (MAX, FIRST, UINT32)
    GB_SEMIRING_GRB (MAX, FIRST, UINT64)
    GB_SEMIRING_GRB (MAX, FIRST, FP32)
    GB_SEMIRING_GRB (MAX, FIRST, FP64)

    GB_SEMIRING_GRB (MAX, SECOND, INT8)
    GB_SEMIRING_GRB (MAX, SECOND, INT16)
    GB_SEMIRING_GRB (MAX, SECOND, INT32)
    GB_SEMIRING_GRB (MAX, SECOND, INT64)
    GB_SEMIRING_GRB (MAX, SECOND, UINT8)
    GB_SEMIRING_GRB (MAX, SECOND, UINT16)
    GB_SEMIRING_GRB (MAX, SECOND, UINT32)
    GB_SEMIRING_GRB (MAX, SECOND, UINT64)
    GB_SEMIRING_GRB (MAX, SECOND, FP32)
    GB_SEMIRING_GRB (MAX, SECOND, FP64)

    GB_SEMIRING_GRB (MAX, MIN, INT8)
    GB_SEMIRING_GRB (MAX, MIN, INT16)
    GB_SEMIRING_GRB (MAX, MIN, INT32)
    GB_SEMIRING_GRB (MAX, MIN, INT64)
    GB_SEMIRING_GRB (MAX, MIN, UINT8)
    GB_SEMIRING_GRB (MAX, MIN, UINT16)
    GB_SEMIRING_GRB (MAX, MIN, UINT32)
    GB_SEMIRING_GRB (MAX, MIN, UINT64)
    GB_SEMIRING_GRB (MAX, MIN, FP32)
    GB_SEMIRING_GRB (MAX, MIN, FP64)

