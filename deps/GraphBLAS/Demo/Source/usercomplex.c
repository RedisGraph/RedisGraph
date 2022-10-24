//------------------------------------------------------------------------------
// GraphBLAS/Demo/Source/usercomplex.c:  complex numbers as a user-defined type
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#ifdef MATLAB_MEX_FILE

    #include "GB_mex.h"

    #define OK(method)                                                      \
    {                                                                       \
        info = method ;                                                     \
        if (! (info == GrB_SUCCESS || info == GrB_NO_VALUE))                \
        {                                                                   \
            return (info) ;                                                 \
        }                                                                   \
    }

#else

    #include "GraphBLAS.h"
    #undef GB_PUBLIC
    #define GB_LIBRARY
    #include "graphblas_demos.h"

    #if defined __INTEL_COMPILER
    #pragma warning (disable: 58 167 144 161 177 181 186 188 589 593 869 981 \
        1418 1419 1572 1599 2259 2282 2557 2547 3280 )
    #elif defined __GNUC__
    #pragma GCC diagnostic ignored "-Wunused-parameter"
    #if !defined ( __cplusplus )
    #pragma GCC diagnostic ignored "-Wincompatible-pointer-types"
    #endif
    #endif

    #undef OK
    #define OK(method)                                                      \
    {                                                                       \
        info = method ;                                                     \
        if (! (info == GrB_SUCCESS || info == GrB_NO_VALUE))                \
        {                                                                   \
            Complex_finalize ( ) ;                                          \
            return (info) ;                                                 \
        }                                                                   \
    }

#endif

GrB_BinaryOp Complex_first = NULL, Complex_second = NULL, Complex_min = NULL,
             Complex_max   = NULL, Complex_plus   = NULL, Complex_minus = NULL,
             Complex_times = NULL, Complex_div    = NULL, Complex_rminus = NULL,
             Complex_rdiv  = NULL, Complex_pair   = NULL ;

GrB_BinaryOp Complex_iseq = NULL, Complex_isne = NULL,
             Complex_isgt = NULL, Complex_islt = NULL,
             Complex_isge = NULL, Complex_isle = NULL ;

GrB_BinaryOp Complex_or = NULL, Complex_and = NULL, Complex_xor = NULL ;

GrB_BinaryOp Complex_eq = NULL, Complex_ne = NULL,
             Complex_gt = NULL, Complex_lt = NULL,
             Complex_ge = NULL, Complex_le = NULL ;

GrB_BinaryOp Complex_complex = NULL ;

GrB_UnaryOp  Complex_identity = NULL, Complex_ainv = NULL, Complex_minv = NULL,
             Complex_not = NULL,      Complex_conj = NULL,
             Complex_one = NULL,      Complex_abs  = NULL ;

GrB_UnaryOp Complex_real = NULL, Complex_imag = NULL,
            Complex_cabs = NULL, Complex_angle = NULL ;

GrB_UnaryOp Complex_complex_real = NULL, Complex_complex_imag = NULL ;

GrB_Type Complex = NULL ;
GrB_Monoid   Complex_plus_monoid = NULL, Complex_times_monoid = NULL ;
GrB_Semiring Complex_plus_times = NULL ;

//------------------------------------------------------------------------------
// binary functions, z=f(x,y), where CxC -> Complex
//------------------------------------------------------------------------------

 void complex_first (GxB_FC64_t *z, const GxB_FC64_t *x, const GxB_FC64_t *y)
 {
    (*z) = (*x) ;
 }

#define COMPLEX_FIRST                                                         \
"void "                                                                       \
"complex_first GxB_FC64_t *z, const GxB_FC64_t *x, const GxB_FC64_t *y)   \n" \
"{                                                                        \n" \
"   (*z) = (*x) :                                                         \n" \
"}"

 void complex_second (GxB_FC64_t *z, const GxB_FC64_t *x, const GxB_FC64_t *y)
 {
    (*z) = (*y) ;
 }

#define COMPLEX_SECOND                                                        \
"void "                                                                       \
"complex_second (GxB_FC64_t *z, const GxB_FC64_t *x, const GxB_FC64_t *y) \n" \
"{                                                                        \n" \
"   (*z) = (*y) ;                                                         \n" \
"}"

 void complex_pair (GxB_FC64_t *z, const GxB_FC64_t *x, const GxB_FC64_t *y)
 {
    (*z) = GxB_CMPLX (1,0) ;
 }

#define COMPLEX_PAIR                                                          \
"void "                                                                       \
"complex_pair (GxB_FC64_t *z, const GxB_FC64_t *x, const GxB_FC64_t *y)   \n" \
"{                                                                        \n" \
"   (*z) = GxB_CMPLX (1,0) ;                                              \n" \
"}"

 void
 complex_plus (GxB_FC64_t *z, const GxB_FC64_t *x, const GxB_FC64_t *y)
 {
    (*z) = (*x) + (*y) ;
 }

#define COMPLEX_PLUS                                                          \
"void "                                                                       \
"complex_plus (GxB_FC64_t *z, const GxB_FC64_t *x, const GxB_FC64_t *y)   \n" \
"{                                                                        \n" \
"   (*z) = (*x) + (*y) ;                                                  \n" \
"}"

 void complex_minus (GxB_FC64_t *z, const GxB_FC64_t *x, const GxB_FC64_t *y)
 {
    (*z) = (*x) - (*y) ;
 }

