//------------------------------------------------------------------------------
// GB_ops_template.h: define the unary and binary functions and operators
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// This file is #include'd many times in GB.h to define the unary and binary
// functions.  That file defines the GB(x) macro, which is GB_##x##_BOOL,
// GB_##x##_INT8, etc.

#define Z_X_ARGS       TYPE *z, const TYPE *x
#define Z_X_Y_ARGS     TYPE *z, const TYPE *x, const TYPE *y
#define Zbool_X_Y_ARGS bool *z, const TYPE *x, const TYPE *y

// unary functions, z = f(x)
void GB (ONE_f)      (Z_X_ARGS) ;
void GB (IDENTITY_f) (Z_X_ARGS) ;
void GB (AINV_f)     (Z_X_ARGS) ;
void GB (ABS_f)      (Z_X_ARGS) ;
void GB (MINV_f)     (Z_X_ARGS) ;
void GB (LNOT_f)     (Z_X_ARGS) ;

// binary functions that return the same type as their inputs, z = f (x,y)
void GB (FIRST_f)  (Z_X_Y_ARGS) ;
void GB (SECOND_f) (Z_X_Y_ARGS) ;
void GB (MIN_f)    (Z_X_Y_ARGS) ;
void GB (MAX_f)    (Z_X_Y_ARGS) ;
void GB (PLUS_f)   (Z_X_Y_ARGS) ;
void GB (MINUS_f)  (Z_X_Y_ARGS) ;
void GB (TIMES_f)  (Z_X_Y_ARGS) ;
void GB (DIV_f)    (Z_X_Y_ARGS) ;

// binary comparison functions z=f(x,y) where x,y,z all have the same type
void GB (ISEQ_f) (Z_X_Y_ARGS) ;
void GB (ISNE_f) (Z_X_Y_ARGS) ;
void GB (ISGT_f) (Z_X_Y_ARGS) ;
void GB (ISLT_f) (Z_X_Y_ARGS) ;
void GB (ISGE_f) (Z_X_Y_ARGS) ;
void GB (ISLE_f) (Z_X_Y_ARGS) ;

// 3 logical functions; x,y,z all the same type
void GB (LOR_f)  (Z_X_Y_ARGS) ;
void GB (LAND_f) (Z_X_Y_ARGS) ;
void GB (LXOR_f) (Z_X_Y_ARGS) ;

// binary functions that return bool, z = f (x,y)
void GB (EQ_f) (Zbool_X_Y_ARGS) ;
void GB (NE_f) (Zbool_X_Y_ARGS) ;
void GB (GT_f) (Zbool_X_Y_ARGS) ;
void GB (LT_f) (Zbool_X_Y_ARGS) ;
void GB (GE_f) (Zbool_X_Y_ARGS) ;
void GB (LE_f) (Zbool_X_Y_ARGS) ;

// unary casts:  for example, GB_cast_bool_int8_t casts the input (bool) x to
// the result (int8_t) z.  The s argument is not used for built-in types, but
// these functions must have the same signature as GB_copy_user_user, which
// needs to know the size of the user-defined type.
void CAST_NAME (bool    ) (void *z, const void *x, size_t s) ;
void CAST_NAME (int8_t  ) (void *z, const void *x, size_t s) ;
void CAST_NAME (uint8_t ) (void *z, const void *x, size_t s) ;
void CAST_NAME (int16_t ) (void *z, const void *x, size_t s) ;
void CAST_NAME (uint16_t) (void *z, const void *x, size_t s) ;
void CAST_NAME (int32_t ) (void *z, const void *x, size_t s) ;
void CAST_NAME (uint32_t) (void *z, const void *x, size_t s) ;
void CAST_NAME (int64_t ) (void *z, const void *x, size_t s) ;
void CAST_NAME (uint64_t) (void *z, const void *x, size_t s) ;
void CAST_NAME (float   ) (void *z, const void *x, size_t s) ;
void CAST_NAME (double  ) (void *z, const void *x, size_t s) ;

#undef GB
#undef TYPE
#undef CAST_NAME

#undef Z_X_ARGS
#undef Z_X_Y_ARGS
#undef Zbool_X_Y_ARGS

