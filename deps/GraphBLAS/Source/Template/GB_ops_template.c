//------------------------------------------------------------------------------
// GB_ops_template.c: built-in unary and binary functions and operators
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// This file is #include'd many times in GB_ops.c to define the built-in unary
// and binary operators.  In that file, GB_TYPE is a built-in C type (bool,
// int8_t, uint64_t, double, etc), and GB(x) is the corresponding macro that
// creates the function name (GB_*_BOOL, GB_*_INT8, etc).

#define GB_Z_X_ARGS       GB_TYPE *z, const GB_TYPE *x
#define GB_Z_X_Y_ARGS     GB_TYPE *z, const GB_TYPE *x, const GB_TYPE *y
#define GB_Zbool_X_Y_ARGS bool *z, const GB_TYPE *x, const GB_TYPE *y

//------------------------------------------------------------------------------
// 6 unary functions z=f(x), where z and x have the same type
//------------------------------------------------------------------------------

extern void GB (ONE_f)      (GB_Z_X_ARGS) ;
extern void GB (IDENTITY_f) (GB_Z_X_ARGS) ;
extern void GB (AINV_f)     (GB_Z_X_ARGS) ;
extern void GB (ABS_f)      (GB_Z_X_ARGS) ;
extern void GB (MINV_f)     (GB_Z_X_ARGS) ;
extern void GB (LNOT_f)     (GB_Z_X_ARGS) ;

GB_UNARY_OP_DEFINE (GxB_, ONE,      "one")
GB_UNARY_OP_DEFINE (GrB_, IDENTITY, "identity")
GB_UNARY_OP_DEFINE (GrB_, AINV,     "ainv")
GB_UNARY_OP_DEFINE (GxB_, ABS,      "abs")
GB_UNARY_OP_DEFINE (GrB_, MINV,     "minv")
GB_UNARY_OP_DEFINE (GxB_, LNOT,     "not")

//------------------------------------------------------------------------------
// 12 binary functions z=f(x,y) where x,y,z have the same type
//------------------------------------------------------------------------------

extern void GB (FIRST_f)  (GB_Z_X_Y_ARGS) ;
extern void GB (SECOND_f) (GB_Z_X_Y_ARGS) ;
extern void GB (PAIR_f)   (GB_Z_X_Y_ARGS) ;
extern void GB (ANY_f)    (GB_Z_X_Y_ARGS) ;

extern void GB (PLUS_f)   (GB_Z_X_Y_ARGS) ;
extern void GB (MINUS_f)  (GB_Z_X_Y_ARGS) ;
extern void GB (RMINUS_f) (GB_Z_X_Y_ARGS) ;
extern void GB (TIMES_f)  (GB_Z_X_Y_ARGS) ;

extern void GB (MIN_f)    (GB_Z_X_Y_ARGS) ;
extern void GB (MAX_f)    (GB_Z_X_Y_ARGS) ;
extern void GB (DIV_f)    (GB_Z_X_Y_ARGS) ;
extern void GB (RDIV_f)   (GB_Z_X_Y_ARGS) ;

GB_BINARY_OP_DEFINE (GrB_, FIRST,  "first" )
GB_BINARY_OP_DEFINE (GrB_, SECOND, "second")
GB_BINARY_OP_DEFINE (GxB_, PAIR,   "pair"  )
GB_BINARY_OP_DEFINE (GxB_, ANY,    "any"   )

GB_BINARY_OP_DEFINE (GrB_, PLUS,   "plus"  )
GB_BINARY_OP_DEFINE (GrB_, MINUS,  "minus" )
GB_BINARY_OP_DEFINE (GxB_, RMINUS, "rminus")
GB_BINARY_OP_DEFINE (GrB_, TIMES,  "times" )

GB_BINARY_OP_DEFINE (GrB_, MIN,    "min"   )
GB_BINARY_OP_DEFINE (GrB_, MAX,    "max"   )
GB_BINARY_OP_DEFINE (GrB_, DIV,    "div"   )
GB_BINARY_OP_DEFINE (GxB_, RDIV,   "rdiv"  )

//------------------------------------------------------------------------------
// 6 binary comparison functions z=f(x,y), where x,y,z have the same type
//------------------------------------------------------------------------------