#define COMPLEX_MINUS                                                         \
"void "                                                                       \
"complex_minus (GxB_FC64_t *z, const GxB_FC64_t *x, const GxB_FC64_t *y)  \n" \
"{                                                                        \n" \
"   (*z) = (*x) - (*y) ;                                                  \n" \
"}"

 void complex_rminus (GxB_FC64_t *z, const GxB_FC64_t *x, const GxB_FC64_t *y)
 {
    (*z) = (*y) - (*x) ;
 }

#define COMPLEX_RMINUS                                                        \
"void "                                                                       \
"complex_rminus (GxB_FC64_t *z, const GxB_FC64_t *x, const GxB_FC64_t *y) \n" \
"{                                                                        \n" \
"   (*z) = (*y) - (*x) ;                                                  \n" \
"}"

 void complex_times (GxB_FC64_t *z, const GxB_FC64_t *x, const GxB_FC64_t *y)
 {
    (*z) = (*x) * (*y) ;
 }

#define COMPLEX_TIMES                                                         \
"void "                                                                       \
"complex_times (GxB_FC64_t *z, const GxB_FC64_t *x, const GxB_FC64_t *y)  \n" \
"{                                                                        \n" \
"   (*z) = (*x) * (*y) ;                                                  \n" \
"}"

 void complex_div (GxB_FC64_t *z, const GxB_FC64_t *x, const GxB_FC64_t *y)
 {
    (*z) = (*x) / (*y) ;
 }

#define COMPLEX_DIV                                                           \
"void "                                                                       \
"complex_div (GxB_FC64_t *z, const GxB_FC64_t *x, const GxB_FC64_t *y)    \n" \
"{                                                                        \n" \
"   (*z) = (*x) / (*y) ;                                                  \n" \
"}"

 void complex_rdiv (GxB_FC64_t *z, const GxB_FC64_t *x, const GxB_FC64_t *y)
 {
    (*z) = (*y) / (*x) ;
 }

#define COMPLEX_RDIV                                                          \
"void "                                                                       \
"complex_rdiv (GxB_FC64_t *z, const GxB_FC64_t *x, const GxB_FC64_t *y)   \n" \
"{                                                                        \n" \
"   (*z) = (*y) / (*x) ;                                                  \n" \
"}"

// min (x,y): complex number with smallest magnitude.  If tied, select the
// one with the smallest phase angle.  No special cases for NaNs.

 void complex_min (GxB_FC64_t *z, const GxB_FC64_t *x, const GxB_FC64_t *y)
 {
    double absx = cabs (*x) ;
    double absy = cabs (*y) ;
    if (absx < absy)
    {
        (*z) = (*x) ;
    }
    else if (absx > absy)
    {
        (*z) = (*y) ;
    }
    else
    {
        (*z) = (carg (*x) < carg (*y)) ? (*x) : (*y) ;
    }
 }

#define COMPLEX_MIN                                                           \
"void "                                                                       \
"complex_min (GxB_FC64_t *z, const GxB_FC64_t *x, const GxB_FC64_t *y)    \n" \
"{                                                                        \n" \
"   double absx = cabs (*x) ;                                             \n" \
"   double absy = cabs (*y) ;                                             \n" \
"   if (absx < absy)                                                      \n" \
"   {                                                                     \n" \
"       (*z) = (*x) ;                                                     \n" \
"   }                                                                     \n" \
"   else if (absx > absy)                                                 \n" \
"   {                                                                     \n" \
"       (*z) = (*y) ;                                                     \n" \
"   }                                                                     \n" \
"   else                                                                  \n" \
"   {                                                                     \n" \
"       (*z) = (carg (*x) < carg (*y)) ? (*x) : (*y) ;                    \n" \
"   }                                                                     \n" \
"}"

// max (x,y): complex number with largest magnitude.  If tied, select the one
// with the largest phase angle.  No special cases for NaNs.

 void complex_max (GxB_FC64_t *z, const GxB_FC64_t *x, const GxB_FC64_t *y)
 {
    double absx = cabs (*x) ;
    double absy = cabs (*y) ;
    if (absx > absy)
    {
        (*z) = (*x) ;
    }
    else if (absx < absy)
    {
        (*z) = (*y) ;
    }
    else
    {
        (*z) = (carg (*x) > carg (*y)) ? (*x) : (*y) ;
    }
 }

#define COMPLEX_MAX                                                           \
"void "                                                                       \
"complex_max (GxB_FC64_t *z, const GxB_FC64_t *x, const GxB_FC64_t *y)    \n" \
"{                                                                        \n" \
"   double absx = cabs (*x) ;                                             \n" \
"   double absy = cabs (*y) ;                                             \n" \
"   if (absx > absy)                                                      \n" \
"   {                                                                     \n" \
"       (*z) = (*x) ;                                                     \n" \
"   }                                                                     \n" \
"   else if (absx < absy)                                                 \n" \
"   {                                                                     \n" \
"       (*z) = (*y) ;                                                     \n" \
"   }                                                                     \n" \
"   else                                                                  \n" \
"   {                                                                     \n" \
"       (*z) = (carg (*x) > carg (*y)) ? (*x) : (*y) ;                    \n" \
"   }                                                                     \n" \
"}"

//------------------------------------------------------------------------------
// 6 binary functions, z=f(x,y); CxC -> Complex ; (1,0) = true, (0,0) = false
//------------------------------------------------------------------------------

 void complex_iseq (GxB_FC64_t *z, const GxB_FC64_t *x, const GxB_FC64_t *y)
 {
    bool eq = (creal (*x) == creal (*y) && cimag (*x) == cimag (*y)) ;
    (*z) = eq ? GxB_CMPLX (1,0) : GxB_CMPLX (0,0) ;
 }

