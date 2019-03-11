//------------------------------------------------------------------------------
// GraphBLAS/Demo/Include/usercomplex.h:  complex numbers as a user-defined type
//------------------------------------------------------------------------------

#ifndef USERCOMPLEX_H
#define USERCOMPLEX_H

#include "GraphBLAS.h"
#include <complex.h>

#ifndef CMPLX
#define CMPLX(real,imag) \
    ( \
    (double complex)((double)(real)) + \
    (double complex)((double)(imag) * _Complex_I) \
    )
#endif

// "I" is used in GraphBLAS to denote a list of row indices; remove it here
#undef I

//------------------------------------------------------------------------------
// 8 binary functions, z=f(x,y), where CxC -> C
//------------------------------------------------------------------------------

extern
GrB_BinaryOp Complex_first , Complex_second , Complex_min ,
             Complex_max   , Complex_plus   , Complex_minus ,
             Complex_times , Complex_div    ;

//------------------------------------------------------------------------------
// 6 binary comparison functions, z=f(x,y), where CxC -> C
//------------------------------------------------------------------------------

extern
GrB_BinaryOp Complex_iseq , Complex_isne ,
             Complex_isgt , Complex_islt ,
             Complex_isge , Complex_isle ;

//------------------------------------------------------------------------------
// 3 binary boolean functions, z=f(x,y), where CxC -> C
//------------------------------------------------------------------------------

extern
GrB_BinaryOp Complex_or , Complex_and , Complex_xor ;

//------------------------------------------------------------------------------
// 6 binary comparison functions, z=f(x,y), where CxC -> bool
//------------------------------------------------------------------------------

extern
GrB_BinaryOp Complex_eq , Complex_ne ,
             Complex_gt , Complex_lt ,
             Complex_ge , Complex_le ;

//------------------------------------------------------------------------------
// 1 binary function, z=f(x,y), where double x double -> C
//------------------------------------------------------------------------------

extern GrB_BinaryOp Complex_complex ;

//------------------------------------------------------------------------------
// 5 unary functions, z=f(x) where C -> C
//------------------------------------------------------------------------------

extern
GrB_UnaryOp  Complex_identity , Complex_ainv , Complex_minv ,
             Complex_not ,      Complex_conj,
             Complex_one ,      Complex_abs  ;

//------------------------------------------------------------------------------
// 4 unary functions, z=f(x) where C -> double
//------------------------------------------------------------------------------

extern 
GrB_UnaryOp Complex_real, Complex_imag,
            Complex_cabs, Complex_angle ;

//------------------------------------------------------------------------------
// 2 unary functions, z=f(x) where double -> C
//------------------------------------------------------------------------------

extern GrB_UnaryOp Complex_complex_real, Complex_complex_imag ;

//------------------------------------------------------------------------------
// Complex type, scalars, monoids, and semiring
//------------------------------------------------------------------------------

#ifdef MY_COMPLEX
// use the pre-defined type in User/my_complex.m4
#define Complex My_Complex
#else
extern GrB_Type Complex ;
#endif

extern GrB_Monoid   Complex_plus_monoid, Complex_times_monoid ;
extern GrB_Semiring Complex_plus_times ;
extern double complex Complex_1  ;
extern double complex Complex_0 ;
GrB_Info Complex_init ( ) ;
GrB_Info Complex_finalize ( ) ;

#endif
