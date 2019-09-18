//------------------------------------------------------------------------------
// GB_user.h: definitions for compile-time user-defined objects
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Definitions of built-in types and functions, which can be referenced
// by user-defined objects constructed at compile-time.

#ifndef GB_USER_H
#define GB_USER_H

//------------------------------------------------------
// built-in types
//------------------------------------------------------

#define GB_DEF_GrB_BOOL_type bool
#define GB_DEF_GrB_INT8_type int8_t
#define GB_DEF_GrB_UINT8_type uint8_t
#define GB_DEF_GrB_INT16_type int16_t
#define GB_DEF_GrB_UINT16_type uint16_t
#define GB_DEF_GrB_INT32_type int32_t
#define GB_DEF_GrB_UINT32_type uint32_t
#define GB_DEF_GrB_INT64_type int64_t
#define GB_DEF_GrB_UINT64_type uint64_t
#define GB_DEF_GrB_FP32_type float
#define GB_DEF_GrB_FP64_type double

//------------------------------------------------------
// built-in unary operators
//------------------------------------------------------

// op: IDENTITY
#define GB_DEF_GrB_IDENTITY_BOOL_function GB_IDENTITY_f_BOOL
#define GB_DEF_GrB_IDENTITY_BOOL_ztype bool
#define GB_DEF_GrB_IDENTITY_BOOL_xtype bool

#define GB_DEF_GrB_IDENTITY_INT8_function GB_IDENTITY_f_INT8
#define GB_DEF_GrB_IDENTITY_INT8_ztype int8_t
#define GB_DEF_GrB_IDENTITY_INT8_xtype int8_t

#define GB_DEF_GrB_IDENTITY_UINT8_function GB_IDENTITY_f_UINT8
#define GB_DEF_GrB_IDENTITY_UINT8_ztype uint8_t
#define GB_DEF_GrB_IDENTITY_UINT8_xtype uint8_t

#define GB_DEF_GrB_IDENTITY_INT16_function GB_IDENTITY_f_INT16
#define GB_DEF_GrB_IDENTITY_INT16_ztype int16_t
#define GB_DEF_GrB_IDENTITY_INT16_xtype int16_t

#define GB_DEF_GrB_IDENTITY_UINT16_function GB_IDENTITY_f_UINT16
#define GB_DEF_GrB_IDENTITY_UINT16_ztype uint16_t
#define GB_DEF_GrB_IDENTITY_UINT16_xtype uint16_t

#define GB_DEF_GrB_IDENTITY_INT32_function GB_IDENTITY_f_INT32
#define GB_DEF_GrB_IDENTITY_INT32_ztype int32_t
#define GB_DEF_GrB_IDENTITY_INT32_xtype int32_t

#define GB_DEF_GrB_IDENTITY_UINT32_function GB_IDENTITY_f_UINT32
#define GB_DEF_GrB_IDENTITY_UINT32_ztype uint32_t
#define GB_DEF_GrB_IDENTITY_UINT32_xtype uint32_t

#define GB_DEF_GrB_IDENTITY_INT64_function GB_IDENTITY_f_INT64
#define GB_DEF_GrB_IDENTITY_INT64_ztype int64_t
#define GB_DEF_GrB_IDENTITY_INT64_xtype int64_t

#define GB_DEF_GrB_IDENTITY_UINT64_function GB_IDENTITY_f_UINT64
#define GB_DEF_GrB_IDENTITY_UINT64_ztype uint64_t
#define GB_DEF_GrB_IDENTITY_UINT64_xtype uint64_t

#define GB_DEF_GrB_IDENTITY_FP32_function GB_IDENTITY_f_FP32
#define GB_DEF_GrB_IDENTITY_FP32_ztype float
#define GB_DEF_GrB_IDENTITY_FP32_xtype float

#define GB_DEF_GrB_IDENTITY_FP64_function GB_IDENTITY_f_FP64
#define GB_DEF_GrB_IDENTITY_FP64_ztype double
#define GB_DEF_GrB_IDENTITY_FP64_xtype double

// op: AINV
#define GB_DEF_GrB_AINV_BOOL_function GB_AINV_f_BOOL
#define GB_DEF_GrB_AINV_BOOL_ztype bool
#define GB_DEF_GrB_AINV_BOOL_xtype bool

#define GB_DEF_GrB_AINV_INT8_function GB_AINV_f_INT8
#define GB_DEF_GrB_AINV_INT8_ztype int8_t
#define GB_DEF_GrB_AINV_INT8_xtype int8_t

#define GB_DEF_GrB_AINV_UINT8_function GB_AINV_f_UINT8
#define GB_DEF_GrB_AINV_UINT8_ztype uint8_t
#define GB_DEF_GrB_AINV_UINT8_xtype uint8_t

#define GB_DEF_GrB_AINV_INT16_function GB_AINV_f_INT16
#define GB_DEF_GrB_AINV_INT16_ztype int16_t
#define GB_DEF_GrB_AINV_INT16_xtype int16_t

#define GB_DEF_GrB_AINV_UINT16_function GB_AINV_f_UINT16
#define GB_DEF_GrB_AINV_UINT16_ztype uint16_t
#define GB_DEF_GrB_AINV_UINT16_xtype uint16_t

#define GB_DEF_GrB_AINV_INT32_function GB_AINV_f_INT32
#define GB_DEF_GrB_AINV_INT32_ztype int32_t
#define GB_DEF_GrB_AINV_INT32_xtype int32_t

#define GB_DEF_GrB_AINV_UINT32_function GB_AINV_f_UINT32
#define GB_DEF_GrB_AINV_UINT32_ztype uint32_t
#define GB_DEF_GrB_AINV_UINT32_xtype uint32_t

#define GB_DEF_GrB_AINV_INT64_function GB_AINV_f_INT64
#define GB_DEF_GrB_AINV_INT64_ztype int64_t
#define GB_DEF_GrB_AINV_INT64_xtype int64_t

#define GB_DEF_GrB_AINV_UINT64_function GB_AINV_f_UINT64
#define GB_DEF_GrB_AINV_UINT64_ztype uint64_t
#define GB_DEF_GrB_AINV_UINT64_xtype uint64_t

#define GB_DEF_GrB_AINV_FP32_function GB_AINV_f_FP32
#define GB_DEF_GrB_AINV_FP32_ztype float
#define GB_DEF_GrB_AINV_FP32_xtype float

#define GB_DEF_GrB_AINV_FP64_function GB_AINV_f_FP64
#define GB_DEF_GrB_AINV_FP64_ztype double
#define GB_DEF_GrB_AINV_FP64_xtype double

// op: MINV
#define GB_DEF_GrB_MINV_BOOL_function GB_MINV_f_BOOL
#define GB_DEF_GrB_MINV_BOOL_ztype bool
#define GB_DEF_GrB_MINV_BOOL_xtype bool

#define GB_DEF_GrB_MINV_INT8_function GB_MINV_f_INT8
#define GB_DEF_GrB_MINV_INT8_ztype int8_t
#define GB_DEF_GrB_MINV_INT8_xtype int8_t

#define GB_DEF_GrB_MINV_UINT8_function GB_MINV_f_UINT8
#define GB_DEF_GrB_MINV_UINT8_ztype uint8_t
#define GB_DEF_GrB_MINV_UINT8_xtype uint8_t

#define GB_DEF_GrB_MINV_INT16_function GB_MINV_f_INT16
#define GB_DEF_GrB_MINV_INT16_ztype int16_t
#define GB_DEF_GrB_MINV_INT16_xtype int16_t

#define GB_DEF_GrB_MINV_UINT16_function GB_MINV_f_UINT16
#define GB_DEF_GrB_MINV_UINT16_ztype uint16_t
#define GB_DEF_GrB_MINV_UINT16_xtype uint16_t

#define GB_DEF_GrB_MINV_INT32_function GB_MINV_f_INT32
#define GB_DEF_GrB_MINV_INT32_ztype int32_t
#define GB_DEF_GrB_MINV_INT32_xtype int32_t

#define GB_DEF_GrB_MINV_UINT32_function GB_MINV_f_UINT32
#define GB_DEF_GrB_MINV_UINT32_ztype uint32_t
#define GB_DEF_GrB_MINV_UINT32_xtype uint32_t

#define GB_DEF_GrB_MINV_INT64_function GB_MINV_f_INT64
#define GB_DEF_GrB_MINV_INT64_ztype int64_t
#define GB_DEF_GrB_MINV_INT64_xtype int64_t

#define GB_DEF_GrB_MINV_UINT64_function GB_MINV_f_UINT64
#define GB_DEF_GrB_MINV_UINT64_ztype uint64_t
#define GB_DEF_GrB_MINV_UINT64_xtype uint64_t

#define GB_DEF_GrB_MINV_FP32_function GB_MINV_f_FP32
#define GB_DEF_GrB_MINV_FP32_ztype float
#define GB_DEF_GrB_MINV_FP32_xtype float

#define GB_DEF_GrB_MINV_FP64_function GB_MINV_f_FP64
#define GB_DEF_GrB_MINV_FP64_ztype double
#define GB_DEF_GrB_MINV_FP64_xtype double

// op: LNOT
#define GB_DEF_GrB_LNOT_function GB_LNOT_f_BOOL
#define GB_DEF_GrB_LNOT_ztype bool
#define GB_DEF_GrB_LNOT_xtype bool

#define GB_DEF_GxB_LNOT_BOOL_function GB_LNOT_f_BOOL
#define GB_DEF_GxB_LNOT_BOOL_ztype bool
#define GB_DEF_GxB_LNOT_BOOL_xtype bool

#define GB_DEF_GxB_LNOT_INT8_function GB_LNOT_f_INT8
#define GB_DEF_GxB_LNOT_INT8_ztype int8_t
#define GB_DEF_GxB_LNOT_INT8_xtype int8_t

#define GB_DEF_GxB_LNOT_UINT8_function GB_LNOT_f_UINT8
#define GB_DEF_GxB_LNOT_UINT8_ztype uint8_t
#define GB_DEF_GxB_LNOT_UINT8_xtype uint8_t

#define GB_DEF_GxB_LNOT_INT16_function GB_LNOT_f_INT16
#define GB_DEF_GxB_LNOT_INT16_ztype int16_t
#define GB_DEF_GxB_LNOT_INT16_xtype int16_t

#define GB_DEF_GxB_LNOT_UINT16_function GB_LNOT_f_UINT16
#define GB_DEF_GxB_LNOT_UINT16_ztype uint16_t
#define GB_DEF_GxB_LNOT_UINT16_xtype uint16_t

#define GB_DEF_GxB_LNOT_INT32_function GB_LNOT_f_INT32
#define GB_DEF_GxB_LNOT_INT32_ztype int32_t
#define GB_DEF_GxB_LNOT_INT32_xtype int32_t

#define GB_DEF_GxB_LNOT_UINT32_function GB_LNOT_f_UINT32
#define GB_DEF_GxB_LNOT_UINT32_ztype uint32_t
#define GB_DEF_GxB_LNOT_UINT32_xtype uint32_t

#define GB_DEF_GxB_LNOT_INT64_function GB_LNOT_f_INT64
#define GB_DEF_GxB_LNOT_INT64_ztype int64_t
#define GB_DEF_GxB_LNOT_INT64_xtype int64_t

#define GB_DEF_GxB_LNOT_UINT64_function GB_LNOT_f_UINT64
#define GB_DEF_GxB_LNOT_UINT64_ztype uint64_t
#define GB_DEF_GxB_LNOT_UINT64_xtype uint64_t

#define GB_DEF_GxB_LNOT_FP32_function GB_LNOT_f_FP32
#define GB_DEF_GxB_LNOT_FP32_ztype float
#define GB_DEF_GxB_LNOT_FP32_xtype float

#define GB_DEF_GxB_LNOT_FP64_function GB_LNOT_f_FP64
#define GB_DEF_GxB_LNOT_FP64_ztype double
#define GB_DEF_GxB_LNOT_FP64_xtype double

// op: ONE
#define GB_DEF_GxB_ONE_BOOL_function GB_ONE_f_BOOL
#define GB_DEF_GxB_ONE_BOOL_ztype bool
#define GB_DEF_GxB_ONE_BOOL_xtype bool

#define GB_DEF_GxB_ONE_INT8_function GB_ONE_f_INT8
#define GB_DEF_GxB_ONE_INT8_ztype int8_t
#define GB_DEF_GxB_ONE_INT8_xtype int8_t

#define GB_DEF_GxB_ONE_UINT8_function GB_ONE_f_UINT8
#define GB_DEF_GxB_ONE_UINT8_ztype uint8_t
#define GB_DEF_GxB_ONE_UINT8_xtype uint8_t

#define GB_DEF_GxB_ONE_INT16_function GB_ONE_f_INT16
#define GB_DEF_GxB_ONE_INT16_ztype int16_t
#define GB_DEF_GxB_ONE_INT16_xtype int16_t

#define GB_DEF_GxB_ONE_UINT16_function GB_ONE_f_UINT16
#define GB_DEF_GxB_ONE_UINT16_ztype uint16_t
#define GB_DEF_GxB_ONE_UINT16_xtype uint16_t

#define GB_DEF_GxB_ONE_INT32_function GB_ONE_f_INT32
#define GB_DEF_GxB_ONE_INT32_ztype int32_t
#define GB_DEF_GxB_ONE_INT32_xtype int32_t

#define GB_DEF_GxB_ONE_UINT32_function GB_ONE_f_UINT32
#define GB_DEF_GxB_ONE_UINT32_ztype uint32_t
#define GB_DEF_GxB_ONE_UINT32_xtype uint32_t

#define GB_DEF_GxB_ONE_INT64_function GB_ONE_f_INT64
#define GB_DEF_GxB_ONE_INT64_ztype int64_t
#define GB_DEF_GxB_ONE_INT64_xtype int64_t

#define GB_DEF_GxB_ONE_UINT64_function GB_ONE_f_UINT64
#define GB_DEF_GxB_ONE_UINT64_ztype uint64_t
#define GB_DEF_GxB_ONE_UINT64_xtype uint64_t

#define GB_DEF_GxB_ONE_FP32_function GB_ONE_f_FP32
#define GB_DEF_GxB_ONE_FP32_ztype float
#define GB_DEF_GxB_ONE_FP32_xtype float

#define GB_DEF_GxB_ONE_FP64_function GB_ONE_f_FP64
#define GB_DEF_GxB_ONE_FP64_ztype double
#define GB_DEF_GxB_ONE_FP64_xtype double

