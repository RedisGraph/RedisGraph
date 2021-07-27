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

        case GB_NOP_opcode       : return ("nop") ;    // no operation

        //======================================================================
        // unary operators
        //======================================================================

        //----------------------------------------------------------------------
        // primary unary operators x=f(x)
        //----------------------------------------------------------------------

        case GB_ONE_opcode       : return ("one") ;         // z = 1
        case GB_IDENTITY_opcode  : return ("identity") ;    // z = x
        case GB_AINV_opcode      : return ("ainv") ;        // z = -x
        case GB_ABS_opcode       : return ("abs") ;         // z = abs(x)
        case GB_MINV_opcode      : return ("minv") ;        // z = 1/x
        case GB_LNOT_opcode      : return ("lnot") ;        // z = !x
        case GB_BNOT_opcode      : return ("bnot") ;        // z = ~x

        //----------------------------------------------------------------------
        // unary operators for floating-point types (real and complex)
        //----------------------------------------------------------------------

        case GB_SQRT_opcode      : return ("sqrt") ;    // z = sqrt (x)
        case GB_LOG_opcode       : return ("log") ;     // z = log (x)
        case GB_EXP_opcode       : return ("exp") ;     // z = exp (x)

        case GB_SIN_opcode       : return ("sin") ;     // z = sin (x)
        case GB_COS_opcode       : return ("cos") ;     // z = cos (x)
        case GB_TAN_opcode       : return ("tan") ;     // z = tan (x)

        case GB_ASIN_opcode      : return ("asin") ;    // z = asin (x)
        case GB_ACOS_opcode      : return ("acos") ;    // z = acos (x)
        case GB_ATAN_opcode      : return ("atan") ;    // z = atan (x)

        case GB_SINH_opcode      : return ("sinh") ;    // z = sinh (x)
        case GB_COSH_opcode      : return ("cosh") ;    // z = cosh (x)
        case GB_TANH_opcode      : return ("tanh") ;    // z = tanh (x)

        case GB_ASINH_opcode     : return ("asinh") ;   // z = asinh (x)
        case GB_ACOSH_opcode     : return ("acosh") ;   // z = acosh (x)
        case GB_ATANH_opcode     : return ("atanh") ;   // z = atanh (x)

        case GB_SIGNUM_opcode    : return ("signum") ;  // z = signum (x)
        case GB_CEIL_opcode      : return ("ceil") ;    // z = ceil (x)
        case GB_FLOOR_opcode     : return ("floor") ;   // z = floor (x)
        case GB_ROUND_opcode     : return ("round") ;   // z = round (x)
        case GB_TRUNC_opcode     : return ("trunc") ;   // z = trunc (x)

        case GB_EXP2_opcode      : return ("exp2") ;    // z = exp2 (x)
        case GB_EXPM1_opcode     : return ("expm1") ;   // z = expm1 (x)
        case GB_LOG10_opcode     : return ("log10") ;   // z = log10 (x)
        case GB_LOG1P_opcode     : return ("log1p") ;   // z = log1P (x)
        case GB_LOG2_opcode      : return ("log2") ;    // z = log2 (x)

        //----------------------------------------------------------------------
        // unary operators for real floating-point types
        //----------------------------------------------------------------------

        case GB_LGAMMA_opcode    : return ("lgamma") ;  // z = lgamma (x)
        case GB_TGAMMA_opcode    : return ("tgamma") ;  // z = tgamma (x)
        case GB_ERF_opcode       : return ("erf") ;     // z = erf (x)
        case GB_ERFC_opcode      : return ("erfc") ;    // z = erfc (x)
        case GB_FREXPX_opcode    : return ("frexpx") ;  // z = frexpx (x)
        case GB_FREXPE_opcode    : return ("frexpe") ;  // z = frexpe (x)

        //----------------------------------------------------------------------
        // unary operators for complex types only
        //----------------------------------------------------------------------

        case GB_CONJ_opcode      : return ("conj") ;    // z = conj (x)

        //----------------------------------------------------------------------
        // unary operators where z is real and x is complex
        //----------------------------------------------------------------------

        case GB_CREAL_opcode     : return ("creal") ;   // z = creal (x)
        case GB_CIMAG_opcode     : return ("cimag") ;   // z = cimag (x)
        case GB_CARG_opcode      : return ("carg") ;    // z = carg (x)

        //----------------------------------------------------------------------
        // unary operators where z is bool and x is any floating-point type
        //----------------------------------------------------------------------

        case GB_ISINF_opcode     : return ("isinf") ;       // z = isinf (x)
        case GB_ISNAN_opcode     : return ("isnan") ;       // z = isnan (x)
        case GB_ISFINITE_opcode  : return ("isfinite") ;    // z = isfinite (x)

        //----------------------------------------------------------------------
        // positional unary operators: z is int64, x is ignored
        //----------------------------------------------------------------------

        case GB_POSITIONI_opcode     : return ("positioni") ;
        case GB_POSITIONI1_opcode    : return ("positioni1") ;
        case GB_POSITIONJ_opcode     : return ("positionj") ;
        case GB_POSITIONJ1_opcode    : return ("positionj1") ;

        //======================================================================
        // binary operators
        //======================================================================

        //----------------------------------------------------------------------
        // binary operators z=f(x,y) that return the same type as their inputs
        //----------------------------------------------------------------------

        case GB_FIRST_opcode     : return ("first") ;  // z = x
        case GB_SECOND_opcode    : return ("second") ; // z = y
        case GB_ANY_opcode       : return ("any") ;    // z = pick x or y
        case GB_PAIR_opcode      : return ("pair") ;   // z = 1
        case GB_MIN_opcode       : return ("min") ;    // z = min(x,y)
        case GB_MAX_opcode       : return ("max") ;    // z = max(x,y)
        case GB_PLUS_opcode      : return ("plus") ;   // z = x + y
        case GB_MINUS_opcode     : return ("minus") ;  // z = x - y
        case GB_RMINUS_opcode    : return ("rminus") ; // z = y - x
        case GB_TIMES_opcode     : return ("times") ;  // z = x * y
        case GB_DIV_opcode       : return ("div") ;    // z = x / y
        case GB_RDIV_opcode      : return ("rdiv") ;   // z = y / x
        case GB_POW_opcode       : return ("pow") ;    // z = pow (x,y)

        case GB_ISEQ_opcode      : return ("iseq") ;   // z = (x == y)
        case GB_ISNE_opcode      : return ("isne") ;   // z = (x != y)
        case GB_ISGT_opcode      : return ("isgt") ;   // z = (x >  y)
        case GB_ISLT_opcode      : return ("islt") ;   // z = (x <  y)
        case GB_ISGE_opcode      : return ("isge") ;   // z = (x >= y)
        case GB_ISLE_opcode      : return ("isle") ;   // z = (x <= y)

        case GB_LOR_opcode       : return ("lor") ;    // z = x || y
        case GB_LAND_opcode      : return ("land") ;   // z = x && y
        case GB_LXOR_opcode      : return ("lxor") ;   // z = x != y

        case GB_BOR_opcode       : return ("bor") ;     // z = (x | y), bitwise
        case GB_BAND_opcode      : return ("band") ;    // z = (x & y), bitwise
        case GB_BXOR_opcode      : return ("bxor") ;    // z = (x ^ y), bitwise
        case GB_BXNOR_opcode     : return ("bxnor") ;   // z = ~(x ^ y), bitwise
        case GB_BGET_opcode      : return ("bget") ;    // z = bitget (x,y)
        case GB_BSET_opcode      : return ("bset") ;    // z = bitset (x,y)
        case GB_BCLR_opcode      : return ("bclr") ;    // z = bitclr (x,y)
        case GB_BSHIFT_opcode    : return ("bshift") ;  // z = bitshift (x,y)

        //----------------------------------------------------------------------
        // binary operators z=f(x,y) that return bool (TxT -> bool)
        //----------------------------------------------------------------------

        case GB_EQ_opcode        : return ("eq") ;   // z = (x == y)
        case GB_NE_opcode        : return ("ne") ;   // z = (x != y)
        case GB_GT_opcode        : return ("gt") ;   // z = (x >  y)
        case GB_LT_opcode        : return ("lt") ;   // z = (x <  y)
        case GB_GE_opcode        : return ("ge") ;   // z = (x >= y)
        case GB_LE_opcode        : return ("le") ;   // z = (x <= y)

        //----------------------------------------------------------------------
        // binary operators for real floating-point types (TxT -> T)
        //----------------------------------------------------------------------

        case GB_ATAN2_opcode     : return ("atan2") ;       // z = atan2 (x,y)
        case GB_HYPOT_opcode     : return ("hypot") ;       // z = hypot (x,y)
        case GB_FMOD_opcode      : return ("fmod") ;        // z = fmod (x,y)
        case GB_REMAINDER_opcode : return ("remainder") ;   // z=remainder(x,y)
        case GB_COPYSIGN_opcode  : return ("copysign") ;    // z=copysign (x,y)
        case GB_LDEXP_opcode     : return ("ldexp") ;       // z = ldexp (x,y)

        //----------------------------------------------------------------------
        // binary operator z=f(x,y) where z is complex, x,y real:
        //----------------------------------------------------------------------

        case GB_CMPLX_opcode     : return ("cmplx") ;       // z = cmplx (x,y)

        //----------------------------------------------------------------------
        // positional binary operators: z is int64, x and y are ignored
        //----------------------------------------------------------------------

        case GB_FIRSTI_opcode    : return ("firsti") ;
        case GB_FIRSTI1_opcode   : return ("firsti1") ;
        case GB_FIRSTJ_opcode    : return ("firstj") ;
        case GB_FIRSTJ1_opcode   : return ("firstj1") ;

        case GB_SECONDI_opcode   : return ("secondi") ;
        case GB_SECONDI1_opcode  : return ("secondi1") ;
        case GB_SECONDJ_opcode   : return ("secondj") ;
        case GB_SECONDJ1_opcode  : return ("secondj1") ;

        //======================================================================
        // user-defined: unary and binary operators
        //======================================================================

        case GB_USER_opcode : return ("user") ;             // unary or binary

        //======================================================================
        // invalid opcode
        //======================================================================

        default : break ;
    }

    return ("") ;
}

