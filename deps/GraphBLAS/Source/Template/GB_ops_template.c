//------------------------------------------------------------------------------
// GB_ops_template.c: built-in unary and binary functions and operators
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// This file is #include'd many times in GB_builtin.c to define the built-in
// unary and binary operators.  In that file, TYPE is a built-in C type (bool,
// int8_t, uint64_t, double, etc), and GB(x) is the corresponding macro that
// creates the function name (GB_*_BOOL, GB_*_INT8, etc).

// helper macros to access the input and output arguments, x, y, and z
#define X (*x)
#define Y (*y)
#define Z (*z)

#define Z_X_ARGS       TYPE *z, const TYPE *x
#define Z_X_Y_ARGS     TYPE *z, const TYPE *x, const TYPE *y
#define Zbool_X_Y_ARGS bool *z, const TYPE *x, const TYPE *y

//------------------------------------------------------------------------------
// define the built-in GraphBLAS functions
//------------------------------------------------------------------------------

// See Source/GB.h for discussion of special cases for MINV, MIN, MAX, and DIV.

// 6 unary functions z=f(x), where z and x have the same type
void GB (ONE_f)      (Z_X_ARGS) { Z = 1 ; }
void GB (IDENTITY_f) (Z_X_ARGS) { Z = X ; }

#ifdef FLOATING_POINT
// floating-point
void GB (AINV_f)     (Z_X_ARGS) { Z = -X ; }
void GB (ABS_f)      (Z_X_ARGS) { Z = FABS (X) ; }
void GB (MINV_f)     (Z_X_ARGS) { Z = 1. / (X) ; }  // floating-point 1/x
#else
#ifdef BOOLEAN
// boolean
void GB (AINV_f)     (Z_X_ARGS) { Z = X ; }
void GB (ABS_f)      (Z_X_ARGS) { Z = X ; }
void GB (MINV_f)     (Z_X_ARGS) { Z = true ; }  // see comments in Source/GB.h
#else
#ifdef UNSIGNED
// unsigned integer
void GB (AINV_f)     (Z_X_ARGS) { Z = -X ; }    // modulo wrap is intentional
void GB (ABS_f)      (Z_X_ARGS) { Z = X ; }
void GB (MINV_f)     (Z_X_ARGS) { Z = IMINV (X) ; }
#else
// signed integer
void GB (AINV_f)     (Z_X_ARGS) { Z = -X ; }
void GB (ABS_f)      (Z_X_ARGS) { Z = IABS (X) ; }
void GB (MINV_f)     (Z_X_ARGS) { Z = IMINV (X) ; }
#endif
#endif
#endif

#ifdef BOOLEAN
void GB (LNOT_f)     (Z_X_ARGS) { Z = ! X ; }
#else
void GB (LNOT_f)     (Z_X_ARGS) { Z = ! (X != 0) ; }
#endif

// 8 binary functions z=f(x,y) where x,y,z have the same type
void GB (FIRST_f)  (Z_X_Y_ARGS) { Z = X ; }
void GB (SECOND_f) (Z_X_Y_ARGS) { Z = Y ; }
#ifdef FLOATING_POINT
void GB (MIN_f)    (Z_X_Y_ARGS) { Z = FMIN (X,Y) ; }    // special NaN cases
void GB (MAX_f)    (Z_X_Y_ARGS) { Z = FMAX (X,Y) ; }
#else
void GB (MIN_f)    (Z_X_Y_ARGS) { Z = IMIN (X,Y) ; }
void GB (MAX_f)    (Z_X_Y_ARGS) { Z = IMAX (X,Y) ; }
#endif
void GB (PLUS_f)   (Z_X_Y_ARGS) { Z = X + Y ; }
void GB (MINUS_f)  (Z_X_Y_ARGS) { Z = X - Y ; }
void GB (TIMES_f)  (Z_X_Y_ARGS) { Z = X * Y ; }
#ifdef FLOATING_POINT
void GB (DIV_f)    (Z_X_Y_ARGS) { Z = X / Y ; } // floating-point division
#else
#ifdef BOOLEAN
void GB (DIV_f)    (Z_X_Y_ARGS) { Z = X ; }     // see comments in Source/GB.h
#else
void GB (DIV_f)    (Z_X_Y_ARGS) { Z = IDIV (X,Y) ; }    // int* and uint* only
#endif
#endif

// 6 binary comparison functions z=f(x,y), where x,y,z have the same type
void GB (ISEQ_f)   (Z_X_Y_ARGS) { Z = (X == Y) ; }
void GB (ISNE_f)   (Z_X_Y_ARGS) { Z = (X != Y) ; }
void GB (ISGT_f)   (Z_X_Y_ARGS) { Z = (X >  Y) ; }
void GB (ISLT_f)   (Z_X_Y_ARGS) { Z = (X <  Y) ; }
void GB (ISGE_f)   (Z_X_Y_ARGS) { Z = (X >= Y) ; }
void GB (ISLE_f)   (Z_X_Y_ARGS) { Z = (X <= Y) ; }

