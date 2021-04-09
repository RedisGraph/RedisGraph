//------------------------------------------------------------------------------
// LAGraph_complex:  complex number support for LAGraph
//------------------------------------------------------------------------------

/*
    LAGraph:  graph algorithms based on GraphBLAS

    Copyright 2019 LAGraph Contributors.

    (see Contributors.txt for a full list of Contributors; see
    ContributionInstructions.txt for information on how you can Contribute to
    this project).

    All Rights Reserved.

    NO WARRANTY. THIS MATERIAL IS FURNISHED ON AN "AS-IS" BASIS. THE LAGRAPH
    CONTRIBUTORS MAKE NO WARRANTIES OF ANY KIND, EITHER EXPRESSED OR IMPLIED,
    AS TO ANY MATTER INCLUDING, BUT NOT LIMITED TO, WARRANTY OF FITNESS FOR
    PURPOSE OR MERCHANTABILITY, EXCLUSIVITY, OR RESULTS OBTAINED FROM USE OF
    THE MATERIAL. THE CONTRIBUTORS DO NOT MAKE ANY WARRANTY OF ANY KIND WITH
    RESPECT TO FREEDOM FROM PATENT, TRADEMARK, OR COPYRIGHT INFRINGEMENT.

    Released under a BSD license, please see the LICENSE file distributed with
    this Software or contact permission@sei.cmu.edu for full terms.

    Created, in part, with funding and support from the United States
    Government.  (see Acknowledgments.txt file).

    This program includes and/or can make use of certain third party source
    code, object code, documentation and other files ("Third Party Software").
    See LICENSE file for more details.

*/

//------------------------------------------------------------------------------

// Contributed by Michel Pelletier. Adapted from 'usercomplex.c' code
// in SuiteSparse Demo by Dr. Tim Davis.

#include "LAGraph_internal.h"

// a global value for returning the complex type in a Matrix Market file:
GrB_Type LAGraph_ComplexFP64 = NULL ;

#if defined __INTEL_COMPILER
#pragma warning (disable: 58 167 144 161 177 181 186 188 589 593 869 981 1418 1419 1572 1599 2259 2282 2557 2547 3280 )
#elif defined __GNUC__
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wincompatible-pointer-types"
#endif

#define C double complex
#define X *x
#define Y *y
#define Z *z

#define ONE  (CMPLX(1.0,0.0))
#define ZERO (CMPLX(0.0,0.0))
#define T ONE
#define F ZERO
#define BOOL(X) (X != ZERO)

//------------------------------------------------------------------------------
// 8 binary functions, z=f(x,y), where CxC -> C
//------------------------------------------------------------------------------

void complexfp64_first  (C Z, const C X, const C Y) { Z = X ; }
void complexfp64_second (C Z, const C X, const C Y) { Z = Y ; }
void complexfp64_plus   (C Z, const C X, const C Y) { Z = X + Y ; }
void complexfp64_minus  (C Z, const C X, const C Y) { Z = X - Y ; }
void complexfp64_rminus (C Z, const C X, const C Y) { Z = Y - X ; }
void complexfp64_times  (C Z, const C X, const C Y) { Z = X * Y ; }
void complexfp64_div    (C Z, const C X, const C Y) { Z = X / Y ; }
void complexfp64_rdiv   (C Z, const C X, const C Y) { Z = Y / X ; }

void complexfp64_min (C Z, const C X, const C Y)
{
    // min (x,y): complex number with smallest magnitude.  If tied, select the
    // one with the smallest phase angle (same as MATLAB definition).
    // No special cases for NaNs.
    double absx = cabs (X) ;
    double absy = cabs (Y) ;
    if (absx < absy)
    {
        Z = X ;
    }
    else if (absx > absy)
    {
        Z = Y ;
    }
    else
    {
        if (carg (X) < carg (Y))
        {
            Z = X ;
        }
        else
        {
            Z = Y ;
        }
    }
}

