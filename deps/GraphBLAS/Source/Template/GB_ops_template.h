//------------------------------------------------------------------------------
// GB_ops_template.h: define the unary and binary functions and operators
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// This file is #include'd many times in GB.h to define the unary and binary
// functions.  That file defines the GB(x) macro, which is GB_##x##_BOOL,
// GB_##x##_INT8, etc.

#define GB_Z_X_ARGS       GB_TYPE *z, const GB_TYPE *x
#define GB_Z_X_Y_ARGS     GB_TYPE *z, const GB_TYPE *x, const GB_TYPE *y
#define GB_Zbool_X_Y_ARGS bool *z, const GB_TYPE *x, const GB_TYPE *y

//------------------------------------------------------------------------------
// 6 unary functions z=f(x), where z and x have the same type
//------------------------------------------------------------------------------

// one, identity
inline void GB (ONE_f)      (GB_Z_X_ARGS) { (*z) = 1 ; }
inline void GB (IDENTITY_f) (GB_Z_X_ARGS) { (*z) = (*x) ; }

// ainv, abs, minv
#ifdef GB_FLOATING_POINT
    // floating-point
    inline void GB (AINV_f) (GB_Z_X_ARGS) { (*z) = -(*x) ; }
    #ifdef GB_FLOATING_POINT_DOUBLE
        inline void GB (ABS_f)  (GB_Z_X_ARGS) { (*z) = fabs ((*x)) ; }
    #else
        inline void GB (ABS_f)  (GB_Z_X_ARGS) { (*z) = fabsf ((*x)) ; }
    #endif
    inline void GB (MINV_f) (GB_Z_X_ARGS) { (*z) = 1 / ((*x)) ; }
#else
    #ifdef GB_BOOLEAN
        // boolean
        inline void GB (AINV_f) (GB_Z_X_ARGS) { (*z) = (*x) ; }
        inline void GB (ABS_f)  (GB_Z_X_ARGS) { (*z) = (*x) ; }
        inline void GB (MINV_f) (GB_Z_X_ARGS) { (*z) = true ; }
    #else
        // integer
        inline void GB (AINV_f) (GB_Z_X_ARGS) { (*z) = -(*x) ; }
        #ifdef GB_UNSIGNED
            // unsigned integer
            inline void GB (ABS_f)  (GB_Z_X_ARGS) { (*z) = (*x) ; }
            inline void GB (MINV_f) (GB_Z_X_ARGS)
            {
                (*z) = GB_IMINV_UNSIGNED ((*x), GB_BITS) ;
            }
        #else
            // signed integer
            inline void GB (ABS_f)  (GB_Z_X_ARGS) { (*z) = GB_IABS ((*x)) ; }
            inline void GB (MINV_f) (GB_Z_X_ARGS)
            {
                (*z) = GB_IMINV_SIGNED ((*x), GB_BITS) ;
            }
        #endif
    #endif
#endif

// logical not
#ifdef GB_BOOLEAN
    inline void GB (LNOT_f)     (GB_Z_X_ARGS) { (*z) = ! (*x) ; }
#else
    inline void GB (LNOT_f)     (GB_Z_X_ARGS) { (*z) = ! ((*x) != 0) ; }
#endif

GB_PUBLIC struct GB_UnaryOp_opaque
    GB (opaque_GxB_ONE),
    GB (opaque_GrB_IDENTITY),
    GB (opaque_GrB_AINV),
    GB (opaque_GxB_ABS),
    GB (opaque_GrB_MINV),
    GB (opaque_GxB_LNOT) ;

//------------------------------------------------------------------------------
// 12 binary functions z=f(x,y) where x,y,z have the same type
//------------------------------------------------------------------------------

inline void GB (FIRST_f)  (GB_Z_X_Y_ARGS) { (*z) = (*x) ; }
inline void GB (SECOND_f) (GB_Z_X_Y_ARGS) { (*z) = (*y) ; }
inline void GB (PAIR_f)   (GB_Z_X_Y_ARGS) { (*z) = 1 ; }
inline void GB (ANY_f)    (GB_Z_X_Y_ARGS) { (*z) = (*y) ; } // same as SECOND

