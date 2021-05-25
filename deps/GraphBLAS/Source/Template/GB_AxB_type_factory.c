//------------------------------------------------------------------------------
// GB_AxB_type_factory.c: switch factory for C=A*B
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// A template file #include'd in GB_AxB_factory.c, which calls up to 61
// semirings.  Not all multiplicative operators and types are used with every
// monoid.  The 2 complex types appear only in the times, plus, and any
// monoids, for a subset of the multiply operators.

//  min monoid:     10 real, non-boolean types
//  max monoid:     10 real, non-boolean types
//  times monoid:   10 real, non-boolean types (+2 if complex)
//  plus monoid:    10 real, non-boolean types (+2 if complex)
//  any monoid:     10 real, non-boolean types (+2 if complex)
//  boolean:        5 monoids: lor, land, eq, lxor, any

// GB_NO_BOOLEAN is defined for multiply operators in the #include'ing file
// (min, max, plus, minus, rminus, times, div, rdiv, is*) since those multiply
// operators are redundant and have been renamed.  For these, the boolean
// monoids are not needed.

// For the PAIR multiply operator, the monoids MIN, MAX, TIMES, EQ, LAND, 
// and LOR have been renamed to ANY_PAIR.  See GB_AxB_semiring_builtin.c.

// the additive operator is a monoid, where all types of x,y,z are the same
ASSERT (zcode == xcode) ;
ASSERT (zcode == ycode) ;
ASSERT (mult_opcode != GB_ANY_opcode) ;

