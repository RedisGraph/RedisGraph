//------------------------------------------------------------------------------
// GB_AxB_template.c
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// A template file #include'd in GB_AxB_factory.c

// This file is used for 17 operators.  The multiply operator is combined here
// with 40 or 44 monoids to create 40 or 44 unique semiring workers.

//      FIRST, SECOND, MIN, MAX, PLUS, MINUS, TIMES, DIV,
//      ISEQ, ISNE, ISGT, ISLT, ISGE, ISLE,
//      LAND, LOR, LXOR.

// For all of them, the types of x, y, and z are the same.
// There are 40 non-boolean monoids and 4 boolean monoids defined here.

// GB_NO_BOOLEAN is defined for 12 of these multiply operators in the
// #include'ing file (min, max, plus, minus, times, div, is*) since those 12
// multiply operators are redundant and have been renamed.  For these 12, the
// boolean monoids are not needed.

ASSERT (zcode == xycode) ;

if (zcode != GB_BOOL_code)
{
    switch (add_opcode)
    {

        case GB_MIN_opcode:

            // w = min (w,t), identity is +inf
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

            // w = max (w,t), identity is -inf
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

        case GB_PLUS_opcode:

            // w += t, identity is 0
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

        case GB_TIMES_opcode:

            // w *= t, identity is 1
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

        default: ;
    }
}

#ifndef GB_NO_BOOLEAN
else
{
        switch (add_opcode)
        {
            case GB_LOR_opcode  : GB_AxB_WORKER (_lor , GB_MULT_NAME, _bool)
            case GB_LAND_opcode : GB_AxB_WORKER (_land, GB_MULT_NAME, _bool)
            case GB_LXOR_opcode : GB_AxB_WORKER (_lxor, GB_MULT_NAME, _bool)
            case GB_EQ_opcode   : GB_AxB_WORKER (_eq  , GB_MULT_NAME, _bool)
            default: ;
        }
}
#endif

#undef GB_NO_BOOLEAN
#undef GB_MULT_NAME