inline void GB (PLUS_f)   (GB_Z_X_Y_ARGS) { (*z) = (*x) + (*y) ; }
inline void GB (MINUS_f)  (GB_Z_X_Y_ARGS) { (*z) = (*x) - (*y) ; }
inline void GB (RMINUS_f) (GB_Z_X_Y_ARGS) { (*z) = (*y) - (*x) ; }
inline void GB (TIMES_f)  (GB_Z_X_Y_ARGS) { (*z) = (*x) * (*y) ; }

// min, max
#ifdef GB_FLOATING_POINT
    #ifdef GB_FLOATING_POINT_DOUBLE
        // double min, max
        inline void GB (MIN_f) (GB_Z_X_Y_ARGS) { (*z) = fmin ((*x),(*y)) ; }
        inline void GB (MAX_f) (GB_Z_X_Y_ARGS) { (*z) = fmax ((*x),(*y)) ; }
    #else
        // float min, max
        inline void GB (MIN_f) (GB_Z_X_Y_ARGS) { (*z) = fminf ((*x),(*y)) ; }
        inline void GB (MAX_f) (GB_Z_X_Y_ARGS) { (*z) = fmaxf ((*x),(*y)) ; }
    #endif
#else
    // integer min, max
    inline void GB (MIN_f) (GB_Z_X_Y_ARGS) { (*z) = GB_IMIN ((*x),(*y)) ; }
    inline void GB (MAX_f) (GB_Z_X_Y_ARGS) { (*z) = GB_IMAX ((*x),(*y)) ; }
#endif

// div and rdiv
#ifdef GB_FLOATING_POINT
    // float and double div, rdiv
    inline void GB (DIV_f)  (GB_Z_X_Y_ARGS) { (*z) = (*x) / (*y) ; }
    inline void GB (RDIV_f) (GB_Z_X_Y_ARGS) { (*z) = (*y) / (*x) ; }
#else
    #ifdef GB_BOOLEAN
        // boolean div (== first)
        inline void GB (DIV_f)  (GB_Z_X_Y_ARGS) { (*z) = (*x) ; }
        // boolean rdiv (== second)
        inline void GB (RDIV_f) (GB_Z_X_Y_ARGS) { (*z) = (*y) ; }
    #else
        #ifdef GB_UNSIGNED
            // unsigned integer div, rdiv
            inline void GB (DIV_f)  (GB_Z_X_Y_ARGS)
            {
                (*z) = GB_IDIV_UNSIGNED ((*x),(*y),GB_BITS) ;
            }
            inline void GB (RDIV_f) (GB_Z_X_Y_ARGS)
            {
                (*z) = GB_IDIV_UNSIGNED ((*y),(*x),GB_BITS) ;
            }
        #else
            // signed integer div, rdiv
            inline void GB (DIV_f)  (GB_Z_X_Y_ARGS)
            {
                (*z) = GB_IDIV_SIGNED ((*x),(*y),GB_BITS) ;
            }
            inline void GB (RDIV_f) (GB_Z_X_Y_ARGS)
            {
                (*z) = GB_IDIV_SIGNED ((*y),(*x),GB_BITS) ;
            }
        #endif
    #endif
#endif

GB_PUBLIC struct GB_BinaryOp_opaque
    GB (opaque_GrB_FIRST),
    GB (opaque_GrB_SECOND),
    GB (opaque_GxB_PAIR),
    GB (opaque_GxB_ANY),

    GB (opaque_GrB_PLUS),
    GB (opaque_GrB_MINUS),
    GB (opaque_GxB_RMINUS),
    GB (opaque_GrB_TIMES),

    GB (opaque_GrB_MIN),
    GB (opaque_GrB_MAX),
    GB (opaque_GrB_DIV),
    GB (opaque_GxB_RDIV) ;

//------------------------------------------------------------------------------
// 6 binary comparison functions z=f(x,y), where x,y,z have the same type
//------------------------------------------------------------------------------

