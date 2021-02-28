//------------------------------------------------------------------------------
// GB_1type_factory.c: 1-type switch factory
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// This is a generic switch factory for creating 11 workers that operate on
// one built-in data types, to be #include'd in another file.

// GB_1TYPE_WORKER(xname) is a macro defined in the #including file.  The last
// statement of GB_1TYPE_WORKER should be a break or return since it does not
// appear here.

// User-defined types are not handled.

{
    switch (ccode)
    {
        case GB_BOOL_code   : GB_1TYPE_WORKER (_bool  )
        case GB_INT8_code   : GB_1TYPE_WORKER (_int8  )
        case GB_INT16_code  : GB_1TYPE_WORKER (_int16 )
        case GB_INT32_code  : GB_1TYPE_WORKER (_int32 )
        case GB_INT64_code  : GB_1TYPE_WORKER (_int64 )
        case GB_UINT8_code  : GB_1TYPE_WORKER (_uint8 )
        case GB_UINT16_code : GB_1TYPE_WORKER (_uint16)
        case GB_UINT32_code : GB_1TYPE_WORKER (_uint32)
        case GB_UINT64_code : GB_1TYPE_WORKER (_uint64)
        case GB_FP32_code   : GB_1TYPE_WORKER (_fp32  )
        case GB_FP64_code   : GB_1TYPE_WORKER (_fp64  )
        default: ;
    }
}