#define COMPLEX_ISEQ                                                          \
"void "                                                                       \
"complex_iseq (GxB_FC64_t *z, const GxB_FC64_t *x, const GxB_FC64_t *y)   \n" \
"{                                                                        \n" \
"   bool eq = (creal (*x) == creal (*y) && cimag (*x) == cimag (*y)) ;    \n" \
"   (*z) = eq ? GxB_CMPLX (1,0) : GxB_CMPLX (0,0) ;                       \n" \
"}"

 void complex_isne (GxB_FC64_t *z, const GxB_FC64_t *x, const GxB_FC64_t *y)
 {
    bool ne = (creal (*x) != creal (*y) || cimag (*x) != cimag (*y)) ;
    (*z) = ne ? GxB_CMPLX (1,0) : GxB_CMPLX (0,0) ;
 }

#define COMPLEX_ISNE                                                          \
"void "                                                                       \
"complex_isne (GxB_FC64_t *z, const GxB_FC64_t *x, const GxB_FC64_t *y)   \n" \
"{                                                                        \n" \
"   bool ne = (creal (*x) != creal (*y) || cimag (*x) != cimag (*y)) ;    \n" \
"   (*z) = ne ? GxB_CMPLX (1,0) : GxB_CMPLX (0,0) ;                       \n" \
"}"

 void complex_isgt (GxB_FC64_t *z, const GxB_FC64_t *x, const GxB_FC64_t *y)
 {
    bool gt = (creal (*x) > creal (*y)) ;
    (*z) = gt ? GxB_CMPLX (1,0) : GxB_CMPLX (0,0) ;
 }

#define COMPLEX_ISGT                                                          \
"void "                                                                       \
"complex_isgt (GxB_FC64_t *z, const GxB_FC64_t *x, const GxB_FC64_t *y)   \n" \
"{                                                                        \n" \
"   bool gt = (creal (*x) > creal (*y)) ;                                 \n" \
"   (*z) = gt ? GxB_CMPLX (1,0) : GxB_CMPLX (0,0) ;                       \n" \
"}"

 void complex_islt (GxB_FC64_t *z, const GxB_FC64_t *x, const GxB_FC64_t *y)
 {
    bool lt = (creal (*x) < creal (*y)) ;
    (*z) = lt ? GxB_CMPLX (1,0) : GxB_CMPLX (0,0) ;
 }

#define COMPLEX_ISLT                                                          \
"void "                                                                       \
"complex_islt (GxB_FC64_t *z, const GxB_FC64_t *x, const GxB_FC64_t *y)   \n" \
"{                                                                        \n" \
"   bool lt = (creal (*x) < creal (*y)) ;                                 \n" \
"   (*z) = lt ? GxB_CMPLX (1,0) : GxB_CMPLX (0,0) ;                       \n" \
"}"

 void complex_isge (GxB_FC64_t *z, const GxB_FC64_t *x, const GxB_FC64_t *y)
 {
    bool ge = (creal (*x) >= creal (*y)) ;
    (*z) = ge ? GxB_CMPLX (1,0) : GxB_CMPLX (0,0) ;
 }

#define COMPLEX_ISGE                                                          \
"void "                                                                       \
"complex_isge (GxB_FC64_t *z, const GxB_FC64_t *x, const GxB_FC64_t *y)   \n" \
"{                                                                        \n" \
"   bool ge = (creal (*x) >= creal (*y)) ;                                \n" \
"   (*z) = ge ? GxB_CMPLX (1,0) : GxB_CMPLX (0,0) ;                       \n" \
"}"

 void complex_isle (GxB_FC64_t *z, const GxB_FC64_t *x, const GxB_FC64_t *y)
 {
    bool le = (creal (*x) <= creal (*y)) ;
    (*z) = le ? GxB_CMPLX (1,0) : GxB_CMPLX (0,0) ;
 }

#define COMPLEX_ISLE                                                          \
"void "                                                                       \
"complex_isle (GxB_FC64_t *z, const GxB_FC64_t *x, const GxB_FC64_t *y)   \n" \
"{                                                                        \n" \
"   bool le = (creal (*x) <= creal (*y)) ;                                \n" \
"   (*z) = le ? GxB_CMPLX (1,0) : GxB_CMPLX (0,0) ;                       \n" \
"}"

//------------------------------------------------------------------------------
// binary boolean functions, z=f(x,y), where CxC -> Complex
//------------------------------------------------------------------------------

 void complex_or (GxB_FC64_t *z, const GxB_FC64_t *x, const GxB_FC64_t *y)
 {
    bool xbool = (creal (*x) != 0 || cimag (*x) != 0) ;
    bool ybool = (creal (*y) != 0 || cimag (*y) != 0) ;
    (*z) = (xbool || ybool) ? GxB_CMPLX (1,0) : GxB_CMPLX (0,0) ;
 }

#define COMPLEX_OR                                                            \
"void "                                                                       \
"complex_or (GxB_FC64_t *z, const GxB_FC64_t *x, const GxB_FC64_t *y)     \n" \
"{                                                                        \n" \
"   bool xbool = (creal (*x) != 0 || cimag (*x) != 0) ;                   \n" \
"   bool ybool = (creal (*y) != 0 || cimag (*y) != 0) ;                   \n" \
"   (*z) = (xbool || ybool) ? GxB_CMPLX (1,0) : GxB_CMPLX (0,0) ;         \n" \
"}"

 void complex_and (GxB_FC64_t *z, const GxB_FC64_t *x, const GxB_FC64_t *y)
 {
    bool xbool = (creal (*x) != 0 || cimag (*x) != 0) ;
    bool ybool = (creal (*y) != 0 || cimag (*y) != 0) ;
    (*z) = (xbool && ybool) ? GxB_CMPLX (1,0) : GxB_CMPLX (0,0) ;
 }