extern void GB (ISEQ_f) (GB_Z_X_Y_ARGS) ;
extern void GB (ISNE_f) (GB_Z_X_Y_ARGS) ;
extern void GB (ISGT_f) (GB_Z_X_Y_ARGS) ;
extern void GB (ISLT_f) (GB_Z_X_Y_ARGS) ;
extern void GB (ISGE_f) (GB_Z_X_Y_ARGS) ;
extern void GB (ISLE_f) (GB_Z_X_Y_ARGS) ;

GB_BINARY_OP_DEFINE (GxB_, ISEQ,  "iseq")
GB_BINARY_OP_DEFINE (GxB_, ISNE,  "isne")
GB_BINARY_OP_DEFINE (GxB_, ISGT,  "isgt")
GB_BINARY_OP_DEFINE (GxB_, ISLT,  "islt")
GB_BINARY_OP_DEFINE (GxB_, ISGE,  "isge")
GB_BINARY_OP_DEFINE (GxB_, ISLE,  "isle")

//------------------------------------------------------------------------------
// 3 boolean binary functions z=f(x,y), all x,y,z the same type
//------------------------------------------------------------------------------

extern void GB (LOR_f)  (GB_Z_X_Y_ARGS) ;
extern void GB (LAND_f) (GB_Z_X_Y_ARGS) ;
extern void GB (LXOR_f) (GB_Z_X_Y_ARGS) ;

GB_BINARY_OP_DEFINE (GxB_, LOR,  "or" )
GB_BINARY_OP_DEFINE (GxB_, LAND, "and")
GB_BINARY_OP_DEFINE (GxB_, LXOR, "xor")

//------------------------------------------------------------------------------
// 6 binary functions z=f(x,y) for any built-in type but return bool
//------------------------------------------------------------------------------

extern void GB (EQ_f) (GB_Zbool_X_Y_ARGS) ;
extern void GB (NE_f) (GB_Zbool_X_Y_ARGS) ;
extern void GB (GT_f) (GB_Zbool_X_Y_ARGS) ;
extern void GB (LT_f) (GB_Zbool_X_Y_ARGS) ;
extern void GB (GE_f) (GB_Zbool_X_Y_ARGS) ;
extern void GB (LE_f) (GB_Zbool_X_Y_ARGS) ;

GB_BINARY_BOOL_OP_DEFINE (GrB_, EQ, "eq")
GB_BINARY_BOOL_OP_DEFINE (GrB_, NE, "ne")
GB_BINARY_BOOL_OP_DEFINE (GrB_, GT, "gt")
GB_BINARY_BOOL_OP_DEFINE (GrB_, LT, "lt")
GB_BINARY_BOOL_OP_DEFINE (GrB_, GE, "ge")
GB_BINARY_BOOL_OP_DEFINE (GrB_, LE, "le")

//------------------------------------------------------------------------------
// unary typecast operators, used in GB_cast_factory.c
//------------------------------------------------------------------------------

extern void GB_CAST_NAME (bool    ) (void *z, const void *x, size_t s) ;
extern void GB_CAST_NAME (int8_t  ) (void *z, const void *x, size_t s) ;
extern void GB_CAST_NAME (uint8_t ) (void *z, const void *x, size_t s) ;
extern void GB_CAST_NAME (int16_t ) (void *z, const void *x, size_t s) ;
extern void GB_CAST_NAME (uint16_t) (void *z, const void *x, size_t s) ;
extern void GB_CAST_NAME (int32_t ) (void *z, const void *x, size_t s) ;
extern void GB_CAST_NAME (uint32_t) (void *z, const void *x, size_t s) ;
extern void GB_CAST_NAME (int64_t ) (void *z, const void *x, size_t s) ;
extern void GB_CAST_NAME (uint64_t) (void *z, const void *x, size_t s) ;
extern void GB_CAST_NAME (float   ) (void *z, const void *x, size_t s) ;
extern void GB_CAST_NAME (double  ) (void *z, const void *x, size_t s) ;

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
#undef GB_BOOLEAN
#undef GB_FLOATING_POINT
#undef GB_UNSIGNED