// op: ABS
#define GB_DEF_GxB_ABS_BOOL_function GB_ABS_f_BOOL
#define GB_DEF_GxB_ABS_BOOL_ztype bool
#define GB_DEF_GxB_ABS_BOOL_xtype bool

#define GB_DEF_GxB_ABS_INT8_function GB_ABS_f_INT8
#define GB_DEF_GxB_ABS_INT8_ztype int8_t
#define GB_DEF_GxB_ABS_INT8_xtype int8_t

#define GB_DEF_GxB_ABS_UINT8_function GB_ABS_f_UINT8
#define GB_DEF_GxB_ABS_UINT8_ztype uint8_t
#define GB_DEF_GxB_ABS_UINT8_xtype uint8_t

#define GB_DEF_GxB_ABS_INT16_function GB_ABS_f_INT16
#define GB_DEF_GxB_ABS_INT16_ztype int16_t
#define GB_DEF_GxB_ABS_INT16_xtype int16_t

#define GB_DEF_GxB_ABS_UINT16_function GB_ABS_f_UINT16
#define GB_DEF_GxB_ABS_UINT16_ztype uint16_t
#define GB_DEF_GxB_ABS_UINT16_xtype uint16_t

#define GB_DEF_GxB_ABS_INT32_function GB_ABS_f_INT32
#define GB_DEF_GxB_ABS_INT32_ztype int32_t
#define GB_DEF_GxB_ABS_INT32_xtype int32_t

#define GB_DEF_GxB_ABS_UINT32_function GB_ABS_f_UINT32
#define GB_DEF_GxB_ABS_UINT32_ztype uint32_t
#define GB_DEF_GxB_ABS_UINT32_xtype uint32_t

#define GB_DEF_GxB_ABS_INT64_function GB_ABS_f_INT64
#define GB_DEF_GxB_ABS_INT64_ztype int64_t
#define GB_DEF_GxB_ABS_INT64_xtype int64_t

#define GB_DEF_GxB_ABS_UINT64_function GB_ABS_f_UINT64
#define GB_DEF_GxB_ABS_UINT64_ztype uint64_t
#define GB_DEF_GxB_ABS_UINT64_xtype uint64_t

#define GB_DEF_GxB_ABS_FP32_function GB_ABS_f_FP32
#define GB_DEF_GxB_ABS_FP32_ztype float
#define GB_DEF_GxB_ABS_FP32_xtype float

#define GB_DEF_GxB_ABS_FP64_function GB_ABS_f_FP64
#define GB_DEF_GxB_ABS_FP64_ztype double
#define GB_DEF_GxB_ABS_FP64_xtype double

#define GB_DEF_GrB_LNOT_function GB_LNOT_f_BOOL
#define GB_DEF_GrB_LNOT_ztype bool
#define GB_DEF_GrB_LNOT_xtype bool

//------------------------------------------------------
// binary operators of the form z=f(x,y): TxT -> T
//------------------------------------------------------

// op: FIRST
#define GB_DEF_GrB_FIRST_BOOL_function GB_FIRST_f_BOOL
#define GB_DEF_GrB_FIRST_BOOL_ztype bool
#define GB_DEF_GrB_FIRST_BOOL_xtype bool
#define GB_DEF_GrB_FIRST_BOOL_ytype bool

#define GB_DEF_GrB_FIRST_INT8_function GB_FIRST_f_INT8
#define GB_DEF_GrB_FIRST_INT8_ztype int8_t
#define GB_DEF_GrB_FIRST_INT8_xtype int8_t
#define GB_DEF_GrB_FIRST_INT8_ytype int8_t

#define GB_DEF_GrB_FIRST_UINT8_function GB_FIRST_f_UINT8
#define GB_DEF_GrB_FIRST_UINT8_ztype uint8_t
#define GB_DEF_GrB_FIRST_UINT8_xtype uint8_t
#define GB_DEF_GrB_FIRST_UINT8_ytype uint8_t

#define GB_DEF_GrB_FIRST_INT16_function GB_FIRST_f_INT16
#define GB_DEF_GrB_FIRST_INT16_ztype int16_t
#define GB_DEF_GrB_FIRST_INT16_xtype int16_t
#define GB_DEF_GrB_FIRST_INT16_ytype int16_t

#define GB_DEF_GrB_FIRST_UINT16_function GB_FIRST_f_UINT16
#define GB_DEF_GrB_FIRST_UINT16_ztype uint16_t
#define GB_DEF_GrB_FIRST_UINT16_xtype uint16_t
#define GB_DEF_GrB_FIRST_UINT16_ytype uint16_t

#define GB_DEF_GrB_FIRST_INT32_function GB_FIRST_f_INT32
#define GB_DEF_GrB_FIRST_INT32_ztype int32_t
#define GB_DEF_GrB_FIRST_INT32_xtype int32_t
#define GB_DEF_GrB_FIRST_INT32_ytype int32_t

#define GB_DEF_GrB_FIRST_UINT32_function GB_FIRST_f_UINT32
#define GB_DEF_GrB_FIRST_UINT32_ztype uint32_t
#define GB_DEF_GrB_FIRST_UINT32_xtype uint32_t
#define GB_DEF_GrB_FIRST_UINT32_ytype uint32_t

#define GB_DEF_GrB_FIRST_INT64_function GB_FIRST_f_INT64
#define GB_DEF_GrB_FIRST_INT64_ztype int64_t
#define GB_DEF_GrB_FIRST_INT64_xtype int64_t
#define GB_DEF_GrB_FIRST_INT64_ytype int64_t

#define GB_DEF_GrB_FIRST_UINT64_function GB_FIRST_f_UINT64
#define GB_DEF_GrB_FIRST_UINT64_ztype uint64_t
#define GB_DEF_GrB_FIRST_UINT64_xtype uint64_t
#define GB_DEF_GrB_FIRST_UINT64_ytype uint64_t

#define GB_DEF_GrB_FIRST_FP32_function GB_FIRST_f_FP32
#define GB_DEF_GrB_FIRST_FP32_ztype float
#define GB_DEF_GrB_FIRST_FP32_xtype float
#define GB_DEF_GrB_FIRST_FP32_ytype float

#define GB_DEF_GrB_FIRST_FP64_function GB_FIRST_f_FP64
#define GB_DEF_GrB_FIRST_FP64_ztype double
#define GB_DEF_GrB_FIRST_FP64_xtype double
#define GB_DEF_GrB_FIRST_FP64_ytype double

// op: SECOND
#define GB_DEF_GrB_SECOND_BOOL_function GB_SECOND_f_BOOL
#define GB_DEF_GrB_SECOND_BOOL_ztype bool
#define GB_DEF_GrB_SECOND_BOOL_xtype bool
#define GB_DEF_GrB_SECOND_BOOL_ytype bool

#define GB_DEF_GrB_SECOND_INT8_function GB_SECOND_f_INT8
#define GB_DEF_GrB_SECOND_INT8_ztype int8_t
#define GB_DEF_GrB_SECOND_INT8_xtype int8_t
#define GB_DEF_GrB_SECOND_INT8_ytype int8_t

#define GB_DEF_GrB_SECOND_UINT8_function GB_SECOND_f_UINT8
#define GB_DEF_GrB_SECOND_UINT8_ztype uint8_t
#define GB_DEF_GrB_SECOND_UINT8_xtype uint8_t
#define GB_DEF_GrB_SECOND_UINT8_ytype uint8_t

#define GB_DEF_GrB_SECOND_INT16_function GB_SECOND_f_INT16
#define GB_DEF_GrB_SECOND_INT16_ztype int16_t
#define GB_DEF_GrB_SECOND_INT16_xtype int16_t
#define GB_DEF_GrB_SECOND_INT16_ytype int16_t

#define GB_DEF_GrB_SECOND_UINT16_function GB_SECOND_f_UINT16
#define GB_DEF_GrB_SECOND_UINT16_ztype uint16_t
#define GB_DEF_GrB_SECOND_UINT16_xtype uint16_t
#define GB_DEF_GrB_SECOND_UINT16_ytype uint16_t

#define GB_DEF_GrB_SECOND_INT32_function GB_SECOND_f_INT32
#define GB_DEF_GrB_SECOND_INT32_ztype int32_t
#define GB_DEF_GrB_SECOND_INT32_xtype int32_t
#define GB_DEF_GrB_SECOND_INT32_ytype int32_t

#define GB_DEF_GrB_SECOND_UINT32_function GB_SECOND_f_UINT32
#define GB_DEF_GrB_SECOND_UINT32_ztype uint32_t
#define GB_DEF_GrB_SECOND_UINT32_xtype uint32_t
#define GB_DEF_GrB_SECOND_UINT32_ytype uint32_t

#define GB_DEF_GrB_SECOND_INT64_function GB_SECOND_f_INT64
#define GB_DEF_GrB_SECOND_INT64_ztype int64_t
#define GB_DEF_GrB_SECOND_INT64_xtype int64_t
#define GB_DEF_GrB_SECOND_INT64_ytype int64_t

#define GB_DEF_GrB_SECOND_UINT64_function GB_SECOND_f_UINT64
#define GB_DEF_GrB_SECOND_UINT64_ztype uint64_t
#define GB_DEF_GrB_SECOND_UINT64_xtype uint64_t
#define GB_DEF_GrB_SECOND_UINT64_ytype uint64_t

#define GB_DEF_GrB_SECOND_FP32_function GB_SECOND_f_FP32
#define GB_DEF_GrB_SECOND_FP32_ztype float
#define GB_DEF_GrB_SECOND_FP32_xtype float
#define GB_DEF_GrB_SECOND_FP32_ytype float

#define GB_DEF_GrB_SECOND_FP64_function GB_SECOND_f_FP64
#define GB_DEF_GrB_SECOND_FP64_ztype double
#define GB_DEF_GrB_SECOND_FP64_xtype double
#define GB_DEF_GrB_SECOND_FP64_ytype double

// op: MIN
#define GB_DEF_GrB_MIN_BOOL_function GB_MIN_f_BOOL
#define GB_DEF_GrB_MIN_BOOL_ztype bool
#define GB_DEF_GrB_MIN_BOOL_xtype bool
#define GB_DEF_GrB_MIN_BOOL_ytype bool

#define GB_DEF_GrB_MIN_INT8_function GB_MIN_f_INT8
#define GB_DEF_GrB_MIN_INT8_ztype int8_t
#define GB_DEF_GrB_MIN_INT8_xtype int8_t
#define GB_DEF_GrB_MIN_INT8_ytype int8_t

#define GB_DEF_GrB_MIN_UINT8_function GB_MIN_f_UINT8
#define GB_DEF_GrB_MIN_UINT8_ztype uint8_t
#define GB_DEF_GrB_MIN_UINT8_xtype uint8_t
#define GB_DEF_GrB_MIN_UINT8_ytype uint8_t

#define GB_DEF_GrB_MIN_INT16_function GB_MIN_f_INT16
#define GB_DEF_GrB_MIN_INT16_ztype int16_t
#define GB_DEF_GrB_MIN_INT16_xtype int16_t
#define GB_DEF_GrB_MIN_INT16_ytype int16_t

#define GB_DEF_GrB_MIN_UINT16_function GB_MIN_f_UINT16
#define GB_DEF_GrB_MIN_UINT16_ztype uint16_t
#define GB_DEF_GrB_MIN_UINT16_xtype uint16_t
#define GB_DEF_GrB_MIN_UINT16_ytype uint16_t

#define GB_DEF_GrB_MIN_INT32_function GB_MIN_f_INT32
#define GB_DEF_GrB_MIN_INT32_ztype int32_t
#define GB_DEF_GrB_MIN_INT32_xtype int32_t
#define GB_DEF_GrB_MIN_INT32_ytype int32_t

#define GB_DEF_GrB_MIN_UINT32_function GB_MIN_f_UINT32
#define GB_DEF_GrB_MIN_UINT32_ztype uint32_t
#define GB_DEF_GrB_MIN_UINT32_xtype uint32_t
#define GB_DEF_GrB_MIN_UINT32_ytype uint32_t

#define GB_DEF_GrB_MIN_INT64_function GB_MIN_f_INT64
#define GB_DEF_GrB_MIN_INT64_ztype int64_t
#define GB_DEF_GrB_MIN_INT64_xtype int64_t
#define GB_DEF_GrB_MIN_INT64_ytype int64_t

#define GB_DEF_GrB_MIN_UINT64_function GB_MIN_f_UINT64
#define GB_DEF_GrB_MIN_UINT64_ztype uint64_t
#define GB_DEF_GrB_MIN_UINT64_xtype uint64_t
#define GB_DEF_GrB_MIN_UINT64_ytype uint64_t

#define GB_DEF_GrB_MIN_FP32_function GB_MIN_f_FP32
#define GB_DEF_GrB_MIN_FP32_ztype float
#define GB_DEF_GrB_MIN_FP32_xtype float
#define GB_DEF_GrB_MIN_FP32_ytype float

#define GB_DEF_GrB_MIN_FP64_function GB_MIN_f_FP64
#define GB_DEF_GrB_MIN_FP64_ztype double
#define GB_DEF_GrB_MIN_FP64_xtype double
#define GB_DEF_GrB_MIN_FP64_ytype double

// op: MAX
#define GB_DEF_GrB_MAX_BOOL_function GB_MAX_f_BOOL
#define GB_DEF_GrB_MAX_BOOL_ztype bool
#define GB_DEF_GrB_MAX_BOOL_xtype bool
#define GB_DEF_GrB_MAX_BOOL_ytype bool

#define GB_DEF_GrB_MAX_INT8_function GB_MAX_f_INT8
#define GB_DEF_GrB_MAX_INT8_ztype int8_t
#define GB_DEF_GrB_MAX_INT8_xtype int8_t
#define GB_DEF_GrB_MAX_INT8_ytype int8_t

#define GB_DEF_GrB_MAX_UINT8_function GB_MAX_f_UINT8
#define GB_DEF_GrB_MAX_UINT8_ztype uint8_t
#define GB_DEF_GrB_MAX_UINT8_xtype uint8_t
#define GB_DEF_GrB_MAX_UINT8_ytype uint8_t

#define GB_DEF_GrB_MAX_INT16_function GB_MAX_f_INT16
#define GB_DEF_GrB_MAX_INT16_ztype int16_t
#define GB_DEF_GrB_MAX_INT16_xtype int16_t
#define GB_DEF_GrB_MAX_INT16_ytype int16_t

#define GB_DEF_GrB_MAX_UINT16_function GB_MAX_f_UINT16
#define GB_DEF_GrB_MAX_UINT16_ztype uint16_t
#define GB_DEF_GrB_MAX_UINT16_xtype uint16_t
#define GB_DEF_GrB_MAX_UINT16_ytype uint16_t

