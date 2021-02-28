//------------------------------------------------------------------------------
// GB_AxB_type_factory.c: switch factory for C=A*B
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// A template file #include'd in GB_AxB_factory.c

// GB_NO_BOOLEAN is defined for multiply operators in the #include'ing file
// (min, max, plus, minus, rminus, times, div, rdiv, is*) since those multiply
// operators are redundant and have been renamed.  For these, the boolean
// monoids are not needed.

// For the PAIR multiply operator, the monoids MIN, MAX, TIMES, EQ, LAND, 
// and LOR have been renamed to ANY_PAIR.  See GB_AxB_semiring_builtin.c.

ASSERT (zcode == xycode) ;

if (zcode != GB_BOOL_code)
{
    switch (add_opcode)
    {

        // MIN_PAIR, MAX_PAIR, and TIMES_PAIR have been renamed to ANY_PAIR
        #ifndef GB_MULT_IS_PAIR_OPERATOR

        case GB_MIN_opcode:

            switch (zcode)
            {
                case GB_INT8_code   : GB_AxB_WORKER (_min, GB_MULT_NAME, _int8  )
                case GB_UINT8_code  : GB_AxB_WORKER (_min, GB_MULT_NAME, _uint8 )
                case GB_INT16_code  : GB_AxB_WORKER (_min, GB_MULT_NAME, _int16 )
                case GB_UINT16_code : GB_AxB_WORKER (_min, GB_MULT_NAME, _uint16)
                case GB_INT32_code  : GB_AxB_WORKER (_min, GB_MULT_NAME, _int32 )
                case GB_UINT32_code : GB_AxB_WORKER (_min, GB_MULT_NAME, _uint32)
                case GB_INT64_code  : GB_AxB_WORKER (_min, GB_MULT_NAME, _int64 )
                case GB_UINT64_code : GB_AxB_WORKER (_min, GB_MULT_NAME, _uint64)
                case GB_FP32_code   : GB_AxB_WORKER (_min, GB_MULT_NAME, _fp32  )
                case GB_FP64_code   : GB_AxB_WORKER (_min, GB_MULT_NAME, _fp64  )
                default: ;
            }
            break ;

        case GB_MAX_opcode:

            switch (zcode)
            {
                case GB_INT8_code   : GB_AxB_WORKER (_max, GB_MULT_NAME, _int8  )
                case GB_UINT8_code  : GB_AxB_WORKER (_max, GB_MULT_NAME, _uint8 )
                case GB_INT16_code  : GB_AxB_WORKER (_max, GB_MULT_NAME, _int16 )
                case GB_UINT16_code : GB_AxB_WORKER (_max, GB_MULT_NAME, _uint16)
                case GB_INT32_code  : GB_AxB_WORKER (_max, GB_MULT_NAME, _int32 )
                case GB_UINT32_code : GB_AxB_WORKER (_max, GB_MULT_NAME, _uint32)
                case GB_INT64_code  : GB_AxB_WORKER (_max, GB_MULT_NAME, _int64 )
                case GB_UINT64_code : GB_AxB_WORKER (_max, GB_MULT_NAME, _uint64)
                case GB_FP32_code   : GB_AxB_WORKER (_max, GB_MULT_NAME, _fp32  )
                case GB_FP64_code   : GB_AxB_WORKER (_max, GB_MULT_NAME, _fp64  )
                default: ;
            }
            break ;

        case GB_TIMES_opcode:

            switch (zcode)
            {
                case GB_INT8_code   : GB_AxB_WORKER (_times, GB_MULT_NAME, _int8  )
                case GB_UINT8_code  : GB_AxB_WORKER (_times, GB_MULT_NAME, _uint8 )
                case GB_INT16_code  : GB_AxB_WORKER (_times, GB_MULT_NAME, _int16 )
                case GB_UINT16_code : GB_AxB_WORKER (_times, GB_MULT_NAME, _uint16)
                case GB_INT32_code  : GB_AxB_WORKER (_times, GB_MULT_NAME, _int32 )
                case GB_UINT32_code : GB_AxB_WORKER (_times, GB_MULT_NAME, _uint32)
                case GB_INT64_code  : GB_AxB_WORKER (_times, GB_MULT_NAME, _int64 )
                case GB_UINT64_code : GB_AxB_WORKER (_times, GB_MULT_NAME, _uint64)
                case GB_FP32_code   : GB_AxB_WORKER (_times, GB_MULT_NAME, _fp32  )
                case GB_FP64_code   : GB_AxB_WORKER (_times, GB_MULT_NAME, _fp64  )
                default: ;
            }
            break ;

        #endif

        case GB_PLUS_opcode:

            switch (zcode)
            {
                case GB_INT8_code   : GB_AxB_WORKER (_plus, GB_MULT_NAME, _int8  )
                case GB_UINT8_code  : GB_AxB_WORKER (_plus, GB_MULT_NAME, _uint8 )
                case GB_INT16_code  : GB_AxB_WORKER (_plus, GB_MULT_NAME, _int16 )
                case GB_UINT16_code : GB_AxB_WORKER (_plus, GB_MULT_NAME, _uint16)
                case GB_INT32_code  : GB_AxB_WORKER (_plus, GB_MULT_NAME, _int32 )
                case GB_UINT32_code : GB_AxB_WORKER (_plus, GB_MULT_NAME, _uint32)
                case GB_INT64_code  : GB_AxB_WORKER (_plus, GB_MULT_NAME, _int64 )
                case GB_UINT64_code : GB_AxB_WORKER (_plus, GB_MULT_NAME, _uint64)
                case GB_FP32_code   : GB_AxB_WORKER (_plus, GB_MULT_NAME, _fp32  )
                case GB_FP64_code   : GB_AxB_WORKER (_plus, GB_MULT_NAME, _fp64  )
                default: ;
            }
            break ;

        case GB_ANY_opcode:

            switch (zcode)
            {
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

#ifndef GB_NO_BOOLEAN
else
{
        switch (add_opcode)
        {
            // EQ_PAIR, LOR_PAIR, LAND_PAIR, been renamed to ANY_PAIR
            #ifndef GB_MULT_IS_PAIR_OPERATOR
            case GB_LOR_opcode  : GB_AxB_WORKER (_lor , GB_MULT_NAME, _bool)
            case GB_LAND_opcode : GB_AxB_WORKER (_land, GB_MULT_NAME, _bool)
            case GB_EQ_opcode   : GB_AxB_WORKER (_eq  , GB_MULT_NAME, _bool)
            #endif
            case GB_LXOR_opcode : GB_AxB_WORKER (_lxor, GB_MULT_NAME, _bool)
            case GB_ANY_opcode  : GB_AxB_WORKER (_any , GB_MULT_NAME, _bool)
            default: ;
        }
}
#endif

#undef GB_NO_BOOLEAN
#undef GB_MULT_NAME

