//------------------------------------------------------------------------------
// GB_binop_type_factory.c: switch factory for binary operators
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// A template file #include'd in GB_binop_factory.c

// This file is used for 19 operators.  The operator is combined here
// with 10 or 11 types to create 10 or 11 unique workers.

//      FIRST, SECOND, MIN, MAX, PLUS, MINUS, RMINUS, TIMES, DIV, RDIV,
//      ISEQ, ISNE, ISGT, ISLT, ISGE, ISLE,
//      LAND, LOR, LXOR.

// For all of them, the types of x, y, and z are the same.  There are 10
// non-boolean operators and 0 or 1 boolean operators defined here.

// GB_NO_BOOLEAN is defined for 15 of these multiply operators in the
// #include'ing file (min, max, plus, minus, rminus, times, div, rdiv, is*)
// since those multiply operators are redundant and have been renamed.  For
// these, the boolean operators are not needed.

{
    switch (xycode)
    {
        #ifndef GB_NO_BOOLEAN
        case GB_BOOL_code   : GB_BINOP_WORKER (GB_BINOP_NAME, _bool  )
        #endif
        case GB_INT8_code   : GB_BINOP_WORKER (GB_BINOP_NAME, _int8  )
        case GB_INT16_code  : GB_BINOP_WORKER (GB_BINOP_NAME, _int16 )
        case GB_INT32_code  : GB_BINOP_WORKER (GB_BINOP_NAME, _int32 )
        case GB_INT64_code  : GB_BINOP_WORKER (GB_BINOP_NAME, _int64 )
        case GB_UINT8_code  : GB_BINOP_WORKER (GB_BINOP_NAME, _uint8 )
        case GB_UINT16_code : GB_BINOP_WORKER (GB_BINOP_NAME, _uint16)
        case GB_UINT32_code : GB_BINOP_WORKER (GB_BINOP_NAME, _uint32)
        case GB_UINT64_code : GB_BINOP_WORKER (GB_BINOP_NAME, _uint64)
        case GB_FP32_code   : GB_BINOP_WORKER (GB_BINOP_NAME, _fp32  )
        case GB_FP64_code   : GB_BINOP_WORKER (GB_BINOP_NAME, _fp64  )
        default: ;
    }
    break ;
}

#undef GB_NO_BOOLEAN
#undef GB_BINOP_NAME

