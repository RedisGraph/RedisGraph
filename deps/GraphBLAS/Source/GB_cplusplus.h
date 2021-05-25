//------------------------------------------------------------------------------
// GB_cplusplus.h: definitions for C++
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// These macros allow GraphBLAS to be compiled with a C++ compiler.  See:
// https://www.drdobbs.com/complex-arithmetic-in-the-intersection-o/184401628#

#ifndef GB_CPLUSPLUS_H
#define GB_CPLUSPLUS_H

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