#define COMPLEX_AND                                                           \
"void "                                                                       \
"complex_and (GxB_FC64_t *z, const GxB_FC64_t *x, const GxB_FC64_t *y)    \n" \
"{                                                                        \n" \
"   bool xbool = (creal (*x) != 0 || cimag (*x) != 0) ;                   \n" \
"   bool ybool = (creal (*y) != 0 || cimag (*y) != 0) ;                   \n" \
"   (*z) = (xbool && ybool) ? GxB_CMPLX (1,0) : GxB_CMPLX (0,0) ;         \n" \
"}"

 void complex_xor (GxB_FC64_t *z, const GxB_FC64_t *x, const GxB_FC64_t *y)
 {
    bool xbool = (creal (*x) != 0 || cimag (*x) != 0) ;
    bool ybool = (creal (*y) != 0 || cimag (*y) != 0) ;
    (*z) = (xbool != ybool) ? GxB_CMPLX (1,0) : GxB_CMPLX (0,0) ;
 }

#define COMPLEX_XOR                                                           \
"void "                                                                       \
"complex_xor (GxB_FC64_t *z, const GxB_FC64_t *x, const GxB_FC64_t *y)    \n" \
"{                                                                        \n" \
"   bool xbool = (creal (*x) != 0 || cimag (*x) != 0) ;                   \n" \
"   bool ybool = (creal (*y) != 0 || cimag (*y) != 0) ;                   \n" \
"   (*z) = (xbool != ybool) ? GxB_CMPLX (1,0) : GxB_CMPLX (0,0) ;         \n" \
"}"

//------------------------------------------------------------------------------
// 6 binary functions, z=f(x,y), where CxC -> bool
//------------------------------------------------------------------------------

 void complex_eq (bool *z, const GxB_FC64_t *x, const GxB_FC64_t *y)
 {
    (*z) = (creal (*x) == creal (*y) && cimag (*x) == cimag (*y)) ;
 }

#define COMPLEX_EQ                                                            \
"void complex_eq (bool *z, const GxB_FC64_t *x, const GxB_FC64_t *y)      \n" \
"{                                                                        \n" \
"   (*z) = (creal (*x) == creal (*y) && cimag (*x) == cimag (*y)) ;       \n" \
"}"

 void complex_ne (bool *z, const GxB_FC64_t *x, const GxB_FC64_t *y)
 {
    (*z) = (creal (*x) != creal (*y) || cimag (*x) != cimag (*y)) ;
 }

#define COMPLEX_NE                                                            \
"void complex_ne (bool *z, const GxB_FC64_t *x, const GxB_FC64_t *y)      \n" \
"{                                                                        \n" \
"   (*z) = (creal (*x) != creal (*y) || cimag (*x) != cimag (*y)) ;       \n" \
"}"

 void complex_gt (bool *z, const GxB_FC64_t *x, const GxB_FC64_t *y)
 {
    (*z) = (creal (*x) > creal (*y)) ;
 }

#define COMPLEX_GT                                                            \
"void complex_gt (bool *z, const GxB_FC64_t *x, const GxB_FC64_t *y)      \n" \
"{                                                                        \n" \
"   (*z) = (creal (*x) > creal (*y)) ;                                    \n" \
"}"

 void complex_lt (bool *z, const GxB_FC64_t *x, const GxB_FC64_t *y)
 {
    (*z) = (creal (*x) < creal (*y)) ;
 }

#define COMPLEX_LT                                                            \
"void complex_lt (bool *z, const GxB_FC64_t *x, const GxB_FC64_t *y)      \n" \
"{                                                                        \n" \
"   (*z) = (creal (*x) < creal (*y)) ;                                    \n" \
"}"

 void complex_ge (bool *z, const GxB_FC64_t *x, const GxB_FC64_t *y)
 {
    (*z) = (creal (*x) >= creal (*y)) ;
 }

#define COMPLEX_GE                                                            \
"void complex_ge (bool *z, const GxB_FC64_t *x, const GxB_FC64_t *y)      \n" \
"{                                                                        \n" \
"   (*z) = (creal (*x) >= creal (*y)) ;                                   \n" \
"}"

 void complex_le (bool *z, const GxB_FC64_t *x, const GxB_FC64_t *y)
 {
    (*z) = (creal (*x) <= creal (*y)) ;
 }

#define COMPLEX_LE                                                            \
"void complex_le (bool *z, const GxB_FC64_t *x, const GxB_FC64_t *y)      \n" \
"{                                                                        \n" \
"   (*z) = (creal (*x) <= creal (*y)) ;                                   \n" \
"}"

//------------------------------------------------------------------------------
// binary functions, z=f(x,y), where double x double -> complex
//------------------------------------------------------------------------------

 void complex_complex (GxB_FC64_t *z, const double *x, const double *y)
 {
    (*z) = GxB_CMPLX (*x,*y) ;
 }

#define COMPLEX_COMPLEX                                                       \
"void complex_complex (GxB_FC64_t *z, const double *x, const double *y)   \n" \
"{                                                                        \n" \
"   (*z) = GxB_CMPLX (*x,*y) ;                                            \n" \
"}"

//------------------------------------------------------------------------------
// unary functions, z=f(x) where Complex -> Complex
//------------------------------------------------------------------------------

 void complex_one (GxB_FC64_t *z, const GxB_FC64_t *x)
 {
    (*z) = GxB_CMPLX (1,0) ;
 }