#define GB_DEF_GrB_MAX_INT32_function GB_MAX_f_INT32
#define GB_DEF_GrB_MAX_INT32_ztype int32_t
#define GB_DEF_GrB_MAX_INT32_xtype int32_t
#define GB_DEF_GrB_MAX_INT32_ytype int32_t

#define GB_DEF_GrB_MAX_UINT32_function GB_MAX_f_UINT32
#define GB_DEF_GrB_MAX_UINT32_ztype uint32_t
#define GB_DEF_GrB_MAX_UINT32_xtype uint32_t
#define GB_DEF_GrB_MAX_UINT32_ytype uint32_t

#define GB_DEF_GrB_MAX_INT64_function GB_MAX_f_INT64
#define GB_DEF_GrB_MAX_INT64_ztype int64_t
#define GB_DEF_GrB_MAX_INT64_xtype int64_t
#define GB_DEF_GrB_MAX_INT64_ytype int64_t

#define GB_DEF_GrB_MAX_UINT64_function GB_MAX_f_UINT64
#define GB_DEF_GrB_MAX_UINT64_ztype uint64_t
#define GB_DEF_GrB_MAX_UINT64_xtype uint64_t
#define GB_DEF_GrB_MAX_UINT64_ytype uint64_t

#define GB_DEF_GrB_MAX_FP32_function GB_MAX_f_FP32
#define GB_DEF_GrB_MAX_FP32_ztype float
#define GB_DEF_GrB_MAX_FP32_xtype float
#define GB_DEF_GrB_MAX_FP32_ytype float

#define GB_DEF_GrB_MAX_FP64_function GB_MAX_f_FP64
#define GB_DEF_GrB_MAX_FP64_ztype double
#define GB_DEF_GrB_MAX_FP64_xtype double
#define GB_DEF_GrB_MAX_FP64_ytype double

// op: PLUS
#define GB_DEF_GrB_PLUS_BOOL_function GB_PLUS_f_BOOL
#define GB_DEF_GrB_PLUS_BOOL_ztype bool
#define GB_DEF_GrB_PLUS_BOOL_xtype bool
#define GB_DEF_GrB_PLUS_BOOL_ytype bool

#define GB_DEF_GrB_PLUS_INT8_function GB_PLUS_f_INT8
#define GB_DEF_GrB_PLUS_INT8_ztype int8_t
#define GB_DEF_GrB_PLUS_INT8_xtype int8_t
#define GB_DEF_GrB_PLUS_INT8_ytype int8_t

#define GB_DEF_GrB_PLUS_UINT8_function GB_PLUS_f_UINT8
#define GB_DEF_GrB_PLUS_UINT8_ztype uint8_t
#define GB_DEF_GrB_PLUS_UINT8_xtype uint8_t
#define GB_DEF_GrB_PLUS_UINT8_ytype uint8_t

#define GB_DEF_GrB_PLUS_INT16_function GB_PLUS_f_INT16
#define GB_DEF_GrB_PLUS_INT16_ztype int16_t
#define GB_DEF_GrB_PLUS_INT16_xtype int16_t
#define GB_DEF_GrB_PLUS_INT16_ytype int16_t

#define GB_DEF_GrB_PLUS_UINT16_function GB_PLUS_f_UINT16
#define GB_DEF_GrB_PLUS_UINT16_ztype uint16_t
#define GB_DEF_GrB_PLUS_UINT16_xtype uint16_t
#define GB_DEF_GrB_PLUS_UINT16_ytype uint16_t

#define GB_DEF_GrB_PLUS_INT32_function GB_PLUS_f_INT32
#define GB_DEF_GrB_PLUS_INT32_ztype int32_t
#define GB_DEF_GrB_PLUS_INT32_xtype int32_t
#define GB_DEF_GrB_PLUS_INT32_ytype int32_t

#define GB_DEF_GrB_PLUS_UINT32_function GB_PLUS_f_UINT32
#define GB_DEF_GrB_PLUS_UINT32_ztype uint32_t
#define GB_DEF_GrB_PLUS_UINT32_xtype uint32_t
#define GB_DEF_GrB_PLUS_UINT32_ytype uint32_t

#define GB_DEF_GrB_PLUS_INT64_function GB_PLUS_f_INT64
#define GB_DEF_GrB_PLUS_INT64_ztype int64_t
#define GB_DEF_GrB_PLUS_INT64_xtype int64_t
#define GB_DEF_GrB_PLUS_INT64_ytype int64_t

#define GB_DEF_GrB_PLUS_UINT64_function GB_PLUS_f_UINT64
#define GB_DEF_GrB_PLUS_UINT64_ztype uint64_t
#define GB_DEF_GrB_PLUS_UINT64_xtype uint64_t
#define GB_DEF_GrB_PLUS_UINT64_ytype uint64_t

#define GB_DEF_GrB_PLUS_FP32_function GB_PLUS_f_FP32
#define GB_DEF_GrB_PLUS_FP32_ztype float
#define GB_DEF_GrB_PLUS_FP32_xtype float
#define GB_DEF_GrB_PLUS_FP32_ytype float

#define GB_DEF_GrB_PLUS_FP64_function GB_PLUS_f_FP64
#define GB_DEF_GrB_PLUS_FP64_ztype double
#define GB_DEF_GrB_PLUS_FP64_xtype double
#define GB_DEF_GrB_PLUS_FP64_ytype double

// op: MINUS
#define GB_DEF_GrB_MINUS_BOOL_function GB_MINUS_f_BOOL
#define GB_DEF_GrB_MINUS_BOOL_ztype bool
#define GB_DEF_GrB_MINUS_BOOL_xtype bool
#define GB_DEF_GrB_MINUS_BOOL_ytype bool

#define GB_DEF_GrB_MINUS_INT8_function GB_MINUS_f_INT8
#define GB_DEF_GrB_MINUS_INT8_ztype int8_t
#define GB_DEF_GrB_MINUS_INT8_xtype int8_t
#define GB_DEF_GrB_MINUS_INT8_ytype int8_t

#define GB_DEF_GrB_MINUS_UINT8_function GB_MINUS_f_UINT8
#define GB_DEF_GrB_MINUS_UINT8_ztype uint8_t
#define GB_DEF_GrB_MINUS_UINT8_xtype uint8_t
#define GB_DEF_GrB_MINUS_UINT8_ytype uint8_t

#define GB_DEF_GrB_MINUS_INT16_function GB_MINUS_f_INT16
#define GB_DEF_GrB_MINUS_INT16_ztype int16_t
#define GB_DEF_GrB_MINUS_INT16_xtype int16_t
#define GB_DEF_GrB_MINUS_INT16_ytype int16_t

#define GB_DEF_GrB_MINUS_UINT16_function GB_MINUS_f_UINT16
#define GB_DEF_GrB_MINUS_UINT16_ztype uint16_t
#define GB_DEF_GrB_MINUS_UINT16_xtype uint16_t
#define GB_DEF_GrB_MINUS_UINT16_ytype uint16_t

#define GB_DEF_GrB_MINUS_INT32_function GB_MINUS_f_INT32
#define GB_DEF_GrB_MINUS_INT32_ztype int32_t
#define GB_DEF_GrB_MINUS_INT32_xtype int32_t
#define GB_DEF_GrB_MINUS_INT32_ytype int32_t

#define GB_DEF_GrB_MINUS_UINT32_function GB_MINUS_f_UINT32
#define GB_DEF_GrB_MINUS_UINT32_ztype uint32_t
#define GB_DEF_GrB_MINUS_UINT32_xtype uint32_t
#define GB_DEF_GrB_MINUS_UINT32_ytype uint32_t

#define GB_DEF_GrB_MINUS_INT64_function GB_MINUS_f_INT64
#define GB_DEF_GrB_MINUS_INT64_ztype int64_t
#define GB_DEF_GrB_MINUS_INT64_xtype int64_t
#define GB_DEF_GrB_MINUS_INT64_ytype int64_t

#define GB_DEF_GrB_MINUS_UINT64_function GB_MINUS_f_UINT64
#define GB_DEF_GrB_MINUS_UINT64_ztype uint64_t
#define GB_DEF_GrB_MINUS_UINT64_xtype uint64_t
#define GB_DEF_GrB_MINUS_UINT64_ytype uint64_t

#define GB_DEF_GrB_MINUS_FP32_function GB_MINUS_f_FP32
#define GB_DEF_GrB_MINUS_FP32_ztype float
#define GB_DEF_GrB_MINUS_FP32_xtype float
#define GB_DEF_GrB_MINUS_FP32_ytype float

#define GB_DEF_GrB_MINUS_FP64_function GB_MINUS_f_FP64
#define GB_DEF_GrB_MINUS_FP64_ztype double
#define GB_DEF_GrB_MINUS_FP64_xtype double
#define GB_DEF_GrB_MINUS_FP64_ytype double

// op: RMINUS
#define GB_DEF_GxB_RMINUS_BOOL_function GB_RMINUS_f_BOOL
#define GB_DEF_GxB_RMINUS_BOOL_ztype bool
#define GB_DEF_GxB_RMINUS_BOOL_xtype bool
#define GB_DEF_GxB_RMINUS_BOOL_ytype bool

#define GB_DEF_GxB_RMINUS_INT8_function GB_RMINUS_f_INT8
#define GB_DEF_GxB_RMINUS_INT8_ztype int8_t
#define GB_DEF_GxB_RMINUS_INT8_xtype int8_t
#define GB_DEF_GxB_RMINUS_INT8_ytype int8_t

#define GB_DEF_GxB_RMINUS_UINT8_function GB_RMINUS_f_UINT8
#define GB_DEF_GxB_RMINUS_UINT8_ztype uint8_t
#define GB_DEF_GxB_RMINUS_UINT8_xtype uint8_t
#define GB_DEF_GxB_RMINUS_UINT8_ytype uint8_t

#define GB_DEF_GxB_RMINUS_INT16_function GB_RMINUS_f_INT16
#define GB_DEF_GxB_RMINUS_INT16_ztype int16_t
#define GB_DEF_GxB_RMINUS_INT16_xtype int16_t
#define GB_DEF_GxB_RMINUS_INT16_ytype int16_t

#define GB_DEF_GxB_RMINUS_UINT16_function GB_RMINUS_f_UINT16
#define GB_DEF_GxB_RMINUS_UINT16_ztype uint16_t
#define GB_DEF_GxB_RMINUS_UINT16_xtype uint16_t
#define GB_DEF_GxB_RMINUS_UINT16_ytype uint16_t

#define GB_DEF_GxB_RMINUS_INT32_function GB_RMINUS_f_INT32
#define GB_DEF_GxB_RMINUS_INT32_ztype int32_t
#define GB_DEF_GxB_RMINUS_INT32_xtype int32_t
#define GB_DEF_GxB_RMINUS_INT32_ytype int32_t

#define GB_DEF_GxB_RMINUS_UINT32_function GB_RMINUS_f_UINT32
#define GB_DEF_GxB_RMINUS_UINT32_ztype uint32_t
#define GB_DEF_GxB_RMINUS_UINT32_xtype uint32_t
#define GB_DEF_GxB_RMINUS_UINT32_ytype uint32_t

#define GB_DEF_GxB_RMINUS_INT64_function GB_RMINUS_f_INT64
#define GB_DEF_GxB_RMINUS_INT64_ztype int64_t
#define GB_DEF_GxB_RMINUS_INT64_xtype int64_t
#define GB_DEF_GxB_RMINUS_INT64_ytype int64_t

#define GB_DEF_GxB_RMINUS_UINT64_function GB_RMINUS_f_UINT64
#define GB_DEF_GxB_RMINUS_UINT64_ztype uint64_t
#define GB_DEF_GxB_RMINUS_UINT64_xtype uint64_t
#define GB_DEF_GxB_RMINUS_UINT64_ytype uint64_t

#define GB_DEF_GxB_RMINUS_FP32_function GB_RMINUS_f_FP32
#define GB_DEF_GxB_RMINUS_FP32_ztype float
#define GB_DEF_GxB_RMINUS_FP32_xtype float
#define GB_DEF_GxB_RMINUS_FP32_ytype float

#define GB_DEF_GxB_RMINUS_FP64_function GB_RMINUS_f_FP64
#define GB_DEF_GxB_RMINUS_FP64_ztype double
#define GB_DEF_GxB_RMINUS_FP64_xtype double
#define GB_DEF_GxB_RMINUS_FP64_ytype double

// op: TIMES
#define GB_DEF_GrB_TIMES_BOOL_function GB_TIMES_f_BOOL
#define GB_DEF_GrB_TIMES_BOOL_ztype bool
#define GB_DEF_GrB_TIMES_BOOL_xtype bool
#define GB_DEF_GrB_TIMES_BOOL_ytype bool

#define GB_DEF_GrB_TIMES_INT8_function GB_TIMES_f_INT8
#define GB_DEF_GrB_TIMES_INT8_ztype int8_t
#define GB_DEF_GrB_TIMES_INT8_xtype int8_t
#define GB_DEF_GrB_TIMES_INT8_ytype int8_t

#define GB_DEF_GrB_TIMES_UINT8_function GB_TIMES_f_UINT8
#define GB_DEF_GrB_TIMES_UINT8_ztype uint8_t
#define GB_DEF_GrB_TIMES_UINT8_xtype uint8_t
#define GB_DEF_GrB_TIMES_UINT8_ytype uint8_t

#define GB_DEF_GrB_TIMES_INT16_function GB_TIMES_f_INT16
#define GB_DEF_GrB_TIMES_INT16_ztype int16_t
#define GB_DEF_GrB_TIMES_INT16_xtype int16_t
#define GB_DEF_GrB_TIMES_INT16_ytype int16_t

#define GB_DEF_GrB_TIMES_UINT16_function GB_TIMES_f_UINT16
#define GB_DEF_GrB_TIMES_UINT16_ztype uint16_t
#define GB_DEF_GrB_TIMES_UINT16_xtype uint16_t
#define GB_DEF_GrB_TIMES_UINT16_ytype uint16_t

#define GB_DEF_GrB_TIMES_INT32_function GB_TIMES_f_INT32
#define GB_DEF_GrB_TIMES_INT32_ztype int32_t
#define GB_DEF_GrB_TIMES_INT32_xtype int32_t
#define GB_DEF_GrB_TIMES_INT32_ytype int32_t

#define GB_DEF_GrB_TIMES_UINT32_function GB_TIMES_f_UINT32
#define GB_DEF_GrB_TIMES_UINT32_ztype uint32_t
#define GB_DEF_GrB_TIMES_UINT32_xtype uint32_t
#define GB_DEF_GrB_TIMES_UINT32_ytype uint32_t

