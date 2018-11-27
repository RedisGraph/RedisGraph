//------------------------------------------------------------------------------
// GB_mx_string_to_UnaryOp.c: get a GraphBLAS operator from MATLAB strings
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB_mex.h"

// opname_mx: a MATLAB string defining the operator name (built-in):
//      'one', 'identity', 'ainv', 'abs', 'minv', 'not'
// or a user-defined operator defined at run-time:
//      'conj', 'real', 'imag', 'cabs', 'angle', 'complex_real', 'complex_imag'
// or a user-defined operator defined at compile-time:
//      'my_scale'

// default_opcode: default if opname_mx is NULL

// opclass_mx: a MATLAB string defining the operator type for built-in ops:
//  'logical', 'int8', 'uint8', 'int16', 'uint16', 'int32', 'uint32', 'int64',
//  'uint64', 'single', 'double'

// Total ops: 4*11 = 44, not including GrB_LNOT

// default_opclass: default class if opclass_mx is NULL

bool GB_mx_string_to_UnaryOp           // true if successful, false otherwise
(
    GrB_UnaryOp *handle,                // the unary op
    const GB_Opcode default_opcode,     // default operator
    const mxClassID default_opclass,    // default operator class
    const mxArray *opname_mx,           // MATLAB string with operator name
    const mxArray *opclass_mx,          // MATLAB string with operator class
    GB_Opcode *opcode_return,           // opcode
    mxClassID *opclass_return,          // opclass
    const bool XisComplex               // true if X is complex
)
{
    (*handle) = NULL ;
    GrB_UnaryOp op = NULL ;

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
    // convert the string to a GraphBLAS unary operator, built-in or Complex
    //--------------------------------------------------------------------------

    if (XisComplex)
    {

        //----------------------------------------------------------------------
        // X complex
        //----------------------------------------------------------------------

        // user-defined Complex unary operator
        opcode  = GB_USER_R_opcode ;    // generic user-defined opcode
        opclass = mxDOUBLE_CLASS ;      // MATLAB class for complex

        if (len == 0)
        {
            op = NULL ;                 // no default Complex operator
        }

        // 7 unary operators z=f(x), both x,z are Complex (6 same as builtin)
        else if (MATCH (opname, "one"     )) { op = Complex_one      ; }
        else if (MATCH (opname, "identity")) { op = Complex_identity ; }
        else if (MATCH (opname, "ainv"    )) { op = Complex_ainv     ; }
        else if (MATCH (opname, "abs"     )) { op = Complex_abs      ; }
        else if (MATCH (opname, "minv"    )) { op = Complex_minv     ; }
        else if (MATCH (opname, "not"     )) { op = Complex_not      ; }
        // ---
        else if (MATCH (opname, "conj"    )) { op = Complex_conj     ; }

        // 4 unary operators z=f(x), x is Complex, z is double
        else if (MATCH (opname, "real"    )) { op = Complex_real     ; }
        else if (MATCH (opname, "imag"    )) { op = Complex_imag     ; }
        else if (MATCH (opname, "cabs"    )) { op = Complex_cabs     ; }
        else if (MATCH (opname, "angle"   )) { op = Complex_angle    ; }

        else
        {
            mexWarnMsgIdAndTxt ("GB:warn", "Complex op unrecognized") ;
        }

    }
    else
    {

        //----------------------------------------------------------------------
        // X is real (Z might be Complex)
        //----------------------------------------------------------------------

        // convert the opname to an opcode
        opclass = default_opclass ;         // default if no opclass specified

        if (len == 0)
        {
            // default if opname_mx is NULL
            opcode = default_opcode ;
        }

        // 6 unary operators z=f(x), x and z the same type
        else if (MATCH (opname, "one"     )) { opcode = GB_ONE_opcode ; }
        else if (MATCH (opname, "identity")) { opcode = GB_IDENTITY_opcode ; }
        else if (MATCH (opname, "ainv"    )) { opcode = GB_AINV_opcode ; }
        else if (MATCH (opname, "abs"     )) { opcode = GB_ABS_opcode ; }
        else if (MATCH (opname, "minv"    )) { opcode = GB_MINV_opcode ; }
        else if (MATCH (opname, "not"     )) { opcode = GB_LNOT_opcode ; }

        // 2 unary operators z=f(x) where x is double and z is Complex
        else if (MATCH (opname, "complex_real"))
        { 
            // z = cmplx (x,0), convert x double to real part of Complex z
            op = Complex_complex_real ;
            opcode = GB_USER_R_opcode ;
            opclass = mxDOUBLE_CLASS ;
        }
        else if (MATCH (opname, "complex_imag" ))
        { 
            // z = cmplx (0,x), convert x double to imag part of Complex z
            op = Complex_complex_imag ;
            opcode = GB_USER_R_opcode ;
            opclass = mxDOUBLE_CLASS ;
        }

        #ifdef MY_SCALE

        else if (MATCH (opname, "my_scale" ))
        { 
            // z = my_scalar*x; default value of my_scalar is 2
            op = My_scale ;
            opcode = GB_USER_C_opcode ;
            opclass = mxDOUBLE_CLASS ;
            my_scalar = 2 ;
        }

        #endif

        else
        {
            mexWarnMsgIdAndTxt ("GB:warn", "unrecognised function name") ;
            return (false) ;
        }

        if (opcode < GB_USER_C_opcode)
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

            case GB_ONE_opcode:

                switch (opclass)
                {
                    case mxLOGICAL_CLASS : op = GxB_ONE_BOOL   ; break ;
                    case mxINT8_CLASS    : op = GxB_ONE_INT8   ; break ;
                    case mxUINT8_CLASS   : op = GxB_ONE_UINT8  ; break ;
                    case mxINT16_CLASS   : op = GxB_ONE_INT16  ; break ;
                    case mxUINT16_CLASS  : op = GxB_ONE_UINT16 ; break ;
                    case mxINT32_CLASS   : op = GxB_ONE_INT32  ; break ;
                    case mxUINT32_CLASS  : op = GxB_ONE_UINT32 ; break ;
                    case mxINT64_CLASS   : op = GxB_ONE_INT64  ; break ;
                    case mxUINT64_CLASS  : op = GxB_ONE_UINT64 ; break ;
                    case mxSINGLE_CLASS  : op = GxB_ONE_FP32   ; break ;
                    case mxDOUBLE_CLASS  : op = GxB_ONE_FP64   ; break ;
                    default              : 
                        mexWarnMsgIdAndTxt ("GB:warn","unknown type") ;
                        return (false) ;
                }
                break ;

            case GB_IDENTITY_opcode:

                switch (opclass)
                {
                    case mxLOGICAL_CLASS : op = GrB_IDENTITY_BOOL   ; break ;
                    case mxINT8_CLASS    : op = GrB_IDENTITY_INT8   ; break ;
                    case mxUINT8_CLASS   : op = GrB_IDENTITY_UINT8  ; break ;
                    case mxINT16_CLASS   : op = GrB_IDENTITY_INT16  ; break ;
                    case mxUINT16_CLASS  : op = GrB_IDENTITY_UINT16 ; break ;
                    case mxINT32_CLASS   : op = GrB_IDENTITY_INT32  ; break ;
                    case mxUINT32_CLASS  : op = GrB_IDENTITY_UINT32 ; break ;
                    case mxINT64_CLASS   : op = GrB_IDENTITY_INT64  ; break ;
                    case mxUINT64_CLASS  : op = GrB_IDENTITY_UINT64 ; break ;
                    case mxSINGLE_CLASS  : op = GrB_IDENTITY_FP32   ; break ;
                    case mxDOUBLE_CLASS  : op = GrB_IDENTITY_FP64   ; break ;
                    default              : 
                        mexWarnMsgIdAndTxt ("GB:warn","unknown type") ;
                        return (false) ;
                }
                break ;

            case GB_ABS_opcode:

                switch (opclass)
                {
                    case mxLOGICAL_CLASS : op = GxB_ABS_BOOL   ; break ;
                    case mxINT8_CLASS    : op = GxB_ABS_INT8   ; break ;
                    case mxUINT8_CLASS   : op = GxB_ABS_UINT8  ; break ;
                    case mxINT16_CLASS   : op = GxB_ABS_INT16  ; break ;
                    case mxUINT16_CLASS  : op = GxB_ABS_UINT16 ; break ;
                    case mxINT32_CLASS   : op = GxB_ABS_INT32  ; break ;
                    case mxUINT32_CLASS  : op = GxB_ABS_UINT32 ; break ;
                    case mxINT64_CLASS   : op = GxB_ABS_INT64  ; break ;
                    case mxUINT64_CLASS  : op = GxB_ABS_UINT64 ; break ;
                    case mxSINGLE_CLASS  : op = GxB_ABS_FP32   ; break ;
                    case mxDOUBLE_CLASS  : op = GxB_ABS_FP64   ; break ;
                    default              : 
                        mexWarnMsgIdAndTxt ("GB:warn","unknown type") ;
                        return (false) ;
                }
                break ;

            case GB_AINV_opcode:

                switch (opclass)
                {
                    case mxLOGICAL_CLASS : op = GrB_AINV_BOOL   ; break ;
                    case mxINT8_CLASS    : op = GrB_AINV_INT8   ; break ;
                    case mxUINT8_CLASS   : op = GrB_AINV_UINT8  ; break ;
                    case mxINT16_CLASS   : op = GrB_AINV_INT16  ; break ;
                    case mxUINT16_CLASS  : op = GrB_AINV_UINT16 ; break ;
                    case mxINT32_CLASS   : op = GrB_AINV_INT32  ; break ;
                    case mxUINT32_CLASS  : op = GrB_AINV_UINT32 ; break ;
                    case mxINT64_CLASS   : op = GrB_AINV_INT64  ; break ;
                    case mxUINT64_CLASS  : op = GrB_AINV_UINT64 ; break ;
                    case mxSINGLE_CLASS  : op = GrB_AINV_FP32   ; break ;
                    case mxDOUBLE_CLASS  : op = GrB_AINV_FP64   ; break ;
                    default              : 
                        mexWarnMsgIdAndTxt ("GB:warn","unknown type") ;
                        return (false) ;
                }
                break ;

            case GB_MINV_opcode   :

                switch (opclass)
                {
                    case mxLOGICAL_CLASS : op = GrB_MINV_BOOL   ; break ;
                    case mxINT8_CLASS    : op = GrB_MINV_INT8   ; break ;
                    case mxUINT8_CLASS   : op = GrB_MINV_UINT8  ; break ;
                    case mxINT16_CLASS   : op = GrB_MINV_INT16  ; break ;
                    case mxUINT16_CLASS  : op = GrB_MINV_UINT16 ; break ;
                    case mxINT32_CLASS   : op = GrB_MINV_INT32  ; break ;
                    case mxUINT32_CLASS  : op = GrB_MINV_UINT32 ; break ;
                    case mxINT64_CLASS   : op = GrB_MINV_INT64  ; break ;
                    case mxUINT64_CLASS  : op = GrB_MINV_UINT64 ; break ;
                    case mxSINGLE_CLASS  : op = GrB_MINV_FP32   ; break ;
                    case mxDOUBLE_CLASS  : op = GrB_MINV_FP64   ; break ;
                    default              : 
                        mexWarnMsgIdAndTxt ("GB:warn","unknown type") ;
                        return (false) ;
                }
                break ;

            case GB_LNOT_opcode   :

                switch (opclass)
                {
                    case mxLOGICAL_CLASS : op = GrB_LNOT        ; break ;
                    case mxINT8_CLASS    : op = GxB_LNOT_INT8   ; break ;
                    case mxUINT8_CLASS   : op = GxB_LNOT_UINT8  ; break ;
                    case mxINT16_CLASS   : op = GxB_LNOT_INT16  ; break ;
                    case mxUINT16_CLASS  : op = GxB_LNOT_UINT16 ; break ;
                    case mxINT32_CLASS   : op = GxB_LNOT_INT32  ; break ;
                    case mxUINT32_CLASS  : op = GxB_LNOT_UINT32 ; break ;
                    case mxINT64_CLASS   : op = GxB_LNOT_INT64  ; break ;
                    case mxUINT64_CLASS  : op = GxB_LNOT_UINT64 ; break ;
                    case mxSINGLE_CLASS  : op = GxB_LNOT_FP32   ; break ;
                    case mxDOUBLE_CLASS  : op = GxB_LNOT_FP64   ; break ;
                    default              : 
                        mexWarnMsgIdAndTxt ("GB:warn","unknown type") ;
                        return (false) ;
                }
                break ;

            case GB_NOP_opcode   :
            case GB_USER_R_opcode   :
            case GB_USER_C_opcode   :

                // no operation is requested so return NULL, or user-defined
                break ;

            default : 
                mexWarnMsgIdAndTxt ("GB:warn","unknown unary operator") ;
                return (false) ;
        }

    }

    //--------------------------------------------------------------------------
    // return result
    //--------------------------------------------------------------------------

    // return the opclass and opcode to the caller
    if (opclass_return != NULL) *opclass_return = opclass ;
    if (opcode_return  != NULL) *opcode_return  = opcode ;

    // return the unary operator to the caller
    ASSERT_OK (GB_check (op, "got unary op", GB0)) ;
    (*handle) = op ;
    return (true) ;
}

