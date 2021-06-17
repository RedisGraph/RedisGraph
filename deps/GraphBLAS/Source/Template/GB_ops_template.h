//------------------------------------------------------------------------------
// GB_ops_template.h: define the unary and binary functions and operators
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// This file is #include'd many times in GB.h to define the unary and binary
// functions.

//------------------------------------------------------------------------------
// z = one (x)
//------------------------------------------------------------------------------

GB_PUBLIC struct GB_UnaryOp_opaque GB (opaque_GxB_ONE) ;

inline void GB (ONE_f) (GB_TYPE *z, const GB_TYPE *x)
{
    #if defined ( GB_FLOAT_COMPLEX )
    (*z) = GxB_CMPLXF (1,0) ;
    #elif defined ( GB_DOUBLE_COMPLEX )
    (*z) = GxB_CMPLX (1,0) ;
    #else
    (*z) = ((GB_TYPE) 1) ;
    #endif
}

//------------------------------------------------------------------------------
// z = identity (x)
//------------------------------------------------------------------------------

GB_PUBLIC struct GB_UnaryOp_opaque GB (opaque_GrB_IDENTITY) ;

inline void GB (IDENTITY_f) (GB_TYPE *z, const GB_TYPE *x)
{
    (*z) = (*x) ;
}

//------------------------------------------------------------------------------
// z = ainv (x)
//------------------------------------------------------------------------------

GB_PUBLIC struct GB_UnaryOp_opaque GB (opaque_GrB_AINV) ;

inline void GB (AINV_f) (GB_TYPE *z, const GB_TYPE *x)
{
    #if defined ( GB_FLOAT_COMPLEX )
        (*z) = GB_FC32_ainv (*x) ;
    #elif defined ( GB_DOUBLE_COMPLEX )
        (*z) = GB_FC64_ainv (*x) ;
    #elif defined ( GB_BOOLEAN )
        (*z) = (*x) ;
    #else
        // integer (signed or unsigned).  unsigned int remains unsigned.
        (*z) = -(*x) ;
    #endif
}

//------------------------------------------------------------------------------
// z = minv (x)
//------------------------------------------------------------------------------

GB_PUBLIC struct GB_UnaryOp_opaque GB (opaque_GrB_MINV) ;

inline void GB (MINV_f) (GB_TYPE *z, const GB_TYPE *x)
{
    #if defined ( GB_BOOLEAN )
        (*z) = true ;
    #elif defined ( GB_SIGNED_INT )
        (*z) = GB_IMINV_SIGNED ((*x), GB_BITS) ;
    #elif defined ( GB_UNSIGNED_INT )
        (*z) = GB_IMINV_UNSIGNED ((*x), GB_BITS) ;
    #elif defined ( GB_FLOAT )
        (*z) = 1 / (*x) ;
    #elif defined ( GB_DOUBLE )
        (*z) = 1 / (*x) ;
    #elif defined ( GB_FLOAT_COMPLEX )
        (*z) = GB_FC32_minv (*x) ;
    #elif defined ( GB_DOUBLE_COMPLEX )
        (*z) = GB_FC64_minv (*x) ;
    #endif
}

//------------------------------------------------------------------------------
// z = abs (x)
//------------------------------------------------------------------------------

#if defined ( GB_REAL )

    // GrB_ABS_* is now in the v1.3 spec, as a built-in operator
    GB_PUBLIC struct GB_UnaryOp_opaque GB (opaque_GrB_ABS) ;

    inline void GB (ABS_f) (GB_TYPE *z, const GB_TYPE *x)
    {
        #if defined ( GB_BOOLEAN )
            (*z) = (*x) ;
        #elif defined ( GB_SIGNED_INT )
            (*z) = GB_IABS ((*x)) ;
        #elif defined ( GB_UNSIGNED_INT )
            (*z) = (*x) ;
        #elif defined ( GB_FLOAT )
            (*z) = fabsf (*x) ;
        #elif defined ( GB_DOUBLE )
            (*z) = fabs (*x) ;
        #endif
    }

#else

    // GxB_ABS_FC* for complex types (not in the v1.3 spec)
    GB_PUBLIC struct GB_UnaryOp_opaque GB (opaque_GxB_ABS) ;

    #if defined ( GB_FLOAT_COMPLEX )
        inline void GB (ABS_f) (float *z, const GB_TYPE *x)
        {
            (*z) = cabsf (*x) ;
        }
    #else
        inline void GB (ABS_f) (double *z, const GB_TYPE *x)
        {
            (*z) = cabs (*x) ;
        }
    #endif

#endif

//------------------------------------------------------------------------------
// z = lnot (x), for real types only
//------------------------------------------------------------------------------

#if defined ( GB_REAL )

    GB_PUBLIC struct GB_UnaryOp_opaque GB (opaque_GxB_LNOT) ;

    inline void GB (LNOT_f) (GB_TYPE *z, const GB_TYPE *x)
    {
        #if defined ( GB_BOOLEAN )
            (*z) = ! (*x) ;
        #else
            (*z) = ! ((*x) != 0) ;
        #endif
    }