#define GB_DEF_GrB_TIMES_INT64_function GB_TIMES_f_INT64
#define GB_DEF_GrB_TIMES_INT64_ztype int64_t
#define GB_DEF_GrB_TIMES_INT64_xtype int64_t
#define GB_DEF_GrB_TIMES_INT64_ytype int64_t

#define GB_DEF_GrB_TIMES_UINT64_function GB_TIMES_f_UINT64
#define GB_DEF_GrB_TIMES_UINT64_ztype uint64_t
#define GB_DEF_GrB_TIMES_UINT64_xtype uint64_t
#define GB_DEF_GrB_TIMES_UINT64_ytype uint64_t

#define GB_DEF_GrB_TIMES_FP32_function GB_TIMES_f_FP32
#define GB_DEF_GrB_TIMES_FP32_ztype float
#define GB_DEF_GrB_TIMES_FP32_xtype float
#define GB_DEF_GrB_TIMES_FP32_ytype float

#define GB_DEF_GrB_TIMES_FP64_function GB_TIMES_f_FP64
#define GB_DEF_GrB_TIMES_FP64_ztype double
#define GB_DEF_GrB_TIMES_FP64_xtype double
#define GB_DEF_GrB_TIMES_FP64_ytype double

// op: DIV
#define GB_DEF_GrB_DIV_BOOL_function GB_DIV_f_BOOL
#define GB_DEF_GrB_DIV_BOOL_ztype bool
#define GB_DEF_GrB_DIV_BOOL_xtype bool
#define GB_DEF_GrB_DIV_BOOL_ytype bool

#define GB_DEF_GrB_DIV_INT8_function GB_DIV_f_INT8
#define GB_DEF_GrB_DIV_INT8_ztype int8_t
#define GB_DEF_GrB_DIV_INT8_xtype int8_t
#define GB_DEF_GrB_DIV_INT8_ytype int8_t

#define GB_DEF_GrB_DIV_UINT8_function GB_DIV_f_UINT8
#define GB_DEF_GrB_DIV_UINT8_ztype uint8_t
#define GB_DEF_GrB_DIV_UINT8_xtype uint8_t
#define GB_DEF_GrB_DIV_UINT8_ytype uint8_t

#define GB_DEF_GrB_DIV_INT16_function GB_DIV_f_INT16
#define GB_DEF_GrB_DIV_INT16_ztype int16_t
#define GB_DEF_GrB_DIV_INT16_xtype int16_t
#define GB_DEF_GrB_DIV_INT16_ytype int16_t

#define GB_DEF_GrB_DIV_UINT16_function GB_DIV_f_UINT16
#define GB_DEF_GrB_DIV_UINT16_ztype uint16_t
#define GB_DEF_GrB_DIV_UINT16_xtype uint16_t
#define GB_DEF_GrB_DIV_UINT16_ytype uint16_t

#define GB_DEF_GrB_DIV_INT32_function GB_DIV_f_INT32
#define GB_DEF_GrB_DIV_INT32_ztype int32_t
#define GB_DEF_GrB_DIV_INT32_xtype int32_t
#define GB_DEF_GrB_DIV_INT32_ytype int32_t

#define GB_DEF_GrB_DIV_UINT32_function GB_DIV_f_UINT32
#define GB_DEF_GrB_DIV_UINT32_ztype uint32_t
#define GB_DEF_GrB_DIV_UINT32_xtype uint32_t
#define GB_DEF_GrB_DIV_UINT32_ytype uint32_t

#define GB_DEF_GrB_DIV_INT64_function GB_DIV_f_INT64
#define GB_DEF_GrB_DIV_INT64_ztype int64_t
#define GB_DEF_GrB_DIV_INT64_xtype int64_t
#define GB_DEF_GrB_DIV_INT64_ytype int64_t

#define GB_DEF_GrB_DIV_UINT64_function GB_DIV_f_UINT64
#define GB_DEF_GrB_DIV_UINT64_ztype uint64_t
#define GB_DEF_GrB_DIV_UINT64_xtype uint64_t
#define GB_DEF_GrB_DIV_UINT64_ytype uint64_t

#define GB_DEF_GrB_DIV_FP32_function GB_DIV_f_FP32
#define GB_DEF_GrB_DIV_FP32_ztype float
#define GB_DEF_GrB_DIV_FP32_xtype float
#define GB_DEF_GrB_DIV_FP32_ytype float

#define GB_DEF_GrB_DIV_FP64_function GB_DIV_f_FP64
#define GB_DEF_GrB_DIV_FP64_ztype double
#define GB_DEF_GrB_DIV_FP64_xtype double
#define GB_DEF_GrB_DIV_FP64_ytype double

// op: RDIV
#define GB_DEF_GxB_RDIV_BOOL_function GB_RDIV_f_BOOL
#define GB_DEF_GxB_RDIV_BOOL_ztype bool
#define GB_DEF_GxB_RDIV_BOOL_xtype bool
#define GB_DEF_GxB_RDIV_BOOL_ytype bool

#define GB_DEF_GxB_RDIV_INT8_function GB_RDIV_f_INT8
#define GB_DEF_GxB_RDIV_INT8_ztype int8_t
#define GB_DEF_GxB_RDIV_INT8_xtype int8_t
#define GB_DEF_GxB_RDIV_INT8_ytype int8_t

#define GB_DEF_GxB_RDIV_UINT8_function GB_RDIV_f_UINT8
#define GB_DEF_GxB_RDIV_UINT8_ztype uint8_t
#define GB_DEF_GxB_RDIV_UINT8_xtype uint8_t
#define GB_DEF_GxB_RDIV_UINT8_ytype uint8_t

#define GB_DEF_GxB_RDIV_INT16_function GB_RDIV_f_INT16
#define GB_DEF_GxB_RDIV_INT16_ztype int16_t
#define GB_DEF_GxB_RDIV_INT16_xtype int16_t
#define GB_DEF_GxB_RDIV_INT16_ytype int16_t

#define GB_DEF_GxB_RDIV_UINT16_function GB_RDIV_f_UINT16
#define GB_DEF_GxB_RDIV_UINT16_ztype uint16_t
#define GB_DEF_GxB_RDIV_UINT16_xtype uint16_t
#define GB_DEF_GxB_RDIV_UINT16_ytype uint16_t

#define GB_DEF_GxB_RDIV_INT32_function GB_RDIV_f_INT32
#define GB_DEF_GxB_RDIV_INT32_ztype int32_t
#define GB_DEF_GxB_RDIV_INT32_xtype int32_t
#define GB_DEF_GxB_RDIV_INT32_ytype int32_t

#define GB_DEF_GxB_RDIV_UINT32_function GB_RDIV_f_UINT32
#define GB_DEF_GxB_RDIV_UINT32_ztype uint32_t
#define GB_DEF_GxB_RDIV_UINT32_xtype uint32_t
#define GB_DEF_GxB_RDIV_UINT32_ytype uint32_t

#define GB_DEF_GxB_RDIV_INT64_function GB_RDIV_f_INT64
#define GB_DEF_GxB_RDIV_INT64_ztype int64_t
#define GB_DEF_GxB_RDIV_INT64_xtype int64_t
#define GB_DEF_GxB_RDIV_INT64_ytype int64_t

#define GB_DEF_GxB_RDIV_UINT64_function GB_RDIV_f_UINT64
#define GB_DEF_GxB_RDIV_UINT64_ztype uint64_t
#define GB_DEF_GxB_RDIV_UINT64_xtype uint64_t
#define GB_DEF_GxB_RDIV_UINT64_ytype uint64_t

#define GB_DEF_GxB_RDIV_FP32_function GB_RDIV_f_FP32
#define GB_DEF_GxB_RDIV_FP32_ztype float
#define GB_DEF_GxB_RDIV_FP32_xtype float
#define GB_DEF_GxB_RDIV_FP32_ytype float

#define GB_DEF_GxB_RDIV_FP64_function GB_RDIV_f_FP64
#define GB_DEF_GxB_RDIV_FP64_ztype double
#define GB_DEF_GxB_RDIV_FP64_xtype double
#define GB_DEF_GxB_RDIV_FP64_ytype double

// op: ISEQ
#define GB_DEF_GxB_ISEQ_BOOL_function GB_ISEQ_f_BOOL
#define GB_DEF_GxB_ISEQ_BOOL_ztype bool
#define GB_DEF_GxB_ISEQ_BOOL_xtype bool
#define GB_DEF_GxB_ISEQ_BOOL_ytype bool

#define GB_DEF_GxB_ISEQ_INT8_function GB_ISEQ_f_INT8
#define GB_DEF_GxB_ISEQ_INT8_ztype int8_t
#define GB_DEF_GxB_ISEQ_INT8_xtype int8_t
#define GB_DEF_GxB_ISEQ_INT8_ytype int8_t

#define GB_DEF_GxB_ISEQ_UINT8_function GB_ISEQ_f_UINT8
#define GB_DEF_GxB_ISEQ_UINT8_ztype uint8_t
#define GB_DEF_GxB_ISEQ_UINT8_xtype uint8_t
#define GB_DEF_GxB_ISEQ_UINT8_ytype uint8_t

#define GB_DEF_GxB_ISEQ_INT16_function GB_ISEQ_f_INT16
#define GB_DEF_GxB_ISEQ_INT16_ztype int16_t
#define GB_DEF_GxB_ISEQ_INT16_xtype int16_t
#define GB_DEF_GxB_ISEQ_INT16_ytype int16_t

#define GB_DEF_GxB_ISEQ_UINT16_function GB_ISEQ_f_UINT16
#define GB_DEF_GxB_ISEQ_UINT16_ztype uint16_t
#define GB_DEF_GxB_ISEQ_UINT16_xtype uint16_t
#define GB_DEF_GxB_ISEQ_UINT16_ytype uint16_t

#define GB_DEF_GxB_ISEQ_INT32_function GB_ISEQ_f_INT32
#define GB_DEF_GxB_ISEQ_INT32_ztype int32_t
#define GB_DEF_GxB_ISEQ_INT32_xtype int32_t
#define GB_DEF_GxB_ISEQ_INT32_ytype int32_t

#define GB_DEF_GxB_ISEQ_UINT32_function GB_ISEQ_f_UINT32
#define GB_DEF_GxB_ISEQ_UINT32_ztype uint32_t
#define GB_DEF_GxB_ISEQ_UINT32_xtype uint32_t
#define GB_DEF_GxB_ISEQ_UINT32_ytype uint32_t

#define GB_DEF_GxB_ISEQ_INT64_function GB_ISEQ_f_INT64
#define GB_DEF_GxB_ISEQ_INT64_ztype int64_t
#define GB_DEF_GxB_ISEQ_INT64_xtype int64_t
#define GB_DEF_GxB_ISEQ_INT64_ytype int64_t

#define GB_DEF_GxB_ISEQ_UINT64_function GB_ISEQ_f_UINT64
#define GB_DEF_GxB_ISEQ_UINT64_ztype uint64_t
#define GB_DEF_GxB_ISEQ_UINT64_xtype uint64_t
#define GB_DEF_GxB_ISEQ_UINT64_ytype uint64_t

#define GB_DEF_GxB_ISEQ_FP32_function GB_ISEQ_f_FP32
#define GB_DEF_GxB_ISEQ_FP32_ztype float
#define GB_DEF_GxB_ISEQ_FP32_xtype float
#define GB_DEF_GxB_ISEQ_FP32_ytype float

#define GB_DEF_GxB_ISEQ_FP64_function GB_ISEQ_f_FP64
#define GB_DEF_GxB_ISEQ_FP64_ztype double
#define GB_DEF_GxB_ISEQ_FP64_xtype double
#define GB_DEF_GxB_ISEQ_FP64_ytype double

// op: ISNE
#define GB_DEF_GxB_ISNE_BOOL_function GB_ISNE_f_BOOL
#define GB_DEF_GxB_ISNE_BOOL_ztype bool
#define GB_DEF_GxB_ISNE_BOOL_xtype bool
#define GB_DEF_GxB_ISNE_BOOL_ytype bool

#define GB_DEF_GxB_ISNE_INT8_function GB_ISNE_f_INT8
#define GB_DEF_GxB_ISNE_INT8_ztype int8_t
#define GB_DEF_GxB_ISNE_INT8_xtype int8_t
#define GB_DEF_GxB_ISNE_INT8_ytype int8_t

#define GB_DEF_GxB_ISNE_UINT8_function GB_ISNE_f_UINT8
#define GB_DEF_GxB_ISNE_UINT8_ztype uint8_t
#define GB_DEF_GxB_ISNE_UINT8_xtype uint8_t
#define GB_DEF_GxB_ISNE_UINT8_ytype uint8_t

#define GB_DEF_GxB_ISNE_INT16_function GB_ISNE_f_INT16
#define GB_DEF_GxB_ISNE_INT16_ztype int16_t
#define GB_DEF_GxB_ISNE_INT16_xtype int16_t
#define GB_DEF_GxB_ISNE_INT16_ytype int16_t

#define GB_DEF_GxB_ISNE_UINT16_function GB_ISNE_f_UINT16
#define GB_DEF_GxB_ISNE_UINT16_ztype uint16_t
#define GB_DEF_GxB_ISNE_UINT16_xtype uint16_t
#define GB_DEF_GxB_ISNE_UINT16_ytype uint16_t

#define GB_DEF_GxB_ISNE_INT32_function GB_ISNE_f_INT32
#define GB_DEF_GxB_ISNE_INT32_ztype int32_t
#define GB_DEF_GxB_ISNE_INT32_xtype int32_t
#define GB_DEF_GxB_ISNE_INT32_ytype int32_t

#define GB_DEF_GxB_ISNE_UINT32_function GB_ISNE_f_UINT32
#define GB_DEF_GxB_ISNE_UINT32_ztype uint32_t
#define GB_DEF_GxB_ISNE_UINT32_xtype uint32_t
#define GB_DEF_GxB_ISNE_UINT32_ytype uint32_t

#define GB_DEF_GxB_ISNE_INT64_function GB_ISNE_f_INT64
#define GB_DEF_GxB_ISNE_INT64_ztype int64_t
#define GB_DEF_GxB_ISNE_INT64_xtype int64_t
#define GB_DEF_GxB_ISNE_INT64_ytype int64_t

#define GB_DEF_GxB_ISNE_UINT64_function GB_ISNE_f_UINT64
#define GB_DEF_GxB_ISNE_UINT64_ztype uint64_t
#define GB_DEF_GxB_ISNE_UINT64_xtype uint64_t
#define GB_DEF_GxB_ISNE_UINT64_ytype uint64_t