void complexfp64_max (C Z, const C X, const C Y)
{
    // max (x,y): complex number with largest magnitude.  If tied, select the
    // one with the largest phase angle (same as MATLAB definition).
    // No special cases for NaNs.
    double absx = cabs (X) ;
    double absy = cabs (Y) ;
    if (absx > absy)
    {
        Z = X ;
    }
    else if (absx < absy)
    {
        Z = Y ;
    }
    else
    {
        if (carg (X) > carg (Y))
        {
            Z = X ;
        }
        else
        {
            Z = Y ;
        }
    }
}

void complexfp64_skew (C Z, const C X, const C Y)
{
    Z = X == -Y ;
}

void complexfp64_pair (C Z, const C X, const C Y)
{
    Z = ONE ;
}

void complexfp64_any (C Z, const C X, const C Y)
{
    Z = Y ;
}

void complexfp64_hermitian (C Z, const C X, const C Y)
{
    Z = X == conj (Y) ;
}

GrB_BinaryOp
    LAGraph_FIRST_ComplexFP64 = NULL          ,
    LAGraph_SECOND_ComplexFP64 = NULL         ,
    LAGraph_MIN_ComplexFP64 = NULL            ,
    LAGraph_MAX_ComplexFP64 = NULL            ,
    LAGraph_PLUS_ComplexFP64 = NULL           ,
    LAGraph_MINUS_ComplexFP64 = NULL          ,
    LAGraph_TIMES_ComplexFP64 = NULL          ,
    LAGraph_DIV_ComplexFP64 = NULL            ,
    LAGraph_RMINUS_ComplexFP64 = NULL         ,
    LAGraph_RDIV_ComplexFP64 = NULL           ,
    LAGraph_SKEW_ComplexFP64 = NULL           ,
    LAGraph_PAIR_ComplexFP64 = NULL           ,
    LAGraph_ANY_ComplexFP64 = NULL            ,
    LAGraph_HERMITIAN_ComplexFP64 = NULL      ;

//------------------------------------------------------------------------------
// 6 binary functions, z=f(x,y), where CxC -> C ; (1,0) = true, (0,0) = false
//------------------------------------------------------------------------------

// inequality operators follow the MATLAB convention

#define R(x) creal(x)

void complexfp64_iseq (C Z, const C X, const C Y) { Z = (X == Y) ? T : F ; }
void complexfp64_isne (C Z, const C X, const C Y) { Z = (X != Y) ? T : F ; }
void complexfp64_isgt (C Z, const C X, const C Y) { Z = (R(X) >  R(Y)) ? T : F ; }
void complexfp64_islt (C Z, const C X, const C Y) { Z = (R(X) <  R(Y)) ? T : F ; }
void complexfp64_isge (C Z, const C X, const C Y) { Z = (R(X) >= R(Y)) ? T : F ; }
void complexfp64_isle (C Z, const C X, const C Y) { Z = (R(X) <= R(Y)) ? T : F ; }

GrB_BinaryOp
    LAGraph_ISEQ_ComplexFP64 = NULL       ,
    LAGraph_ISNE_ComplexFP64 = NULL       ,
    LAGraph_ISGT_ComplexFP64 = NULL       ,
    LAGraph_ISLT_ComplexFP64 = NULL       ,
    LAGraph_ISGE_ComplexFP64 = NULL       ,
    LAGraph_ISLE_ComplexFP64 = NULL       ;

//------------------------------------------------------------------------------
// binary boolean functions, z=f(x,y), where CxC -> C
//------------------------------------------------------------------------------

void complexfp64_or (C Z, const C X, const C Y)
{
    Z = (BOOL (X) || BOOL (Y)) ? T : F ;
}

void complexfp64_and (C Z, const C X, const C Y)
{
    Z = (BOOL (X) && BOOL (Y)) ? T : F ;
}

void complexfp64_xor (C Z, const C X, const C Y)
{
    Z = (BOOL (X) != BOOL (Y)) ? T : F ;
}

GrB_BinaryOp
    LAGraph_OR_ComplexFP64 = NULL        ,
    LAGraph_AND_ComplexFP64 = NULL       ,
    LAGraph_XOR_ComplexFP64 = NULL       ;