#endif

//------------------------------------------------------------------------------
// z = bnot (x), bitwise complement, for integer types only
//------------------------------------------------------------------------------

#if defined ( GB_SIGNED_INT ) || defined ( GB_UNSIGNED_INT )

    // new to the v1.3 spec
    GB_PUBLIC struct GB_UnaryOp_opaque GB (opaque_GrB_BNOT) ;
    inline void GB (BNOT_f) (GB_TYPE *z, const GB_TYPE *x)
    {
        (*z) = ~ (*x) ;
    }

#endif

//------------------------------------------------------------------------------
// z = frexpx (x) and z = frexpe (x)
//------------------------------------------------------------------------------

#if defined ( GB_FLOAT )

    GB_PUBLIC struct GB_UnaryOp_opaque GB (opaque_GxB_FREXPX) ;
    inline void GB (FREXPX_f) (float *z, const float *x)
    {
        (*z) = GB_frexpxf (*x) ;
    }

    GB_PUBLIC struct GB_UnaryOp_opaque GB (opaque_GxB_FREXPE) ;
    inline void GB (FREXPE_f) (float *z, const float *x)
    {
        (*z) = GB_frexpef (*x) ;
    }

#elif defined ( GB_DOUBLE )

    GB_PUBLIC struct GB_UnaryOp_opaque GB (opaque_GxB_FREXPX) ;
    inline void GB (FREXPX_f) (double *z, const double *x)
    {
        (*z) = GB_frexpx (*x) ;
    }

    GB_PUBLIC struct GB_UnaryOp_opaque GB (opaque_GxB_FREXPE) ;
    inline void GB (FREXPE_f) (double *z, const double *x)
    {
        (*z) = GB_frexpe (*x) ;
    }

#endif

//------------------------------------------------------------------------------
// unary operators for floating-point types
//------------------------------------------------------------------------------

// For these operators, the input and output types are the same.