#define GB_DEF_GxB_ISNE_FP32_function GB_ISNE_f_FP32
#define GB_DEF_GxB_ISNE_FP32_ztype float
#define GB_DEF_GxB_ISNE_FP32_xtype float
#define GB_DEF_GxB_ISNE_FP32_ytype float

#define GB_DEF_GxB_ISNE_FP64_function GB_ISNE_f_FP64
#define GB_DEF_GxB_ISNE_FP64_ztype double
#define GB_DEF_GxB_ISNE_FP64_xtype double
#define GB_DEF_GxB_ISNE_FP64_ytype double

// op: ISGT
#define GB_DEF_GxB_ISGT_BOOL_function GB_ISGT_f_BOOL
#define GB_DEF_GxB_ISGT_BOOL_ztype bool
#define GB_DEF_GxB_ISGT_BOOL_xtype bool
#define GB_DEF_GxB_ISGT_BOOL_ytype bool

#define GB_DEF_GxB_ISGT_INT8_function GB_ISGT_f_INT8
#define GB_DEF_GxB_ISGT_INT8_ztype int8_t
#define GB_DEF_GxB_ISGT_INT8_xtype int8_t
#define GB_DEF_GxB_ISGT_INT8_ytype int8_t

#define GB_DEF_GxB_ISGT_UINT8_function GB_ISGT_f_UINT8
#define GB_DEF_GxB_ISGT_UINT8_ztype uint8_t
#define GB_DEF_GxB_ISGT_UINT8_xtype uint8_t
#define GB_DEF_GxB_ISGT_UINT8_ytype uint8_t

#define GB_DEF_GxB_ISGT_INT16_function GB_ISGT_f_INT16
#define GB_DEF_GxB_ISGT_INT16_ztype int16_t
#define GB_DEF_GxB_ISGT_INT16_xtype int16_t
#define GB_DEF_GxB_ISGT_INT16_ytype int16_t

#define GB_DEF_GxB_ISGT_UINT16_function GB_ISGT_f_UINT16
#define GB_DEF_GxB_ISGT_UINT16_ztype uint16_t
#define GB_DEF_GxB_ISGT_UINT16_xtype uint16_t
#define GB_DEF_GxB_ISGT_UINT16_ytype uint16_t

#define GB_DEF_GxB_ISGT_INT32_function GB_ISGT_f_INT32
#define GB_DEF_GxB_ISGT_INT32_ztype int32_t
#define GB_DEF_GxB_ISGT_INT32_xtype int32_t
#define GB_DEF_GxB_ISGT_INT32_ytype int32_t

#define GB_DEF_GxB_ISGT_UINT32_function GB_ISGT_f_UINT32
#define GB_DEF_GxB_ISGT_UINT32_ztype uint32_t
#define GB_DEF_GxB_ISGT_UINT32_xtype uint32_t
#define GB_DEF_GxB_ISGT_UINT32_ytype uint32_t

#define GB_DEF_GxB_ISGT_INT64_function GB_ISGT_f_INT64
#define GB_DEF_GxB_ISGT_INT64_ztype int64_t
#define GB_DEF_GxB_ISGT_INT64_xtype int64_t
#define GB_DEF_GxB_ISGT_INT64_ytype int64_t

#define GB_DEF_GxB_ISGT_UINT64_function GB_ISGT_f_UINT64
#define GB_DEF_GxB_ISGT_UINT64_ztype uint64_t
#define GB_DEF_GxB_ISGT_UINT64_xtype uint64_t
#define GB_DEF_GxB_ISGT_UINT64_ytype uint64_t

#define GB_DEF_GxB_ISGT_FP32_function GB_ISGT_f_FP32
#define GB_DEF_GxB_ISGT_FP32_ztype float
#define GB_DEF_GxB_ISGT_FP32_xtype float
#define GB_DEF_GxB_ISGT_FP32_ytype float

#define GB_DEF_GxB_ISGT_FP64_function GB_ISGT_f_FP64
#define GB_DEF_GxB_ISGT_FP64_ztype double
#define GB_DEF_GxB_ISGT_FP64_xtype double
#define GB_DEF_GxB_ISGT_FP64_ytype double

// op: ISLT
#define GB_DEF_GxB_ISLT_BOOL_function GB_ISLT_f_BOOL
#define GB_DEF_GxB_ISLT_BOOL_ztype bool
#define GB_DEF_GxB_ISLT_BOOL_xtype bool
#define GB_DEF_GxB_ISLT_BOOL_ytype bool

#define GB_DEF_GxB_ISLT_INT8_function GB_ISLT_f_INT8
#define GB_DEF_GxB_ISLT_INT8_ztype int8_t
#define GB_DEF_GxB_ISLT_INT8_xtype int8_t
#define GB_DEF_GxB_ISLT_INT8_ytype int8_t

#define GB_DEF_GxB_ISLT_UINT8_function GB_ISLT_f_UINT8
#define GB_DEF_GxB_ISLT_UINT8_ztype uint8_t
#define GB_DEF_GxB_ISLT_UINT8_xtype uint8_t
#define GB_DEF_GxB_ISLT_UINT8_ytype uint8_t

#define GB_DEF_GxB_ISLT_INT16_function GB_ISLT_f_INT16
#define GB_DEF_GxB_ISLT_INT16_ztype int16_t
#define GB_DEF_GxB_ISLT_INT16_xtype int16_t
#define GB_DEF_GxB_ISLT_INT16_ytype int16_t

#define GB_DEF_GxB_ISLT_UINT16_function GB_ISLT_f_UINT16
#define GB_DEF_GxB_ISLT_UINT16_ztype uint16_t
#define GB_DEF_GxB_ISLT_UINT16_xtype uint16_t
#define GB_DEF_GxB_ISLT_UINT16_ytype uint16_t

#define GB_DEF_GxB_ISLT_INT32_function GB_ISLT_f_INT32
#define GB_DEF_GxB_ISLT_INT32_ztype int32_t
#define GB_DEF_GxB_ISLT_INT32_xtype int32_t
#define GB_DEF_GxB_ISLT_INT32_ytype int32_t

#define GB_DEF_GxB_ISLT_UINT32_function GB_ISLT_f_UINT32
#define GB_DEF_GxB_ISLT_UINT32_ztype uint32_t
#define GB_DEF_GxB_ISLT_UINT32_xtype uint32_t
#define GB_DEF_GxB_ISLT_UINT32_ytype uint32_t

#define GB_DEF_GxB_ISLT_INT64_function GB_ISLT_f_INT64
#define GB_DEF_GxB_ISLT_INT64_ztype int64_t
#define GB_DEF_GxB_ISLT_INT64_xtype int64_t
#define GB_DEF_GxB_ISLT_INT64_ytype int64_t

#define GB_DEF_GxB_ISLT_UINT64_function GB_ISLT_f_UINT64
#define GB_DEF_GxB_ISLT_UINT64_ztype uint64_t
#define GB_DEF_GxB_ISLT_UINT64_xtype uint64_t
#define GB_DEF_GxB_ISLT_UINT64_ytype uint64_t

#define GB_DEF_GxB_ISLT_FP32_function GB_ISLT_f_FP32
#define GB_DEF_GxB_ISLT_FP32_ztype float
#define GB_DEF_GxB_ISLT_FP32_xtype float
#define GB_DEF_GxB_ISLT_FP32_ytype float

#define GB_DEF_GxB_ISLT_FP64_function GB_ISLT_f_FP64
#define GB_DEF_GxB_ISLT_FP64_ztype double
#define GB_DEF_GxB_ISLT_FP64_xtype double
#define GB_DEF_GxB_ISLT_FP64_ytype double

// op: ISGE
#define GB_DEF_GxB_ISGE_BOOL_function GB_ISGE_f_BOOL
#define GB_DEF_GxB_ISGE_BOOL_ztype bool
#define GB_DEF_GxB_ISGE_BOOL_xtype bool
#define GB_DEF_GxB_ISGE_BOOL_ytype bool

#define GB_DEF_GxB_ISGE_INT8_function GB_ISGE_f_INT8
#define GB_DEF_GxB_ISGE_INT8_ztype int8_t
#define GB_DEF_GxB_ISGE_INT8_xtype int8_t
#define GB_DEF_GxB_ISGE_INT8_ytype int8_t

#define GB_DEF_GxB_ISGE_UINT8_function GB_ISGE_f_UINT8
#define GB_DEF_GxB_ISGE_UINT8_ztype uint8_t
#define GB_DEF_GxB_ISGE_UINT8_xtype uint8_t
#define GB_DEF_GxB_ISGE_UINT8_ytype uint8_t

#define GB_DEF_GxB_ISGE_INT16_function GB_ISGE_f_INT16
#define GB_DEF_GxB_ISGE_INT16_ztype int16_t
#define GB_DEF_GxB_ISGE_INT16_xtype int16_t
#define GB_DEF_GxB_ISGE_INT16_ytype int16_t

#define GB_DEF_GxB_ISGE_UINT16_function GB_ISGE_f_UINT16
#define GB_DEF_GxB_ISGE_UINT16_ztype uint16_t
#define GB_DEF_GxB_ISGE_UINT16_xtype uint16_t
#define GB_DEF_GxB_ISGE_UINT16_ytype uint16_t

#define GB_DEF_GxB_ISGE_INT32_function GB_ISGE_f_INT32
#define GB_DEF_GxB_ISGE_INT32_ztype int32_t
#define GB_DEF_GxB_ISGE_INT32_xtype int32_t
#define GB_DEF_GxB_ISGE_INT32_ytype int32_t

#define GB_DEF_GxB_ISGE_UINT32_function GB_ISGE_f_UINT32
#define GB_DEF_GxB_ISGE_UINT32_ztype uint32_t
#define GB_DEF_GxB_ISGE_UINT32_xtype uint32_t
#define GB_DEF_GxB_ISGE_UINT32_ytype uint32_t

#define GB_DEF_GxB_ISGE_INT64_function GB_ISGE_f_INT64
#define GB_DEF_GxB_ISGE_INT64_ztype int64_t
#define GB_DEF_GxB_ISGE_INT64_xtype int64_t
#define GB_DEF_GxB_ISGE_INT64_ytype int64_t

#define GB_DEF_GxB_ISGE_UINT64_function GB_ISGE_f_UINT64
#define GB_DEF_GxB_ISGE_UINT64_ztype uint64_t
#define GB_DEF_GxB_ISGE_UINT64_xtype uint64_t
#define GB_DEF_GxB_ISGE_UINT64_ytype uint64_t

#define GB_DEF_GxB_ISGE_FP32_function GB_ISGE_f_FP32
#define GB_DEF_GxB_ISGE_FP32_ztype float
#define GB_DEF_GxB_ISGE_FP32_xtype float
#define GB_DEF_GxB_ISGE_FP32_ytype float

#define GB_DEF_GxB_ISGE_FP64_function GB_ISGE_f_FP64
#define GB_DEF_GxB_ISGE_FP64_ztype double
#define GB_DEF_GxB_ISGE_FP64_xtype double
#define GB_DEF_GxB_ISGE_FP64_ytype double

// op: ISLE
#define GB_DEF_GxB_ISLE_BOOL_function GB_ISLE_f_BOOL
#define GB_DEF_GxB_ISLE_BOOL_ztype bool
#define GB_DEF_GxB_ISLE_BOOL_xtype bool
#define GB_DEF_GxB_ISLE_BOOL_ytype bool

#define GB_DEF_GxB_ISLE_INT8_function GB_ISLE_f_INT8
#define GB_DEF_GxB_ISLE_INT8_ztype int8_t
#define GB_DEF_GxB_ISLE_INT8_xtype int8_t
#define GB_DEF_GxB_ISLE_INT8_ytype int8_t

#define GB_DEF_GxB_ISLE_UINT8_function GB_ISLE_f_UINT8
#define GB_DEF_GxB_ISLE_UINT8_ztype uint8_t
#define GB_DEF_GxB_ISLE_UINT8_xtype uint8_t
#define GB_DEF_GxB_ISLE_UINT8_ytype uint8_t

#define GB_DEF_GxB_ISLE_INT16_function GB_ISLE_f_INT16
#define GB_DEF_GxB_ISLE_INT16_ztype int16_t
#define GB_DEF_GxB_ISLE_INT16_xtype int16_t
#define GB_DEF_GxB_ISLE_INT16_ytype int16_t

#define GB_DEF_GxB_ISLE_UINT16_function GB_ISLE_f_UINT16
#define GB_DEF_GxB_ISLE_UINT16_ztype uint16_t
#define GB_DEF_GxB_ISLE_UINT16_xtype uint16_t
#define GB_DEF_GxB_ISLE_UINT16_ytype uint16_t

#define GB_DEF_GxB_ISLE_INT32_function GB_ISLE_f_INT32
#define GB_DEF_GxB_ISLE_INT32_ztype int32_t
#define GB_DEF_GxB_ISLE_INT32_xtype int32_t
#define GB_DEF_GxB_ISLE_INT32_ytype int32_t

#define GB_DEF_GxB_ISLE_UINT32_function GB_ISLE_f_UINT32
#define GB_DEF_GxB_ISLE_UINT32_ztype uint32_t
#define GB_DEF_GxB_ISLE_UINT32_xtype uint32_t
#define GB_DEF_GxB_ISLE_UINT32_ytype uint32_t

#define GB_DEF_GxB_ISLE_INT64_function GB_ISLE_f_INT64
#define GB_DEF_GxB_ISLE_INT64_ztype int64_t
#define GB_DEF_GxB_ISLE_INT64_xtype int64_t
#define GB_DEF_GxB_ISLE_INT64_ytype int64_t

#define GB_DEF_GxB_ISLE_UINT64_function GB_ISLE_f_UINT64
#define GB_DEF_GxB_ISLE_UINT64_ztype uint64_t
#define GB_DEF_GxB_ISLE_UINT64_xtype uint64_t
#define GB_DEF_GxB_ISLE_UINT64_ytype uint64_t

#define GB_DEF_GxB_ISLE_FP32_function GB_ISLE_f_FP32
#define GB_DEF_GxB_ISLE_FP32_ztype float
#define GB_DEF_GxB_ISLE_FP32_xtype float
#define GB_DEF_GxB_ISLE_FP32_ytype float

#define GB_DEF_GxB_ISLE_FP64_function GB_ISLE_f_FP64
#define GB_DEF_GxB_ISLE_FP64_ztype double
#define GB_DEF_GxB_ISLE_FP64_xtype double
#define GB_DEF_GxB_ISLE_FP64_ytype double

