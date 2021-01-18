//------------------------------------------------------------------------------
// GraphBLAS/Demo/Include/usercomplex.h:  complex numbers as a user-defined type
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#ifndef USERCOMPLEX_H
#define USERCOMPLEX_H

//------------------------------------------------------------------------------
// 10 binary functions, z=f(x,y), where CxC -> C
//------------------------------------------------------------------------------

GB_PUBLIC
GrB_BinaryOp Complex_first , Complex_second , Complex_min ,
             Complex_max   , Complex_plus   , Complex_minus ,
             Complex_times , Complex_div    , Complex_rdiv  ,
             Complex_rminus, Complex_pair ;

//------------------------------------------------------------------------------
// 6 binary comparison functions, z=f(x,y), where CxC -> C
//------------------------------------------------------------------------------

GB_PUBLIC
GrB_BinaryOp Complex_iseq , Complex_isne ,
             Complex_isgt , Complex_islt ,
             Complex_isge , Complex_isle ;

//------------------------------------------------------------------------------
// 3 binary boolean functions, z=f(x,y), where CxC -> C
//------------------------------------------------------------------------------

GB_PUBLIC
GrB_BinaryOp Complex_or , Complex_and , Complex_xor ;

//------------------------------------------------------------------------------
// 6 binary comparison functions, z=f(x,y), where CxC -> bool
//------------------------------------------------------------------------------

GB_PUBLIC
GrB_BinaryOp Complex_eq , Complex_ne ,
             Complex_gt , Complex_lt ,
             Complex_ge , Complex_le ;

//------------------------------------------------------------------------------
// 1 binary function, z=f(x,y), where double x double -> C
//------------------------------------------------------------------------------

GB_PUBLIC GrB_BinaryOp Complex_complex ;

//------------------------------------------------------------------------------
// 5 unary functions, z=f(x) where C -> C
//------------------------------------------------------------------------------

GB_PUBLIC
GrB_UnaryOp  Complex_identity , Complex_ainv , Complex_minv ,
             Complex_not ,      Complex_conj,
             Complex_one ,      Complex_abs  ;

//------------------------------------------------------------------------------
// 4 unary functions, z=f(x) where C -> double
//------------------------------------------------------------------------------

GB_PUBLIC 
GrB_UnaryOp Complex_real, Complex_imag,
            Complex_cabs, Complex_angle ;

//------------------------------------------------------------------------------
// 2 unary functions, z=f(x) where double -> C
//------------------------------------------------------------------------------

GB_PUBLIC GrB_UnaryOp Complex_complex_real, Complex_complex_imag ;

//------------------------------------------------------------------------------
// Complex type, scalars, monoids, and semiring
//------------------------------------------------------------------------------

GB_PUBLIC GrB_Type Complex ;
GB_PUBLIC GrB_Monoid   Complex_plus_monoid, Complex_times_monoid ;
GB_PUBLIC GrB_Semiring Complex_plus_times ;
GB_PUBLIC GrB_Info Complex_init (bool builtin_complex) ;
GB_PUBLIC GrB_Info Complex_finalize ( ) ;

//------------------------------------------------------------------------------
// C++ compatibility
//------------------------------------------------------------------------------

#if defined ( __cplusplus )

    using namespace std ;

    #define crealf(x)   real(x)
    #define creal(x)    real(x)
    #define cimagf(x)   imag(x)
    #define cimag(x)    imag(x)
    #define cpowf(x,y)  pow(x,y)
    #define cpow(x,y)   pow(x,y)
    #define powf(x,y)   pow(x,y)
    #define cexpf(x)    exp(x)
    #define cexp(x)     exp(x)
    #define expf(x)     exp(x)

    #define clogf(x)    log(x)
    #define clog(x)     log(x)
    #define logf(x)     log(x)

    #define cabsf(x)    abs(x)
    #define cabs(x)     abs(x)
    #define absf(x)     abs(x)

    #define csqrtf(x)   sqrt(x)
    #define csqrt(x)    sqrt(x)
    #define sqrtf(x)    sqrt(x)

    #define conjf(x)    conj(x)

    #define cargf(x)    arg(x)
    #define carg(x)     arg(x)

    #define csinf(x)    sin(x)
    #define csin(x)     sin(x)
    #define sinf(x)     sin(x)
    #define ccosf(x)    cos(x)
    #define ccos(x)     cos(x)
    #define cosf(x)     cos(x)
    #define ctanf(x)    tan(x)
    #define ctan(x)     tan(x)
    #define tanf(x)     tan(x)

    #define casinf(x)   asin(x)
    #define casin(x)    asin(x)
    #define asinf(x)    asin(x)
    #define cacosf(x)   acos(x)
    #define cacos(x)    acos(x)
    #define acosf(x)    acos(x)
    #define catanf(x)   atan(x)
    #define catan(x)    atan(x)
    #define atanf(x)    atan(x)

    #define csinhf(x)   sinh(x)
    #define csinh(x)    sinh(x)
    #define sinhf(x)    sinh(x)
    #define ccoshf(x)   cosh(x)
    #define ccosh(x)    cosh(x)
    #define coshf(x)    cosh(x)
    #define ctanhf(x)   tanh(x)
    #define ctanh(x)    tanh(x)
    #define tanhf(x)    tanh(x)

    #define casinhf(x)  asinh(x)
    #define casinh(x)   asinh(x)
    #define asinhf(x)   asinh(x)
    #define cacoshf(x)  acosh(x)
    #define cacosh(x)   acosh(x)
    #define acoshf(x)   acosh(x)
    #define catanhf(x)  atanh(x)
    #define catanh(x)   atanh(x)
    #define atanhf(x)   atanh(x)

#endif


#endif