#define COMPLEX_ONE                                                           \
"void complex_one (GxB_FC64_t *z, const GxB_FC64_t *x)                    \n" \
"{                                                                        \n" \
"   (*z) = GxB_CMPLX (1,0) ;                                              \n" \
"}"

 void complex_identity (GxB_FC64_t *z, const GxB_FC64_t *x)
 {
    (*z) = (*x)  ;
 }

#define COMPLEX_IDENTITY                                                      \
"void complex_identity (GxB_FC64_t *z, const GxB_FC64_t *x)               \n" \
"{                                                                        \n" \
"   (*z) = (*x)  ;                                                        \n" \
"}"

 void complex_ainv (GxB_FC64_t *z, const GxB_FC64_t *x)
 {
    (*z) = -(*x)  ;
 }

#define COMPLEX_AINV                                                          \
"void complex_ainv (GxB_FC64_t *z, const GxB_FC64_t *x)                   \n" \
"{                                                                        \n" \
"   (*z) = -(*x)  ;                                                       \n" \
"}"

 void complex_minv (GxB_FC64_t *z, const GxB_FC64_t *x)
 {
    (*z) =  1. / (*x) ;
 }

#define COMPLEX_MINV                                                          \
"void complex_minv (GxB_FC64_t *z, const GxB_FC64_t *x)                   \n" \
"{                                                                        \n" \
"   (*z) =  1. / (*x) ;                                                   \n" \
"}"

 void complex_conj (GxB_FC64_t *z, const GxB_FC64_t *x)
 {
    (*z) = conj (*x) ;
 }

#define COMPLEX_CONJ                                                          \
"void complex_conj (GxB_FC64_t *z, const GxB_FC64_t *x)                   \n" \
"{                                                                        \n" \
"   (*z) = conj (*x) ;                                                    \n" \
"}"

 void complex_abs (GxB_FC64_t *z, const GxB_FC64_t *x)
 {
    (*z) = GxB_CMPLX (cabs (*x), 0) ;
 }

#define COMPLEX_ABS                                                           \
"void complex_abs (GxB_FC64_t *z, const GxB_FC64_t *x)                    \n" \
"{                                                                        \n" \
"   (*z) = GxB_CMPLX (cabs (*x), 0) ;                                     \n" \
"}"

 void complex_not (GxB_FC64_t *z, const GxB_FC64_t *x)
 {
    bool xbool = (creal (*x) != 0 || cimag (*x) != 0) ;
    (*z) = xbool ? GxB_CMPLX (0,0) : GxB_CMPLX (1,0) ;
 }

#define COMPLEX_NOT                                                           \
"void complex_not (GxB_FC64_t *z, const GxB_FC64_t *x)                    \n" \
"{                                                                        \n" \
"   bool xbool = (creal (*x) != 0 || cimag (*x) != 0) ;                   \n" \
"   (*z) = xbool ? GxB_CMPLX (0,0) : GxB_CMPLX (1,0) ;                    \n" \
"}"

//------------------------------------------------------------------------------
// unary functions, z=f(x) where Complex -> double
//------------------------------------------------------------------------------

 void complex_real (double *z, const GxB_FC64_t *x)
 {
    (*z) = creal (*x) ;
 }

#define COMPLEX_REAL                                                          \
"void complex_real (double *z, const GxB_FC64_t *x)                       \n" \
"{                                                                        \n" \
"   (*z) = creal (*x) ;                                                   \n" \
"}"

 void complex_imag (double *z, const GxB_FC64_t *x)
 {
    (*z) = cimag (*x) ;
 }

#define COMPLEX_IMAG                                                          \
"void complex_imag (double *z, const GxB_FC64_t *x)                       \n" \
"{                                                                        \n" \
"   (*z) = cimag (*x) ;                                                   \n" \
"}"

 void complex_cabs (double *z, const GxB_FC64_t *x)
 {
    (*z) = cabs (*x) ;
 }

#define COMPLEX_CABS                                                          \
"void complex_cabs (double *z, const GxB_FC64_t *x)                       \n" \
"{                                                                        \n" \
"   (*z) = cabs (*x) ;                                                    \n" \
"}"

 void complex_angle (double *z, const GxB_FC64_t *x)
 {
    (*z) = carg (*x) ;
 }

#define COMPLEX_ANGLE                                                         \
"void complex_angle (double *z, const GxB_FC64_t *x)                      \n" \
"{                                                                        \n" \
"   (*z) = carg (*x) ;                                                    \n" \
"}"

//------------------------------------------------------------------------------
// unary functions, z=f(x) where double -> Complex
//------------------------------------------------------------------------------

 void complex_complex_real (GxB_FC64_t *z, const double *x)
 {
    (*z) = GxB_CMPLX (*x, 0) ;
 }

#define COMPLEX_COMPLEX_REAL                                                  \
"void complex_complex_real (GxB_FC64_t *z, const double *x)               \n" \
"{                                                                        \n" \
"   (*z) = GxB_CMPLX (*x, 0) ;                                            \n" \
"}"

 void complex_complex_imag (GxB_FC64_t *z, const double *x)
 {
    (*z) = GxB_CMPLX (0, *x) ;
 }

#define COMPLEX_COMPLEX_IMAG                                                  \
"void complex_complex_imag (GxB_FC64_t *z, const double *x)               \n" \
"{                                                                        \n" \
"   (*z) = GxB_CMPLX (0, *x) ;                                            \n" \
"}"

//------------------------------------------------------------------------------
// Complex_init: create the complex type, operators, monoids, and semiring
//------------------------------------------------------------------------------

#define U (GxB_unary_function)
#define B (GxB_binary_function)