//------------------------------------------------------------------------------
// 6 binary functions, z=f(x,y), where CxC -> bool
//------------------------------------------------------------------------------

// inequality operators follow the MATLAB convention

void complexfp64_eq (bool Z, const C X, const C Y) { Z = (X == Y) ; }
void complexfp64_ne (bool Z, const C X, const C Y) { Z = (X != Y) ; }
void complexfp64_gt (bool Z, const C X, const C Y) { Z = (R (X) >  R (Y)) ;}
void complexfp64_lt (bool Z, const C X, const C Y) { Z = (R (X) <  R (Y)) ;}
void complexfp64_ge (bool Z, const C X, const C Y) { Z = (R (X) >= R (Y)) ;}
void complexfp64_le (bool Z, const C X, const C Y) { Z = (R (X) <= R (Y)) ;}

GrB_BinaryOp
    LAGraph_EQ_ComplexFP64 = NULL        ,
    LAGraph_NE_ComplexFP64 = NULL        ,
    LAGraph_GT_ComplexFP64 = NULL        ,
    LAGraph_LT_ComplexFP64 = NULL        ,
    LAGraph_GE_ComplexFP64 = NULL        ,
    LAGraph_LE_ComplexFP64 = NULL        ;

//------------------------------------------------------------------------------
// binary functions, z=f(x,y), where double x double -> complex
//------------------------------------------------------------------------------

void complexfp64_complex (C Z, const double X, const double Y) { Z = CMPLX (X,Y) ; }

GrB_BinaryOp LAGraph_COMPLEX_ComplexFP64 = NULL ;

//------------------------------------------------------------------------------
// unary functions, z=f(x) where C -> C
//------------------------------------------------------------------------------

void complexfp64_one      (C Z, const C X) { Z =       1. ; }
void complexfp64_identity (C Z, const C X) { Z =       X  ; }
void complexfp64_ainv     (C Z, const C X) { Z =      -X  ; }
void complexfp64_abs      (C Z, const C X) { Z = CMPLX (cabs (X), 0) ; }
void complexfp64_minv     (C Z, const C X) { Z =  1. / X  ; } 
void complexfp64_not      (C Z, const C X) { Z = BOOL (X) ? F : T ; }
void complexfp64_conj     (C Z, const C X) { Z = conj (X) ; }

void complexfp64_isone (bool Z, C X)
{
    Z = X == 1 ;
}

void complexfp64_true_bool (bool Z, C X)
{
    Z = true ;
}

GrB_UnaryOp
    LAGraph_IDENTITY_ComplexFP64 = NULL        ,
    LAGraph_AINV_ComplexFP64 = NULL            ,
    LAGraph_MINV_ComplexFP64 = NULL            ,
    LAGraph_NOT_ComplexFP64 = NULL             ,
    LAGraph_CONJ_ComplexFP64 = NULL            ,
    LAGraph_ONE_ComplexFP64 = NULL             ,
    LAGraph_ABS_ComplexFP64  = NULL            ,
    LAGraph_TRUE_BOOL_ComplexFP64 = NULL       ,     
    LAGraph_ISONE_ComplexFP64  = NULL          ;

//------------------------------------------------------------------------------
// unary functions, z=f(x) where C -> double
//------------------------------------------------------------------------------

void complexfp64_real  (double Z, const C X) { Z = creal (X) ; }
void complexfp64_imag  (double Z, const C X) { Z = cimag (X) ; }
void complexfp64_cabs  (double Z, const C X) { Z = cabs  (X) ; }
void complexfp64_angle (double Z, const C X) { Z = carg  (X) ; }

GrB_UnaryOp
    LAGraph_REAL_ComplexFP64 = NULL            ,
    LAGraph_IMAG_ComplexFP64 = NULL            ,
    LAGraph_CABS_ComplexFP64 = NULL            ,
    LAGraph_ANGLE_ComplexFP64 = NULL           ;

//------------------------------------------------------------------------------
// unary functions, z=f(x) where double -> C
//------------------------------------------------------------------------------

void complexfp64_complex_real (C Z, const double X) { Z = CMPLX (X, 0) ; }
void complexfp64_complex_imag (C Z, const double X) { Z = CMPLX (0, X) ; }

