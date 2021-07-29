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
GB_OPAQUE (BOOL)   = {GB_MAGIC, 0, sizeof (bool)      , GB_BOOL_code  , "bool"          },
GB_OPAQUE (INT8)   = {GB_MAGIC, 0, sizeof (int8_t)    , GB_INT8_code  , "int8_t"        },
GB_OPAQUE (UINT8)  = {GB_MAGIC, 0, sizeof (uint8_t)   , GB_UINT8_code , "uint8_t"       },
GB_OPAQUE (INT16)  = {GB_MAGIC, 0, sizeof (int16_t)   , GB_INT16_code , "int16_t"       },
GB_OPAQUE (UINT16) = {GB_MAGIC, 0, sizeof (uint16_t)  , GB_UINT16_code, "uint16_t"      },
GB_OPAQUE (INT32)  = {GB_MAGIC, 0, sizeof (int32_t)   , GB_INT32_code , "int32_t"       },
GB_OPAQUE (UINT32) = {GB_MAGIC, 0, sizeof (uint32_t)  , GB_UINT32_code, "uint32_t"      },
GB_OPAQUE (INT64)  = {GB_MAGIC, 0, sizeof (int64_t)   , GB_INT64_code , "int64_t"       },
GB_OPAQUE (UINT64) = {GB_MAGIC, 0, sizeof (uint64_t)  , GB_UINT64_code, "uint64_t"      },
GB_OPAQUE (FP32)   = {GB_MAGIC, 0, sizeof (float)     , GB_FP32_code  , "float"         },
GB_OPAQUE (FP64)   = {GB_MAGIC, 0, sizeof (double)    , GB_FP64_code  , "double"        },
GB_OPAQUE (FC32)   = {GB_MAGIC, 0, sizeof (GxB_FC32_t), GB_FC32_code  , "float complex" },
GB_OPAQUE (FC64)   = {GB_MAGIC, 0, sizeof (GxB_FC64_t), GB_FC64_code  , "double complex"} ;

// extern predefined types (handles to opaque types)
GrB_Type
    GrB_BOOL   = & GB_OPAQUE (BOOL)   ,
    GrB_INT8   = & GB_OPAQUE (INT8)   ,
    GrB_UINT8  = & GB_OPAQUE (UINT8)  ,
    GrB_INT16  = & GB_OPAQUE (INT16)  ,
    GrB_UINT16 = & GB_OPAQUE (UINT16) ,
    GrB_INT32  = & GB_OPAQUE (INT32)  ,
    GrB_UINT32 = & GB_OPAQUE (UINT32) ,
    GrB_INT64  = & GB_OPAQUE (INT64)  ,
    GrB_UINT64 = & GB_OPAQUE (UINT64) ,
    GrB_FP32   = & GB_OPAQUE (FP32)   ,
    GrB_FP64   = & GB_OPAQUE (FP64)   ,
    GxB_FC32   = & GB_OPAQUE (FC32)   ,
    GxB_FC64   = & GB_OPAQUE (FC64)   ;

//------------------------------------------------------------------------------
// built-in descriptors
//------------------------------------------------------------------------------

#define o ((GrB_Desc_Value) GxB_DEFAULT)

