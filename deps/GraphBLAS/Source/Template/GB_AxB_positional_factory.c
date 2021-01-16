//------------------------------------------------------------------------------
// GB_AxB_positional_factory.c: switch factory for C=A*B for positional ops
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// the additive operator is a monoid, where all types of x,y,z are int64_t or
// int32_t

ASSERT (xcode == zcode) ;
ASSERT (ycode == zcode) ;
ASSERT (GB_OPCODE_IS_POSITIONAL (mult_opcode)) ;

{
    if (zcode == GB_INT32_code)
    {
        switch (add_opcode)
        {
            case GB_MIN_opcode   : GB_AxB_WORKER (_min,   GB_MNAME, _int32)
            case GB_MAX_opcode   : GB_AxB_WORKER (_max,   GB_MNAME, _int32)
            case GB_TIMES_opcode : GB_AxB_WORKER (_times, GB_MNAME, _int32)
            case GB_PLUS_opcode  : GB_AxB_WORKER (_plus,  GB_MNAME, _int32)
            case GB_ANY_opcode   : GB_AxB_WORKER (_any,   GB_MNAME, _int32)
            default: ;
        }
    }
    else // zcode == GB_INT64_code
    {
        ASSERT (zcode == GB_INT64_code) ;
        switch (add_opcode)
        {
            case GB_MIN_opcode   : GB_AxB_WORKER (_min,   GB_MNAME, _int64)
            case GB_MAX_opcode   : GB_AxB_WORKER (_max,   GB_MNAME, _int64)
            case GB_TIMES_opcode : GB_AxB_WORKER (_times, GB_MNAME, _int64)
            case GB_PLUS_opcode  : GB_AxB_WORKER (_plus,  GB_MNAME, _int64)
            case GB_ANY_opcode   : GB_AxB_WORKER (_any,   GB_MNAME, _int64)
            default: ;
        }
    }
}

#undef GB_MNAME