// op: LOR
#define GB_DEF_GrB_LOR_function GB_LOR_f_BOOL
#define GB_DEF_GrB_LOR_ztype bool
#define GB_DEF_GrB_LOR_xtype bool
#define GB_DEF_GrB_LOR_ytype bool

#define GB_DEF_GxB_LOR_BOOL_function GB_LOR_f_BOOL
#define GB_DEF_GxB_LOR_BOOL_ztype bool
#define GB_DEF_GxB_LOR_BOOL_xtype bool
#define GB_DEF_GxB_LOR_BOOL_ytype bool

#define GB_DEF_GxB_LOR_INT8_function GB_LOR_f_INT8
#define GB_DEF_GxB_LOR_INT8_ztype int8_t
#define GB_DEF_GxB_LOR_INT8_xtype int8_t
#define GB_DEF_GxB_LOR_INT8_ytype int8_t

#define GB_DEF_GxB_LOR_UINT8_function GB_LOR_f_UINT8
#define GB_DEF_GxB_LOR_UINT8_ztype uint8_t
#define GB_DEF_GxB_LOR_UINT8_xtype uint8_t
#define GB_DEF_GxB_LOR_UINT8_ytype uint8_t

#define GB_DEF_GxB_LOR_INT16_function GB_LOR_f_INT16
#define GB_DEF_GxB_LOR_INT16_ztype int16_t
#define GB_DEF_GxB_LOR_INT16_xtype int16_t
#define GB_DEF_GxB_LOR_INT16_ytype int16_t

#define GB_DEF_GxB_LOR_UINT16_function GB_LOR_f_UINT16
#define GB_DEF_GxB_LOR_UINT16_ztype uint16_t
#define GB_DEF_GxB_LOR_UINT16_xtype uint16_t
#define GB_DEF_GxB_LOR_UINT16_ytype uint16_t

#define GB_DEF_GxB_LOR_INT32_function GB_LOR_f_INT32
#define GB_DEF_GxB_LOR_INT32_ztype int32_t
#define GB_DEF_GxB_LOR_INT32_xtype int32_t
#define GB_DEF_GxB_LOR_INT32_ytype int32_t

#define GB_DEF_GxB_LOR_UINT32_function GB_LOR_f_UINT32
#define GB_DEF_GxB_LOR_UINT32_ztype uint32_t
#define GB_DEF_GxB_LOR_UINT32_xtype uint32_t
#define GB_DEF_GxB_LOR_UINT32_ytype uint32_t

#define GB_DEF_GxB_LOR_INT64_function GB_LOR_f_INT64
#define GB_DEF_GxB_LOR_INT64_ztype int64_t
#define GB_DEF_GxB_LOR_INT64_xtype int64_t
#define GB_DEF_GxB_LOR_INT64_ytype int64_t

#define GB_DEF_GxB_LOR_UINT64_function GB_LOR_f_UINT64
#define GB_DEF_GxB_LOR_UINT64_ztype uint64_t
#define GB_DEF_GxB_LOR_UINT64_xtype uint64_t
#define GB_DEF_GxB_LOR_UINT64_ytype uint64_t

#define GB_DEF_GxB_LOR_FP32_function GB_LOR_f_FP32
#define GB_DEF_GxB_LOR_FP32_ztype float
#define GB_DEF_GxB_LOR_FP32_xtype float
#define GB_DEF_GxB_LOR_FP32_ytype float

#define GB_DEF_GxB_LOR_FP64_function GB_LOR_f_FP64
#define GB_DEF_GxB_LOR_FP64_ztype double
#define GB_DEF_GxB_LOR_FP64_xtype double
#define GB_DEF_GxB_LOR_FP64_ytype double

// op: LAND
#define GB_DEF_GrB_LAND_function GB_LAND_f_BOOL
#define GB_DEF_GrB_LAND_ztype bool
#define GB_DEF_GrB_LAND_xtype bool
#define GB_DEF_GrB_LAND_ytype bool

#define GB_DEF_GxB_LAND_BOOL_function GB_LAND_f_BOOL
#define GB_DEF_GxB_LAND_BOOL_ztype bool
#define GB_DEF_GxB_LAND_BOOL_xtype bool
#define GB_DEF_GxB_LAND_BOOL_ytype bool

#define GB_DEF_GxB_LAND_INT8_function GB_LAND_f_INT8
#define GB_DEF_GxB_LAND_INT8_ztype int8_t
#define GB_DEF_GxB_LAND_INT8_xtype int8_t
#define GB_DEF_GxB_LAND_INT8_ytype int8_t

#define GB_DEF_GxB_LAND_UINT8_function GB_LAND_f_UINT8
#define GB_DEF_GxB_LAND_UINT8_ztype uint8_t
#define GB_DEF_GxB_LAND_UINT8_xtype uint8_t
#define GB_DEF_GxB_LAND_UINT8_ytype uint8_t

#define GB_DEF_GxB_LAND_INT16_function GB_LAND_f_INT16
#define GB_DEF_GxB_LAND_INT16_ztype int16_t
#define GB_DEF_GxB_LAND_INT16_xtype int16_t
#define GB_DEF_GxB_LAND_INT16_ytype int16_t

#define GB_DEF_GxB_LAND_UINT16_function GB_LAND_f_UINT16
#define GB_DEF_GxB_LAND_UINT16_ztype uint16_t
#define GB_DEF_GxB_LAND_UINT16_xtype uint16_t
#define GB_DEF_GxB_LAND_UINT16_ytype uint16_t

#define GB_DEF_GxB_LAND_INT32_function GB_LAND_f_INT32
#define GB_DEF_GxB_LAND_INT32_ztype int32_t
#define GB_DEF_GxB_LAND_INT32_xtype int32_t
#define GB_DEF_GxB_LAND_INT32_ytype int32_t

#define GB_DEF_GxB_LAND_UINT32_function GB_LAND_f_UINT32
#define GB_DEF_GxB_LAND_UINT32_ztype uint32_t
#define GB_DEF_GxB_LAND_UINT32_xtype uint32_t
#define GB_DEF_GxB_LAND_UINT32_ytype uint32_t

#define GB_DEF_GxB_LAND_INT64_function GB_LAND_f_INT64
#define GB_DEF_GxB_LAND_INT64_ztype int64_t
#define GB_DEF_GxB_LAND_INT64_xtype int64_t
#define GB_DEF_GxB_LAND_INT64_ytype int64_t

#define GB_DEF_GxB_LAND_UINT64_function GB_LAND_f_UINT64
#define GB_DEF_GxB_LAND_UINT64_ztype uint64_t
#define GB_DEF_GxB_LAND_UINT64_xtype uint64_t
#define GB_DEF_GxB_LAND_UINT64_ytype uint64_t

#define GB_DEF_GxB_LAND_FP32_function GB_LAND_f_FP32
#define GB_DEF_GxB_LAND_FP32_ztype float
#define GB_DEF_GxB_LAND_FP32_xtype float
#define GB_DEF_GxB_LAND_FP32_ytype float

#define GB_DEF_GxB_LAND_FP64_function GB_LAND_f_FP64
#define GB_DEF_GxB_LAND_FP64_ztype double
#define GB_DEF_GxB_LAND_FP64_xtype double
#define GB_DEF_GxB_LAND_FP64_ytype double

// op: LXOR
#define GB_DEF_GrB_LXOR_function GB_LXOR_f_BOOL
#define GB_DEF_GrB_LXOR_ztype bool
#define GB_DEF_GrB_LXOR_xtype bool
#define GB_DEF_GrB_LXOR_ytype bool

#define GB_DEF_GxB_LXOR_BOOL_function GB_LXOR_f_BOOL
#define GB_DEF_GxB_LXOR_BOOL_ztype bool
#define GB_DEF_GxB_LXOR_BOOL_xtype bool
#define GB_DEF_GxB_LXOR_BOOL_ytype bool

#define GB_DEF_GxB_LXOR_INT8_function GB_LXOR_f_INT8
#define GB_DEF_GxB_LXOR_INT8_ztype int8_t
#define GB_DEF_GxB_LXOR_INT8_xtype int8_t
#define GB_DEF_GxB_LXOR_INT8_ytype int8_t

#define GB_DEF_GxB_LXOR_UINT8_function GB_LXOR_f_UINT8
#define GB_DEF_GxB_LXOR_UINT8_ztype uint8_t
#define GB_DEF_GxB_LXOR_UINT8_xtype uint8_t
#define GB_DEF_GxB_LXOR_UINT8_ytype uint8_t

#define GB_DEF_GxB_LXOR_INT16_function GB_LXOR_f_INT16
#define GB_DEF_GxB_LXOR_INT16_ztype int16_t
#define GB_DEF_GxB_LXOR_INT16_xtype int16_t
#define GB_DEF_GxB_LXOR_INT16_ytype int16_t

#define GB_DEF_GxB_LXOR_UINT16_function GB_LXOR_f_UINT16
#define GB_DEF_GxB_LXOR_UINT16_ztype uint16_t
#define GB_DEF_GxB_LXOR_UINT16_xtype uint16_t
#define GB_DEF_GxB_LXOR_UINT16_ytype uint16_t

#define GB_DEF_GxB_LXOR_INT32_function GB_LXOR_f_INT32
#define GB_DEF_GxB_LXOR_INT32_ztype int32_t
#define GB_DEF_GxB_LXOR_INT32_xtype int32_t
#define GB_DEF_GxB_LXOR_INT32_ytype int32_t

#define GB_DEF_GxB_LXOR_UINT32_function GB_LXOR_f_UINT32
#define GB_DEF_GxB_LXOR_UINT32_ztype uint32_t
#define GB_DEF_GxB_LXOR_UINT32_xtype uint32_t
#define GB_DEF_GxB_LXOR_UINT32_ytype uint32_t

#define GB_DEF_GxB_LXOR_INT64_function GB_LXOR_f_INT64
#define GB_DEF_GxB_LXOR_INT64_ztype int64_t
#define GB_DEF_GxB_LXOR_INT64_xtype int64_t
#define GB_DEF_GxB_LXOR_INT64_ytype int64_t

#define GB_DEF_GxB_LXOR_UINT64_function GB_LXOR_f_UINT64
#define GB_DEF_GxB_LXOR_UINT64_ztype uint64_t
#define GB_DEF_GxB_LXOR_UINT64_xtype uint64_t
#define GB_DEF_GxB_LXOR_UINT64_ytype uint64_t

#define GB_DEF_GxB_LXOR_FP32_function GB_LXOR_f_FP32
#define GB_DEF_GxB_LXOR_FP32_ztype float
#define GB_DEF_GxB_LXOR_FP32_xtype float
#define GB_DEF_GxB_LXOR_FP32_ytype float

#define GB_DEF_GxB_LXOR_FP64_function GB_LXOR_f_FP64
#define GB_DEF_GxB_LXOR_FP64_ztype double
#define GB_DEF_GxB_LXOR_FP64_xtype double
#define GB_DEF_GxB_LXOR_FP64_ytype double


//------------------------------------------------------
// binary operators of the form z=f(x,y): TxT -> bool
//------------------------------------------------------

// op: EQ
#define GB_DEF_GrB_EQ_BOOL_function GB_EQ_f_BOOL
#define GB_DEF_GrB_EQ_BOOL_ztype bool
#define GB_DEF_GrB_EQ_BOOL_xtype bool
#define GB_DEF_GrB_EQ_BOOL_ytype bool

#define GB_DEF_GrB_EQ_INT8_function GB_EQ_f_INT8
#define GB_DEF_GrB_EQ_INT8_ztype bool
#define GB_DEF_GrB_EQ_INT8_xtype int8_t
#define GB_DEF_GrB_EQ_INT8_ytype int8_t

#define GB_DEF_GrB_EQ_UINT8_function GB_EQ_f_UINT8
#define GB_DEF_GrB_EQ_UINT8_ztype bool
#define GB_DEF_GrB_EQ_UINT8_xtype uint8_t
#define GB_DEF_GrB_EQ_UINT8_ytype uint8_t

#define GB_DEF_GrB_EQ_INT16_function GB_EQ_f_INT16
#define GB_DEF_GrB_EQ_INT16_ztype bool
#define GB_DEF_GrB_EQ_INT16_xtype int16_t
#define GB_DEF_GrB_EQ_INT16_ytype int16_t

#define GB_DEF_GrB_EQ_UINT16_function GB_EQ_f_UINT16
#define GB_DEF_GrB_EQ_UINT16_ztype bool
#define GB_DEF_GrB_EQ_UINT16_xtype uint16_t
#define GB_DEF_GrB_EQ_UINT16_ytype uint16_t

#define GB_DEF_GrB_EQ_INT32_function GB_EQ_f_INT32
#define GB_DEF_GrB_EQ_INT32_ztype bool
#define GB_DEF_GrB_EQ_INT32_xtype int32_t
#define GB_DEF_GrB_EQ_INT32_ytype int32_t

#define GB_DEF_GrB_EQ_UINT32_function GB_EQ_f_UINT32
#define GB_DEF_GrB_EQ_UINT32_ztype bool
#define GB_DEF_GrB_EQ_UINT32_xtype uint32_t
#define GB_DEF_GrB_EQ_UINT32_ytype uint32_t

#define GB_DEF_GrB_EQ_INT64_function GB_EQ_f_INT64
#define GB_DEF_GrB_EQ_INT64_ztype bool
#define GB_DEF_GrB_EQ_INT64_xtype int64_t
#define GB_DEF_GrB_EQ_INT64_ytype int64_t

#define GB_DEF_GrB_EQ_UINT64_function GB_EQ_f_UINT64
#define GB_DEF_GrB_EQ_UINT64_ztype bool
#define GB_DEF_GrB_EQ_UINT64_xtype uint64_t
#define GB_DEF_GrB_EQ_UINT64_ytype uint64_t

#define GB_DEF_GrB_EQ_FP32_function GB_EQ_f_FP32
#define GB_DEF_GrB_EQ_FP32_ztype bool
#define GB_DEF_GrB_EQ_FP32_xtype float
#define GB_DEF_GrB_EQ_FP32_ytype float

#define GB_DEF_GrB_EQ_FP64_function GB_EQ_f_FP64
#define GB_DEF_GrB_EQ_FP64_ztype bool
#define GB_DEF_GrB_EQ_FP64_xtype double
#define GB_DEF_GrB_EQ_FP64_ytype double

// op: NE
#define GB_DEF_GrB_NE_BOOL_function GB_NE_f_BOOL
#define GB_DEF_GrB_NE_BOOL_ztype bool
#define GB_DEF_GrB_NE_BOOL_xtype bool
#define GB_DEF_GrB_NE_BOOL_ytype bool