inline void GB (ISEQ_f)   (GB_Z_X_Y_ARGS) { (*z) = ((*x) == (*y)) ; }
inline void GB (ISNE_f)   (GB_Z_X_Y_ARGS) { (*z) = ((*x) != (*y)) ; }
inline void GB (ISGT_f)   (GB_Z_X_Y_ARGS) { (*z) = ((*x) >  (*y)) ; }
inline void GB (ISLT_f)   (GB_Z_X_Y_ARGS) { (*z) = ((*x) <  (*y)) ; }
inline void GB (ISGE_f)   (GB_Z_X_Y_ARGS) { (*z) = ((*x) >= (*y)) ; }
inline void GB (ISLE_f)   (GB_Z_X_Y_ARGS) { (*z) = ((*x) <= (*y)) ; }

GB_PUBLIC struct GB_BinaryOp_opaque
    GB (opaque_GxB_ISEQ),
    GB (opaque_GxB_ISNE),
    GB (opaque_GxB_ISGT),
    GB (opaque_GxB_ISLT),
    GB (opaque_GxB_ISGE),
    GB (opaque_GxB_ISLE) ;

//------------------------------------------------------------------------------
// 3 boolean binary functions z=f(x,y), all x,y,z the same type
//------------------------------------------------------------------------------

#ifdef GB_BOOLEAN
inline void GB (LOR_f)    (GB_Z_X_Y_ARGS) { (*z) = ((*x) || (*y)) ; }
inline void GB (LAND_f)   (GB_Z_X_Y_ARGS) { (*z) = ((*x) && (*y)) ; }
inline void GB (LXOR_f)   (GB_Z_X_Y_ARGS) { (*z) = ((*x) != (*y)) ; }
#else
// The inputs are of type T but are then implicitly converted to boolean
// The output z is of type T, either 1 or 0 in that type.
inline void GB (LOR_f)  (GB_Z_X_Y_ARGS) { (*z) = (((*x) != 0) || ((*y) != 0)) ;}
inline void GB (LAND_f) (GB_Z_X_Y_ARGS) { (*z) = (((*x) != 0) && ((*y) != 0)) ;}
inline void GB (LXOR_f) (GB_Z_X_Y_ARGS) { (*z) = (((*x) != 0) != ((*y) != 0)) ;}
#endif

GB_PUBLIC struct GB_BinaryOp_opaque
    GB (opaque_GxB_LOR),
    GB (opaque_GxB_LAND),
    GB (opaque_GxB_LXOR) ;

//------------------------------------------------------------------------------
// 6 binary functions z=f(x,y) for any built-in type but return bool
//------------------------------------------------------------------------------

inline void GB (EQ_f) (GB_Zbool_X_Y_ARGS) { (*z) = ((*x) == (*y)) ; }
inline void GB (NE_f) (GB_Zbool_X_Y_ARGS) { (*z) = ((*x) != (*y)) ; }
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

//------------------------------------------------------------------------------
// unary typecast operators, used in GB_cast_factory.c
//------------------------------------------------------------------------------

// unary casts:  for example, GB_cast_bool_int8_t casts the input (bool) x to
// the result (int8_t) z.  The s argument is not used for built-in types, but
// these functions must have the same signature as GB_copy_user_user, which
// needs to know the size of the user-defined type.

// GB_cast_ztype_xtype casts a single value ((xtype) x) into the result
// ((ztype) z).  Functions with the xtype and ztype the same, such as
// GB_cast_double_double, simply copy their input to their output.  For more
// details, see the GB_CAST_* macros in GB.h.

// The s parameter is not used in these functions.  It is present because one
// function returned by GB_cast_factory requires it (GB_copy_user_user).

#define GB_CAST_FUNCTION_ANSI(xtype)                                        \
    (                                                                       \
        void *z,            /* typecasted output, of type ztype */          \
        const void *x,      /* input value to typecast, of type xtype */    \
        size_t s            /* size of type, for GB_copy_user_user only */  \
    )                                                                       \
    {                                                                       \
        /* the types of z and x are known at compile time */                \
        GB_TYPE zz ;                                                        \
        xtype xx = (*((xtype *) x)) ;                                       \
        zz = (GB_TYPE) xx ;                                                 \
        (*((GB_TYPE *) z)) = zz ;                                           \
    }