GrB_UnaryOp
    LAGraph_COMPLEX_REAL_ComplexFP64 = NULL    ,
    LAGraph_COMPLEX_IMAG_ComplexFP64 = NULL    ;

//------------------------------------------------------------------------------
// Complex type, scalars, monoids, and semiring
//------------------------------------------------------------------------------

GrB_Monoid
    LAGraph_PLUS_ComplexFP64_MONOID = NULL     ,
    LAGraph_TIMES_ComplexFP64_MONOID = NULL    ;
    
GrB_Semiring LAGraph_PLUS_TIMES_ComplexFP64 = NULL ;
C LAGraph_ComplexFP64_1  = ONE ;
C LAGraph_ComplexFP64_0 = ZERO ;

#define OK(method)                      \
    info = method ;                     \
    if (info != GrB_SUCCESS)            \
    {                                   \
        LAGraph_Complex_finalize ( ) ;  \
        return (info) ;                 \
    }

//------------------------------------------------------------------------------
// LAGraph_ComplexFP64_init: create the complex type, operators, monoids, and semiring
//------------------------------------------------------------------------------

GrB_Info LAGraph_Complex_init ( )
{

    GrB_Info info ;

    //--------------------------------------------------------------------------
    // create the Complex type
    //--------------------------------------------------------------------------

    OK (GrB_Type_new (&LAGraph_ComplexFP64, sizeof (C))) ;

    #undef C
    #undef D
    #define C LAGraph_ComplexFP64
    #define D GrB_FP64

    //--------------------------------------------------------------------------
    // create the Complex binary operators, CxC->C
    //--------------------------------------------------------------------------

    OK (GrB_BinaryOp_new (&LAGraph_FIRST_ComplexFP64       , complexfp64_first       , C, C, C)) ;
    OK (GrB_BinaryOp_new (&LAGraph_SECOND_ComplexFP64      , complexfp64_second      , C, C, C)) ;
    OK (GrB_BinaryOp_new (&LAGraph_MIN_ComplexFP64         , complexfp64_min         , C, C, C)) ;
    OK (GrB_BinaryOp_new (&LAGraph_MAX_ComplexFP64         , complexfp64_max         , C, C, C)) ;
    OK (GrB_BinaryOp_new (&LAGraph_PLUS_ComplexFP64        , complexfp64_plus        , C, C, C)) ;
    OK (GrB_BinaryOp_new (&LAGraph_MINUS_ComplexFP64       , complexfp64_minus       , C, C, C)) ;
    OK (GrB_BinaryOp_new (&LAGraph_RMINUS_ComplexFP64      , complexfp64_rminus      , C, C, C)) ;
    OK (GrB_BinaryOp_new (&LAGraph_TIMES_ComplexFP64       , complexfp64_times       , C, C, C)) ;
    OK (GrB_BinaryOp_new (&LAGraph_DIV_ComplexFP64         , complexfp64_div         , C, C, C)) ;
    OK (GrB_BinaryOp_new (&LAGraph_RDIV_ComplexFP64        , complexfp64_rdiv        , C, C, C)) ;
    OK (GrB_BinaryOp_new (&LAGraph_PAIR_ComplexFP64        , complexfp64_pair        , C, C, C)) ;
    OK (GrB_BinaryOp_new (&LAGraph_ANY_ComplexFP64         , complexfp64_any         , C, C, C)) ;
    OK (GrB_BinaryOp_new (&LAGraph_SKEW_ComplexFP64        , complexfp64_skew        , GrB_BOOL, C, C)) ;
    OK (GrB_BinaryOp_new (&LAGraph_HERMITIAN_ComplexFP64   , complexfp64_hermitian   , GrB_BOOL, C, C)) ;

    //--------------------------------------------------------------------------
    // create the Complex binary comparison operators, CxC -> C
    //--------------------------------------------------------------------------

    OK (GrB_BinaryOp_new (&LAGraph_ISEQ_ComplexFP64 , complexfp64_iseq ,  C, C, C)) ;
    OK (GrB_BinaryOp_new (&LAGraph_ISNE_ComplexFP64 , complexfp64_isne ,  C, C, C)) ;
    OK (GrB_BinaryOp_new (&LAGraph_ISGT_ComplexFP64 , complexfp64_isgt ,  C, C, C)) ;
    OK (GrB_BinaryOp_new (&LAGraph_ISLT_ComplexFP64 , complexfp64_islt ,  C, C, C)) ;
    OK (GrB_BinaryOp_new (&LAGraph_ISGE_ComplexFP64 , complexfp64_isge ,  C, C, C)) ;
    OK (GrB_BinaryOp_new (&LAGraph_ISLE_ComplexFP64 , complexfp64_isle ,  C, C, C)) ;

    //--------------------------------------------------------------------------
    // create the Complex boolean operators, CxC -> C
    //--------------------------------------------------------------------------

    OK (GrB_BinaryOp_new (&LAGraph_OR_ComplexFP64  , complexfp64_or  ,  C, C, C)) ;
    OK (GrB_BinaryOp_new (&LAGraph_AND_ComplexFP64 , complexfp64_and ,  C, C, C)) ;
    OK (GrB_BinaryOp_new (&LAGraph_XOR_ComplexFP64 , complexfp64_xor ,  C, C, C)) ;

    //--------------------------------------------------------------------------
    // create the Complex binary operators, CxC -> bool
    //--------------------------------------------------------------------------

    OK (GrB_BinaryOp_new (&LAGraph_EQ_ComplexFP64 , complexfp64_eq ,  GrB_BOOL, C, C)) ;
    OK (GrB_BinaryOp_new (&LAGraph_NE_ComplexFP64 , complexfp64_ne ,  GrB_BOOL, C, C)) ;
    OK (GrB_BinaryOp_new (&LAGraph_GT_ComplexFP64 , complexfp64_gt ,  GrB_BOOL, C, C)) ;
    OK (GrB_BinaryOp_new (&LAGraph_LT_ComplexFP64 , complexfp64_lt ,  GrB_BOOL, C, C)) ;
    OK (GrB_BinaryOp_new (&LAGraph_GE_ComplexFP64 , complexfp64_ge ,  GrB_BOOL, C, C)) ;
    OK (GrB_BinaryOp_new (&LAGraph_LE_ComplexFP64 , complexfp64_le ,  GrB_BOOL, C, C)) ;

    //--------------------------------------------------------------------------
    // create the Complex binary operator, double x double -> C
    //--------------------------------------------------------------------------

    OK (GrB_BinaryOp_new (&LAGraph_COMPLEX_ComplexFP64, complexfp64_complex, C, D, D)) ;

    //--------------------------------------------------------------------------
    // create the Complex unary operators, C->C
    //--------------------------------------------------------------------------

    OK (GrB_UnaryOp_new (&LAGraph_ONE_ComplexFP64       , complexfp64_one       , C, C)) ;
    OK (GrB_UnaryOp_new (&LAGraph_IDENTITY_ComplexFP64  , complexfp64_identity  , C, C)) ;
    OK (GrB_UnaryOp_new (&LAGraph_AINV_ComplexFP64      , complexfp64_ainv      , C, C)) ;
    OK (GrB_UnaryOp_new (&LAGraph_ABS_ComplexFP64       , complexfp64_abs       , C, C)) ;
    OK (GrB_UnaryOp_new (&LAGraph_MINV_ComplexFP64      , complexfp64_minv      , C, C)) ;
    OK (GrB_UnaryOp_new (&LAGraph_NOT_ComplexFP64       , complexfp64_not       , C, C)) ;
    OK (GrB_UnaryOp_new (&LAGraph_CONJ_ComplexFP64      , complexfp64_conj      , C, C)) ;
    OK (GrB_UnaryOp_new (&LAGraph_ISONE_ComplexFP64     , complexfp64_isone     , GrB_BOOL, C)) ;
    OK (GrB_UnaryOp_new (&LAGraph_TRUE_BOOL_ComplexFP64 , complexfp64_true_bool , GrB_BOOL, C)) ;

    //--------------------------------------------------------------------------
    // create the unary functions, C -> double
    //--------------------------------------------------------------------------

    OK (GrB_UnaryOp_new (&LAGraph_REAL_ComplexFP64  , complexfp64_real  , D, C)) ;
    OK (GrB_UnaryOp_new (&LAGraph_IMAG_ComplexFP64  , complexfp64_imag  , D, C)) ;
    OK (GrB_UnaryOp_new (&LAGraph_CABS_ComplexFP64  , complexfp64_cabs  , D, C)) ;
    OK (GrB_UnaryOp_new (&LAGraph_ANGLE_ComplexFP64 , complexfp64_angle , D, C)) ;

    //--------------------------------------------------------------------------
    // create the unary functions, double -> C
    //--------------------------------------------------------------------------

    OK (GrB_UnaryOp_new (&LAGraph_COMPLEX_REAL_ComplexFP64 , complexfp64_complex_real , C, D)) ;
    OK (GrB_UnaryOp_new (&LAGraph_COMPLEX_IMAG_ComplexFP64 , complexfp64_complex_imag , C, D)) ;

    //--------------------------------------------------------------------------
    // create the Complex monoids
    //--------------------------------------------------------------------------

    OK (GrB_Monoid_new_UDT (&LAGraph_PLUS_ComplexFP64_MONOID,  LAGraph_PLUS_ComplexFP64,  &LAGraph_ComplexFP64_0)) ;
    OK (GrB_Monoid_new_UDT (&LAGraph_TIMES_ComplexFP64_MONOID, LAGraph_TIMES_ComplexFP64, &LAGraph_ComplexFP64_1)) ;

    //--------------------------------------------------------------------------
    // create the Complex plus-times semiring
    //--------------------------------------------------------------------------

    // more could be created, but this suffices for testing GraphBLAS
    OK (GrB_Semiring_new
        (&LAGraph_PLUS_TIMES_ComplexFP64, LAGraph_PLUS_ComplexFP64_MONOID, LAGraph_TIMES_ComplexFP64)) ;

    return (GrB_SUCCESS) ;
}