#define GB_DEF_GrB_NE_INT8_function GB_NE_f_INT8
#define GB_DEF_GrB_NE_INT8_ztype bool
#define GB_DEF_GrB_NE_INT8_xtype int8_t
#define GB_DEF_GrB_NE_INT8_ytype int8_t

#define GB_DEF_GrB_NE_UINT8_function GB_NE_f_UINT8
#define GB_DEF_GrB_NE_UINT8_ztype bool
#define GB_DEF_GrB_NE_UINT8_xtype uint8_t
#define GB_DEF_GrB_NE_UINT8_ytype uint8_t

#define GB_DEF_GrB_NE_INT16_function GB_NE_f_INT16
#define GB_DEF_GrB_NE_INT16_ztype bool
#define GB_DEF_GrB_NE_INT16_xtype int16_t
#define GB_DEF_GrB_NE_INT16_ytype int16_t

#define GB_DEF_GrB_NE_UINT16_function GB_NE_f_UINT16
#define GB_DEF_GrB_NE_UINT16_ztype bool
#define GB_DEF_GrB_NE_UINT16_xtype uint16_t
#define GB_DEF_GrB_NE_UINT16_ytype uint16_t

#define GB_DEF_GrB_NE_INT32_function GB_NE_f_INT32
#define GB_DEF_GrB_NE_INT32_ztype bool
#define GB_DEF_GrB_NE_INT32_xtype int32_t
#define GB_DEF_GrB_NE_INT32_ytype int32_t

#define GB_DEF_GrB_NE_UINT32_function GB_NE_f_UINT32
#define GB_DEF_GrB_NE_UINT32_ztype bool
#define GB_DEF_GrB_NE_UINT32_xtype uint32_t
#define GB_DEF_GrB_NE_UINT32_ytype uint32_t

#define GB_DEF_GrB_NE_INT64_function GB_NE_f_INT64
#define GB_DEF_GrB_NE_INT64_ztype bool
#define GB_DEF_GrB_NE_INT64_xtype int64_t
#define GB_DEF_GrB_NE_INT64_ytype int64_t

#define GB_DEF_GrB_NE_UINT64_function GB_NE_f_UINT64
#define GB_DEF_GrB_NE_UINT64_ztype bool
#define GB_DEF_GrB_NE_UINT64_xtype uint64_t
#define GB_DEF_GrB_NE_UINT64_ytype uint64_t

#define GB_DEF_GrB_NE_FP32_function GB_NE_f_FP32
#define GB_DEF_GrB_NE_FP32_ztype bool
#define GB_DEF_GrB_NE_FP32_xtype float
#define GB_DEF_GrB_NE_FP32_ytype float

#define GB_DEF_GrB_NE_FP64_function GB_NE_f_FP64
#define GB_DEF_GrB_NE_FP64_ztype bool
#define GB_DEF_GrB_NE_FP64_xtype double
#define GB_DEF_GrB_NE_FP64_ytype double

// op: GT
#define GB_DEF_GrB_GT_BOOL_function GB_GT_f_BOOL
#define GB_DEF_GrB_GT_BOOL_ztype bool
#define GB_DEF_GrB_GT_BOOL_xtype bool
#define GB_DEF_GrB_GT_BOOL_ytype bool

#define GB_DEF_GrB_GT_INT8_function GB_GT_f_INT8
#define GB_DEF_GrB_GT_INT8_ztype bool
#define GB_DEF_GrB_GT_INT8_xtype int8_t
#define GB_DEF_GrB_GT_INT8_ytype int8_t

#define GB_DEF_GrB_GT_UINT8_function GB_GT_f_UINT8
#define GB_DEF_GrB_GT_UINT8_ztype bool
#define GB_DEF_GrB_GT_UINT8_xtype uint8_t
#define GB_DEF_GrB_GT_UINT8_ytype uint8_t

#define GB_DEF_GrB_GT_INT16_function GB_GT_f_INT16
#define GB_DEF_GrB_GT_INT16_ztype bool
#define GB_DEF_GrB_GT_INT16_xtype int16_t
#define GB_DEF_GrB_GT_INT16_ytype int16_t

#define GB_DEF_GrB_GT_UINT16_function GB_GT_f_UINT16
#define GB_DEF_GrB_GT_UINT16_ztype bool
#define GB_DEF_GrB_GT_UINT16_xtype uint16_t
#define GB_DEF_GrB_GT_UINT16_ytype uint16_t

#define GB_DEF_GrB_GT_INT32_function GB_GT_f_INT32
#define GB_DEF_GrB_GT_INT32_ztype bool
#define GB_DEF_GrB_GT_INT32_xtype int32_t
#define GB_DEF_GrB_GT_INT32_ytype int32_t

#define GB_DEF_GrB_GT_UINT32_function GB_GT_f_UINT32
#define GB_DEF_GrB_GT_UINT32_ztype bool
#define GB_DEF_GrB_GT_UINT32_xtype uint32_t
#define GB_DEF_GrB_GT_UINT32_ytype uint32_t

#define GB_DEF_GrB_GT_INT64_function GB_GT_f_INT64
#define GB_DEF_GrB_GT_INT64_ztype bool
#define GB_DEF_GrB_GT_INT64_xtype int64_t
#define GB_DEF_GrB_GT_INT64_ytype int64_t

#define GB_DEF_GrB_GT_UINT64_function GB_GT_f_UINT64
#define GB_DEF_GrB_GT_UINT64_ztype bool
#define GB_DEF_GrB_GT_UINT64_xtype uint64_t
#define GB_DEF_GrB_GT_UINT64_ytype uint64_t

#define GB_DEF_GrB_GT_FP32_function GB_GT_f_FP32
#define GB_DEF_GrB_GT_FP32_ztype bool
#define GB_DEF_GrB_GT_FP32_xtype float
#define GB_DEF_GrB_GT_FP32_ytype float

#define GB_DEF_GrB_GT_FP64_function GB_GT_f_FP64
#define GB_DEF_GrB_GT_FP64_ztype bool
#define GB_DEF_GrB_GT_FP64_xtype double
#define GB_DEF_GrB_GT_FP64_ytype double

// op: LT
#define GB_DEF_GrB_LT_BOOL_function GB_LT_f_BOOL
#define GB_DEF_GrB_LT_BOOL_ztype bool
#define GB_DEF_GrB_LT_BOOL_xtype bool
#define GB_DEF_GrB_LT_BOOL_ytype bool

#define GB_DEF_GrB_LT_INT8_function GB_LT_f_INT8
#define GB_DEF_GrB_LT_INT8_ztype bool
#define GB_DEF_GrB_LT_INT8_xtype int8_t
#define GB_DEF_GrB_LT_INT8_ytype int8_t

#define GB_DEF_GrB_LT_UINT8_function GB_LT_f_UINT8
#define GB_DEF_GrB_LT_UINT8_ztype bool
#define GB_DEF_GrB_LT_UINT8_xtype uint8_t
#define GB_DEF_GrB_LT_UINT8_ytype uint8_t

#define GB_DEF_GrB_LT_INT16_function GB_LT_f_INT16
#define GB_DEF_GrB_LT_INT16_ztype bool
#define GB_DEF_GrB_LT_INT16_xtype int16_t
#define GB_DEF_GrB_LT_INT16_ytype int16_t

#define GB_DEF_GrB_LT_UINT16_function GB_LT_f_UINT16
#define GB_DEF_GrB_LT_UINT16_ztype bool
#define GB_DEF_GrB_LT_UINT16_xtype uint16_t
#define GB_DEF_GrB_LT_UINT16_ytype uint16_t

#define GB_DEF_GrB_LT_INT32_function GB_LT_f_INT32
#define GB_DEF_GrB_LT_INT32_ztype bool
#define GB_DEF_GrB_LT_INT32_xtype int32_t
#define GB_DEF_GrB_LT_INT32_ytype int32_t

#define GB_DEF_GrB_LT_UINT32_function GB_LT_f_UINT32
#define GB_DEF_GrB_LT_UINT32_ztype bool
#define GB_DEF_GrB_LT_UINT32_xtype uint32_t
#define GB_DEF_GrB_LT_UINT32_ytype uint32_t

#define GB_DEF_GrB_LT_INT64_function GB_LT_f_INT64
#define GB_DEF_GrB_LT_INT64_ztype bool
#define GB_DEF_GrB_LT_INT64_xtype int64_t
#define GB_DEF_GrB_LT_INT64_ytype int64_t

#define GB_DEF_GrB_LT_UINT64_function GB_LT_f_UINT64
#define GB_DEF_GrB_LT_UINT64_ztype bool
#define GB_DEF_GrB_LT_UINT64_xtype uint64_t
#define GB_DEF_GrB_LT_UINT64_ytype uint64_t

#define GB_DEF_GrB_LT_FP32_function GB_LT_f_FP32
#define GB_DEF_GrB_LT_FP32_ztype bool
#define GB_DEF_GrB_LT_FP32_xtype float
#define GB_DEF_GrB_LT_FP32_ytype float

#define GB_DEF_GrB_LT_FP64_function GB_LT_f_FP64
#define GB_DEF_GrB_LT_FP64_ztype bool
#define GB_DEF_GrB_LT_FP64_xtype double
#define GB_DEF_GrB_LT_FP64_ytype double

// op: GE
#define GB_DEF_GrB_GE_BOOL_function GB_GE_f_BOOL
#define GB_DEF_GrB_GE_BOOL_ztype bool
#define GB_DEF_GrB_GE_BOOL_xtype bool
#define GB_DEF_GrB_GE_BOOL_ytype bool

#define GB_DEF_GrB_GE_INT8_function GB_GE_f_INT8
#define GB_DEF_GrB_GE_INT8_ztype bool
#define GB_DEF_GrB_GE_INT8_xtype int8_t
#define GB_DEF_GrB_GE_INT8_ytype int8_t

#define GB_DEF_GrB_GE_UINT8_function GB_GE_f_UINT8
#define GB_DEF_GrB_GE_UINT8_ztype bool
#define GB_DEF_GrB_GE_UINT8_xtype uint8_t
#define GB_DEF_GrB_GE_UINT8_ytype uint8_t

#define GB_DEF_GrB_GE_INT16_function GB_GE_f_INT16
#define GB_DEF_GrB_GE_INT16_ztype bool
#define GB_DEF_GrB_GE_INT16_xtype int16_t
#define GB_DEF_GrB_GE_INT16_ytype int16_t

#define GB_DEF_GrB_GE_UINT16_function GB_GE_f_UINT16
#define GB_DEF_GrB_GE_UINT16_ztype bool
#define GB_DEF_GrB_GE_UINT16_xtype uint16_t
#define GB_DEF_GrB_GE_UINT16_ytype uint16_t

#define GB_DEF_GrB_GE_INT32_function GB_GE_f_INT32
#define GB_DEF_GrB_GE_INT32_ztype bool
#define GB_DEF_GrB_GE_INT32_xtype int32_t
#define GB_DEF_GrB_GE_INT32_ytype int32_t

#define GB_DEF_GrB_GE_UINT32_function GB_GE_f_UINT32
#define GB_DEF_GrB_GE_UINT32_ztype bool
#define GB_DEF_GrB_GE_UINT32_xtype uint32_t
#define GB_DEF_GrB_GE_UINT32_ytype uint32_t

#define GB_DEF_GrB_GE_INT64_function GB_GE_f_INT64
#define GB_DEF_GrB_GE_INT64_ztype bool
#define GB_DEF_GrB_GE_INT64_xtype int64_t
#define GB_DEF_GrB_GE_INT64_ytype int64_t

#define GB_DEF_GrB_GE_UINT64_function GB_GE_f_UINT64
#define GB_DEF_GrB_GE_UINT64_ztype bool
#define GB_DEF_GrB_GE_UINT64_xtype uint64_t
#define GB_DEF_GrB_GE_UINT64_ytype uint64_t

#define GB_DEF_GrB_GE_FP32_function GB_GE_f_FP32
#define GB_DEF_GrB_GE_FP32_ztype bool
#define GB_DEF_GrB_GE_FP32_xtype float
#define GB_DEF_GrB_GE_FP32_ytype float

#define GB_DEF_GrB_GE_FP64_function GB_GE_f_FP64
#define GB_DEF_GrB_GE_FP64_ztype bool
#define GB_DEF_GrB_GE_FP64_xtype double
#define GB_DEF_GrB_GE_FP64_ytype double

// op: LE
#define GB_DEF_GrB_LE_BOOL_function GB_LE_f_BOOL
#define GB_DEF_GrB_LE_BOOL_ztype bool
#define GB_DEF_GrB_LE_BOOL_xtype bool
#define GB_DEF_GrB_LE_BOOL_ytype bool

#define GB_DEF_GrB_LE_INT8_function GB_LE_f_INT8
#define GB_DEF_GrB_LE_INT8_ztype bool
#define GB_DEF_GrB_LE_INT8_xtype int8_t
#define GB_DEF_GrB_LE_INT8_ytype int8_t

#define GB_DEF_GrB_LE_UINT8_function GB_LE_f_UINT8
#define GB_DEF_GrB_LE_UINT8_ztype bool
#define GB_DEF_GrB_LE_UINT8_xtype uint8_t
#define GB_DEF_GrB_LE_UINT8_ytype uint8_t

#define GB_DEF_GrB_LE_INT16_function GB_LE_f_INT16
#define GB_DEF_GrB_LE_INT16_ztype bool
#define GB_DEF_GrB_LE_INT16_xtype int16_t
#define GB_DEF_GrB_LE_INT16_ytype int16_t

#define GB_DEF_GrB_LE_UINT16_function GB_LE_f_UINT16
#define GB_DEF_GrB_LE_UINT16_ztype bool
#define GB_DEF_GrB_LE_UINT16_xtype uint16_t
#define GB_DEF_GrB_LE_UINT16_ytype uint16_t

#define GB_DEF_GrB_LE_INT32_function GB_LE_f_INT32
#define GB_DEF_GrB_LE_INT32_ztype bool
#define GB_DEF_GrB_LE_INT32_xtype int32_t
#define GB_DEF_GrB_LE_INT32_ytype int32_t

#define GB_DEF_GrB_LE_UINT32_function GB_LE_f_UINT32
#define GB_DEF_GrB_LE_UINT32_ztype bool
#define GB_DEF_GrB_LE_UINT32_xtype uint32_t
#define GB_DEF_GrB_LE_UINT32_ytype uint32_t

