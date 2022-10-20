//------------------------------------------------------------------------------
// GB_math.h: definitions for complex types, and mathematical operators
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#ifndef GB_MATH_H
#define GB_MATH_H

//------------------------------------------------------------------------------
// CUDA vs CPU math functions
//------------------------------------------------------------------------------

#ifdef GB_CUDA_KERNEL
// fixme for CUDA: this could likely be: "__device__ static inline"
#define GB_MATH_KERNEL __device__ inline
#else
#define GB_MATH_KERNEL inline
#endif

//------------------------------------------------------------------------------
// complex macros
//------------------------------------------------------------------------------

#if GB_COMPILER_MSC

    //--------------------------------------------------------------------------
    // Microsoft Visual Studio compiler with its own complex type
    //--------------------------------------------------------------------------

    // complex-complex multiply: z = x*y where both x and y are complex
    #define GB_FC32_mul(x,y) (_FCmulcc (x, y))
    #define GB_FC64_mul(x,y) ( _Cmulcc (x, y))

    // complex-real multiply: z = x*y where x is complex and y is real
    #define GB_FC32_rmul(x,y) (_FCmulcr (x, y))
    #define GB_FC64_rmul(x,y) ( _Cmulcr (x, y))

    // complex-complex addition: z = x+y where both x and y are complex
    #define GB_FC32_add(x,y) \
        GxB_CMPLXF (crealf (x) + crealf (y), cimagf (x) + cimagf (y))
    #define GB_FC64_add(x,y) \
        GxB_CMPLX  (creal  (x) + creal  (y), cimag  (x) + cimag  (y))

    // complex-complex subtraction: z = x-y where both x and y are complex
    #define GB_FC32_minus(x,y) \
        GxB_CMPLXF (crealf (x) - crealf (y), cimagf (x) - cimagf (y))
    #define GB_FC64_minus(x,y) \
        GxB_CMPLX  (creal  (x) - creal  (y), cimag  (x) - cimag  (y))

    // complex negation: z = -x
    #define GB_FC32_ainv(x) GxB_CMPLXF (-crealf (x), -cimagf (x))
    #define GB_FC64_ainv(x) GxB_CMPLX (-creal  (x), -cimag  (x))

#else

    //--------------------------------------------------------------------------
    // native complex type support
    //--------------------------------------------------------------------------

    // complex-complex multiply: z = x*y where both x and y are complex
    #define GB_FC32_mul(x,y) ((x) * (y))
    #define GB_FC64_mul(x,y) ((x) * (y))

    // complex-real multiply: z = x*y where x is complex and y is real
    #define GB_FC32_rmul(x,y) ((x) * (y))
    #define GB_FC64_rmul(x,y) ((x) * (y))

    // complex-complex addition: z = x+y where both x and y are complex
    #define GB_FC32_add(x,y) ((x) + (y))
    #define GB_FC64_add(x,y) ((x) + (y))

    // complex-complex subtraction: z = x-y where both x and y are complex
    #define GB_FC32_minus(x,y) ((x) - (y))
    #define GB_FC64_minus(x,y) ((x) - (y))

    // complex negation
    #define GB_FC32_ainv(x) (-(x))
    #define GB_FC64_ainv(x) (-(x))

#endif

// complex inverse: z = 1/x
#define GB_FC32_minv(x) GB_FC32_div (GxB_CMPLXF (1,0), x)
#define GB_FC64_minv(x) GB_FC64_div (GxB_CMPLX (1,0), x)

// complex comparators
#define GB_FC32_eq(x,y) ((crealf(x) == crealf(y)) && (cimagf(x) == cimagf(y)))
#define GB_FC64_eq(x,y) ((creal (x) == creal (y)) && (cimag (x) == cimag (y)))

#define GB_FC32_ne(x,y) ((crealf(x) != crealf(y)) || (cimagf(x) != cimagf(y)))
#define GB_FC64_ne(x,y) ((creal (x) != creal (y)) || (cimag (x) != cimag (y)))

#define GB_FC32_iseq(x,y) GxB_CMPLXF ((float)  GB_FC32_eq (x,y), 0)
#define GB_FC64_iseq(x,y) GxB_CMPLX  ((double) GB_FC64_eq (x,y), 0)