GrB_Info Complex_init (bool builtin_complex)
{

    GrB_Info info ;

    //--------------------------------------------------------------------------
    // create the Complex type, or set to GxB_FC64
    //--------------------------------------------------------------------------

    if (builtin_complex)
    {
        // use the built-in type
        Complex = GxB_FC64 ;
    }
    else
    {
        // create the user-defined type
        // Normally, the typename should be "GxB_FC64_t",
        // but the C type GxB_FC64_t is already defined.
        OK (GxB_Type_new (&Complex, sizeof (GxB_FC64_t), "mycomplex",
            "typedef struct { double xreal ; double ximag ; } mycomplex ;" )) ;
    }

    //--------------------------------------------------------------------------
    // create the Complex binary operators, CxC->Complex
    //--------------------------------------------------------------------------

    if (builtin_complex)
    {
        // use the built-in versions
        Complex_first  = GxB_FIRST_FC64 ;
        Complex_second = GxB_SECOND_FC64 ;
        Complex_pair   = GxB_PAIR_FC64 ;
        Complex_plus   = GxB_PLUS_FC64 ;
        Complex_minus  = GxB_MINUS_FC64 ;
        Complex_rminus = GxB_RMINUS_FC64 ;
        Complex_times  = GxB_TIMES_FC64 ;
        Complex_div    = GxB_DIV_FC64 ;
        Complex_rdiv   = GxB_RDIV_FC64 ;
    }
    else
    {
        // create user-defined versions
        OK (GxB_BinaryOp_new (&Complex_first  , B complex_first  ,
            Complex, Complex, Complex, "complex_first", COMPLEX_FIRST)) ;

        // FUTURE C API suggestion:
//      OK (GrB_BinaryOp_new (&Complex_first  , B complex_first  ,
//          Complex, Complex, Complex)) ;
//      GrB_set (Complex_first, GRB_NAME, "complex_first") ;
//      GrB_set (Complex_first, GRB_DEFN, COMPLEX_FIRST) ;

        OK (GxB_BinaryOp_new (&Complex_second , B complex_second ,
            Complex, Complex, Complex, "complex_second", COMPLEX_SECOND)) ;
        OK (GxB_BinaryOp_new (&Complex_pair   , B complex_pair   ,
            Complex, Complex, Complex, "complex_pair", COMPLEX_PAIR)) ;
        OK (GxB_BinaryOp_new (&Complex_plus   , B complex_plus   ,
            Complex, Complex, Complex, "complex_plus", COMPLEX_PLUS)) ;
        OK (GxB_BinaryOp_new (&Complex_minus  , B complex_minus  ,
            Complex, Complex, Complex, "complex_minus", COMPLEX_MINUS)) ;
        OK (GxB_BinaryOp_new (&Complex_rminus , B complex_rminus ,
            Complex, Complex, Complex, "complex_rminus", COMPLEX_RMINUS)) ;
        OK (GxB_BinaryOp_new (&Complex_times  , B complex_times  ,
            Complex, Complex, Complex, "complex_times", COMPLEX_TIMES)) ;
        OK (GxB_BinaryOp_new (&Complex_div    , B complex_div    ,
            Complex, Complex, Complex, "complex_div", COMPLEX_DIV)) ;
        OK (GxB_BinaryOp_new (&Complex_rdiv   , B complex_rdiv   ,
            Complex, Complex, Complex, "complex_rdiv", COMPLEX_RDIV)) ;
    }

    // these are not built-in
    OK (GxB_BinaryOp_new (&Complex_min    , B complex_min    ,
        Complex, Complex, Complex, "complex_min", COMPLEX_MIN)) ;
    OK (GxB_BinaryOp_new (&Complex_max    , B complex_max    ,
        Complex, Complex, Complex, "complex_max", COMPLEX_MAX)) ;

    //--------------------------------------------------------------------------
    // create the Complex binary comparators, CxC -> Complex
    //--------------------------------------------------------------------------

    if (builtin_complex)
    {
        // use the built-in versions
        Complex_iseq = GxB_ISEQ_FC64 ;
        Complex_isne = GxB_ISNE_FC64 ;
    }
    else
    {
        // create user-defined versions
        OK (GxB_BinaryOp_new (&Complex_iseq , B complex_iseq ,
            Complex, Complex, Complex, "complex_iseq", COMPLEX_ISEQ)) ;
        OK (GxB_BinaryOp_new (&Complex_isne , B complex_isne ,
            Complex, Complex, Complex, "complex_isne", COMPLEX_ISNE)) ;
    }

    // these are not built-in
    OK (GxB_BinaryOp_new (&Complex_isgt , B complex_isgt ,
        Complex, Complex, Complex, "complex_isgt", COMPLEX_ISGT)) ;
    OK (GxB_BinaryOp_new (&Complex_islt , B complex_islt ,
        Complex, Complex, Complex, "complex_islt", COMPLEX_ISLT)) ;
    OK (GxB_BinaryOp_new (&Complex_isge , B complex_isge ,
        Complex, Complex, Complex, "complex_isge", COMPLEX_ISGE)) ;
    OK (GxB_BinaryOp_new (&Complex_isle , B complex_isle ,
        Complex, Complex, Complex, "complex_isle", COMPLEX_ISLE)) ;

    //--------------------------------------------------------------------------
    // create the Complex boolean operators, CxC -> Complex
    //--------------------------------------------------------------------------

    // these are not built-in
    OK (GxB_BinaryOp_new (&Complex_or  , B complex_or  ,
        Complex, Complex, Complex, "complex_or", COMPLEX_OR)) ;
    OK (GxB_BinaryOp_new (&Complex_and , B complex_and ,
        Complex, Complex, Complex, "complex_and", COMPLEX_AND)) ;
    OK (GxB_BinaryOp_new (&Complex_xor , B complex_xor ,
        Complex, Complex, Complex, "complex_xor", COMPLEX_XOR)) ;

    //--------------------------------------------------------------------------
    // create the Complex binary operators, CxC -> bool
    //--------------------------------------------------------------------------

    if (builtin_complex)
    {
        // use the built-in versions
        Complex_eq = GxB_EQ_FC64 ;
        Complex_ne = GxB_NE_FC64 ;
    }
    else
    {
        // create user-defined versions
        OK (GxB_BinaryOp_new (&Complex_eq , B complex_eq ,
            GrB_BOOL, Complex, Complex, "complex_eq", COMPLEX_EQ)) ;
        OK (GxB_BinaryOp_new (&Complex_ne , B complex_ne ,
            GrB_BOOL, Complex, Complex, "complex_ne", COMPLEX_NE)) ;
    }

    // these are not built-in
    OK (GxB_BinaryOp_new (&Complex_gt , B complex_gt ,
        GrB_BOOL, Complex, Complex, "complex_gt", COMPLEX_GT)) ;
    OK (GxB_BinaryOp_new (&Complex_lt , B complex_lt ,
        GrB_BOOL, Complex, Complex, "complex_lt", COMPLEX_LT)) ;
    OK (GxB_BinaryOp_new (&Complex_ge , B complex_ge ,
        GrB_BOOL, Complex, Complex, "complex_ge", COMPLEX_GE)) ;
    OK (GxB_BinaryOp_new (&Complex_le , B complex_le ,
        GrB_BOOL, Complex, Complex, "complex_le", COMPLEX_LE)) ;

    //--------------------------------------------------------------------------
    // create the Complex binary operator, double x double -> Complex
    //--------------------------------------------------------------------------

    if (builtin_complex)
    {
        // use the built-in versions
        Complex_complex = GxB_CMPLX_FP64 ;
    }
    else
    {
        // create user-defined versions
        OK (GxB_BinaryOp_new (&Complex_complex, B complex_complex,
            Complex, GrB_FP64, GrB_FP64, "complex_complex", COMPLEX_COMPLEX)) ;
    }

    //--------------------------------------------------------------------------
    // create the Complex unary operators, Complex->Complex
    //--------------------------------------------------------------------------

    if (builtin_complex)
    {
        // use the built-in versions
        Complex_one      = GxB_ONE_FC64 ;
        Complex_identity = GxB_IDENTITY_FC64 ;
        Complex_ainv     = GxB_AINV_FC64 ;
        Complex_minv     = GxB_MINV_FC64 ;
        Complex_conj     = GxB_CONJ_FC64 ;
    }
    else
    {
        // create user-defined versions
        OK (GxB_UnaryOp_new (&Complex_one     , U complex_one     ,
            Complex, Complex, "complex_one", COMPLEX_ONE)) ;
        OK (GxB_UnaryOp_new (&Complex_identity, U complex_identity,
            Complex, Complex, "complex_identity", COMPLEX_IDENTITY)) ;
        OK (GxB_UnaryOp_new (&Complex_ainv    , U complex_ainv    ,
            Complex, Complex, "complex_ainv", COMPLEX_AINV)) ;
        OK (GxB_UnaryOp_new (&Complex_minv    , U complex_minv    ,
            Complex, Complex, "complex_minv", COMPLEX_MINV)) ;
        OK (GxB_UnaryOp_new (&Complex_conj    , U complex_conj    ,
            Complex, Complex, "complex_conj", COMPLEX_CONJ)) ;
    }

    // these are not built-in
    OK (GxB_UnaryOp_new (&Complex_abs     , U complex_abs     ,
        Complex, Complex, "complex_abs", COMPLEX_ABS)) ;
    OK (GxB_UnaryOp_new (&Complex_not     , U complex_not     ,
        Complex, Complex, "complex_not", COMPLEX_NOT)) ;

    //--------------------------------------------------------------------------
    // create the unary functions, Complex -> double
    //--------------------------------------------------------------------------

    if (builtin_complex)
    {
        // use the built-in versions
        Complex_real  = GxB_CREAL_FC64 ;
        Complex_imag  = GxB_CIMAG_FC64 ;
        Complex_cabs  = GxB_ABS_FC64 ;
        Complex_angle = GxB_CARG_FC64 ;
    }
    else
    {
        // create user-defined versions
        OK (GxB_UnaryOp_new (&Complex_real  , U complex_real  ,
            GrB_FP64, Complex, "complex_real", COMPLEX_REAL)) ;
        OK (GxB_UnaryOp_new (&Complex_imag  , U complex_imag  ,
            GrB_FP64, Complex, "complex_imag", COMPLEX_IMAG)) ;
        OK (GxB_UnaryOp_new (&Complex_cabs  , U complex_cabs  ,
            GrB_FP64, Complex, "complex_cabs", COMPLEX_CABS)) ;
        OK (GxB_UnaryOp_new (&Complex_angle , U complex_angle ,
            GrB_FP64, Complex, "complex_angle", COMPLEX_ANGLE)) ;
    }

    //--------------------------------------------------------------------------
    // create the unary functions, double -> Complex
    //--------------------------------------------------------------------------

    // these are not built-in
    OK (GxB_UnaryOp_new (&Complex_complex_real, U complex_complex_real,
        Complex, GrB_FP64, "complex_complex_real", COMPLEX_COMPLEX_REAL)) ;
    OK (GxB_UnaryOp_new (&Complex_complex_imag, U complex_complex_imag,
        Complex, GrB_FP64, "complex_complex_imag", COMPLEX_COMPLEX_IMAG)) ;

    //--------------------------------------------------------------------------
    // create the Complex monoids
    //--------------------------------------------------------------------------

    if (builtin_complex)
    {
        // use the built-in versions
        Complex_plus_monoid  = GxB_PLUS_FC64_MONOID ;
        Complex_times_monoid = GxB_TIMES_FC64_MONOID ;
    }
    else
    {
        // create user-defined versions
        GxB_FC64_t C_1 = GxB_CMPLX (1,0) ;
        GxB_FC64_t C_0 = GxB_CMPLX (0,0) ;
        OK (GrB_Monoid_new_UDT (&Complex_plus_monoid,  Complex_plus,  &C_0)) ;
        OK (GrB_Monoid_new_UDT (&Complex_times_monoid, Complex_times, &C_1)) ;
    }

    //----------------------------------------------------------------------
    // create the Complex plus-times semiring
    //----------------------------------------------------------------------

    if (builtin_complex)
    {
        // use the built-in versions
        Complex_plus_times = GxB_PLUS_TIMES_FC64 ;
    }
    else
    {
        // more could be created, but this suffices for testing GraphBLAS
        OK (GrB_Semiring_new (&Complex_plus_times, Complex_plus_monoid,
            Complex_times)) ;
    }

    return (GrB_SUCCESS) ;
}


