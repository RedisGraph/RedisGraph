//------------------------------------------------------------------------------
// GB_AxB_compare_factory.c: switch factory for C=A*B with comparator ops
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// A template file #include'd in GB_AxB_factory.c

// The multiply operator is a comparator: EQ, NE, GT, LT, GE, LE.
// z=f(x,y): x and x are either boolean or non-boolean.  z is boolean.

// Since z is boolean, the only monoids available are OR, AND, XOR, and EQ
// All the other four (max==plus==or, min==times==and) are redundant.
// Those opcodes have been renamed, and handled by the OR and AND workers
// defined here.

// There is one special case to consider.  For boolean x, y, and z, the
// function z=NE(x,y) is the same as z=XOR(x,y).  If z is boolean, the multiply
// operator NE has already been renamed XOR by GB_AxB_semiring_builtin, and
// thus NE will never use the boolean case, below.  Thus it is removed with the
// #ifndef GB_NO_BOOLEAN.

ASSERT (zcode == GB_BOOL_code) ;
{

    // C = A*B where C is boolean, but A and B are non-boolean.
    // The result of the compare(A,B) operation is boolean.
    // There are 4 monoids available: OR, AND, XOR, EQ

    switch (add_opcode)
    {

        case GB_LOR_opcode     :

            switch (xycode)
            {
                #ifndef GB_NO_BOOLEAN
                case GB_BOOL_code   : GB_AxB_WORKER (_lor, GB_MULT_NAME, _bool  )
                #endif
                case GB_INT8_code   : GB_AxB_WORKER (_lor, GB_MULT_NAME, _int8  )
                case GB_UINT8_code  : GB_AxB_WORKER (_lor, GB_MULT_NAME, _uint8 )
                case GB_INT16_code  : GB_AxB_WORKER (_lor, GB_MULT_NAME, _int16 )
                case GB_UINT16_code : GB_AxB_WORKER (_lor, GB_MULT_NAME, _uint16)
                case GB_INT32_code  : GB_AxB_WORKER (_lor, GB_MULT_NAME, _int32 )
                case GB_UINT32_code : GB_AxB_WORKER (_lor, GB_MULT_NAME, _uint32)
                case GB_INT64_code  : GB_AxB_WORKER (_lor, GB_MULT_NAME, _int64 )
                case GB_UINT64_code : GB_AxB_WORKER (_lor, GB_MULT_NAME, _uint64)
                case GB_FP32_code   : GB_AxB_WORKER (_lor, GB_MULT_NAME, _fp32  )
                case GB_FP64_code   : GB_AxB_WORKER (_lor, GB_MULT_NAME, _fp64  )
                default: ;
            }
            break ;

        case GB_LAND_opcode    :

            switch (xycode)
            {
                #ifndef GB_NO_BOOLEAN
                case GB_BOOL_code   : GB_AxB_WORKER (_land, GB_MULT_NAME, _bool  )
                #endif
                case GB_INT8_code   : GB_AxB_WORKER (_land, GB_MULT_NAME, _int8  )
                case GB_UINT8_code  : GB_AxB_WORKER (_land, GB_MULT_NAME, _uint8 )
                case GB_INT16_code  : GB_AxB_WORKER (_land, GB_MULT_NAME, _int16 )
                case GB_UINT16_code : GB_AxB_WORKER (_land, GB_MULT_NAME, _uint16)
                case GB_INT32_code  : GB_AxB_WORKER (_land, GB_MULT_NAME, _int32 )
                case GB_UINT32_code : GB_AxB_WORKER (_land, GB_MULT_NAME, _uint32)
                case GB_INT64_code  : GB_AxB_WORKER (_land, GB_MULT_NAME, _int64 )
                case GB_UINT64_code : GB_AxB_WORKER (_land, GB_MULT_NAME, _uint64)
                case GB_FP32_code   : GB_AxB_WORKER (_land, GB_MULT_NAME, _fp32  )
                case GB_FP64_code   : GB_AxB_WORKER (_land, GB_MULT_NAME, _fp64  )
                default: ;
            }
            break ;

        case GB_LXOR_opcode    :

            switch (xycode)
            {
                #ifndef GB_NO_BOOLEAN
                case GB_BOOL_code   : GB_AxB_WORKER (_lxor, GB_MULT_NAME, _bool  )
                #endif
                case GB_INT8_code   : GB_AxB_WORKER (_lxor, GB_MULT_NAME, _int8  )
                case GB_UINT8_code  : GB_AxB_WORKER (_lxor, GB_MULT_NAME, _uint8 )
                case GB_INT16_code  : GB_AxB_WORKER (_lxor, GB_MULT_NAME, _int16 )
                case GB_UINT16_code : GB_AxB_WORKER (_lxor, GB_MULT_NAME, _uint16)
                case GB_INT32_code  : GB_AxB_WORKER (_lxor, GB_MULT_NAME, _int32 )
                case GB_UINT32_code : GB_AxB_WORKER (_lxor, GB_MULT_NAME, _uint32)
                case GB_INT64_code  : GB_AxB_WORKER (_lxor, GB_MULT_NAME, _int64 )
                case GB_UINT64_code : GB_AxB_WORKER (_lxor, GB_MULT_NAME, _uint64)
                case GB_FP32_code   : GB_AxB_WORKER (_lxor, GB_MULT_NAME, _fp32  )
                case GB_FP64_code   : GB_AxB_WORKER (_lxor, GB_MULT_NAME, _fp64  )
                default: ;
            }
            break ;

        case GB_EQ_opcode    :

            switch (xycode)
            {
                #ifndef GB_NO_BOOLEAN
                case GB_BOOL_code   : GB_AxB_WORKER (_eq, GB_MULT_NAME, _bool  )
                #endif
                case GB_INT8_code   : GB_AxB_WORKER (_eq, GB_MULT_NAME, _int8  )
                case GB_UINT8_code  : GB_AxB_WORKER (_eq, GB_MULT_NAME, _uint8 )
                case GB_INT16_code  : GB_AxB_WORKER (_eq, GB_MULT_NAME, _int16 )
                case GB_UINT16_code : GB_AxB_WORKER (_eq, GB_MULT_NAME, _uint16)
                case GB_INT32_code  : GB_AxB_WORKER (_eq, GB_MULT_NAME, _int32 )
                case GB_UINT32_code : GB_AxB_WORKER (_eq, GB_MULT_NAME, _uint32)
                case GB_INT64_code  : GB_AxB_WORKER (_eq, GB_MULT_NAME, _int64 )
                case GB_UINT64_code : GB_AxB_WORKER (_eq, GB_MULT_NAME, _uint64)
                case GB_FP32_code   : GB_AxB_WORKER (_eq, GB_MULT_NAME, _fp32  )
                case GB_FP64_code   : GB_AxB_WORKER (_eq, GB_MULT_NAME, _fp64  )
                default: ;
            }
            break ;

        case GB_ANY_opcode    :

            switch (xycode)
            {
                #ifndef GB_NO_BOOLEAN
                case GB_BOOL_code   : GB_AxB_WORKER (_any, GB_MULT_NAME, _bool  )
                #endif
                case GB_INT8_code   : GB_AxB_WORKER (_any, GB_MULT_NAME, _int8  )
                case GB_UINT8_code  : GB_AxB_WORKER (_any, GB_MULT_NAME, _uint8 )
                case GB_INT16_code  : GB_AxB_WORKER (_any, GB_MULT_NAME, _int16 )
                case GB_UINT16_code : GB_AxB_WORKER (_any, GB_MULT_NAME, _uint16)
                case GB_INT32_code  : GB_AxB_WORKER (_any, GB_MULT_NAME, _int32 )
                case GB_UINT32_code : GB_AxB_WORKER (_any, GB_MULT_NAME, _uint32)
                case GB_INT64_code  : GB_AxB_WORKER (_any, GB_MULT_NAME, _int64 )
                case GB_UINT64_code : GB_AxB_WORKER (_any, GB_MULT_NAME, _uint64)
                case GB_FP32_code   : GB_AxB_WORKER (_any, GB_MULT_NAME, _fp32  )
                case GB_FP64_code   : GB_AxB_WORKER (_any, GB_MULT_NAME, _fp64  )
                default: ;
            }
            break ;

        default: ;
    }
}

#undef GB_NO_BOOLEAN
#undef GB_MULT_NAME