#define GB_DESC(name,out,mask,in0,in1)                                  \
    struct GB_Descriptor_opaque GB_OPAQUE (desc_ ## name) =             \
    {                                                                   \
        GB_MAGIC, 0,            /* initialized */                       \
        "", 0,                  /* logger */                            \
        (double) GxB_DEFAULT,   /* chunk */                             \
        (GrB_Desc_Value) (out),                                         \
        (GrB_Desc_Value) (mask),                                        \
        (GrB_Desc_Value) (in0),                                         \
        (GrB_Desc_Value) (in1),                                         \
        o, o,                   /* default: axb, #threads */            \
        0                       /* no sort */                           \
    } ;                                                                 \
    GrB_Descriptor GRB (DESC_ ## name) = & GB_OPAQUE (desc_ ## name) ;

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

#define GB_OP(op) GB_EVAL3 (op, _, GB_XTYPE)

// helper macros to define unary operators
#define GB_OP1zx(op,str,z_t,ztype,x_t,xtype)                                \
    extern void GB_FUNC_T (op, xtype) (z_t *z, const x_t *x) ;              \
    struct GB_UnaryOp_opaque GB_OPAQUE (GB_OP (op)) =                       \
    {                                                                       \
        GB_MAGIC, 0,                                                        \
        & GB_OPAQUE (xtype),                                                \
        & GB_OPAQUE (ztype),                                                \
        (GxB_unary_function) (& GB_FUNC_T (op, xtype)),                     \
        str,                                                                \
        GB_ ## op ## _opcode                                                \
    } ;

#define GRB_OP1z(op,str,z_t,ztype)                                          \
    GB_OP1zx (op, str, z_t, ztype, GB_TYPE, GB_XTYPE)                       \
    GrB_UnaryOp GRB (GB_OP (op)) = & GB_OPAQUE (GB_OP (op)) ;

#define GRB_OP1(op,str) GRB_OP1z (op, str, GB_TYPE, GB_XTYPE)

#define GXB_OP1z(op,str,z_t,ztype)                                          \
    GB_OP1zx (op, str, z_t, ztype, GB_TYPE, GB_XTYPE)                       \
    GrB_UnaryOp GXB (GB_OP (op)) = & GB_OPAQUE (GB_OP (op)) ;

#define GXB_OP1(op,str) GXB_OP1z (op, str, GB_TYPE, GB_XTYPE)

#define GXB_OP1_RENAME(op)                                                  \
    GrB_UnaryOp GXB (GB_OP (op)) = & GB_OPAQUE (GB_OP (op)) ;

// helper macros to define binary operators
#define GB_OP2zxy(op,str,z_t,ztype,x_t,xtype,y_t,ytype)                     \
    extern void GB_FUNC_T(op,xtype) (z_t *z, const x_t *x, const y_t *y) ;  \
    struct GB_BinaryOp_opaque GB_OPAQUE (GB_OP (op)) =                      \
    {                                                                       \
        GB_MAGIC, 0,                                                        \
        & GB_OPAQUE (xtype),                                                \
        & GB_OPAQUE (ytype),                                                \
        & GB_OPAQUE (ztype),                                                \
        (GxB_binary_function) (& GB_FUNC_T (op, xtype)),                    \
        str,                                                                \
        GB_ ## op ## _opcode                                                \
    } ;

#define GRB_OP2z(op,str,z_t,ztype)                                          \
    GB_OP2zxy (op, str, z_t, ztype, GB_TYPE, GB_XTYPE, GB_TYPE, GB_XTYPE)   \
    GrB_BinaryOp GRB (GB_OP (op)) = & GB_OPAQUE (GB_OP (op)) ;

#define GRB_OP2(op,str) GRB_OP2z (op, str, GB_TYPE, GB_XTYPE)

#define GXB_OP2z(op,str,z_t,ztype)                                          \
    GB_OP2zxy (op, str, z_t, ztype, GB_TYPE, GB_XTYPE, GB_TYPE, GB_XTYPE)   \
    GrB_BinaryOp GXB (GB_OP (op)) = & GB_OPAQUE (GB_OP (op)) ;

#define GXB_OP2(op,str) GXB_OP2z (op, str, GB_TYPE, GB_XTYPE)

#define GXB_OP2shift(op,str) \
    GB_OP2zxy (op, str, GB_TYPE, GB_XTYPE, GB_TYPE, GB_XTYPE, int8_t, INT8) \
    GrB_BinaryOp GXB (GB_OP (op)) = & GB_OPAQUE (GB_OP (op)) ;

#define GB_TYPE             bool
#define GB_XTYPE            BOOL
#include "GB_ops_template.c"

#define GB_TYPE             int8_t
#define GB_XTYPE            INT8
#define GB_SIGNED_INT
#include "GB_ops_template.c"

#define GB_TYPE             uint8_t
#define GB_XTYPE            UINT8
#define GB_UNSIGNED_INT
#include "GB_ops_template.c"

#define GB_TYPE             int16_t
#define GB_XTYPE            INT16
#define GB_SIGNED_INT
#include "GB_ops_template.c"

#define GB_TYPE             uint16_t
#define GB_XTYPE            UINT16
#define GB_UNSIGNED_INT
#include "GB_ops_template.c"

#define GB_TYPE             int32_t
#define GB_XTYPE            INT32
#define GB_SIGNED_INT
#include "GB_ops_template.c"

#define GB_TYPE             uint32_t
#define GB_XTYPE            UINT32
#define GB_UNSIGNED_INT
#include "GB_ops_template.c"

#define GB_TYPE             int64_t
#define GB_XTYPE            INT64
#define GB_SIGNED_INT
#include "GB_ops_template.c"

#define GB_TYPE             uint64_t
#define GB_XTYPE            UINT64
#define GB_UNSIGNED_INT
#include "GB_ops_template.c"

#define GB_TYPE             float
#define GB_XTYPE            FP32
#define GB_FLOAT
#define GB_FLOATING_POINT
#include "GB_ops_template.c"

#define GB_TYPE             double
#define GB_XTYPE            FP64
#define GB_DOUBLE
#define GB_FLOATING_POINT
#include "GB_ops_template.c"

#define GB_TYPE             GxB_FC32_t
#define GB_XTYPE            FC32
#define GB_FLOAT_COMPLEX
#define GB_COMPLEX
#define GB_FLOATING_POINT
#include "GB_ops_template.c"

#define GB_TYPE             GxB_FC64_t
#define GB_XTYPE            FC64
#define GB_DOUBLE_COMPLEX
#define GB_COMPLEX
#define GB_FLOATING_POINT
#include "GB_ops_template.c"

//------------------------------------------------------------------------------
// special cases for functions and operators
//------------------------------------------------------------------------------

// 5 special cases:
// purely boolean operators: these do not have _BOOL in their name
// They are not created by the templates above.
GrB_UnaryOp  GrB_LNOT  = & GB_OPAQUE (LNOT_BOOL) ;
GrB_BinaryOp GrB_LOR   = & GB_OPAQUE (LOR_BOOL) ;
GrB_BinaryOp GrB_LAND  = & GB_OPAQUE (LAND_BOOL) ;
GrB_BinaryOp GrB_LXOR  = & GB_OPAQUE (LXOR_BOOL) ;
GrB_BinaryOp GrB_LXNOR = & GB_OPAQUE (EQ_BOOL) ;

//------------------------------------------------------------------------------
// positional unary and binary operators
//------------------------------------------------------------------------------

// The function pointer inside a positional operator cannot be called directly,
// since it does not depend on the values of its two arguments.  The operator
// can only be implemented via its opcode.

// helper macros to define positional unary operators
#define GXB_OP1_POS(op,str,type)                                            \
    struct GB_UnaryOp_opaque GB_OPAQUE (op ## _ ## type) =                  \
    {                                                                       \
        GB_MAGIC, 0,                                                        \
        & GB_OPAQUE (type),                                                 \
        & GB_OPAQUE (type),                                                 \
        NULL,  /* op->function is NULL; it cannot be called */              \
        str,                                                                \
        GB_ ## op ## _opcode                                                \
    } ;                                                                     \
    GrB_UnaryOp GXB (op ## _ ## type) = & GB_OPAQUE (op ## _ ## type) ;

// helper macros to define positional binary operators
#define GXB_OP2_POS(op,str,type)                                            \
    struct GB_BinaryOp_opaque GB_OPAQUE (op ## _ ## type) =                 \
    {                                                                       \
        GB_MAGIC, 0,                                                        \
        & GB_OPAQUE (type),                                                 \
        & GB_OPAQUE (type),                                                 \
        & GB_OPAQUE (type),                                                 \
        NULL,  /* op->function is NULL; it cannot be called */              \
        str,                                                                \
        GB_ ## op ## _opcode                                                \
    } ;                                                                     \
    GrB_BinaryOp GXB (op ## _ ## type) = & GB_OPAQUE (op ## _ ## type) ;

GXB_OP1_POS (POSITIONI , "positioni" , INT32) ;
GXB_OP1_POS (POSITIONI , "positioni" , INT64) ;
GXB_OP1_POS (POSITIONI1, "positioni1", INT32) ;
GXB_OP1_POS (POSITIONI1, "positioni1", INT64) ;
GXB_OP1_POS (POSITIONJ , "positionj" , INT32) ;
GXB_OP1_POS (POSITIONJ , "positionj" , INT64) ;
GXB_OP1_POS (POSITIONJ1, "positionj1", INT32) ;
GXB_OP1_POS (POSITIONJ1, "positionj1", INT64) ;

GXB_OP2_POS (FIRSTI    , "firsti"    , INT32) ;
GXB_OP2_POS (FIRSTI    , "firsti"    , INT64) ;
GXB_OP2_POS (FIRSTI1   , "firsti1"   , INT32) ;
GXB_OP2_POS (FIRSTI1   , "firsti1"   , INT64) ;
GXB_OP2_POS (FIRSTJ    , "firstj"    , INT32) ;
GXB_OP2_POS (FIRSTJ    , "firstj"    , INT64) ;
GXB_OP2_POS (FIRSTJ1   , "firstj1"   , INT32) ;
GXB_OP2_POS (FIRSTJ1   , "firstj1"   , INT64) ;

GXB_OP2_POS (SECONDI   , "secondi"   , INT32) ;
GXB_OP2_POS (SECONDI   , "secondi"   , INT64) ;
GXB_OP2_POS (SECONDI1  , "secondi1"  , INT32) ;
GXB_OP2_POS (SECONDI1  , "secondi1"  , INT64) ;
GXB_OP2_POS (SECONDJ   , "secondj"   , INT32) ;
GXB_OP2_POS (SECONDJ   , "secondj"   , INT64) ;
GXB_OP2_POS (SECONDJ1  , "secondj1"  , INT32) ;
GXB_OP2_POS (SECONDJ1  , "secondj1"  , INT64) ;

//------------------------------------------------------------------------------
// built-in select operators
//------------------------------------------------------------------------------

struct GB_SelectOp_opaque GB_OPAQUE (TRIL    ) = { GB_MAGIC, 0, NULL, NULL, NULL, "tril"    , GB_TRIL_opcode     } ;
struct GB_SelectOp_opaque GB_OPAQUE (TRIU    ) = { GB_MAGIC, 0, NULL, NULL, NULL, "triu"    , GB_TRIU_opcode     } ;
struct GB_SelectOp_opaque GB_OPAQUE (DIAG    ) = { GB_MAGIC, 0, NULL, NULL, NULL, "diag"    , GB_DIAG_opcode     } ;
struct GB_SelectOp_opaque GB_OPAQUE (OFFDIAG ) = { GB_MAGIC, 0, NULL, NULL, NULL, "offdiag" , GB_OFFDIAG_opcode  } ;
struct GB_SelectOp_opaque GB_OPAQUE (NONZERO ) = { GB_MAGIC, 0, NULL, NULL, NULL, "nonzero" , GB_NONZERO_opcode  } ;
struct GB_SelectOp_opaque GB_OPAQUE (EQ_ZERO ) = { GB_MAGIC, 0, NULL, NULL, NULL, "eq_zero" , GB_EQ_ZERO_opcode  } ;
struct GB_SelectOp_opaque GB_OPAQUE (GT_ZERO ) = { GB_MAGIC, 0, NULL, NULL, NULL, "gt_zero" , GB_GT_ZERO_opcode  } ;
struct GB_SelectOp_opaque GB_OPAQUE (GE_ZERO ) = { GB_MAGIC, 0, NULL, NULL, NULL, "ge_zero" , GB_GE_ZERO_opcode  } ;
struct GB_SelectOp_opaque GB_OPAQUE (LT_ZERO ) = { GB_MAGIC, 0, NULL, NULL, NULL, "lt_zero" , GB_LT_ZERO_opcode  } ;
struct GB_SelectOp_opaque GB_OPAQUE (LE_ZERO ) = { GB_MAGIC, 0, NULL, NULL, NULL, "le_zero" , GB_LE_ZERO_opcode  } ;
struct GB_SelectOp_opaque GB_OPAQUE (NE_THUNK) = { GB_MAGIC, 0, NULL, NULL, NULL, "ne_thunk", GB_NE_THUNK_opcode } ;
struct GB_SelectOp_opaque GB_OPAQUE (EQ_THUNK) = { GB_MAGIC, 0, NULL, NULL, NULL, "eq_thunk", GB_EQ_THUNK_opcode } ;
struct GB_SelectOp_opaque GB_OPAQUE (GT_THUNK) = { GB_MAGIC, 0, NULL, NULL, NULL, "gt_thunk", GB_GT_THUNK_opcode } ;
struct GB_SelectOp_opaque GB_OPAQUE (GE_THUNK) = { GB_MAGIC, 0, NULL, NULL, NULL, "ge_thunk", GB_GE_THUNK_opcode } ; 
struct GB_SelectOp_opaque GB_OPAQUE (LT_THUNK) = { GB_MAGIC, 0, NULL, NULL, NULL, "lt_thunk", GB_LT_THUNK_opcode } ;
struct GB_SelectOp_opaque GB_OPAQUE (LE_THUNK) = { GB_MAGIC, 0, NULL, NULL, NULL, "le_thunk", GB_LE_THUNK_opcode } ;

GxB_SelectOp GxB_TRIL     = & GB_OPAQUE (TRIL) ;
GxB_SelectOp GxB_TRIU     = & GB_OPAQUE (TRIU) ;
GxB_SelectOp GxB_DIAG     = & GB_OPAQUE (DIAG) ;
GxB_SelectOp GxB_OFFDIAG  = & GB_OPAQUE (OFFDIAG) ;

GxB_SelectOp GxB_NONZERO  = & GB_OPAQUE (NONZERO) ;
GxB_SelectOp GxB_EQ_ZERO  = & GB_OPAQUE (EQ_ZERO) ;
GxB_SelectOp GxB_GT_ZERO  = & GB_OPAQUE (GT_ZERO) ;
GxB_SelectOp GxB_GE_ZERO  = & GB_OPAQUE (GE_ZERO) ;
GxB_SelectOp GxB_LT_ZERO  = & GB_OPAQUE (LT_ZERO) ;
GxB_SelectOp GxB_LE_ZERO  = & GB_OPAQUE (LE_ZERO) ;

GxB_SelectOp GxB_NE_THUNK = & GB_OPAQUE (NE_THUNK) ;
GxB_SelectOp GxB_EQ_THUNK = & GB_OPAQUE (EQ_THUNK) ;
GxB_SelectOp GxB_GT_THUNK = & GB_OPAQUE (GT_THUNK) ;
GxB_SelectOp GxB_GE_THUNK = & GB_OPAQUE (GE_THUNK) ;
GxB_SelectOp GxB_LT_THUNK = & GB_OPAQUE (LT_THUNK) ;
GxB_SelectOp GxB_LE_THUNK = & GB_OPAQUE (LE_THUNK) ;

//------------------------------------------------------------------------------
// GrB_ALL
//------------------------------------------------------------------------------

// The GrB_ALL pointer is never deferenced.  It is passed in as an argument to
// indicate that all indices are to be used, as in the colon in C = A(:,j).

GrB_Index GB_OPAQUE (ALL) = 0 ;
const GrB_Index *GrB_ALL = & GB_OPAQUE (ALL) ;

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
#define GB_MONOID_DEF(op,ztype,identity)                                    \
    ztype GB_OPAQUE (GB_EVAL2 (identity_, op)) = identity ;                 \
    struct GB_Monoid_opaque GB_OPAQUE (GB_EVAL2 (op, _MONOID)) =            \
    {                                                                       \
        GB_MAGIC, 0,                                                        \
        & GB_OPAQUE (op),                                                   \
        & GB_OPAQUE (GB_EVAL2 (identity_, op)),                             \
        NULL,                                                               \
        0, 0                                                                \
    } ;                                                                     \
    GrB_Monoid GXB (GB_EVAL2 (op, _MONOID)) =                               \
        & GB_OPAQUE (GB_EVAL2 (op, _MONOID)) ;

// helper macro to define built-in monoids (with terminal value)
#define GB_MONOID_DEFT(op,ztype,identity,terminal)                          \
    ztype GB_OPAQUE (GB_EVAL2 (identity_, op)) = identity ;                 \
    ztype GB_OPAQUE (GB_EVAL2 (terminal_, op)) = terminal ;                 \
    struct GB_Monoid_opaque GB_OPAQUE (GB_EVAL2 (op, _MONOID)) =            \
    {                                                                       \
        GB_MAGIC, 0,                                                        \
        & GB_OPAQUE (op),                                                   \
        & GB_OPAQUE (GB_EVAL2 (identity_, op)),                             \
        & GB_OPAQUE (GB_EVAL2 (terminal_, op)),                             \
        0, 0                                                                \
    } ;                                                                     \
    GrB_Monoid GXB (GB_EVAL2 (op, _MONOID)) =                               \
        & GB_OPAQUE (GB_EVAL2 (op, _MONOID)) ;

// macro to construct GrB_* monoids in the updated specification
#define GB_MONOID_GRB(op,type)                                              \
GrB_Monoid GRB (GB_EVAL3 (op, _MONOID_, type)) =                            \
    & GB_OPAQUE (GB_EVAL4 (op, _, type, _MONOID)) ;

// MIN monoids:
GB_MONOID_DEFT ( MIN_INT8     , int8_t    , INT8_MAX    , INT8_MIN  )
GB_MONOID_DEFT ( MIN_INT16    , int16_t   , INT16_MAX   , INT16_MIN )
GB_MONOID_DEFT ( MIN_INT32    , int32_t   , INT32_MAX   , INT32_MIN )
GB_MONOID_DEFT ( MIN_INT64    , int64_t   , INT64_MAX   , INT64_MIN )
GB_MONOID_DEFT ( MIN_UINT8    , uint8_t   , UINT8_MAX   , 0         )
GB_MONOID_DEFT ( MIN_UINT16   , uint16_t  , UINT16_MAX  , 0         )
GB_MONOID_DEFT ( MIN_UINT32   , uint32_t  , UINT32_MAX  , 0         )
GB_MONOID_DEFT ( MIN_UINT64   , uint64_t  , UINT64_MAX  , 0         )
GB_MONOID_DEFT ( MIN_FP32     , float     , INFINITY    , -INFINITY )
GB_MONOID_DEFT ( MIN_FP64     , double    , INFINITY    , -INFINITY )

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
GB_MONOID_DEFT ( MAX_INT8     , int8_t    , INT8_MIN    , INT8_MAX  )
GB_MONOID_DEFT ( MAX_INT16    , int16_t   , INT16_MIN   , INT16_MAX )
GB_MONOID_DEFT ( MAX_INT32    , int32_t   , INT32_MIN   , INT32_MAX )
GB_MONOID_DEFT ( MAX_INT64    , int64_t   , INT64_MIN   , INT64_MAX )
GB_MONOID_DEFT ( MAX_UINT8    , uint8_t   , 0           , UINT8_MAX )
GB_MONOID_DEFT ( MAX_UINT16   , uint16_t  , 0           , UINT16_MAX)
GB_MONOID_DEFT ( MAX_UINT32   , uint32_t  , 0           , UINT32_MAX)
GB_MONOID_DEFT ( MAX_UINT64   , uint64_t  , 0           , UINT64_MAX)
GB_MONOID_DEFT ( MAX_FP32     , float     , -INFINITY   , INFINITY  )
GB_MONOID_DEFT ( MAX_FP64     , double    , -INFINITY   , INFINITY  )

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
GB_MONOID_DEF  ( PLUS_INT8    , int8_t    , 0           )
GB_MONOID_DEF  ( PLUS_INT16   , int16_t   , 0           )
GB_MONOID_DEF  ( PLUS_INT32   , int32_t   , 0           )
GB_MONOID_DEF  ( PLUS_INT64   , int64_t   , 0           )
GB_MONOID_DEF  ( PLUS_UINT8   , uint8_t   , 0           )
GB_MONOID_DEF  ( PLUS_UINT16  , uint16_t  , 0           )
GB_MONOID_DEF  ( PLUS_UINT32  , uint32_t  , 0           )
GB_MONOID_DEF  ( PLUS_UINT64  , uint64_t  , 0           )
GB_MONOID_DEF  ( PLUS_FP32    , float     , 0           )
GB_MONOID_DEF  ( PLUS_FP64    , double    , 0           )
GB_MONOID_DEF  ( PLUS_FC32    , GxB_FC32_t, GB_FC32_ZERO)
GB_MONOID_DEF  ( PLUS_FC64    , GxB_FC64_t, GB_FC64_ZERO)

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
GB_MONOID_DEFT ( TIMES_INT8   , int8_t    , 1           , 0)
GB_MONOID_DEFT ( TIMES_INT16  , int16_t   , 1           , 0)
GB_MONOID_DEFT ( TIMES_INT32  , int32_t   , 1           , 0)
GB_MONOID_DEFT ( TIMES_INT64  , int64_t   , 1           , 0)
GB_MONOID_DEFT ( TIMES_UINT8  , uint8_t   , 1           , 0)
GB_MONOID_DEFT ( TIMES_UINT16 , uint16_t  , 1           , 0)
GB_MONOID_DEFT ( TIMES_UINT32 , uint32_t  , 1           , 0)
GB_MONOID_DEFT ( TIMES_UINT64 , uint64_t  , 1           , 0)
GB_MONOID_DEF  ( TIMES_FP32   , float     , 1           )
GB_MONOID_DEF  ( TIMES_FP64   , double    , 1           )
GB_MONOID_DEF  ( TIMES_FC32   , GxB_FC32_t, GB_FC32_ONE )
GB_MONOID_DEF  ( TIMES_FC64   , GxB_FC64_t, GB_FC64_ONE )

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
GB_MONOID_DEFT ( ANY_INT8     , int8_t    , 0           , 0)
GB_MONOID_DEFT ( ANY_INT16    , int16_t   , 0           , 0)
GB_MONOID_DEFT ( ANY_INT32    , int32_t   , 0           , 0)
GB_MONOID_DEFT ( ANY_INT64    , int64_t   , 0           , 0)
GB_MONOID_DEFT ( ANY_UINT8    , uint8_t   , 0           , 0)
GB_MONOID_DEFT ( ANY_UINT16   , uint16_t  , 0           , 0)
GB_MONOID_DEFT ( ANY_UINT32   , uint32_t  , 0           , 0)
GB_MONOID_DEFT ( ANY_UINT64   , uint64_t  , 0           , 0)
GB_MONOID_DEFT ( ANY_FP32     , float     , 0           , 0)
GB_MONOID_DEFT ( ANY_FP64     , double    , 0           , 0)
GB_MONOID_DEFT ( ANY_FC32     , GxB_FC32_t, GB_FC32_ZERO, GB_FC32_ZERO)
GB_MONOID_DEFT ( ANY_FC64     , GxB_FC64_t, GB_FC64_ZERO, GB_FC64_ZERO)

// Boolean monoids:
GB_MONOID_DEFT ( ANY_BOOL     , bool      , false       , false)
GB_MONOID_DEFT ( LOR_BOOL     , bool      , false       , true )
GB_MONOID_DEFT ( LAND_BOOL    , bool      , true        , false)
GB_MONOID_DEF  ( LXOR_BOOL    , bool      , false       )
GB_MONOID_DEF  ( EQ_BOOL      , bool      , true        )
// GrB_LXNOR_BOOL_MONIOD is the same as GrB_EQ_BOOL_MONIOD:
GrB_Monoid GxB_LXNOR_BOOL_MONOID = & GB_OPAQUE (EQ_BOOL_MONOID) ;

GB_MONOID_GRB  ( LOR  , BOOL     )
GB_MONOID_GRB  ( LAND , BOOL     )
GB_MONOID_GRB  ( LXOR , BOOL     )
GrB_Monoid GrB_LXNOR_MONOID_BOOL = & GB_OPAQUE (EQ_BOOL_MONOID) ;

// BOR monoids (bitwise or):
GB_MONOID_DEFT ( BOR_UINT8    , uint8_t   , 0, 0xFF               )
GB_MONOID_DEFT ( BOR_UINT16   , uint16_t  , 0, 0xFFFF             )
GB_MONOID_DEFT ( BOR_UINT32   , uint32_t  , 0, 0xFFFFFFFF         )
GB_MONOID_DEFT ( BOR_UINT64   , uint64_t  , 0, 0xFFFFFFFFFFFFFFFF )

// BAND monoids (bitwise and):
GB_MONOID_DEFT ( BAND_UINT8   , uint8_t   , 0xFF              , 0 )
GB_MONOID_DEFT ( BAND_UINT16  , uint16_t  , 0xFFFF            , 0 )
GB_MONOID_DEFT ( BAND_UINT32  , uint32_t  , 0xFFFFFFFF        , 0 )
GB_MONOID_DEFT ( BAND_UINT64  , uint64_t  , 0xFFFFFFFFFFFFFFFF, 0 )

// BXOR monoids (bitwise xor):
GB_MONOID_DEF  ( BXOR_UINT8   , uint8_t   , 0)
GB_MONOID_DEF  ( BXOR_UINT16  , uint16_t  , 0)
GB_MONOID_DEF  ( BXOR_UINT32  , uint32_t  , 0)
GB_MONOID_DEF  ( BXOR_UINT64  , uint64_t  , 0)

// BXNOR monoids (bitwise xnor):
GB_MONOID_DEF  ( BXNOR_UINT8  , uint8_t   , 0xFF               )
GB_MONOID_DEF  ( BXNOR_UINT16 , uint16_t  , 0xFFFF             )
GB_MONOID_DEF  ( BXNOR_UINT32 , uint32_t  , 0xFFFFFFFF         )
GB_MONOID_DEF  ( BXNOR_UINT64 , uint64_t  , 0xFFFFFFFFFFFFFFFF )

//------------------------------------------------------------------------------
// predefined built-in semirings
//------------------------------------------------------------------------------

#define GB_SEMIRING_NAME(add,mult) \
    GB_EVAL5 (add, _, mult, _, GB_XTYPE)

// helper macro to define semirings: all x,y,z types the same
#define GXB_SEMIRING(add,mult)                                              \
    struct GB_Semiring_opaque GB_OPAQUE (GB_SEMIRING_NAME(add, mult)) =     \
    {                                                                       \
        GB_MAGIC, 0,                                                        \
        & GB_OPAQUE (GB_EVAL4 (add, _, GB_XTYPE, _MONOID)),                 \
        & GB_OPAQUE (GB_EVAL3 (mult, _, GB_XTYPE)),                         \
    } ;                                                                     \
    GrB_Semiring GXB (GB_SEMIRING_NAME (add, mult)) =                       \
         & GB_OPAQUE (GB_SEMIRING_NAME (add, mult)) ;

// helper macro to define semirings: x,y types the same, z boolean
#define GB_SEMIRING_COMPARE_DEFINE(add,mult)                                \
    struct GB_Semiring_opaque GB_OPAQUE (GB_SEMIRING_NAME(add, mult)) =     \
    {                                                                       \
        GB_MAGIC, 0,                                                        \
        & GB_OPAQUE (GB_EVAL2 (add, _BOOL_MONOID)),                         \
        & GB_OPAQUE (GB_EVAL3 (mult, _, GB_XTYPE)),                         \
    } ;

#define GXB_SEMIRING_COMPARE(add,mult)                                      \
    GB_SEMIRING_COMPARE_DEFINE (add, mult)                                  \
    GrB_Semiring GXB (GB_SEMIRING_NAME (add, mult)) =                       \
         & GB_OPAQUE (GB_SEMIRING_NAME (add, mult)) ;

#define GB_XTYPE    BOOL
#define GB_BOOLEAN
#include "GB_semiring_template.c"

#define GB_XTYPE    INT8
#include "GB_semiring_template.c"

#define GB_XTYPE    UINT8
#define GB_UNSIGNED_INT
#include "GB_semiring_template.c"

#define GB_XTYPE    INT16
#include "GB_semiring_template.c"

#define GB_XTYPE    UINT16
#define GB_UNSIGNED_INT
#include "GB_semiring_template.c"

#define GB_XTYPE    INT32
#define GB_POSITIONAL
#include "GB_semiring_template.c"

#define GB_UNSIGNED_INT
#define GB_XTYPE    UINT32
#include "GB_semiring_template.c"

#define GB_POSITIONAL
#define GB_XTYPE    INT64
#include "GB_semiring_template.c"

#define GB_UNSIGNED_INT
#define GB_XTYPE    UINT64
#include "GB_semiring_template.c"

#define GB_XTYPE    FP32
#include "GB_semiring_template.c"

#define GB_XTYPE    FP64
#include "GB_semiring_template.c"

#define GB_COMPLEX
#define GB_XTYPE    FC32
#include "GB_semiring_template.c"

#define GB_COMPLEX
#define GB_XTYPE    FC64
#include "GB_semiring_template.c"

//------------------------------------------------------------------------------
// 124 predefined built-in semirings in the v1.3 C API
//------------------------------------------------------------------------------

// These predefined semirings have been added to the spec, as of v1.3.
// They are identical to the GxB* semirings of the same name, except for
// GrB_LXNOR_LOR_SEMIRING_BOOL, which is identical to GxB_EQ_LOR_BOOL.

#define GRB_SEMIRING(add,mult,xtype)                                \
GrB_Semiring GRB (GB_EVAL5 (add, _, mult, _SEMIRING_, xtype)) =     \
    & GB_OPAQUE (GB_EVAL5 (add, _, mult, _, xtype)) ;

    //--------------------------------------------------------------------------
    // 4 boolean semirings
    //--------------------------------------------------------------------------

    GRB_SEMIRING (LOR, LAND, BOOL)
    GRB_SEMIRING (LAND, LOR, BOOL)
    GRB_SEMIRING (LXOR, LAND, BOOL)
    GrB_Semiring GRB (LXNOR_LOR_SEMIRING_BOOL) = & GB_OPAQUE (EQ_LOR_BOOL) ;

    //--------------------------------------------------------------------------
    // 20 semirings with PLUS monoids
    //--------------------------------------------------------------------------

    GRB_SEMIRING (PLUS, TIMES, INT8)
    GRB_SEMIRING (PLUS, TIMES, INT16)
    GRB_SEMIRING (PLUS, TIMES, INT32)
    GRB_SEMIRING (PLUS, TIMES, INT64)
    GRB_SEMIRING (PLUS, TIMES, UINT8)
    GRB_SEMIRING (PLUS, TIMES, UINT16)
    GRB_SEMIRING (PLUS, TIMES, UINT32)
    GRB_SEMIRING (PLUS, TIMES, UINT64)
    GRB_SEMIRING (PLUS, TIMES, FP32)
    GRB_SEMIRING (PLUS, TIMES, FP64)

    GRB_SEMIRING (PLUS, MIN, INT8)
    GRB_SEMIRING (PLUS, MIN, INT16)
    GRB_SEMIRING (PLUS, MIN, INT32)
    GRB_SEMIRING (PLUS, MIN, INT64)
    GRB_SEMIRING (PLUS, MIN, UINT8)
    GRB_SEMIRING (PLUS, MIN, UINT16)
    GRB_SEMIRING (PLUS, MIN, UINT32)
    GRB_SEMIRING (PLUS, MIN, UINT64)
    GRB_SEMIRING (PLUS, MIN, FP32)
    GRB_SEMIRING (PLUS, MIN, FP64)

    //--------------------------------------------------------------------------
    // 50 semirings with MIN monoids
    //--------------------------------------------------------------------------

    GRB_SEMIRING (MIN, PLUS, INT8)
    GRB_SEMIRING (MIN, PLUS, INT16)
    GRB_SEMIRING (MIN, PLUS, INT32)
    GRB_SEMIRING (MIN, PLUS, INT64)
    GRB_SEMIRING (MIN, PLUS, UINT8)
    GRB_SEMIRING (MIN, PLUS, UINT16)
    GRB_SEMIRING (MIN, PLUS, UINT32)
    GRB_SEMIRING (MIN, PLUS, UINT64)
    GRB_SEMIRING (MIN, PLUS, FP32)
    GRB_SEMIRING (MIN, PLUS, FP64)

    GRB_SEMIRING (MIN, TIMES, INT8)
    GRB_SEMIRING (MIN, TIMES, INT16)
    GRB_SEMIRING (MIN, TIMES, INT32)
    GRB_SEMIRING (MIN, TIMES, INT64)
    GRB_SEMIRING (MIN, TIMES, UINT8)
    GRB_SEMIRING (MIN, TIMES, UINT16)
    GRB_SEMIRING (MIN, TIMES, UINT32)
    GRB_SEMIRING (MIN, TIMES, UINT64)
    GRB_SEMIRING (MIN, TIMES, FP32)
    GRB_SEMIRING (MIN, TIMES, FP64)

    GRB_SEMIRING (MIN, FIRST, INT8)
    GRB_SEMIRING (MIN, FIRST, INT16)
    GRB_SEMIRING (MIN, FIRST, INT32)
    GRB_SEMIRING (MIN, FIRST, INT64)
    GRB_SEMIRING (MIN, FIRST, UINT8)
    GRB_SEMIRING (MIN, FIRST, UINT16)
    GRB_SEMIRING (MIN, FIRST, UINT32)
    GRB_SEMIRING (MIN, FIRST, UINT64)
    GRB_SEMIRING (MIN, FIRST, FP32)
    GRB_SEMIRING (MIN, FIRST, FP64)

    GRB_SEMIRING (MIN, SECOND, INT8)
    GRB_SEMIRING (MIN, SECOND, INT16)
    GRB_SEMIRING (MIN, SECOND, INT32)
    GRB_SEMIRING (MIN, SECOND, INT64)
    GRB_SEMIRING (MIN, SECOND, UINT8)
    GRB_SEMIRING (MIN, SECOND, UINT16)
    GRB_SEMIRING (MIN, SECOND, UINT32)
    GRB_SEMIRING (MIN, SECOND, UINT64)
    GRB_SEMIRING (MIN, SECOND, FP32)
    GRB_SEMIRING (MIN, SECOND, FP64)

    GRB_SEMIRING (MIN, MAX, INT8)
    GRB_SEMIRING (MIN, MAX, INT16)
    GRB_SEMIRING (MIN, MAX, INT32)
    GRB_SEMIRING (MIN, MAX, INT64)
    GRB_SEMIRING (MIN, MAX, UINT8)
    GRB_SEMIRING (MIN, MAX, UINT16)
    GRB_SEMIRING (MIN, MAX, UINT32)
    GRB_SEMIRING (MIN, MAX, UINT64)
    GRB_SEMIRING (MIN, MAX, FP32)
    GRB_SEMIRING (MIN, MAX, FP64)

    //--------------------------------------------------------------------------
    // 50 semirings with MAX monoids
    //--------------------------------------------------------------------------

    GRB_SEMIRING (MAX, PLUS, INT8)
    GRB_SEMIRING (MAX, PLUS, INT16)
    GRB_SEMIRING (MAX, PLUS, INT32)
    GRB_SEMIRING (MAX, PLUS, INT64)
    GRB_SEMIRING (MAX, PLUS, UINT8)
    GRB_SEMIRING (MAX, PLUS, UINT16)
    GRB_SEMIRING (MAX, PLUS, UINT32)
    GRB_SEMIRING (MAX, PLUS, UINT64)
    GRB_SEMIRING (MAX, PLUS, FP32)
    GRB_SEMIRING (MAX, PLUS, FP64)

    GRB_SEMIRING (MAX, TIMES, INT8)
    GRB_SEMIRING (MAX, TIMES, INT16)
    GRB_SEMIRING (MAX, TIMES, INT32)
    GRB_SEMIRING (MAX, TIMES, INT64)
    GRB_SEMIRING (MAX, TIMES, UINT8)
    GRB_SEMIRING (MAX, TIMES, UINT16)
    GRB_SEMIRING (MAX, TIMES, UINT32)
    GRB_SEMIRING (MAX, TIMES, UINT64)
    GRB_SEMIRING (MAX, TIMES, FP32)
    GRB_SEMIRING (MAX, TIMES, FP64)

    GRB_SEMIRING (MAX, FIRST, INT8)
    GRB_SEMIRING (MAX, FIRST, INT16)
    GRB_SEMIRING (MAX, FIRST, INT32)
    GRB_SEMIRING (MAX, FIRST, INT64)
    GRB_SEMIRING (MAX, FIRST, UINT8)
    GRB_SEMIRING (MAX, FIRST, UINT16)
    GRB_SEMIRING (MAX, FIRST, UINT32)
    GRB_SEMIRING (MAX, FIRST, UINT64)
    GRB_SEMIRING (MAX, FIRST, FP32)
    GRB_SEMIRING (MAX, FIRST, FP64)

    GRB_SEMIRING (MAX, SECOND, INT8)
    GRB_SEMIRING (MAX, SECOND, INT16)
    GRB_SEMIRING (MAX, SECOND, INT32)
    GRB_SEMIRING (MAX, SECOND, INT64)
    GRB_SEMIRING (MAX, SECOND, UINT8)
    GRB_SEMIRING (MAX, SECOND, UINT16)
    GRB_SEMIRING (MAX, SECOND, UINT32)
    GRB_SEMIRING (MAX, SECOND, UINT64)
    GRB_SEMIRING (MAX, SECOND, FP32)
    GRB_SEMIRING (MAX, SECOND, FP64)

    GRB_SEMIRING (MAX, MIN, INT8)
    GRB_SEMIRING (MAX, MIN, INT16)
    GRB_SEMIRING (MAX, MIN, INT32)
    GRB_SEMIRING (MAX, MIN, INT64)
    GRB_SEMIRING (MAX, MIN, UINT8)
    GRB_SEMIRING (MAX, MIN, UINT16)
    GRB_SEMIRING (MAX, MIN, UINT32)
    GRB_SEMIRING (MAX, MIN, UINT64)
    GRB_SEMIRING (MAX, MIN, FP32)
    GRB_SEMIRING (MAX, MIN, FP64)