#define GB_FC32_isne(x,y) GxB_CMPLXF ((float)  GB_FC32_ne (x,y), 0)
#define GB_FC64_isne(x,y) GxB_CMPLX  ((double) GB_FC64_ne (x,y), 0)

#define GB_FC32_eq0(x) ((crealf(x) == 0) && (cimagf(x) == 0))
#define GB_FC64_eq0(x) ((creal (x) == 0) && (cimag (x) == 0))

#define GB_FC32_ne0(x) ((crealf(x) != 0) || (cimagf(x) != 0))
#define GB_FC64_ne0(x) ((creal (x) != 0) || (cimag (x) != 0))

//------------------------------------------------------------------------------
// min, max, and NaN handling
//------------------------------------------------------------------------------

// For floating-point computations, SuiteSparse:GraphBLAS relies on the IEEE
// 754 standard for the basic operations (+ - / *).  Comparator also
// work as they should; any compare with NaN is always false, even
// eq(NaN,NaN) is false.  This follows the IEEE 754 standard.

// For integer MIN and MAX, SuiteSparse:GraphBLAS relies on one compator:
// z = min(x,y) = (x < y) ? x : y
// z = max(x,y) = (x > y) ? x : y

// However, this is not suitable for floating-point x and y.  Compares with
// NaN always return false, so if either x or y are NaN, then z = y, for both
// min(x,y) and max(x,y).

// The ANSI C11 fmin, fminf, fmax, and fmaxf functions have the 'omitnan'
// behavior.  These are used in SuiteSparse:GraphBLAS v2.3.0 and later.

// for integers only:
#define GB_IABS(x) (((x) >= 0) ? (x) : (-(x)))

// suitable for integers, and non-NaN floating point:
#include "GB_imin.h"

// ceiling of a/b for two integers a and b
#include "GB_iceil.h"

//------------------------------------------------------------------------------
// division by zero
//------------------------------------------------------------------------------

// Integer division is done carefully so that GraphBLAS does not terminate the
// user's application on divide-by-zero.  To compute x/0: if x is zero, the
// result is zero (like NaN).  if x is negative, the result is the negative
// integer with biggest magnitude (like -infinity).  if x is positive, the
// result is the biggest positive integer (like +infinity).

// For places affected by this decision in the code do:
// grep "integer division"

// Signed and unsigned integer division, z = x/y.  The bits parameter can be 8,
// 16, 32, or 64.
#define GB_INT_MIN(bits)  INT ## bits ## _MIN
#define GB_INT_MAX(bits)  INT ## bits ## _MAX
#define GB_UINT_MAX(bits) UINT ## bits ## _MAX

// x/y when x and y are signed integers
#define GB_IDIV_SIGNED(x,y,bits)                                            \
(                                                                           \
    ((y) == -1) ?                                                           \
    (                                                                       \
        /* INT32_MIN/(-1) causes floating point exception; avoid it  */     \
        -(x)                                                                \
    )                                                                       \
    :                                                                       \
    (                                                                       \
        ((y) == 0) ?                                                        \
        (                                                                   \
            /* x/0 */                                                       \
            ((x) == 0) ?                                                    \
            (                                                               \
                /* zero divided by zero gives 'Nan' */                      \
                0                                                           \
            )                                                               \
            :                                                               \
            (                                                               \
                /* x/0 and x is nonzero */                                  \
                ((x) < 0) ?                                                 \
                (                                                           \
                    /* x is negative: x/0 gives '-Inf' */                   \
                    GB_INT_MIN (bits)                                       \
                )                                                           \
                :                                                           \
                (                                                           \
                    /* x is positive: x/0 gives '+Inf' */                   \
                    GB_INT_MAX (bits)                                       \
                )                                                           \
            )                                                               \
        )                                                                   \
        :                                                                   \
        (                                                                   \
            /* normal case for signed integer division */                   \
            (x) / (y)                                                       \
        )                                                                   \
    )                                                                       \
)

GB_MATH_KERNEL int8_t GB_idiv_int8 (int8_t x, int8_t y)
{
    return (GB_IDIV_SIGNED (x, y, 8)) ;
}

