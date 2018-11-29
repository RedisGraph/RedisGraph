//------------------------------------------------------------------------------
// GB_mx_string_to_BinaryOp.c: get a GraphBLAS operator from MATLAB strings
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB_mex.h"

// opname_mx: a MATLAB string defining the operator name (23 kinds):
//  8: 'first', 'second', 'min', 'max', 'plus', 'minus', 'times', 'div',
//  6: 'iseq', 'isne', 'isgt', 'islt', 'isge', 'isle',
//  6: 'eq', 'ne', 'gt', 'lt', 'ge', 'le',
//  3: 'or', 'and' 'xor'

// default_opcode: default if opname_mx is NULL

// opclass_mx: a MATLAB string defining one of 11 operator types:
//  'logical', 'int8', 'uint8', 'int16', 'uint16', 'int32', 'uint32', 'int64',
//  'uint64', 'single', 'double'

// Total # of ops: 23*11 = 253, not including GrB_LOR, GrB_LAND, GrB_XOR,
// which are equivalent to the GxB_*_BOOL versions.

// default_opclass: default class if opclass_mx is NULL

// op is NULL if the opname_mx is NULL

bool GB_mx_string_to_BinaryOp          // true if successful, false otherwise
(
    GrB_BinaryOp *handle,               // the binary op
    const GB_Opcode default_opcode,     // default operator
    const mxClassID default_opclass,    // default operator class
    const mxArray *opname_mx,           // MATLAB string with operator name
    const mxArray *opclass_mx,          // MATLAB string with operator class
    GB_Opcode *opcode_return,           // opcode
    mxClassID *opclass_return,          // opclass
    const bool XisComplex,              // true X is complex
    const bool YisComplex               // true Y is complex
)
{

    (*handle) = NULL ;
    GrB_BinaryOp op = NULL ;

    //--------------------------------------------------------------------------
    // get the string
    //--------------------------------------------------------------------------

    #define LEN 256
    char opname [LEN+2] ;
    int len = GB_mx_mxArray_to_string (opname, LEN, opname_mx) ;
    if (len < 0)
    {
        return (false) ;
    }
    mxClassID opclass ;
    GB_Opcode opcode ; 

    //--------------------------------------------------------------------------
    // convert the string to a GraphBLAS binary operator, built-in or Complex
    //--------------------------------------------------------------------------

    if (XisComplex || YisComplex)
    {

        //----------------------------------------------------------------------
        // X or Y complex
        //----------------------------------------------------------------------

        // user-defined Complex binary operator
        opcode  = GB_USER_R_opcode ;    // generic user-defined opcode
        opclass = mxDOUBLE_CLASS ;      // MATLAB class for complex

        if (len == 0)
        {
            op = NULL ;                 // no default Complex operator
        }

        // 8 binary operators z=f(x,y), all x,y,z are Complex
        else if (MATCH (opname, "first"   )) { op = Complex_first  ; }
        else if (MATCH (opname, "second"  )) { op = Complex_second ; }
        else if (MATCH (opname, "min"     )) { op = Complex_min    ; }
        else if (MATCH (opname, "max"     )) { op = Complex_max    ; }
        else if (MATCH (opname, "plus"    )) { op = Complex_plus   ; }
        else if (MATCH (opname, "minus"   )) { op = Complex_minus  ; }
        else if (MATCH (opname, "times"   )) { op = Complex_times  ; }
        else if (MATCH (opname, "div"     )) { op = Complex_div    ; }

        // 6 ops z=f(x,y), where x,y are Complex, z = (1,0) or (0,0)
        else if (MATCH (opname, "iseq"    )) { op = Complex_iseq   ; }
        else if (MATCH (opname, "isne"    )) { op = Complex_isne   ; }
        else if (MATCH (opname, "isgt"    )) { op = Complex_isgt   ; }
        else if (MATCH (opname, "islt"    )) { op = Complex_islt   ; }
        else if (MATCH (opname, "isge"    )) { op = Complex_isge   ; }
        else if (MATCH (opname, "isle"    )) { op = Complex_isle   ; }

        // 3 binary operators z=f(x,y), all x,y,x the same type
        else if (MATCH (opname, "or"      )) { op = Complex_or ; }
        else if (MATCH (opname, "and"     )) { op = Complex_and ; }
        else if (MATCH (opname, "xor"     )) { op = Complex_xor ; }

        // 6 ops z=f(x,y), where x,y are Complex type but z is boolean
        else if (MATCH (opname, "eq"      )) { op = Complex_eq     ; }
        else if (MATCH (opname, "ne"      )) { op = Complex_ne     ; }
        else if (MATCH (opname, "gt"      )) { op = Complex_gt     ; }
        else if (MATCH (opname, "lt"      )) { op = Complex_lt     ; }
        else if (MATCH (opname, "ge"      )) { op = Complex_ge     ; }
        else if (MATCH (opname, "le"      )) { op = Complex_le     ; }

        else
        {
            mexWarnMsgIdAndTxt ("GB:warn", "Complex op unrecognized") ;
        }

    }
    else
    {

        //----------------------------------------------------------------------
        // X and Y real (Z might be Complex)
        //----------------------------------------------------------------------

        // convert the opname to an opcode
        opclass = default_opclass ;         // default if no opclass specified

        if (len == 0)
        {
            // default if opname_mx is NULL
            opcode = default_opcode ;
        }

        // 8 binary operators z=f(x,y), all x,y,z of the same type
        else if (MATCH (opname, "first"   )) { opcode = GB_FIRST_opcode ; }
        else if (MATCH (opname, "second"  )) { opcode = GB_SECOND_opcode ; }
        else if (MATCH (opname, "min"     )) { opcode = GB_MIN_opcode ; }
        else if (MATCH (opname, "max"     )) { opcode = GB_MAX_opcode ; }
        else if (MATCH (opname, "plus"    )) { opcode = GB_PLUS_opcode ; }
        else if (MATCH (opname, "minus"   )) { opcode = GB_MINUS_opcode ; }
        else if (MATCH (opname, "times"   )) { opcode = GB_TIMES_opcode ; }
        else if (MATCH (opname, "div"     )) { opcode = GB_DIV_opcode ; }

        // 6 ops z=f(x,y), all x,y,z the same type
        else if (MATCH (opname, "iseq"    )) { opcode = GB_ISEQ_opcode ; }
        else if (MATCH (opname, "isne"    )) { opcode = GB_ISNE_opcode ; }
        else if (MATCH (opname, "isgt"    )) { opcode = GB_ISGT_opcode ; }
        else if (MATCH (opname, "islt"    )) { opcode = GB_ISLT_opcode ; }
        else if (MATCH (opname, "isge"    )) { opcode = GB_ISGE_opcode ; }
        else if (MATCH (opname, "isle"    )) { opcode = GB_ISLE_opcode ; }

        // 3 binary operators z=f(x,y), all x,y,x the same type
        else if (MATCH (opname, "or"      )) { opcode = GB_LOR_opcode ; }
        else if (MATCH (opname, "and"     )) { opcode = GB_LAND_opcode ; }
        else if (MATCH (opname, "xor"     )) { opcode = GB_LXOR_opcode ; }

        // 6 ops z=f(x,y), where x,y are the requested type but z is boolean
        else if (MATCH (opname, "eq"      )) { opcode = GB_EQ_opcode ; }
        else if (MATCH (opname, "ne"      )) { opcode = GB_NE_opcode ; }
        else if (MATCH (opname, "gt"      )) { opcode = GB_GT_opcode ; }
        else if (MATCH (opname, "lt"      )) { opcode = GB_LT_opcode ; }
        else if (MATCH (opname, "ge"      )) { opcode = GB_GE_opcode ; }
        else if (MATCH (opname, "le"      )) { opcode = GB_LE_opcode ; }

        // 1 user-defined Complex operator z=f(x,y), x,y double and z Complex
        else if (MATCH (opname, "complex" ))
        {
            // z = complex(x,y) = x + i*y
            op = Complex_complex ;
            opcode = GB_USER_R_opcode ;
            opclass = mxDOUBLE_CLASS ;
        }

        else
        {
            mexWarnMsgIdAndTxt ("GB:warn", "unrecognised function name") ;
            return (false) ;
        }

        if (opcode != GB_USER_R_opcode)
        {
            // get the opclass from the opclass_mx string, if present
            opclass = GB_mx_string_to_classID (opclass, opclass_mx) ;
            if (opclass == mxUNKNOWN_CLASS)
            {
                mexWarnMsgIdAndTxt ("GB:warn", "unrecognised op class") ;
                return (false) ;
            }
        }

        switch (opcode)
        {

            case GB_FIRST_opcode :

                switch (opclass)
                {
                    case mxLOGICAL_CLASS : op = GrB_FIRST_BOOL   ; break ;
                    case mxINT8_CLASS    : op = GrB_FIRST_INT8   ; break ;
                    case mxUINT8_CLASS   : op = GrB_FIRST_UINT8  ; break ;
                    case mxINT16_CLASS   : op = GrB_FIRST_INT16  ; break ;
                    case mxUINT16_CLASS  : op = GrB_FIRST_UINT16 ; break ;
                    case mxINT32_CLASS   : op = GrB_FIRST_INT32  ; break ;
                    case mxUINT32_CLASS  : op = GrB_FIRST_UINT32 ; break ;
                    case mxINT64_CLASS   : op = GrB_FIRST_INT64  ; break ;
                    case mxUINT64_CLASS  : op = GrB_FIRST_UINT64 ; break ;
                    case mxSINGLE_CLASS  : op = GrB_FIRST_FP32   ; break ;
                    case mxDOUBLE_CLASS  : op = GrB_FIRST_FP64   ; break ;
                    default              : 
                        mexWarnMsgIdAndTxt ("GB:warn","unknown type") ;
                        return (false) ;
                }
                break ;

            case GB_SECOND_opcode:

                switch (opclass)
                {
                    case mxLOGICAL_CLASS : op = GrB_SECOND_BOOL   ; break ;
                    case mxINT8_CLASS    : op = GrB_SECOND_INT8   ; break ;
                    case mxUINT8_CLASS   : op = GrB_SECOND_UINT8  ; break ;
                    case mxINT16_CLASS   : op = GrB_SECOND_INT16  ; break ;
                    case mxUINT16_CLASS  : op = GrB_SECOND_UINT16 ; break ;
                    case mxINT32_CLASS   : op = GrB_SECOND_INT32  ; break ;
                    case mxUINT32_CLASS  : op = GrB_SECOND_UINT32 ; break ;
                    case mxINT64_CLASS   : op = GrB_SECOND_INT64  ; break ;
                    case mxUINT64_CLASS  : op = GrB_SECOND_UINT64 ; break ;
                    case mxSINGLE_CLASS  : op = GrB_SECOND_FP32   ; break ;
                    case mxDOUBLE_CLASS  : op = GrB_SECOND_FP64   ; break ;
                    default              : 
                        mexWarnMsgIdAndTxt ("GB:warn","unknown type") ;
                        return (false) ;
                }
                break ;

            case GB_MIN_opcode   :

                switch (opclass)
                {
                    case mxLOGICAL_CLASS : op = GrB_MIN_BOOL   ; break ;
                    case mxINT8_CLASS    : op = GrB_MIN_INT8   ; break ;
                    case mxUINT8_CLASS   : op = GrB_MIN_UINT8  ; break ;
                    case mxINT16_CLASS   : op = GrB_MIN_INT16  ; break ;
                    case mxUINT16_CLASS  : op = GrB_MIN_UINT16 ; break ;
                    case mxINT32_CLASS   : op = GrB_MIN_INT32  ; break ;
                    case mxUINT32_CLASS  : op = GrB_MIN_UINT32 ; break ;
                    case mxINT64_CLASS   : op = GrB_MIN_INT64  ; break ;
                    case mxUINT64_CLASS  : op = GrB_MIN_UINT64 ; break ;
                    case mxSINGLE_CLASS  : op = GrB_MIN_FP32   ; break ;
                    case mxDOUBLE_CLASS  : op = GrB_MIN_FP64   ; break ;
                    default              : 
                        mexWarnMsgIdAndTxt ("GB:warn","unknown type") ;
                        break ;
                }
                break ;

            case GB_MAX_opcode   :

                switch (opclass)
                {
                    case mxLOGICAL_CLASS : op = GrB_MAX_BOOL   ; break ;
                    case mxINT8_CLASS    : op = GrB_MAX_INT8   ; break ;
                    case mxUINT8_CLASS   : op = GrB_MAX_UINT8  ; break ;
                    case mxINT16_CLASS   : op = GrB_MAX_INT16  ; break ;
                    case mxUINT16_CLASS  : op = GrB_MAX_UINT16 ; break ;
                    case mxINT32_CLASS   : op = GrB_MAX_INT32  ; break ;
                    case mxUINT32_CLASS  : op = GrB_MAX_UINT32 ; break ;
                    case mxINT64_CLASS   : op = GrB_MAX_INT64  ; break ;
                    case mxUINT64_CLASS  : op = GrB_MAX_UINT64 ; break ;
                    case mxSINGLE_CLASS  : op = GrB_MAX_FP32   ; break ;
                    case mxDOUBLE_CLASS  : op = GrB_MAX_FP64   ; break ;
                    default              : 
                        mexWarnMsgIdAndTxt ("GB:warn","unknown type") ;
                        return (false) ;
                }
                break ;

            case GB_PLUS_opcode  :

                switch (opclass)
                {
                    case mxLOGICAL_CLASS : op = GrB_PLUS_BOOL   ; break ;
                    case mxINT8_CLASS    : op = GrB_PLUS_INT8   ; break ;
                    case mxUINT8_CLASS   : op = GrB_PLUS_UINT8  ; break ;
                    case mxINT16_CLASS   : op = GrB_PLUS_INT16  ; break ;
                    case mxUINT16_CLASS  : op = GrB_PLUS_UINT16 ; break ;
                    case mxINT32_CLASS   : op = GrB_PLUS_INT32  ; break ;
                    case mxUINT32_CLASS  : op = GrB_PLUS_UINT32 ; break ;
                    case mxINT64_CLASS   : op = GrB_PLUS_INT64  ; break ;
                    case mxUINT64_CLASS  : op = GrB_PLUS_UINT64 ; break ;
                    case mxSINGLE_CLASS  : op = GrB_PLUS_FP32   ; break ;
                    case mxDOUBLE_CLASS  : op = GrB_PLUS_FP64   ; break ;
                    default              : 
                        mexWarnMsgIdAndTxt ("GB:warn","unknown type") ;
                        return (false) ;
                }
                break ;

            case GB_MINUS_opcode :

                switch (opclass)
                {
                    case mxLOGICAL_CLASS : op = GrB_MINUS_BOOL   ; break ;
                    case mxINT8_CLASS    : op = GrB_MINUS_INT8   ; break ;
                    case mxUINT8_CLASS   : op = GrB_MINUS_UINT8  ; break ;
                    case mxINT16_CLASS   : op = GrB_MINUS_INT16  ; break ;
                    case mxUINT16_CLASS  : op = GrB_MINUS_UINT16 ; break ;
                    case mxINT32_CLASS   : op = GrB_MINUS_INT32  ; break ;
                    case mxUINT32_CLASS  : op = GrB_MINUS_UINT32 ; break ;
                    case mxINT64_CLASS   : op = GrB_MINUS_INT64  ; break ;
                    case mxUINT64_CLASS  : op = GrB_MINUS_UINT64 ; break ;
                    case mxSINGLE_CLASS  : op = GrB_MINUS_FP32   ; break ;
                    case mxDOUBLE_CLASS  : op = GrB_MINUS_FP64   ; break ;
                    default              : 
                        mexWarnMsgIdAndTxt ("GB:warn","unknown type") ;
                        return (false) ;
                }
                break ;

            case GB_TIMES_opcode :

                switch (opclass)
                {
                    case mxLOGICAL_CLASS : op = GrB_TIMES_BOOL   ; break ;
                    case mxINT8_CLASS    : op = GrB_TIMES_INT8   ; break ;
                    case mxUINT8_CLASS   : op = GrB_TIMES_UINT8  ; break ;
                    case mxINT16_CLASS   : op = GrB_TIMES_INT16  ; break ;
                    case mxUINT16_CLASS  : op = GrB_TIMES_UINT16 ; break ;
                    case mxINT32_CLASS   : op = GrB_TIMES_INT32  ; break ;
                    case mxUINT32_CLASS  : op = GrB_TIMES_UINT32 ; break ;
                    case mxINT64_CLASS   : op = GrB_TIMES_INT64  ; break ;
                    case mxUINT64_CLASS  : op = GrB_TIMES_UINT64 ; break ;
                    case mxSINGLE_CLASS  : op = GrB_TIMES_FP32   ; break ;
                    case mxDOUBLE_CLASS  : op = GrB_TIMES_FP64   ; break ;
                    default              : 
                        mexWarnMsgIdAndTxt ("GB:warn","unknown type") ;
                        return (false) ;
                }
                break ;

            case GB_DIV_opcode   :

                switch (opclass)
                {
                    case mxLOGICAL_CLASS : op = GrB_DIV_BOOL   ; break ;
                    case mxINT8_CLASS    : op = GrB_DIV_INT8   ; break ;
                    case mxUINT8_CLASS   : op = GrB_DIV_UINT8  ; break ;
                    case mxINT16_CLASS   : op = GrB_DIV_INT16  ; break ;
                    case mxUINT16_CLASS  : op = GrB_DIV_UINT16 ; break ;
                    case mxINT32_CLASS   : op = GrB_DIV_INT32  ; break ;
                    case mxUINT32_CLASS  : op = GrB_DIV_UINT32 ; break ;
                    case mxINT64_CLASS   : op = GrB_DIV_INT64  ; break ;
                    case mxUINT64_CLASS  : op = GrB_DIV_UINT64 ; break ;
                    case mxSINGLE_CLASS  : op = GrB_DIV_FP32   ; break ;
                    case mxDOUBLE_CLASS  : op = GrB_DIV_FP64   ; break ;
                    default              : 
                        mexWarnMsgIdAndTxt ("GB:warn","unknown type") ;
                        return (false) ;
                }
                break ;


            case GB_ISEQ_opcode :

                switch (opclass)
                {
                    case mxLOGICAL_CLASS : op = GxB_ISEQ_BOOL   ; break ;
                    case mxINT8_CLASS    : op = GxB_ISEQ_INT8   ; break ;
                    case mxUINT8_CLASS   : op = GxB_ISEQ_UINT8  ; break ;
                    case mxINT16_CLASS   : op = GxB_ISEQ_INT16  ; break ;
                    case mxUINT16_CLASS  : op = GxB_ISEQ_UINT16 ; break ;
                    case mxINT32_CLASS   : op = GxB_ISEQ_INT32  ; break ;
                    case mxUINT32_CLASS  : op = GxB_ISEQ_UINT32 ; break ;
                    case mxINT64_CLASS   : op = GxB_ISEQ_INT64  ; break ;
                    case mxUINT64_CLASS  : op = GxB_ISEQ_UINT64 ; break ;
                    case mxSINGLE_CLASS  : op = GxB_ISEQ_FP32   ; break ;
                    case mxDOUBLE_CLASS  : op = GxB_ISEQ_FP64   ; break ;
                    default              : 
                        mexWarnMsgIdAndTxt ("GB:warn","unknown type") ;
                        return (false) ;
                }
                break ;

            case GB_ISNE_opcode :

                switch (opclass)
                {
                    case mxLOGICAL_CLASS : op = GxB_ISNE_BOOL   ; break ;
                    case mxINT8_CLASS    : op = GxB_ISNE_INT8   ; break ;
                    case mxUINT8_CLASS   : op = GxB_ISNE_UINT8  ; break ;
                    case mxINT16_CLASS   : op = GxB_ISNE_INT16  ; break ;
                    case mxUINT16_CLASS  : op = GxB_ISNE_UINT16 ; break ;
                    case mxINT32_CLASS   : op = GxB_ISNE_INT32  ; break ;
                    case mxUINT32_CLASS  : op = GxB_ISNE_UINT32 ; break ;
                    case mxINT64_CLASS   : op = GxB_ISNE_INT64  ; break ;
                    case mxUINT64_CLASS  : op = GxB_ISNE_UINT64 ; break ;
                    case mxSINGLE_CLASS  : op = GxB_ISNE_FP32   ; break ;
                    case mxDOUBLE_CLASS  : op = GxB_ISNE_FP64   ; break ;
                    default              : 
                        mexWarnMsgIdAndTxt ("GB:warn","unknown type") ;
                        return (false) ;
                }
                break ;

            case GB_ISGT_opcode :

                switch (opclass)
                {
                    case mxLOGICAL_CLASS : op = GxB_ISGT_BOOL   ; break ;
                    case mxINT8_CLASS    : op = GxB_ISGT_INT8   ; break ;
                    case mxUINT8_CLASS   : op = GxB_ISGT_UINT8  ; break ;
                    case mxINT16_CLASS   : op = GxB_ISGT_INT16  ; break ;
                    case mxUINT16_CLASS  : op = GxB_ISGT_UINT16 ; break ;
                    case mxINT32_CLASS   : op = GxB_ISGT_INT32  ; break ;
                    case mxUINT32_CLASS  : op = GxB_ISGT_UINT32 ; break ;
                    case mxINT64_CLASS   : op = GxB_ISGT_INT64  ; break ;
                    case mxUINT64_CLASS  : op = GxB_ISGT_UINT64 ; break ;
                    case mxSINGLE_CLASS  : op = GxB_ISGT_FP32   ; break ;
                    case mxDOUBLE_CLASS  : op = GxB_ISGT_FP64   ; break ;
                    default              : 
                        mexWarnMsgIdAndTxt ("GB:warn","unknown type") ;
                        return (false) ;
                }
                break ;

            case GB_ISLT_opcode :

                switch (opclass)
                {
                    case mxLOGICAL_CLASS : op = GxB_ISLT_BOOL   ; break ;
                    case mxINT8_CLASS    : op = GxB_ISLT_INT8   ; break ;
                    case mxUINT8_CLASS   : op = GxB_ISLT_UINT8  ; break ;
                    case mxINT16_CLASS   : op = GxB_ISLT_INT16  ; break ;
                    case mxUINT16_CLASS  : op = GxB_ISLT_UINT16 ; break ;
                    case mxINT32_CLASS   : op = GxB_ISLT_INT32  ; break ;
                    case mxUINT32_CLASS  : op = GxB_ISLT_UINT32 ; break ;
                    case mxINT64_CLASS   : op = GxB_ISLT_INT64  ; break ;
                    case mxUINT64_CLASS  : op = GxB_ISLT_UINT64 ; break ;
                    case mxSINGLE_CLASS  : op = GxB_ISLT_FP32   ; break ;
                    case mxDOUBLE_CLASS  : op = GxB_ISLT_FP64   ; break ;
                    default              : 
                        mexWarnMsgIdAndTxt ("GB:warn","unknown type") ;
                        return (false) ;
                }
                break ;

            case GB_ISGE_opcode :

                switch (opclass)
                {
                    case mxLOGICAL_CLASS : op = GxB_ISGE_BOOL   ; break ;
                    case mxINT8_CLASS    : op = GxB_ISGE_INT8   ; break ;
                    case mxUINT8_CLASS   : op = GxB_ISGE_UINT8  ; break ;
                    case mxINT16_CLASS   : op = GxB_ISGE_INT16  ; break ;
                    case mxUINT16_CLASS  : op = GxB_ISGE_UINT16 ; break ;
                    case mxINT32_CLASS   : op = GxB_ISGE_INT32  ; break ;
                    case mxUINT32_CLASS  : op = GxB_ISGE_UINT32 ; break ;
                    case mxINT64_CLASS   : op = GxB_ISGE_INT64  ; break ;
                    case mxUINT64_CLASS  : op = GxB_ISGE_UINT64 ; break ;
                    case mxSINGLE_CLASS  : op = GxB_ISGE_FP32   ; break ;
                    case mxDOUBLE_CLASS  : op = GxB_ISGE_FP64   ; break ;
                    default              : 
                        mexWarnMsgIdAndTxt ("GB:warn","unknown type") ;
                        return (false) ;
                }
                break ;

            case GB_ISLE_opcode :

                switch (opclass)
                {
                    case mxLOGICAL_CLASS : op = GxB_ISLE_BOOL   ; break ;
                    case mxINT8_CLASS    : op = GxB_ISLE_INT8   ; break ;
                    case mxUINT8_CLASS   : op = GxB_ISLE_UINT8  ; break ;
                    case mxINT16_CLASS   : op = GxB_ISLE_INT16  ; break ;
                    case mxUINT16_CLASS  : op = GxB_ISLE_UINT16 ; break ;
                    case mxINT32_CLASS   : op = GxB_ISLE_INT32  ; break ;
                    case mxUINT32_CLASS  : op = GxB_ISLE_UINT32 ; break ;
                    case mxINT64_CLASS   : op = GxB_ISLE_INT64  ; break ;
                    case mxUINT64_CLASS  : op = GxB_ISLE_UINT64 ; break ;
                    case mxSINGLE_CLASS  : op = GxB_ISLE_FP32   ; break ;
                    case mxDOUBLE_CLASS  : op = GxB_ISLE_FP64   ; break ;
                    default              : 
                        mexWarnMsgIdAndTxt ("GB:warn","unknown type") ;
                        return (false) ;
                }
                break ;


            case GB_EQ_opcode :

                switch (opclass)
                {
                    case mxLOGICAL_CLASS : op = GrB_EQ_BOOL   ; break ;
                    case mxINT8_CLASS    : op = GrB_EQ_INT8   ; break ;
                    case mxUINT8_CLASS   : op = GrB_EQ_UINT8  ; break ;
                    case mxINT16_CLASS   : op = GrB_EQ_INT16  ; break ;
                    case mxUINT16_CLASS  : op = GrB_EQ_UINT16 ; break ;
                    case mxINT32_CLASS   : op = GrB_EQ_INT32  ; break ;
                    case mxUINT32_CLASS  : op = GrB_EQ_UINT32 ; break ;
                    case mxINT64_CLASS   : op = GrB_EQ_INT64  ; break ;
                    case mxUINT64_CLASS  : op = GrB_EQ_UINT64 ; break ;
                    case mxSINGLE_CLASS  : op = GrB_EQ_FP32   ; break ;
                    case mxDOUBLE_CLASS  : op = GrB_EQ_FP64   ; break ;
                    default              : 
                        mexWarnMsgIdAndTxt ("GB:warn","unknown type") ;
                        return (false) ;
                }
                break ;

            case GB_NE_opcode :

                switch (opclass)
                {
                    case mxLOGICAL_CLASS : op = GrB_NE_BOOL   ; break ;
                    case mxINT8_CLASS    : op = GrB_NE_INT8   ; break ;
                    case mxUINT8_CLASS   : op = GrB_NE_UINT8  ; break ;
                    case mxINT16_CLASS   : op = GrB_NE_INT16  ; break ;
                    case mxUINT16_CLASS  : op = GrB_NE_UINT16 ; break ;
                    case mxINT32_CLASS   : op = GrB_NE_INT32  ; break ;
                    case mxUINT32_CLASS  : op = GrB_NE_UINT32 ; break ;
                    case mxINT64_CLASS   : op = GrB_NE_INT64  ; break ;
                    case mxUINT64_CLASS  : op = GrB_NE_UINT64 ; break ;
                    case mxSINGLE_CLASS  : op = GrB_NE_FP32   ; break ;
                    case mxDOUBLE_CLASS  : op = GrB_NE_FP64   ; break ;
                    default              : 
                        mexWarnMsgIdAndTxt ("GB:warn","unknown type") ;
                        return (false) ;
                }
                break ;

            case GB_GT_opcode :

                switch (opclass)
                {
                    case mxLOGICAL_CLASS : op = GrB_GT_BOOL   ; break ;
                    case mxINT8_CLASS    : op = GrB_GT_INT8   ; break ;
                    case mxUINT8_CLASS   : op = GrB_GT_UINT8  ; break ;
                    case mxINT16_CLASS   : op = GrB_GT_INT16  ; break ;
                    case mxUINT16_CLASS  : op = GrB_GT_UINT16 ; break ;
                    case mxINT32_CLASS   : op = GrB_GT_INT32  ; break ;
                    case mxUINT32_CLASS  : op = GrB_GT_UINT32 ; break ;
                    case mxINT64_CLASS   : op = GrB_GT_INT64  ; break ;
                    case mxUINT64_CLASS  : op = GrB_GT_UINT64 ; break ;
                    case mxSINGLE_CLASS  : op = GrB_GT_FP32   ; break ;
                    case mxDOUBLE_CLASS  : op = GrB_GT_FP64   ; break ;
                    default              : 
                        mexWarnMsgIdAndTxt ("GB:warn","unknown type") ;
                        return (false) ;
                }
                break ;

            case GB_LT_opcode :

                switch (opclass)
                {
                    case mxLOGICAL_CLASS : op = GrB_LT_BOOL   ; break ;
                    case mxINT8_CLASS    : op = GrB_LT_INT8   ; break ;
                    case mxUINT8_CLASS   : op = GrB_LT_UINT8  ; break ;
                    case mxINT16_CLASS   : op = GrB_LT_INT16  ; break ;
                    case mxUINT16_CLASS  : op = GrB_LT_UINT16 ; break ;
                    case mxINT32_CLASS   : op = GrB_LT_INT32  ; break ;
                    case mxUINT32_CLASS  : op = GrB_LT_UINT32 ; break ;
                    case mxINT64_CLASS   : op = GrB_LT_INT64  ; break ;
                    case mxUINT64_CLASS  : op = GrB_LT_UINT64 ; break ;
                    case mxSINGLE_CLASS  : op = GrB_LT_FP32   ; break ;
                    case mxDOUBLE_CLASS  : op = GrB_LT_FP64   ; break ;
                    default              : 
                        mexWarnMsgIdAndTxt ("GB:warn","unknown type") ;
                        return (false) ;
                }
                break ;

            case GB_GE_opcode :

                switch (opclass)
                {
                    case mxLOGICAL_CLASS : op = GrB_GE_BOOL   ; break ;
                    case mxINT8_CLASS    : op = GrB_GE_INT8   ; break ;
                    case mxUINT8_CLASS   : op = GrB_GE_UINT8  ; break ;
                    case mxINT16_CLASS   : op = GrB_GE_INT16  ; break ;
                    case mxUINT16_CLASS  : op = GrB_GE_UINT16 ; break ;
                    case mxINT32_CLASS   : op = GrB_GE_INT32  ; break ;
                    case mxUINT32_CLASS  : op = GrB_GE_UINT32 ; break ;
                    case mxINT64_CLASS   : op = GrB_GE_INT64  ; break ;
                    case mxUINT64_CLASS  : op = GrB_GE_UINT64 ; break ;
                    case mxSINGLE_CLASS  : op = GrB_GE_FP32   ; break ;
                    case mxDOUBLE_CLASS  : op = GrB_GE_FP64   ; break ;
                    default              : 
                        mexWarnMsgIdAndTxt ("GB:warn","unknown type") ;
                        return (false) ;
                }
                break ;

            case GB_LE_opcode :

                switch (opclass)
                {
                    case mxLOGICAL_CLASS : op = GrB_LE_BOOL   ; break ;
                    case mxINT8_CLASS    : op = GrB_LE_INT8   ; break ;
                    case mxUINT8_CLASS   : op = GrB_LE_UINT8  ; break ;
                    case mxINT16_CLASS   : op = GrB_LE_INT16  ; break ;
                    case mxUINT16_CLASS  : op = GrB_LE_UINT16 ; break ;
                    case mxINT32_CLASS   : op = GrB_LE_INT32  ; break ;
                    case mxUINT32_CLASS  : op = GrB_LE_UINT32 ; break ;
                    case mxINT64_CLASS   : op = GrB_LE_INT64  ; break ;
                    case mxUINT64_CLASS  : op = GrB_LE_UINT64 ; break ;
                    case mxSINGLE_CLASS  : op = GrB_LE_FP32   ; break ;
                    case mxDOUBLE_CLASS  : op = GrB_LE_FP64   ; break ;
                    default              : 
                        mexWarnMsgIdAndTxt ("GB:warn","unknown type") ;
                        return (false) ;
                }
                break ;


            case GB_LOR_opcode   :

                switch (opclass)
                {
                    case mxLOGICAL_CLASS : op = GrB_LOR        ; break ;
                    case mxINT8_CLASS    : op = GxB_LOR_INT8   ; break ;
                    case mxUINT8_CLASS   : op = GxB_LOR_UINT8  ; break ;
                    case mxINT16_CLASS   : op = GxB_LOR_INT16  ; break ;
                    case mxUINT16_CLASS  : op = GxB_LOR_UINT16 ; break ;
                    case mxINT32_CLASS   : op = GxB_LOR_INT32  ; break ;
                    case mxUINT32_CLASS  : op = GxB_LOR_UINT32 ; break ;
                    case mxINT64_CLASS   : op = GxB_LOR_INT64  ; break ;
                    case mxUINT64_CLASS  : op = GxB_LOR_UINT64 ; break ;
                    case mxSINGLE_CLASS  : op = GxB_LOR_FP32   ; break ;
                    case mxDOUBLE_CLASS  : op = GxB_LOR_FP64   ; break ;
                    default              : 
                        mexWarnMsgIdAndTxt ("GB:warn","unknown type") ;
                        return (false) ;
                }
                break ;

            case GB_LAND_opcode   :

                switch (opclass)
                {
                    case mxLOGICAL_CLASS : op = GrB_LAND        ; break ;
                    case mxINT8_CLASS    : op = GxB_LAND_INT8   ; break ;
                    case mxUINT8_CLASS   : op = GxB_LAND_UINT8  ; break ;
                    case mxINT16_CLASS   : op = GxB_LAND_INT16  ; break ;
                    case mxUINT16_CLASS  : op = GxB_LAND_UINT16 ; break ;
                    case mxINT32_CLASS   : op = GxB_LAND_INT32  ; break ;
                    case mxUINT32_CLASS  : op = GxB_LAND_UINT32 ; break ;
                    case mxINT64_CLASS   : op = GxB_LAND_INT64  ; break ;
                    case mxUINT64_CLASS  : op = GxB_LAND_UINT64 ; break ;
                    case mxSINGLE_CLASS  : op = GxB_LAND_FP32   ; break ;
                    case mxDOUBLE_CLASS  : op = GxB_LAND_FP64   ; break ;
                    default              : 
                        mexWarnMsgIdAndTxt ("GB:warn","unknown type") ;
                        return (false) ;
                }
                break ;

            case GB_LXOR_opcode   :

                switch (opclass)
                {
                    case mxLOGICAL_CLASS : op = GrB_LXOR        ; break ;
                    case mxINT8_CLASS    : op = GxB_LXOR_INT8   ; break ;
                    case mxUINT8_CLASS   : op = GxB_LXOR_UINT8  ; break ;
                    case mxINT16_CLASS   : op = GxB_LXOR_INT16  ; break ;
                    case mxUINT16_CLASS  : op = GxB_LXOR_UINT16 ; break ;
                    case mxINT32_CLASS   : op = GxB_LXOR_INT32  ; break ;
                    case mxUINT32_CLASS  : op = GxB_LXOR_UINT32 ; break ;
                    case mxINT64_CLASS   : op = GxB_LXOR_INT64  ; break ;
                    case mxUINT64_CLASS  : op = GxB_LXOR_UINT64 ; break ;
                    case mxSINGLE_CLASS  : op = GxB_LXOR_FP32   ; break ;
                    case mxDOUBLE_CLASS  : op = GxB_LXOR_FP64   ; break ;
                    default              : 
                        mexWarnMsgIdAndTxt ("GB:warn","unknown type") ;
                        return (false) ;
                }
                break ;

            case GB_NOP_opcode   :
            case GB_USER_C_opcode   :
            case GB_USER_R_opcode   :

                // no operation is requested so return NULL, or user-defined
                break ;

            default : 
                mexWarnMsgIdAndTxt ("GB:warn","unknown binary operator") ;
                return (false) ;
        }
    }

    //--------------------------------------------------------------------------
    // return result
    //--------------------------------------------------------------------------

    // return the opclass and opcode to the caller
    if (opclass_return != NULL) *opclass_return = opclass ;
    if (opcode_return  != NULL) *opcode_return  = opcode ;


    // return the binary operator to the caller
    ASSERT_OK_OR_NULL (GB_check (op, "got binary op", GB0)) ;
    (*handle) = op ;
    return (true) ;
}