#define GB_DEF_GrB_LE_INT64_function GB_LE_f_INT64
#define GB_DEF_GrB_LE_INT64_ztype bool
#define GB_DEF_GrB_LE_INT64_xtype int64_t
#define GB_DEF_GrB_LE_INT64_ytype int64_t

#define GB_DEF_GrB_LE_UINT64_function GB_LE_f_UINT64
#define GB_DEF_GrB_LE_UINT64_ztype bool
#define GB_DEF_GrB_LE_UINT64_xtype uint64_t
#define GB_DEF_GrB_LE_UINT64_ytype uint64_t

#define GB_DEF_GrB_LE_FP32_function GB_LE_f_FP32
#define GB_DEF_GrB_LE_FP32_ztype bool
#define GB_DEF_GrB_LE_FP32_xtype float
#define GB_DEF_GrB_LE_FP32_ytype float

#define GB_DEF_GrB_LE_FP64_function GB_LE_f_FP64
#define GB_DEF_GrB_LE_FP64_ztype bool
#define GB_DEF_GrB_LE_FP64_xtype double
#define GB_DEF_GrB_LE_FP64_ytype double


//------------------------------------------------------
// binary operators of the form z=f(x,y): bool x bool -> bool
//------------------------------------------------------

#define GB_DEF_GrB_LOR_function GB_LOR_f_BOOL
#define GB_DEF_GrB_LOR_ztype bool
#define GB_DEF_GrB_LOR_xtype bool
#define GB_DEF_GrB_LOR_ytype bool

#define GB_DEF_GrB_LAND_function GB_LAND_f_BOOL
#define GB_DEF_GrB_LAND_ztype bool
#define GB_DEF_GrB_LAND_xtype bool
#define GB_DEF_GrB_LAND_ytype bool

#define GB_DEF_GrB_LXOR_function GB_LXOR_f_BOOL
#define GB_DEF_GrB_LXOR_ztype bool
#define GB_DEF_GrB_LXOR_xtype bool
#define GB_DEF_GrB_LXOR_ytype bool


//------------------------------------------------------
// built-in monoids
//------------------------------------------------------

// op: MIN
#define GB_DEF_GxB_MIN_BOOL_MONOID_add GB_MIN_f_BOOL
#define GB_DEF_GxB_MIN_INT8_MONOID_add GB_MIN_f_INT8
#define GB_DEF_GxB_MIN_UINT8_MONOID_add GB_MIN_f_UINT8
#define GB_DEF_GxB_MIN_INT16_MONOID_add GB_MIN_f_INT16
#define GB_DEF_GxB_MIN_UINT16_MONOID_add GB_MIN_f_UINT16
#define GB_DEF_GxB_MIN_INT32_MONOID_add GB_MIN_f_INT32
#define GB_DEF_GxB_MIN_UINT32_MONOID_add GB_MIN_f_UINT32
#define GB_DEF_GxB_MIN_INT64_MONOID_add GB_MIN_f_INT64
#define GB_DEF_GxB_MIN_UINT64_MONOID_add GB_MIN_f_UINT64
#define GB_DEF_GxB_MIN_FP32_MONOID_add GB_MIN_f_FP32
#define GB_DEF_GxB_MIN_FP64_MONOID_add GB_MIN_f_FP64
// op: MAX
#define GB_DEF_GxB_MAX_BOOL_MONOID_add GB_MAX_f_BOOL
#define GB_DEF_GxB_MAX_INT8_MONOID_add GB_MAX_f_INT8
#define GB_DEF_GxB_MAX_UINT8_MONOID_add GB_MAX_f_UINT8
#define GB_DEF_GxB_MAX_INT16_MONOID_add GB_MAX_f_INT16
#define GB_DEF_GxB_MAX_UINT16_MONOID_add GB_MAX_f_UINT16
#define GB_DEF_GxB_MAX_INT32_MONOID_add GB_MAX_f_INT32
#define GB_DEF_GxB_MAX_UINT32_MONOID_add GB_MAX_f_UINT32
#define GB_DEF_GxB_MAX_INT64_MONOID_add GB_MAX_f_INT64
#define GB_DEF_GxB_MAX_UINT64_MONOID_add GB_MAX_f_UINT64
#define GB_DEF_GxB_MAX_FP32_MONOID_add GB_MAX_f_FP32
#define GB_DEF_GxB_MAX_FP64_MONOID_add GB_MAX_f_FP64
// op: PLUS
#define GB_DEF_GxB_PLUS_BOOL_MONOID_add GB_PLUS_f_BOOL
#define GB_DEF_GxB_PLUS_INT8_MONOID_add GB_PLUS_f_INT8
#define GB_DEF_GxB_PLUS_UINT8_MONOID_add GB_PLUS_f_UINT8
#define GB_DEF_GxB_PLUS_INT16_MONOID_add GB_PLUS_f_INT16
#define GB_DEF_GxB_PLUS_UINT16_MONOID_add GB_PLUS_f_UINT16
#define GB_DEF_GxB_PLUS_INT32_MONOID_add GB_PLUS_f_INT32
#define GB_DEF_GxB_PLUS_UINT32_MONOID_add GB_PLUS_f_UINT32
#define GB_DEF_GxB_PLUS_INT64_MONOID_add GB_PLUS_f_INT64
#define GB_DEF_GxB_PLUS_UINT64_MONOID_add GB_PLUS_f_UINT64
#define GB_DEF_GxB_PLUS_FP32_MONOID_add GB_PLUS_f_FP32
#define GB_DEF_GxB_PLUS_FP64_MONOID_add GB_PLUS_f_FP64
// op: TIMES
#define GB_DEF_GxB_TIMES_BOOL_MONOID_add GB_TIMES_f_BOOL
#define GB_DEF_GxB_TIMES_INT8_MONOID_add GB_TIMES_f_INT8
#define GB_DEF_GxB_TIMES_UINT8_MONOID_add GB_TIMES_f_UINT8
#define GB_DEF_GxB_TIMES_INT16_MONOID_add GB_TIMES_f_INT16
#define GB_DEF_GxB_TIMES_UINT16_MONOID_add GB_TIMES_f_UINT16
#define GB_DEF_GxB_TIMES_INT32_MONOID_add GB_TIMES_f_INT32
#define GB_DEF_GxB_TIMES_UINT32_MONOID_add GB_TIMES_f_UINT32
#define GB_DEF_GxB_TIMES_INT64_MONOID_add GB_TIMES_f_INT64
#define GB_DEF_GxB_TIMES_UINT64_MONOID_add GB_TIMES_f_UINT64
#define GB_DEF_GxB_TIMES_FP32_MONOID_add GB_TIMES_f_FP32
#define GB_DEF_GxB_TIMES_FP64_MONOID_add GB_TIMES_f_FP64

// op: Boolean
#define GB_DEF_GxB_LOR_BOOL_MONOID_add   GB_LOR_f_BOOL
#define GB_DEF_GxB_LAND_BOOL_MONOID_add  GB_LAND_f_BOOL
#define GB_DEF_GxB_LXOR_BOOL_MONOID_add  GB_LXOR_f_BOOL
#define GB_DEF_GxB_EQ_BOOL_MONOID_add    GB_EQ_f_BOOL

// monoid identity values
#define GB_DEF_GxB_MIN_INT8_MONOID_identity   INT8_MAX
#define GB_DEF_GxB_MIN_UINT8_MONOID_identity  UINT8_MAX
#define GB_DEF_GxB_MIN_INT16_MONOID_identity  INT16_MAX
#define GB_DEF_GxB_MIN_UINT16_MONOID_identity UINT16_MAX
#define GB_DEF_GxB_MIN_INT32_MONOID_identity  INT32_MAX
#define GB_DEF_GxB_MIN_UINT32_MONOID_identity UINT32_MAX
#define GB_DEF_GxB_MIN_INT64_MONOID_identity  INT64_MAX
#define GB_DEF_GxB_MIN_UINT64_MONOID_identity UINT64_MAX
#define GB_DEF_GxB_MIN_FP32_MONOID_identity   INFINITY
#define GB_DEF_GxB_MIN_FP64_MONOID_identity   INFINITY

#define GB_DEF_GxB_MAX_INT8_MONOID_identity   INT8_MIN
#define GB_DEF_GxB_MAX_UINT8_MONOID_identity  0
#define GB_DEF_GxB_MAX_INT16_MONOID_identity  INT16_MIN
#define GB_DEF_GxB_MAX_UINT16_MONOID_identity 0
#define GB_DEF_GxB_MAX_INT32_MONOID_identity  INT32_MIN
#define GB_DEF_GxB_MAX_UINT32_MONOID_identity 0
#define GB_DEF_GxB_MAX_INT64_MONOID_identity  INT64_MIN
#define GB_DEF_GxB_MAX_UINT64_MONOID_identity 0
#define GB_DEF_GxB_MAX_FP32_MONOID_identity   (-INFINITY)
#define GB_DEF_GxB_MAX_FP64_MONOID_identity   (-INFINITY)

#define GB_DEF_GxB_PLUS_INT8_MONOID_identity   0
#define GB_DEF_GxB_PLUS_UINT8_MONOID_identity  0
#define GB_DEF_GxB_PLUS_INT16_MONOID_identity  0
#define GB_DEF_GxB_PLUS_UINT16_MONOID_identity 0
#define GB_DEF_GxB_PLUS_INT32_MONOID_identity  0
#define GB_DEF_GxB_PLUS_UINT32_MONOID_identity 0
#define GB_DEF_GxB_PLUS_INT64_MONOID_identity  0
#define GB_DEF_GxB_PLUS_UINT64_MONOID_identity 0
#define GB_DEF_GxB_PLUS_FP32_MONOID_identity   0
#define GB_DEF_GxB_PLUS_FP64_MONOID_identity   0

#define GB_DEF_GxB_TIMES_INT8_MONOID_identity   1
#define GB_DEF_GxB_TIMES_UINT8_MONOID_identity  1
#define GB_DEF_GxB_TIMES_INT16_MONOID_identity  1
#define GB_DEF_GxB_TIMES_UINT16_MONOID_identity 1
#define GB_DEF_GxB_TIMES_INT32_MONOID_identity  1
#define GB_DEF_GxB_TIMES_UINT32_MONOID_identity 1
#define GB_DEF_GxB_TIMES_INT64_MONOID_identity  1
#define GB_DEF_GxB_TIMES_UINT64_MONOID_identity 1
#define GB_DEF_GxB_TIMES_FP32_MONOID_identity   1
#define GB_DEF_GxB_TIMES_FP64_MONOID_identity   1

#define GB_DEF_GxB_LOR_BOOL_MONOID_identity    false
#define GB_DEF_GxB_LAND_BOOL_MONOID_identity   true
#define GB_DEF_GxB_LXOR_BOOL_MONOID_identity   false
#define GB_DEF_GxB_EQ_BOOL_MONOID_identity     true

// monoid terminal values
#define GB_DEF_GxB_MIN_INT8_MONOID_terminal   INT8_MIN
#define GB_DEF_GxB_MIN_UINT8_MONOID_terminal  0
#define GB_DEF_GxB_MIN_INT16_MONOID_terminal  INT16_MIN
#define GB_DEF_GxB_MIN_UINT16_MONOID_terminal 0
#define GB_DEF_GxB_MIN_INT32_MONOID_terminal  INT32_MIN
#define GB_DEF_GxB_MIN_UINT32_MONOID_terminal 0
#define GB_DEF_GxB_MIN_INT64_MONOID_terminal  INT64_MIN
#define GB_DEF_GxB_MIN_UINT64_MONOID_terminal 0
#define GB_DEF_GxB_MIN_FP32_MONOID_terminal   (-INFINITY)
#define GB_DEF_GxB_MIN_FP64_MONOID_terminal   (-INFINITY)

#define GB_DEF_GxB_MAX_INT8_MONOID_terminal   INT8_MAX
#define GB_DEF_GxB_MAX_UINT8_MONOID_terminal  UINT8_MAX
#define GB_DEF_GxB_MAX_INT16_MONOID_terminal  INT16_MAX
#define GB_DEF_GxB_MAX_UINT16_MONOID_terminal UINT16_MAX
#define GB_DEF_GxB_MAX_INT32_MONOID_terminal  INT32_MAX
#define GB_DEF_GxB_MAX_UINT32_MONOID_terminal UINT32_MAX
#define GB_DEF_GxB_MAX_INT64_MONOID_terminal  INT64_MAX
#define GB_DEF_GxB_MAX_UINT64_MONOID_terminal UINT64_MAX
#define GB_DEF_GxB_MAX_FP32_MONOID_terminal   INFINITY
#define GB_DEF_GxB_MAX_FP64_MONOID_terminal   INFINITY

// no #define GB_DEF_GxB_PLUS_INT8_MONOID_terminal
// no #define GB_DEF_GxB_PLUS_UINT8_MONOID_terminal
// no #define GB_DEF_GxB_PLUS_INT16_MONOID_terminal
// no #define GB_DEF_GxB_PLUS_UINT16_MONOID_terminal
// no #define GB_DEF_GxB_PLUS_INT32_MONOID_terminal
// no #define GB_DEF_GxB_PLUS_UINT32_MONOID_terminal
// no #define GB_DEF_GxB_PLUS_INT64_MONOID_terminal
// no #define GB_DEF_GxB_PLUS_UINT64_MONOID_terminal
// no #define GB_DEF_GxB_PLUS_FP32_MONOID_terminal
// no #define GB_DEF_GxB_PLUS_FP64_MONOID_terminal

#define GB_DEF_GxB_TIMES_INT8_MONOID_terminal   0
#define GB_DEF_GxB_TIMES_UINT8_MONOID_terminal  0
#define GB_DEF_GxB_TIMES_INT16_MONOID_terminal  0
#define GB_DEF_GxB_TIMES_UINT16_MONOID_terminal 0
#define GB_DEF_GxB_TIMES_INT32_MONOID_terminal  0
#define GB_DEF_GxB_TIMES_UINT32_MONOID_terminal 0
#define GB_DEF_GxB_TIMES_INT64_MONOID_terminal  0
#define GB_DEF_GxB_TIMES_UINT64_MONOID_terminal 0
// no #define GB_DEF_GxB_TIMES_FP32_MONOID_terminal
// no #define GB_DEF_GxB_TIMES_FP64_MONOID_terminal

#define GB_DEF_GxB_LOR_BOOL_MONOID_terminal    true
#define GB_DEF_GxB_LAND_BOOL_MONOID_terminal   false
// no #define GB_DEF_GxB_LXOR_BOOL_MONOID_terminal
// no #define GB_DEF_GxB_EQ_BOOL_MONOID_terminal

#endif