GB_MATH_KERNEL int16_t GB_idiv_int16 (int16_t x, int16_t y)
{
    return (GB_IDIV_SIGNED (x, y, 16)) ;
}

GB_MATH_KERNEL int32_t GB_idiv_int32 (int32_t x, int32_t y)
{
    return (GB_IDIV_SIGNED (x, y, 32)) ;
}

GB_MATH_KERNEL int64_t GB_idiv_int64 (int64_t x, int64_t y)
{
    return (GB_IDIV_SIGNED (x, y, 64)) ;
}

// x/y when x and y are unsigned integers
#define GB_IDIV_UNSIGNED(x,y,bits)                                          \
(                                                                           \
    ((y) == 0) ?                                                            \
    (                                                                       \
        /* x/0 */                                                           \
        ((x) == 0) ?                                                        \
        (                                                                   \
            /* zero divided by zero gives 'Nan' */                          \
            0                                                               \
        )                                                                   \
        :                                                                   \
        (                                                                   \
            /* x is positive: x/0 gives '+Inf' */                           \
            GB_UINT_MAX (bits)                                              \
        )                                                                   \
    )                                                                       \
    :                                                                       \
    (                                                                       \
        /* normal case for unsigned integer division */                     \
        (x) / (y)                                                           \
    )                                                                       \
)

GB_MATH_KERNEL uint8_t GB_idiv_uint8 (uint8_t x, uint8_t y)
{
    return (GB_IDIV_UNSIGNED (x, y, 8)) ;
}

GB_MATH_KERNEL uint16_t GB_idiv_uint16 (uint16_t x, uint16_t y)
{
    return (GB_IDIV_UNSIGNED (x, y, 16)) ;
}

GB_MATH_KERNEL uint32_t GB_idiv_uint32 (uint32_t x, uint32_t y)
{
    return (GB_IDIV_UNSIGNED (x, y, 32)) ;
}

GB_MATH_KERNEL uint64_t GB_idiv_uint64 (uint64_t x, uint64_t y)
{
    return (GB_IDIV_UNSIGNED (x, y, 64)) ;
}

// 1/y when y is a signed integer
#define GB_IMINV_SIGNED(y,bits)                                             \
(                                                                           \
    ((y) == -1) ?                                                           \
    (                                                                       \
        -1                                                                  \
    )                                                                       \
    :                                                                       \
    (                                                                       \
        ((y) == 0) ?                                                        \
        (                                                                   \
            GB_INT_MAX (bits)                                               \
        )                                                                   \
        :                                                                   \
        (                                                                   \
            ((y) == 1) ?                                                    \
            (                                                               \
                1                                                           \
            )                                                               \
            :                                                               \
            (                                                               \
                0                                                           \
            )                                                               \
        )                                                                   \
    )                                                                       \
)

// 1/y when y is an unsigned integer
#define GB_IMINV_UNSIGNED(y,bits)                                           \
(                                                                           \
    ((y) == 0) ?                                                            \
    (                                                                       \
        GB_UINT_MAX (bits)                                                  \
    )                                                                       \
    :                                                                       \
    (                                                                       \
        ((y) == 1) ?                                                        \
        (                                                                   \
            1                                                               \
        )                                                                   \
        :                                                                   \
        (                                                                   \
            0                                                               \
        )                                                                   \
    )                                                                       \
)                                                                           \

// GraphBLAS includes a built-in GrB_DIV_BOOL operator, so boolean division
// must be defined.  ANSI C11 does not provide a definition either, and
// dividing by zero (boolean false) will typically terminate an application.
// In this GraphBLAS implementation, boolean division is treated as if it were
// int1, where 1/1 = 1, 0/1 = 0, 0/0 = integer NaN = 0, 1/0 = +infinity = 1.
// Thus z=x/y is z=x.  This is arbitrary, but it allows all operators to work
// on all types without causing run time exceptions.  It also means that
// GrB_DIV(x,y) is the same as GrB_FIRST(x,y) for boolean x and y.  See for
// example GB_boolean_rename and Template/GB_ops_template.c.  Similarly,
// GrB_MINV_BOOL, which is 1/x, is simply 'true' for all x.