//------------------------------------------------------------------------------
// LAGraph_ComplexFP64_finalize: free all complex types, operators, monoids, and semiring
//------------------------------------------------------------------------------

GrB_Info LAGraph_Complex_finalize ( )
{

    //--------------------------------------------------------------------------
    // free the Complex plus-times semiring
    //--------------------------------------------------------------------------

    GrB_Semiring_free (&LAGraph_PLUS_TIMES_ComplexFP64) ;

    //--------------------------------------------------------------------------
    // free the Complex monoids
    //--------------------------------------------------------------------------

    GrB_Monoid_free (&LAGraph_PLUS_ComplexFP64_MONOID ) ;
    GrB_Monoid_free (&LAGraph_TIMES_ComplexFP64_MONOID) ;

    //--------------------------------------------------------------------------
    // free the Complex binary operators, CxC->C
    //--------------------------------------------------------------------------

    GrB_BinaryOp_free (&LAGraph_FIRST_ComplexFP64 ) ;
    GrB_BinaryOp_free (&LAGraph_SECOND_ComplexFP64) ;
    GrB_BinaryOp_free (&LAGraph_MIN_ComplexFP64   ) ;
    GrB_BinaryOp_free (&LAGraph_MAX_ComplexFP64   ) ;
    GrB_BinaryOp_free (&LAGraph_PLUS_ComplexFP64  ) ;
    GrB_BinaryOp_free (&LAGraph_MINUS_ComplexFP64 ) ;
    GrB_BinaryOp_free (&LAGraph_RMINUS_ComplexFP64) ;
    GrB_BinaryOp_free (&LAGraph_TIMES_ComplexFP64 ) ;
    GrB_BinaryOp_free (&LAGraph_DIV_ComplexFP64   ) ;
    GrB_BinaryOp_free (&LAGraph_RDIV_ComplexFP64  ) ;
    GrB_BinaryOp_free (&LAGraph_PAIR_ComplexFP64  ) ;
    GrB_BinaryOp_free (&LAGraph_ANY_ComplexFP64  ) ;
    GrB_BinaryOp_free (&LAGraph_SKEW_ComplexFP64  ) ;
    GrB_BinaryOp_free (&LAGraph_HERMITIAN_ComplexFP64  ) ;

    GrB_BinaryOp_free (&LAGraph_ISEQ_ComplexFP64) ;
    GrB_BinaryOp_free (&LAGraph_ISNE_ComplexFP64) ;
    GrB_BinaryOp_free (&LAGraph_ISGT_ComplexFP64) ;
    GrB_BinaryOp_free (&LAGraph_ISLT_ComplexFP64) ;
    GrB_BinaryOp_free (&LAGraph_ISGE_ComplexFP64) ;
    GrB_BinaryOp_free (&LAGraph_ISLE_ComplexFP64) ;

    GrB_BinaryOp_free (&LAGraph_OR_ComplexFP64) ;
    GrB_BinaryOp_free (&LAGraph_AND_ComplexFP64) ;
    GrB_BinaryOp_free (&LAGraph_XOR_ComplexFP64) ;

    //--------------------------------------------------------------------------
    // free the Complex binary operators, CxC -> bool
    //--------------------------------------------------------------------------

    GrB_BinaryOp_free (&LAGraph_EQ_ComplexFP64) ;
    GrB_BinaryOp_free (&LAGraph_NE_ComplexFP64) ;
    GrB_BinaryOp_free (&LAGraph_GT_ComplexFP64) ;
    GrB_BinaryOp_free (&LAGraph_LT_ComplexFP64) ;
    GrB_BinaryOp_free (&LAGraph_GE_ComplexFP64) ;
    GrB_BinaryOp_free (&LAGraph_LE_ComplexFP64) ;

    //--------------------------------------------------------------------------
    // free the Complex binary operator, double x double -> complex
    //--------------------------------------------------------------------------

    GrB_BinaryOp_free (&LAGraph_COMPLEX_ComplexFP64) ;

    //--------------------------------------------------------------------------
    // free the Complex unary operators, C->C
    //--------------------------------------------------------------------------

    GrB_UnaryOp_free (&LAGraph_ONE_ComplexFP64       ) ;
    GrB_UnaryOp_free (&LAGraph_IDENTITY_ComplexFP64  ) ;
    GrB_UnaryOp_free (&LAGraph_AINV_ComplexFP64      ) ;
    GrB_UnaryOp_free (&LAGraph_ABS_ComplexFP64       ) ;
    GrB_UnaryOp_free (&LAGraph_MINV_ComplexFP64      ) ;
    GrB_UnaryOp_free (&LAGraph_NOT_ComplexFP64       ) ;
    GrB_UnaryOp_free (&LAGraph_CONJ_ComplexFP64      ) ;
    GrB_UnaryOp_free (&LAGraph_ISONE_ComplexFP64     ) ;
    GrB_UnaryOp_free (&LAGraph_TRUE_BOOL_ComplexFP64 ) ;

    //--------------------------------------------------------------------------
    // free the unary functions, C -> double
    //--------------------------------------------------------------------------

    GrB_UnaryOp_free (&LAGraph_REAL_ComplexFP64 ) ;
    GrB_UnaryOp_free (&LAGraph_IMAG_ComplexFP64 ) ;
    GrB_UnaryOp_free (&LAGraph_CABS_ComplexFP64 ) ;
    GrB_UnaryOp_free (&LAGraph_ANGLE_ComplexFP64) ;

    //--------------------------------------------------------------------------
    // free the unary functions, double -> C
    //--------------------------------------------------------------------------

    GrB_UnaryOp_free (&LAGraph_COMPLEX_REAL_ComplexFP64) ;
    GrB_UnaryOp_free (&LAGraph_COMPLEX_IMAG_ComplexFP64) ;

    //--------------------------------------------------------------------------
    // free the Complex type
    //--------------------------------------------------------------------------

    GrB_Type_free (&LAGraph_ComplexFP64) ;

    return (GrB_SUCCESS) ;
}

