//------------------------------------------------------------------------------
// GB_bitwise.c: declaring functions from GB_bitwise.h
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#include "GB.h"

// z = bitshift (x,k)
extern int8_t   GB_bitshift_int8   (int8_t   x, int8_t k) ;
extern int16_t  GB_bitshift_int16  (int16_t  x, int8_t k) ;
extern int32_t  GB_bitshift_int32  (int32_t  x, int8_t k) ;
extern int64_t  GB_bitshift_int64  (int64_t  x, int8_t k) ;
extern uint8_t  GB_bitshift_uint8  (uint8_t  x, int8_t k) ;
extern uint16_t GB_bitshift_uint16 (uint16_t x, int8_t k) ;
extern uint32_t GB_bitshift_uint32 (uint32_t x, int8_t k) ;
extern uint64_t GB_bitshift_uint64 (uint64_t x, int8_t k) ;