if (xcode != GB_BOOL_code)
{
    switch (add_opcode)
    {

        // MIN_PAIR, MAX_PAIR, and TIMES_PAIR have been renamed to ANY_PAIR
        #ifndef GB_MULT_IS_PAIR_OPERATOR

        case GB_MIN_opcode:

            switch (xcode)
            {
                // 10 real, non-boolean types
                case GB_INT8_code   : GB_AxB_WORKER (_min, GB_MNAME, _int8  )
                case GB_INT16_code  : GB_AxB_WORKER (_min, GB_MNAME, _int16 )
                case GB_INT32_code  : GB_AxB_WORKER (_min, GB_MNAME, _int32 )
                case GB_INT64_code  : GB_AxB_WORKER (_min, GB_MNAME, _int64 )
                case GB_UINT8_code  : GB_AxB_WORKER (_min, GB_MNAME, _uint8 )
                case GB_UINT16_code : GB_AxB_WORKER (_min, GB_MNAME, _uint16)
                case GB_UINT32_code : GB_AxB_WORKER (_min, GB_MNAME, _uint32)
                case GB_UINT64_code : GB_AxB_WORKER (_min, GB_MNAME, _uint64)
                case GB_FP32_code   : GB_AxB_WORKER (_min, GB_MNAME, _fp32  )
                case GB_FP64_code   : GB_AxB_WORKER (_min, GB_MNAME, _fp64  )
                default: ;
            }
            break ;

        case GB_MAX_opcode:

            switch (xcode)
            {
                // 10 real, non-boolean types
                case GB_INT8_code   : GB_AxB_WORKER (_max, GB_MNAME, _int8  )
                case GB_INT16_code  : GB_AxB_WORKER (_max, GB_MNAME, _int16 )
                case GB_INT32_code  : GB_AxB_WORKER (_max, GB_MNAME, _int32 )
                case GB_INT64_code  : GB_AxB_WORKER (_max, GB_MNAME, _int64 )
                case GB_UINT8_code  : GB_AxB_WORKER (_max, GB_MNAME, _uint8 )
                case GB_UINT16_code : GB_AxB_WORKER (_max, GB_MNAME, _uint16)
                case GB_UINT32_code : GB_AxB_WORKER (_max, GB_MNAME, _uint32)
                case GB_UINT64_code : GB_AxB_WORKER (_max, GB_MNAME, _uint64)
                case GB_FP32_code   : GB_AxB_WORKER (_max, GB_MNAME, _fp32  )
                case GB_FP64_code   : GB_AxB_WORKER (_max, GB_MNAME, _fp64  )
                default: ;
            }
            break ;

        case GB_TIMES_opcode:

            switch (xcode)
            {
                // 10 real, non-boolean types, plus 2 complex
                case GB_INT8_code   : GB_AxB_WORKER (_times, GB_MNAME, _int8  )
                case GB_INT16_code  : GB_AxB_WORKER (_times, GB_MNAME, _int16 )
                case GB_INT32_code  : GB_AxB_WORKER (_times, GB_MNAME, _int32 )
                case GB_INT64_code  : GB_AxB_WORKER (_times, GB_MNAME, _int64 )
                case GB_UINT8_code  : GB_AxB_WORKER (_times, GB_MNAME, _uint8 )
                case GB_UINT16_code : GB_AxB_WORKER (_times, GB_MNAME, _uint16)
                case GB_UINT32_code : GB_AxB_WORKER (_times, GB_MNAME, _uint32)
                case GB_UINT64_code : GB_AxB_WORKER (_times, GB_MNAME, _uint64)
                case GB_FP32_code   : GB_AxB_WORKER (_times, GB_MNAME, _fp32  )
                case GB_FP64_code   : GB_AxB_WORKER (_times, GB_MNAME, _fp64  )
                #if defined ( GB_COMPLEX )
                case GB_FC32_code   : GB_AxB_WORKER (_times, GB_MNAME, _fc32  )
                case GB_FC64_code   : GB_AxB_WORKER (_times, GB_MNAME, _fc64  )
                #endif
                default: ;
            }
            break ;

        #endif

        case GB_PLUS_opcode:

            switch (xcode)
            {
                // 10 real, non-boolean types, plus 2 complex
                case GB_INT8_code   : GB_AxB_WORKER (_plus, GB_MNAME, _int8  )
                case GB_INT16_code  : GB_AxB_WORKER (_plus, GB_MNAME, _int16 )
                case GB_INT32_code  : GB_AxB_WORKER (_plus, GB_MNAME, _int32 )
                case GB_INT64_code  : GB_AxB_WORKER (_plus, GB_MNAME, _int64 )
                case GB_UINT8_code  : GB_AxB_WORKER (_plus, GB_MNAME, _uint8 )
                case GB_UINT16_code : GB_AxB_WORKER (_plus, GB_MNAME, _uint16)
                case GB_UINT32_code : GB_AxB_WORKER (_plus, GB_MNAME, _uint32)
                case GB_UINT64_code : GB_AxB_WORKER (_plus, GB_MNAME, _uint64)
                case GB_FP32_code   : GB_AxB_WORKER (_plus, GB_MNAME, _fp32  )
                case GB_FP64_code   : GB_AxB_WORKER (_plus, GB_MNAME, _fp64  )
                #if defined ( GB_COMPLEX )
                case GB_FC32_code   : GB_AxB_WORKER (_plus, GB_MNAME, _fc32  )
                case GB_FC64_code   : GB_AxB_WORKER (_plus, GB_MNAME, _fc64  )
                #endif
                default: ;
            }
            break ;

        case GB_ANY_opcode:

            switch (xcode)
            {
                // 10 real, non-boolean types, plus 2 complex
                case GB_INT8_code   : GB_AxB_WORKER (_any, GB_MNAME, _int8  )
                case GB_INT16_code  : GB_AxB_WORKER (_any, GB_MNAME, _int16 )
                case GB_INT32_code  : GB_AxB_WORKER (_any, GB_MNAME, _int32 )
                case GB_INT64_code  : GB_AxB_WORKER (_any, GB_MNAME, _int64 )
                case GB_UINT8_code  : GB_AxB_WORKER (_any, GB_MNAME, _uint8 )
                case GB_UINT16_code : GB_AxB_WORKER (_any, GB_MNAME, _uint16)
                case GB_UINT32_code : GB_AxB_WORKER (_any, GB_MNAME, _uint32)
                case GB_UINT64_code : GB_AxB_WORKER (_any, GB_MNAME, _uint64)
                case GB_FP32_code   : GB_AxB_WORKER (_any, GB_MNAME, _fp32  )
                case GB_FP64_code   : GB_AxB_WORKER (_any, GB_MNAME, _fp64  )
                #if defined ( GB_COMPLEX )
                case GB_FC32_code   : GB_AxB_WORKER (_any, GB_MNAME, _fc32  )
                case GB_FC64_code   : GB_AxB_WORKER (_any, GB_MNAME, _fc64  )
                #endif
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
            // 5 boolean monoids
            #ifndef GB_MULT_IS_PAIR_OPERATOR
            // EQ_PAIR, LOR_PAIR, LAND_PAIR, been renamed to ANY_PAIR
            case GB_LOR_opcode  : GB_AxB_WORKER (_lor , GB_MNAME, _bool)
            case GB_LAND_opcode : GB_AxB_WORKER (_land, GB_MNAME, _bool)
            case GB_EQ_opcode   : GB_AxB_WORKER (_eq  , GB_MNAME, _bool)
            #endif
            case GB_LXOR_opcode : GB_AxB_WORKER (_lxor, GB_MNAME, _bool)
            case GB_ANY_opcode  : GB_AxB_WORKER (_any , GB_MNAME, _bool)
            default: ;
        }
}
#endif

#undef GB_NO_BOOLEAN
#undef GB_MNAME
#undef GB_COMPLEX

