//------------------------------------------------------------------------------
// GB_stringify_opcode: convert unary or binary opcode to its name
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#include "GB.h"
#include "GB_stringify.h"

const char *GB_stringify_opcode    // name of unary/binary opcode
(
    GB_Opcode opcode    // opcode of GraphBLAS unary or binary operator
)
{

    switch (opcode)
    {

        //----------------------------------------------------------------------
        // NOP
        //----------------------------------------------------------------------

        case GB_NOP_code         : return ("nop") ;    // no operation

        //======================================================================
        // unary operators
        //======================================================================

        //----------------------------------------------------------------------
        // primary unary operators x=f(x)
        //----------------------------------------------------------------------

        case GB_ONE_unop_code       : return ("one") ;         // z = 1
        case GB_IDENTITY_unop_code  : return ("identity") ;    // z = x
        case GB_AINV_unop_code      : return ("ainv") ;        // z = -x
        case GB_ABS_unop_code       : return ("abs") ;         // z = abs(x)
        case GB_MINV_unop_code      : return ("minv") ;        // z = 1/x
        case GB_LNOT_unop_code      : return ("lnot") ;        // z = !x
        case GB_BNOT_unop_code      : return ("bnot") ;        // z = ~x

        //----------------------------------------------------------------------
        // unary operators for floating-point types (real and complex)
        //----------------------------------------------------------------------

        case GB_SQRT_unop_code      : return ("sqrt") ;    // z = sqrt (x)
        case GB_LOG_unop_code       : return ("log") ;     // z = log (x)
        case GB_EXP_unop_code       : return ("exp") ;     // z = exp (x)

        case GB_SIN_unop_code       : return ("sin") ;     // z = sin (x)
        case GB_COS_unop_code       : return ("cos") ;     // z = cos (x)
        case GB_TAN_unop_code       : return ("tan") ;     // z = tan (x)

        case GB_ASIN_unop_code      : return ("asin") ;    // z = asin (x)
        case GB_ACOS_unop_code      : return ("acos") ;    // z = acos (x)
        case GB_ATAN_unop_code      : return ("atan") ;    // z = atan (x)

        case GB_SINH_unop_code      : return ("sinh") ;    // z = sinh (x)
        case GB_COSH_unop_code      : return ("cosh") ;    // z = cosh (x)
        case GB_TANH_unop_code      : return ("tanh") ;    // z = tanh (x)

        case GB_ASINH_unop_code     : return ("asinh") ;   // z = asinh (x)
        case GB_ACOSH_unop_code     : return ("acosh") ;   // z = acosh (x)
        case GB_ATANH_unop_code     : return ("atanh") ;   // z = atanh (x)

        case GB_SIGNUM_unop_code    : return ("signum") ;  // z = signum (x)
        case GB_CEIL_unop_code      : return ("ceil") ;    // z = ceil (x)
        case GB_FLOOR_unop_code     : return ("floor") ;   // z = floor (x)
        case GB_ROUND_unop_code     : return ("round") ;   // z = round (x)
        case GB_TRUNC_unop_code     : return ("trunc") ;   // z = trunc (x)

        case GB_EXP2_unop_code      : return ("exp2") ;    // z = exp2 (x)
        case GB_EXPM1_unop_code     : return ("expm1") ;   // z = expm1 (x)
        case GB_LOG10_unop_code     : return ("log10") ;   // z = log10 (x)
        case GB_LOG1P_unop_code     : return ("log1p") ;   // z = log1P (x)
        case GB_LOG2_unop_code      : return ("log2") ;    // z = log2 (x)

        //----------------------------------------------------------------------
        // unary operators for real floating-point types
        //----------------------------------------------------------------------

        case GB_LGAMMA_unop_code    : return ("lgamma") ;  // z = lgamma (x)
        case GB_TGAMMA_unop_code    : return ("tgamma") ;  // z = tgamma (x)
        case GB_ERF_unop_code       : return ("erf") ;     // z = erf (x)
        case GB_ERFC_unop_code      : return ("erfc") ;    // z = erfc (x)
        case GB_FREXPX_unop_code    : return ("frexpx") ;  // z = frexpx (x)
        case GB_FREXPE_unop_code    : return ("frexpe") ;  // z = frexpe (x)

        //----------------------------------------------------------------------
        // unary operators for complex types only
        //----------------------------------------------------------------------

        case GB_CONJ_unop_code      : return ("conj") ;    // z = conj (x)

        //----------------------------------------------------------------------
        // unary operators where z is real and x is complex
        //----------------------------------------------------------------------

        case GB_CREAL_unop_code     : return ("creal") ;   // z = creal (x)
        case GB_CIMAG_unop_code     : return ("cimag") ;   // z = cimag (x)
        case GB_CARG_unop_code      : return ("carg") ;    // z = carg (x)

        //----------------------------------------------------------------------
        // unary operators where z is bool and x is any floating-point type
        //----------------------------------------------------------------------

        case GB_ISINF_unop_code     : return ("isinf") ;    // z = isinf (x)
        case GB_ISNAN_unop_code     : return ("isnan") ;    // z = isnan (x)
        case GB_ISFINITE_unop_code  : return ("isfinite") ; // z = isfinite (x)

        //----------------------------------------------------------------------
        // positional unary operators: z is int64, x is ignored
        //----------------------------------------------------------------------

        case GB_POSITIONI_unop_code     : return ("positioni") ;
        case GB_POSITIONI1_unop_code    : return ("positioni1") ;
        case GB_POSITIONJ_unop_code     : return ("positionj") ;
        case GB_POSITIONJ1_unop_code    : return ("positionj1") ;

        //======================================================================
        // binary operators
        //======================================================================

        //----------------------------------------------------------------------
        // binary operators z=f(x,y) that return the same type as their inputs
        //----------------------------------------------------------------------

        case GB_FIRST_binop_code     : return ("first") ;  // z = x
        case GB_SECOND_binop_code    : return ("second") ; // z = y
        case GB_ANY_binop_code       : return ("any") ;    // z = pick x or y
        case GB_PAIR_binop_code      : return ("pair") ;   // z = 1
        case GB_MIN_binop_code       : return ("min") ;    // z = min(x,y)
        case GB_MAX_binop_code       : return ("max") ;    // z = max(x,y)
        case GB_PLUS_binop_code      : return ("plus") ;   // z = x + y
        case GB_MINUS_binop_code     : return ("minus") ;  // z = x - y
        case GB_RMINUS_binop_code    : return ("rminus") ; // z = y - x
        case GB_TIMES_binop_code     : return ("times") ;  // z = x * y
        case GB_DIV_binop_code       : return ("div") ;    // z = x / y
        case GB_RDIV_binop_code      : return ("rdiv") ;   // z = y / x
        case GB_POW_binop_code       : return ("pow") ;    // z = pow (x,y)

        case GB_ISEQ_binop_code      : return ("iseq") ;   // z = (x == y)
        case GB_ISNE_binop_code      : return ("isne") ;   // z = (x != y)
        case GB_ISGT_binop_code      : return ("isgt") ;   // z = (x >  y)
        case GB_ISLT_binop_code      : return ("islt") ;   // z = (x <  y)
        case GB_ISGE_binop_code      : return ("isge") ;   // z = (x >= y)
        case GB_ISLE_binop_code      : return ("isle") ;   // z = (x <= y)

        case GB_LOR_binop_code       : return ("lor") ;    // z = x || y
        case GB_LAND_binop_code      : return ("land") ;   // z = x && y
        case GB_LXOR_binop_code      : return ("lxor") ;   // z = x != y

        case GB_BOR_binop_code       : return ("bor") ;     // z = (x | y), bitwise
        case GB_BAND_binop_code      : return ("band") ;    // z = (x & y), bitwise
        case GB_BXOR_binop_code      : return ("bxor") ;    // z = (x ^ y), bitwise
        case GB_BXNOR_binop_code     : return ("bxnor") ;   // z = ~(x ^ y), bitwise
        case GB_BGET_binop_code      : return ("bget") ;    // z = bitget (x,y)
        case GB_BSET_binop_code      : return ("bset") ;    // z = bitset (x,y)
        case GB_BCLR_binop_code      : return ("bclr") ;    // z = bitclr (x,y)
        case GB_BSHIFT_binop_code    : return ("bshift") ;  // z = bitshift (x,y)

        //----------------------------------------------------------------------
        // binary operators z=f(x,y) that return bool (TxT -> bool)
        //----------------------------------------------------------------------

        case GB_EQ_binop_code        : return ("eq") ;   // z = (x == y)
        case GB_NE_binop_code        : return ("ne") ;   // z = (x != y)
        case GB_GT_binop_code        : return ("gt") ;   // z = (x >  y)
        case GB_LT_binop_code        : return ("lt") ;   // z = (x <  y)
        case GB_GE_binop_code        : return ("ge") ;   // z = (x >= y)
        case GB_LE_binop_code        : return ("le") ;   // z = (x <= y)

        //----------------------------------------------------------------------
        // binary operators for real floating-point types (TxT -> T)
        //----------------------------------------------------------------------

        case GB_ATAN2_binop_code     : return ("atan2") ;       // z = atan2 (x,y)
        case GB_HYPOT_binop_code     : return ("hypot") ;       // z = hypot (x,y)
        case GB_FMOD_binop_code      : return ("fmod") ;        // z = fmod (x,y)
        case GB_REMAINDER_binop_code : return ("remainder") ;   // z=remainder(x,y)
        case GB_COPYSIGN_binop_code  : return ("copysign") ;    // z=copysign (x,y)
        case GB_LDEXP_binop_code     : return ("ldexp") ;       // z = ldexp (x,y)

        //----------------------------------------------------------------------
        // binary operator z=f(x,y) where z is complex, x,y real:
        //----------------------------------------------------------------------

        case GB_CMPLX_binop_code     : return ("cmplx") ;       // z = cmplx (x,y)

        //----------------------------------------------------------------------
        // positional binary operators: z is int64, x and y are ignored
        //----------------------------------------------------------------------

        case GB_FIRSTI_binop_code    : return ("firsti") ;
        case GB_FIRSTI1_binop_code   : return ("firsti1") ;
        case GB_FIRSTJ_binop_code    : return ("firstj") ;
        case GB_FIRSTJ1_binop_code   : return ("firstj1") ;

        case GB_SECONDI_binop_code   : return ("secondi") ;
        case GB_SECONDI1_binop_code  : return ("secondi1") ;
        case GB_SECONDJ_binop_code   : return ("secondj") ;
        case GB_SECONDJ1_binop_code  : return ("secondj1") ;

        //======================================================================
        // user-defined: unary and binary operators
        //======================================================================

        case GB_USER_unop_code : return ("user_unop") ;
        case GB_USER_binop_code : return ("user_binop") ;

        //======================================================================
        // invalid opcode
        //======================================================================

        default : break ;
    }

    return ("") ;
}