//------------------------------------------------------------------------------
// complex division
//------------------------------------------------------------------------------

// z = x/y where z, x, and y are double complex.  The real and imaginary parts
// are passed as separate arguments to this routine.  The NaN case is ignored
// for the double relop yr >= yi.  Returns 1 if the denominator is zero, 0
// otherwise.
//
// This uses ACM Algo 116, by R. L. Smith, 1962, which tries to avoid underflow
// and overflow.
//
// z can be aliased with x or y.
//
// Note that this function has the same signature as SuiteSparse_divcomplex.

GB_MATH_KERNEL int GB_divcomplex
(
    double xr, double xi,       // real and imaginary parts of x
    double yr, double yi,       // real and imaginary parts of y
    double *zr, double *zi      // real and imaginary parts of z
)
{
    double tr, ti, r, den ;

    int yr_class = fpclassify (yr) ;
    int yi_class = fpclassify (yi) ;

    if (yi_class == FP_ZERO)
    {
        den = yr ;
        if (xi == 0)
        {
            tr = xr / den ;
            ti = 0 ;
        }
        else if (xr == 0)
        {
            tr = 0 ;
            ti = xi / den ;
        }
        else
        {
            tr = xr / den ;
            ti = xi / den ;
        }
    }
    else if (yr_class == FP_ZERO)
    {
        den = yi ;
        if (xr == 0)
        {
            tr = xi / den ;
            ti = 0 ;
        }
        else if (xi == 0)
        {
            tr = 0 ;
            ti = -xr / den ;
        }
        else
        {
            tr = xi / den ;
            ti = -xr / den ;
        }
    }
    else if (yi_class == FP_INFINITE && yr_class == FP_INFINITE)
    {
        r = (signbit (yr) == signbit (yi)) ? (1) : (-1) ;
        den = yr + r * yi ;
        tr = (xr + xi * r) / den ;
        ti = (xi - xr * r) / den ;
    }
    else if (fabs (yr) >= fabs (yi))
    {
        r = yi / yr ;
        den = yr + r * yi ;
        tr = (xr + xi * r) / den ;
        ti = (xi - xr * r) / den ;
    }
    else
    {
        r = yr / yi ;
        den = r * yr + yi ;
        tr = (xr * r + xi) / den ;
        ti = (xi * r - xr) / den ;
    }
    (*zr) = tr ;
    (*zi) = ti ;
    return (den == 0) ;
}

GB_MATH_KERNEL GxB_FC64_t GB_FC64_div (GxB_FC64_t x, GxB_FC64_t y)
{
    double zr, zi ;
    GB_divcomplex (creal (x), cimag (x), creal (y), cimag (y), &zr, &zi) ;
    return (GxB_CMPLX (zr, zi)) ;
}

GB_MATH_KERNEL GxB_FC32_t GB_FC32_div (GxB_FC32_t x, GxB_FC32_t y)
{
    // single complex division is slow but as accurate as possible: typecast to
    // double complex, do the division, and then typecast back to single
    // complex.
    double zr, zi ;
    GB_divcomplex ((double) crealf (x), (double) cimagf (x),
                   (double) crealf (y), (double) cimagf (y), &zr, &zi) ;
    return (GxB_CMPLXF ((float) zr, (float) zi)) ;
}

//------------------------------------------------------------------------------
// z = x^y: wrappers for pow, powf, cpow, and cpowf
//------------------------------------------------------------------------------

//      if x or y are NaN, then z is NaN
//      if y is zero, then z is 1
//      if (x and y are complex but with zero imaginary parts, and
//          (x >= 0 or if y is an integer, NaN, or Inf)), then z is real
//      else use the built-in C library function, z = pow (x,y)

GB_MATH_KERNEL float GB_powf (float x, float y)
{
    int xr_class = fpclassify (x) ;
    int yr_class = fpclassify (y) ;
    if (xr_class == FP_NAN || yr_class == FP_NAN)
    {
        // z is nan if either x or y are nan
        return (NAN) ;
    }
    if (yr_class == FP_ZERO)
    {
        // z is 1 if y is zero
        return (1) ;
    }
    // otherwise, z = powf (x,y)
    return (powf (x, y)) ;
}

