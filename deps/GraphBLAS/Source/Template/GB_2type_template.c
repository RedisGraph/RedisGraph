//------------------------------------------------------------------------------
// GB_2type_template.c: 2-type switch factory
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// This is a generic switch factory for creating 121 workers that operate on
// two built-in data types (11 types each), to be #include'd in another file.
// GB_WORKER(type1,type2) is a macro defined in the #including file, where
// type1 and type2 are the built-in types corresponding to code1 and code2,
// respectively or (void *) for a user-defined type.  The last statement of
// GB_WORKER should be a break or return since it doesn't appear here.

// User-defined types are not handled.

// GB_shallow_op and GB_transpose_op use this template to create workers that
// apply unary operators.  Those functions #define GB_BOP(x) for the boolean
// unary operator, GB_IOP(x) for integers, and GB_FOP(x) for floating-point.
// The selection of these operators is controlled by code1.

switch (code1)
{
    case GB_BOOL_code   :

        #define GB_OP(x) GB_BOP(x)
        switch (code2)
        {
            //                            code1 code2
            #ifndef GB_NOT_SAME
            case GB_BOOL_code   : GB_WORKER (bool, bool)
            #endif
            case GB_INT8_code   : GB_WORKER (bool, int8_t)
            case GB_UINT8_code  : GB_WORKER (bool, uint8_t)
            case GB_INT16_code  : GB_WORKER (bool, int16_t)
            case GB_UINT16_code : GB_WORKER (bool, uint16_t)
            case GB_INT32_code  : GB_WORKER (bool, int32_t)
            case GB_UINT32_code : GB_WORKER (bool, uint32_t)
            case GB_INT64_code  : GB_WORKER (bool, int64_t)
            case GB_UINT64_code : GB_WORKER (bool, uint64_t)
            case GB_FP32_code   : GB_WORKER (bool, float)
            case GB_FP64_code   : GB_WORKER (bool, double)
            default: ;
        }
        break ;
        #undef  GB_OP

    case GB_INT8_code   :

        #define GB_OP(x) GB_IOP(x)
        switch (code2)
        {
            //                            code1   code2
            case GB_BOOL_code   : GB_WORKER (int8_t, bool)
            #ifndef GB_NOT_SAME
            case GB_INT8_code   : GB_WORKER (int8_t, int8_t)
            #endif
            case GB_UINT8_code  : GB_WORKER (int8_t, uint8_t)
            case GB_INT16_code  : GB_WORKER (int8_t, int16_t)
            case GB_UINT16_code : GB_WORKER (int8_t, uint16_t)
            case GB_INT32_code  : GB_WORKER (int8_t, int32_t)
            case GB_UINT32_code : GB_WORKER (int8_t, uint32_t)
            case GB_INT64_code  : GB_WORKER (int8_t, int64_t)
            case GB_UINT64_code : GB_WORKER (int8_t, uint64_t)
            case GB_FP32_code   : GB_WORKER (int8_t, float)
            case GB_FP64_code   : GB_WORKER (int8_t, double)
            default: ;
        }
        break ;

    case GB_UINT8_code  :

        switch (code2)
        {
            //                            code1    code2
            case GB_BOOL_code   : GB_WORKER (uint8_t, bool)
            case GB_INT8_code   : GB_WORKER (uint8_t, int8_t)
            #ifndef GB_NOT_SAME
            case GB_UINT8_code  : GB_WORKER (uint8_t, uint8_t)
            #endif
            case GB_INT16_code  : GB_WORKER (uint8_t, int16_t)
            case GB_UINT16_code : GB_WORKER (uint8_t, uint16_t)
            case GB_INT32_code  : GB_WORKER (uint8_t, int32_t)
            case GB_UINT32_code : GB_WORKER (uint8_t, uint32_t)
            case GB_INT64_code  : GB_WORKER (uint8_t, int64_t)
            case GB_UINT64_code : GB_WORKER (uint8_t, uint64_t)
            case GB_FP32_code   : GB_WORKER (uint8_t, float)
            case GB_FP64_code   : GB_WORKER (uint8_t, double)
            default: ;
        }
        break ;

    case GB_INT16_code  :

        switch (code2)
        {
            //                            code1    code2
            case GB_BOOL_code   : GB_WORKER (int16_t, bool)
            case GB_INT8_code   : GB_WORKER (int16_t, int8_t)
            case GB_UINT8_code  : GB_WORKER (int16_t, uint8_t)
            #ifndef GB_NOT_SAME
            case GB_INT16_code  : GB_WORKER (int16_t, int16_t)
            #endif
            case GB_UINT16_code : GB_WORKER (int16_t, uint16_t)
            case GB_INT32_code  : GB_WORKER (int16_t, int32_t)
            case GB_UINT32_code : GB_WORKER (int16_t, uint32_t)
            case GB_INT64_code  : GB_WORKER (int16_t, int64_t)
            case GB_UINT64_code : GB_WORKER (int16_t, uint64_t)
            case GB_FP32_code   : GB_WORKER (int16_t, float)
            case GB_FP64_code   : GB_WORKER (int16_t, double)
            default: ;
        }
        break ;

    case GB_UINT16_code :

        switch (code2)
        {
            //                            code1     code2
            case GB_BOOL_code   : GB_WORKER (uint16_t, bool)
            case GB_INT8_code   : GB_WORKER (uint16_t, int8_t)
            case GB_UINT8_code  : GB_WORKER (uint16_t, uint8_t)
            case GB_INT16_code  : GB_WORKER (uint16_t, int16_t)
            #ifndef GB_NOT_SAME
            case GB_UINT16_code : GB_WORKER (uint16_t, uint16_t)
            #endif
            case GB_INT32_code  : GB_WORKER (uint16_t, int32_t)
            case GB_UINT32_code : GB_WORKER (uint16_t, uint32_t)
            case GB_INT64_code  : GB_WORKER (uint16_t, int64_t)
            case GB_UINT64_code : GB_WORKER (uint16_t, uint64_t)
            case GB_FP32_code   : GB_WORKER (uint16_t, float)
            case GB_FP64_code   : GB_WORKER (uint16_t, double)
            default: ;
        }
        break ;

    case GB_INT32_code  :

        switch (code2)
        {
            //                            code1    code2
            case GB_BOOL_code   : GB_WORKER (int32_t, bool)
            case GB_INT8_code   : GB_WORKER (int32_t, int8_t)
            case GB_UINT8_code  : GB_WORKER (int32_t, uint8_t)
            case GB_INT16_code  : GB_WORKER (int32_t, int16_t)
            case GB_UINT16_code : GB_WORKER (int32_t, uint16_t)
            #ifndef GB_NOT_SAME
            case GB_INT32_code  : GB_WORKER (int32_t, int32_t)
            #endif
            case GB_UINT32_code : GB_WORKER (int32_t, uint32_t)
            case GB_INT64_code  : GB_WORKER (int32_t, int64_t)
            case GB_UINT64_code : GB_WORKER (int32_t, uint64_t)
            case GB_FP32_code   : GB_WORKER (int32_t, float)
            case GB_FP64_code   : GB_WORKER (int32_t, double)
            default: ;
        }
        break ;

    case GB_UINT32_code :

        switch (code2)
        {
            //                            code1     code2
            case GB_BOOL_code   : GB_WORKER (uint32_t, bool)
            case GB_INT8_code   : GB_WORKER (uint32_t, int8_t)
            case GB_UINT8_code  : GB_WORKER (uint32_t, uint8_t)
            case GB_INT16_code  : GB_WORKER (uint32_t, int16_t)
            case GB_UINT16_code : GB_WORKER (uint32_t, uint16_t)
            case GB_INT32_code  : GB_WORKER (uint32_t, int32_t)
            #ifndef GB_NOT_SAME
            case GB_UINT32_code : GB_WORKER (uint32_t, uint32_t)
            #endif
            case GB_INT64_code  : GB_WORKER (uint32_t, int64_t)
            case GB_UINT64_code : GB_WORKER (uint32_t, uint64_t)
            case GB_FP32_code   : GB_WORKER (uint32_t, float)
            case GB_FP64_code   : GB_WORKER (uint32_t, double)
            default: ;
        }
        break ;

    case GB_INT64_code  :

        switch (code2)
        {
            //                            code1    code2
            case GB_BOOL_code   : GB_WORKER (int64_t, bool)
            case GB_INT8_code   : GB_WORKER (int64_t, int8_t)
            case GB_UINT8_code  : GB_WORKER (int64_t, uint8_t)
            case GB_INT16_code  : GB_WORKER (int64_t, int16_t)
            case GB_UINT16_code : GB_WORKER (int64_t, uint16_t)
            case GB_INT32_code  : GB_WORKER (int64_t, int32_t)
            case GB_UINT32_code : GB_WORKER (int64_t, uint32_t)
            #ifndef GB_NOT_SAME
            case GB_INT64_code  : GB_WORKER (int64_t, int64_t)
            #endif
            case GB_UINT64_code : GB_WORKER (int64_t, uint64_t)
            case GB_FP32_code   : GB_WORKER (int64_t, float)
            case GB_FP64_code   : GB_WORKER (int64_t, double)
            default: ;
        }
        break ;

    case GB_UINT64_code :

        switch (code2)
        {
            //                            code1     code2
            case GB_BOOL_code   : GB_WORKER (uint64_t, bool)
            case GB_INT8_code   : GB_WORKER (uint64_t, int8_t)
            case GB_UINT8_code  : GB_WORKER (uint64_t, uint8_t)
            case GB_INT16_code  : GB_WORKER (uint64_t, int16_t)
            case GB_UINT16_code : GB_WORKER (uint64_t, uint16_t)
            case GB_INT32_code  : GB_WORKER (uint64_t, int32_t)
            case GB_UINT32_code : GB_WORKER (uint64_t, uint32_t)
            case GB_INT64_code  : GB_WORKER (uint64_t, int64_t)
            #ifndef GB_NOT_SAME
            case GB_UINT64_code : GB_WORKER (uint64_t, uint64_t)
            #endif
            case GB_FP32_code   : GB_WORKER (uint64_t, float)
            case GB_FP64_code   : GB_WORKER (uint64_t, double)
            default: ;
        }
        break ;
        #undef  GB_OP

    case GB_FP32_code   :

        #define GB_OP(x) GB_FOP(x)
        switch (code2)
        {
            //                            code1  code2
            case GB_BOOL_code   : GB_WORKER (float, bool)
            case GB_INT8_code   : GB_WORKER (float, int8_t)
            case GB_UINT8_code  : GB_WORKER (float, uint8_t)
            case GB_INT16_code  : GB_WORKER (float, int16_t)
            case GB_UINT16_code : GB_WORKER (float, uint16_t)
            case GB_INT32_code  : GB_WORKER (float, int32_t)
            case GB_UINT32_code : GB_WORKER (float, uint32_t)
            case GB_INT64_code  : GB_WORKER (float, int64_t)
            case GB_UINT64_code : GB_WORKER (float, uint64_t)
            #ifndef GB_NOT_SAME
            case GB_FP32_code   : GB_WORKER (float, float)
            #endif
            case GB_FP64_code   : GB_WORKER (float, double)
            default: ;
        }
        break ;

    case GB_FP64_code   :

        switch (code2)
        {
            //                            code1   code2
            case GB_BOOL_code   : GB_WORKER (double, bool)
            case GB_INT8_code   : GB_WORKER (double, int8_t)
            case GB_UINT8_code  : GB_WORKER (double, uint8_t)
            case GB_INT16_code  : GB_WORKER (double, int16_t)
            case GB_UINT16_code : GB_WORKER (double, uint16_t)
            case GB_INT32_code  : GB_WORKER (double, int32_t)
            case GB_UINT32_code : GB_WORKER (double, uint32_t)
            case GB_INT64_code  : GB_WORKER (double, int64_t)
            case GB_UINT64_code : GB_WORKER (double, uint64_t)
            case GB_FP32_code   : GB_WORKER (double, float)
            #ifndef GB_NOT_SAME
            case GB_FP64_code   : GB_WORKER (double, double)
            #endif
            default: ;
        }
        break ;
        #undef  GB_OP

    default: ;
}

#undef GB_OP
#undef GB_IOP
#undef GB_FOP
#undef GB_BOP
#undef GB_NOT_SAME

