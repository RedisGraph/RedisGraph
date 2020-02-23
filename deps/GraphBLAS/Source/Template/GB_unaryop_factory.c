//------------------------------------------------------------------------------
// GB_unaryop_factory.c:  switch factory for unary operators and 2 types
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Switch factory for applying a unary operator, where the input and output
// types differ (the worker does the typecasting as well).  This file is
// #include'd into GB_apply_op.c and GB_transpose_op.c, which must define
// the GrB_UnaryOp op and the GrB_Type Atype.

{

    // switch factory for two types, controlled by code1 and code2
    GB_Type_code code1 = op->ztype->code ;      // defines ztype
    GB_Type_code code2 = Atype->code ;          // defines the type of A

    ASSERT (code1 <= GB_UDT_code) ;
    ASSERT (code2 <= GB_UDT_code) ;

    switch (op->opcode)
    {

        case GB_ONE_opcode :       // z = 1

            switch (code1)
            {
                // A is not accessed
                case GB_BOOL_code   : GB_WORKER (_one, _bool,   bool,     _bool,   bool)
                case GB_INT8_code   : GB_WORKER (_one, _int8,   int8_t,   _int8,   int8_t)
                case GB_INT16_code  : GB_WORKER (_one, _int16,  int16_t,  _int16,  int16_t)
                case GB_INT32_code  : GB_WORKER (_one, _int32,  int32_t,  _int32,  int32_t)
                case GB_INT64_code  : GB_WORKER (_one, _int64,  int64_t,  _int64,  int64_t)
                case GB_UINT8_code  : GB_WORKER (_one, _uint8,  uint8_t,  _uint8,  uint8_t)
                case GB_UINT16_code : GB_WORKER (_one, _uint16, uint16_t, _uint16, uint16_t)
                case GB_UINT32_code : GB_WORKER (_one, _uint32, uint32_t, _uint32, uint32_t)
                case GB_UINT64_code : GB_WORKER (_one, _uint64, uint64_t, _uint64, uint64_t)
                case GB_FP32_code   : GB_WORKER (_one, _fp32,   float,    _fp32,   float)
                case GB_FP64_code   : GB_WORKER (_one, _fp64,   double,   _fp64,   double)
                default: ;
                break ;
            }
            break ;

        case GB_IDENTITY_opcode :  // z = x

            #define GB_OPNAME _identity
            #include "GB_2type_factory.c"
            break ;

        case GB_AINV_opcode :      // z = -x

            #define GB_OPNAME _ainv
            #include "GB_2type_factory.c"
            break ;

        case GB_ABS_opcode :       // z = abs(x)

            #define GB_OPNAME _abs
            #include "GB_2type_factory.c"
            break ;

        case GB_MINV_opcode :      // z = 1/x

            #define GB_OPNAME _minv
            #include "GB_2type_factory.c"
            break ;

        case GB_LNOT_opcode :      // z = ! (x != 0)

            #define GB_OPNAME _lnot
            #include "GB_2type_factory.c"
            break ;

        default: ;
    }
}