GB_MATH_KERNEL double GB_pow (double x, double y)
{
    int xr_class = fpclassify (x) ;
    int yr_class = fpclassify (y) ;
    if (xr_class == FP_NAN || yr_class == FP_NAN)
    {
        // z is nan if either x or y are nan
        return (NAN) ;
    }
    if (yr_class == FP_ZERO)
    {
        // z is 1 if y is zero
        return (1) ;
    }
    // otherwise, z = pow (x,y)
    return (pow (x, y)) ;
}

GB_MATH_KERNEL GxB_FC32_t GB_cpowf (GxB_FC32_t x, GxB_FC32_t y)
{
    float xr = crealf (x) ;
    float yr = crealf (y) ;
    int xr_class = fpclassify (xr) ;
    int yr_class = fpclassify (yr) ;
    int xi_class = fpclassify (cimagf (x)) ;
    int yi_class = fpclassify (cimagf (y)) ;
    if (xi_class == FP_ZERO && yi_class == FP_ZERO)
    {
        // both x and y are real; see if z should be real
        if (xr >= 0 || yr_class == FP_NAN || yr_class == FP_INFINITE ||
            yr == truncf (yr))
        {
            // z is real if x >= 0, or if y is an integer, NaN, or Inf
            return (GxB_CMPLXF (GB_powf (xr, yr), 0)) ;
        }
    }
    if (xr_class == FP_NAN || xi_class == FP_NAN ||
        yr_class == FP_NAN || yi_class == FP_NAN)
    {
        // z is (nan,nan) if any part of x or y are nan
        return (GxB_CMPLXF (NAN, NAN)) ;
    }
    if (yr_class == FP_ZERO && yi_class == FP_ZERO)
    {
        // z is (1,0) if y is (0,0)
        return (GxB_CMPLXF (1, 0)) ;
    }
    return (cpowf (x, y)) ;
}

GB_MATH_KERNEL GxB_FC64_t GB_cpow (GxB_FC64_t x, GxB_FC64_t y)
{
    double xr = creal (x) ;
    double yr = creal (y) ;
    int xr_class = fpclassify (xr) ;
    int yr_class = fpclassify (yr) ;
    int xi_class = fpclassify (cimag (x)) ;
    int yi_class = fpclassify (cimag (y)) ;
    if (xi_class == FP_ZERO && yi_class == FP_ZERO)
    {
        // both x and y are real; see if z should be real
        if (xr >= 0 || yr_class == FP_NAN || yr_class == FP_INFINITE ||
            yr == trunc (yr))
        {
            // z is real if x >= 0, or if y is an integer, NaN, or Inf
            return (GxB_CMPLX (GB_pow (xr, yr), 0)) ;
        }
    }
    if (xr_class == FP_NAN || xi_class == FP_NAN ||
        yr_class == FP_NAN || yi_class == FP_NAN)
    {
        // z is (nan,nan) if any part of x or y are nan
        return (GxB_CMPLX (NAN, NAN)) ;
    }
    if (yr_class == FP_ZERO && yi_class == FP_ZERO)
    {
        // z is (1,0) if y is (0,0)
        return (GxB_CMPLX (1, 0)) ;
    }
    return (cpow (x, y)) ;
}

GB_MATH_KERNEL int8_t GB_pow_int8 (int8_t x, int8_t y)
{
    return (GB_cast_to_int8_t (GB_pow ((double) x, (double) y))) ;
}

GB_MATH_KERNEL int16_t GB_pow_int16 (int16_t x, int16_t y)
{
    return (GB_cast_to_int16_t (GB_pow ((double) x, (double) y))) ;
}

GB_MATH_KERNEL int32_t GB_pow_int32 (int32_t x, int32_t y)
{
    return (GB_cast_to_int32_t (GB_pow ((double) x, (double) y))) ;
}

GB_MATH_KERNEL int64_t GB_pow_int64 (int64_t x, int64_t y)
{
    return (GB_cast_to_int64_t (GB_pow ((double) x, (double) y))) ;
}

