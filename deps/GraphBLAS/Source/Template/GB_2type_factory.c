//------------------------------------------------------------------------------
// GB_2type_factory.c: 2-type switch factory
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// This is a generic switch factory for creating 121 workers that operate on
// two built-in data types (11 types each), to be #include'd in another file.
// GB_WORKER(op,zname,ztype,xname,xtype) is a macro defined in the #including
// file, where ztype and xtype are the built-in types corresponding to code1
// and code2, respectively or (void *) for a user-defined type.  The last
// statement of GB_WORKER should be a break or return since it does not appear
// here.

// User-defined types are not handled.

switch (code1)
{
    case GB_BOOL_code   :

        switch (code2)
        {
            case GB_BOOL_code   : GB_WORKER (GB_OPNAME, _bool, bool, _bool,   bool)
            case GB_INT8_code   : GB_WORKER (GB_OPNAME, _bool, bool, _int8,   int8_t)
            case GB_INT16_code  : GB_WORKER (GB_OPNAME, _bool, bool, _int16,  int16_t)
            case GB_INT32_code  : GB_WORKER (GB_OPNAME, _bool, bool, _int32,  int32_t)
            case GB_INT64_code  : GB_WORKER (GB_OPNAME, _bool, bool, _int64,  int64_t)
            case GB_UINT8_code  : GB_WORKER (GB_OPNAME, _bool, bool, _uint8,  uint8_t)
            case GB_UINT16_code : GB_WORKER (GB_OPNAME, _bool, bool, _uint16, uint16_t)
            case GB_UINT32_code : GB_WORKER (GB_OPNAME, _bool, bool, _uint32, uint32_t)
            case GB_UINT64_code : GB_WORKER (GB_OPNAME, _bool, bool, _uint64, uint64_t)
            case GB_FP32_code   : GB_WORKER (GB_OPNAME, _bool, bool, _fp32,   float)
            case GB_FP64_code   : GB_WORKER (GB_OPNAME, _bool, bool, _fp64,   double)
            default: ;
        }
        break ;

    case GB_INT8_code   :

        switch (code2)
        {
            case GB_BOOL_code   : GB_WORKER (GB_OPNAME, _int8, int8_t, _bool,   bool)
            case GB_INT8_code   : GB_WORKER (GB_OPNAME, _int8, int8_t, _int8,   int8_t)
            case GB_INT16_code  : GB_WORKER (GB_OPNAME, _int8, int8_t, _int16,  int16_t)
            case GB_INT32_code  : GB_WORKER (GB_OPNAME, _int8, int8_t, _int32,  int32_t)
            case GB_INT64_code  : GB_WORKER (GB_OPNAME, _int8, int8_t, _int64,  int64_t)
            case GB_UINT8_code  : GB_WORKER (GB_OPNAME, _int8, int8_t, _uint8,  uint8_t)
            case GB_UINT16_code : GB_WORKER (GB_OPNAME, _int8, int8_t, _uint16, uint16_t)
            case GB_UINT32_code : GB_WORKER (GB_OPNAME, _int8, int8_t, _uint32, uint32_t)
            case GB_UINT64_code : GB_WORKER (GB_OPNAME, _int8, int8_t, _uint64, uint64_t)
            case GB_FP32_code   : GB_WORKER (GB_OPNAME, _int8, int8_t, _fp32,   float)
            case GB_FP64_code   : GB_WORKER (GB_OPNAME, _int8, int8_t, _fp64,   double)
            default: ;
        }
        break ;

    case GB_INT16_code  :

        switch (code2)
        {
            case GB_BOOL_code   : GB_WORKER (GB_OPNAME, _int16, int16_t, _bool,   bool)
            case GB_INT8_code   : GB_WORKER (GB_OPNAME, _int16, int16_t, _int8,   int8_t)
            case GB_INT16_code  : GB_WORKER (GB_OPNAME, _int16, int16_t, _int16,  int16_t)
            case GB_INT32_code  : GB_WORKER (GB_OPNAME, _int16, int16_t, _int32,  int32_t)
            case GB_INT64_code  : GB_WORKER (GB_OPNAME, _int16, int16_t, _int64,  int64_t)
            case GB_UINT8_code  : GB_WORKER (GB_OPNAME, _int16, int16_t, _uint8,  uint8_t)
            case GB_UINT16_code : GB_WORKER (GB_OPNAME, _int16, int16_t, _uint16, uint16_t)
            case GB_UINT32_code : GB_WORKER (GB_OPNAME, _int16, int16_t, _uint32, uint32_t)
            case GB_UINT64_code : GB_WORKER (GB_OPNAME, _int16, int16_t, _uint64, uint64_t)
            case GB_FP32_code   : GB_WORKER (GB_OPNAME, _int16, int16_t, _fp32,   float)
            case GB_FP64_code   : GB_WORKER (GB_OPNAME, _int16, int16_t, _fp64,   double)
            default: ;
        }
        break ;

    case GB_INT32_code  :

        switch (code2)
        {
            case GB_BOOL_code   : GB_WORKER (GB_OPNAME, _int32, int32_t, _bool,   bool)
            case GB_INT8_code   : GB_WORKER (GB_OPNAME, _int32, int32_t, _int8,   int8_t)
            case GB_INT16_code  : GB_WORKER (GB_OPNAME, _int32, int32_t, _int16,  int16_t)
            case GB_INT32_code  : GB_WORKER (GB_OPNAME, _int32, int32_t, _int32,  int32_t)
            case GB_INT64_code  : GB_WORKER (GB_OPNAME, _int32, int32_t, _int64,  int64_t)
            case GB_UINT8_code  : GB_WORKER (GB_OPNAME, _int32, int32_t, _uint8,  uint8_t)
            case GB_UINT16_code : GB_WORKER (GB_OPNAME, _int32, int32_t, _uint16, uint16_t)
            case GB_UINT32_code : GB_WORKER (GB_OPNAME, _int32, int32_t, _uint32, uint32_t)
            case GB_UINT64_code : GB_WORKER (GB_OPNAME, _int32, int32_t, _uint64, uint64_t)
            case GB_FP32_code   : GB_WORKER (GB_OPNAME, _int32, int32_t, _fp32,   float)
            case GB_FP64_code   : GB_WORKER (GB_OPNAME, _int32, int32_t, _fp64,   double)
            default: ;
        }
        break ;

    case GB_INT64_code  :

        switch (code2)
        {
            case GB_BOOL_code   : GB_WORKER (GB_OPNAME, _int64, int64_t, _bool,   bool)
            case GB_INT8_code   : GB_WORKER (GB_OPNAME, _int64, int64_t, _int8,   int8_t)
            case GB_INT16_code  : GB_WORKER (GB_OPNAME, _int64, int64_t, _int16,  int16_t)
            case GB_INT32_code  : GB_WORKER (GB_OPNAME, _int64, int64_t, _int32,  int32_t)
            case GB_INT64_code  : GB_WORKER (GB_OPNAME, _int64, int64_t, _int64,  int64_t)
            case GB_UINT8_code  : GB_WORKER (GB_OPNAME, _int64, int64_t, _uint8,  uint8_t)
            case GB_UINT16_code : GB_WORKER (GB_OPNAME, _int64, int64_t, _uint16, uint16_t)
            case GB_UINT32_code : GB_WORKER (GB_OPNAME, _int64, int64_t, _uint32, uint32_t)
            case GB_UINT64_code : GB_WORKER (GB_OPNAME, _int64, int64_t, _uint64, uint64_t)
            case GB_FP32_code   : GB_WORKER (GB_OPNAME, _int64, int64_t, _fp32,   float)
            case GB_FP64_code   : GB_WORKER (GB_OPNAME, _int64, int64_t, _fp64,   double)
            default: ;
        }
        break ;

    case GB_UINT8_code  :

        switch (code2)
        {
            case GB_BOOL_code   : GB_WORKER (GB_OPNAME, _uint8, uint8_t, _bool,   bool)
            case GB_INT8_code   : GB_WORKER (GB_OPNAME, _uint8, uint8_t, _int8,   int8_t)
            case GB_INT16_code  : GB_WORKER (GB_OPNAME, _uint8, uint8_t, _int16,  int16_t)
            case GB_INT32_code  : GB_WORKER (GB_OPNAME, _uint8, uint8_t, _int32,  int32_t)
            case GB_INT64_code  : GB_WORKER (GB_OPNAME, _uint8, uint8_t, _int64,  int64_t)
            case GB_UINT8_code  : GB_WORKER (GB_OPNAME, _uint8, uint8_t, _uint8,  uint8_t)
            case GB_UINT16_code : GB_WORKER (GB_OPNAME, _uint8, uint8_t, _uint16, uint16_t)
            case GB_UINT32_code : GB_WORKER (GB_OPNAME, _uint8, uint8_t, _uint32, uint32_t)
            case GB_UINT64_code : GB_WORKER (GB_OPNAME, _uint8, uint8_t, _uint64, uint64_t)
            case GB_FP32_code   : GB_WORKER (GB_OPNAME, _uint8, uint8_t, _fp32,   float)
            case GB_FP64_code   : GB_WORKER (GB_OPNAME, _uint8, uint8_t, _fp64,   double)
            default: ;
        }
        break ;

    case GB_UINT16_code :

        switch (code2)
        {
            case GB_BOOL_code   : GB_WORKER (GB_OPNAME, _uint16, uint16_t, _bool,   bool)
            case GB_INT8_code   : GB_WORKER (GB_OPNAME, _uint16, uint16_t, _int8,   int8_t)
            case GB_INT16_code  : GB_WORKER (GB_OPNAME, _uint16, uint16_t, _int16,  int16_t)
            case GB_INT32_code  : GB_WORKER (GB_OPNAME, _uint16, uint16_t, _int32,  int32_t)
            case GB_INT64_code  : GB_WORKER (GB_OPNAME, _uint16, uint16_t, _int64,  int64_t)
            case GB_UINT8_code  : GB_WORKER (GB_OPNAME, _uint16, uint16_t, _uint8,  uint8_t)
            case GB_UINT16_code : GB_WORKER (GB_OPNAME, _uint16, uint16_t, _uint16, uint16_t)
            case GB_UINT32_code : GB_WORKER (GB_OPNAME, _uint16, uint16_t, _uint32, uint32_t)
            case GB_UINT64_code : GB_WORKER (GB_OPNAME, _uint16, uint16_t, _uint64, uint64_t)
            case GB_FP32_code   : GB_WORKER (GB_OPNAME, _uint16, uint16_t, _fp32,   float)
            case GB_FP64_code   : GB_WORKER (GB_OPNAME, _uint16, uint16_t, _fp64,   double)
            default: ;
        }
        break ;

    case GB_UINT32_code :

        switch (code2)
        {
            case GB_BOOL_code   : GB_WORKER (GB_OPNAME, _uint32, uint32_t, _bool,   bool)
            case GB_INT8_code   : GB_WORKER (GB_OPNAME, _uint32, uint32_t, _int8,   int8_t)
            case GB_INT16_code  : GB_WORKER (GB_OPNAME, _uint32, uint32_t, _int16,  int16_t)
            case GB_INT32_code  : GB_WORKER (GB_OPNAME, _uint32, uint32_t, _int32,  int32_t)
            case GB_INT64_code  : GB_WORKER (GB_OPNAME, _uint32, uint32_t, _int64,  int64_t)
            case GB_UINT8_code  : GB_WORKER (GB_OPNAME, _uint32, uint32_t, _uint8,  uint8_t)
            case GB_UINT16_code : GB_WORKER (GB_OPNAME, _uint32, uint32_t, _uint16, uint16_t)
            case GB_UINT32_code : GB_WORKER (GB_OPNAME, _uint32, uint32_t, _uint32, uint32_t)
            case GB_UINT64_code : GB_WORKER (GB_OPNAME, _uint32, uint32_t, _uint64, uint64_t)
            case GB_FP32_code   : GB_WORKER (GB_OPNAME, _uint32, uint32_t, _fp32,   float)
            case GB_FP64_code   : GB_WORKER (GB_OPNAME, _uint32, uint32_t, _fp64,   double)
            default: ;
        }
        break ;

    case GB_UINT64_code :

        switch (code2)
        {
            case GB_BOOL_code   : GB_WORKER (GB_OPNAME, _uint64, uint64_t, _bool,   bool)
            case GB_INT8_code   : GB_WORKER (GB_OPNAME, _uint64, uint64_t, _int8,   int8_t)
            case GB_INT16_code  : GB_WORKER (GB_OPNAME, _uint64, uint64_t, _int16,  int16_t)
            case GB_INT32_code  : GB_WORKER (GB_OPNAME, _uint64, uint64_t, _int32,  int32_t)
            case GB_INT64_code  : GB_WORKER (GB_OPNAME, _uint64, uint64_t, _int64,  int64_t)
            case GB_UINT8_code  : GB_WORKER (GB_OPNAME, _uint64, uint64_t, _uint8,  uint8_t)
            case GB_UINT16_code : GB_WORKER (GB_OPNAME, _uint64, uint64_t, _uint16, uint16_t)
            case GB_UINT32_code : GB_WORKER (GB_OPNAME, _uint64, uint64_t, _uint32, uint32_t)
            case GB_UINT64_code : GB_WORKER (GB_OPNAME, _uint64, uint64_t, _uint64, uint64_t)
            case GB_FP32_code   : GB_WORKER (GB_OPNAME, _uint64, uint64_t, _fp32,   float)
            case GB_FP64_code   : GB_WORKER (GB_OPNAME, _uint64, uint64_t, _fp64,   double)
            default: ;
        }
        break ;

    case GB_FP32_code   :

        switch (code2)
        {
            case GB_BOOL_code   : GB_WORKER (GB_OPNAME, _fp32, float, _bool,   bool)
            case GB_INT8_code   : GB_WORKER (GB_OPNAME, _fp32, float, _int8,   int8_t)
            case GB_INT16_code  : GB_WORKER (GB_OPNAME, _fp32, float, _int16,  int16_t)
            case GB_INT32_code  : GB_WORKER (GB_OPNAME, _fp32, float, _int32,  int32_t)
            case GB_INT64_code  : GB_WORKER (GB_OPNAME, _fp32, float, _int64,  int64_t)
            case GB_UINT8_code  : GB_WORKER (GB_OPNAME, _fp32, float, _uint8,  uint8_t)
            case GB_UINT16_code : GB_WORKER (GB_OPNAME, _fp32, float, _uint16, uint16_t)
            case GB_UINT32_code : GB_WORKER (GB_OPNAME, _fp32, float, _uint32, uint32_t)
            case GB_UINT64_code : GB_WORKER (GB_OPNAME, _fp32, float, _uint64, uint64_t)
            case GB_FP32_code   : GB_WORKER (GB_OPNAME, _fp32, float, _fp32,   float)
            case GB_FP64_code   : GB_WORKER (GB_OPNAME, _fp32, float, _fp64,   double)
            default: ;
        }
        break ;

    case GB_FP64_code   :

        switch (code2)
        {
            case GB_BOOL_code   : GB_WORKER (GB_OPNAME, _fp64, double, _bool,   bool)
            case GB_INT8_code   : GB_WORKER (GB_OPNAME, _fp64, double, _int8,   int8_t)
            case GB_INT16_code  : GB_WORKER (GB_OPNAME, _fp64, double, _int16,  int16_t)
            case GB_INT32_code  : GB_WORKER (GB_OPNAME, _fp64, double, _int32,  int32_t)
            case GB_INT64_code  : GB_WORKER (GB_OPNAME, _fp64, double, _int64,  int64_t)
            case GB_UINT8_code  : GB_WORKER (GB_OPNAME, _fp64, double, _uint8,  uint8_t)
            case GB_UINT16_code : GB_WORKER (GB_OPNAME, _fp64, double, _uint16, uint16_t)
            case GB_UINT32_code : GB_WORKER (GB_OPNAME, _fp64, double, _uint32, uint32_t)
            case GB_UINT64_code : GB_WORKER (GB_OPNAME, _fp64, double, _uint64, uint64_t)
            case GB_FP32_code   : GB_WORKER (GB_OPNAME, _fp64, double, _fp32,   float)
            case GB_FP64_code   : GB_WORKER (GB_OPNAME, _fp64, double, _fp64,   double)
            default: ;
        }
        break ;

    default: ;
}

#undef GB_OPNAME