//------------------------------------------------------------------------------
// Complex_finalize: free all complex types, operators, monoids, and semiring
//------------------------------------------------------------------------------

// These may be built-in types and operators.  They are safe to free; the
// GrB_*_free functions silently do nothing if asked to free bulit-in objects.

GrB_Info Complex_finalize ( )
{

    //--------------------------------------------------------------------------
    // free the Complex plus-times semiring
    //--------------------------------------------------------------------------

    GrB_Semiring_free (&Complex_plus_times) ;

    //--------------------------------------------------------------------------
    // free the Complex monoids
    //--------------------------------------------------------------------------

    GrB_Monoid_free (&Complex_plus_monoid ) ;
    GrB_Monoid_free (&Complex_times_monoid) ;

    //--------------------------------------------------------------------------
    // free the Complex binary operators, CxC->complex
    //--------------------------------------------------------------------------

    GrB_BinaryOp_free (&Complex_first ) ;
    GrB_BinaryOp_free (&Complex_second) ;
    GrB_BinaryOp_free (&Complex_pair  ) ;
    GrB_BinaryOp_free (&Complex_min   ) ;
    GrB_BinaryOp_free (&Complex_max   ) ;
    GrB_BinaryOp_free (&Complex_plus  ) ;
    GrB_BinaryOp_free (&Complex_minus ) ;
    GrB_BinaryOp_free (&Complex_rminus) ;
    GrB_BinaryOp_free (&Complex_times ) ;
    GrB_BinaryOp_free (&Complex_div   ) ;
    GrB_BinaryOp_free (&Complex_rdiv  ) ;

    GrB_BinaryOp_free (&Complex_iseq) ;
    GrB_BinaryOp_free (&Complex_isne) ;
    GrB_BinaryOp_free (&Complex_isgt) ;
    GrB_BinaryOp_free (&Complex_islt) ;
    GrB_BinaryOp_free (&Complex_isge) ;
    GrB_BinaryOp_free (&Complex_isle) ;

    GrB_BinaryOp_free (&Complex_or) ;
    GrB_BinaryOp_free (&Complex_and) ;
    GrB_BinaryOp_free (&Complex_xor) ;

    //--------------------------------------------------------------------------
    // free the Complex binary operators, CxC -> bool
    //--------------------------------------------------------------------------

    GrB_BinaryOp_free (&Complex_eq) ;
    GrB_BinaryOp_free (&Complex_ne) ;
    GrB_BinaryOp_free (&Complex_gt) ;
    GrB_BinaryOp_free (&Complex_lt) ;
    GrB_BinaryOp_free (&Complex_ge) ;
    GrB_BinaryOp_free (&Complex_le) ;

    //--------------------------------------------------------------------------
    // free the Complex binary operator, double x double -> complex
    //--------------------------------------------------------------------------

    GrB_BinaryOp_free (&Complex_complex) ;

    //--------------------------------------------------------------------------
    // free the Complex unary operators, complex->complex
    //--------------------------------------------------------------------------

    GrB_UnaryOp_free (&Complex_one     ) ;
    GrB_UnaryOp_free (&Complex_identity) ;
    GrB_UnaryOp_free (&Complex_ainv    ) ;
    GrB_UnaryOp_free (&Complex_abs     ) ;
    GrB_UnaryOp_free (&Complex_minv    ) ;
    GrB_UnaryOp_free (&Complex_not     ) ;
    GrB_UnaryOp_free (&Complex_conj    ) ;

    //--------------------------------------------------------------------------
    // free the unary functions, complex -> double
    //--------------------------------------------------------------------------

    GrB_UnaryOp_free (&Complex_real ) ;
    GrB_UnaryOp_free (&Complex_imag ) ;
    GrB_UnaryOp_free (&Complex_cabs ) ;
    GrB_UnaryOp_free (&Complex_angle) ;

    //--------------------------------------------------------------------------
    // free the unary functions, double -> complex
    //--------------------------------------------------------------------------

    GrB_UnaryOp_free (&Complex_complex_real) ;
    GrB_UnaryOp_free (&Complex_complex_imag) ;

    //--------------------------------------------------------------------------
    // free the Complex type
    //--------------------------------------------------------------------------

    GrB_Type_free (&Complex) ;

    return (GrB_SUCCESS) ;
}