GB_MATH_KERNEL uint8_t GB_pow_uint8 (uint8_t x, uint8_t y)
{
    return (GB_cast_to_uint8_t (GB_pow ((double) x, (double) y))) ;
}

GB_MATH_KERNEL uint16_t GB_pow_uint16 (uint16_t x, uint16_t y)
{
    return (GB_cast_to_uint16_t (GB_pow ((double) x, (double) y))) ;
}

GB_MATH_KERNEL uint32_t GB_pow_uint32 (uint32_t x, uint32_t y)
{
    return (GB_cast_to_uint32_t (GB_pow ((double) x, (double) y))) ;
}

GB_MATH_KERNEL uint64_t GB_pow_uint64 (uint64_t x, uint64_t y)
{
    return (GB_cast_to_uint64_t (GB_pow ((double) x, (double) y))) ;
}

//------------------------------------------------------------------------------
// frexp for float and double
//------------------------------------------------------------------------------

GB_MATH_KERNEL float GB_frexpxf (float x)
{
    // ignore the exponent and just return the mantissa
    int exp_ignored ;
    return (frexpf (x, &exp_ignored)) ;
}

GB_MATH_KERNEL float GB_frexpef (float x)
{
    // ignore the mantissa and just return the exponent
    int exp ;
    (void) frexpf (x, &exp) ;
    return ((float) exp) ;
}

GB_MATH_KERNEL double GB_frexpx (double x)
{
    // ignore the exponent and just return the mantissa
    int exp_ignored ;
    return (frexp (x, &exp_ignored)) ;
}

GB_MATH_KERNEL double GB_frexpe (double x)
{
    // ignore the mantissa and just return the exponent
    int exp ;
    (void) frexp (x, &exp) ;
    return ((double) exp) ;
}

//------------------------------------------------------------------------------
// signum functions
//------------------------------------------------------------------------------

GB_MATH_KERNEL float GB_signumf (float x)
{
    if (isnan (x)) return (x) ;
    return ((float) ((x < 0) ? (-1) : ((x > 0) ? 1 : 0))) ;
}

GB_MATH_KERNEL double GB_signum (double x)
{
    if (isnan (x)) return (x) ;
    return ((double) ((x < 0) ? (-1) : ((x > 0) ? 1 : 0))) ;
}

GB_MATH_KERNEL GxB_FC32_t GB_csignumf (GxB_FC32_t x)
{
    if (crealf (x) == 0 && cimagf (x) == 0) return (GxB_CMPLXF (0,0)) ;
    float y = cabsf (x) ;
    return (GxB_CMPLXF (crealf (x) / y, cimagf (x) / y)) ;
}

GB_MATH_KERNEL GxB_FC64_t GB_csignum (GxB_FC64_t x)
{
    if (creal (x) == 0 && cimag (x) == 0) return (GxB_CMPLX (0,0)) ;
    double y = cabs (x) ;
    return (GxB_CMPLX (creal (x) / y, cimag (x) / y)) ;
}

//------------------------------------------------------------------------------
// complex functions
//------------------------------------------------------------------------------

// The ANSI C11 math.h header defines the ceil, floor, round, trunc,
// exp2, expm1, log10, log1pm, or log2 functions for float and double,
// but the corresponding functions do not appear in the ANSI C11 complex.h.
// These functions are used instead, for float complex and double complex.

//------------------------------------------------------------------------------
// z = ceil (x) for float complex
//------------------------------------------------------------------------------

GB_MATH_KERNEL GxB_FC32_t GB_cceilf (GxB_FC32_t x)
{
    return (GxB_CMPLXF (ceilf (crealf (x)), ceilf (cimagf (x)))) ;
}

//------------------------------------------------------------------------------
// z = ceil (x) for double complex
//------------------------------------------------------------------------------

GB_MATH_KERNEL GxB_FC64_t GB_cceil (GxB_FC64_t x)
{
    return (GxB_CMPLX (ceil (creal (x)), ceil (cimag (x)))) ;
}

//------------------------------------------------------------------------------
// z = floor (x) for float complex
//------------------------------------------------------------------------------