#undef  GB_OP
#define GB_OP(op,func)                                              \
    GB_PUBLIC struct GB_UnaryOp_opaque GB (opaque_GxB_ ## op) ;     \
    inline void GB (op ## _f) (GB_TYPE *z, const GB_TYPE *x)        \
    {                                                               \
        (*z) = func (*x) ;                                          \
    }

#if defined ( GB_FLOAT )

    //--------------------------------------------------------------------------
    // float
    //--------------------------------------------------------------------------

    GB_OP (SQRT  , sqrtf   )
    GB_OP (LOG   , logf    )
    GB_OP (EXP   , expf    )

    GB_OP (SIN   , sinf    )
    GB_OP (COS   , cosf    )
    GB_OP (TAN   , tanf    )

    GB_OP (ASIN  , asinf   )
    GB_OP (ACOS  , acosf   )
    GB_OP (ATAN  , atanf   )

    GB_OP (SINH  , sinhf   )
    GB_OP (COSH  , coshf   )
    GB_OP (TANH  , tanhf   )

    GB_OP (ASINH , asinhf  )
    GB_OP (ACOSH , acoshf  )
    GB_OP (ATANH , atanhf  )

    GB_OP (SIGNUM, GB_signumf )
    GB_OP (CEIL  , ceilf   )
    GB_OP (FLOOR , floorf  )
    GB_OP (ROUND , roundf  )
    GB_OP (TRUNC , truncf  )

    GB_OP (EXP2  , exp2f   )
    GB_OP (EXPM1 , expm1f  )
    GB_OP (LOG10 , log10f  )
    GB_OP (LOG1P , log1pf  )
    GB_OP (LOG2  , log2f   )

    // real only
    GB_OP (LGAMMA, lgammaf )
    GB_OP (TGAMMA, tgammaf )
    GB_OP (ERF   , erff    )
    GB_OP (ERFC  , erfcf   )

#elif defined ( GB_DOUBLE )

    //--------------------------------------------------------------------------
    // double
    //--------------------------------------------------------------------------

    GB_OP (SQRT  , sqrt    )
    GB_OP (LOG   , log     )
    GB_OP (EXP   , exp     )

    GB_OP (SIN   , sin     )
    GB_OP (COS   , cos     )
    GB_OP (TAN   , tan     )

    GB_OP (ASIN  , asin    )
    GB_OP (ACOS  , acos    )
    GB_OP (ATAN  , atan    )

    GB_OP (SINH  , sinh    )
    GB_OP (COSH  , cosh    )
    GB_OP (TANH  , tanh    )

    GB_OP (ASINH , asinh   )
    GB_OP (ACOSH , acosh   )
    GB_OP (ATANH , atanh   )

    GB_OP (SIGNUM, GB_signum )
    GB_OP (CEIL  , ceil    )
    GB_OP (FLOOR , floor   )
    GB_OP (ROUND , round   )
    GB_OP (TRUNC , trunc   )

    GB_OP (EXP2  , exp2    )
    GB_OP (EXPM1 , expm1   )
    GB_OP (LOG10 , log10   )
    GB_OP (LOG1P , log1p   )
    GB_OP (LOG2  , log2    )

    // real only
    GB_OP (LGAMMA, lgamma )
    GB_OP (TGAMMA, tgamma )
    GB_OP (ERF   , erf    )
    GB_OP (ERFC  , erfc   )

#elif defined ( GB_FLOAT_COMPLEX )

    //--------------------------------------------------------------------------
    // float complex
    //--------------------------------------------------------------------------

    GB_OP (SQRT  , csqrtf   )
    GB_OP (LOG   , clogf    )
    GB_OP (EXP   , cexpf    )

    GB_OP (SIN   , csinf    )
    GB_OP (COS   , ccosf    )
    GB_OP (TAN   , ctanf    )

    GB_OP (ASIN  , casinf   )
    GB_OP (ACOS  , cacosf   )
    GB_OP (ATAN  , catanf   )

    GB_OP (SINH  , csinhf   )
    GB_OP (COSH  , ccoshf   )
    GB_OP (TANH  , ctanhf   )

    GB_OP (ASINH , casinhf  )
    GB_OP (ACOSH , cacoshf  )
    GB_OP (ATANH , catanhf  )

    GB_OP (SIGNUM, GB_csignumf )
    GB_OP (CEIL  , GB_cceilf   )
    GB_OP (FLOOR , GB_cfloorf  )
    GB_OP (ROUND , GB_croundf  )
    GB_OP (TRUNC , GB_ctruncf  )

    GB_OP (EXP2  , GB_cexp2f   )
    GB_OP (EXPM1 , GB_cexpm1f  )
    GB_OP (LOG10 , GB_clog10f  )
    GB_OP (LOG1P , GB_clog1pf  )
    GB_OP (LOG2  , GB_clog2f   )

    // complex only
    GB_OP (CONJ  , conjf       )

#elif defined ( GB_DOUBLE_COMPLEX )

    //--------------------------------------------------------------------------
    // double complex
    //--------------------------------------------------------------------------

    GB_OP (SQRT  , csqrt    )
    GB_OP (LOG   , clog     )
    GB_OP (EXP   , cexp     )

    GB_OP (SIN   , csin     )
    GB_OP (COS   , ccos     )
    GB_OP (TAN   , ctan     )

    GB_OP (ASIN  , casin    )
    GB_OP (ACOS  , cacos    )
    GB_OP (ATAN  , catan    )

    GB_OP (SINH  , csinh    )
    GB_OP (COSH  , ccosh    )
    GB_OP (TANH  , ctanh    )

    GB_OP (ASINH , casinh   )
    GB_OP (ACOSH , cacosh   )
    GB_OP (ATANH , catanh   )

    GB_OP (SIGNUM, GB_csignum  )
    GB_OP (CEIL  , GB_cceil    )
    GB_OP (FLOOR , GB_cfloor   )
    GB_OP (ROUND , GB_cround   )
    GB_OP (TRUNC , GB_ctrunc   )

    GB_OP (EXP2  , GB_cexp2    )
    GB_OP (EXPM1 , GB_cexpm1   )
    GB_OP (LOG10 , GB_clog10   )
    GB_OP (LOG1P , GB_clog1p   )
    GB_OP (LOG2  , GB_clog2    )

    // complex only
    GB_OP (CONJ  , conj        )

#endif

//------------------------------------------------------------------------------
// unary operators z=f(x) where z and x have different types
//------------------------------------------------------------------------------

// x is float, double, float complex, or double complex

#undef  GB_OP
#define GB_OP(op,expression,z_t,x_t)                                \
    GB_PUBLIC struct GB_UnaryOp_opaque GB (opaque_GxB_ ## op) ;     \
    inline void GB (op ## _f) (z_t *z, const x_t *x)                \
    {                                                               \
        (*z) = expression ;                                         \
    }

#if defined ( GB_FLOAT )

    GB_OP (ISINF    , (isinf (*x))    , bool, float)
    GB_OP (ISNAN    , (isnan (*x))    , bool, float)
    GB_OP (ISFINITE , (isfinite (*x)) , bool, float)

#elif defined ( GB_DOUBLE )

    GB_OP (ISINF    , (isinf (*x))    , bool, double)
    GB_OP (ISNAN    , (isnan (*x))    , bool, double)
    GB_OP (ISFINITE , (isfinite (*x)) , bool, double)

#elif defined ( GB_FLOAT_COMPLEX )

    GB_OP (ISINF    , GB_cisinff (*x)   , bool, GxB_FC32_t)
    GB_OP (ISNAN    , GB_cisnanf (*x)   , bool, GxB_FC32_t)
    GB_OP (ISFINITE , GB_cisfinitef (*x), bool, GxB_FC32_t)

    // complex only
    GB_OP (CREAL , crealf (*x), float, GxB_FC32_t)
    GB_OP (CIMAG , cimagf (*x), float, GxB_FC32_t)
    GB_OP (CARG  , cargf  (*x), float, GxB_FC32_t)

#elif defined ( GB_DOUBLE_COMPLEX )

    GB_OP (ISINF    , GB_cisinf (*x)    , bool, GxB_FC64_t)
    GB_OP (ISNAN    , GB_cisnan  (*x)   , bool, GxB_FC64_t)
    GB_OP (ISFINITE , GB_cisfinite (*x) , bool, GxB_FC64_t)

    // complex only
    GB_OP (CREAL , creal (*x), double, GxB_FC64_t)
    GB_OP (CIMAG , cimag (*x), double, GxB_FC64_t)
    GB_OP (CARG  , carg  (*x), double, GxB_FC64_t)

#endif

//------------------------------------------------------------------------------
// binary functions z=f(x,y) where x,y,z have the same type, for all types
//------------------------------------------------------------------------------

// first, second, pair, any, plus, minus, rminus, times, div, rdiv, pow

#define GB_Z_X_Y_ARGS GB_TYPE *z, const GB_TYPE *x, const GB_TYPE *y

inline void GB (FIRST_f) (GB_Z_X_Y_ARGS)
{
    (*z) = (*x) ;
}

inline void GB (SECOND_f) (GB_Z_X_Y_ARGS)
{
    (*z) = (*y) ;
}

inline void GB (PAIR_f) (GB_Z_X_Y_ARGS)
{
    #if defined ( GB_FLOAT_COMPLEX )
        (*z) = GxB_CMPLXF (1, 0) ;
    #elif defined ( GB_DOUBLE_COMPLEX )
        (*z) = GxB_CMPLX (1, 0) ;
    #else
        (*z) = 1 ;
    #endif
}

inline void GB (ANY_f) (GB_Z_X_Y_ARGS)      // same as SECOND
{
    (*z) = (*y) ; 
}

inline void GB (PLUS_f) (GB_Z_X_Y_ARGS)
{
    #if defined ( GB_FLOAT_COMPLEX )
        (*z) = GB_FC32_add (*x,*y) ;
    #elif defined ( GB_DOUBLE_COMPLEX )
        (*z) = GB_FC64_add (*x,*y) ;
    #else
        (*z) = (*x) + (*y) ;
    #endif
}

inline void GB (MINUS_f) (GB_Z_X_Y_ARGS)
{
    #if defined ( GB_FLOAT_COMPLEX )
        (*z) = GB_FC32_minus (*x,*y) ;
    #elif defined ( GB_DOUBLE_COMPLEX )
        (*z) = GB_FC64_minus (*x,*y) ;
    #else
        (*z) = (*x) - (*y) ;
    #endif
}

inline void GB (RMINUS_f) (GB_Z_X_Y_ARGS)
{
    #if defined ( GB_FLOAT_COMPLEX )
        (*z) = GB_FC32_minus (*y,*x) ;
    #elif defined ( GB_DOUBLE_COMPLEX )
        (*z) = GB_FC64_minus (*y,*x) ;
    #else
        (*z) = (*y) - (*x) ;
    #endif
}

inline void GB (TIMES_f) (GB_Z_X_Y_ARGS)
{
    #if defined ( GB_FLOAT_COMPLEX )
        (*z) = GB_FC32_mul (*x,*y) ;
    #elif defined ( GB_DOUBLE_COMPLEX )
        (*z) = GB_FC64_mul (*x,*y) ;
    #else
        (*z) = (*x) * (*y) ;
    #endif
}

inline void GB (DIV_f) (GB_Z_X_Y_ARGS)
{
    #if defined ( GB_BOOLEAN )
        // boolean div (== first)
        (*z) = (*x) ;
    #elif defined ( GB_SIGNED_INT )
        (*z) = GB_IDIV_SIGNED ((*x), (*y), GB_BITS) ;
    #elif defined ( GB_UNSIGNED_INT )
        (*z) = GB_IDIV_UNSIGNED ((*x), (*y), GB_BITS) ;
    #elif defined ( GB_FLOAT ) || defined ( GB_DOUBLE )
        (*z) = (*x) / (*y) ;
    #elif defined ( GB_FLOAT_COMPLEX )
        (*z) = GB_FC32_div (*x, *y) ;
    #elif defined ( GB_DOUBLE_COMPLEX )
        (*z) = GB_FC64_div (*x, *y) ;
    #endif
}

inline void GB (RDIV_f) (GB_Z_X_Y_ARGS)
{
    #if defined ( GB_BOOLEAN )
        // boolean rdiv (== second)
        (*z) = (*y) ;
    #elif defined ( GB_SIGNED_INT )
        (*z) = GB_IDIV_SIGNED ((*y), (*x), GB_BITS) ;
    #elif defined ( GB_UNSIGNED_INT )
        (*z) = GB_IDIV_UNSIGNED ((*y), (*x), GB_BITS) ;
    #elif defined ( GB_FLOAT ) || defined ( GB_DOUBLE )
        (*z) = (*y) / (*x) ;
    #elif defined ( GB_FLOAT_COMPLEX )
        (*z) = GB_FC32_div (*y, *x) ;
    #elif defined ( GB_DOUBLE_COMPLEX )
        (*z) = GB_FC64_div (*y, *x) ;
    #endif
}

// z = pow (x,y)
inline void GB (POW_f) (GB_Z_X_Y_ARGS)
{
    #if defined ( GB_BOOLEAN )
        (*z) = (*x) || (!(*y)) ;
    #elif defined ( GB_SIGNED_INT )
        #if ( GB_BITS == 8)
            (*z) = GB_pow_int8 ((*x), (*y)) ;
        #elif ( GB_BITS == 16)
            (*z) = GB_pow_int16 ((*x), (*y)) ;
        #elif ( GB_BITS == 32)
            (*z) = GB_pow_int32 ((*x), (*y)) ;
        #elif ( GB_BITS == 64)
            (*z) = GB_pow_int64 ((*x), (*y)) ;
        #endif
    #elif defined ( GB_UNSIGNED_INT )
        #if ( GB_BITS == 8)
            (*z) = GB_pow_uint8 ((*x), (*y)) ;
        #elif ( GB_BITS == 16)
            (*z) = GB_pow_uint16 ((*x), (*y)) ;
        #elif ( GB_BITS == 32)
            (*z) = GB_pow_uint32 ((*x), (*y)) ;
        #elif ( GB_BITS == 64)
            (*z) = GB_pow_uint64 ((*x), (*y)) ;
        #endif
    #elif defined ( GB_FLOAT )
        (*z) = GB_powf ((*x), (*y)) ;
    #elif defined ( GB_DOUBLE )
        (*z) = GB_pow ((*x), (*y)) ;
    #elif defined ( GB_FLOAT_COMPLEX )
        (*z) = GB_cpowf ((*x), (*y)) ;
    #elif defined ( GB_DOUBLE_COMPLEX )
        (*z) = GB_cpow ((*x), (*y)) ;
    #endif
}

GB_PUBLIC struct GB_BinaryOp_opaque
    GB (opaque_GrB_FIRST),
    GB (opaque_GrB_SECOND),
    GB (opaque_GxB_PAIR),
    GB (opaque_GxB_ANY),
    GB (opaque_GrB_PLUS),
    GB (opaque_GrB_MINUS),
    GB (opaque_GxB_RMINUS),
    GB (opaque_GrB_TIMES),
    GB (opaque_GrB_DIV),
    GB (opaque_GxB_RDIV),
    GB (opaque_GxB_POW) ;

//------------------------------------------------------------------------------
// binary operators for real types only
//------------------------------------------------------------------------------

// min and max: real only, not complex
#if defined ( GB_REAL )

    inline void GB (MIN_f) (GB_Z_X_Y_ARGS)
    {
        #if defined ( GB_FLOAT )
            (*z) = fminf ((*x), (*y)) ;
        #elif defined ( GB_DOUBLE )
            (*z) = fmin ((*x), (*y)) ;
        #else
            (*z) = GB_IMIN ((*x), (*y)) ;
        #endif
    }

    inline void GB (MAX_f) (GB_Z_X_Y_ARGS)
    {
        #if defined ( GB_FLOAT )
            (*z) = fmaxf ((*x), (*y)) ;
        #elif defined ( GB_DOUBLE )
            (*z) = fmax ((*x), (*y)) ;
        #else
            (*z) = GB_IMAX ((*x), (*y)) ;
        #endif
    }

    GB_PUBLIC struct GB_BinaryOp_opaque
        GB (opaque_GrB_MIN),
        GB (opaque_GrB_MAX) ;
#endif

//------------------------------------------------------------------------------
// binary operators for integer types only
//------------------------------------------------------------------------------

#if defined ( GB_SIGNED_INT ) || defined ( GB_UNSIGNED_INT )

    // these are new to the v1.3 spec
    GB_PUBLIC struct GB_BinaryOp_opaque
        GB (opaque_GrB_BOR),
        GB (opaque_GrB_BAND),
        GB (opaque_GrB_BXOR),
        GB (opaque_GrB_BXNOR) ;

    inline void GB (BOR_f  ) (GB_Z_X_Y_ARGS) { (*z) = (*x) | (*y) ; }
    inline void GB (BAND_f ) (GB_Z_X_Y_ARGS) { (*z) = (*x) & (*y) ; }
    inline void GB (BXOR_f ) (GB_Z_X_Y_ARGS) { (*z) = (*x) ^ (*y) ; }
    inline void GB (BXNOR_f) (GB_Z_X_Y_ARGS) { (*z) = ~((*x) ^ (*y)) ; }

    // these are SuiteSparse:GraphBLAS extensions
    GB_PUBLIC struct GB_BinaryOp_opaque
        GB (opaque_GxB_BGET),
        GB (opaque_GxB_BSET),
        GB (opaque_GxB_BCLR),
        GB (opaque_GxB_BSHIFT) ;

    inline void GB (BGET_f) (GB_Z_X_Y_ARGS)
    {
        // bitget (x,y) returns a single bit from x, as 0 or 1, whose position
        // is given by y.  y = 1 is the least significant bit, and y = GB_BITS
        // (64 for uint64) is the most significant bit. If y is outside this
        // range, the result is zero.
        GB_TYPE k = (*y) ;

        #if defined ( GB_SIGNED_INT )

            #if ( GB_BITS == 8)
                (*z) = GB_BITGET ((*x), k, int8_t,   8) ;
            #elif ( GB_BITS == 16)
                (*z) = GB_BITGET ((*x), k, int16_t, 16) ;
            #elif ( GB_BITS == 32)
                (*z) = GB_BITGET ((*x), k, int32_t, 32) ;
            #elif ( GB_BITS == 64)
                (*z) = GB_BITGET ((*x), k, int64_t, 64) ;
            #endif

        #elif defined ( GB_UNSIGNED_INT )

            #if ( GB_BITS == 8)
                (*z) = GB_BITGET ((*x), k, uint8_t,   8) ;
            #elif ( GB_BITS == 16)
                (*z) = GB_BITGET ((*x), k, uint16_t, 16) ;
            #elif ( GB_BITS == 32)
                (*z) = GB_BITGET ((*x), k, uint32_t, 32) ;
            #elif ( GB_BITS == 64)
                (*z) = GB_BITGET ((*x), k, uint64_t, 64) ;
            #endif

        #endif
    }

    inline void GB (BSET_f) (GB_Z_X_Y_ARGS)
    {
        // bitset (x,y) returns x modified by setting a bit from x to 1, whose
        // position is given by y.  If y is in the range 1 to GB_BITS, then y
        // gives the position of the bit to set.  If y is outside the range 1
        // to GB_BITS, then z = x is returned, unmodified.
        GB_TYPE k = (*y) ;

        #if defined ( GB_SIGNED_INT )

            #if ( GB_BITS == 8)
                (*z) = GB_BITSET ((*x), k, int8_t,   8) ;
            #elif ( GB_BITS == 16)
                (*z) = GB_BITSET ((*x), k, int16_t, 16) ;
            #elif ( GB_BITS == 32)
                (*z) = GB_BITSET ((*x), k, int32_t, 32) ;
            #elif ( GB_BITS == 64)
                (*z) = GB_BITSET ((*x), k, int64_t, 64) ;
            #endif

        #elif defined ( GB_UNSIGNED_INT )

            #if ( GB_BITS == 8)
                (*z) = GB_BITSET ((*x), k, uint8_t,   8) ;
            #elif ( GB_BITS == 16)
                (*z) = GB_BITSET ((*x), k, uint16_t, 16) ;
            #elif ( GB_BITS == 32)
                (*z) = GB_BITSET ((*x), k, uint32_t, 32) ;
            #elif ( GB_BITS == 64)
                (*z) = GB_BITSET ((*x), k, uint64_t, 64) ;
            #endif

        #endif
    }

    inline void GB (BCLR_f) (GB_Z_X_Y_ARGS)
    {
        // bitclr (x,y) returns x modified by setting a bit from x to 0, whose
        // position is given by y.  If y is in the range 1 to GB_BITS, then y
        // gives the position of the bit to clear.  If y is outside the range 1
        // to GB_BITS, then z = x is returned, unmodified.
        GB_TYPE k = (*y) ;

        #if defined ( GB_SIGNED_INT )

            #if ( GB_BITS == 8)
                (*z) = GB_BITCLR ((*x), k, int8_t,   8) ;
            #elif ( GB_BITS == 16)
                (*z) = GB_BITCLR ((*x), k, int16_t, 16) ;
            #elif ( GB_BITS == 32)
                (*z) = GB_BITCLR ((*x), k, int32_t, 32) ;
            #elif ( GB_BITS == 64)
                (*z) = GB_BITCLR ((*x), k, int64_t, 64) ;
            #endif

        #elif defined ( GB_UNSIGNED_INT )

            #if ( GB_BITS == 8)
                (*z) = GB_BITCLR ((*x), k, uint8_t,   8) ;
            #elif ( GB_BITS == 16)
                (*z) = GB_BITCLR ((*x), k, uint16_t, 16) ;
            #elif ( GB_BITS == 32)
                (*z) = GB_BITCLR ((*x), k, uint32_t, 32) ;
            #elif ( GB_BITS == 64)
                (*z) = GB_BITCLR ((*x), k, uint64_t, 64) ;
            #endif

        #endif
    }


    // z = bitshift (x,y)
    inline void GB (BSHIFT_f) (GB_TYPE *z, const GB_TYPE *x, const int8_t *y)
    {
        // bitshift (x,k) shifts x to the left by k bits if k > 0, and the
        // right by -k bits if k < 0.
        int8_t k = (int8_t) (*y) ;

        #if defined ( GB_SIGNED_INT )

            #if ( GB_BITS == 8)
                (*z) = GB_bitshift_int8 ((*x), k) ;
            #elif ( GB_BITS == 16)
                (*z) = GB_bitshift_int16 ((*x), k) ;
            #elif ( GB_BITS == 32)
                (*z) = GB_bitshift_int32 ((*x), k) ;
            #elif ( GB_BITS == 64)
                (*z) = GB_bitshift_int64 ((*x), k) ;
            #endif

        #elif defined ( GB_UNSIGNED_INT )

            #if ( GB_BITS == 8)
                (*z) = GB_bitshift_uint8 ((*x), k) ;
            #elif ( GB_BITS == 16)
                (*z) = GB_bitshift_uint16 ((*x), k) ;
            #elif ( GB_BITS == 32)
                (*z) = GB_bitshift_uint32 ((*x), k) ;
            #elif ( GB_BITS == 64)
                (*z) = GB_bitshift_uint64 ((*x), k) ;
            #endif

        #endif
    }

#endif

//------------------------------------------------------------------------------
// binary operators for real floating-point inputs only
//------------------------------------------------------------------------------

#if defined ( GB_FLOAT )

    inline void GB (ATAN2_f) (GB_Z_X_Y_ARGS) { (*z) = atan2f ((*x),(*y)) ; }
    inline void GB (HYPOT_f) (GB_Z_X_Y_ARGS) { (*z) = hypotf ((*x),(*y)) ; }
    inline void GB (FMOD_f)  (GB_Z_X_Y_ARGS) { (*z) = fmodf  ((*x),(*y)) ; }

    inline void GB (REMAINDER_f) (GB_Z_X_Y_ARGS)
    {
        (*z) = remainderf ((*x),(*y)) ;
    }
    inline void GB (COPYSIGN_f) (GB_Z_X_Y_ARGS)
    {
        (*z) = copysignf ((*x),(*y)) ;
    }
    inline void GB (LDEXP_f) (GB_Z_X_Y_ARGS)
    {
        (*z) = ldexpf ((*x), (int) truncf (*y)) ;
    }
    inline void GB (CMPLX_f) (GxB_FC32_t *z, const float *x, const float *y)
    {
        (*z) = GxB_CMPLXF ((*x),(*y)) ;
    }

#elif defined ( GB_DOUBLE )

    inline void GB (ATAN2_f) (GB_Z_X_Y_ARGS) { (*z) = atan2 ((*x),(*y)) ; }
    inline void GB (HYPOT_f) (GB_Z_X_Y_ARGS) { (*z) = hypot ((*x),(*y)) ; }
    inline void GB (FMOD_f)  (GB_Z_X_Y_ARGS) { (*z) = fmod  ((*x),(*y)) ; }

    inline void GB (REMAINDER_f) (GB_Z_X_Y_ARGS)
    {
        (*z) = remainder ((*x),(*y)) ;
    }
    inline void GB (COPYSIGN_f) (GB_Z_X_Y_ARGS)
    {
        (*z) = copysign ((*x),(*y)) ;
    }
    inline void GB (LDEXP_f) (GB_Z_X_Y_ARGS)
    {
        (*z) = ldexp ((*x), (int) trunc (*y)) ;
    }
    inline void GB (CMPLX_f) (GxB_FC64_t *z, const double *x, const double *y)
    {
        (*z) = GxB_CMPLX ((*x),(*y)) ;
    }

#endif

#if defined (GB_FLOAT) || defined (GB_DOUBLE)

    GB_PUBLIC struct GB_BinaryOp_opaque
        GB (opaque_GxB_ATAN2),
        GB (opaque_GxB_HYPOT),
        GB (opaque_GxB_FMOD),
        GB (opaque_GxB_REMAINDER),
        GB (opaque_GxB_COPYSIGN),
        GB (opaque_GxB_LDEXP),
        GB (opaque_GxB_CMPLX) ;

#endif

//------------------------------------------------------------------------------
// 6 binary comparison functions z=f(x,y), where x,y,z have the same type
//------------------------------------------------------------------------------

// iseq and isne: all 13 types, including complex types.
// isgt, islt, isge, isle: 11 real types only.

inline void GB (ISEQ_f) (GB_Z_X_Y_ARGS)
{
    #if defined ( GB_FLOAT_COMPLEX )
    (*z) = GB_FC32_iseq (*x, *y) ;
    #elif defined ( GB_DOUBLE_COMPLEX )
    (*z) = GB_FC64_iseq (*x, *y) ;
    #else
    (*z) = (GB_TYPE) ((*x) == (*y)) ;
    #endif
}

inline void GB (ISNE_f) (GB_Z_X_Y_ARGS)
{
    #if defined ( GB_FLOAT_COMPLEX )
    (*z) = GB_FC32_isne (*x, *y) ;
    #elif defined ( GB_DOUBLE_COMPLEX )
    (*z) = GB_FC64_isne (*x, *y) ;
    #else
    (*z) = (GB_TYPE) ((*x) != (*y)) ;
    #endif
}

GB_PUBLIC struct GB_BinaryOp_opaque
    GB (opaque_GxB_ISEQ),
    GB (opaque_GxB_ISNE) ;

#if defined ( GB_REAL )

    inline void GB (ISGT_f) (GB_Z_X_Y_ARGS) { (*z) = (GB_TYPE) ((*x) >  (*y)) ; }
    inline void GB (ISLT_f) (GB_Z_X_Y_ARGS) { (*z) = (GB_TYPE) ((*x) <  (*y)) ; }
    inline void GB (ISGE_f) (GB_Z_X_Y_ARGS) { (*z) = (GB_TYPE) ((*x) >= (*y)) ; }
    inline void GB (ISLE_f) (GB_Z_X_Y_ARGS) { (*z) = (GB_TYPE) ((*x) <= (*y)) ; }

    GB_PUBLIC struct GB_BinaryOp_opaque
        GB (opaque_GxB_ISGT),
        GB (opaque_GxB_ISLT),
        GB (opaque_GxB_ISGE),
        GB (opaque_GxB_ISLE) ;

#endif

//------------------------------------------------------------------------------
// 3 boolean binary functions z=f(x,y), all x,y,z the same type, real types only
//------------------------------------------------------------------------------

#if defined ( GB_REAL )
#if defined ( GB_BOOLEAN )
inline void GB (LOR_f)  (GB_Z_X_Y_ARGS) { (*z) = ((*x) || (*y)) ; }
inline void GB (LAND_f) (GB_Z_X_Y_ARGS) { (*z) = ((*x) && (*y)) ; }
inline void GB (LXOR_f) (GB_Z_X_Y_ARGS) { (*z) = ((*x) != (*y)) ; }
#else
// The inputs are of type T but are then implicitly converted to boolean
// The output z is of type T, either 1 or 0 in that type.
inline void GB (LOR_f)  (GB_Z_X_Y_ARGS) { (*z) = (GB_TYPE) (((*x) != 0) || ((*y) != 0)) ; }
inline void GB (LAND_f) (GB_Z_X_Y_ARGS) { (*z) = (GB_TYPE) (((*x) != 0) && ((*y) != 0)) ; }
inline void GB (LXOR_f) (GB_Z_X_Y_ARGS) { (*z) = (GB_TYPE) (((*x) != 0) != ((*y) != 0)) ; }
#endif

GB_PUBLIC struct GB_BinaryOp_opaque
    GB (opaque_GxB_LOR),
    GB (opaque_GxB_LAND),
    GB (opaque_GxB_LXOR) ;
#endif

#undef GB_Z_X_Y_ARGS

//------------------------------------------------------------------------------
// 6 binary functions z=f(x,y), returning bool
//------------------------------------------------------------------------------

// eq, ne: for all 13 types
// gt, lt, ge, le: for 11 real types, not complex

#define GB_Zbool_X_Y_ARGS bool *z, const GB_TYPE *x, const GB_TYPE *y

inline void GB (EQ_f) (GB_Zbool_X_Y_ARGS)
{
    #if defined ( GB_FLOAT_COMPLEX )
    (*z) = GB_FC32_eq (*x, *y) ;
    #elif defined ( GB_DOUBLE_COMPLEX )
    (*z) = GB_FC64_eq (*x, *y) ;
    #else
    (*z) = ((*x) == (*y)) ;
    #endif
}

inline void GB (NE_f) (GB_Zbool_X_Y_ARGS)
{
    #if defined ( GB_FLOAT_COMPLEX )
    (*z) = GB_FC32_ne (*x, *y) ;
    #elif defined ( GB_DOUBLE_COMPLEX )
    (*z) = GB_FC64_ne (*x, *y) ;
    #else
    (*z) = ((*x) != (*y)) ;
    #endif
}

#if defined ( GB_COMPLEX )

    GB_PUBLIC struct GB_BinaryOp_opaque
        GB (opaque_GxB_EQ),
        GB (opaque_GxB_NE) ;

#else

    inline void GB (GT_f) (GB_Zbool_X_Y_ARGS) { (*z) = ((*x) >  (*y)) ; }
    inline void GB (LT_f) (GB_Zbool_X_Y_ARGS) { (*z) = ((*x) <  (*y)) ; }
    inline void GB (GE_f) (GB_Zbool_X_Y_ARGS) { (*z) = ((*x) >= (*y)) ; }
    inline void GB (LE_f) (GB_Zbool_X_Y_ARGS) { (*z) = ((*x) <= (*y)) ; }

    GB_PUBLIC struct GB_BinaryOp_opaque
        GB (opaque_GrB_EQ),
        GB (opaque_GrB_NE),
        GB (opaque_GrB_GT),
        GB (opaque_GrB_LT),
        GB (opaque_GrB_GE),
        GB (opaque_GrB_LE) ;

#endif

#undef GB_Zbool_X_Y_ARGS

//------------------------------------------------------------------------------
// clear macros for next use of this file
//------------------------------------------------------------------------------

#undef GB
#undef GB_TYPE
#undef GB_OP
#undef GB_BOOLEAN
#undef GB_FLOATING_POINT
#undef GB_UNSIGNED_INT
#undef GB_SIGNED_INT
#undef GB_BITS
#undef GB_REAL
#undef GB_DOUBLE
#undef GB_FLOAT
#undef GB_DOUBLE_COMPLEX
#undef GB_FLOAT_COMPLEX
#undef GB_COMPLEX

