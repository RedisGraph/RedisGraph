//------------------------------------------------------------------------------
// GxB_Scalar_extractElement: extract a single entry from a GxB_Scalar
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// Extract a single entry, x = s, typecasting from the type of s to the type of
// x, as needed.

// Returns GrB_SUCCESS if s is present, and sets x to its value.
// Returns GrB_NO_VALUE if s does not have an entry, and x is unmodified.

#include "GB.h"

#define GB_FREE_ALL ;
#define GB_WHERE_STRING "GxB_Scalar_extractElement (&x, s)"

#define GB_EXTRACT_ELEMENT GxB_Scalar_extractElement_BOOL
#define GB_XTYPE bool
#define GB_XCODE GB_BOOL_code
#include "GB_Scalar_extractElement.c"

#define GB_EXTRACT_ELEMENT GxB_Scalar_extractElement_INT8
#define GB_XTYPE int8_t
#define GB_XCODE GB_INT8_code
#include "GB_Scalar_extractElement.c"

#define GB_EXTRACT_ELEMENT GxB_Scalar_extractElement_INT16
#define GB_XTYPE int16_t
#define GB_XCODE GB_INT16_code
#include "GB_Scalar_extractElement.c"

#define GB_EXTRACT_ELEMENT GxB_Scalar_extractElement_INT32
#define GB_XTYPE int32_t
#define GB_XCODE GB_INT32_code
#include "GB_Scalar_extractElement.c"

#define GB_EXTRACT_ELEMENT GxB_Scalar_extractElement_INT64
#define GB_XTYPE int64_t
#define GB_XCODE GB_INT64_code
#include "GB_Scalar_extractElement.c"

#define GB_EXTRACT_ELEMENT GxB_Scalar_extractElement_UINT8
#define GB_XTYPE uint8_t
#define GB_XCODE GB_UINT8_code
#include "GB_Scalar_extractElement.c"

#define GB_EXTRACT_ELEMENT GxB_Scalar_extractElement_UINT16
#define GB_XTYPE uint16_t
#define GB_XCODE GB_UINT16_code
#include "GB_Scalar_extractElement.c"

#define GB_EXTRACT_ELEMENT GxB_Scalar_extractElement_UINT32
#define GB_XTYPE uint32_t
#define GB_XCODE GB_UINT32_code
#include "GB_Scalar_extractElement.c"

#define GB_EXTRACT_ELEMENT GxB_Scalar_extractElement_UINT64
#define GB_XTYPE uint64_t
#define GB_XCODE GB_UINT64_code
#include "GB_Scalar_extractElement.c"

#define GB_EXTRACT_ELEMENT GxB_Scalar_extractElement_FP32
#define GB_XTYPE float
#define GB_XCODE GB_FP32_code
#include "GB_Scalar_extractElement.c"

#define GB_EXTRACT_ELEMENT GxB_Scalar_extractElement_FP64
#define GB_XTYPE double
#define GB_XCODE GB_FP64_code
#include "GB_Scalar_extractElement.c"

#define GB_EXTRACT_ELEMENT GxB_Scalar_extractElement_FC32
#define GB_XTYPE GxB_FC32_t
#define GB_XCODE GB_FC32_code
#include "GB_Scalar_extractElement.c"

#define GB_EXTRACT_ELEMENT GxB_Scalar_extractElement_FC64
#define GB_XTYPE GxB_FC64_t
#define GB_XCODE GB_FC64_code
#include "GB_Scalar_extractElement.c"

#define GB_UDT_EXTRACT
#define GB_EXTRACT_ELEMENT GxB_Scalar_extractElement_UDT
#define GB_XTYPE void
#define GB_XCODE GB_UDT_code
#include "GB_Scalar_extractElement.c"