GB_MATH_KERNEL GxB_FC32_t GB_cfloorf (GxB_FC32_t x)
{
    return (GxB_CMPLXF (floorf (crealf (x)), floorf (cimagf (x)))) ;
}

//------------------------------------------------------------------------------
// z = floor (x) for double complex
//------------------------------------------------------------------------------

GB_MATH_KERNEL GxB_FC64_t GB_cfloor (GxB_FC64_t x)
{
    return (GxB_CMPLX (floor (creal (x)), floor (cimag (x)))) ;
}

//------------------------------------------------------------------------------
// z = round (x) for float complex
//------------------------------------------------------------------------------

GB_MATH_KERNEL GxB_FC32_t GB_croundf (GxB_FC32_t x)
{
    return (GxB_CMPLXF (roundf (crealf (x)), roundf (cimagf (x)))) ;
}

//------------------------------------------------------------------------------
// z = round (x) for double complex
//------------------------------------------------------------------------------

GB_MATH_KERNEL GxB_FC64_t GB_cround (GxB_FC64_t x)
{
    return (GxB_CMPLX (round (creal (x)), round (cimag (x)))) ;
}

//------------------------------------------------------------------------------
// z = trunc (x) for float complex
//------------------------------------------------------------------------------

GB_MATH_KERNEL GxB_FC32_t GB_ctruncf (GxB_FC32_t x)
{
    return (GxB_CMPLXF (truncf (crealf (x)), truncf (cimagf (x)))) ;
}

//------------------------------------------------------------------------------
// z = trunc (x) for double complex
//------------------------------------------------------------------------------

GB_MATH_KERNEL GxB_FC64_t GB_ctrunc (GxB_FC64_t x)
{
    return (GxB_CMPLX (trunc (creal (x)), trunc (cimag (x)))) ;
}

//------------------------------------------------------------------------------
// z = exp2 (x) for float complex
//------------------------------------------------------------------------------

GB_MATH_KERNEL GxB_FC32_t GB_cexp2f (GxB_FC32_t x)
{
    if (fpclassify (cimagf (x)) == FP_ZERO)
    {
        // x is real, use exp2f
        return (GxB_CMPLXF (exp2f (crealf (x)), 0)) ;
    }
    return (GB_cpowf (GxB_CMPLXF (2,0), x)) ;     // z = 2^x
}

//------------------------------------------------------------------------------
// z = exp2 (x) for double complex
//------------------------------------------------------------------------------

GB_MATH_KERNEL GxB_FC64_t GB_cexp2 (GxB_FC64_t x)
{
    if (fpclassify (cimag (x)) == FP_ZERO)
    {
        // x is real, use exp2
        return (GxB_CMPLX (exp2 (creal (x)), 0)) ;
    }
    return (GB_cpow (GxB_CMPLX (2,0), x)) ;      // z = 2^x
}

//------------------------------------------------------------------------------
// z = expm1 (x) for double complex
//------------------------------------------------------------------------------

GB_MATH_KERNEL GxB_FC64_t GB_cexpm1 (GxB_FC64_t x)
{
    // FUTURE: GB_cexpm1 is not accurate
    // z = cexp (x) - 1
    GxB_FC64_t z = cexp (x) ;
    return (GxB_CMPLX (creal (z) - 1, cimag (z))) ;
}

//------------------------------------------------------------------------------
// z = expm1 (x) for float complex
//------------------------------------------------------------------------------

GB_MATH_KERNEL GxB_FC32_t GB_cexpm1f (GxB_FC32_t x)
{
    // typecast to double and use GB_cexpm1
    GxB_FC64_t z = GxB_CMPLX ((double) crealf (x), (double) cimagf (x)) ;
    z = GB_cexpm1 (z) ;
    return (GxB_CMPLXF ((float) creal (z), (float) cimag (z))) ;
}

//------------------------------------------------------------------------------
// z = log1p (x) for double complex
//------------------------------------------------------------------------------

GB_MATH_KERNEL GxB_FC64_t GB_clog1p (GxB_FC64_t x)
{
    // FUTURE: GB_clog1p is not accurate
    // z = clog (1+x)
    return (clog (GxB_CMPLX (creal (x) + 1, cimag (x)))) ;
}