// the argument is the xtype, and for these cases is not float or double, so
// use the ANSI C11 typecasting rules.
inline void GB_CAST_NAME (bool    ) GB_CAST_FUNCTION_ANSI (bool    )
inline void GB_CAST_NAME (int8_t  ) GB_CAST_FUNCTION_ANSI (int8_t  )
inline void GB_CAST_NAME (int16_t ) GB_CAST_FUNCTION_ANSI (int16_t )
inline void GB_CAST_NAME (int32_t ) GB_CAST_FUNCTION_ANSI (int32_t )
inline void GB_CAST_NAME (int64_t ) GB_CAST_FUNCTION_ANSI (int64_t )
inline void GB_CAST_NAME (uint8_t ) GB_CAST_FUNCTION_ANSI (uint8_t )
inline void GB_CAST_NAME (uint16_t) GB_CAST_FUNCTION_ANSI (uint16_t)
inline void GB_CAST_NAME (uint32_t) GB_CAST_FUNCTION_ANSI (uint32_t)
inline void GB_CAST_NAME (uint64_t) GB_CAST_FUNCTION_ANSI (uint64_t)

#define GB_CAST_FUNCTION_UNSIGNED(xtype,bits)                               \
    (                                                                       \
        void *z,            /* typecasted output, of type ztype */          \
        const void *x,      /* input value to typecast, of type xtype */    \
        size_t s            /* size of type, for GB_copy_user_user only */  \
    )                                                                       \
    {                                                                       \
        /* the types of z and x are known at compile time */                \
        GB_TYPE zz ;                                                        \
        xtype xx = (*((xtype *) x)) ;                                       \
        GB_CAST_UNSIGNED (zz, xx, bits) ;                                   \
        (*((GB_TYPE *) z)) = zz ;                                           \
    }

#define GB_CAST_FUNCTION_SIGNED(xtype,bits)                                 \
    (                                                                       \
        void *z,            /* typecasted output, of type ztype */          \
        const void *x,      /* input value to typecast, of type xtype */    \
        size_t s            /* size of type, for GB_copy_user_user only */  \
    )                                                                       \
    {                                                                       \
        /* the types of z and x are known at compile time */                \
        GB_TYPE zz ;                                                        \
        xtype xx = (*((xtype *) x)) ;                                       \
        GB_CAST_SIGNED (zz, xx, bits) ;                                     \
        (*((GB_TYPE *) z)) = zz ;                                           \
    }

// xtype is float or double; check for typecasting to integer
#ifdef GB_FLOATING_POINT
    // ztype is float or double
    inline void GB_CAST_NAME (float ) GB_CAST_FUNCTION_ANSI (float )
    inline void GB_CAST_NAME (double) GB_CAST_FUNCTION_ANSI (double)
#else
    #ifdef GB_BOOLEAN
        // ztype is bool
        inline void GB_CAST_NAME (float ) GB_CAST_FUNCTION_ANSI (float )
        inline void GB_CAST_NAME (double) GB_CAST_FUNCTION_ANSI (double)
    #else
        #ifdef GB_UNSIGNED
            // ztype is unsigned integer, xtype is float or double
            inline void GB_CAST_NAME (float )
                GB_CAST_FUNCTION_UNSIGNED (float, GB_BITS)
            inline void GB_CAST_NAME (double)
                GB_CAST_FUNCTION_UNSIGNED (double, GB_BITS)
        #else
            // ztype is signed integer, xtype is float or double
            inline void GB_CAST_NAME (float )
                GB_CAST_FUNCTION_SIGNED (float, GB_BITS)
            inline void GB_CAST_NAME (double)
                GB_CAST_FUNCTION_SIGNED (double, GB_BITS)
        #endif
    #endif
#endif

//------------------------------------------------------------------------------
// clear macros for next use of this file
//------------------------------------------------------------------------------

#undef GB
#undef GB_TYPE

#undef GB_Z_X_ARGS
#undef GB_Z_X_Y_ARGS
#undef GB_Zbool_X_Y_ARGS

#undef GrB_NAME
#undef GB_CAST_NAME
#undef GB_CAST_FUNCTION
#undef GB_CAST_FUNCTION_ANSI
#undef GB_CAST_FUNCTION_SIGNED
#undef GB_CAST_FUNCTION_UNSIGNED
#undef GB_BOOLEAN
#undef GB_FLOATING_POINT
#undef GB_FLOATING_POINT_DOUBLE
#undef GB_UNSIGNED
#undef GB_BITS