// 3 boolean binary functions z=f(x,y), all x,y,z the same type
#ifdef BOOLEAN
void GB (LOR_f)    (Z_X_Y_ARGS) { Z = (X || Y) ; }
void GB (LAND_f)   (Z_X_Y_ARGS) { Z = (X && Y) ; }
void GB (LXOR_f)   (Z_X_Y_ARGS) { Z = (X != Y) ; }
#else
// The inputs are of type T but are then implicitly converted to boolean
// The output z is of type T, either 1 or 0 in that type.
void GB (LOR_f)    (Z_X_Y_ARGS) { Z = ((X != 0) || (Y != 0)) ; }
void GB (LAND_f)   (Z_X_Y_ARGS) { Z = ((X != 0) && (Y != 0)) ; }
void GB (LXOR_f)   (Z_X_Y_ARGS) { Z = ((X != 0) != (Y != 0)) ; }
#endif

// 6 binary functions z=f(x,y) for any built-in type but return bool
void GB (EQ_f) (Zbool_X_Y_ARGS) { Z = (X == Y) ; }
void GB (NE_f) (Zbool_X_Y_ARGS) { Z = (X != Y) ; }
void GB (GT_f) (Zbool_X_Y_ARGS) { Z = (X >  Y) ; }
void GB (LT_f) (Zbool_X_Y_ARGS) { Z = (X <  Y) ; }
void GB (GE_f) (Zbool_X_Y_ARGS) { Z = (X >= Y) ; }
void GB (LE_f) (Zbool_X_Y_ARGS) { Z = (X <= Y) ; }

//------------------------------------------------------------------------------
// define the GraphBLAS operators
//------------------------------------------------------------------------------

// 6 unary operators z=f(x), z and x the same type
UNARY (GxB_, ONE,      "one") ;
UNARY (GrB_, IDENTITY, "identity") ;
UNARY (GrB_, AINV,     "ainv") ;
UNARY (GxB_, ABS,      "abs") ;
UNARY (GrB_, MINV,     "minv") ;
UNARY (GxB_, LNOT,     "not") ;

// 8 binary operators z=f(x,y) where x,y,z are all the same type
BINARY (GrB_, FIRST,  "first") ;
BINARY (GrB_, SECOND, "second") ;
BINARY (GrB_, MIN,    "min") ;
BINARY (GrB_, MAX,    "max") ;
BINARY (GrB_, PLUS,   "plus") ;
BINARY (GrB_, MINUS,  "minus") ;
BINARY (GrB_, TIMES,  "times") ;
BINARY (GrB_, DIV,    "div") ;

// 6 binary comparison operators z=f(x,y) where x,y,z are all the same type
BINARY (GxB_, ISEQ,  "iseq") ;
BINARY (GxB_, ISNE,  "isne") ;
BINARY (GxB_, ISGT,  "isgt") ;
BINARY (GxB_, ISLT,  "islt") ;
BINARY (GxB_, ISGE,  "isge") ;
BINARY (GxB_, ISLE,  "isle") ;

// 3 boolean operators z=f(x,y), where x,y,z are all the same type
BINARY (GxB_, LOR,  "or") ;
BINARY (GxB_, LAND, "and") ;
BINARY (GxB_, LXOR, "xor") ;

// 6 binary operators z=f(x,y) that return bool z for any types x and y:
BINARY_BOOL (GrB_, EQ, "eq") ;
BINARY_BOOL (GrB_, NE, "ne") ;
BINARY_BOOL (GrB_, GT, "gt") ;
BINARY_BOOL (GrB_, LT, "lt") ;
BINARY_BOOL (GrB_, GE, "ge") ;
BINARY_BOOL (GrB_, LE, "le") ;

//------------------------------------------------------------------------------
// define unary typecast operators, used in GB_cast_factory.c
//------------------------------------------------------------------------------

// GB_cast_ztype_xtype casts a single value ((xtype) x) into the result
// ((ztype) z).  Functions with the xtype and ztype the same, such as
// GB_cast_double_double, simply copy their input to their output.  For more
// details, see the CAST(z,x) macro in GB.h.

// The s parameter is not used in these functions.  It is present because one
// function returned by GB_cast_factory requires it (GB_copy_user_user).

#define CAST_FUNCTION(xtype)                                                \
    (                                                                       \
        void *z,            /* typecasted output, of type ztype */          \
        const void *x,      /* input value to typecast, of type xtype */    \
        size_t s            /* size of type, for GB_copy_user_user only */  \
    )                                                                       \
    {                                                                       \
        /* the types of z and x are known at compile time */                \
        CAST ((*((TYPE *) z)) , (*((const xtype *) x))) ;                   \
    }

void CAST_NAME (bool    ) CAST_FUNCTION (bool    )
void CAST_NAME (int8_t  ) CAST_FUNCTION (int8_t  )
void CAST_NAME (uint8_t ) CAST_FUNCTION (uint8_t )
void CAST_NAME (int16_t ) CAST_FUNCTION (int16_t )
void CAST_NAME (uint16_t) CAST_FUNCTION (uint16_t)
void CAST_NAME (int32_t ) CAST_FUNCTION (int32_t )
void CAST_NAME (uint32_t) CAST_FUNCTION (uint32_t)
void CAST_NAME (int64_t ) CAST_FUNCTION (int64_t )
void CAST_NAME (uint64_t) CAST_FUNCTION (uint64_t)
void CAST_NAME (float   ) CAST_FUNCTION (float   )
void CAST_NAME (double  ) CAST_FUNCTION (double  )

#undef X
#undef Y
#undef Z
#undef Zbool
#undef TYPE
#undef GB
#undef GRB_NAME
#undef CAST_NAME
#undef CAST_FUNCTION
#undef BOOLEAN
#undef FLOATING_POINT
#undef UNSIGNED