//------------------------------------------------------------------------------
// z = log1p (x) for float complex
//------------------------------------------------------------------------------

GB_MATH_KERNEL GxB_FC32_t GB_clog1pf (GxB_FC32_t x)
{
    // typecast to double and use GB_clog1p
    GxB_FC64_t z = GxB_CMPLX ((double) crealf (x), (double) cimagf (x)) ;
    z = GB_clog1p (z) ;
    return (GxB_CMPLXF ((float) creal (z), (float) cimag (z))) ;
}

//------------------------------------------------------------------------------
// z = log10 (x) for float complex
//------------------------------------------------------------------------------

// log_e (10) in single precision
#define GB_LOG10EF 2.3025851f

GB_MATH_KERNEL GxB_FC32_t GB_clog10f (GxB_FC32_t x)
{
    // z = log (x) / log (10)
    return (GB_FC32_div (clogf (x), GxB_CMPLXF (GB_LOG10EF, 0))) ;
}

//------------------------------------------------------------------------------
// z = log10 (x) for double complex
//------------------------------------------------------------------------------

// log_e (10) in double precision
#define GB_LOG10E 2.302585092994045901

GB_MATH_KERNEL GxB_FC64_t GB_clog10 (GxB_FC64_t x)
{
    // z = log (x) / log (10)
    return (GB_FC64_div (clog (x), GxB_CMPLX (GB_LOG10E, 0))) ;
}

//------------------------------------------------------------------------------
// z = log2 (x) for float complex
//------------------------------------------------------------------------------

// log_e (2) in single precision
#define GB_LOG2EF 0.69314718f

GB_MATH_KERNEL GxB_FC32_t GB_clog2f (GxB_FC32_t x)
{
    // z = log (x) / log (2)
    return (GB_FC32_div (clogf (x), GxB_CMPLXF (GB_LOG2EF, 0))) ;
}

//------------------------------------------------------------------------------
// z = log2 (x) for double complex
//------------------------------------------------------------------------------

// log_e (2) in double precision
#define GB_LOG2E 0.693147180559945286

GB_MATH_KERNEL GxB_FC64_t GB_clog2 (GxB_FC64_t x)
{
    // z = log (x) / log (2)
    return (GB_FC64_div (clog (x), GxB_CMPLX (GB_LOG2E, 0))) ;
}

//------------------------------------------------------------------------------
// z = isinf (x) for float complex
//------------------------------------------------------------------------------

GB_MATH_KERNEL bool GB_cisinff (GxB_FC32_t x)
{
    return (isinf (crealf (x)) || isinf (cimagf (x))) ;
}

//------------------------------------------------------------------------------
// z = isinf (x) for double complex
//------------------------------------------------------------------------------

GB_MATH_KERNEL bool GB_cisinf (GxB_FC64_t x)
{
    return (isinf (creal (x)) || isinf (cimag (x))) ;
}

//------------------------------------------------------------------------------
// z = isnan (x) for float complex
//------------------------------------------------------------------------------

GB_MATH_KERNEL bool GB_cisnanf (GxB_FC32_t x)
{
    return (isnan (crealf (x)) || isnan (cimagf (x))) ;
}

//------------------------------------------------------------------------------
// z = isnan (x) for double complex
//------------------------------------------------------------------------------

GB_MATH_KERNEL bool GB_cisnan (GxB_FC64_t x)
{
    return (isnan (creal (x)) || isnan (cimag (x))) ;
}

//------------------------------------------------------------------------------
// z = isfinite (x) for float complex
//------------------------------------------------------------------------------

GB_MATH_KERNEL bool GB_cisfinitef (GxB_FC32_t x)
{
    return (isfinite (crealf (x)) && isfinite (cimagf (x))) ;
}

//------------------------------------------------------------------------------
// z = isfinite (x) for double complex
//------------------------------------------------------------------------------

GB_MATH_KERNEL bool GB_cisfinite (GxB_FC64_t x)
{
    return (isfinite (creal (x)) && isfinite (cimag (x))) ;
}

#undef GB_MATH_KERNEL
#endif

