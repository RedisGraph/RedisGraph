//------------------------------------------------------------------------------
// GraphBLAS.h: definitions for the GraphBLAS package
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS is a complete implementation of the GraphBLAS
// standard, which defines a set of sparse matrix operations on an extended
// algebra of semirings, using an almost unlimited variety of operators and
// types.  When applied to sparse adjacency matrices, these algebraic
// operations are equivalent to computations on graphs.  GraphBLAS provides a
// powerful and expressive framework creating graph algorithms based on the
// elegant mathematics of sparse matrix operations on a semiring.

// This GraphBLAS.h file contains GraphBLAS definitions for user applications
// to #include.  Functions and variables with the prefix GB_ need to be defined
// in this file and are thus technically visible to the user, but they must not
// be accessed in user code.  They are here only so that the ANSI C11 _Generic
// feature can be used in the user-accessible polymorphic functions.  For
// example GrB_free is a macro that uses _Generic to select the right method,
// depending on the type of its argument.

// This implementation (nearly) fully conforms to the GraphBLAS API
// Specification (see the notes in the User Guide regarding GrB_wait,
// GrB_error, and GrB_Matrix_reduce_BinaryOp).

// It also includes functions and features that are extensions to the spec,
// which are given names of the form GxB_* for functions, built-in objects, and
// macros, so it is clear which are in the spec and which are extensions.
// Extensions with the name GxB_* are user-accessible in SuiteSparse:GraphBLAS
// but cannot be guaranteed to appear in all GraphBLAS implementations.

#ifndef GRAPHBLAS_H
#define GRAPHBLAS_H

//==============================================================================
// include files required by GraphBLAS
//==============================================================================

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <inttypes.h>
#include <stddef.h>
#include <limits.h>
#include <math.h>
#include <stdarg.h>

//==============================================================================
// compiler variations
//==============================================================================

// Exporting/importing symbols for Microsoft Visual Studio

#if ( _MSC_VER && !__INTEL_COMPILER )
#ifdef GB_LIBRARY
// compiling SuiteSparse:GraphBLAS itself, exporting symbols to user apps
#define GB_PUBLIC extern __declspec ( dllexport )
#else
// compiling the user application, importing symbols from SuiteSparse:GraphBLAS
#define GB_PUBLIC extern __declspec ( dllimport )
#endif
#else
// for other compilers
#define GB_PUBLIC extern
#endif

// GraphBLAS requires an ANSI C11 compiler for its polymorphic functions (using
// the _Generic keyword), but it can be used in an C90 compiler if those
// functions are disabled.

// With ANSI C11 and later, _Generic keyword and polymorphic functions can be
// used.  Earlier versions of the language do not have this feature.

#ifdef __STDC_VERSION__
// ANSI C11: 201112L
// ANSI C99: 199901L
// ANSI C95: 199409L
#define GxB_STDC_VERSION __STDC_VERSION__
#else
// assume ANSI C90 / C89
#define GxB_STDC_VERSION 199001L
#endif

//------------------------------------------------------------------------------
// definitions for complex types
//------------------------------------------------------------------------------

// See:
// https://www.drdobbs.com/complex-arithmetic-in-the-intersection-o/184401628#

#if defined ( __cplusplus )

    extern "C++" {
        // C++ complex types
        #include <cmath>
        #include <complex>
        #undef I
        typedef std::complex<float>  GxB_FC32_t ;
        typedef std::complex<double> GxB_FC64_t ;
    }

    #define GxB_CMPLXF(r,i) GxB_FC32_t(r,i)
    #define GxB_CMPLX(r,i)  GxB_FC64_t(r,i)

#elif ( _MSC_VER && !__INTEL_COMPILER )

    // Microsoft Windows complex types
    #include <complex.h>
    #undef I
    typedef _Fcomplex GxB_FC32_t ;
    typedef _Dcomplex GxB_FC64_t ;

    #define GxB_CMPLXF(r,i) (_FCbuild (r,i))
    #define GxB_CMPLX(r,i)  ( _Cbuild (r,i))

#else

    // ANSI C11 complex types
    #include <complex.h>
    #undef I
    typedef float  complex GxB_FC32_t ;
    typedef double complex GxB_FC64_t ;

    #ifndef CMPLX
        // gcc 6.2 on the the Mac doesn't #define CMPLX
        #define GxB_CMPLX(r,i) \
        ((GxB_FC64_t)((double)(r)) + (GxB_FC64_t)((double)(i) * _Complex_I))
    #else
        // use the ANSI C11 CMPLX macro
        #define GxB_CMPLX(r,i) CMPLX (r,i)
    #endif

    #ifndef CMPLXF
        // gcc 6.2 on the the Mac doesn't #define CMPLXF
        #define GxB_CMPLXF(r,i) \
        ((GxB_FC32_t)((float)(r)) + (GxB_FC32_t)((float)(i) * _Complex_I))
    #else
        // use the ANSI C11 CMPLX macro
        #define GxB_CMPLXF(r,i) CMPLXF (r,i)
    #endif

#endif

//==============================================================================
// version control
//==============================================================================

// There are two version numbers that user codes can check against with
// compile-time #if tests:  the version of this GraphBLAS implementation,
// and the version of the GraphBLAS specification it conforms to.  User code
// can use tests like this:
//
//      #if GxB_SPEC_VERSION >= GxB_VERSION (2,0,3)
//      ... use features in GraphBLAS specification 2.0.3 ...
//      #else
//      ... only use features in early specifications
//      #endif
//
//      #if GxB_IMPLEMENTATION > GxB_VERSION (1,4,0)
//      ... use features from version 1.4.0 of a GraphBLAS package
//      #endif

// X_GRAPHBLAS: names this particular implementation:
#define GxB_SUITESPARSE_GRAPHBLAS

// GxB_VERSION: a single integer for comparing spec and version levels
#define GxB_VERSION(major,minor,sub) \
    (((major)*1000ULL + (minor))*1000ULL + (sub))

// The version of this implementation, and the GraphBLAS API version:
#define GxB_IMPLEMENTATION_NAME "SuiteSparse:GraphBLAS"
#define GxB_IMPLEMENTATION_DATE "Jan 4, 2021"
#define GxB_IMPLEMENTATION_MAJOR 4
#define GxB_IMPLEMENTATION_MINOR 0
#define GxB_IMPLEMENTATION_SUB   1
#define GxB_SPEC_DATE "Sept 25, 2019"
#define GxB_SPEC_MAJOR 1
#define GxB_SPEC_MINOR 3
#define GxB_SPEC_SUB   0

#define GxB_IMPLEMENTATION \
        GxB_VERSION (GxB_IMPLEMENTATION_MAJOR, \
                     GxB_IMPLEMENTATION_MINOR, \
                     GxB_IMPLEMENTATION_SUB)

// The 'about' string the describes this particular implementation of GraphBLAS:
#define GxB_IMPLEMENTATION_ABOUT \
"SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved." \
"\nhttp://suitesparse.com  Dept of Computer Sci. & Eng, Texas A&M University.\n"

// The GraphBLAS license for this particular implementation of GraphBLAS:
#define GxB_IMPLEMENTATION_LICENSE \
"SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved." \
"\nLicensed under the Apache License, Version 2.0 (the \"License\"); you may\n"\
"not use SuiteSparse:GraphBLAS except in compliance with the License.  You\n"  \
"may obtain a copy of the License at\n\n"                                      \
"    http://www.apache.org/licenses/LICENSE-2.0\n\n"                           \
"Unless required by applicable law or agreed to in writing, software\n"        \
"distributed under the License is distributed on an \"AS IS\" BASIS,\n"        \
"WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.\n"   \
"See the License for the specific language governing permissions and\n"        \
"limitations under the License.\n"

//------------------------------------------------------------------------------
// GraphBLAS C API version
//------------------------------------------------------------------------------

#define GxB_SPEC_VERSION GxB_VERSION(GxB_SPEC_MAJOR,GxB_SPEC_MINOR,GxB_SPEC_SUB)

// The 'spec' string describes the GraphBLAS spec:
#define GxB_SPEC_ABOUT \
"GraphBLAS C API, by Aydin Buluc, Timothy Mattson, Scott McMillan,\n"         \
"Jose' Moreira, Carl Yang, and Benjamin Brock.  Based on 'GraphBLAS\n"        \
"Mathematics by Jeremy Kepner.  See also 'Graph Algorithms in the Language\n" \
"of Linear Algebra,' edited by J. Kepner and J. Gilbert, SIAM, 2011.\n"

//==============================================================================
// GrB_Index: the GraphBLAS integer
//==============================================================================

// GrB_Index: row or column index, or matrix dimension.  This typedef is used
// for row and column indices, or matrix and vector dimensions.

typedef uint64_t GrB_Index ;

// The largest valid dimension permitted in this implementation is 2^60.
#define GxB_INDEX_MAX ((GrB_Index) (1ULL << 60))

//==============================================================================
// GraphBLAS error and informational codes
//==============================================================================

// All GraphBLAS functions return a code that indicates if it was successful
// or not.  If more information is required, the GrB_error function can be
// called, which returns a string that provides more information on the last
// return value from GraphBLAS.

typedef enum
{

    GrB_SUCCESS = 0,            // all is well

    //--------------------------------------------------------------------------
    // informational codes, not an error:
    //--------------------------------------------------------------------------

    GrB_NO_VALUE = 1,           // A(i,j) requested but not there

    //--------------------------------------------------------------------------
    // API errors:
    //--------------------------------------------------------------------------

    GrB_UNINITIALIZED_OBJECT = 2,   // object has not been initialized
    GrB_INVALID_OBJECT = 3,         // object is corrupted
    GrB_NULL_POINTER = 4,           // input pointer is NULL
    GrB_INVALID_VALUE = 5,          // generic error code; some value is bad
    GrB_INVALID_INDEX = 6,          // a row or column index is out of bounds
    GrB_DOMAIN_MISMATCH = 7,        // object domains are not compatible
    GrB_DIMENSION_MISMATCH = 8,     // matrix dimensions do not match
    GrB_OUTPUT_NOT_EMPTY = 9,       // output matrix already has values in it

    //--------------------------------------------------------------------------
    // execution errors:
    //--------------------------------------------------------------------------

    GrB_OUT_OF_MEMORY = 10,         // out of memory
    GrB_INSUFFICIENT_SPACE = 11,    // output array not large enough
    GrB_INDEX_OUT_OF_BOUNDS = 12,   // a row or column index is out of bounds
    GrB_PANIC = 13                  // unknown error, or GrB_init not called.
}
GrB_Info ;

//==============================================================================
// GrB_init / GrB_finalize
//==============================================================================

// GrB_init must called before any other GraphBLAS operation.  GrB_finalize
// must be called as the last GraphBLAS operation.

// GrB_init defines the mode that GraphBLAS will use:  blocking or
// non-blocking.  With blocking mode, all operations finish before returning to
// the user application.  With non-blocking mode, operations can be left
// pending, and are computed only when needed.

// The extension GxB_init does the work of GrB_init, but it also defines the
// memory management functions that SuiteSparse:GraphBLAS will use internally.

typedef enum
{
    GrB_NONBLOCKING = 0,    // methods may return with pending computations
    GrB_BLOCKING = 1        // no computations are ever left pending
}
GrB_Mode ;

GB_PUBLIC
GrB_Info GrB_init           // start up GraphBLAS
(
    GrB_Mode mode           // blocking or non-blocking mode
) ;

GB_PUBLIC
GrB_Info GxB_init           // start up GraphBLAS and also define malloc, etc
(
    GrB_Mode mode,          // blocking or non-blocking mode
    // pointers to memory management functions
    void * (* user_malloc_function  ) (size_t),
    void * (* user_calloc_function  ) (size_t, size_t),
    void * (* user_realloc_function ) (void *, size_t),
    void   (* user_free_function    ) (void *),
    bool user_malloc_is_thread_safe     // ADDED in V3.0: thread_safe arg
) ;

// GxB_cuda_init (DRAFT: in progress; do not rely on this function)
GB_PUBLIC
GrB_Info GxB_cuda_init      // start up GraphBLAS for use with CUDA
(
    GrB_Mode mode           // blocking or non-blocking mode
) ;

GB_PUBLIC
GrB_Info GrB_finalize (void) ;     // finish GraphBLAS

//==============================================================================
// GrB_getVersion: GraphBLAS C API version
//==============================================================================

// compile-time access to the C API Version number of this library.
#define GRB_VERSION     GxB_SPEC_MAJOR
#define GRB_SUBVERSION  GxB_SPEC_MINOR

// GrB_getVersion provides a runtime access of the C API Version.
GB_PUBLIC
GrB_Info GrB_getVersion         // runtime access to C API version number
(
    unsigned int *version,      // returns GRB_VERSION
    unsigned int *subversion    // returns GRB_SUBVERSION
) ;

//==============================================================================
// GrB_Type: data types
//==============================================================================

typedef struct GB_Type_opaque *GrB_Type ;

// GraphBLAS predefined types and their counterparts in pure C and in MATLAB
GB_PUBLIC GrB_Type
    GrB_BOOL   ,        // in C: bool               in MATLAB: logical
    GrB_INT8   ,        // in C: int8_t             in MATLAB: int8
    GrB_INT16  ,        // in C: int16_t            in MATLAB: int16
    GrB_INT32  ,        // in C: int32_t            in MATLAB: int32
    GrB_INT64  ,        // in C: int64_t            in MATLAB: int64
    GrB_UINT8  ,        // in C: uint8_t            in MATLAB: uint8
    GrB_UINT16 ,        // in C: uint16_t           in MATLAB: uint16
    GrB_UINT32 ,        // in C: uint32_t           in MATLAB: uint32
    GrB_UINT64 ,        // in C: uint64_t           in MATLAB: uint64
    GrB_FP32   ,        // in C: float              in MATLAB: single
    GrB_FP64   ,        // in C: double             in MATLAB: double
    GxB_FC32   ,        // in C: float complex      in MATLAB: single complex
    GxB_FC64   ;        // in C: double complex     in MATLAB: double complex

//------------------------------------------------------------------------------
// GB_ helper macro for polymorphic functions: do not use outside this file
//------------------------------------------------------------------------------

#if GxB_STDC_VERSION >= 201112L
#define GB_(p,prefix,func)                                      \
        const bool       p : prefix ## _ ## func ## _BOOL   ,   \
              bool       p : prefix ## _ ## func ## _BOOL   ,   \
        const int8_t     p : prefix ## _ ## func ## _INT8   ,   \
              int8_t     p : prefix ## _ ## func ## _INT8   ,   \
        const int16_t    p : prefix ## _ ## func ## _INT16  ,   \
              int16_t    p : prefix ## _ ## func ## _INT16  ,   \
        const int32_t    p : prefix ## _ ## func ## _INT32  ,   \
              int32_t    p : prefix ## _ ## func ## _INT32  ,   \
        const int64_t    p : prefix ## _ ## func ## _INT64  ,   \
              int64_t    p : prefix ## _ ## func ## _INT64  ,   \
        const uint8_t    p : prefix ## _ ## func ## _UINT8  ,   \
              uint8_t    p : prefix ## _ ## func ## _UINT8  ,   \
        const uint16_t   p : prefix ## _ ## func ## _UINT16 ,   \
              uint16_t   p : prefix ## _ ## func ## _UINT16 ,   \
        const uint32_t   p : prefix ## _ ## func ## _UINT32 ,   \
              uint32_t   p : prefix ## _ ## func ## _UINT32 ,   \
        const uint64_t   p : prefix ## _ ## func ## _UINT64 ,   \
              uint64_t   p : prefix ## _ ## func ## _UINT64 ,   \
        const float      p : prefix ## _ ## func ## _FP32   ,   \
              float      p : prefix ## _ ## func ## _FP32   ,   \
        const double     p : prefix ## _ ## func ## _FP64   ,   \
              double     p : prefix ## _ ## func ## _FP64   ,   \
        const GxB_FC32_t p : GxB    ## _ ## func ## _FC32   ,   \
              GxB_FC32_t p : GxB    ## _ ## func ## _FC32   ,   \
        const GxB_FC64_t p : GxB    ## _ ## func ## _FC64   ,   \
              GxB_FC64_t p : GxB    ## _ ## func ## _FC64   ,   \
        const void       * : prefix ## _ ## func ## _UDT    ,   \
              void       * : prefix ## _ ## func ## _UDT
#endif

//------------------------------------------------------------------------------
// GrB_Type_new:  create a new type
//------------------------------------------------------------------------------

// GrB_Type_new is implemented both as a macro and a function.  Both are
// user-callable.  The default is to use the macro, since this allows the name
// of the type to be saved as a string, for subsequent error reporting by
// GrB_error.

// If SuiteSparse:GraphBLAS is compiled with -DNMACRO then the macro versions
// of GrB_Type_new, GrB_UnaryOp_new, GrB_BinaryOp_new, and GxB_SelectOp_new
// are not made available.  The function versions are always used instead.
// #define NMACRO

#undef GrB_Type_new

GB_PUBLIC
GrB_Info GrB_Type_new           // create a new GraphBLAS type
(
    GrB_Type *type,             // handle of user type to create
    size_t sizeof_ctype         // size = sizeof (ctype) of the C type
) ;

// user code should not directly use GB_STR or GB_XSTR
// GB_STR: convert the content of x into a string "x"
#define GB_XSTR(x) GB_STR(x)
#define GB_STR(x) #x

// GrB_Type_new as a user-callable macro, which allows the name of the ctype
// to be added to the new type.
#ifndef NMACRO
#define GrB_Type_new(utype, sizeof_ctype) \
    GB_Type_new (utype, sizeof_ctype, GB_STR(sizeof_ctype))
#endif

GB_PUBLIC
GrB_Info GB_Type_new            // not user-callable; use GrB_Type_new instead
(
    GrB_Type *type,             // handle of user type to create
    size_t sizeof_ctype,        // size of the user type
    const char *name            // name of the type, as "sizeof (ctype)"
) ;

GB_PUBLIC
GrB_Info GxB_Type_size          // determine the size of the type
(
    size_t *size,               // the sizeof the type
    GrB_Type type               // type to determine the sizeof
) ;

GB_PUBLIC
GrB_Info GrB_Type_free          // free a user-defined type
(
    GrB_Type *type              // handle of user-defined type to free
) ;

//==============================================================================
// GrB_UnaryOp: unary operators
//==============================================================================

// GrB_UnaryOp: a function z=f(x).  The function f must have the signature:

//      void f (void *z, const void *x) ;

// The pointers are void * but they are always of pointers to objects of type
// ztype and xtype, respectively.  The function must typecast its arguments as
// needed from void* to ztype* and xtype*.

typedef struct GB_UnaryOp_opaque *GrB_UnaryOp ;

//------------------------------------------------------------------------------
// built-in unary operators, z = f(x)
//------------------------------------------------------------------------------

GB_PUBLIC GrB_UnaryOp
    // For these functions z=f(x), z and x have the same type.
    // The suffix in the name is the type of x and z.
    // z = x             z = -x             z = 1/x             z = ! (x != 0)
    // identity          additive           multiplicative      logical
    //                   inverse            inverse             negation
    GrB_IDENTITY_BOOL,   GrB_AINV_BOOL,     GrB_MINV_BOOL,      GxB_LNOT_BOOL,
    GrB_IDENTITY_INT8,   GrB_AINV_INT8,     GrB_MINV_INT8,      GxB_LNOT_INT8,
    GrB_IDENTITY_INT16,  GrB_AINV_INT16,    GrB_MINV_INT16,     GxB_LNOT_INT16,
    GrB_IDENTITY_INT32,  GrB_AINV_INT32,    GrB_MINV_INT32,     GxB_LNOT_INT32,
    GrB_IDENTITY_INT64,  GrB_AINV_INT64,    GrB_MINV_INT64,     GxB_LNOT_INT64,
    GrB_IDENTITY_UINT8,  GrB_AINV_UINT8,    GrB_MINV_UINT8,     GxB_LNOT_UINT8,
    GrB_IDENTITY_UINT16, GrB_AINV_UINT16,   GrB_MINV_UINT16,    GxB_LNOT_UINT16,
    GrB_IDENTITY_UINT32, GrB_AINV_UINT32,   GrB_MINV_UINT32,    GxB_LNOT_UINT32,
    GrB_IDENTITY_UINT64, GrB_AINV_UINT64,   GrB_MINV_UINT64,    GxB_LNOT_UINT64,
    GrB_IDENTITY_FP32,   GrB_AINV_FP32,     GrB_MINV_FP32,      GxB_LNOT_FP32,
    GrB_IDENTITY_FP64,   GrB_AINV_FP64,     GrB_MINV_FP64,      GxB_LNOT_FP64,
    // complex unary operators:
    GxB_IDENTITY_FC32,   GxB_AINV_FC32,     GxB_MINV_FC32,      // no LNOT
    GxB_IDENTITY_FC64,   GxB_AINV_FC64,     GxB_MINV_FC64,      // for complex

    // z = 1             z = abs(x)         z = bnot(x)         z = signum
    // one               absolute value     bitwise negation
    GxB_ONE_BOOL,        GrB_ABS_BOOL,
    GxB_ONE_INT8,        GrB_ABS_INT8,      GrB_BNOT_INT8,
    GxB_ONE_INT16,       GrB_ABS_INT16,     GrB_BNOT_INT16,
    GxB_ONE_INT32,       GrB_ABS_INT32,     GrB_BNOT_INT32,
    GxB_ONE_INT64,       GrB_ABS_INT64,     GrB_BNOT_INT64,
    GxB_ONE_UINT8,       GrB_ABS_UINT8,     GrB_BNOT_UINT8,
    GxB_ONE_UINT16,      GrB_ABS_UINT16,    GrB_BNOT_UINT16,
    GxB_ONE_UINT32,      GrB_ABS_UINT32,    GrB_BNOT_UINT32,
    GxB_ONE_UINT64,      GrB_ABS_UINT64,    GrB_BNOT_UINT64,
    GxB_ONE_FP32,        GrB_ABS_FP32,
    GxB_ONE_FP64,        GrB_ABS_FP64,
    // complex unary operators:
    GxB_ONE_FC32,        // for complex types, z = abs(x)
    GxB_ONE_FC64,        // is real; listed below.

    // Boolean negation, z = !x, where both z and x are boolean.  There is no
    // suffix since z and x are only boolean.  This operator is identical to
    // GxB_LNOT_BOOL; it just has a different name.
    GrB_LNOT ;

// GxB_ABS is now in the v1.3 spec, the following names are deprecated:
GB_PUBLIC GrB_UnaryOp

    // z = abs(x)
    GxB_ABS_BOOL,
    GxB_ABS_INT8,
    GxB_ABS_INT16,
    GxB_ABS_INT32,
    GxB_ABS_INT64,
    GxB_ABS_UINT8,
    GxB_ABS_UINT16,
    GxB_ABS_UINT32,
    GxB_ABS_UINT64,
    GxB_ABS_FP32,
    GxB_ABS_FP64 ;

//------------------------------------------------------------------------------
// Unary operators for floating-point types only
//------------------------------------------------------------------------------

// The following floating-point unary operators and their ANSI C11 equivalents,
// are only defined for floating-point (real and complex) types.

GB_PUBLIC GrB_UnaryOp

    //--------------------------------------------------------------------------
    // z = f(x) where z and x have the same type (all 4 floating-point types)
    //--------------------------------------------------------------------------

    // z = sqrt (x)     z = log (x)         z = exp (x)         z = log2 (x)
    GxB_SQRT_FP32,      GxB_LOG_FP32,       GxB_EXP_FP32,       GxB_LOG2_FP32,
    GxB_SQRT_FP64,      GxB_LOG_FP64,       GxB_EXP_FP64,       GxB_LOG2_FP64,
    GxB_SQRT_FC32,      GxB_LOG_FC32,       GxB_EXP_FC32,       GxB_LOG2_FC32,
    GxB_SQRT_FC64,      GxB_LOG_FC64,       GxB_EXP_FC64,       GxB_LOG2_FC64,

    // z = sin (x)      z = cos (x)         z = tan (x)
    GxB_SIN_FP32,       GxB_COS_FP32,       GxB_TAN_FP32,
    GxB_SIN_FP64,       GxB_COS_FP64,       GxB_TAN_FP64,
    GxB_SIN_FC32,       GxB_COS_FC32,       GxB_TAN_FC32,
    GxB_SIN_FC64,       GxB_COS_FC64,       GxB_TAN_FC64,

    // z = acos (x)     z = asin (x)        z = atan (x)
    GxB_ACOS_FP32,      GxB_ASIN_FP32,      GxB_ATAN_FP32,
    GxB_ACOS_FP64,      GxB_ASIN_FP64,      GxB_ATAN_FP64,
    GxB_ACOS_FC32,      GxB_ASIN_FC32,      GxB_ATAN_FC32,
    GxB_ACOS_FC64,      GxB_ASIN_FC64,      GxB_ATAN_FC64,

    // z = sinh (x)     z = cosh (x)        z = tanh (x)
    GxB_SINH_FP32,      GxB_COSH_FP32,      GxB_TANH_FP32,
    GxB_SINH_FP64,      GxB_COSH_FP64,      GxB_TANH_FP64,
    GxB_SINH_FC32,      GxB_COSH_FC32,      GxB_TANH_FC32,
    GxB_SINH_FC64,      GxB_COSH_FC64,      GxB_TANH_FC64,

    // z = acosh (x)    z = asinh (x)       z = atanh (x)       z = signum (x)
    GxB_ACOSH_FP32,     GxB_ASINH_FP32,     GxB_ATANH_FP32,     GxB_SIGNUM_FP32,
    GxB_ACOSH_FP64,     GxB_ASINH_FP64,     GxB_ATANH_FP64,     GxB_SIGNUM_FP64,
    GxB_ACOSH_FC32,     GxB_ASINH_FC32,     GxB_ATANH_FC32,     GxB_SIGNUM_FC32,
    GxB_ACOSH_FC64,     GxB_ASINH_FC64,     GxB_ATANH_FC64,     GxB_SIGNUM_FC64,

    // z = ceil (x)     z = floor (x)       z = round (x)       z = trunc (x)
    GxB_CEIL_FP32,      GxB_FLOOR_FP32,     GxB_ROUND_FP32,     GxB_TRUNC_FP32,
    GxB_CEIL_FP64,      GxB_FLOOR_FP64,     GxB_ROUND_FP64,     GxB_TRUNC_FP64,
    GxB_CEIL_FC32,      GxB_FLOOR_FC32,     GxB_ROUND_FC32,     GxB_TRUNC_FC32,
    GxB_CEIL_FC64,      GxB_FLOOR_FC64,     GxB_ROUND_FC64,     GxB_TRUNC_FC64,

    // z = exp2 (x)     z = expm1 (x)       z = log10 (x)       z = log1p (x)
    GxB_EXP2_FP32,      GxB_EXPM1_FP32,     GxB_LOG10_FP32,     GxB_LOG1P_FP32,
    GxB_EXP2_FP64,      GxB_EXPM1_FP64,     GxB_LOG10_FP64,     GxB_LOG1P_FP64,
    GxB_EXP2_FC32,      GxB_EXPM1_FC32,     GxB_LOG10_FC32,     GxB_LOG1P_FC32,
    GxB_EXP2_FC64,      GxB_EXPM1_FC64,     GxB_LOG10_FC64,     GxB_LOG1P_FC64,

    //--------------------------------------------------------------------------
    // z = f(x) where z and x are the same type (floating-point real only)
    //--------------------------------------------------------------------------

    // z = lgamma (x)   z = tgamma (x)      z = erf (x)         z = erfc (x)
    GxB_LGAMMA_FP32,    GxB_TGAMMA_FP32,    GxB_ERF_FP32,       GxB_ERFC_FP32,
    GxB_LGAMMA_FP64,    GxB_TGAMMA_FP64,    GxB_ERF_FP64,       GxB_ERFC_FP64,

    // frexpx and frexpe return the mantissa and exponent, respectively,
    // from the ANSI C11 frexp function.  The exponent is returned as a
    // floating-point value, not an integer.

    // z = frexpx (x)   z = frexpe (x)
    GxB_FREXPX_FP32,    GxB_FREXPE_FP32,
    GxB_FREXPX_FP64,    GxB_FREXPE_FP64,

    //--------------------------------------------------------------------------
    // z = f(x) where z and x are the same type (complex only)
    //--------------------------------------------------------------------------

    // z = conj (x)
    GxB_CONJ_FC32,
    GxB_CONJ_FC64,

    //--------------------------------------------------------------------------
    // z = f(x) where z is real and x is complex:
    //--------------------------------------------------------------------------

    // z = creal (x)    z = cimag (x)       z = carg (x)       z = abs (x)
    GxB_CREAL_FC32,     GxB_CIMAG_FC32,     GxB_CARG_FC32,     GxB_ABS_FC32,
    GxB_CREAL_FC64,     GxB_CIMAG_FC64,     GxB_CARG_FC64,     GxB_ABS_FC64,

    //--------------------------------------------------------------------------
    // z = f(x) where z is bool and x is any floating-point type
    //--------------------------------------------------------------------------

    // z = isinf (x)
    GxB_ISINF_FP32,
    GxB_ISINF_FP64,
    GxB_ISINF_FC32,     // isinf (creal (x)) || isinf (cimag (x))
    GxB_ISINF_FC64,     // isinf (creal (x)) || isinf (cimag (x))

    // z = isnan (x)
    GxB_ISNAN_FP32,
    GxB_ISNAN_FP64,
    GxB_ISNAN_FC32,     // isnan (creal (x)) || isnan (cimag (x))
    GxB_ISNAN_FC64,     // isnan (creal (x)) || isnan (cimag (x))

    // z = isfinite (x)
    GxB_ISFINITE_FP32,
    GxB_ISFINITE_FP64,
    GxB_ISFINITE_FC32,  // isfinite (real (x)) && isfinite (cimag (x))
    GxB_ISFINITE_FC64 ; // isfinite (real (x)) && isfinite (cimag (x))

//------------------------------------------------------------------------------
// methods for unary operators
//------------------------------------------------------------------------------

typedef void (*GxB_unary_function)  (void *, const void *) ;

#undef GrB_UnaryOp_new

GB_PUBLIC
GrB_Info GrB_UnaryOp_new            // create a new user-defined unary operator
(
    GrB_UnaryOp *unaryop,           // handle for the new unary operator
    GxB_unary_function function,    // pointer to the unary function
    GrB_Type ztype,                 // type of output z
    GrB_Type xtype                  // type of input x
) ;

#ifndef NMACRO
#define GrB_UnaryOp_new(op,f,z,x) GB_UnaryOp_new (op,f,z,x, GB_STR(f))
#endif

GB_PUBLIC
GrB_Info GB_UnaryOp_new             // not user-callable; use GrB_UnaryOp_new
(
    GrB_UnaryOp *unaryop,           // handle for the new unary operator
    GxB_unary_function function,    // pointer to the unary function
    GrB_Type ztype,                 // type of output z
    GrB_Type xtype,                 // type of input x
    const char *name                // name of the underlying function
) ;

GB_PUBLIC
GrB_Info GxB_UnaryOp_ztype          // return the type of z
(
    GrB_Type *ztype,                // return type of output z
    GrB_UnaryOp unaryop             // unary operator
) ;

GB_PUBLIC
GrB_Info GxB_UnaryOp_xtype          // return the type of x
(
    GrB_Type *xtype,                // return type of input x
    GrB_UnaryOp unaryop             // unary operator
) ;

GB_PUBLIC
GrB_Info GrB_UnaryOp_free           // free a user-created unary operator
(
    GrB_UnaryOp *unaryop            // handle of unary operator to free
) ;

//==============================================================================
// GrB_BinaryOp: binary operators
//==============================================================================

// GrB_BinaryOp: a function z=f(x,y).  The function f must have the signature:

//      void f (void *z, const void *x, const void *y) ;

// The pointers are void * but they are always of pointers to objects of type
// ztype, xtype, and ytype, respectively.  See Demo/usercomplex.c for examples.

typedef struct GB_BinaryOp_opaque *GrB_BinaryOp ;

//------------------------------------------------------------------------------
// built-in binary operators, z = f(x,y), where x,y,z all have the same type
//------------------------------------------------------------------------------

GB_PUBLIC GrB_BinaryOp

    // operators for all 13 types (including complex):

    // z = x            z = y               z = pow (x,y)
    GrB_FIRST_BOOL,     GrB_SECOND_BOOL,    GxB_POW_BOOL,
    GrB_FIRST_INT8,     GrB_SECOND_INT8,    GxB_POW_INT8,
    GrB_FIRST_INT16,    GrB_SECOND_INT16,   GxB_POW_INT16,
    GrB_FIRST_INT32,    GrB_SECOND_INT32,   GxB_POW_INT32,
    GrB_FIRST_INT64,    GrB_SECOND_INT64,   GxB_POW_INT64,
    GrB_FIRST_UINT8,    GrB_SECOND_UINT8,   GxB_POW_UINT8,
    GrB_FIRST_UINT16,   GrB_SECOND_UINT16,  GxB_POW_UINT16,
    GrB_FIRST_UINT32,   GrB_SECOND_UINT32,  GxB_POW_UINT32,
    GrB_FIRST_UINT64,   GrB_SECOND_UINT64,  GxB_POW_UINT64,
    GrB_FIRST_FP32,     GrB_SECOND_FP32,    GxB_POW_FP32,
    GrB_FIRST_FP64,     GrB_SECOND_FP64,    GxB_POW_FP64,
    // complex:
    GxB_FIRST_FC32,     GxB_SECOND_FC32,    GxB_POW_FC32,
    GxB_FIRST_FC64,     GxB_SECOND_FC64,    GxB_POW_FC64,

    // z = x+y          z = x-y             z = x*y             z = x/y
    GrB_PLUS_BOOL,      GrB_MINUS_BOOL,     GrB_TIMES_BOOL,     GrB_DIV_BOOL,
    GrB_PLUS_INT8,      GrB_MINUS_INT8,     GrB_TIMES_INT8,     GrB_DIV_INT8,
    GrB_PLUS_INT16,     GrB_MINUS_INT16,    GrB_TIMES_INT16,    GrB_DIV_INT16,
    GrB_PLUS_INT32,     GrB_MINUS_INT32,    GrB_TIMES_INT32,    GrB_DIV_INT32,
    GrB_PLUS_INT64,     GrB_MINUS_INT64,    GrB_TIMES_INT64,    GrB_DIV_INT64,
    GrB_PLUS_UINT8,     GrB_MINUS_UINT8,    GrB_TIMES_UINT8,    GrB_DIV_UINT8,
    GrB_PLUS_UINT16,    GrB_MINUS_UINT16,   GrB_TIMES_UINT16,   GrB_DIV_UINT16,
    GrB_PLUS_UINT32,    GrB_MINUS_UINT32,   GrB_TIMES_UINT32,   GrB_DIV_UINT32,
    GrB_PLUS_UINT64,    GrB_MINUS_UINT64,   GrB_TIMES_UINT64,   GrB_DIV_UINT64,
    GrB_PLUS_FP32,      GrB_MINUS_FP32,     GrB_TIMES_FP32,     GrB_DIV_FP32,
    GrB_PLUS_FP64,      GrB_MINUS_FP64,     GrB_TIMES_FP64,     GrB_DIV_FP64,
    // complex:
    GxB_PLUS_FC32,      GxB_MINUS_FC32,     GxB_TIMES_FC32,     GxB_DIV_FC32,
    GxB_PLUS_FC64,      GxB_MINUS_FC64,     GxB_TIMES_FC64,     GxB_DIV_FC64,

    // z = y-x          z = y/x             z = 1               z = any(x,y)
    GxB_RMINUS_BOOL,    GxB_RDIV_BOOL,      GxB_PAIR_BOOL,      GxB_ANY_BOOL,
    GxB_RMINUS_INT8,    GxB_RDIV_INT8,      GxB_PAIR_INT8,      GxB_ANY_INT8,
    GxB_RMINUS_INT16,   GxB_RDIV_INT16,     GxB_PAIR_INT16,     GxB_ANY_INT16,
    GxB_RMINUS_INT32,   GxB_RDIV_INT32,     GxB_PAIR_INT32,     GxB_ANY_INT32,
    GxB_RMINUS_INT64,   GxB_RDIV_INT64,     GxB_PAIR_INT64,     GxB_ANY_INT64,
    GxB_RMINUS_UINT8,   GxB_RDIV_UINT8,     GxB_PAIR_UINT8,     GxB_ANY_UINT8,
    GxB_RMINUS_UINT16,  GxB_RDIV_UINT16,    GxB_PAIR_UINT16,    GxB_ANY_UINT16,
    GxB_RMINUS_UINT32,  GxB_RDIV_UINT32,    GxB_PAIR_UINT32,    GxB_ANY_UINT32,
    GxB_RMINUS_UINT64,  GxB_RDIV_UINT64,    GxB_PAIR_UINT64,    GxB_ANY_UINT64,
    GxB_RMINUS_FP32,    GxB_RDIV_FP32,      GxB_PAIR_FP32,      GxB_ANY_FP32,
    GxB_RMINUS_FP64,    GxB_RDIV_FP64,      GxB_PAIR_FP64,      GxB_ANY_FP64,
    // complex:
    GxB_RMINUS_FC32,    GxB_RDIV_FC32,      GxB_PAIR_FC32,      GxB_ANY_FC32,
    GxB_RMINUS_FC64,    GxB_RDIV_FC64,      GxB_PAIR_FC64,      GxB_ANY_FC64,

    // The GxB_IS* comparison operators z=f(x,y) return the same type as their
    // inputs.  Each of them compute z = (x OP y), where x, y, and z all have
    // the same type.  The value z is either 1 for true or 0 for false, but it
    // is a value with the same type as x and y.

    // z = (x == y)     z = (x != y)        
    GxB_ISEQ_BOOL,      GxB_ISNE_BOOL,      
    GxB_ISEQ_INT8,      GxB_ISNE_INT8,      
    GxB_ISEQ_INT16,     GxB_ISNE_INT16,     
    GxB_ISEQ_INT32,     GxB_ISNE_INT32,     
    GxB_ISEQ_INT64,     GxB_ISNE_INT64,     
    GxB_ISEQ_UINT8,     GxB_ISNE_UINT8,     
    GxB_ISEQ_UINT16,    GxB_ISNE_UINT16,    
    GxB_ISEQ_UINT32,    GxB_ISNE_UINT32,    
    GxB_ISEQ_UINT64,    GxB_ISNE_UINT64,    
    GxB_ISEQ_FP32,      GxB_ISNE_FP32,      
    GxB_ISEQ_FP64,      GxB_ISNE_FP64,      
    // complex:
    GxB_ISEQ_FC32,      GxB_ISNE_FC32,
    GxB_ISEQ_FC64,      GxB_ISNE_FC64,

    // z = (x > y)      z = (x < y)         z = (x >= y)     z = (x <= y)
    GxB_ISGT_BOOL,      GxB_ISLT_BOOL,      GxB_ISGE_BOOL,      GxB_ISLE_BOOL,
    GxB_ISGT_INT8,      GxB_ISLT_INT8,      GxB_ISGE_INT8,      GxB_ISLE_INT8,
    GxB_ISGT_INT16,     GxB_ISLT_INT16,     GxB_ISGE_INT16,     GxB_ISLE_INT16,
    GxB_ISGT_INT32,     GxB_ISLT_INT32,     GxB_ISGE_INT32,     GxB_ISLE_INT32,
    GxB_ISGT_INT64,     GxB_ISLT_INT64,     GxB_ISGE_INT64,     GxB_ISLE_INT64,
    GxB_ISGT_UINT8,     GxB_ISLT_UINT8,     GxB_ISGE_UINT8,     GxB_ISLE_UINT8,
    GxB_ISGT_UINT16,    GxB_ISLT_UINT16,    GxB_ISGE_UINT16,    GxB_ISLE_UINT16,
    GxB_ISGT_UINT32,    GxB_ISLT_UINT32,    GxB_ISGE_UINT32,    GxB_ISLE_UINT32,
    GxB_ISGT_UINT64,    GxB_ISLT_UINT64,    GxB_ISGE_UINT64,    GxB_ISLE_UINT64,
    GxB_ISGT_FP32,      GxB_ISLT_FP32,      GxB_ISGE_FP32,      GxB_ISLE_FP32,
    GxB_ISGT_FP64,      GxB_ISLT_FP64,      GxB_ISGE_FP64,      GxB_ISLE_FP64,

    // z = min(x,y)     z = max (x,y)
    GrB_MIN_BOOL,       GrB_MAX_BOOL,
    GrB_MIN_INT8,       GrB_MAX_INT8,
    GrB_MIN_INT16,      GrB_MAX_INT16,
    GrB_MIN_INT32,      GrB_MAX_INT32,
    GrB_MIN_INT64,      GrB_MAX_INT64,
    GrB_MIN_UINT8,      GrB_MAX_UINT8,
    GrB_MIN_UINT16,     GrB_MAX_UINT16,
    GrB_MIN_UINT32,     GrB_MAX_UINT32,
    GrB_MIN_UINT64,     GrB_MAX_UINT64,
    GrB_MIN_FP32,       GrB_MAX_FP32,
    GrB_MIN_FP64,       GrB_MAX_FP64,

    // Binary operators for each of the 11 real types:

    // The operators convert non-boolean types internally to boolean and return
    // a value 1 or 0 in the same type, for true or false.  Each computes z =
    // ((x != 0) OP (y != 0)), where x, y, and z all the same type.  These
    // operators are useful as multiplicative operators when combined with
    // non-boolean monoids of the same type.

    // z = (x || y)     z = (x && y)        z = (x != y)
    GxB_LOR_BOOL,       GxB_LAND_BOOL,      GxB_LXOR_BOOL,
    GxB_LOR_INT8,       GxB_LAND_INT8,      GxB_LXOR_INT8,
    GxB_LOR_INT16,      GxB_LAND_INT16,     GxB_LXOR_INT16,
    GxB_LOR_INT32,      GxB_LAND_INT32,     GxB_LXOR_INT32,
    GxB_LOR_INT64,      GxB_LAND_INT64,     GxB_LXOR_INT64,
    GxB_LOR_UINT8,      GxB_LAND_UINT8,     GxB_LXOR_UINT8,
    GxB_LOR_UINT16,     GxB_LAND_UINT16,    GxB_LXOR_UINT16,
    GxB_LOR_UINT32,     GxB_LAND_UINT32,    GxB_LXOR_UINT32,
    GxB_LOR_UINT64,     GxB_LAND_UINT64,    GxB_LXOR_UINT64,
    GxB_LOR_FP32,       GxB_LAND_FP32,      GxB_LXOR_FP32,
    GxB_LOR_FP64,       GxB_LAND_FP64,      GxB_LXOR_FP64,

    // Binary operators that operate only on boolean types: LOR, LAND, LXOR,
    // and LXNOR.  The naming convention differs (_BOOL is not appended to the
    // name).  They are the same as GxB_LOR_BOOL, GxB_LAND_BOOL, and
    // GxB_LXOR_BOOL, and GrB_EQ_BOOL, respectively.

    // z = (x || y)     z = (x && y)        z = (x != y)        z = (x == y)
    GrB_LOR,            GrB_LAND,           GrB_LXOR,           GrB_LXNOR,

    // Operators for floating-point reals:

    // z = atan2(x,y)   z = hypot(x,y)      z = fmod(x,y)   z = remainder(x,y)
    GxB_ATAN2_FP32,     GxB_HYPOT_FP32,     GxB_FMOD_FP32,  GxB_REMAINDER_FP32,
    GxB_ATAN2_FP64,     GxB_HYPOT_FP64,     GxB_FMOD_FP64,  GxB_REMAINDER_FP64,

    // z = ldexp(x,y)   z = copysign (x,y)
    GxB_LDEXP_FP32,     GxB_COPYSIGN_FP32,
    GxB_LDEXP_FP64,     GxB_COPYSIGN_FP64,

    // Bitwise operations on signed and unsigned integers: note that
    // bitwise operations on signed integers can lead to different results,
    // depending on your compiler; results are implementation-defined.

    // z = (x | y)      z = (x & y)         z = (x ^ y)        z = ~(x ^ y)
    GrB_BOR_INT8,       GrB_BAND_INT8,      GrB_BXOR_INT8,     GrB_BXNOR_INT8,
    GrB_BOR_INT16,      GrB_BAND_INT16,     GrB_BXOR_INT16,    GrB_BXNOR_INT16,
    GrB_BOR_INT32,      GrB_BAND_INT32,     GrB_BXOR_INT32,    GrB_BXNOR_INT32,
    GrB_BOR_INT64,      GrB_BAND_INT64,     GrB_BXOR_INT64,    GrB_BXNOR_INT64,
    GrB_BOR_UINT8,      GrB_BAND_UINT8,     GrB_BXOR_UINT8,    GrB_BXNOR_UINT8,
    GrB_BOR_UINT16,     GrB_BAND_UINT16,    GrB_BXOR_UINT16,   GrB_BXNOR_UINT16,
    GrB_BOR_UINT32,     GrB_BAND_UINT32,    GrB_BXOR_UINT32,   GrB_BXNOR_UINT32,
    GrB_BOR_UINT64,     GrB_BAND_UINT64,    GrB_BXOR_UINT64,   GrB_BXNOR_UINT64,

    // z = bitget(x,y)  z = bitset(x,y)     z = bitclr(x,y)
    GxB_BGET_INT8,      GxB_BSET_INT8,      GxB_BCLR_INT8,
    GxB_BGET_INT16,     GxB_BSET_INT16,     GxB_BCLR_INT16,
    GxB_BGET_INT32,     GxB_BSET_INT32,     GxB_BCLR_INT32,
    GxB_BGET_INT64,     GxB_BSET_INT64,     GxB_BCLR_INT64,
    GxB_BGET_UINT8,     GxB_BSET_UINT8,     GxB_BCLR_UINT8,
    GxB_BGET_UINT16,    GxB_BSET_UINT16,    GxB_BCLR_UINT16,
    GxB_BGET_UINT32,    GxB_BSET_UINT32,    GxB_BCLR_UINT32,
    GxB_BGET_UINT64,    GxB_BSET_UINT64,    GxB_BCLR_UINT64 ;

//------------------------------------------------------------------------------
// z=f(x,y) where z and x have the same type, but y is GrB_INT8
//------------------------------------------------------------------------------

    // z = bitshift (x,y) computes z = x left-shifted by y bits if y >= 0, or z
    // = x right-shifted by (-y) bits if y < 0.  z is equal to x if y is zero.
    // z and x have the same type, as given by the suffix on the operator name.
    // Since y must be signed, it cannot have the same type as x when x is
    // unsigned; it is always GrB_INT8 for all 8 versions of this operator.
    // The GxB_BSHIFT_* operators compute the arithmetic shift, and produce the
    // same results as the MATLAB BITSHIFT function, for all possible inputs.

GB_PUBLIC GrB_BinaryOp

    // z = bitshift(x,y)
    GxB_BSHIFT_INT8,
    GxB_BSHIFT_INT16,
    GxB_BSHIFT_INT32,
    GxB_BSHIFT_INT64,
    GxB_BSHIFT_UINT8,
    GxB_BSHIFT_UINT16,
    GxB_BSHIFT_UINT32,
    GxB_BSHIFT_UINT64 ;

//------------------------------------------------------------------------------
// z=f(x,y) where z is BOOL and the type of x,y is given by the suffix
//------------------------------------------------------------------------------

GB_PUBLIC GrB_BinaryOp

    // Six comparison operators z=f(x,y) return their result as boolean, but
    // where x and y have the same type.  The suffix in their names refers to
    // the type of x and y since z is always boolean.  If used as multiply
    // operators in a semiring, they can only be combined with boolean monoids.
    // The _BOOL versions of these operators give the same results as their
    // IS*_BOOL counterparts.  GrB_EQ_BOOL and GrB_LXNOR are identical.

    // z = (x == y)     z = (x != y)        z = (x > y)         z = (x < y)
    GrB_EQ_BOOL,        GrB_NE_BOOL,        GrB_GT_BOOL,        GrB_LT_BOOL,
    GrB_EQ_INT8,        GrB_NE_INT8,        GrB_GT_INT8,        GrB_LT_INT8,
    GrB_EQ_INT16,       GrB_NE_INT16,       GrB_GT_INT16,       GrB_LT_INT16,
    GrB_EQ_INT32,       GrB_NE_INT32,       GrB_GT_INT32,       GrB_LT_INT32,
    GrB_EQ_INT64,       GrB_NE_INT64,       GrB_GT_INT64,       GrB_LT_INT64,
    GrB_EQ_UINT8,       GrB_NE_UINT8,       GrB_GT_UINT8,       GrB_LT_UINT8,
    GrB_EQ_UINT16,      GrB_NE_UINT16,      GrB_GT_UINT16,      GrB_LT_UINT16,
    GrB_EQ_UINT32,      GrB_NE_UINT32,      GrB_GT_UINT32,      GrB_LT_UINT32,
    GrB_EQ_UINT64,      GrB_NE_UINT64,      GrB_GT_UINT64,      GrB_LT_UINT64,
    GrB_EQ_FP32,        GrB_NE_FP32,        GrB_GT_FP32,        GrB_LT_FP32,
    GrB_EQ_FP64,        GrB_NE_FP64,        GrB_GT_FP64,        GrB_LT_FP64,
    // complex:
    GxB_EQ_FC32,        GxB_NE_FC32,
    GxB_EQ_FC64,        GxB_NE_FC64,

    // z = (x >= y)     z = (x <= y)
    GrB_GE_BOOL,        GrB_LE_BOOL,
    GrB_GE_INT8,        GrB_LE_INT8,
    GrB_GE_INT16,       GrB_LE_INT16,
    GrB_GE_INT32,       GrB_LE_INT32,
    GrB_GE_INT64,       GrB_LE_INT64,
    GrB_GE_UINT8,       GrB_LE_UINT8,
    GrB_GE_UINT16,      GrB_LE_UINT16,
    GrB_GE_UINT32,      GrB_LE_UINT32,
    GrB_GE_UINT64,      GrB_LE_UINT64,
    GrB_GE_FP32,        GrB_LE_FP32,
    GrB_GE_FP64,        GrB_LE_FP64 ;

//------------------------------------------------------------------------------
// z=f(x,y) where z is complex and the type of x,y is given by the suffix
//------------------------------------------------------------------------------

GB_PUBLIC GrB_BinaryOp

    // z = cmplx (x,y)
    GxB_CMPLX_FP32,
    GxB_CMPLX_FP64 ;

//==============================================================================
// positional GrB_UnaryOp and GrB_BinaryOp operators
//==============================================================================

// Positional operators do not depend on the value of an entry, but its row or
// column index in the matrix instead.  For example, for an entry A(i,j),
// first_i(A(i,j),y) is equal to i.  These operators are useful for returning
// node id's as the result of a semiring operation.  If used as a mask, zero
// has a special value, and thus z=first_i1(A(i,j),j) returns i+1 instead of i.
// This can be useful when using a positional operator to construct a mask
// matrix or vector for another GraphBLAS operation.  It is also essential for
// the MATLAB interface, since the user view of matrix indices in MATLAB is
// 1-based, not 0-based.

// When applied to a vector, j is always equal to 0.  For a GxB_SCALAR,
// both i and j are always zero.

// GraphBLAS defines a GrB_Index as uint64_t, but these operators return a
// GrB_INT32 or GrB_INT64 type, which is more flexible to use because the
// result of this operator can be negated, to flag an entry for example.  The
// value -1 can be used to denote "no node" or "no position".  GrB_INT32 is
// useful for graphs smaller than 2^31 nodes.  If the row or column index
// exceeds INT32_MAX, the result is determined by the typecast from the
// 64-bit index to the smaller 32-bit index.

// Positional operators cannot be used to construct monoids.  They can be used
// as multiplicative operators in semirings, and as operators for GrB_eWise*,
// and GrB_apply (bind first or second).  For the latter, the operator cannot
// depend on the bound scalar.

// When used as multiplicative operators in a semiring, FIRSTJ and SECONDI
// are identical.  If C(i,j) += t is computed where t = A(i,k)*B(k,j), then
// t = k in both cases.  Likewise, FIRSTJ1 and SECONDI1 are identical.

GB_PUBLIC GrB_BinaryOp

    GxB_FIRSTI_INT32,   GxB_FIRSTI_INT64,    // z = first_i(A(i,j),y) == i
    GxB_FIRSTI1_INT32,  GxB_FIRSTI1_INT64,   // z = first_i1(A(i,j),y) == i+1
    GxB_FIRSTJ_INT32,   GxB_FIRSTJ_INT64,    // z = first_j(A(i,j),y) == j
    GxB_FIRSTJ1_INT32,  GxB_FIRSTJ1_INT64,   // z = first_j1(A(i,j),y) == j+1
    GxB_SECONDI_INT32,  GxB_SECONDI_INT64,   // z = second_i(x,B(i,j)) == i
    GxB_SECONDI1_INT32, GxB_SECONDI1_INT64,  // z = second_i1(x,B(i,j)) == i+1
    GxB_SECONDJ_INT32,  GxB_SECONDJ_INT64,   // z = second_j(x,B(i,j)) == j
    GxB_SECONDJ1_INT32, GxB_SECONDJ1_INT64 ; // z = second_j1(x,B(i,j)) == j+1

GB_PUBLIC GrB_UnaryOp

    GxB_POSITIONI_INT32,  GxB_POSITIONI_INT64,  // z=position_i(A(i,j)) == i
    GxB_POSITIONI1_INT32, GxB_POSITIONI1_INT64, // z=position_i1(A(i,j)) == i+1
    GxB_POSITIONJ_INT32,  GxB_POSITIONJ_INT64,  // z=position_j(A(i,j)) == j
    GxB_POSITIONJ1_INT32, GxB_POSITIONJ1_INT64 ;// z=position_j1(A(i,j)) == j+1

//==============================================================================
// About boolean and bitwise binary operators
//==============================================================================

// Some of the boolean operators compute the same thing with different names.
// For example, x*y and x&&y give the same results for boolean x and y.
// Operations such as x < y when x and y are boolean are treated as if true=1
// and false=0.  Below is the truth table for all binary operators with boolean
// inputs.  This table is defined by how C typecasts boolean values for
// non-boolean operations.  For example, if x, y, and z are boolean, x = true,
// and y = true, then z = x + y = true + true = true.  DIV (x/y) is defined
// below.  RDIV (y/x) is shown as \ in the table; it is the same as 2nd.

//  x y  1st 2nd min max +  -  *  /  or and xor eq ne > < ge le \ pow pair
//  0 0  0   0   0   0   0  0  0  0  0  0   0   1  0  0 0 1  1  0 1   1
//  0 1  0   1   0   1   1  1  0  0  1  0   1   0  1  0 1 0  1  1 0   1
//  1 0  1   0   0   1   1  1  0  1  1  0   1   0  1  1 0 1  0  0 1   1
//  1 1  1   1   1   1   1  0  1  1  1  1   0   1  0  0 0 1  1  1 1   1

// GraphBLAS includes a GrB_DIV_BOOL operator in its specification, but does
// not define what boolean "division" means.  SuiteSparse:GraphBLAS makes the
// following interpretation.

// GraphBLAS does not generate exceptions for divide-by-zero, so the results
// are defined just as they are in MATLAB.  Floating-point divide-by-zero
// follows the IEEE 754 standard: 1/0 is +Inf, -1/0 is -Inf, and 0/0 is NaN.
// For integer division by zero, if x is positive, x/0 is the largest integer,
// -x/0 is the integer minimum (zero for unsigned integers), and 0/0 is zero.
// For example, for int8, 1/0 is 127, and -1/0 is -128.  For uint8, 1/0 is 255
// and 0/0 is zero.

// Boolean division is not in MATLAB.  For SuiteSparse:GraphBLAS, boolean
// division is treated as if it were an unsigned integer type with true=1 and
// false=0, and with the max and min value being 1 and 0.  As a result,
// GrB_IDENTITY_BOOL, GrB_AINV_BOOL, and GrB_MINV_BOOL all give the same result
// (z = x).

// With this convention for boolean "division", there are 11 unique binary
// operators that are purely boolean.  Other named *_BOOL operators are
// redundant but are included in GraphBLAS so that the name space of operators
// is complete.  Below is a list of all operators and their equivalents.

//                   x: 0 0 1 1
//                   y: 0 1 0 1
//                   z: see below
//
//      z = 0           0 0 0 0     (zero function, not predefined)
//      z = (x && y)    0 0 0 1     AND, MIN, TIMES
//      z = (x > y)     0 0 1 0     GT, ISGT, and set diff (x\y)
//      z = x           0 0 1 1     FIRST, DIV
//
//      z = (x < y)     0 1 0 0     LT, ISLT, and set diff (y\x)
//      z = y           0 1 0 1     SECOND, RDIV
//      z = (x != y)    0 1 1 0     XOR, MINUS, RMINUS, NE, ISNE
//      z = (x || y)    0 1 1 1     OR, MAX, PLUS
//
//      z = ~(x || y)   1 0 0 0     (nor(x,y) function, not predefined)
//      z = (x == y)    1 0 0 1     LXNOR, EQ, ISEQ
//      z = ~y          1 0 1 0     (not(y), not predefined)
//      z = (x >= y)    1 0 1 1     GE, ISGE, POW, and "x implies y"
//
//      z = ~x          1 1 0 0     (not(x), not predefined)
//      z = (x <= y)    1 1 0 1     LE, ISLE, and "y implies x"
//      z = ~(x && y)   1 1 1 0     (nand(x,y) function, not predefined)
//      z = 1           1 1 1 1     PAIR
//
//      z = any(x,y)    . . . .     ANY (pick x or y arbitrarily)

// Four more that have no _BOOL suffix are also redundant with the operators
// of the form GxB_*_BOOL (GrB_LOR, GrB_LAND, GrB_LXOR, and GrB_LXNOR).

// Note that the boolean binary operator space is not complete.  Five other
// boolean functions could be pre-defined as well:  z = 0, nor(x,y),
// nand(x,y), not(x), and not(y).

// Four of the possible 16 bitwise operators are pre-defined: BOR, BAND,
// BXOR, and BXNOR.  This assumes that the computations for each bit are
// entirely independent (so BSHIFT would not fit in the table above).

//------------------------------------------------------------------------------
// methods for binary operators
//------------------------------------------------------------------------------

typedef void (*GxB_binary_function) (void *, const void *, const void *) ;

#undef GrB_BinaryOp_new

GB_PUBLIC
GrB_Info GrB_BinaryOp_new
(
    GrB_BinaryOp *binaryop,         // handle for the new binary operator
    GxB_binary_function function,   // pointer to the binary function
    GrB_Type ztype,                 // type of output z
    GrB_Type xtype,                 // type of input x
    GrB_Type ytype                  // type of input y
) ;

#ifndef NMACRO
#define GrB_BinaryOp_new(op,f,z,x,y) GB_BinaryOp_new (op,f,z,x,y, GB_STR(f))
#endif

GB_PUBLIC
GrB_Info GB_BinaryOp_new            // not user-callable; use GrB_BinaryOp_new
(
    GrB_BinaryOp *binaryop,         // handle for the new binary operator
    GxB_binary_function function,   // pointer to the binary function
    GrB_Type ztype,                 // type of output z
    GrB_Type xtype,                 // type of input x
    GrB_Type ytype,                 // type of input y
    const char *name                // name of the underlying function
) ;

GB_PUBLIC
GrB_Info GxB_BinaryOp_ztype         // return the type of z
(
    GrB_Type *ztype,                // return type of output z
    GrB_BinaryOp binaryop           // binary operator to query
) ;

GB_PUBLIC
GrB_Info GxB_BinaryOp_xtype         // return the type of x
(
    GrB_Type *xtype,                // return type of input x
    GrB_BinaryOp binaryop           // binary operator to query
) ;

GB_PUBLIC
GrB_Info GxB_BinaryOp_ytype         // return the type of y
(
    GrB_Type *ytype,                // return type of input y
    GrB_BinaryOp binaryop           // binary operator to query
) ;

GB_PUBLIC
GrB_Info GrB_BinaryOp_free          // free a user-created binary operator
(
    GrB_BinaryOp *binaryop          // handle of binary operator to free
) ;

//==============================================================================
// GxB_SelectOp: select operators
//==============================================================================

// GxB_SelectOp is an operator used by GxB_select to select entries from an
// input matrix A that are kept in the output C.  If an entry A(i,j) in the
// matrix A, of size nrows-by-ncols, has the value aij, then it calls the
// select function as result = f (i, j, aij, thunk).  If the function returns
// true, the entry is kept in the output C.  If f returns false, the entry is
// not kept in C.  The type of x for the GxB_SelectOp operator may be any of
// the 11 built-in types, or any user-defined type.  It may also be GrB_NULL,
// to indicate that the function is type-generic and does not depend at all on
// the value aij.  In this case, x is passed to f as a NULL pointer.

// The optional Thunk parameter to GxB_select is a GxB_Scalar.  For built-in
// select operators (TRIL, TRIU, DIAG, and OFFDIAG), Thunk must have any
// built-in type, and thunk = (int64_t) Thunk is used to specific the diagonal
// for these operators.  Thunk may be NULL, in which case its value is treated
// as zero, if it has a built-in type. The value of Thunk (if present) is not
// modified by any built-in select operator.

// For user-defined select operators, Thunk is not typecasted at all.  If
// the user operator is defined with a non-NULL Thunk input, then it must
// be non-NULL and of the same type, when calling GxB_select.

// GxB_SelectOp:  a function z=f(i,j,x,thunk) for the GxB_Select operation.
// The function f must have the signature:

//      bool f (GrB_Index i, GrB_Index j,
//              const void *x, const void *thunk) ;

// NOTE: GxB_SelectOp has changed in v4.  In v3.3.3 it had this syntax:

//      bool f (GrB_Index i, GrB_Index j,
//              GrB_Index nrows, GrB_Index ncols,
//              const void *x, const void *thunk) ;

typedef struct GB_SelectOp_opaque *GxB_SelectOp ;

//------------------------------------------------------------------------------
// built-in select operators
//------------------------------------------------------------------------------

// GxB_select (C, Mask, accum, op, A, Thunk, desc) always returns a matrix C of
// the same size as A (or A' if GrB_TRAN is in the descriptor).

GB_PUBLIC GxB_SelectOp

    GxB_TRIL,       // C=tril(A,thunk):   returns true if ((j-i) <= thunk)
    GxB_TRIU,       // C=triu(A,thunk):   returns true if ((j-i) >= thunk)
    GxB_DIAG,       // C=diag(A,thunk):   returns true if ((j-i) == thunk)
    GxB_OFFDIAG,    // C=A-diag(A,thunk): returns true if ((j-i) != thunk)

    GxB_NONZERO,    // C=A(A ~= 0)
    GxB_EQ_ZERO,    // C=A(A == 0)
    GxB_GT_ZERO,    // C=A(A >  0)
    GxB_GE_ZERO,    // C=A(A >= 0)
    GxB_LT_ZERO,    // C=A(A <  0)
    GxB_LE_ZERO,    // C=A(A <= 0)

    GxB_NE_THUNK,   // C=A(A ~= thunk)
    GxB_EQ_THUNK,   // C=A(A == thunk)
    GxB_GT_THUNK,   // C=A(A >  thunk)
    GxB_GE_THUNK,   // C=A(A >= thunk)
    GxB_LT_THUNK,   // C=A(A <  thunk)
    GxB_LE_THUNK ;  // C=A(A <= thunk)

// For GxB_TRIL, GxB_TRIU, GxB_DIAG, and GxB_OFFDIAG, the parameter Thunk is a
// GxB_Scalar of any built-in type.  If GrB_NULL, or empty, Thunk is treated as
// zero.  Otherwise, the single entry is typecasted as (int64_t) Thunk.
// These select operators do not depend on the values of A, but just their
// position, and they work on matrices of any type.

// For GxB_*ZERO, the result depends only on the value of A(i,j).  The Thunk
// parameter to GxB_select is ignored and may be GrB_NULL.

// The operators GxB_TRIL, GxB_TRIU, GxB_DIAG, GxB_OFFDIAG, GxB_NONZERO,
// GxB_EQ_ZERO, GxB_NE_THUNK, and GxB_EQ_THUNK work on all built-in types and
// all user-defined types.

// GxB_GT_*, GxB_GE_*, GxB_LT_*, and GxB_LE_* only work on the 11 built-in
// types (not complex).  They cannot be used for user-defined types.

//------------------------------------------------------------------------------
// select operators
//------------------------------------------------------------------------------

typedef bool (*GxB_select_function)      // return true if A(i,j) is kept
(
    GrB_Index i,                // row index of A(i,j)
    GrB_Index j,                // column index of A(i,j)
    const void *x,              // value of A(i,j)
    const void *thunk           // optional input for select function
) ;

#undef GxB_SelectOp_new

GB_PUBLIC
GrB_Info GxB_SelectOp_new       // create a new user-defined select operator
(
    GxB_SelectOp *selectop,     // handle for the new select operator
    GxB_select_function function,// pointer to the select function
    GrB_Type xtype,             // type of input x, or NULL if type-generic
    GrB_Type ttype              // type of thunk, or NULL if not used
) ;

#ifndef NMACRO
#define GxB_SelectOp_new(op,f,x,t) GB_SelectOp_new (op,f,x,t, GB_STR(f))
#endif

GB_PUBLIC
GrB_Info GB_SelectOp_new        // not user-callable; use GxB_SelectOp_new
(
    GxB_SelectOp *selectop,     // handle for the new select operator
    GxB_select_function function,// pointer to the select function
    GrB_Type xtype,             // type of input x
    GrB_Type ttype,             // type of thunk, or NULL if not used
    const char *name            // name of the underlying function
) ;

GB_PUBLIC
GrB_Info GxB_SelectOp_xtype     // return the type of x
(
    GrB_Type *xtype,            // return type of input x
    GxB_SelectOp selectop       // select operator
) ;

GB_PUBLIC
GrB_Info GxB_SelectOp_ttype     // return the type of thunk
(
    GrB_Type *ttype,            // return type of input thunk
    GxB_SelectOp selectop       // select operator
) ;

GB_PUBLIC
GrB_Info GxB_SelectOp_free      // free a user-created select operator
(
    GxB_SelectOp *selectop      // handle of select operator to free
) ;

//==============================================================================
// GrB_Monoid
//==============================================================================

// A monoid is an associative operator z=op(x,y) where all three types of z, x,
// and y are identical.  The monoid also has an identity element, such that
// op(x,identity) = op(identity,x) = x.

typedef struct GB_Monoid_opaque *GrB_Monoid ;

GB_PUBLIC
GrB_Info GrB_Monoid_new_BOOL        // create a new boolean monoid
(
    GrB_Monoid *monoid,             // handle of monoid to create
    GrB_BinaryOp op,                // binary operator of the monoid
    bool identity                   // identity value of the monoid
) ;

GB_PUBLIC
GrB_Info GrB_Monoid_new_INT8        // create a new int8 monoid
(
    GrB_Monoid *monoid,             // handle of monoid to create
    GrB_BinaryOp op,                // binary operator of the monoid
    int8_t identity                 // identity value of the monoid
) ;

GB_PUBLIC
GrB_Info GrB_Monoid_new_UINT8       // create a new uint8 monoid
(
    GrB_Monoid *monoid,             // handle of monoid to create
    GrB_BinaryOp op,                // binary operator of the monoid
    uint8_t identity                // identity value of the monoid
) ;

GB_PUBLIC
GrB_Info GrB_Monoid_new_INT16       // create a new int16 monoid
(
    GrB_Monoid *monoid,             // handle of monoid to create
    GrB_BinaryOp op,                // binary operator of the monoid
    int16_t identity                // identity value of the monoid
) ;

GB_PUBLIC
GrB_Info GrB_Monoid_new_UINT16      // create a new uint16 monoid
(
    GrB_Monoid *monoid,             // handle of monoid to create
    GrB_BinaryOp op,                // binary operator of the monoid
    uint16_t identity               // identity value of the monoid
) ;

GB_PUBLIC
GrB_Info GrB_Monoid_new_INT32       // create a new int32 monoid
(
    GrB_Monoid *monoid,             // handle of monoid to create
    GrB_BinaryOp op,                // binary operator of the monoid
    int32_t identity                // identity value of the monoid
) ;

GB_PUBLIC
GrB_Info GrB_Monoid_new_UINT32      // create a new uint32 monoid
(
    GrB_Monoid *monoid,             // handle of monoid to create
    GrB_BinaryOp op,                // binary operator of the monoid
    uint32_t identity               // identity value of the monoid
) ;

GB_PUBLIC
GrB_Info GrB_Monoid_new_INT64       // create a new int64 monoid
(
    GrB_Monoid *monoid,             // handle of monoid to create
    GrB_BinaryOp op,                // binary operator of the monoid
    int64_t identity                // identity value of the monoid
) ;

GB_PUBLIC
GrB_Info GrB_Monoid_new_UINT64      // create a new uint64 monoid
(
    GrB_Monoid *monoid,             // handle of monoid to create
    GrB_BinaryOp op,                // binary operator of the monoid
    uint64_t identity               // identity value of the monoid
) ;

GB_PUBLIC
GrB_Info GrB_Monoid_new_FP32        // create a new float monoid
(
    GrB_Monoid *monoid,             // handle of monoid to create
    GrB_BinaryOp op,                // binary operator of the monoid
    float identity                  // identity value of the monoid
) ;

GB_PUBLIC
GrB_Info GrB_Monoid_new_FP64        // create a new double monoid
(
    GrB_Monoid *monoid,             // handle of monoid to create
    GrB_BinaryOp op,                // binary operator of the monoid
    double identity                 // identity value of the monoid
) ;

GB_PUBLIC
GrB_Info GxB_Monoid_new_FC32        // create a new float complex monoid
(
    GrB_Monoid *monoid,             // handle of monoid to create
    GrB_BinaryOp op,                // binary operator of the monoid
    GxB_FC32_t identity             // identity value of the monoid
) ;

GB_PUBLIC
GrB_Info GxB_Monoid_new_FC64        // create a new double complex monoid
(
    GrB_Monoid *monoid,             // handle of monoid to create
    GrB_BinaryOp op,                // binary operator of the monoid
    GxB_FC64_t identity             // identity value of the monoid
) ;

GB_PUBLIC
GrB_Info GrB_Monoid_new_UDT         // create a monoid with a user-defined type
(
    GrB_Monoid *monoid,             // handle of monoid to create
    GrB_BinaryOp op,                // binary operator of the monoid
    void *identity                  // identity value of the monoid
) ;

// Type-generic method for creating a new monoid:

/*

GB_PUBLIC
GrB_Info GrB_Monoid_new             // create a monoid
(
    GrB_Monoid *monoid,             // handle of monoid to create
    GrB_BinaryOp op,          // binary operator of the monoid
    <type> identity           // identity value of the monoid
) ;

*/

#if GxB_STDC_VERSION >= 201112L
#define GrB_Monoid_new(monoid,op,identity) \
    _Generic ((identity), GB_(, GrB, Monoid_new)) (monoid, op, identity) ;
#endif

// GxB_Monoid_terminal_new is identical to GrB_Monoid_new, except that a
// terminal value can be specified.  The terminal may be NULL, which indicates
// no terminal value (and in this case, it is identical to GrB_Monoid_new).
// The terminal value, if not NULL, must have the same type as the identity.

GB_PUBLIC
GrB_Info GxB_Monoid_terminal_new_BOOL        // create a new boolean monoid
(
    GrB_Monoid *monoid,             // handle of monoid to create
    GrB_BinaryOp op,                // binary operator of the monoid
    bool identity,                  // identity value of the monoid
    bool terminal                   // terminal value of the monoid
) ;

GB_PUBLIC
GrB_Info GxB_Monoid_terminal_new_INT8        // create a new int8 monoid
(
    GrB_Monoid *monoid,             // handle of monoid to create
    GrB_BinaryOp op,                // binary operator of the monoid
    int8_t identity,                // identity value of the monoid
    int8_t terminal                 // terminal value of the monoid
) ;

GB_PUBLIC
GrB_Info GxB_Monoid_terminal_new_UINT8       // create a new uint8 monoid
(
    GrB_Monoid *monoid,             // handle of monoid to create
    GrB_BinaryOp op,                // binary operator of the monoid
    uint8_t identity,               // identity value of the monoid
    uint8_t terminal                // terminal value of the monoid
) ;

GB_PUBLIC
GrB_Info GxB_Monoid_terminal_new_INT16       // create a new int16 monoid
(
    GrB_Monoid *monoid,             // handle of monoid to create
    GrB_BinaryOp op,                // binary operator of the monoid
    int16_t identity,               // identity value of the monoid
    int16_t terminal                // terminal value of the monoid
) ;

GB_PUBLIC
GrB_Info GxB_Monoid_terminal_new_UINT16      // create a new uint16 monoid
(
    GrB_Monoid *monoid,             // handle of monoid to create
    GrB_BinaryOp op,                // binary operator of the monoid
    uint16_t identity,              // identity value of the monoid
    uint16_t terminal               // terminal value of the monoid
) ;

GB_PUBLIC
GrB_Info GxB_Monoid_terminal_new_INT32       // create a new int32 monoid
(
    GrB_Monoid *monoid,             // handle of monoid to create
    GrB_BinaryOp op,                // binary operator of the monoid
    int32_t identity,               // identity value of the monoid
    int32_t terminal                // terminal value of the monoid
) ;

GB_PUBLIC
GrB_Info GxB_Monoid_terminal_new_UINT32      // create a new uint32 monoid
(
    GrB_Monoid *monoid,             // handle of monoid to create
    GrB_BinaryOp op,                // binary operator of the monoid
    uint32_t identity,              // identity value of the monoid
    uint32_t terminal               // terminal value of the monoid
) ;

GB_PUBLIC
GrB_Info GxB_Monoid_terminal_new_INT64       // create a new int64 monoid
(
    GrB_Monoid *monoid,             // handle of monoid to create
    GrB_BinaryOp op,                // binary operator of the monoid
    int64_t identity,               // identity value of the monoid
    int64_t terminal                // terminal value of the monoid
) ;

GB_PUBLIC
GrB_Info GxB_Monoid_terminal_new_UINT64      // create a new uint64 monoid
(
    GrB_Monoid *monoid,             // handle of monoid to create
    GrB_BinaryOp op,                // binary operator of the monoid
    uint64_t identity,              // identity value of the monoid
    uint64_t terminal               // terminal value of the monoid
) ;

GB_PUBLIC
GrB_Info GxB_Monoid_terminal_new_FP32        // create a new float monoid
(
    GrB_Monoid *monoid,             // handle of monoid to create
    GrB_BinaryOp op,                // binary operator of the monoid
    float identity,                 // identity value of the monoid
    float terminal                  // terminal value of the monoid
) ;

GB_PUBLIC
GrB_Info GxB_Monoid_terminal_new_FP64        // create a new double monoid
(
    GrB_Monoid *monoid,             // handle of monoid to create
    GrB_BinaryOp op,                // binary operator of the monoid
    double identity,                // identity value of the monoid
    double terminal                 // terminal value of the monoid
) ;

GB_PUBLIC
GrB_Info GxB_Monoid_terminal_new_FC32   // create a new float complex monoid
(
    GrB_Monoid *monoid,             // handle of monoid to create
    GrB_BinaryOp op,                // binary operator of the monoid
    GxB_FC32_t identity,            // identity value of the monoid
    GxB_FC32_t terminal             // terminal value of the monoid
) ;

GB_PUBLIC
GrB_Info GxB_Monoid_terminal_new_FC64   // create a new double complex monoid
(
    GrB_Monoid *monoid,             // handle of monoid to create
    GrB_BinaryOp op,                // binary operator of the monoid
    GxB_FC64_t identity,            // identity value of the monoid
    GxB_FC64_t terminal             // terminal value of the monoid
) ;

GB_PUBLIC
GrB_Info GxB_Monoid_terminal_new_UDT    // create a monoid with a user type
(
    GrB_Monoid *monoid,             // handle of monoid to create
    GrB_BinaryOp op,                // binary operator of the monoid
    void *identity,                 // identity value of the monoid
    void *terminal                  // terminal value of the monoid
) ;

// Type-generic method for creating a new monoid with a terminal value:

/*

GB_PUBLIC
GrB_Info GxB_Monoid_terminal_new             // create a monoid
(
    GrB_Monoid *monoid,             // handle of monoid to create
    GrB_BinaryOp op,                // binary operator of the monoid
    <type> identity,                // identity value of the monoid
    <type> terminal                 // terminal value of the monoid
) ;

*/

#if GxB_STDC_VERSION >= 201112L
#define GxB_Monoid_terminal_new(monoid,op,identity,terminal)    \
    _Generic ((identity), GB_(, GxB, Monoid_terminal_new))      \
    (monoid, op, identity, terminal) ;
#endif

GB_PUBLIC
GrB_Info GxB_Monoid_operator        // return the monoid operator
(
    GrB_BinaryOp *op,               // returns the binary op of the monoid
    GrB_Monoid monoid               // monoid to query
) ;

GB_PUBLIC
GrB_Info GxB_Monoid_identity        // return the monoid identity
(
    void *identity,                 // returns the identity of the monoid
    GrB_Monoid monoid               // monoid to query
) ;

GB_PUBLIC
GrB_Info GxB_Monoid_terminal        // return the monoid terminal
(
    bool *has_terminal,             // true if the monoid has a terminal value
    void *terminal,                 // returns the terminal of the monoid,
                                    // unmodified if has_terminal is false
    GrB_Monoid monoid               // monoid to query
) ;

GB_PUBLIC
GrB_Info GrB_Monoid_free            // free a user-created monoid
(
    GrB_Monoid *monoid              // handle of monoid to free
) ;

//==============================================================================
// GrB_Semiring
//==============================================================================

typedef struct GB_Semiring_opaque *GrB_Semiring ;

GB_PUBLIC
GrB_Info GrB_Semiring_new           // create a semiring
(
    GrB_Semiring *semiring,         // handle of semiring to create
    GrB_Monoid add,                 // add monoid of the semiring
    GrB_BinaryOp multiply           // multiply operator of the semiring
) ;

GB_PUBLIC
GrB_Info GxB_Semiring_add           // return the add monoid of a semiring
(
    GrB_Monoid *add,                // returns add monoid of the semiring
    GrB_Semiring semiring           // semiring to query
) ;

GB_PUBLIC
GrB_Info GxB_Semiring_multiply      // return multiply operator of a semiring
(
    GrB_BinaryOp *multiply,         // returns multiply operator of the semiring
    GrB_Semiring semiring           // semiring to query
) ;

GB_PUBLIC
GrB_Info GrB_Semiring_free          // free a user-created semiring
(
    GrB_Semiring *semiring          // handle of semiring to free
) ;

//==============================================================================
// GxB_Scalar: a GraphBLAS scalar
//==============================================================================

typedef struct GB_Scalar_opaque *GxB_Scalar ;

// These methods create, free, copy, and clear a GxB_Scalar.  The nvals,
// and type methods return basic information about a GxB_Scalar.

GB_PUBLIC
GrB_Info GxB_Scalar_new     // create a new GxB_Scalar with no entry
(
    GxB_Scalar *s,          // handle of GxB_Scalar to create
    GrB_Type type           // type of GxB_Scalar to create
) ;

GB_PUBLIC
GrB_Info GxB_Scalar_dup     // make an exact copy of a GxB_Scalar
(
    GxB_Scalar *s,          // handle of output GxB_Scalar to create
    const GxB_Scalar t      // input GxB_Scalar to copy
) ;

GB_PUBLIC
GrB_Info GxB_Scalar_clear   // clear a GxB_Scalar of its entry
(                           // type remains unchanged.
    GxB_Scalar s            // GxB_Scalar to clear
) ;

GB_PUBLIC
GrB_Info GxB_Scalar_nvals   // get the number of entries in a GxB_Scalar
(
    GrB_Index *nvals,       // GxB_Scalar has nvals entries (0 or 1)
    const GxB_Scalar s      // GxB_Scalar to query
) ;

GB_PUBLIC
GrB_Info GxB_Scalar_type    // get the type of a GxB_Scalar
(
    GrB_Type *type,         // returns the type of the GxB_Scalar
    const GxB_Scalar s      // GxB_Scalar to query
) ;

GB_PUBLIC
GrB_Info GxB_Scalar_free    // free a GxB_Scalar
(
    GxB_Scalar *s           // handle of GxB_Scalar to free
) ;

//------------------------------------------------------------------------------
// GxB_Scalar_setElement
//------------------------------------------------------------------------------

// Set a single GxB_Scalar s, from a user scalar x: s = x, typecasting from the
// type of x to the type of w as needed.

GB_PUBLIC
GrB_Info GxB_Scalar_setElement_BOOL     // s = x
(
    GxB_Scalar s,                       // GxB_Scalar to modify
    bool x                              // user scalar to assign to s
) ;

GB_PUBLIC
GrB_Info GxB_Scalar_setElement_INT8     // s = x
(
    GxB_Scalar s,                       // GxB_Scalar to modify
    int8_t x                            // user scalar to assign to s
) ;

GB_PUBLIC
GrB_Info GxB_Scalar_setElement_UINT8    // s = x
(
    GxB_Scalar s,                       // GxB_Scalar to modify
    uint8_t x                           // user scalar to assign to s
) ;

GB_PUBLIC
GrB_Info GxB_Scalar_setElement_INT16    // s = x
(
    GxB_Scalar s,                       // GxB_Scalar to modify
    int16_t x                           // user scalar to assign to s
) ;

GB_PUBLIC
GrB_Info GxB_Scalar_setElement_UINT16   // s = x
(
    GxB_Scalar s,                       // GxB_Scalar to modify
    uint16_t x                          // user scalar to assign to s
) ;

GB_PUBLIC
GrB_Info GxB_Scalar_setElement_INT32    // s = x
(
    GxB_Scalar s,                       // GxB_Scalar to modify
    int32_t x                           // user scalar to assign to s
) ;

GB_PUBLIC
GrB_Info GxB_Scalar_setElement_UINT32   // s = x
(
    GxB_Scalar s,                       // GxB_Scalar to modify
    uint32_t x                          // user scalar to assign to s
) ;

GB_PUBLIC
GrB_Info GxB_Scalar_setElement_INT64    // s = x
(
    GxB_Scalar s,                       // GxB_Scalar to modify
    int64_t x                           // user scalar to assign to s
) ;

GB_PUBLIC
GrB_Info GxB_Scalar_setElement_UINT64   // s = x
(
    GxB_Scalar s,                       // GxB_Scalar to modify
    uint64_t x                          // user scalar to assign to s
) ;

GB_PUBLIC
GrB_Info GxB_Scalar_setElement_FP32     // s = x
(
    GxB_Scalar s,                       // GxB_Scalar to modify
    float x                             // user scalar to assign to s
) ;

GB_PUBLIC
GrB_Info GxB_Scalar_setElement_FP64     // s = x
(
    GxB_Scalar s,                       // GxB_Scalar to modify
    double x                            // user scalar to assign to s
) ;

GB_PUBLIC
GrB_Info GxB_Scalar_setElement_FC32     // s = x
(
    GxB_Scalar s,                       // GxB_Scalar to modify
    GxB_FC32_t x                        // user scalar to assign to s
) ;

GB_PUBLIC
GrB_Info GxB_Scalar_setElement_FC64     // s = x
(
    GxB_Scalar s,                       // GxB_Scalar to modify
    GxB_FC64_t x                        // user scalar to assign to s
) ;

GB_PUBLIC
GrB_Info GxB_Scalar_setElement_UDT      // s = x
(
    GxB_Scalar s,                       // GxB_Scalar to modify
    void *x                             // user scalar to assign to s
) ;

// Type-generic version:  x can be any supported C type or void * for a
// user-defined type.

/*

GB_PUBLIC
GrB_Info GxB_Scalar_setElement          // s = x
(
    GxB_Scalar s,                       // GxB_Scalar to modify
    <type> x                            // user scalar to assign to s
) ;

*/

#if GxB_STDC_VERSION >= 201112L
#define GxB_Scalar_setElement(s,x)  \
    _Generic ((x), GB_(, GxB, Scalar_setElement)) (s, x)
#endif

//------------------------------------------------------------------------------
// GxB_Scalar_extractElement
//------------------------------------------------------------------------------

// Extract a single entry from a GxB_Scalar, x = s, typecasting from the type
// of s to the type of x as needed.

GB_PUBLIC
GrB_Info GxB_Scalar_extractElement_BOOL     // x = s
(
    bool *x,                        // user scalar extracted
    const GxB_Scalar s              // GxB_Scalar to extract an entry from
) ;

GB_PUBLIC
GrB_Info GxB_Scalar_extractElement_INT8     // x = s
(
    int8_t *x,                      // user scalar extracted
    const GxB_Scalar s              // GxB_Scalar to extract an entry from
) ;

GB_PUBLIC
GrB_Info GxB_Scalar_extractElement_UINT8    // x = s
(
    uint8_t *x,                     // user scalar extracted
    const GxB_Scalar s              // GxB_Scalar to extract an entry from
) ;

GB_PUBLIC
GrB_Info GxB_Scalar_extractElement_INT16    // x = s
(
    int16_t *x,                     // user scalar extracted
    const GxB_Scalar s              // GxB_Scalar to extract an entry from
) ;

GB_PUBLIC
GrB_Info GxB_Scalar_extractElement_UINT16   // x = s
(
    uint16_t *x,                    // user scalar extracted
    const GxB_Scalar s              // GxB_Scalar to extract an entry from
) ;

GB_PUBLIC
GrB_Info GxB_Scalar_extractElement_INT32    // x = s
(
    int32_t *x,                     // user scalar extracted
    const GxB_Scalar s              // GxB_Scalar to extract an entry from
) ;

GB_PUBLIC
GrB_Info GxB_Scalar_extractElement_UINT32   // x = s
(
    uint32_t *x,                    // user scalar extracted
    const GxB_Scalar s              // GxB_Scalar to extract an entry from
) ;

GB_PUBLIC
GrB_Info GxB_Scalar_extractElement_INT64    // x = s
(
    int64_t *x,                     // user scalar extracted
    const GxB_Scalar s              // GxB_Scalar to extract an entry from
) ;

GB_PUBLIC
GrB_Info GxB_Scalar_extractElement_UINT64   // x = s
(
    uint64_t *x,                    // user scalar extracted
    const GxB_Scalar s              // GxB_Scalar to extract an entry from
) ;

GB_PUBLIC
GrB_Info GxB_Scalar_extractElement_FP32     // x = s
(
    float *x,                       // user scalar extracted
    const GxB_Scalar s              // GxB_Scalar to extract an entry from
) ;

GB_PUBLIC
GrB_Info GxB_Scalar_extractElement_FP64     // x = s
(
    double *x,                      // user scalar extracted
    const GxB_Scalar s              // GxB_Scalar to extract an entry from
) ;

GB_PUBLIC
GrB_Info GxB_Scalar_extractElement_FC32     // x = s
(
    GxB_FC32_t *x,                  // user scalar extracted
    const GxB_Scalar s              // GxB_Scalar to extract an entry from
) ;

GB_PUBLIC
GrB_Info GxB_Scalar_extractElement_FC64     // x = s
(
    GxB_FC64_t *x,                  // user scalar extracted
    const GxB_Scalar s              // GxB_Scalar to extract an entry from
) ;

GB_PUBLIC
GrB_Info GxB_Scalar_extractElement_UDT      // x = s
(
    void *x,                        // user scalar extracted
    const GxB_Scalar s              // GxB_Scalar to extract an entry from
) ;

// Type-generic version:  x can be a pointer to any supported C type or void *
// for a user-defined type.

/*

GB_PUBLIC
GrB_Info GxB_Scalar_extractElement  // x = s
(
    <type> *x,                      // user scalar extracted
    const GxB_Scalar s              // GxB_Scalar to extract an entry from
) ;

*/

#if GxB_STDC_VERSION >= 201112L
#define GxB_Scalar_extractElement(x,s)  \
    _Generic ((x), GB_(*, GxB, Scalar_extractElement)) (x, s)
#endif

//==============================================================================
// GrB_Vector: a GraphBLAS vector
//==============================================================================

typedef struct GB_Vector_opaque *GrB_Vector ;

// These methods create, free, copy, and clear a vector.  The size, nvals,
// and type methods return basic information about a vector.

GB_PUBLIC
GrB_Info GrB_Vector_new     // create a new vector with no entries
(
    GrB_Vector *v,          // handle of vector to create
    GrB_Type type,          // type of vector to create
    GrB_Index n             // vector dimension is n-by-1
) ;

GB_PUBLIC
GrB_Info GrB_Vector_dup     // make an exact copy of a vector
(
    GrB_Vector *w,          // handle of output vector to create
    const GrB_Vector u      // input vector to copy
) ;

GB_PUBLIC
GrB_Info GrB_Vector_clear   // clear a vector of all entries;
(                           // type and dimension remain unchanged.
    GrB_Vector v            // vector to clear
) ;

GB_PUBLIC
GrB_Info GrB_Vector_size    // get the dimension of a vector
(
    GrB_Index *n,           // vector dimension is n-by-1
    const GrB_Vector v      // vector to query
) ;

GB_PUBLIC
GrB_Info GrB_Vector_nvals   // get the number of entries in a vector
(
    GrB_Index *nvals,       // vector has nvals entries
    const GrB_Vector v      // vector to query
) ;

GB_PUBLIC
GrB_Info GxB_Vector_type    // get the type of a vector
(
    GrB_Type *type,         // returns the type of the vector
    const GrB_Vector v      // vector to query
) ;

GB_PUBLIC
GrB_Info GrB_Vector_free    // free a vector
(
    GrB_Vector *v           // handle of vector to free
) ;

//------------------------------------------------------------------------------
// GrB_Vector_build
//------------------------------------------------------------------------------

// GrB_Vector_build:  w = sparse (I,1,X) in MATLAB notation, but using any
// associative operator to assemble duplicate entries.

GB_PUBLIC
GrB_Info GrB_Vector_build_BOOL      // build a vector from (I,X) tuples
(
    GrB_Vector w,                   // vector to build
    const GrB_Index *I,             // array of row indices of tuples
    const bool *X,                  // array of values of tuples
    GrB_Index nvals,                // number of tuples
    const GrB_BinaryOp dup          // binary function to assemble duplicates
) ;

GB_PUBLIC
GrB_Info GrB_Vector_build_INT8      // build a vector from (I,X) tuples
(
    GrB_Vector w,                   // vector to build
    const GrB_Index *I,             // array of row indices of tuples
    const int8_t *X,                // array of values of tuples
    GrB_Index nvals,                // number of tuples
    const GrB_BinaryOp dup          // binary function to assemble duplicates
) ;

GB_PUBLIC
GrB_Info GrB_Vector_build_UINT8     // build a vector from (I,X) tuples
(
    GrB_Vector w,                   // vector to build
    const GrB_Index *I,             // array of row indices of tuples
    const uint8_t *X,               // array of values of tuples
    GrB_Index nvals,                // number of tuples
    const GrB_BinaryOp dup          // binary function to assemble duplicates
) ;

GB_PUBLIC
GrB_Info GrB_Vector_build_INT16     // build a vector from (I,X) tuples
(
    GrB_Vector w,                   // vector to build
    const GrB_Index *I,             // array of row indices of tuples
    const int16_t *X,               // array of values of tuples
    GrB_Index nvals,                // number of tuples
    const GrB_BinaryOp dup          // binary function to assemble duplicates
) ;

GB_PUBLIC
GrB_Info GrB_Vector_build_UINT16    // build a vector from (I,X) tuples
(
    GrB_Vector w,                   // vector to build
    const GrB_Index *I,             // array of row indices of tuples
    const uint16_t *X,              // array of values of tuples
    GrB_Index nvals,                // number of tuples
    const GrB_BinaryOp dup          // binary function to assemble duplicates
) ;

GB_PUBLIC
GrB_Info GrB_Vector_build_INT32     // build a vector from (I,X) tuples
(
    GrB_Vector w,                   // vector to build
    const GrB_Index *I,             // array of row indices of tuples
    const int32_t *X,               // array of values of tuples
    GrB_Index nvals,                // number of tuples
    const GrB_BinaryOp dup          // binary function to assemble duplicates
) ;

GB_PUBLIC
GrB_Info GrB_Vector_build_UINT32    // build a vector from (I,X) tuples
(
    GrB_Vector w,                   // vector to build
    const GrB_Index *I,             // array of row indices of tuples
    const uint32_t *X,              // array of values of tuples
    GrB_Index nvals,                // number of tuples
    const GrB_BinaryOp dup          // binary function to assemble duplicates
) ;

GB_PUBLIC
GrB_Info GrB_Vector_build_INT64     // build a vector from (I,X) tuples
(
    GrB_Vector w,                   // vector to build
    const GrB_Index *I,             // array of row indices of tuples
    const int64_t *X,               // array of values of tuples
    GrB_Index nvals,                // number of tuples
    const GrB_BinaryOp dup          // binary function to assemble duplicates
) ;

GB_PUBLIC
GrB_Info GrB_Vector_build_UINT64    // build a vector from (I,X) tuples
(
    GrB_Vector w,                   // vector to build
    const GrB_Index *I,             // array of row indices of tuples
    const uint64_t *X,              // array of values of tuples
    GrB_Index nvals,                // number of tuples
    const GrB_BinaryOp dup          // binary function to assemble duplicates
) ;

GB_PUBLIC
GrB_Info GrB_Vector_build_FP32      // build a vector from (I,X) tuples
(
    GrB_Vector w,                   // vector to build
    const GrB_Index *I,             // array of row indices of tuples
    const float *X,                 // array of values of tuples
    GrB_Index nvals,                // number of tuples
    const GrB_BinaryOp dup          // binary function to assemble duplicates
) ;

GB_PUBLIC
GrB_Info GrB_Vector_build_FP64      // build a vector from (I,X) tuples
(
    GrB_Vector w,                   // vector to build
    const GrB_Index *I,             // array of row indices of tuples
    const double *X,                // array of values of tuples
    GrB_Index nvals,                // number of tuples
    const GrB_BinaryOp dup          // binary function to assemble duplicates
) ;

GB_PUBLIC
GrB_Info GxB_Vector_build_FC32      // build a vector from (I,X) tuples
(
    GrB_Vector w,                   // vector to build
    const GrB_Index *I,             // array of row indices of tuples
    const GxB_FC32_t *X,            // array of values of tuples
    GrB_Index nvals,                // number of tuples
    const GrB_BinaryOp dup          // binary function to assemble duplicates
) ;

GB_PUBLIC
GrB_Info GxB_Vector_build_FC64      // build a vector from (I,X) tuples
(
    GrB_Vector w,                   // vector to build
    const GrB_Index *I,             // array of row indices of tuples
    const GxB_FC64_t *X,            // array of values of tuples
    GrB_Index nvals,                // number of tuples
    const GrB_BinaryOp dup          // binary function to assemble duplicates
) ;

GB_PUBLIC
GrB_Info GrB_Vector_build_UDT       // build a vector from (I,X) tuples
(
    GrB_Vector w,                   // vector to build
    const GrB_Index *I,             // array of row indices of tuples
    const void *X,                  // array of values of tuples
    GrB_Index nvals,                // number of tuples
    const GrB_BinaryOp dup          // binary function to assemble duplicates
) ;

// Type-generic version:  X can be a pointer to any supported C type or void *
// for a user-defined type.

/*

GB_PUBLIC
GrB_Info GrB_Vector_build           // build a vector from (I,X) tuples
(
    GrB_Vector w,                   // vector to build
    const GrB_Index *I,             // array of row indices of tuples
    const <type> *X,                // array of values of tuples
    GrB_Index nvals,                // number of tuples
    const GrB_BinaryOp dup          // binary function to assemble duplicates
) ;

*/

#if GxB_STDC_VERSION >= 201112L
#define GrB_Vector_build(w,I,X,nvals,dup)           \
    _Generic ((X), GB_(*, GrB, Vector_build))       \
    (w, I, ((const void *) (X)), nvals, dup)
#endif

//------------------------------------------------------------------------------
// GrB_Vector_setElement
//------------------------------------------------------------------------------

// Set a single scalar in a vector, w(i) = x, typecasting from the type of x to
// the type of w as needed.

GB_PUBLIC
GrB_Info GrB_Vector_setElement_BOOL     // w(i) = x
(
    GrB_Vector w,                       // vector to modify
    bool x,                             // scalar to assign to w(i)
    GrB_Index i                         // row index
) ;

GB_PUBLIC
GrB_Info GrB_Vector_setElement_INT8     // w(i) = x
(
    GrB_Vector w,                       // vector to modify
    int8_t x,                           // scalar to assign to w(i)
    GrB_Index i                         // row index
) ;

GB_PUBLIC
GrB_Info GrB_Vector_setElement_UINT8    // w(i) = x
(
    GrB_Vector w,                       // vector to modify
    uint8_t x,                          // scalar to assign to w(i)
    GrB_Index i                         // row index
) ;

GB_PUBLIC
GrB_Info GrB_Vector_setElement_INT16    // w(i) = x
(
    GrB_Vector w,                       // vector to modify
    int16_t x,                          // scalar to assign to w(i)
    GrB_Index i                         // row index
) ;

GB_PUBLIC
GrB_Info GrB_Vector_setElement_UINT16   // w(i) = x
(
    GrB_Vector w,                       // vector to modify
    uint16_t x,                         // scalar to assign to w(i)
    GrB_Index i                         // row index
) ;

GB_PUBLIC
GrB_Info GrB_Vector_setElement_INT32    // w(i) = x
(
    GrB_Vector w,                       // vector to modify
    int32_t x,                          // scalar to assign to w(i)
    GrB_Index i                         // row index
) ;

GB_PUBLIC
GrB_Info GrB_Vector_setElement_UINT32   // w(i) = x
(
    GrB_Vector w,                       // vector to modify
    uint32_t x,                         // scalar to assign to w(i)
    GrB_Index i                         // row index
) ;

GB_PUBLIC
GrB_Info GrB_Vector_setElement_INT64    // w(i) = x
(
    GrB_Vector w,                       // vector to modify
    int64_t x,                          // scalar to assign to w(i)
    GrB_Index i                         // row index
) ;

GB_PUBLIC
GrB_Info GrB_Vector_setElement_UINT64   // w(i) = x
(
    GrB_Vector w,                       // vector to modify
    uint64_t x,                         // scalar to assign to w(i)
    GrB_Index i                         // row index
) ;

GB_PUBLIC
GrB_Info GrB_Vector_setElement_FP32     // w(i) = x
(
    GrB_Vector w,                       // vector to modify
    float x,                            // scalar to assign to w(i)
    GrB_Index i                         // row index
) ;

GB_PUBLIC
GrB_Info GrB_Vector_setElement_FP64     // w(i) = x
(
    GrB_Vector w,                       // vector to modify
    double x,                           // scalar to assign to w(i)
    GrB_Index i                         // row index
) ;

GB_PUBLIC
GrB_Info GxB_Vector_setElement_FC32     // w(i) = x
(
    GrB_Vector w,                       // vector to modify
    GxB_FC32_t x,                       // scalar to assign to w(i)
    GrB_Index i                         // row index
) ;

GB_PUBLIC
GrB_Info GxB_Vector_setElement_FC64     // w(i) = x
(
    GrB_Vector w,                       // vector to modify
    GxB_FC64_t x,                       // scalar to assign to w(i)
    GrB_Index i                         // row index
) ;

GB_PUBLIC
GrB_Info GrB_Vector_setElement_UDT      // w(i) = x
(
    GrB_Vector w,                       // vector to modify
    void *x,                            // scalar to assign to w(i)
    GrB_Index i                         // row index
) ;

// Type-generic version:  x can be any supported C type or void * for a
// user-defined type.

/*

GB_PUBLIC
GrB_Info GrB_Vector_setElement          // w(i) = x
(
    GrB_Vector w,                       // vector to modify
    <type> x,                           // scalar to assign to w(i)
    GrB_Index i                         // row index
) ;

*/

#if GxB_STDC_VERSION >= 201112L
#define GrB_Vector_setElement(w,x,i)    \
    _Generic ((x), GB_(, GrB, Vector_setElement)) (w, x, i)
#endif

//------------------------------------------------------------------------------
// GrB_Vector_extractElement
//------------------------------------------------------------------------------

// Extract a single entry from a vector, x = v(i), typecasting from the type of
// v to the type of x as needed.

GB_PUBLIC
GrB_Info GrB_Vector_extractElement_BOOL     // x = v(i)
(
    bool *x,                        // scalar extracted
    const GrB_Vector v,             // vector to extract an entry from
    GrB_Index i                     // row index
) ;

GB_PUBLIC
GrB_Info GrB_Vector_extractElement_INT8     // x = v(i)
(
    int8_t *x,                      // scalar extracted
    const GrB_Vector v,             // vector to extract an entry from
    GrB_Index i                     // row index
) ;

GB_PUBLIC
GrB_Info GrB_Vector_extractElement_UINT8    // x = v(i)
(
    uint8_t *x,                     // scalar extracted
    const GrB_Vector v,             // vector to extract an entry from
    GrB_Index i                     // row index
) ;

GB_PUBLIC
GrB_Info GrB_Vector_extractElement_INT16    // x = v(i)
(
    int16_t *x,                     // scalar extracted
    const GrB_Vector v,             // vector to extract an entry from
    GrB_Index i                     // row index
) ;

GB_PUBLIC
GrB_Info GrB_Vector_extractElement_UINT16   // x = v(i)
(
    uint16_t *x,                    // scalar extracted
    const GrB_Vector v,             // vector to extract an entry from
    GrB_Index i                     // row index
) ;

GB_PUBLIC
GrB_Info GrB_Vector_extractElement_INT32    // x = v(i)
(
    int32_t *x,                     // scalar extracted
    const GrB_Vector v,             // vector to extract an entry from
    GrB_Index i                     // row index
) ;

GB_PUBLIC
GrB_Info GrB_Vector_extractElement_UINT32   // x = v(i)
(
    uint32_t *x,                    // scalar extracted
    const GrB_Vector v,             // vector to extract an entry from
    GrB_Index i                     // row index
) ;

GB_PUBLIC
GrB_Info GrB_Vector_extractElement_INT64    // x = v(i)
(
    int64_t *x,                     // scalar extracted
    const GrB_Vector v,             // vector to extract an entry from
    GrB_Index i                     // row index
) ;

GB_PUBLIC
GrB_Info GrB_Vector_extractElement_UINT64   // x = v(i)
(
    uint64_t *x,                    // scalar extracted
    const GrB_Vector v,             // vector to extract an entry from
    GrB_Index i                     // row index
) ;

GB_PUBLIC
GrB_Info GrB_Vector_extractElement_FP32     // x = v(i)
(
    float *x,                       // scalar extracted
    const GrB_Vector v,             // vector to extract an entry from
    GrB_Index i                     // row index
) ;

GB_PUBLIC
GrB_Info GrB_Vector_extractElement_FP64     // x = v(i)
(
    double *x,                      // scalar extracted
    const GrB_Vector v,             // vector to extract an entry from
    GrB_Index i                     // row index
) ;

GB_PUBLIC
GrB_Info GxB_Vector_extractElement_FC32     // x = v(i)
(
    GxB_FC32_t *x,                  // scalar extracted
    const GrB_Vector v,             // vector to extract an entry from
    GrB_Index i                     // row index
) ;

GB_PUBLIC
GrB_Info GxB_Vector_extractElement_FC64     // x = v(i)
(
    GxB_FC64_t *x,                  // scalar extracted
    const GrB_Vector v,             // vector to extract an entry from
    GrB_Index i                     // row index
) ;

GB_PUBLIC
GrB_Info GrB_Vector_extractElement_UDT      // x = v(i)
(
    void *x,                        // scalar extracted
    const GrB_Vector v,             // vector to extract an entry from
    GrB_Index i                     // row index
) ;

// Type-generic version:  x can be a pointer to any supported C type or void *
// for a user-defined type.

/*

GB_PUBLIC
GrB_Info GrB_Vector_extractElement  // x = v(i)
(
    <type> *x,                      // scalar extracted
    const GrB_Vector v,             // vector to extract an entry from
    GrB_Index i                     // row index
) ;

*/

#if GxB_STDC_VERSION >= 201112L
#define GrB_Vector_extractElement(x,v,i)    \
    _Generic ((x), GB_(*, GrB, Vector_extractElement)) (x, v, i)
#endif

//------------------------------------------------------------------------------
// GrB_Vector_removeElement
//------------------------------------------------------------------------------

// GrB_Vector_removeElement (v,i) removes the element v(i) from the vector v.

GB_PUBLIC
GrB_Info GrB_Vector_removeElement
(
    GrB_Vector v,                   // vector to remove an element from
    GrB_Index i                     // index
) ;

//------------------------------------------------------------------------------
// GrB_Vector_extractTuples
//------------------------------------------------------------------------------

// Extracts all tuples from a vector, like [I,~,X] = find (v) in MATLAB.  If
// any parameter I and/or X is NULL, then that component is not extracted.  For
// example, to extract just the row indices, pass I as non-NULL, and X as NULL.
// This is like [I,~,~] = find (v) in MATLAB.

GB_PUBLIC
GrB_Info GrB_Vector_extractTuples_BOOL      // [I,~,X] = find (v)
(
    GrB_Index *I,               // array for returning row indices of tuples
    bool *X,                    // array for returning values of tuples
    GrB_Index *nvals,           // I, X size on input; # tuples on output
    const GrB_Vector v          // vector to extract tuples from
) ;

GB_PUBLIC
GrB_Info GrB_Vector_extractTuples_INT8      // [I,~,X] = find (v)
(
    GrB_Index *I,               // array for returning row indices of tuples
    int8_t *X,                  // array for returning values of tuples
    GrB_Index *nvals,           // I, X size on input; # tuples on output
    const GrB_Vector v          // vector to extract tuples from
) ;

GB_PUBLIC
GrB_Info GrB_Vector_extractTuples_UINT8     // [I,~,X] = find (v)
(
    GrB_Index *I,               // array for returning row indices of tuples
    uint8_t *X,                 // array for returning values of tuples
    GrB_Index *nvals,           // I, X size on input; # tuples on output
    const GrB_Vector v          // vector to extract tuples from
) ;

GB_PUBLIC
GrB_Info GrB_Vector_extractTuples_INT16     // [I,~,X] = find (v)
(
    GrB_Index *I,               // array for returning row indices of tuples
    int16_t *X,                 // array for returning values of tuples
    GrB_Index *nvals,           // I, X size on input; # tuples on output
    const GrB_Vector v          // vector to extract tuples from
) ;

GB_PUBLIC
GrB_Info GrB_Vector_extractTuples_UINT16    // [I,~,X] = find (v)
(
    GrB_Index *I,               // array for returning row indices of tuples
    uint16_t *X,                // array for returning values of tuples
    GrB_Index *nvals,           // I, X size on input; # tuples on output
    const GrB_Vector v          // vector to extract tuples from
) ;

GB_PUBLIC
GrB_Info GrB_Vector_extractTuples_INT32     // [I,~,X] = find (v)
(
    GrB_Index *I,               // array for returning row indices of tuples
    int32_t *X,                 // array for returning values of tuples
    GrB_Index *nvals,           // I, X size on input; # tuples on output
    const GrB_Vector v          // vector to extract tuples from
) ;

GB_PUBLIC
GrB_Info GrB_Vector_extractTuples_UINT32    // [I,~,X] = find (v)
(
    GrB_Index *I,               // array for returning row indices of tuples
    uint32_t *X,                // array for returning values of tuples
    GrB_Index *nvals,           // I, X size on input; # tuples on output
    const GrB_Vector v          // vector to extract tuples from
) ;

GB_PUBLIC
GrB_Info GrB_Vector_extractTuples_INT64     // [I,~,X] = find (v)
(
    GrB_Index *I,               // array for returning row indices of tuples
    int64_t *X,                 // array for returning values of tuples
    GrB_Index *nvals,           // I, X size on input; # tuples on output
    const GrB_Vector v          // vector to extract tuples from
) ;

GB_PUBLIC
GrB_Info GrB_Vector_extractTuples_UINT64    // [I,~,X] = find (v)
(
    GrB_Index *I,               // array for returning row indices of tuples
    uint64_t *X,                // array for returning values of tuples
    GrB_Index *nvals,           // I, X size on input; # tuples on output
    const GrB_Vector v          // vector to extract tuples from
) ;

GB_PUBLIC
GrB_Info GrB_Vector_extractTuples_FP32      // [I,~,X] = find (v)
(
    GrB_Index *I,               // array for returning row indices of tuples
    float *X,                   // array for returning values of tuples
    GrB_Index *nvals,           // I, X size on input; # tuples on output
    const GrB_Vector v          // vector to extract tuples from
) ;

GB_PUBLIC
GrB_Info GrB_Vector_extractTuples_FP64      // [I,~,X] = find (v)
(
    GrB_Index *I,               // array for returning row indices of tuples
    double *X,                  // array for returning values of tuples
    GrB_Index *nvals,           // I, X size on input; # tuples on output
    const GrB_Vector v          // vector to extract tuples from
) ;

GB_PUBLIC
GrB_Info GxB_Vector_extractTuples_FC32      // [I,~,X] = find (v)
(
    GrB_Index *I,               // array for returning row indices of tuples
    GxB_FC32_t *X,              // array for returning values of tuples
    GrB_Index *nvals,           // I, X size on input; # tuples on output
    const GrB_Vector v          // vector to extract tuples from
) ;

GB_PUBLIC
GrB_Info GxB_Vector_extractTuples_FC64      // [I,~,X] = find (v)
(
    GrB_Index *I,               // array for returning row indices of tuples
    GxB_FC64_t *X,              // array for returning values of tuples
    GrB_Index *nvals,           // I, X size on input; # tuples on output
    const GrB_Vector v          // vector to extract tuples from
) ;

GB_PUBLIC
GrB_Info GrB_Vector_extractTuples_UDT       // [I,~,X] = find (v)
(
    GrB_Index *I,               // array for returning row indices of tuples
    void *X,                    // array for returning values of tuples
    GrB_Index *nvals,           // I, X size on input; # tuples on output
    const GrB_Vector v          // vector to extract tuples from
) ;

// Type-generic version:  X can be a pointer to any supported C type or void *
// for a user-defined type.

/*

GB_PUBLIC
GrB_Info GrB_Vector_extractTuples           // [I,~,X] = find (v)
(
    GrB_Index *I,               // array for returning row indices of tuples
    <type> *X,                  // array for returning values of tuples
    GrB_Index *nvals,           // I, X size on input; # tuples on output
    const GrB_Vector v          // vector to extract tuples from
) ;

*/

#if GxB_STDC_VERSION >= 201112L
#define GrB_Vector_extractTuples(I,X,nvals,v)   \
    _Generic ((X), GB_(*, GrB, Vector_extractTuples)) (I, X, nvals, v)
#endif

//==============================================================================
// GrB_Matrix: a GraphBLAS matrix
//==============================================================================

typedef struct GB_Matrix_opaque *GrB_Matrix ;

// These methods create, free, copy, and clear a matrix.  The nrows, ncols,
// nvals, and type methods return basic information about a matrix.

GB_PUBLIC
GrB_Info GrB_Matrix_new     // create a new matrix with no entries
(
    GrB_Matrix *A,          // handle of matrix to create
    GrB_Type type,          // type of matrix to create
    GrB_Index nrows,        // matrix dimension is nrows-by-ncols
    GrB_Index ncols
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_dup     // make an exact copy of a matrix
(
    GrB_Matrix *C,          // handle of output matrix to create
    const GrB_Matrix A      // input matrix to copy
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_clear   // clear a matrix of all entries;
(                           // type and dimensions remain unchanged
    GrB_Matrix A            // matrix to clear
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_nrows   // get the number of rows of a matrix
(
    GrB_Index *nrows,       // matrix has nrows rows
    const GrB_Matrix A      // matrix to query
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_ncols   // get the number of columns of a matrix
(
    GrB_Index *ncols,       // matrix has ncols columns
    const GrB_Matrix A      // matrix to query
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_nvals   // get the number of entries in a matrix
(
    GrB_Index *nvals,       // matrix has nvals entries
    const GrB_Matrix A      // matrix to query
) ;

GB_PUBLIC
GrB_Info GxB_Matrix_type    // get the type of a matrix
(
    GrB_Type *type,         // returns the type of the matrix
    const GrB_Matrix A      // matrix to query
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_free    // free a matrix
(
    GrB_Matrix *A           // handle of matrix to free
) ;

//------------------------------------------------------------------------------
// GrB_Matrix_build
//------------------------------------------------------------------------------

// GrB_Matrix_build:  C = sparse (I,J,X) in MATLAB notation, but using any
// associative operator to assemble duplicate entries.

GB_PUBLIC
GrB_Info GrB_Matrix_build_BOOL      // build a matrix from (I,J,X) tuples
(
    GrB_Matrix C,                   // matrix to build
    const GrB_Index *I,             // array of row indices of tuples
    const GrB_Index *J,             // array of column indices of tuples
    const bool *X,                  // array of values of tuples
    GrB_Index nvals,                // number of tuples
    const GrB_BinaryOp dup          // binary function to assemble duplicates
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_build_INT8      // build a matrix from (I,J,X) tuples
(
    GrB_Matrix C,                   // matrix to build
    const GrB_Index *I,             // array of row indices of tuples
    const GrB_Index *J,             // array of column indices of tuples
    const int8_t *X,                // array of values of tuples
    GrB_Index nvals,                // number of tuples
    const GrB_BinaryOp dup          // binary function to assemble duplicates
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_build_UINT8     // build a matrix from (I,J,X) tuples
(
    GrB_Matrix C,                   // matrix to build
    const GrB_Index *I,             // array of row indices of tuples
    const GrB_Index *J,             // array of column indices of tuples
    const uint8_t *X,               // array of values of tuples
    GrB_Index nvals,                // number of tuples
    const GrB_BinaryOp dup          // binary function to assemble duplicates
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_build_INT16     // build a matrix from (I,J,X) tuples
(
    GrB_Matrix C,                   // matrix to build
    const GrB_Index *I,             // array of row indices of tuples
    const GrB_Index *J,             // array of column indices of tuples
    const int16_t *X,               // array of values of tuples
    GrB_Index nvals,                // number of tuples
    const GrB_BinaryOp dup          // binary function to assemble duplicates
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_build_UINT16    // build a matrix from (I,J,X) tuples
(
    GrB_Matrix C,                   // matrix to build
    const GrB_Index *I,             // array of row indices of tuples
    const GrB_Index *J,             // array of column indices of tuples
    const uint16_t *X,              // array of values of tuples
    GrB_Index nvals,                // number of tuples
    const GrB_BinaryOp dup          // binary function to assemble duplicates
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_build_INT32     // build a matrix from (I,J,X) tuples
(
    GrB_Matrix C,                   // matrix to build
    const GrB_Index *I,             // array of row indices of tuples
    const GrB_Index *J,             // array of column indices of tuples
    const int32_t *X,               // array of values of tuples
    GrB_Index nvals,                // number of tuples
    const GrB_BinaryOp dup          // binary function to assemble duplicates
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_build_UINT32    // build a matrix from (I,J,X) tuples
(
    GrB_Matrix C,                   // matrix to build
    const GrB_Index *I,             // array of row indices of tuples
    const GrB_Index *J,             // array of column indices of tuples
    const uint32_t *X,              // array of values of tuples
    GrB_Index nvals,                // number of tuples
    const GrB_BinaryOp dup          // binary function to assemble duplicates
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_build_INT64     // build a matrix from (I,J,X) tuples
(
    GrB_Matrix C,                   // matrix to build
    const GrB_Index *I,             // array of row indices of tuples
    const GrB_Index *J,             // array of column indices of tuples
    const int64_t *X,               // array of values of tuples
    GrB_Index nvals,                // number of tuples
    const GrB_BinaryOp dup          // binary function to assemble duplicates
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_build_UINT64    // build a matrix from (I,J,X) tuples
(
    GrB_Matrix C,                   // matrix to build
    const GrB_Index *I,             // array of row indices of tuples
    const GrB_Index *J,             // array of column indices of tuples
    const uint64_t *X,              // array of values of tuples
    GrB_Index nvals,                // number of tuples
    const GrB_BinaryOp dup          // binary function to assemble duplicates
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_build_FP32      // build a matrix from (I,J,X) tuples
(
    GrB_Matrix C,                   // matrix to build
    const GrB_Index *I,             // array of row indices of tuples
    const GrB_Index *J,             // array of column indices of tuples
    const float *X,                 // array of values of tuples
    GrB_Index nvals,                // number of tuples
    const GrB_BinaryOp dup          // binary function to assemble duplicates
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_build_FP64      // build a matrix from (I,J,X) tuples
(
    GrB_Matrix C,                   // matrix to build
    const GrB_Index *I,             // array of row indices of tuples
    const GrB_Index *J,             // array of column indices of tuples
    const double *X,                // array of values of tuples
    GrB_Index nvals,                // number of tuples
    const GrB_BinaryOp dup          // binary function to assemble duplicates
) ;

GB_PUBLIC
GrB_Info GxB_Matrix_build_FC32      // build a matrix from (I,J,X) tuples
(
    GrB_Matrix C,                   // matrix to build
    const GrB_Index *I,             // array of row indices of tuples
    const GrB_Index *J,             // array of column indices of tuples
    const GxB_FC32_t *X,            // array of values of tuples
    GrB_Index nvals,                // number of tuples
    const GrB_BinaryOp dup          // binary function to assemble duplicates
) ;

GB_PUBLIC
GrB_Info GxB_Matrix_build_FC64      // build a matrix from (I,J,X) tuples
(
    GrB_Matrix C,                   // matrix to build
    const GrB_Index *I,             // array of row indices of tuples
    const GrB_Index *J,             // array of column indices of tuples
    const GxB_FC64_t *X,            // array of values of tuples
    GrB_Index nvals,                // number of tuples
    const GrB_BinaryOp dup          // binary function to assemble duplicates
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_build_UDT       // build a matrix from (I,J,X) tuples
(
    GrB_Matrix C,                   // matrix to build
    const GrB_Index *I,             // array of row indices of tuples
    const GrB_Index *J,             // array of column indices of tuples
    const void *X,                  // array of values of tuples
    GrB_Index nvals,                // number of tuples
    const GrB_BinaryOp dup          // binary function to assemble duplicates
) ;

// Type-generic version:  X can be a pointer to any supported C type or void *
// for a user-defined type.

/*

GB_PUBLIC
GrB_Info GrB_Matrix_build           // build a matrix from (I,J,X) tuples
(
    GrB_Matrix C,                   // matrix to build
    const GrB_Index *I,             // array of row indices of tuples
    const GrB_Index *J,             // array of column indices of tuples
    const <type> *X,                // array of values of tuples
    GrB_Index nvals,                // number of tuples
    const GrB_BinaryOp dup          // binary function to assemble duplicates
) ;

*/

#if GxB_STDC_VERSION >= 201112L
#define GrB_Matrix_build(C,I,J,X,nvals,dup)         \
    _Generic ((X), GB_(*, GrB, Matrix_build))       \
    (C, I, J, ((const void *) (X)), nvals, dup)
#endif

//------------------------------------------------------------------------------
// GrB_Matrix_setElement
//------------------------------------------------------------------------------

// Set a single entry in a matrix, C(i,j) = x in MATLAB notation, typecasting
// from the type of x to the type of C, as needed.

GB_PUBLIC
GrB_Info GrB_Matrix_setElement_BOOL     // C (i,j) = x
(
    GrB_Matrix C,                       // matrix to modify
    bool x,                             // scalar to assign to C(i,j)
    GrB_Index i,                        // row index
    GrB_Index j                         // column index
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_setElement_INT8     // C (i,j) = x
(
    GrB_Matrix C,                       // matrix to modify
    int8_t x,                           // scalar to assign to C(i,j)
    GrB_Index i,                        // row index
    GrB_Index j                         // column index
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_setElement_UINT8    // C (i,j) = x
(
    GrB_Matrix C,                       // matrix to modify
    uint8_t x,                          // scalar to assign to C(i,j)
    GrB_Index i,                        // row index
    GrB_Index j                         // column index
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_setElement_INT16    // C (i,j) = x
(
    GrB_Matrix C,                       // matrix to modify
    int16_t x,                          // scalar to assign to C(i,j)
    GrB_Index i,                        // row index
    GrB_Index j                         // column index
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_setElement_UINT16   // C (i,j) = x
(
    GrB_Matrix C,                       // matrix to modify
    uint16_t x,                         // scalar to assign to C(i,j)
    GrB_Index i,                        // row index
    GrB_Index j                         // column index
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_setElement_INT32    // C (i,j) = x
(
    GrB_Matrix C,                       // matrix to modify
    int32_t x,                          // scalar to assign to C(i,j)
    GrB_Index i,                        // row index
    GrB_Index j                         // column index
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_setElement_UINT32   // C (i,j) = x
(
    GrB_Matrix C,                       // matrix to modify
    uint32_t x,                         // scalar to assign to C(i,j)
    GrB_Index i,                        // row index
    GrB_Index j                         // column index
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_setElement_INT64    // C (i,j) = x
(
    GrB_Matrix C,                       // matrix to modify
    int64_t x,                          // scalar to assign to C(i,j)
    GrB_Index i,                        // row index
    GrB_Index j                         // column index
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_setElement_UINT64   // C (i,j) = x
(
    GrB_Matrix C,                       // matrix to modify
    uint64_t x,                         // scalar to assign to C(i,j)
    GrB_Index i,                        // row index
    GrB_Index j                         // column index
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_setElement_FP32     // C (i,j) = x
(
    GrB_Matrix C,                       // matrix to modify
    float x,                            // scalar to assign to C(i,j)
    GrB_Index i,                        // row index
    GrB_Index j                         // column index
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_setElement_FP64     // C (i,j) = x
(
    GrB_Matrix C,                       // matrix to modify
    double x,                           // scalar to assign to C(i,j)
    GrB_Index i,                        // row index
    GrB_Index j                         // column index
) ;

GB_PUBLIC
GrB_Info GxB_Matrix_setElement_FC32     // C (i,j) = x
(
    GrB_Matrix C,                       // matrix to modify
    GxB_FC32_t x,                       // scalar to assign to C(i,j)
    GrB_Index i,                        // row index
    GrB_Index j                         // column index
) ;

GB_PUBLIC
GrB_Info GxB_Matrix_setElement_FC64     // C (i,j) = x
(
    GrB_Matrix C,                       // matrix to modify
    GxB_FC64_t x,                       // scalar to assign to C(i,j)
    GrB_Index i,                        // row index
    GrB_Index j                         // column index
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_setElement_UDT      // C (i,j) = x
(
    GrB_Matrix C,                       // matrix to modify
    void *x,                            // scalar to assign to C(i,j)
    GrB_Index i,                        // row index
    GrB_Index j                         // column index
) ;

// Type-generic version:  x can be any supported C type or void * for a
// user-defined type.

/*

GB_PUBLIC
GrB_Info GrB_Matrix_setElement          // C (i,j) = x
(
    GrB_Matrix C,                       // matrix to modify
    <type> x,                           // scalar to assign to C(i,j)
    GrB_Index i,                        // row index
    GrB_Index j                         // column index
) ;

*/

#if GxB_STDC_VERSION >= 201112L
#define GrB_Matrix_setElement(C,x,i,j)                      \
    _Generic ((x), GB_(, GrB, Matrix_setElement)) (C, x, i, j)
#endif

//------------------------------------------------------------------------------
// GrB_Matrix_extractElement
//------------------------------------------------------------------------------

// Extract a single entry from a matrix, x = A(i,j), typecasting from the type
// of A to the type of x, as needed.

GB_PUBLIC
GrB_Info GrB_Matrix_extractElement_BOOL     // x = A(i,j)
(
    bool *x,                            // extracted scalar
    const GrB_Matrix A,                 // matrix to extract a scalar from
    GrB_Index i,                        // row index
    GrB_Index j                         // column index
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_extractElement_INT8     // x = A(i,j)
(
    int8_t *x,                          // extracted scalar
    const GrB_Matrix A,                 // matrix to extract a scalar from
    GrB_Index i,                        // row index
    GrB_Index j                         // column index
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_extractElement_UINT8    // x = A(i,j)
(
    uint8_t *x,                         // extracted scalar
    const GrB_Matrix A,                 // matrix to extract a scalar from
    GrB_Index i,                        // row index
    GrB_Index j                         // column index
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_extractElement_INT16    // x = A(i,j)
(
    int16_t *x,                         // extracted scalar
    const GrB_Matrix A,                 // matrix to extract a scalar from
    GrB_Index i,                        // row index
    GrB_Index j                         // column index
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_extractElement_UINT16   // x = A(i,j)
(
    uint16_t *x,                        // extracted scalar
    const GrB_Matrix A,                 // matrix to extract a scalar from
    GrB_Index i,                        // row index
    GrB_Index j                         // column index
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_extractElement_INT32    // x = A(i,j)
(
    int32_t *x,                         // extracted scalar
    const GrB_Matrix A,                 // matrix to extract a scalar from
    GrB_Index i,                        // row index
    GrB_Index j                         // column index
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_extractElement_UINT32   // x = A(i,j)
(
    uint32_t *x,                        // extracted scalar
    const GrB_Matrix A,                 // matrix to extract a scalar from
    GrB_Index i,                        // row index
    GrB_Index j                         // column index
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_extractElement_INT64    // x = A(i,j)
(
    int64_t *x,                         // extracted scalar
    const GrB_Matrix A,                 // matrix to extract a scalar from
    GrB_Index i,                        // row index
    GrB_Index j                         // column index
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_extractElement_UINT64   // x = A(i,j)
(
    uint64_t *x,                        // extracted scalar
    const GrB_Matrix A,                 // matrix to extract a scalar from
    GrB_Index i,                        // row index
    GrB_Index j                         // column index
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_extractElement_FP32     // x = A(i,j)
(
    float *x,                           // extracted scalar
    const GrB_Matrix A,                 // matrix to extract a scalar from
    GrB_Index i,                        // row index
    GrB_Index j                         // column index
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_extractElement_FP64     // x = A(i,j)
(
    double *x,                          // extracted scalar
    const GrB_Matrix A,                 // matrix to extract a scalar from
    GrB_Index i,                        // row index
    GrB_Index j                         // column index
) ;

GB_PUBLIC
GrB_Info GxB_Matrix_extractElement_FC32     // x = A(i,j)
(
    GxB_FC32_t *x,                      // extracted scalar
    const GrB_Matrix A,                 // matrix to extract a scalar from
    GrB_Index i,                        // row index
    GrB_Index j                         // column index
) ;

GB_PUBLIC
GrB_Info GxB_Matrix_extractElement_FC64     // x = A(i,j)
(
    GxB_FC64_t *x,                      // extracted scalar
    const GrB_Matrix A,                 // matrix to extract a scalar from
    GrB_Index i,                        // row index
    GrB_Index j                         // column index
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_extractElement_UDT      // x = A(i,j)
(
    void *x,                            // extracted scalar
    const GrB_Matrix A,                 // matrix to extract a scalar from
    GrB_Index i,                        // row index
    GrB_Index j                         // column index
) ;

// Type-generic version:  x can be a pointer to any supported C type or void *
// for a user-defined type.

/*

GB_PUBLIC
GrB_Info GrB_Matrix_extractElement      // x = A(i,j)
(
    <type> *x,                          // extracted scalar
    const GrB_Matrix A,                 // matrix to extract a scalar from
    GrB_Index i,                        // row index
    GrB_Index j                         // column index
) ;

*/

#if GxB_STDC_VERSION >= 201112L
#define GrB_Matrix_extractElement(x,A,i,j)  \
    _Generic ((x), GB_(*, GrB, Matrix_extractElement)) (x, A, i, j)
#endif

//------------------------------------------------------------------------------
// GrB_Matrix_removeElement
//------------------------------------------------------------------------------

// GrB_Matrix_removeElement (A,i,j) removes the entry A(i,j) from the matrix A.

GB_PUBLIC
GrB_Info GrB_Matrix_removeElement
(
    GrB_Matrix C,                   // matrix to remove entry from
    GrB_Index i,                    // row index
    GrB_Index j                     // column index
) ;

//------------------------------------------------------------------------------
// GrB_Matrix_extractTuples
//------------------------------------------------------------------------------

// Extracts all tuples from a matrix, like [I,J,X] = find (A) in MATLAB.  If
// any parameter I, J and/or X is NULL, then that component is not extracted.
// For example, to extract just the row and col indices, pass I and J as
// non-NULL, and X as NULL.  This is like [I,J,~] = find (A).

GB_PUBLIC
GrB_Info GrB_Matrix_extractTuples_BOOL      // [I,J,X] = find (A)
(
    GrB_Index *I,               // array for returning row indices of tuples
    GrB_Index *J,               // array for returning col indices of tuples
    bool *X,                    // array for returning values of tuples
    GrB_Index *nvals,           // I,J,X size on input; # tuples on output
    const GrB_Matrix A          // matrix to extract tuples from
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_extractTuples_INT8      // [I,J,X] = find (A)
(
    GrB_Index *I,               // array for returning row indices of tuples
    GrB_Index *J,               // array for returning col indices of tuples
    int8_t *X,                  // array for returning values of tuples
    GrB_Index *nvals,           // I,J,X size on input; # tuples on output
    const GrB_Matrix A          // matrix to extract tuples from
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_extractTuples_UINT8     // [I,J,X] = find (A)
(
    GrB_Index *I,               // array for returning row indices of tuples
    GrB_Index *J,               // array for returning col indices of tuples
    uint8_t *X,                 // array for returning values of tuples
    GrB_Index *nvals,           // I,J,X size on input; # tuples on output
    const GrB_Matrix A          // matrix to extract tuples from
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_extractTuples_INT16     // [I,J,X] = find (A)
(
    GrB_Index *I,               // array for returning row indices of tuples
    GrB_Index *J,               // array for returning col indices of tuples
    int16_t *X,                 // array for returning values of tuples
    GrB_Index *nvals,           // I,J,X size on input; # tuples on output
    const GrB_Matrix A          // matrix to extract tuples from
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_extractTuples_UINT16    // [I,J,X] = find (A)
(
    GrB_Index *I,               // array for returning row indices of tuples
    GrB_Index *J,               // array for returning col indices of tuples
    uint16_t *X,                // array for returning values of tuples
    GrB_Index *nvals,           // I,J,X size on input; # tuples on output
    const GrB_Matrix A          // matrix to extract tuples from
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_extractTuples_INT32     // [I,J,X] = find (A)
(
    GrB_Index *I,               // array for returning row indices of tuples
    GrB_Index *J,               // array for returning col indices of tuples
    int32_t *X,                 // array for returning values of tuples
    GrB_Index *nvals,           // I,J,X size on input; # tuples on output
    const GrB_Matrix A          // matrix to extract tuples from
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_extractTuples_UINT32    // [I,J,X] = find (A)
(
    GrB_Index *I,               // array for returning row indices of tuples
    GrB_Index *J,               // array for returning col indices of tuples
    uint32_t *X,                // array for returning values of tuples
    GrB_Index *nvals,           // I,J,X size on input; # tuples on output
    const GrB_Matrix A          // matrix to extract tuples from
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_extractTuples_INT64     // [I,J,X] = find (A)
(
    GrB_Index *I,               // array for returning row indices of tuples
    GrB_Index *J,               // array for returning col indices of tuples
    int64_t *X,                 // array for returning values of tuples
    GrB_Index *nvals,           // I,J,X size on input; # tuples on output
    const GrB_Matrix A          // matrix to extract tuples from
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_extractTuples_UINT64    // [I,J,X] = find (A)
(
    GrB_Index *I,               // array for returning row indices of tuples
    GrB_Index *J,               // array for returning col indices of tuples
    uint64_t *X,                // array for returning values of tuples
    GrB_Index *nvals,           // I,J,X size on input; # tuples on output
    const GrB_Matrix A          // matrix to extract tuples from
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_extractTuples_FP32      // [I,J,X] = find (A)
(
    GrB_Index *I,               // array for returning row indices of tuples
    GrB_Index *J,               // array for returning col indices of tuples
    float *X,                   // array for returning values of tuples
    GrB_Index *nvals,           // I,J,X size on input; # tuples on output
    const GrB_Matrix A          // matrix to extract tuples from
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_extractTuples_FP64      // [I,J,X] = find (A)
(
    GrB_Index *I,               // array for returning row indices of tuples
    GrB_Index *J,               // array for returning col indices of tuples
    double *X,                  // array for returning values of tuples
    GrB_Index *nvals,           // I,J,X size on input; # tuples on output
    const GrB_Matrix A          // matrix to extract tuples from
) ;

GB_PUBLIC
GrB_Info GxB_Matrix_extractTuples_FC32      // [I,J,X] = find (A)
(
    GrB_Index *I,               // array for returning row indices of tuples
    GrB_Index *J,               // array for returning col indices of tuples
    GxB_FC32_t *X,              // array for returning values of tuples
    GrB_Index *nvals,           // I,J,X size on input; # tuples on output
    const GrB_Matrix A          // matrix to extract tuples from
) ;

GB_PUBLIC
GrB_Info GxB_Matrix_extractTuples_FC64      // [I,J,X] = find (A)
(
    GrB_Index *I,               // array for returning row indices of tuples
    GrB_Index *J,               // array for returning col indices of tuples
    GxB_FC64_t *X,              // array for returning values of tuples
    GrB_Index *nvals,           // I,J,X size on input; # tuples on output
    const GrB_Matrix A          // matrix to extract tuples from
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_extractTuples_UDT       // [I,J,X] = find (A)
(
    GrB_Index *I,               // array for returning row indices of tuples
    GrB_Index *J,               // array for returning col indices of tuples
    void *X,                    // array for returning values of tuples
    GrB_Index *nvals,           // I,J,X size on input; # tuples on output
    const GrB_Matrix A          // matrix to extract tuples from
) ;

// Type-generic version:  X can be a pointer to any supported C type or void *
// for a user-defined type.

/*

GB_PUBLIC
GrB_Info GrB_Matrix_extractTuples           // [I,J,X] = find (A)
(
    GrB_Index *I,               // array for returning row indices of tuples
    GrB_Index *J,               // array for returning col indices of tuples
    <type> *X,                  // array for returning values of tuples
    GrB_Index *nvals,           // I,J,X size on input; # tuples on output
    const GrB_Matrix A          // matrix to extract tuples from
) ;

*/

#if GxB_STDC_VERSION >= 201112L
#define GrB_Matrix_extractTuples(I,J,X,nvals,A) \
    _Generic ((X), GB_(*, GrB, Matrix_extractTuples)) (I, J, X, nvals, A)
#endif

//==============================================================================
// GrB_Descriptor: the GraphBLAS descriptor
//==============================================================================

// The GrB_Descriptor is used to modify the behavior of GraphBLAS operations.
//
// GrB_OUTP: can be GxB_DEFAULT or GrB_REPLACE.  If GrB_REPLACE, then C is
//       cleared after taking part in the accum operation but before the mask.
//       In other words, C<Mask> = accum (C,T) is split into Z = accum(C,T) ;
//       C=0 ; C<Mask> = Z.
//
// GrB_MASK: can be GxB_DEFAULT, GrB_COMP, GrB_STRUCTURE, or set to both
//      GrB_COMP and GrB_STRUCTURE.  If GxB_DEFAULT, the mask is used
//      normally, where Mask(i,j)=1 means C(i,j) can be modified by C<Mask>=Z,
//      and Mask(i,j)=0 means it cannot be modified even if Z(i,j) is has been
//      computed and differs from C(i,j).  If GrB_COMP, this is the same as
//      taking the logical complement of the Mask.  If GrB_STRUCTURE is set,
//      the value of the mask is not considered, just its pattern.  The
//      GrB_COMP and GrB_STRUCTURE settings can be combined.
//
// GrB_INP0: can be GxB_DEFAULT or GrB_TRAN.  If GxB_DEFAULT, the first input
//      is used as-is.  If GrB_TRAN, it is transposed.  Only matrices are
//      transposed this way.  Vectors are never transposed via the
//      GrB_Descriptor.
//
// GrB_INP1: the same as GrB_INP0 but for the second input
//
// GxB_NTHREADS: the maximum number of threads to use in the current method.
//      If <= GxB_DEFAULT (which is zero), then the number of threads is
//      determined automatically.  This is the default value.
//
// GxB_CHUNK: an integer parameter that determines the number of threads to use
//      for a small problem.  If w is the work to be performed, and chunk is
//      the value of this parameter, then the # of threads is limited to floor
//      (w/chunk).  The default chunk is currently 64K, but this may change in
//      the future.  If chunk is set to <= GxB_DEFAULT (that is, zero), the
//      default is used.
//
// GxB_AxB_METHOD: this is a hint to SuiteSparse:GraphBLAS on which algorithm
//      it should use to compute C=A*B, in GrB_mxm, GrB_mxv, and GrB_vxm.
//      SuiteSparse:GraphBLAS has four different heuristics, and the default
//      method (GxB_DEFAULT) selects between them automatically.  The complete
//      rule is in the User Guide.  The brief discussion here assumes all
//      matrices are stored by column.  All methods compute the same result,
//      except that floating-point roundoff may differ when working on
//      floating-point data types.
//
//      GxB_AxB_SAXPY:  C(:,j)=A*B(:,j) is computed using a mix of Gustavson
//          and Hash methods.  Each task in the parallel computation makes its
//          own decision between these two methods, via a heuristic.
//
//      GxB_AxB_GUSTAVSON:  This is the same as GxB_AxB_SAXPY, except that
//          every task uses Gustavon's method, computing C(:,j)=A*B(:,j) via a
//          gather/scatter workspace of size equal to the number of rows of A.
//          Very good general-purpose method, but sometimes the workspace can
//          be too large when many threads are used.
//
//      GxB_AxB_HASH: This is the same as GxB_AxB_SAXPY, except that every
//          task uses the Hash method.  It is very good for hypersparse
//          matrices and uses very little workspace, and so it scales well to
//          many threads.
//
//      GxB_AxB_DOT: computes C(i,j) = A(:,i)'*B(:,j), for each entry C(i,j).
//          A very specialized method that works well only if the mask is
//          present, very sparse, and not complemented, or when C is a dense
//          vector or matrix, or when C is small.
//
// GxB_SORT: GrB_mxm and other methods may return a matrix in a 'jumbled'
//      state, with indices out of order.  The sort is left pending.  Some
//      methods can tolerate jumbled matrices on input, so this can be faster.
//      However, in some cases, it can be faster for GrB_mxm to sort its output
//      as it is computed.  With GxB_SORT set to GxB_DEFAULT, the sort is left
//      pending.  With GxB_SORT set to a nonzero value, GrB_mxm typically sorts
//      the resulting matrix C (but not always; this is just a hint).  If
//      GrB_init is called with GrB_BLOCKING mode, the sort will always be
//      done, and this setting has no effect.

// The following are enumerated values in both the GrB_Desc_Field and the
// GxB_Option_Field for global options.  They are defined with the same integer
// value for both enums, so the user can use them for both.
#define GxB_NTHREADS 5
#define GxB_CHUNK 7

// GPU control (DRAFT: in progress, do not use)
#define GxB_GPU_CONTROL 21
#define GxB_GPU_CHUNK   22

typedef enum
{
    GrB_OUTP = 0,   // descriptor for output of a method
    GrB_MASK = 1,   // descriptor for the mask input of a method
    GrB_INP0 = 2,   // descriptor for the first input of a method
    GrB_INP1 = 3,   // descriptor for the second input of a method

    GxB_DESCRIPTOR_NTHREADS = GxB_NTHREADS,     // max number of threads to use.
                    // If <= GxB_DEFAULT, then GraphBLAS selects the number
                    // of threads automatically.

    GxB_DESCRIPTOR_CHUNK = GxB_CHUNK,   // chunk size for small problems.
                    // If <= GxB_DEFAULT, then the default is used.

    // GPU control (DRAFT: in progress, do not use)
    GxB_DESCRIPTOR_GPU_CONTROL = GxB_GPU_CONTROL,
    GxB_DESCRIPTOR_GPU_CHUNK   = GxB_GPU_CHUNK,

    GxB_AxB_METHOD = 1000,  // descriptor for selecting C=A*B algorithm
    GxB_SORT = 35           // control sort in GrB_mxm
}
GrB_Desc_Field ;

typedef enum
{
    // for all GrB_Descriptor fields:
    GxB_DEFAULT = 0,    // default behavior of the method

    // for GrB_OUTP only:
    GrB_REPLACE = 1,    // clear the output before assigning new values to it

    // for GrB_MASK only:
    GrB_COMP = 2,       // use the structural complement of the input
    GrB_SCMP = 2,       // same as GrB_COMP (deprecated; use GrB_COMP instead)
    GrB_STRUCTURE = 4,  // use the only pattern of the mask, not its values

    // for GrB_INP0 and GrB_INP1 only:
    GrB_TRAN = 3,       // use the transpose of the input

    // for GxB_GPU_CONTROL only (DRAFT: in progress, do not use)
    GxB_GPU_ALWAYS  = 2001,
    GxB_GPU_NEVER   = 2002,

    // for GxB_AxB_METHOD only:
    GxB_AxB_GUSTAVSON = 1001,   // gather-scatter saxpy method
    GxB_AxB_DOT       = 1003,   // dot product
    GxB_AxB_HASH      = 1004,   // hash-based saxpy method
    GxB_AxB_SAXPY     = 1005    // saxpy method (any kind)
}
GrB_Desc_Value ;

typedef struct GB_Descriptor_opaque *GrB_Descriptor ;

GB_PUBLIC
GrB_Info GrB_Descriptor_new     // create a new descriptor
(
    GrB_Descriptor *descriptor  // handle of descriptor to create
) ;

GB_PUBLIC
GrB_Info GrB_Descriptor_set     // set a parameter in a descriptor
(
    GrB_Descriptor desc,        // descriptor to modify
    GrB_Desc_Field field,       // parameter to change
    GrB_Desc_Value val          // value to change it to
) ;

GB_PUBLIC
GrB_Info GxB_Descriptor_get     // get a parameter from a descriptor
(
    GrB_Desc_Value *val,        // value of the parameter
    GrB_Descriptor desc,        // descriptor to query; NULL means defaults
    GrB_Desc_Field field        // parameter to query
) ;

GB_PUBLIC
GrB_Info GxB_Desc_set           // set a parameter in a descriptor
(
    GrB_Descriptor desc,        // descriptor to modify
    GrB_Desc_Field field,       // parameter to change
    ...                         // value to change it to
) ;

GB_PUBLIC
GrB_Info GxB_Desc_get           // get a parameter from a descriptor
(
    GrB_Descriptor desc,        // descriptor to query; NULL means defaults
    GrB_Desc_Field field,       // parameter to query
    ...                         // value of the parameter
) ;

GB_PUBLIC
GrB_Info GrB_Descriptor_free    // free a descriptor
(
    GrB_Descriptor *descriptor  // handle of descriptor to free
) ;

// Predefined descriptors and their values:

GB_PUBLIC
GrB_Descriptor     // OUTP         MASK           MASK       INP0      INP1
                   //              structural     complement
                   // ===========  ============== ========== ========  ========

// GrB_NULL        // -            -              -          -         -
GrB_DESC_T1      , // -            -              -          -         GrB_TRAN
GrB_DESC_T0      , // -            -              -          GrB_TRAN  -
GrB_DESC_T0T1    , // -            -              -          GrB_TRAN  GrB_TRAN

GrB_DESC_C       , // -            -              GrB_COMP   -         -
GrB_DESC_CT1     , // -            -              GrB_COMP   -         GrB_TRAN
GrB_DESC_CT0     , // -            -              GrB_COMP   GrB_TRAN  -
GrB_DESC_CT0T1   , // -            -              GrB_COMP   GrB_TRAN  GrB_TRAN

GrB_DESC_S       , // -            GrB_STRUCTURE  -          -         -
GrB_DESC_ST1     , // -            GrB_STRUCTURE  -          -         GrB_TRAN
GrB_DESC_ST0     , // -            GrB_STRUCTURE  -          GrB_TRAN  -
GrB_DESC_ST0T1   , // -            GrB_STRUCTURE  -          GrB_TRAN  GrB_TRAN

GrB_DESC_SC      , // -            GrB_STRUCTURE  GrB_COMP   -         -
GrB_DESC_SCT1    , // -            GrB_STRUCTURE  GrB_COMP   -         GrB_TRAN
GrB_DESC_SCT0    , // -            GrB_STRUCTURE  GrB_COMP   GrB_TRAN  -
GrB_DESC_SCT0T1  , // -            GrB_STRUCTURE  GrB_COMP   GrB_TRAN  GrB_TRAN

GrB_DESC_R       , // GrB_REPLACE  -              -          -         -
GrB_DESC_RT1     , // GrB_REPLACE  -              -          -         GrB_TRAN
GrB_DESC_RT0     , // GrB_REPLACE  -              -          GrB_TRAN  -
GrB_DESC_RT0T1   , // GrB_REPLACE  -              -          GrB_TRAN  GrB_TRAN

GrB_DESC_RC      , // GrB_REPLACE  -              GrB_COMP   -         -
GrB_DESC_RCT1    , // GrB_REPLACE  -              GrB_COMP   -         GrB_TRAN
GrB_DESC_RCT0    , // GrB_REPLACE  -              GrB_COMP   GrB_TRAN  -
GrB_DESC_RCT0T1  , // GrB_REPLACE  -              GrB_COMP   GrB_TRAN  GrB_TRAN

GrB_DESC_RS      , // GrB_REPLACE  GrB_STRUCTURE  -          -         -
GrB_DESC_RST1    , // GrB_REPLACE  GrB_STRUCTURE  -          -         GrB_TRAN
GrB_DESC_RST0    , // GrB_REPLACE  GrB_STRUCTURE  -          GrB_TRAN  -
GrB_DESC_RST0T1  , // GrB_REPLACE  GrB_STRUCTURE  -          GrB_TRAN  GrB_TRAN

GrB_DESC_RSC     , // GrB_REPLACE  GrB_STRUCTURE  GrB_COMP   -         -
GrB_DESC_RSCT1   , // GrB_REPLACE  GrB_STRUCTURE  GrB_COMP   -         GrB_TRAN
GrB_DESC_RSCT0   , // GrB_REPLACE  GrB_STRUCTURE  GrB_COMP   GrB_TRAN  -
GrB_DESC_RSCT0T1 ; // GrB_REPLACE  GrB_STRUCTURE  GrB_COMP   GrB_TRAN  GrB_TRAN

// GrB_NULL is the default descriptor, with all settings at their defaults:
//
//      OUTP: do not replace the output
//      MASK: mask is valued and not complemented
//      INP0: first input not transposed
//      INP1: second input not transposed

// Predefined descriptors may not be modified or freed.  Attempting to modify
// them results in an error (GrB_INVALID_VALUE).  Attempts to free them are
// silently ignored.

//------------------------------------------------------------------------------
// SuiteSparse:GraphBLAS options
//------------------------------------------------------------------------------

// The following options modify how SuiteSparse:GraphBLAS stores and operates
// on its matrices.  The GxB_*Option* methods allow the user to suggest how the
// internal representation of a matrix, or all matrices, should be held.  These
// options have no effect on the result (except for minor roundoff differences
// for floating-point types). They only affect the time and memory usage of the
// computations.

//      GxB_Matrix_Option_set:  sets an option for a specific matrix
//      GxB_Matrix_Option_get:  queries the current option of a specific matrix
//      GxB_Vector_Option_set:  sets an option for a specific vector
//      GxB_Vector_Option_get:  queries the current option of a specific vector
//      GxB_Global_Option_set:  sets an option for all future matrices
//      GxB_Global_Option_get:  queries current option for all future matrices

#define GxB_HYPER 0     // (deprecated, use GxB_HYPER_SWITCH)

typedef enum            // for global options or matrix options
{

    //------------------------------------------------------------
    // for GxB_Matrix_Option_get/set and GxB_Global_Option_get/set:
    //------------------------------------------------------------

    GxB_HYPER_SWITCH = 0,   // defines switch to hypersparse (a double value)
    GxB_BITMAP_SWITCH = 34, // defines switch to bitmap (a double value)
    GxB_FORMAT = 1,         // defines CSR/CSC format: GxB_BY_ROW or GxB_BY_COL

    //------------------------------------------------------------
    // for GxB_Global_Option_get only:
    //------------------------------------------------------------

    GxB_MODE = 2,       // mode passed to GrB_init (blocking or non-blocking)
    GxB_LIBRARY_NAME = 8,           // name of the library (char *)
    GxB_LIBRARY_VERSION = 9,        // library version (3 int's)
    GxB_LIBRARY_DATE = 10,          // date of the library (char *)
    GxB_LIBRARY_ABOUT = 11,         // about the library (char *)
    GxB_LIBRARY_URL = 12,           // URL for the library (char *)
    GxB_LIBRARY_LICENSE = 13,       // license of the library (char *)
    GxB_LIBRARY_COMPILE_DATE = 14,  // date library was compiled (char *)
    GxB_LIBRARY_COMPILE_TIME = 15,  // time library was compiled (char *)
    GxB_API_VERSION = 16,           // API version (3 int's)
    GxB_API_DATE = 17,              // date of the API (char *)
    GxB_API_ABOUT = 18,             // about the API (char *)
    GxB_API_URL = 19,               // URL for the API (char *)

    //------------------------------------------------------------
    // for GxB_Global_Option_get/set only:
    //------------------------------------------------------------

    GxB_GLOBAL_NTHREADS = GxB_NTHREADS,  // max number of threads to use
                        // If <= GxB_DEFAULT, then GraphBLAS selects the number
                        // of threads automatically.

    GxB_GLOBAL_CHUNK = GxB_CHUNK,       // chunk size for small problems.
                        // If <= GxB_DEFAULT, then the default is used.

    GxB_BURBLE = 99,    // diagnostic output (bool *)

    //------------------------------------------------------------
    // for GxB_Matrix_Option_get only:
    //------------------------------------------------------------

    GxB_SPARSITY_STATUS = 33,       // hyper, sparse, bitmap or full (1,2,4,8)
    GxB_IS_HYPER = 6,               // deprecated; use GxB_SPARSITY_STATUS

    //------------------------------------------------------------
    // for GxB_Matrix_Option_get/set only:
    //------------------------------------------------------------

    GxB_SPARSITY_CONTROL = 32,      // sparsity control: 0 to 15; see below

    //------------------------------------------------------------
    // GPU and options (DRAFT: do not use)
    //------------------------------------------------------------

    GxB_GLOBAL_GPU_CONTROL = GxB_GPU_CONTROL,
    GxB_GLOBAL_GPU_CHUNK   = GxB_GPU_CHUNK,

} GxB_Option_Field ;

// GxB_FORMAT can be by row or by column:
typedef enum
{
    GxB_BY_ROW = 0,     // CSR: compressed sparse row format
    GxB_BY_COL = 1,     // CSC: compressed sparse column format
    GxB_NO_FORMAT = -1  // format not defined
}
GxB_Format_Value ;

// The default format is by column, just like MATLAB.  These constants are
// defined as GB_PUBLIC const, so that if SuiteSparse:GraphBLAS is recompiled
// with a different default format, and the application is relinked but not
// recompiled, it will acquire the new default values.
GB_PUBLIC const GxB_Format_Value GxB_FORMAT_DEFAULT ;

// the default hyper_switch parameter
GB_PUBLIC const double GxB_HYPER_DEFAULT ;

// GxB_SPARSITY_CONTROL can be any sum or bitwise OR of these 4 values:
#define GxB_HYPERSPARSE 1   // store matrix in hypersparse form
#define GxB_SPARSE      2   // store matrix as sparse form (compressed vector)
#define GxB_BITMAP      4   // store matrix as a bitmap
#define GxB_FULL        8   // store matrix as full; all entries must be present

// size of b array for GxB_set/get (GxB_BITMAP_SWITCH, b)
#define GxB_NBITMAP_SWITCH 8    // size of bitmap_switch parameter array

// any sparsity value:
#define GxB_ANY_SPARSITY (GxB_HYPERSPARSE + GxB_SPARSE + GxB_BITMAP + GxB_FULL)

// the default sparsity control is any format:
#define GxB_AUTO_SPARSITY GxB_ANY_SPARSITY

// GxB_Matrix_Option_set (A, GxB_SPARSITY_CONTROL, scontrol) provides hints
// about which data structure GraphBLAS should use for the matrix A:
//
//      GxB_AUTO_SPARSITY: GraphBLAS selects automatically.
//      GxB_HYPERSPARSE: always hypersparse, taking O(nvals(A)) space.
//      GxB_SPARSE: always in a sparse struture: compressed-sparse row/column,
//          taking O(nrows+nvals(A)) space if stored by row, or
//          O(ncols+nvals(A)) if stored by column.
//      GxB_BITMAP: always in a bitmap struture, taking O(nrows*ncols) space.
//      GxB_FULL: always in a full structure, taking O(nrows*ncols) space,
//          unless not all entries are present, in which case the bitmap
//          storage is used.
//
// These options can be summed.  For example, to allow a matrix to be sparse
// or hypersparse, but not bitmap or full, use GxB_SPARSE + GxB_HYPERSPARSE.
// Since GxB_FULL can only be used when all entries are present, matrices with
// the just GxB_FULL control setting are stored in bitmap form if any entries
// are not present.
//
// Only the least 4 bits of the sparsity control are considered, so the
// formats can be bitwise negated.  For example, to allow for any format
// except full, use ~GxB_FULL.
//
// GxB_Matrix_Option_get (A, GxB_SPARSITY_STATUS, &sparsity) returns the
// current data structure currently used for the matrix A (either hypersparse,
// sparse, bitmap, or full).
//
// GxB_Matrix_Option_get (A, GxB_SPARSITY_CONTROL, &scontrol) returns the hint
// for how A should be stored (hypersparse, sparse, bitmap, or full, or any
// combination).

// GxB_HYPER_SWITCH:
//      If the matrix or vector structure can be sparse or hypersparse, the
//      GxB_HYPER_SWITCH parameter controls when each of these structures are
//      used.  The parameter is not used if the matrix or vector is full or
//      bitmap.
//
//      Let k be the actual number of non-empty vectors (with at least one
//      entry).  This value k is not dependent on whether or not the matrix is
//      stored in hypersparse structure.  Let n be the number of vectors (the #
//      of columns if CSC, or rows if CSR).  Let h be the value of the
//      GxB_HYPER_SWITCH setting of the matrix.
//
//      If a matrix is currently hypersparse, it can be converted to
//      non-hypersparse if (n <= 1  || k > 2*n*h).  Otherwise it stays
//      hypersparse.  If (n <= 1) the matrix is always stored as
//      non-hypersparse.
//
//      If currently non-hypersparse, it can be converted to hypersparse if (n
//      > 1 && k <= n*h).  Otherwise, it stays non-hypersparse.  If (n <= 1)
//      the matrix always remains non-hypersparse.
//
//      Setting GxB_HYPER_SWITCH to GxB_ALWAYS_HYPER or GxB_NEVER_HYPER ensures
//      a matrix always stays hypersparse, or always stays non-hypersparse,
//      respectively.

GB_PUBLIC const double GxB_ALWAYS_HYPER, GxB_NEVER_HYPER ;

GB_PUBLIC
GrB_Info GxB_Matrix_Option_set      // set an option in a matrix
(
    GrB_Matrix A,                   // matrix to modify
    GxB_Option_Field field,         // option to change
    ...                             // value to change it to
) ;

GB_PUBLIC
GrB_Info GxB_Matrix_Option_get      // gets the current option of a matrix
(
    GrB_Matrix A,                   // matrix to query
    GxB_Option_Field field,         // option to query
    ...                             // return value of the matrix option
) ;

GB_PUBLIC
GrB_Info GxB_Vector_Option_set      // set an option in a vector
(
    GrB_Vector A,                   // vector to modify
    GxB_Option_Field field,         // option to change
    ...                             // value to change it to
) ;

GB_PUBLIC
GrB_Info GxB_Vector_Option_get      // gets the current option of a vector
(
    GrB_Vector A,                   // vector to query
    GxB_Option_Field field,         // option to query
    ...                             // return value of the vector option
) ;

// GxB_Global_Option_set controls the global defaults used when a new matrix is
// created.  GrB_init defines the following initial settings:
//
//      GxB_Global_Option_set (GxB_HYPER_SWITCH, GxB_HYPER_DEFAULT) ;
//      GxB_Global_Option_set (GxB_BITMAP_SWITCH, NULL) ;
//      GxB_Global_Option_set (GxB_FORMAT, GxB_FORMAT_DEFAULT) ;
//
// The compile-time constants GxB_HYPER_DEFAULT and GxB_FORMAT_DEFAULT are
// equal to 0.0625 and GxB_BY_ROW, by default.  That is, by default, all new
// matrices are held by row in CSR format.  If a matrix has fewer than n/16
// columns, it can be converted to hypersparse structure.  If it has more than
// n/8 columns, it can be converted to a sparse structure.  Modifying these
// global settings via GxB_Global_Option_set has no effect on matrices already
// created.

GB_PUBLIC
GrB_Info GxB_Global_Option_set      // set a global default option
(
    GxB_Option_Field field,         // option to change
    ...                             // value to change it to
) ;

GB_PUBLIC
GrB_Info GxB_Global_Option_get      // gets the current global default option
(
    GxB_Option_Field field,         // option to query
    ...                             // return value of the global option
) ;

//------------------------------------------------------------------------------
// GxB_set and GxB_get
//------------------------------------------------------------------------------

// The simplest way to set/get a value of a GrB_Descriptor is with
// the generic GxB_set and GxB_get functions:

//      GxB_set (desc, field, value) ;
//      GxB_get (desc, field, &value) ;

// GxB_set and GxB_get are generic methods that and set or query the options in
// a GrB_Matrix, a GrB_Descriptor, or in the global options.  They can be used
// with the following syntax.  Note that GxB_NTHREADS can be used for both the
// global nthreads_max, and for the # of threads in the descriptor.

// To set/get the global options:
//
//      GxB_set (GxB_HYPER_SWITCH, double h) ;
//      GxB_set (GxB_HYPER_SWITCH, GxB_ALWAYS_HYPER) ;
//      GxB_set (GxB_HYPER_SWITCH, GxB_NEVER_HYPER) ;
//      GxB_get (GxB_HYPER_SWITCH, double *h) ;
//
//      double b [GxB_NBITMAP_SWITCH] ;
//      GxB_set (GxB_BITMAP_SWITCH, b) ;
//      GxB_set (GxB_BITMAP_SWITCH, NULL) ;     // set defaults
//      GxB_get (GxB_BITMAP_SWITCH, b) ;
//
//      GxB_set (GxB_FORMAT, GxB_BY_ROW) ;
//      GxB_set (GxB_FORMAT, GxB_BY_COL) ;
//      GxB_get (GxB_FORMAT, GxB_Format_Value *s) ;
//
//      GxB_set (GxB_NTHREADS, nthreads_max) ;
//      GxB_get (GxB_NTHREADS, int *nthreads_max) ;
//
//      GxB_set (GxB_CHUNK, double chunk) ;
//      GxB_get (GxB_CHUNK, double *chunk) ;
//
//      GxB_set (GxB_BURBLE, bool burble) ;
//      GxB_get (GxB_BURBLE, bool *burble) ;

// To get global options that can be queried but not modified:
//
//      GxB_get (GxB_MODE, GrB_Mode *mode) ;

// To set/get a matrix option:
//
//      GxB_set (GrB_Matrix A, GxB_HYPER_SWITCH, double h) ;
//      GxB_set (GrB_Matrix A, GxB_HYPER_SWITCH, GxB_ALWAYS_HYPER) ;
//      GxB_set (GrB_Matrix A, GxB_HYPER_SWITCH, GxB_NEVER_HYPER) ;
//      GxB_get (GrB_Matrix A, GxB_HYPER_SWITCH, double *h) ;
//
//      GxB_set (GrB_Matrix A, GxB_BITMAP_SWITCH, double b) ;
//      GxB_get (GrB_Matrix A, GxB_BITMAP_SWITCH, double *b) ;
//
//      GxB_set (GrB_Matrix A, GxB_FORMAT, GxB_BY_ROW) ;
//      GxB_set (GrB_Matrix A, GxB_FORMAT, GxB_BY_COL) ;
//      GxB_get (GrB_Matrix A, GxB_FORMAT, GxB_Format_Value *s) ;
//
//      GxB_set (GrB_Matrix A, GxB_SPARSITY_CONTROL, GxB_AUTO_SPARSITY) ;
//      GxB_set (GrB_Matrix A, GxB_SPARSITY_CONTROL, scontrol) ;
//      GxB_get (GrB_Matrix A, GxB_SPARSITY_CONTROL, int *scontrol) ;
//
//      GxB_get (GrB_Matrix A, GxB_SPARSITY_STATUS, int *sparsity) ;

// To set/get a vector option or status:
//
//      GxB_set (GrB_Vector v, GxB_BITMAP_SWITCH, double b) ;
//      GxB_get (GrB_Vector v, GxB_BITMAP_SWITCH, double *b) ;
//
//      GxB_set (GrB_Vector v, GxB_FORMAT, GxB_BY_ROW) ;
//      GxB_set (GrB_Vector v, GxB_FORMAT, GxB_BY_COL) ;
//      GxB_get (GrB_Vector v, GxB_FORMAT, GxB_Format_Value *s) ;
//
//      GxB_set (GrB_Vector v, GxB_SPARSITY_CONTROL, GxB_AUTO_SPARSITY) ;
//      GxB_set (GrB_Vector v, GxB_SPARSITY_CONTROL, scontrol) ;
//      GxB_get (GrB_Vector v, GxB_SPARSITY_CONTROL, int *scontrol) ;
//
//      GxB_get (GrB_Vector v, GxB_SPARSITY_STATUS, int *sparsity) ;

// To set/get a descriptor field:
//
//      GxB_set (GrB_Descriptor d, GrB_OUTP, GxB_DEFAULT) ;
//      GxB_set (GrB_Descriptor d, GrB_OUTP, GrB_REPLACE) ;
//      GxB_get (GrB_Descriptor d, GrB_OUTP, GrB_Desc_Value *v) ;
//
//      GxB_set (GrB_Descriptor d, GrB_MASK, GxB_DEFAULT) ;
//      GxB_set (GrB_Descriptor d, GrB_MASK, GrB_COMP) ;
//      GxB_set (GrB_Descriptor d, GrB_MASK, GrB_STRUCTURE) ;
//      GxB_set (GrB_Descriptor d, GrB_MASK, GrB_COMP + GrB_STRUCTURE) ;
//      GxB_get (GrB_Descriptor d, GrB_MASK, GrB_Desc_Value *v) ;
//
//      GxB_set (GrB_Descriptor d, GrB_INP0, GxB_DEFAULT) ;
//      GxB_set (GrB_Descriptor d, GrB_INP0, GrB_TRAN) ;
//      GxB_get (GrB_Descriptor d, GrB_INP0, GrB_Desc_Value *v) ;
//
//      GxB_set (GrB_Descriptor d, GrB_INP1, GxB_DEFAULT) ;
//      GxB_set (GrB_Descriptor d, GrB_INP1, GrB_TRAN) ;
//      GxB_get (GrB_Descriptor d, GrB_INP1, GrB_Desc_Value *v) ;
//
//      GxB_set (GrB_Descriptor d, GxB_AxB_METHOD, GxB_DEFAULT) ;
//      GxB_set (GrB_Descriptor d, GxB_AxB_METHOD, GxB_AxB_GUSTAVSON) ;
//      GxB_set (GrB_Descriptor d, GxB_AxB_METHOD, GxB_AxB_HASH) ;
//      GxB_set (GrB_Descriptor d, GxB_AxB_METHOD, GxB_AxB_SAXPY) ;
//      GxB_set (GrB_Descriptor d, GxB_AxB_METHOD, GxB_AxB_DOT) ;
//      GxB_get (GrB_Descriptor d, GrB_AxB_METHOD, GrB_Desc_Value *v) ;
//
//      GxB_set (GrB_Descriptor d, GxB_NTHREADS, nthreads) ;
//      GxB_get (GrB_Descriptor d, GxB_NTHREADS, int *nthreads) ;
//
//      GxB_set (GrB_Descriptor d, GxB_CHUNK, double chunk) ;
//      GxB_get (GrB_Descriptor d, GxB_CHUNK, double *chunk) ;
//
//      GxB_set (GrB_Descriptor d, GxB_SORT, sort) ;
//      GxB_get (GrB_Descriptor d, GxB_SORT, int *sort) ;

#if GxB_STDC_VERSION >= 201112L
#define GxB_set(arg1,...)                                   \
    _Generic                                                \
    (                                                       \
        (arg1),                                             \
              int              : GxB_Global_Option_set ,    \
              GxB_Option_Field : GxB_Global_Option_set ,    \
              GrB_Vector       : GxB_Vector_Option_set ,    \
              GrB_Matrix       : GxB_Matrix_Option_set ,    \
              GrB_Descriptor   : GxB_Desc_set               \
    )                                                       \
    (arg1, __VA_ARGS__)

#define GxB_get(arg1,...)                                   \
    _Generic                                                \
    (                                                       \
        (arg1),                                             \
        const int              : GxB_Global_Option_get ,    \
              int              : GxB_Global_Option_get ,    \
        const GxB_Option_Field : GxB_Global_Option_get ,    \
              GxB_Option_Field : GxB_Global_Option_get ,    \
        const GrB_Vector       : GxB_Vector_Option_get ,    \
              GrB_Vector       : GxB_Vector_Option_get ,    \
        const GrB_Matrix       : GxB_Matrix_Option_get ,    \
              GrB_Matrix       : GxB_Matrix_Option_get ,    \
        const GrB_Descriptor   : GxB_Desc_get          ,    \
              GrB_Descriptor   : GxB_Desc_get               \
    )                                                       \
    (arg1, __VA_ARGS__)
#endif

//==============================================================================
// GrB_free: free any GraphBLAS object
//==============================================================================

// for null and invalid objects
#define GrB_NULL NULL
#define GrB_INVALID_HANDLE NULL

#if GxB_STDC_VERSION >= 201112L
#define GrB_free(object)                         \
    _Generic                                     \
    (                                            \
        (object),                                \
        GrB_Type       *: GrB_Type_free       ,  \
        GrB_UnaryOp    *: GrB_UnaryOp_free    ,  \
        GrB_BinaryOp   *: GrB_BinaryOp_free   ,  \
        GxB_SelectOp   *: GxB_SelectOp_free   ,  \
        GrB_Monoid     *: GrB_Monoid_free     ,  \
        GrB_Semiring   *: GrB_Semiring_free   ,  \
        GxB_Scalar     *: GxB_Scalar_free     ,  \
        GrB_Vector     *: GrB_Vector_free     ,  \
        GrB_Matrix     *: GrB_Matrix_free     ,  \
        GrB_Descriptor *: GrB_Descriptor_free    \
    )                                            \
    (object)
#endif

//==============================================================================
// GrB_wait: finish computations
//==============================================================================

// Finish all pending work in a specific object.

GB_PUBLIC GrB_Info GrB_Type_wait       (GrB_Type       *type    ) ;
GB_PUBLIC GrB_Info GrB_UnaryOp_wait    (GrB_UnaryOp    *op      ) ;
GB_PUBLIC GrB_Info GrB_BinaryOp_wait   (GrB_BinaryOp   *op      ) ;
GB_PUBLIC GrB_Info GxB_SelectOp_wait   (GxB_SelectOp   *op      ) ;
GB_PUBLIC GrB_Info GrB_Monoid_wait     (GrB_Monoid     *monoid  ) ;
GB_PUBLIC GrB_Info GrB_Semiring_wait   (GrB_Semiring   *semiring) ;
GB_PUBLIC GrB_Info GrB_Descriptor_wait (GrB_Descriptor *desc    ) ;
GB_PUBLIC GrB_Info GxB_Scalar_wait     (GxB_Scalar     *s       ) ;
GB_PUBLIC GrB_Info GrB_Vector_wait     (GrB_Vector     *v       ) ;
GB_PUBLIC GrB_Info GrB_Matrix_wait     (GrB_Matrix     *A       ) ;

// GrB_wait (&object) polymorphic function:
#if GxB_STDC_VERSION >= 201112L
#define GrB_wait(object)                         \
    _Generic                                     \
    (                                            \
        (object),                                \
        GrB_Type       *: GrB_Type_wait       ,  \
        GrB_UnaryOp    *: GrB_UnaryOp_wait    ,  \
        GrB_BinaryOp   *: GrB_BinaryOp_wait   ,  \
        GxB_SelectOp   *: GxB_SelectOp_wait   ,  \
        GrB_Monoid     *: GrB_Monoid_wait     ,  \
        GrB_Semiring   *: GrB_Semiring_wait   ,  \
        GxB_Scalar     *: GxB_Scalar_wait     ,  \
        GrB_Vector     *: GrB_Vector_wait     ,  \
        GrB_Matrix     *: GrB_Matrix_wait     ,  \
        GrB_Descriptor *: GrB_Descriptor_wait    \
    )                                            \
    (object)
#endif

//==============================================================================
// GrB_error: error handling
//==============================================================================

// Each GraphBLAS method and operation returns a GrB_Info error code.
// GrB_error returns additional information on the error in a thread-safe
// null-terminated string.  The string returned by GrB_error is owned by
// the GraphBLAS library and must not be free'd.

GB_PUBLIC GrB_Info GrB_Type_error       (const char **error, const GrB_Type       type) ;
GB_PUBLIC GrB_Info GrB_UnaryOp_error    (const char **error, const GrB_UnaryOp    op) ;
GB_PUBLIC GrB_Info GrB_BinaryOp_error   (const char **error, const GrB_BinaryOp   op) ;
GB_PUBLIC GrB_Info GxB_SelectOp_error   (const char **error, const GxB_SelectOp   op) ;
GB_PUBLIC GrB_Info GrB_Monoid_error     (const char **error, const GrB_Monoid     monoid) ;
GB_PUBLIC GrB_Info GrB_Semiring_error   (const char **error, const GrB_Semiring   semiring) ;
GB_PUBLIC GrB_Info GxB_Scalar_error     (const char **error, const GxB_Scalar     s) ;
GB_PUBLIC GrB_Info GrB_Vector_error     (const char **error, const GrB_Vector     v) ;
GB_PUBLIC GrB_Info GrB_Matrix_error     (const char **error, const GrB_Matrix     A) ;
GB_PUBLIC GrB_Info GrB_Descriptor_error (const char **error, const GrB_Descriptor d) ;

// GrB_error (error,object) polymorphic function:
#if GxB_STDC_VERSION >= 201112L
#define GrB_error(error,object)                         \
    _Generic                                            \
    (                                                   \
        (object),                                       \
        const GrB_Type       : GrB_Type_error       ,   \
              GrB_Type       : GrB_Type_error       ,   \
        const GrB_UnaryOp    : GrB_UnaryOp_error    ,   \
              GrB_UnaryOp    : GrB_UnaryOp_error    ,   \
        const GrB_BinaryOp   : GrB_BinaryOp_error   ,   \
              GrB_BinaryOp   : GrB_BinaryOp_error   ,   \
        const GxB_SelectOp   : GxB_SelectOp_error   ,   \
              GxB_SelectOp   : GxB_SelectOp_error   ,   \
        const GrB_Monoid     : GrB_Monoid_error     ,   \
              GrB_Monoid     : GrB_Monoid_error     ,   \
        const GrB_Semiring   : GrB_Semiring_error   ,   \
              GrB_Semiring   : GrB_Semiring_error   ,   \
        const GxB_Scalar     : GxB_Scalar_error     ,   \
              GxB_Scalar     : GxB_Scalar_error     ,   \
        const GrB_Vector     : GrB_Vector_error     ,   \
              GrB_Vector     : GrB_Vector_error     ,   \
        const GrB_Matrix     : GrB_Matrix_error     ,   \
              GrB_Matrix     : GrB_Matrix_error     ,   \
        const GrB_Descriptor : GrB_Descriptor_error ,   \
              GrB_Descriptor : GrB_Descriptor_error     \
    )                                                   \
    (error, object)
#endif

//==============================================================================
// GrB_mxm, vxm, mxv: matrix multiplication over a semiring
//==============================================================================

GB_PUBLIC
GrB_Info GrB_mxm                    // C<Mask> = accum (C, A*B)
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Matrix Mask,          // optional mask for C, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C,T)
    const GrB_Semiring semiring,    // defines '+' and '*' for A*B
    const GrB_Matrix A,             // first input:  matrix A
    const GrB_Matrix B,             // second input: matrix B
    const GrB_Descriptor desc       // descriptor for C, Mask, A, and B
) ;

GB_PUBLIC
GrB_Info GrB_vxm                    // w'<Mask> = accum (w, u'*A)
(
    GrB_Vector w,                   // input/output vector for results
    const GrB_Vector mask,          // optional mask for w, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for z=accum(w,t)
    const GrB_Semiring semiring,    // defines '+' and '*' for u'*A
    const GrB_Vector u,             // first input:  vector u
    const GrB_Matrix A,             // second input: matrix A
    const GrB_Descriptor desc       // descriptor for w, mask, and A
) ;

GB_PUBLIC
GrB_Info GrB_mxv                    // w<Mask> = accum (w, A*u)
(
    GrB_Vector w,                   // input/output vector for results
    const GrB_Vector mask,          // optional mask for w, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for z=accum(w,t)
    const GrB_Semiring semiring,    // defines '+' and '*' for A*B
    const GrB_Matrix A,             // first input:  matrix A
    const GrB_Vector u,             // second input: vector u
    const GrB_Descriptor desc       // descriptor for w, mask, and A
) ;

//==============================================================================
// GrB_eWiseMult: element-wise matrix and vector operations, set intersection
//==============================================================================

// GrB_eWiseMult computes C<Mask> = accum (C, A.*B), where ".*" is MATLAB
// notation, and where pairs of elements in two matrices (or vectors) are
// pairwise "multiplied" with C(i,j) = mult (A(i,j),B(i,j)).

GB_PUBLIC
GrB_Info GrB_Vector_eWiseMult_Semiring       // w<Mask> = accum (w, u.*v)
(
    GrB_Vector w,                   // input/output vector for results
    const GrB_Vector mask,          // optional mask for w, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for z=accum(w,t)
    const GrB_Semiring semiring,    // defines '.*' for t=u.*v
    const GrB_Vector u,             // first input:  vector u
    const GrB_Vector v,             // second input: vector v
    const GrB_Descriptor desc       // descriptor for w and mask
) ;

GB_PUBLIC
GrB_Info GrB_Vector_eWiseMult_Monoid         // w<Mask> = accum (w, u.*v)
(
    GrB_Vector w,                   // input/output vector for results
    const GrB_Vector mask,          // optional mask for w, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for z=accum(w,t)
    const GrB_Monoid monoid,        // defines '.*' for t=u.*v
    const GrB_Vector u,             // first input:  vector u
    const GrB_Vector v,             // second input: vector v
    const GrB_Descriptor desc       // descriptor for w and mask
) ;

GB_PUBLIC
GrB_Info GrB_Vector_eWiseMult_BinaryOp       // w<Mask> = accum (w, u.*v)
(
    GrB_Vector w,                   // input/output vector for results
    const GrB_Vector mask,          // optional mask for w, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for z=accum(w,t)
    const GrB_BinaryOp mult,        // defines '.*' for t=u.*v
    const GrB_Vector u,             // first input:  vector u
    const GrB_Vector v,             // second input: vector v
    const GrB_Descriptor desc       // descriptor for w and mask
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_eWiseMult_Semiring       // C<Mask> = accum (C, A.*B)
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Matrix Mask,          // optional mask for C, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C,T)
    const GrB_Semiring semiring,    // defines '.*' for T=A.*B
    const GrB_Matrix A,             // first input:  matrix A
    const GrB_Matrix B,             // second input: matrix B
    const GrB_Descriptor desc       // descriptor for C, Mask, A, and B
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_eWiseMult_Monoid         // C<Mask> = accum (C, A.*B)
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Matrix Mask,          // optional mask for C, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C,T)
    const GrB_Monoid monoid,        // defines '.*' for T=A.*B
    const GrB_Matrix A,             // first input:  matrix A
    const GrB_Matrix B,             // second input: matrix B
    const GrB_Descriptor desc       // descriptor for C, Mask, A, and B
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_eWiseMult_BinaryOp       // C<Mask> = accum (C, A.*B)
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Matrix Mask,          // optional mask for C, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C,T)
    const GrB_BinaryOp mult,        // defines '.*' for T=A.*B
    const GrB_Matrix A,             // first input:  matrix A
    const GrB_Matrix B,             // second input: matrix B
    const GrB_Descriptor desc       // descriptor for C, Mask, A, and B
) ;

// All 6 of the above type-specific functions are captured in a single
// type-generic function, GrB_eWiseMult:

#if GxB_STDC_VERSION >= 201112L
#define GrB_eWiseMult(C,Mask,accum,op,A,B,desc)                         \
    _Generic                                                            \
    (                                                                   \
        (C),                                                            \
        GrB_Matrix :                                                    \
            _Generic                                                    \
            (                                                           \
                (op),                                                   \
                const GrB_Semiring : GrB_Matrix_eWiseMult_Semiring ,    \
                      GrB_Semiring : GrB_Matrix_eWiseMult_Semiring ,    \
                const GrB_Monoid   : GrB_Matrix_eWiseMult_Monoid   ,    \
                      GrB_Monoid   : GrB_Matrix_eWiseMult_Monoid   ,    \
                const GrB_BinaryOp : GrB_Matrix_eWiseMult_BinaryOp ,    \
                      GrB_BinaryOp : GrB_Matrix_eWiseMult_BinaryOp      \
            ),                                                          \
        GrB_Vector :                                                    \
            _Generic                                                    \
            (                                                           \
                (op),                                                   \
                const GrB_Semiring : GrB_Vector_eWiseMult_Semiring ,    \
                      GrB_Semiring : GrB_Vector_eWiseMult_Semiring ,    \
                const GrB_Monoid   : GrB_Vector_eWiseMult_Monoid   ,    \
                      GrB_Monoid   : GrB_Vector_eWiseMult_Monoid   ,    \
                const GrB_BinaryOp : GrB_Vector_eWiseMult_BinaryOp ,    \
                      GrB_BinaryOp : GrB_Vector_eWiseMult_BinaryOp      \
            )                                                           \
    )                                                                   \
    (C, Mask, accum, op, A, B, desc)
#endif

//==============================================================================
// GrB_eWiseAdd: element-wise matrix and vector operations, set union
//==============================================================================

// GrB_eWiseAdd computes C<Mask> = accum (C, A+B), where pairs of elements in
// two matrices (or two vectors) are pairwise "added".

GB_PUBLIC
GrB_Info GrB_Vector_eWiseAdd_Semiring       // w<Mask> = accum (w, u+v)
(
    GrB_Vector w,                   // input/output vector for results
    const GrB_Vector mask,          // optional mask for w, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for z=accum(w,t)
    const GrB_Semiring semiring,    // defines '+' for t=u+v
    const GrB_Vector u,             // first input:  vector u
    const GrB_Vector v,             // second input: vector v
    const GrB_Descriptor desc       // descriptor for w and mask
) ;

GB_PUBLIC
GrB_Info GrB_Vector_eWiseAdd_Monoid         // w<Mask> = accum (w, u+v)
(
    GrB_Vector w,                   // input/output vector for results
    const GrB_Vector mask,          // optional mask for w, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for z=accum(w,t)
    const GrB_Monoid monoid,        // defines '+' for t=u+v
    const GrB_Vector u,             // first input:  vector u
    const GrB_Vector v,             // second input: vector v
    const GrB_Descriptor desc       // descriptor for w and mask
) ;

GB_PUBLIC
GrB_Info GrB_Vector_eWiseAdd_BinaryOp       // w<Mask> = accum (w, u+v)
(
    GrB_Vector w,                   // input/output vector for results
    const GrB_Vector mask,          // optional mask for w, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for z=accum(w,t)
    const GrB_BinaryOp add,         // defines '+' for t=u+v
    const GrB_Vector u,             // first input:  vector u
    const GrB_Vector v,             // second input: vector v
    const GrB_Descriptor desc       // descriptor for w and mask
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_eWiseAdd_Semiring       // C<Mask> = accum (C, A+B)
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Matrix Mask,          // optional mask for C, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C,T)
    const GrB_Semiring semiring,    // defines '+' for T=A+B
    const GrB_Matrix A,             // first input:  matrix A
    const GrB_Matrix B,             // second input: matrix B
    const GrB_Descriptor desc       // descriptor for C, Mask, A, and B
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_eWiseAdd_Monoid         // C<Mask> = accum (C, A+B)
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Matrix Mask,          // optional mask for C, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C,T)
    const GrB_Monoid monoid,        // defines '+' for T=A+B
    const GrB_Matrix A,             // first input:  matrix A
    const GrB_Matrix B,             // second input: matrix B
    const GrB_Descriptor desc       // descriptor for C, Mask, A, and B
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_eWiseAdd_BinaryOp       // C<Mask> = accum (C, A+B)
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Matrix Mask,          // optional mask for C, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C,T)
    const GrB_BinaryOp add,         // defines '+' for T=A+B
    const GrB_Matrix A,             // first input:  matrix A
    const GrB_Matrix B,             // second input: matrix B
    const GrB_Descriptor desc       // descriptor for C, Mask, A, and B
) ;

#if GxB_STDC_VERSION >= 201112L
#define GrB_eWiseAdd(C,Mask,accum,op,A,B,desc)                          \
    _Generic                                                            \
    (                                                                   \
        (C),                                                            \
        GrB_Matrix :                                                    \
            _Generic                                                    \
            (                                                           \
                (op),                                                   \
                const GrB_Semiring : GrB_Matrix_eWiseAdd_Semiring ,     \
                      GrB_Semiring : GrB_Matrix_eWiseAdd_Semiring ,     \
                const GrB_Monoid   : GrB_Matrix_eWiseAdd_Monoid   ,     \
                      GrB_Monoid   : GrB_Matrix_eWiseAdd_Monoid   ,     \
                const GrB_BinaryOp : GrB_Matrix_eWiseAdd_BinaryOp ,     \
                      GrB_BinaryOp : GrB_Matrix_eWiseAdd_BinaryOp       \
            ),                                                          \
        GrB_Vector :                                                    \
            _Generic                                                    \
            (                                                           \
                (op),                                                   \
                const GrB_Semiring : GrB_Vector_eWiseAdd_Semiring ,     \
                      GrB_Semiring : GrB_Vector_eWiseAdd_Semiring ,     \
                const GrB_Monoid   : GrB_Vector_eWiseAdd_Monoid   ,     \
                      GrB_Monoid   : GrB_Vector_eWiseAdd_Monoid   ,     \
                const GrB_BinaryOp : GrB_Vector_eWiseAdd_BinaryOp ,     \
                      GrB_BinaryOp : GrB_Vector_eWiseAdd_BinaryOp       \
            )                                                           \
    )                                                                   \
    (C, Mask, accum, op, A, B, desc)
#endif

//==============================================================================
// GrB_extract: extract a submatrix or subvector
//==============================================================================

// Extract entries from a matrix or vector; T = A(I,J) in MATLAB notation.
// This (like most GraphBLAS methods) is then followed by C<Mask>=accum(C,T).

// To extract all rows of a matrix or vector, as in A (:,J) in MATLAB, use
// I=GrB_ALL as the input argument.  For all columns of a matrix, use
// J=GrB_ALL.

GB_PUBLIC const uint64_t *GrB_ALL ;

// To extract a range of rows and columns, I and J can be a list of 2 or 3
// indices that defines a range (begin:end) or a strided range (begin:inc:end),
// in MATLAB notation.  To specify the MATLAB syntax I = begin:end, the array I
// has size at least 2, where I [GxB_BEGIN] = begin and I [GxB_END] = end.  The
// parameter ni is then passed as the special value GxB_RANGE.  To specify the
// MATLAB syntax I = begin:inc:end, the array I has size at least three, with
// the values begin, end, and inc (in that order), and then pass in the value
// ni = GxB_STRIDE.  The same can be done for the list J and its size, nj.

// These special values of ni and nj can be used for GrB_assign,
// GrB_extract, and GxB_subassign.
#define GxB_RANGE       (INT64_MAX)
#define GxB_STRIDE      (INT64_MAX-1)
#define GxB_BACKWARDS   (INT64_MAX-2)

// for the strided range begin:inc:end, I [GxB_BEGIN] is the value of begin, I
// [GxB_END] is the value end, I [GxB_INC] is the magnitude of the stride.  If
// the stride is negative, use ni = GxB_BACKWARDS.
#define GxB_BEGIN (0)
#define GxB_END   (1)
#define GxB_INC   (2)

// For example, the MATLAB notation 10:-2:1 defines a sequence [10 8 6 4 2].
// The end point of the sequence (1) need not appear in the sequence, if
// the last increment goes past it.  To specify the same in GraphBLAS,
// use:

//      GrB_Index I [3], ni = GxB_BACKWARDS ;
//      I [GxB_BEGIN ] = 10 ;               // the start of the sequence
//      I [GxB_INC   ] = 2 ;                // the magnitude of the increment
//      I [GxB_END   ] = 1 ;                // the end of the sequence

GB_PUBLIC
GrB_Info GrB_Vector_extract         // w<mask> = accum (w, u(I))
(
    GrB_Vector w,                   // input/output vector for results
    const GrB_Vector mask,          // optional mask for w, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for z=accum(w,t)
    const GrB_Vector u,             // first input:  vector u
    const GrB_Index *I,             // row indices
    GrB_Index ni,                   // number of row indices
    const GrB_Descriptor desc       // descriptor for w and mask
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_extract         // C<Mask> = accum (C, A(I,J))
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Matrix Mask,          // optional mask for C, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C,T)
    const GrB_Matrix A,             // first input:  matrix A
    const GrB_Index *I,             // row indices
    GrB_Index ni,                   // number of row indices
    const GrB_Index *J,             // column indices
    GrB_Index nj,                   // number of column indices
    const GrB_Descriptor desc       // descriptor for C, Mask, and A
) ;

GB_PUBLIC
GrB_Info GrB_Col_extract            // w<mask> = accum (w, A(I,j))
(
    GrB_Vector w,                   // input/output matrix for results
    const GrB_Vector mask,          // optional mask for w, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for z=accum(w,t)
    const GrB_Matrix A,             // first input:  matrix A
    const GrB_Index *I,             // row indices
    GrB_Index ni,                   // number of row indices
    GrB_Index j,                    // column index
    const GrB_Descriptor desc       // descriptor for w, mask, and A
) ;

//------------------------------------------------------------------------------
// GrB_extract: generic matrix/vector extraction
//------------------------------------------------------------------------------

// GrB_extract is a generic interface to the following functions:

// GrB_Vector_extract (w,mask,acc,u,I,ni,d)      // w<m>    = acc (w, u(I))
// GrB_Col_extract    (w,mask,acc,A,I,ni,j,d)    // w<m>    = acc (w, A(I,j))
// GrB_Matrix_extract (C,Mask,acc,A,I,ni,J,nj,d) // C<Mask> = acc (C, A(I,J))

#if GxB_STDC_VERSION >= 201112L
#define GrB_extract(arg1,Mask,accum,arg4,...) \
    _Generic                                                \
    (                                                       \
        (arg1),                                             \
        GrB_Vector :                                        \
            _Generic                                        \
            (                                               \
                (arg4),                                     \
                const GrB_Vector : GrB_Vector_extract ,     \
                      GrB_Vector : GrB_Vector_extract ,     \
                const GrB_Matrix : GrB_Col_extract    ,     \
                      GrB_Matrix : GrB_Col_extract          \
            ),                                              \
        GrB_Matrix : GrB_Matrix_extract                     \
    )                                                       \
    (arg1, Mask, accum, arg4, __VA_ARGS__)
#endif

//==============================================================================
// GxB_subassign: matrix and vector subassign: C(I,J)<Mask> = accum (C(I,J), A)
//==============================================================================

// Assign entries in a matrix or vector; C(I,J) = A in MATLAB notation.

// Each GxB_subassign function is very similar to its corresponding GrB_assign
// function in the spec, but they differ in two ways: (1) the mask in
// GxB_subassign has the same size as w(I) for vectors and C(I,J) for matrices,
// and (2) they differ in the GrB_REPLACE option.  See the user guide for
// details.

// In GraphBLAS notation, the two methods can be described as follows:

// matrix and vector subassign: C(I,J)<Mask> = accum (C(I,J), A)
// matrix and vector    assign: C<Mask>(I,J) = accum (C(I,J), A)

// --- assign ------------------------------------------------------------------
//
// GrB_Matrix_assign      C<M>(I,J) += A        M same size as matrix C.
//                                              A is |I|-by-|J|
//
// GrB_Vector_assign      w<m>(I)   += u        m same size as column vector w.
//                                              u is |I|-by-1
//
// GrB_Row_assign         C<m'>(i,J) += u'      m is a column vector the same
//                                              size as a row of C.
//                                              u is |J|-by-1, i is a scalar.
//
// GrB_Col_assign         C<m>(I,j) += u        m is a column vector the same
//                                              size as a column of C.
//                                              u is |I|-by-1, j is a scalar.
//
// --- subassign ---------------------------------------------------------------
//
// GxB_Matrix_subassign   C(I,J)<M> += A        M same size as matrix A.
//                                              A is |I|-by-|J|
//
// GxB_Vector_subassign   w(I)<m>   += u        m same size as column vector u.
//                                              u is |I|-by-1
//
// GxB_Row_subassign      C(i,J)<m'> += u'      m same size as column vector u.
//                                              u is |J|-by-1, i is a scalar.
//
// GxB_Col_subassign      C(I,j)<m> += u        m same size as column vector u.
//                                              u is |I|-by-1, j is a scalar.

GB_PUBLIC
GrB_Info GxB_Vector_subassign       // w(I)<mask> = accum (w(I),u)
(
    GrB_Vector w,                   // input/output matrix for results
    const GrB_Vector mask,          // optional mask for w(I), unused if NULL
    const GrB_BinaryOp accum,       // optional accum for z=accum(w(I),t)
    const GrB_Vector u,             // first input:  vector u
    const GrB_Index *I,             // row indices
    GrB_Index ni,                   // number of row indices
    const GrB_Descriptor desc       // descriptor for w(I) and mask
) ;

GB_PUBLIC
GrB_Info GxB_Matrix_subassign       // C(I,J)<Mask> = accum (C(I,J),A)
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Matrix Mask,          // optional mask for C(I,J), unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C(I,J),T)
    const GrB_Matrix A,             // first input:  matrix A
    const GrB_Index *I,             // row indices
    GrB_Index ni,                   // number of row indices
    const GrB_Index *J,             // column indices
    GrB_Index nj,                   // number of column indices
    const GrB_Descriptor desc       // descriptor for C(I,J), Mask, and A
) ;

GB_PUBLIC
GrB_Info GxB_Col_subassign          // C(I,j)<mask> = accum (C(I,j),u)
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Vector mask,          // optional mask for C(I,j), unused if NULL
    const GrB_BinaryOp accum,       // optional accum for z=accum(C(I,j),t)
    const GrB_Vector u,             // input vector
    const GrB_Index *I,             // row indices
    GrB_Index ni,                   // number of row indices
    GrB_Index j,                    // column index
    const GrB_Descriptor desc       // descriptor for C(I,j) and mask
) ;

GB_PUBLIC
GrB_Info GxB_Row_subassign          // C(i,J)<mask'> = accum (C(i,J),u')
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Vector mask,          // optional mask for C(i,J), unused if NULL
    const GrB_BinaryOp accum,       // optional accum for z=accum(C(i,J),t)
    const GrB_Vector u,             // input vector
    GrB_Index i,                    // row index
    const GrB_Index *J,             // column indices
    GrB_Index nj,                   // number of column indices
    const GrB_Descriptor desc       // descriptor for C(i,J) and mask
) ;

//------------------------------------------------------------------------------
// GxB_Vector_subassign_[SCALAR]:  scalar expansion assignment to subvector
//------------------------------------------------------------------------------

// Assigns a single scalar to a subvector, w(I)<mask> = accum(w(I),x).  The
// scalar x is implicitly expanded into a vector u of size ni-by-1, with each
// entry in u equal to x, and then w(I)<mask> = accum(w(I),u) is done.

GB_PUBLIC
GrB_Info GxB_Vector_subassign_BOOL  // w(I)<mask> = accum (w(I),x)
(
    GrB_Vector w,                   // input/output vector for results
    const GrB_Vector mask,          // optional mask for w(I), unused if NULL
    const GrB_BinaryOp accum,       // optional accum for z=accum(w(I),x)
    bool x,                         // scalar to assign to w(I)
    const GrB_Index *I,             // row indices
    GrB_Index ni,                   // number of row indices
    const GrB_Descriptor desc       // descriptor for w(I) and mask
) ;

GB_PUBLIC
GrB_Info GxB_Vector_subassign_INT8  // w(I)<mask> = accum (w(I),x)
(
    GrB_Vector w,                   // input/output vector for results
    const GrB_Vector mask,          // optional mask for w(I), unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(w(I),x)
    int8_t x,                       // scalar to assign to w(I)
    const GrB_Index *I,             // row indices
    GrB_Index ni,                   // number of row indices
    const GrB_Descriptor desc       // descriptor for w(I) and mask
) ;

GB_PUBLIC
GrB_Info GxB_Vector_subassign_UINT8 // w(I)<mask> = accum (w(I),x)
(
    GrB_Vector w,                   // input/output vector for results
    const GrB_Vector mask,          // optional mask for w(I), unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(w(I),x)
    uint8_t x,                      // scalar to assign to w(I)
    const GrB_Index *I,             // row indices
    GrB_Index ni,                   // number of row indices
    const GrB_Descriptor desc       // descriptor for w(I) and mask
) ;

GB_PUBLIC
GrB_Info GxB_Vector_subassign_INT16 // w(I)<mask> = accum (w(I),x)
(
    GrB_Vector w,                   // input/output vector for results
    const GrB_Vector mask,          // optional mask for w(I), unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(w(I),x)
    int16_t x,                      // scalar to assign to w(I)
    const GrB_Index *I,             // row indices
    GrB_Index ni,                   // number of row indices
    const GrB_Descriptor desc       // descriptor for w(I) and mask
) ;

GB_PUBLIC
GrB_Info GxB_Vector_subassign_UINT16   // w(I)<mask> = accum (w(I),x)
(
    GrB_Vector w,                   // input/output vector for results
    const GrB_Vector mask,          // optional mask for w(I), unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(w(I),x)
    uint16_t x,                     // scalar to assign to w(I)
    const GrB_Index *I,             // row indices
    GrB_Index ni,                   // number of row indices
    const GrB_Descriptor desc       // descriptor for w(I) and mask
) ;

GB_PUBLIC
GrB_Info GxB_Vector_subassign_INT32    // w(I)<mask> = accum (w(I),x)
(
    GrB_Vector w,                   // input/output vector for results
    const GrB_Vector mask,          // optional mask for w(I), unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(w(I),x)
    int32_t x,                      // scalar to assign to w(I)
    const GrB_Index *I,             // row indices
    GrB_Index ni,                   // number of row indices
    const GrB_Descriptor desc       // descriptor for w(I) and mask
) ;

GB_PUBLIC
GrB_Info GxB_Vector_subassign_UINT32   // w(I)<mask> = accum (w(I),x)
(
    GrB_Vector w,                   // input/output vector for results
    const GrB_Vector mask,          // optional mask for w(I), unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(w(I),x)
    uint32_t x,                     // scalar to assign to w(I)
    const GrB_Index *I,             // row indices
    GrB_Index ni,                   // number of row indices
    const GrB_Descriptor desc       // descriptor for w(I) and mask
) ;

GB_PUBLIC
GrB_Info GxB_Vector_subassign_INT64    // w(I)<mask> = accum (w(I),x)
(
    GrB_Vector w,                   // input/output vector for results
    const GrB_Vector mask,          // optional mask for w(I), unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(w(I),x)
    int64_t x,                      // scalar to assign to w(I)
    const GrB_Index *I,             // row indices
    GrB_Index ni,                   // number of row indices
    const GrB_Descriptor desc       // descriptor for w(I) and mask
) ;

GB_PUBLIC
GrB_Info GxB_Vector_subassign_UINT64   // w(I)<mask> = accum (w(I),x)
(
    GrB_Vector w,                   // input/output vector for results
    const GrB_Vector mask,          // optional mask for w(I), unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(w(I),x)
    uint64_t x,                     // scalar to assign to w(I)
    const GrB_Index *I,             // row indices
    GrB_Index ni,                   // number of row indices
    const GrB_Descriptor desc       // descriptor for w(I) and mask
) ;

GB_PUBLIC
GrB_Info GxB_Vector_subassign_FP32     // w(I)<mask> = accum (w(I),x)
(
    GrB_Vector w,                   // input/output vector for results
    const GrB_Vector mask,          // optional mask for w(I), unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(w(I),x)
    float x,                        // scalar to assign to w(I)
    const GrB_Index *I,             // row indices
    GrB_Index ni,                   // number of row indices
    const GrB_Descriptor desc       // descriptor for w(I) and mask
) ;

GB_PUBLIC
GrB_Info GxB_Vector_subassign_FP64     // w(I)<mask> = accum (w(I),x)
(
    GrB_Vector w,                   // input/output vector for results
    const GrB_Vector mask,          // optional mask for w(I), unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(w(I),x)
    double x,                       // scalar to assign to w(I)
    const GrB_Index *I,             // row indices
    GrB_Index ni,                   // number of row indices
    const GrB_Descriptor desc       // descriptor for w(I) and mask
) ;

GB_PUBLIC
GrB_Info GxB_Vector_subassign_FC32     // w(I)<mask> = accum (w(I),x)
(
    GrB_Vector w,                   // input/output vector for results
    const GrB_Vector mask,          // optional mask for w(I), unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(w(I),x)
    GxB_FC32_t x,                   // scalar to assign to w(I)
    const GrB_Index *I,             // row indices
    GrB_Index ni,                   // number of row indices
    const GrB_Descriptor desc       // descriptor for w(I) and mask
) ;

GB_PUBLIC
GrB_Info GxB_Vector_subassign_FC64     // w(I)<mask> = accum (w(I),x)
(
    GrB_Vector w,                   // input/output vector for results
    const GrB_Vector mask,          // optional mask for w(I), unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(w(I),x)
    GxB_FC64_t x,                   // scalar to assign to w(I)
    const GrB_Index *I,             // row indices
    GrB_Index ni,                   // number of row indices
    const GrB_Descriptor desc       // descriptor for w(I) and mask
) ;

GB_PUBLIC
GrB_Info GxB_Vector_subassign_UDT      // w(I)<mask> = accum (w(I),x)
(
    GrB_Vector w,                   // input/output vector for results
    const GrB_Vector mask,          // optional mask for w(I), unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(w(I),x)
    void *x,                        // scalar to assign to w(I)
    const GrB_Index *I,             // row indices
    GrB_Index ni,                   // number of row indices
    const GrB_Descriptor desc       // descriptor for w(I) and mask
) ;

//------------------------------------------------------------------------------
// GxB_Matrix_subassign_[SCALAR]:  scalar expansion assignment to submatrix
//------------------------------------------------------------------------------

// Assigns a single scalar to a submatrix, C(I,J)<Mask> = accum(C(I,J),x).  The
// scalar x is implicitly expanded into a matrix A of size ni-by-nj, with each
// entry in A equal to x, and then C(I,J)<Mask> = accum(C(I,J),A) is done.

GB_PUBLIC
GrB_Info GxB_Matrix_subassign_BOOL  // C(I,J)<Mask> = accum (C(I,J),x)
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Matrix Mask,          // optional mask for C(I,J), unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C(I,J),x)
    bool x,                         // scalar to assign to C(I,J)
    const GrB_Index *I,             // row indices
    GrB_Index ni,                   // number of row indices
    const GrB_Index *J,             // column indices
    GrB_Index nj,                   // number of column indices
    const GrB_Descriptor desc       // descriptor for C(I,J) and Mask
) ;

GB_PUBLIC
GrB_Info GxB_Matrix_subassign_INT8  // C(I,J)<Mask> = accum (C(I,J),x)
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Matrix Mask,          // optional mask for C(I,J), unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C(I,J),x)
    int8_t x,                       // scalar to assign to C(I,J)
    const GrB_Index *I,             // row indices
    GrB_Index ni,                   // number of row indices
    const GrB_Index *J,             // column indices
    GrB_Index nj,                   // number of column indices
    const GrB_Descriptor desc       // descriptor for C(I,J) and Mask
) ;

GB_PUBLIC
GrB_Info GxB_Matrix_subassign_UINT8 // C(I,J)<Mask> = accum (C(I,J),x)
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Matrix Mask,          // optional mask for C(I,J), unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C(I,J),x)
    uint8_t x,                      // scalar to assign to C(I,J)
    const GrB_Index *I,             // row indices
    GrB_Index ni,                   // number of row indices
    const GrB_Index *J,             // column indices
    GrB_Index nj,                   // number of column indices
    const GrB_Descriptor desc       // descriptor for C(I,J) and Mask
) ;

GB_PUBLIC
GrB_Info GxB_Matrix_subassign_INT16 // C(I,J)<Mask> = accum (C(I,J),x)
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Matrix Mask,          // optional mask for C(I,J), unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C(I,J),x)
    int16_t x,                      // scalar to assign to C(I,J)
    const GrB_Index *I,             // row indices
    GrB_Index ni,                   // number of row indices
    const GrB_Index *J,             // column indices
    GrB_Index nj,                   // number of column indices
    const GrB_Descriptor desc       // descriptor for C(I,J) and Mask
) ;

GB_PUBLIC
GrB_Info GxB_Matrix_subassign_UINT16   // C(I,J)<Mask> = accum (C(I,J),x)
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Matrix Mask,          // optional mask for C(I,J), unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C(I,J),x)
    uint16_t x,                     // scalar to assign to C(I,J)
    const GrB_Index *I,             // row indices
    GrB_Index ni,                   // number of row indices
    const GrB_Index *J,             // column indices
    GrB_Index nj,                   // number of column indices
    const GrB_Descriptor desc       // descriptor for C(I,J) and Mask
) ;

GB_PUBLIC
GrB_Info GxB_Matrix_subassign_INT32    // C(I,J)<Mask> = accum (C(I,J),x)
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Matrix Mask,          // optional mask for C(I,J), unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C(I,J),x)
    int32_t x,                      // scalar to assign to C(I,J)
    const GrB_Index *I,             // row indices
    GrB_Index ni,                   // number of row indices
    const GrB_Index *J,             // column indices
    GrB_Index nj,                   // number of column indices
    const GrB_Descriptor desc       // descriptor for C(I,J) and Mask
) ;

GB_PUBLIC
GrB_Info GxB_Matrix_subassign_UINT32   // C(I,J)<Mask> = accum (C(I,J),x)
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Matrix Mask,          // optional mask for C(I,J), unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C(I,J),x)
    uint32_t x,                     // scalar to assign to C(I,J)
    const GrB_Index *I,             // row indices
    GrB_Index ni,                   // number of row indices
    const GrB_Index *J,             // column indices
    GrB_Index nj,                   // number of column indices
    const GrB_Descriptor desc       // descriptor for C(I,J) and Mask
) ;

GB_PUBLIC
GrB_Info GxB_Matrix_subassign_INT64    // C(I,J)<Mask> = accum (C(I,J),x)
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Matrix Mask,          // optional mask for C(I,J), unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C(I,J),x)
    int64_t x,                      // scalar to assign to C(I,J)
    const GrB_Index *I,             // row indices
    GrB_Index ni,                   // number of row indices
    const GrB_Index *J,             // column indices
    GrB_Index nj,                   // number of column indices
    const GrB_Descriptor desc       // descriptor for C(I,J) and Mask
) ;

GB_PUBLIC
GrB_Info GxB_Matrix_subassign_UINT64   // C(I,J)<Mask> = accum (C(I,J),x)
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Matrix Mask,          // optional mask for C(I,J), unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C(I,J),x)
    uint64_t x,                     // scalar to assign to C(I,J)
    const GrB_Index *I,             // row indices
    GrB_Index ni,                   // number of row indices
    const GrB_Index *J,             // column indices
    GrB_Index nj,                   // number of column indices
    const GrB_Descriptor desc       // descriptor for C(I,J) and Mask
) ;

GB_PUBLIC
GrB_Info GxB_Matrix_subassign_FP32     // C(I,J)<Mask> = accum (C(I,J),x)
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Matrix Mask,          // optional mask for C(I,J), unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C(I,J),x)
    float x,                        // scalar to assign to C(I,J)
    const GrB_Index *I,             // row indices
    GrB_Index ni,                   // number of row indices
    const GrB_Index *J,             // column indices
    GrB_Index nj,                   // number of column indices
    const GrB_Descriptor desc       // descriptor for C(I,J) and Mask
) ;

GB_PUBLIC
GrB_Info GxB_Matrix_subassign_FP64     // C(I,J)<Mask> = accum (C(I,J),x)
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Matrix Mask,          // optional mask for C(I,J), unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C(I,J),x)
    double x,                       // scalar to assign to C(I,J)
    const GrB_Index *I,             // row indices
    GrB_Index ni,                   // number of row indices
    const GrB_Index *J,             // column indices
    GrB_Index nj,                   // number of column indices
    const GrB_Descriptor desc       // descriptor for C(I,J) and Mask
) ;

GB_PUBLIC
GrB_Info GxB_Matrix_subassign_FC32     // C(I,J)<Mask> = accum (C(I,J),x)
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Matrix Mask,          // optional mask for C(I,J), unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C(I,J),x)
    GxB_FC32_t x,                   // scalar to assign to C(I,J)
    const GrB_Index *I,             // row indices
    GrB_Index ni,                   // number of row indices
    const GrB_Index *J,             // column indices
    GrB_Index nj,                   // number of column indices
    const GrB_Descriptor desc       // descriptor for C(I,J) and Mask
) ;

GB_PUBLIC
GrB_Info GxB_Matrix_subassign_FC64     // C(I,J)<Mask> = accum (C(I,J),x)
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Matrix Mask,          // optional mask for C(I,J), unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C(I,J),x)
    GxB_FC64_t x,                   // scalar to assign to C(I,J)
    const GrB_Index *I,             // row indices
    GrB_Index ni,                   // number of row indices
    const GrB_Index *J,             // column indices
    GrB_Index nj,                   // number of column indices
    const GrB_Descriptor desc       // descriptor for C(I,J) and Mask
) ;

GB_PUBLIC
GrB_Info GxB_Matrix_subassign_UDT      // C(I,J)<Mask> = accum (C(I,J),x)
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Matrix Mask,          // optional mask for C(I,J), unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C(I,J),x)
    void *x,                        // scalar to assign to C(I,J)
    const GrB_Index *I,             // row indices
    GrB_Index ni,                   // number of row indices
    const GrB_Index *J,             // column indices
    GrB_Index nj,                   // number of column indices
    const GrB_Descriptor desc       // descriptor for C(I,J) and Mask
) ;

//------------------------------------------------------------------------------
// GxB_subassign: generic submatrix/subvector assignment
//------------------------------------------------------------------------------

// GxB_subassign is a generic function that provides access to all specific
// GxB_*_subassign* functions:

// GxB_Vector_subassign   (w,m,acc,u,I,ni,d)      // w(I)<m>    = acc(w(I),u)
// GxB_Matrix_subassign   (C,M,acc,A,I,ni,J,nj,d) // C(I,J)<M>  = acc(C(I,J),A)
// GxB_Col_subassign      (C,m,acc,u,I,ni,j,d)    // C(I,j)<m>  = acc(C(I,j),u)
// GxB_Row_subassign      (C,m,acc,u,i,J,nj,d)    // C(i,J)<m'> = acc(C(i,J),u')
// GxB_Vector_subassign_T (w,m,acc,x,I,ni,d)      // w(I)<m>    = acc(w(I),x)
// GxB_Matrix_subassign_T (C,M,acc,x,I,ni,J,nj,d) // C(I,J)<M>  = acc(C(I,J),x)

#if GxB_STDC_VERSION >= 201112L
#define GxB_subassign(arg1,Mask,accum,arg4,arg5,...)               \
    _Generic                                                       \
    (                                                              \
        (arg1),                                                    \
        GrB_Vector :                                               \
            _Generic                                               \
            (                                                      \
                (arg4),                                            \
                GB_(, GxB, Vector_subassign) ,                     \
                default : GxB_Vector_subassign                     \
            ),                                                     \
        default :                                                  \
            _Generic                                               \
            (                                                      \
                (arg4),                                            \
                GB_(, GxB, Matrix_subassign) ,                     \
                const GrB_Vector :                                 \
                    _Generic                                       \
                    (                                              \
                        (arg5),                                    \
                        const GrB_Index *: GxB_Col_subassign ,     \
                              GrB_Index *: GxB_Col_subassign ,     \
                        default          : GxB_Row_subassign       \
                    ),                                             \
                GrB_Vector :                                       \
                    _Generic                                       \
                    (                                              \
                        (arg5),                                    \
                        const GrB_Index *: GxB_Col_subassign ,     \
                              GrB_Index *: GxB_Col_subassign ,     \
                        default          : GxB_Row_subassign       \
                    ),                                             \
                default    : GxB_Matrix_subassign                  \
            )                                                      \
    )                                                              \
    (arg1, Mask, accum, arg4, arg5, __VA_ARGS__)
#endif

//==============================================================================
// GrB_assign: matrix and vector assign: C<Mask>(I,J) = accum (C(I,J), A)
//==============================================================================

// Assign entries in a matrix or vector; C(I,J) = A in MATLAB notation.
// Each of these can be used with their generic name, GrB_assign.

GB_PUBLIC
GrB_Info GrB_Vector_assign          // w<mask>(I) = accum (w(I),u)
(
    GrB_Vector w,                   // input/output matrix for results
    const GrB_Vector mask,          // optional mask for w, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for z=accum(w(I),t)
    const GrB_Vector u,             // first input:  vector u
    const GrB_Index *I,             // row indices
    GrB_Index ni,                   // number of row indices
    const GrB_Descriptor desc       // descriptor for w and mask
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_assign          // C<Mask>(I,J) = accum (C(I,J),A)
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Matrix Mask,          // optional mask for C, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C(I,J),T)
    const GrB_Matrix A,             // first input:  matrix A
    const GrB_Index *I,             // row indices
    GrB_Index ni,                   // number of row indices
    const GrB_Index *J,             // column indices
    GrB_Index nj,                   // number of column indices
    const GrB_Descriptor desc       // descriptor for C, Mask, and A
) ;

GB_PUBLIC
GrB_Info GrB_Col_assign             // C<mask>(I,j) = accum (C(I,j),u)
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Vector mask,          // optional mask for C(:,j), unused if NULL
    const GrB_BinaryOp accum,       // optional accum for z=accum(C(I,j),t)
    const GrB_Vector u,             // input vector
    const GrB_Index *I,             // row indices
    GrB_Index ni,                   // number of row indices
    GrB_Index j,                    // column index
    const GrB_Descriptor desc       // descriptor for C(:,j) and mask
) ;

GB_PUBLIC
GrB_Info GrB_Row_assign             // C<mask'>(i,J) = accum (C(i,J),u')
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Vector mask,          // optional mask for C(i,:), unused if NULL
    const GrB_BinaryOp accum,       // optional accum for z=accum(C(i,J),t)
    const GrB_Vector u,             // input vector
    GrB_Index i,                    // row index
    const GrB_Index *J,             // column indices
    GrB_Index nj,                   // number of column indices
    const GrB_Descriptor desc       // descriptor for C(i,:) and mask
) ;

//------------------------------------------------------------------------------
// GrB_Vector_assign_[SCALAR]:  scalar expansion assignment to subvector
//------------------------------------------------------------------------------

// Assigns a single scalar to a subvector, w<mask>(I) = accum(w(I),x).  The
// scalar x is implicitly expanded into a vector u of size ni-by-1, with each
// entry in u equal to x, and then w<mask>(I) = accum(w(I),u) is done.

GB_PUBLIC
GrB_Info GrB_Vector_assign_BOOL     // w<mask>(I) = accum (w(I),x)
(
    GrB_Vector w,                   // input/output vector for results
    const GrB_Vector mask,          // optional mask for w, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for z=accum(w(I),x)
    bool x,                         // scalar to assign to w(I)
    const GrB_Index *I,             // row indices
    GrB_Index ni,                   // number of row indices
    const GrB_Descriptor desc       // descriptor for w and mask
) ;

GB_PUBLIC
GrB_Info GrB_Vector_assign_INT8     // w<mask>(I) = accum (w(I),x)
(
    GrB_Vector w,                   // input/output vector for results
    const GrB_Vector mask,          // optional mask for w, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(w(I),x)
    int8_t x,                       // scalar to assign to w(I)
    const GrB_Index *I,             // row indices
    GrB_Index ni,                   // number of row indices
    const GrB_Descriptor desc       // descriptor for w and mask
) ;

GB_PUBLIC
GrB_Info GrB_Vector_assign_UINT8    // w<mask>(I) = accum (w(I),x)
(
    GrB_Vector w,                   // input/output vector for results
    const GrB_Vector mask,          // optional mask for w, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(w(I),x)
    uint8_t x,                      // scalar to assign to w(I)
    const GrB_Index *I,             // row indices
    GrB_Index ni,                   // number of row indices
    const GrB_Descriptor desc       // descriptor for w and mask
) ;

GB_PUBLIC
GrB_Info GrB_Vector_assign_INT16    // w<mask>(I) = accum (w(I),x)
(
    GrB_Vector w,                   // input/output vector for results
    const GrB_Vector mask,          // optional mask for w, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(w(I),x)
    int16_t x,                      // scalar to assign to w(I)
    const GrB_Index *I,             // row indices
    GrB_Index ni,                   // number of row indices
    const GrB_Descriptor desc       // descriptor for w and mask
) ;

GB_PUBLIC
GrB_Info GrB_Vector_assign_UINT16   // w<mask>(I) = accum (w(I),x)
(
    GrB_Vector w,                   // input/output vector for results
    const GrB_Vector mask,          // optional mask for w, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(w(I),x)
    uint16_t x,                     // scalar to assign to w(I)
    const GrB_Index *I,             // row indices
    GrB_Index ni,                   // number of row indices
    const GrB_Descriptor desc       // descriptor for w and mask
) ;

GB_PUBLIC
GrB_Info GrB_Vector_assign_INT32    // w<mask>(I) = accum (w(I),x)
(
    GrB_Vector w,                   // input/output vector for results
    const GrB_Vector mask,          // optional mask for w, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(w(I),x)
    int32_t x,                      // scalar to assign to w(I)
    const GrB_Index *I,             // row indices
    GrB_Index ni,                   // number of row indices
    const GrB_Descriptor desc       // descriptor for w and mask
) ;

GB_PUBLIC
GrB_Info GrB_Vector_assign_UINT32   // w<mask>(I) = accum (w(I),x)
(
    GrB_Vector w,                   // input/output vector for results
    const GrB_Vector mask,          // optional mask for w, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(w(I),x)
    uint32_t x,                     // scalar to assign to w(I)
    const GrB_Index *I,             // row indices
    GrB_Index ni,                   // number of row indices
    const GrB_Descriptor desc       // descriptor for w and mask
) ;

GB_PUBLIC
GrB_Info GrB_Vector_assign_INT64    // w<mask>(I) = accum (w(I),x)
(
    GrB_Vector w,                   // input/output vector for results
    const GrB_Vector mask,          // optional mask for w, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(w(I),x)
    int64_t x,                      // scalar to assign to w(I)
    const GrB_Index *I,             // row indices
    GrB_Index ni,                   // number of row indices
    const GrB_Descriptor desc       // descriptor for w and mask
) ;

GB_PUBLIC
GrB_Info GrB_Vector_assign_UINT64   // w<mask>(I) = accum (w(I),x)
(
    GrB_Vector w,                   // input/output vector for results
    const GrB_Vector mask,          // optional mask for w, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(w(I),x)
    uint64_t x,                     // scalar to assign to w(I)
    const GrB_Index *I,             // row indices
    GrB_Index ni,                   // number of row indices
    const GrB_Descriptor desc       // descriptor for w and mask
) ;

GB_PUBLIC
GrB_Info GrB_Vector_assign_FP32     // w<mask>(I) = accum (w(I),x)
(
    GrB_Vector w,                   // input/output vector for results
    const GrB_Vector mask,          // optional mask for w, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(w(I),x)
    float x,                        // scalar to assign to w(I)
    const GrB_Index *I,             // row indices
    GrB_Index ni,                   // number of row indices
    const GrB_Descriptor desc       // descriptor for w and mask
) ;

GB_PUBLIC
GrB_Info GrB_Vector_assign_FP64     // w<mask>(I) = accum (w(I),x)
(
    GrB_Vector w,                   // input/output vector for results
    const GrB_Vector mask,          // optional mask for w, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(w(I),x)
    double x,                       // scalar to assign to w(I)
    const GrB_Index *I,             // row indices
    GrB_Index ni,                   // number of row indices
    const GrB_Descriptor desc       // descriptor for w and mask
) ;

GB_PUBLIC
GrB_Info GxB_Vector_assign_FC32     // w<mask>(I) = accum (w(I),x)
(
    GrB_Vector w,                   // input/output vector for results
    const GrB_Vector mask,          // optional mask for w, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(w(I),x)
    GxB_FC32_t x,                   // scalar to assign to w(I)
    const GrB_Index *I,             // row indices
    GrB_Index ni,                   // number of row indices
    const GrB_Descriptor desc       // descriptor for w and mask
) ;

GB_PUBLIC
GrB_Info GxB_Vector_assign_FC64     // w<mask>(I) = accum (w(I),x)
(
    GrB_Vector w,                   // input/output vector for results
    const GrB_Vector mask,          // optional mask for w, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(w(I),x)
    GxB_FC64_t x,                   // scalar to assign to w(I)
    const GrB_Index *I,             // row indices
    GrB_Index ni,                   // number of row indices
    const GrB_Descriptor desc       // descriptor for w and mask
) ;

GB_PUBLIC
GrB_Info GrB_Vector_assign_UDT      // w<mask>(I) = accum (w(I),x)
(
    GrB_Vector w,                   // input/output vector for results
    const GrB_Vector mask,          // optional mask for w, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(w(I),x)
    void *x,                        // scalar to assign to w(I)
    const GrB_Index *I,             // row indices
    GrB_Index ni,                   // number of row indices
    const GrB_Descriptor desc       // descriptor for w and mask
) ;

//------------------------------------------------------------------------------
// GrB_Matrix_assign_[SCALAR]:  scalar expansion assignment to submatrix
//------------------------------------------------------------------------------

// Assigns a single scalar to a submatrix, C<Mask>(I,J) = accum(C(I,J),x).  The
// scalar x is implicitly expanded into a matrix A of size ni-by-nj, with each
// entry in A equal to x, and then C<Mask>(I,J) = accum(C(I,J),A) is done.

GB_PUBLIC
GrB_Info GrB_Matrix_assign_BOOL     // C<Mask>(I,J) = accum (C(I,J),x)
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Matrix Mask,          // optional mask for C, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C(I,J),x)
    bool x,                         // scalar to assign to C(I,J)
    const GrB_Index *I,             // row indices
    GrB_Index ni,                   // number of row indices
    const GrB_Index *J,             // column indices
    GrB_Index nj,                   // number of column indices
    const GrB_Descriptor desc       // descriptor for C and Mask
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_assign_INT8     // C<Mask>(I,J) = accum (C(I,J),x)
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Matrix Mask,          // optional mask for C, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C(I,J),x)
    int8_t x,                       // scalar to assign to C(I,J)
    const GrB_Index *I,             // row indices
    GrB_Index ni,                   // number of row indices
    const GrB_Index *J,             // column indices
    GrB_Index nj,                   // number of column indices
    const GrB_Descriptor desc       // descriptor for C and Mask
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_assign_UINT8    // C<Mask>(I,J) = accum (C(I,J),x)
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Matrix Mask,          // optional mask for C, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C(I,J),x)
    uint8_t x,                      // scalar to assign to C(I,J)
    const GrB_Index *I,             // row indices
    GrB_Index ni,                   // number of row indices
    const GrB_Index *J,             // column indices
    GrB_Index nj,                   // number of column indices
    const GrB_Descriptor desc       // descriptor for C and Mask
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_assign_INT16    // C<Mask>(I,J) = accum (C(I,J),x)
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Matrix Mask,          // optional mask for C, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C(I,J),x)
    int16_t x,                      // scalar to assign to C(I,J)
    const GrB_Index *I,             // row indices
    GrB_Index ni,                   // number of row indices
    const GrB_Index *J,             // column indices
    GrB_Index nj,                   // number of column indices
    const GrB_Descriptor desc       // descriptor for C and Mask
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_assign_UINT16   // C<Mask>(I,J) = accum (C(I,J),x)
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Matrix Mask,          // optional mask for C, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C(I,J),x)
    uint16_t x,                     // scalar to assign to C(I,J)
    const GrB_Index *I,             // row indices
    GrB_Index ni,                   // number of row indices
    const GrB_Index *J,             // column indices
    GrB_Index nj,                   // number of column indices
    const GrB_Descriptor desc       // descriptor for C and Mask
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_assign_INT32    // C<Mask>(I,J) = accum (C(I,J),x)
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Matrix Mask,          // optional mask for C, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C(I,J),x)
    int32_t x,                      // scalar to assign to C(I,J)
    const GrB_Index *I,             // row indices
    GrB_Index ni,                   // number of row indices
    const GrB_Index *J,             // column indices
    GrB_Index nj,                   // number of column indices
    const GrB_Descriptor desc       // descriptor for C and Mask
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_assign_UINT32   // C<Mask>(I,J) = accum (C(I,J),x)
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Matrix Mask,          // optional mask for C, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C(I,J),x)
    uint32_t x,                     // scalar to assign to C(I,J)
    const GrB_Index *I,             // row indices
    GrB_Index ni,                   // number of row indices
    const GrB_Index *J,             // column indices
    GrB_Index nj,                   // number of column indices
    const GrB_Descriptor desc       // descriptor for C and Mask
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_assign_INT64    // C<Mask>(I,J) = accum (C(I,J),x)
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Matrix Mask,          // optional mask for C, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C(I,J),x)
    int64_t x,                      // scalar to assign to C(I,J)
    const GrB_Index *I,             // row indices
    GrB_Index ni,                   // number of row indices
    const GrB_Index *J,             // column indices
    GrB_Index nj,                   // number of column indices
    const GrB_Descriptor desc       // descriptor for C and Mask
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_assign_UINT64   // C<Mask>(I,J) = accum (C(I,J),x)
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Matrix Mask,          // optional mask for C, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C(I,J),x)
    uint64_t x,                     // scalar to assign to C(I,J)
    const GrB_Index *I,             // row indices
    GrB_Index ni,                   // number of row indices
    const GrB_Index *J,             // column indices
    GrB_Index nj,                   // number of column indices
    const GrB_Descriptor desc       // descriptor for C and Mask
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_assign_FP32     // C<Mask>(I,J) = accum (C(I,J),x)
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Matrix Mask,          // optional mask for C, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C(I,J),x)
    float x,                        // scalar to assign to C(I,J)
    const GrB_Index *I,             // row indices
    GrB_Index ni,                   // number of row indices
    const GrB_Index *J,             // column indices
    GrB_Index nj,                   // number of column indices
    const GrB_Descriptor desc       // descriptor for C and Mask
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_assign_FP64     // C<Mask>(I,J) = accum (C(I,J),x)
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Matrix Mask,          // optional mask for C, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C(I,J),x)
    double x,                       // scalar to assign to C(I,J)
    const GrB_Index *I,             // row indices
    GrB_Index ni,                   // number of row indices
    const GrB_Index *J,             // column indices
    GrB_Index nj,                   // number of column indices
    const GrB_Descriptor desc       // descriptor for C and Mask
) ;

GB_PUBLIC
GrB_Info GxB_Matrix_assign_FC32     // C<Mask>(I,J) = accum (C(I,J),x)
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Matrix Mask,          // optional mask for C, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C(I,J),x)
    GxB_FC32_t x,                   // scalar to assign to C(I,J)
    const GrB_Index *I,             // row indices
    GrB_Index ni,                   // number of row indices
    const GrB_Index *J,             // column indices
    GrB_Index nj,                   // number of column indices
    const GrB_Descriptor desc       // descriptor for C and Mask
) ;

GB_PUBLIC
GrB_Info GxB_Matrix_assign_FC64     // C<Mask>(I,J) = accum (C(I,J),x)
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Matrix Mask,          // optional mask for C, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C(I,J),x)
    GxB_FC64_t x,                   // scalar to assign to C(I,J)
    const GrB_Index *I,             // row indices
    GrB_Index ni,                   // number of row indices
    const GrB_Index *J,             // column indices
    GrB_Index nj,                   // number of column indices
    const GrB_Descriptor desc       // descriptor for C and Mask
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_assign_UDT      // C<Mask>(I,J) = accum (C(I,J),x)
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Matrix Mask,          // optional mask for C, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C(I,J),x)
    void *x,                        // scalar to assign to C(I,J)
    const GrB_Index *I,             // row indices
    GrB_Index ni,                   // number of row indices
    const GrB_Index *J,             // column indices
    GrB_Index nj,                   // number of column indices
    const GrB_Descriptor desc       // descriptor for C and Mask
) ;

//------------------------------------------------------------------------------
// GrB_assign: generic submatrix/subvector assignment
//------------------------------------------------------------------------------

// GrB_assign is a generic function that provides access to all specific
// GrB_*_assign* functions:

// GrB_Vector_assign   (w,m,acc,u,I,ni,d)      // w<m>(I)    = acc(w(I),u)
// GrB_Matrix_assign   (C,M,acc,A,I,ni,J,nj,d) // C<M>(I,J)  = acc(C(I,J),A)
// GrB_Col_assign      (C,m,acc,u,I,ni,j,d)    // C<m>(I,j)  = acc(C(I,j),u)
// GrB_Row_assign      (C,m,acc,u,i,J,nj,d)    // C<m'>(i,J) = acc(C(i,J),u')
// GrB_Vector_assign_T (w,m,acc,x,I,ni,d)      // w<m>(I)    = acc(w(I),x)
// GrB_Matrix_assign_T (C,M,acc,x,I,ni,J,nj,d) // C<M>(I,J)  = acc(C(I,J),x)

#if GxB_STDC_VERSION >= 201112L
#define GrB_assign(arg1,Mask,accum,arg4,arg5,...)               \
    _Generic                                                    \
    (                                                           \
        (arg1),                                                 \
        GrB_Vector :                                            \
            _Generic                                            \
            (                                                   \
                (arg4),                                         \
                GB_(, GrB, Vector_assign) ,                     \
                default : GrB_Vector_assign                     \
            ),                                                  \
        default :                                               \
            _Generic                                            \
            (                                                   \
                (arg4),                                         \
                GB_(, GrB, Matrix_assign) ,                     \
                const GrB_Vector :                              \
                    _Generic                                    \
                    (                                           \
                        (arg5),                                 \
                        const GrB_Index *: GrB_Col_assign ,     \
                              GrB_Index *: GrB_Col_assign ,     \
                        default          : GrB_Row_assign       \
                    ),                                          \
                GrB_Vector :                                    \
                    _Generic                                    \
                    (                                           \
                        (arg5),                                 \
                        const GrB_Index *: GrB_Col_assign ,     \
                              GrB_Index *: GrB_Col_assign ,     \
                        default          : GrB_Row_assign       \
                    ),                                          \
                default    : GrB_Matrix_assign                  \
            )                                                   \
    )                                                           \
    (arg1, Mask, accum, arg4, arg5, __VA_ARGS__)
#endif

//==============================================================================
// GrB_apply: matrix and vector apply
//==============================================================================

// Apply a unary operator to entries in a matrix or vector,
// C<M> = accum (C, op (A)).

GB_PUBLIC
GrB_Info GrB_Vector_apply           // w<mask> = accum (w, op(u))
(
    GrB_Vector w,                   // input/output vector for results
    const GrB_Vector mask,          // optional mask for w, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for z=accum(w,t)
    const GrB_UnaryOp op,           // operator to apply to the entries
    const GrB_Vector u,             // first input:  vector u
    const GrB_Descriptor desc       // descriptor for w and mask
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_apply           // C<Mask> = accum (C, op(A)) or op(A')
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Matrix Mask,          // optional mask for C, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C,T)
    const GrB_UnaryOp op,           // operator to apply to the entries
    const GrB_Matrix A,             // first input:  matrix A
    const GrB_Descriptor desc       // descriptor for C, mask, and A
) ;

//-------------------------------------------
// vector apply: binaryop variants (bind 1st)
//-------------------------------------------

// Apply a binary operator to the entries in a vector, binding the first
// input to a scalar x, w<mask> = accum (w, op (x,u)).

GB_PUBLIC
GrB_Info GxB_Vector_apply_BinaryOp1st           // w<mask> = accum (w, op(x,u))
(
    GrB_Vector w,                   // input/output vector for results
    const GrB_Vector mask,          // optional mask for w, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for z=accum(w,t)
    const GrB_BinaryOp op,          // operator to apply to the entries
    const GxB_Scalar x,             // first input:  scalar x
    const GrB_Vector u,             // second input: vector u
    const GrB_Descriptor desc       // descriptor for w and mask
) ;

GB_PUBLIC
GrB_Info GrB_Vector_apply_BinaryOp1st_BOOL      // w<mask> = accum (w, op(x,u))
(
    GrB_Vector w,                   // input/output vector for results
    const GrB_Vector mask,          // optional mask for w, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for z=accum(w,t)
    const GrB_BinaryOp op,          // operator to apply to the entries
    bool x,                         // first input:  scalar x
    const GrB_Vector u,             // second input: vector u
    const GrB_Descriptor desc       // descriptor for w and mask
) ;

GB_PUBLIC
GrB_Info GrB_Vector_apply_BinaryOp1st_INT8      // w<mask> = accum (w, op(x,u))
(
    GrB_Vector w,                   // input/output vector for results
    const GrB_Vector mask,          // optional mask for w, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for z=accum(w,t)
    const GrB_BinaryOp op,          // operator to apply to the entries
    int8_t x,                       // first input:  scalar x
    const GrB_Vector u,             // second input: vector u
    const GrB_Descriptor desc       // descriptor for w and mask
) ;

GB_PUBLIC
GrB_Info GrB_Vector_apply_BinaryOp1st_INT16     // w<mask> = accum (w, op(x,u))
(
    GrB_Vector w,                   // input/output vector for results
    const GrB_Vector mask,          // optional mask for w, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for z=accum(w,t)
    const GrB_BinaryOp op,          // operator to apply to the entries
    int16_t x,                      // first input:  scalar x
    const GrB_Vector u,             // second input: vector u
    const GrB_Descriptor desc       // descriptor for w and mask
) ;

GB_PUBLIC
GrB_Info GrB_Vector_apply_BinaryOp1st_INT32     // w<mask> = accum (w, op(x,u))
(
    GrB_Vector w,                   // input/output vector for results
    const GrB_Vector mask,          // optional mask for w, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for z=accum(w,t)
    const GrB_BinaryOp op,          // operator to apply to the entries
    int32_t x,                      // first input:  scalar x
    const GrB_Vector u,             // second input: vector u
    const GrB_Descriptor desc       // descriptor for w and mask
) ;

GB_PUBLIC
GrB_Info GrB_Vector_apply_BinaryOp1st_INT64     // w<mask> = accum (w, op(x,u))
(
    GrB_Vector w,                   // input/output vector for results
    const GrB_Vector mask,          // optional mask for w, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for z=accum(w,t)
    const GrB_BinaryOp op,          // operator to apply to the entries
    int64_t x,                      // first input:  scalar x
    const GrB_Vector u,             // second input: vector u
    const GrB_Descriptor desc       // descriptor for w and mask
) ;

GB_PUBLIC
GrB_Info GrB_Vector_apply_BinaryOp1st_UINT8     // w<mask> = accum (w, op(x,u))
(
    GrB_Vector w,                   // input/output vector for results
    const GrB_Vector mask,          // optional mask for w, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for z=accum(w,t)
    const GrB_BinaryOp op,          // operator to apply to the entries
    uint8_t x,                      // first input:  scalar x
    const GrB_Vector u,             // second input: vector u
    const GrB_Descriptor desc       // descriptor for w and mask
) ;

GB_PUBLIC
GrB_Info GrB_Vector_apply_BinaryOp1st_UINT16    // w<mask> = accum (w, op(x,u))
(
    GrB_Vector w,                   // input/output vector for results
    const GrB_Vector mask,          // optional mask for w, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for z=accum(w,t)
    const GrB_BinaryOp op,          // operator to apply to the entries
    uint16_t x,                     // first input:  scalar x
    const GrB_Vector u,             // second input: vector u
    const GrB_Descriptor desc       // descriptor for w and mask
) ;

GB_PUBLIC
GrB_Info GrB_Vector_apply_BinaryOp1st_UINT32    // w<mask> = accum (w, op(x,u))
(
    GrB_Vector w,                   // input/output vector for results
    const GrB_Vector mask,          // optional mask for w, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for z=accum(w,t)
    const GrB_BinaryOp op,          // operator to apply to the entries
    uint32_t x,                     // first input:  scalar x
    const GrB_Vector u,             // second input: vector u
    const GrB_Descriptor desc       // descriptor for w and mask
) ;

GB_PUBLIC
GrB_Info GrB_Vector_apply_BinaryOp1st_UINT64    // w<mask> = accum (w, op(x,u))
(
    GrB_Vector w,                   // input/output vector for results
    const GrB_Vector mask,          // optional mask for w, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for z=accum(w,t)
    const GrB_BinaryOp op,          // operator to apply to the entries
    uint64_t x,                     // first input:  scalar x
    const GrB_Vector u,             // second input: vector u
    const GrB_Descriptor desc       // descriptor for w and mask
) ;

GB_PUBLIC
GrB_Info GrB_Vector_apply_BinaryOp1st_FP32      // w<mask> = accum (w, op(x,u))
(
    GrB_Vector w,                   // input/output vector for results
    const GrB_Vector mask,          // optional mask for w, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for z=accum(w,t)
    const GrB_BinaryOp op,          // operator to apply to the entries
    float x,                        // first input:  scalar x
    const GrB_Vector u,             // second input: vector u
    const GrB_Descriptor desc       // descriptor for w and mask
) ;

GB_PUBLIC
GrB_Info GrB_Vector_apply_BinaryOp1st_FP64      // w<mask> = accum (w, op(x,u))
(
    GrB_Vector w,                   // input/output vector for results
    const GrB_Vector mask,          // optional mask for w, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for z=accum(w,t)
    const GrB_BinaryOp op,          // operator to apply to the entries
    double x,                       // first input:  scalar x
    const GrB_Vector u,             // second input: vector u
    const GrB_Descriptor desc       // descriptor for w and mask
) ;

GB_PUBLIC
GrB_Info GxB_Vector_apply_BinaryOp1st_FC32      // w<mask> = accum (w, op(x,u))
(
    GrB_Vector w,                   // input/output vector for results
    const GrB_Vector mask,          // optional mask for w, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for z=accum(w,t)
    const GrB_BinaryOp op,          // operator to apply to the entries
    GxB_FC32_t x,                   // first input:  scalar x
    const GrB_Vector u,             // second input: vector u
    const GrB_Descriptor desc       // descriptor for w and mask
) ;

GB_PUBLIC
GrB_Info GxB_Vector_apply_BinaryOp1st_FC64      // w<mask> = accum (w, op(x,u))
(
    GrB_Vector w,                   // input/output vector for results
    const GrB_Vector mask,          // optional mask for w, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for z=accum(w,t)
    const GrB_BinaryOp op,          // operator to apply to the entries
    GxB_FC64_t x,                   // first input:  scalar x
    const GrB_Vector u,             // second input: vector u
    const GrB_Descriptor desc       // descriptor for w and mask
) ;

GB_PUBLIC
GrB_Info GrB_Vector_apply_BinaryOp1st_UDT       // w<mask> = accum (w, op(x,u))
(
    GrB_Vector w,                   // input/output vector for results
    const GrB_Vector mask,          // optional mask for w, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for z=accum(w,t)
    const GrB_BinaryOp op,          // operator to apply to the entries
    const void *x,                  // first input:  scalar x
    const GrB_Vector u,             // second input: vector u
    const GrB_Descriptor desc       // descriptor for w and mask
) ;

//-------------------------------------------
// vector apply: binaryop variants (bind 2nd)
//-------------------------------------------

// Apply a binary operator to the entries in a vector, binding the second
// input to a scalar y, w<mask> = accum (w, op (u,y)).

GB_PUBLIC
GrB_Info GxB_Vector_apply_BinaryOp2nd           // w<mask> = accum (w, op(u,y))
(
    GrB_Vector w,                   // input/output vector for results
    const GrB_Vector mask,          // optional mask for w, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for z=accum(w,t)
    const GrB_BinaryOp op,          // operator to apply to the entries
    const GrB_Vector u,             // first input:  vector u
    const GxB_Scalar y,             // second input: scalar y
    const GrB_Descriptor desc       // descriptor for w and mask
) ;

GB_PUBLIC
GrB_Info GrB_Vector_apply_BinaryOp2nd_BOOL      // w<mask> = accum (w, op(u,y))
(
    GrB_Vector w,                   // input/output vector for results
    const GrB_Vector mask,          // optional mask for w, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for z=accum(w,t)
    const GrB_BinaryOp op,          // operator to apply to the entries
    const GrB_Vector u,             // first input:  vector u
    bool y,                         // second input: scalar y
    const GrB_Descriptor desc       // descriptor for w and mask
) ;

GB_PUBLIC
GrB_Info GrB_Vector_apply_BinaryOp2nd_INT8      // w<mask> = accum (w, op(u,y))
(
    GrB_Vector w,                   // input/output vector for results
    const GrB_Vector mask,          // optional mask for w, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for z=accum(w,t)
    const GrB_BinaryOp op,          // operator to apply to the entries
    const GrB_Vector u,             // first input:  vector u
    int8_t y,                       // second input: scalar y
    const GrB_Descriptor desc       // descriptor for w and mask
) ;

GB_PUBLIC
GrB_Info GrB_Vector_apply_BinaryOp2nd_INT16     // w<mask> = accum (w, op(u,y))
(
    GrB_Vector w,                   // input/output vector for results
    const GrB_Vector mask,          // optional mask for w, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for z=accum(w,t)
    const GrB_BinaryOp op,          // operator to apply to the entries
    const GrB_Vector u,             // first input:  vector u
    int16_t y,                      // second input: scalar y
    const GrB_Descriptor desc       // descriptor for w and mask
) ;

GB_PUBLIC
GrB_Info GrB_Vector_apply_BinaryOp2nd_INT32     // w<mask> = accum (w, op(u,y))
(
    GrB_Vector w,                   // input/output vector for results
    const GrB_Vector mask,          // optional mask for w, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for z=accum(w,t)
    const GrB_BinaryOp op,          // operator to apply to the entries
    const GrB_Vector u,             // first input:  vector u
    int32_t y,                      // second input: scalar y
    const GrB_Descriptor desc       // descriptor for w and mask
) ;

GB_PUBLIC
GrB_Info GrB_Vector_apply_BinaryOp2nd_INT64     // w<mask> = accum (w, op(u,y))
(
    GrB_Vector w,                   // input/output vector for results
    const GrB_Vector mask,          // optional mask for w, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for z=accum(w,t)
    const GrB_BinaryOp op,          // operator to apply to the entries
    const GrB_Vector u,             // first input:  vector u
    int64_t y,                      // second input: scalar y
    const GrB_Descriptor desc       // descriptor for w and mask
) ;

GB_PUBLIC
GrB_Info GrB_Vector_apply_BinaryOp2nd_UINT8     // w<mask> = accum (w, op(u,y))
(
    GrB_Vector w,                   // input/output vector for results
    const GrB_Vector mask,          // optional mask for w, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for z=accum(w,t)
    const GrB_BinaryOp op,          // operator to apply to the entries
    const GrB_Vector u,             // first input:  vector u
    uint8_t y,                      // second input: scalar y
    const GrB_Descriptor desc       // descriptor for w and mask
) ;

GB_PUBLIC
GrB_Info GrB_Vector_apply_BinaryOp2nd_UINT16    // w<mask> = accum (w, op(u,y))
(
    GrB_Vector w,                   // input/output vector for results
    const GrB_Vector mask,          // optional mask for w, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for z=accum(w,t)
    const GrB_BinaryOp op,          // operator to apply to the entries
    const GrB_Vector u,             // first input:  vector u
    uint16_t y,                     // second input: scalar y
    const GrB_Descriptor desc       // descriptor for w and mask
) ;

GB_PUBLIC
GrB_Info GrB_Vector_apply_BinaryOp2nd_UINT32    // w<mask> = accum (w, op(u,y))
(
    GrB_Vector w,                   // input/output vector for results
    const GrB_Vector mask,          // optional mask for w, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for z=accum(w,t)
    const GrB_BinaryOp op,          // operator to apply to the entries
    const GrB_Vector u,             // first input:  vector u
    uint32_t y,                     // second input: scalar y
    const GrB_Descriptor desc       // descriptor for w and mask
) ;

GB_PUBLIC
GrB_Info GrB_Vector_apply_BinaryOp2nd_UINT64    // w<mask> = accum (w, op(u,y))
(
    GrB_Vector w,                   // input/output vector for results
    const GrB_Vector mask,          // optional mask for w, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for z=accum(w,t)
    const GrB_BinaryOp op,          // operator to apply to the entries
    const GrB_Vector u,             // first input:  vector u
    uint64_t y,                     // second input: scalar y
    const GrB_Descriptor desc       // descriptor for w and mask
) ;

GB_PUBLIC
GrB_Info GrB_Vector_apply_BinaryOp2nd_FP32      // w<mask> = accum (w, op(u,y))
(
    GrB_Vector w,                   // input/output vector for results
    const GrB_Vector mask,          // optional mask for w, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for z=accum(w,t)
    const GrB_BinaryOp op,          // operator to apply to the entries
    const GrB_Vector u,             // first input:  vector u
    float y,                        // second input: scalar y
    const GrB_Descriptor desc       // descriptor for w and mask
) ;

GB_PUBLIC
GrB_Info GrB_Vector_apply_BinaryOp2nd_FP64      // w<mask> = accum (w, op(u,y))
(
    GrB_Vector w,                   // input/output vector for results
    const GrB_Vector mask,          // optional mask for w, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for z=accum(w,t)
    const GrB_BinaryOp op,          // operator to apply to the entries
    const GrB_Vector u,             // first input:  vector u
    double y,                       // second input: scalar y
    const GrB_Descriptor desc       // descriptor for w and mask
) ;

GB_PUBLIC
GrB_Info GxB_Vector_apply_BinaryOp2nd_FC32      // w<mask> = accum (w, op(u,y))
(
    GrB_Vector w,                   // input/output vector for results
    const GrB_Vector mask,          // optional mask for w, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for z=accum(w,t)
    const GrB_BinaryOp op,          // operator to apply to the entries
    const GrB_Vector u,             // first input:  vector u
    GxB_FC32_t y,                   // second input: scalar y
    const GrB_Descriptor desc       // descriptor for w and mask
) ;

GB_PUBLIC
GrB_Info GxB_Vector_apply_BinaryOp2nd_FC64      // w<mask> = accum (w, op(u,y))
(
    GrB_Vector w,                   // input/output vector for results
    const GrB_Vector mask,          // optional mask for w, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for z=accum(w,t)
    const GrB_BinaryOp op,          // operator to apply to the entries
    const GrB_Vector u,             // first input:  vector u
    GxB_FC64_t y,                   // second input: scalar y
    const GrB_Descriptor desc       // descriptor for w and mask
) ;

GB_PUBLIC
GrB_Info GrB_Vector_apply_BinaryOp2nd_UDT       // w<mask> = accum (w, op(u,y))
(
    GrB_Vector w,                   // input/output vector for results
    const GrB_Vector mask,          // optional mask for w, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for z=accum(w,t)
    const GrB_BinaryOp op,          // operator to apply to the entries
    const GrB_Vector u,             // first input:  vector u
    const void *y,                  // second input: scalar y
    const GrB_Descriptor desc       // descriptor for w and mask
) ;

//-------------------------------------------
// matrix apply: binaryop variants (bind 1st)
//-------------------------------------------

// Apply a binary operator to the entries in a matrix, binding the first input
// to a scalar x, C<Mask> = accum (C, op (x,A)), or op(x,A').

GB_PUBLIC
GrB_Info GxB_Matrix_apply_BinaryOp1st           // C<M>=accum(C,op(x,A))
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Matrix Mask,          // optional mask for C, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C,T)
    const GrB_BinaryOp op,          // operator to apply to the entries
    const GxB_Scalar x,             // first input:  scalar x
    const GrB_Matrix A,             // second input: matrix A
    const GrB_Descriptor desc       // descriptor for C, mask, and A
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_apply_BinaryOp1st_BOOL      // C<M>=accum(C,op(x,A))
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Matrix Mask,          // optional mask for C, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C,T)
    const GrB_BinaryOp op,          // operator to apply to the entries
    bool x,                         // first input:  scalar x
    const GrB_Matrix A,             // second input: matrix A
    const GrB_Descriptor desc       // descriptor for C, mask, and A
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_apply_BinaryOp1st_INT8      // C<M>=accum(C,op(x,A))
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Matrix Mask,          // optional mask for C, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C,T)
    const GrB_BinaryOp op,          // operator to apply to the entries
    int8_t x,                       // first input:  scalar x
    const GrB_Matrix A,             // second input: matrix A
    const GrB_Descriptor desc       // descriptor for C, mask, and A
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_apply_BinaryOp1st_INT16     // C<M>=accum(C,op(x,A))
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Matrix Mask,          // optional mask for C, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C,T)
    const GrB_BinaryOp op,          // operator to apply to the entries
    int16_t x,                      // first input:  scalar x
    const GrB_Matrix A,             // second input: matrix A
    const GrB_Descriptor desc       // descriptor for C, mask, and A
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_apply_BinaryOp1st_INT32     // C<M>=accum(C,op(x,A))
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Matrix Mask,          // optional mask for C, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C,T)
    const GrB_BinaryOp op,          // operator to apply to the entries
    int32_t x,                      // first input:  scalar x
    const GrB_Matrix A,             // second input: matrix A
    const GrB_Descriptor desc       // descriptor for C, mask, and A
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_apply_BinaryOp1st_INT64     // C<M>=accum(C,op(x,A))
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Matrix Mask,          // optional mask for C, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C,T)
    const GrB_BinaryOp op,          // operator to apply to the entries
    int64_t x,                      // first input:  scalar x
    const GrB_Matrix A,             // second input: matrix A
    const GrB_Descriptor desc       // descriptor for C, mask, and A
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_apply_BinaryOp1st_UINT8      // C<M>=accum(C,op(x,A))
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Matrix Mask,          // optional mask for C, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C,T)
    const GrB_BinaryOp op,          // operator to apply to the entries
    uint8_t x,                      // first input:  scalar x
    const GrB_Matrix A,             // second input: matrix A
    const GrB_Descriptor desc       // descriptor for C, mask, and A
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_apply_BinaryOp1st_UINT16     // C<M>=accum(C,op(x,A))
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Matrix Mask,          // optional mask for C, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C,T)
    const GrB_BinaryOp op,          // operator to apply to the entries
    uint16_t x,                     // first input:  scalar x
    const GrB_Matrix A,             // second input: matrix A
    const GrB_Descriptor desc       // descriptor for C, mask, and A
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_apply_BinaryOp1st_UINT32     // C<M>=accum(C,op(x,A))
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Matrix Mask,          // optional mask for C, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C,T)
    const GrB_BinaryOp op,          // operator to apply to the entries
    uint32_t x,                     // first input:  scalar x
    const GrB_Matrix A,             // second input: matrix A
    const GrB_Descriptor desc       // descriptor for C, mask, and A
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_apply_BinaryOp1st_UINT64     // C<M>=accum(C,op(x,A))
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Matrix Mask,          // optional mask for C, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C,T)
    const GrB_BinaryOp op,          // operator to apply to the entries
    uint64_t x,                     // first input:  scalar x
    const GrB_Matrix A,             // second input: matrix A
    const GrB_Descriptor desc       // descriptor for C, mask, and A
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_apply_BinaryOp1st_FP32      // C<M>=accum(C,op(x,A))
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Matrix Mask,          // optional mask for C, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C,T)
    const GrB_BinaryOp op,          // operator to apply to the entries
    float x,                        // first input:  scalar x
    const GrB_Matrix A,             // second input: matrix A
    const GrB_Descriptor desc       // descriptor for C, mask, and A
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_apply_BinaryOp1st_FP64      // C<M>=accum(C,op(x,A))
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Matrix Mask,          // optional mask for C, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C,T)
    const GrB_BinaryOp op,          // operator to apply to the entries
    double x,                       // first input:  scalar x
    const GrB_Matrix A,             // second input: matrix A
    const GrB_Descriptor desc       // descriptor for C, mask, and A
) ;

GB_PUBLIC
GrB_Info GxB_Matrix_apply_BinaryOp1st_FC32      // C<M>=accum(C,op(x,A))
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Matrix Mask,          // optional mask for C, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C,T)
    const GrB_BinaryOp op,          // operator to apply to the entries
    GxB_FC32_t x,                   // first input:  scalar x
    const GrB_Matrix A,             // second input: matrix A
    const GrB_Descriptor desc       // descriptor for C, mask, and A
) ;

GB_PUBLIC
GrB_Info GxB_Matrix_apply_BinaryOp1st_FC64      // C<M>=accum(C,op(x,A))
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Matrix Mask,          // optional mask for C, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C,T)
    const GrB_BinaryOp op,          // operator to apply to the entries
    GxB_FC64_t x,                   // first input:  scalar x
    const GrB_Matrix A,             // second input: matrix A
    const GrB_Descriptor desc       // descriptor for C, mask, and A
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_apply_BinaryOp1st_UDT       // C<M>=accum(C,op(x,A))
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Matrix Mask,          // optional mask for C, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C,T)
    const GrB_BinaryOp op,          // operator to apply to the entries
    const void *x,                  // first input:  scalar x
    const GrB_Matrix A,             // second input: matrix A
    const GrB_Descriptor desc       // descriptor for C, mask, and A
) ;

//-------------------------------------------
// matrix apply: binaryop variants (bind 2nd)
//-------------------------------------------

// Apply a binary operator to the entries in a matrix, binding the second input
// to a scalar y, C<Mask> = accum (C, op (A,y)), or op(A',y).

GB_PUBLIC
GrB_Info GxB_Matrix_apply_BinaryOp2nd           // C<M>=accum(C,op(x,A))
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Matrix Mask,          // optional mask for C, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C,T)
    const GrB_BinaryOp op,          // operator to apply to the entries
    const GrB_Matrix A,             // first input:  matrix A
    const GxB_Scalar y,             // second input: scalar y
    const GrB_Descriptor desc       // descriptor for C, mask, and A
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_apply_BinaryOp2nd_BOOL      // C<M>=accum(C,op(x,A))
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Matrix Mask,          // optional mask for C, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C,T)
    const GrB_BinaryOp op,          // operator to apply to the entries
    const GrB_Matrix A,             // first input:  matrix A
    bool y,                         // second input: scalar y
    const GrB_Descriptor desc       // descriptor for C, mask, and A
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_apply_BinaryOp2nd_INT8      // C<M>=accum(C,op(x,A))
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Matrix Mask,          // optional mask for C, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C,T)
    const GrB_BinaryOp op,          // operator to apply to the entries
    const GrB_Matrix A,             // first input:  matrix A
    int8_t y,                       // second input: scalar y
    const GrB_Descriptor desc       // descriptor for C, mask, and A
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_apply_BinaryOp2nd_INT16     // C<M>=accum(C,op(x,A))
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Matrix Mask,          // optional mask for C, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C,T)
    const GrB_BinaryOp op,          // operator to apply to the entries
    const GrB_Matrix A,             // first input:  matrix A
    int16_t y,                      // second input: scalar y
    const GrB_Descriptor desc       // descriptor for C, mask, and A
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_apply_BinaryOp2nd_INT32     // C<M>=accum(C,op(x,A))
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Matrix Mask,          // optional mask for C, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C,T)
    const GrB_BinaryOp op,          // operator to apply to the entries
    const GrB_Matrix A,             // first input:  matrix A
    int32_t y,                      // second input: scalar y
    const GrB_Descriptor desc       // descriptor for C, mask, and A
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_apply_BinaryOp2nd_INT64     // C<M>=accum(C,op(x,A))
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Matrix Mask,          // optional mask for C, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C,T)
    const GrB_BinaryOp op,          // operator to apply to the entries
    const GrB_Matrix A,             // first input:  matrix A
    int64_t y,                      // second input: scalar y
    const GrB_Descriptor desc       // descriptor for C, mask, and A
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_apply_BinaryOp2nd_UINT8      // C<M>=accum(C,op(x,A))
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Matrix Mask,          // optional mask for C, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C,T)
    const GrB_BinaryOp op,          // operator to apply to the entries
    const GrB_Matrix A,             // first input:  matrix A
    uint8_t y,                      // second input: scalar y
    const GrB_Descriptor desc       // descriptor for C, mask, and A
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_apply_BinaryOp2nd_UINT16     // C<M>=accum(C,op(x,A))
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Matrix Mask,          // optional mask for C, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C,T)
    const GrB_BinaryOp op,          // operator to apply to the entries
    const GrB_Matrix A,             // first input:  matrix A
    uint16_t y,                     // second input: scalar y
    const GrB_Descriptor desc       // descriptor for C, mask, and A
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_apply_BinaryOp2nd_UINT32     // C<M>=accum(C,op(x,A))
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Matrix Mask,          // optional mask for C, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C,T)
    const GrB_BinaryOp op,          // operator to apply to the entries
    const GrB_Matrix A,             // first input:  matrix A
    uint32_t y,                     // second input: scalar y
    const GrB_Descriptor desc       // descriptor for C, mask, and A
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_apply_BinaryOp2nd_UINT64     // C<M>=accum(C,op(x,A))
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Matrix Mask,          // optional mask for C, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C,T)
    const GrB_BinaryOp op,          // operator to apply to the entries
    const GrB_Matrix A,             // first input:  matrix A
    uint64_t y,                     // second input: scalar y
    const GrB_Descriptor desc       // descriptor for C, mask, and A
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_apply_BinaryOp2nd_FP32      // C<M>=accum(C,op(x,A))
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Matrix Mask,          // optional mask for C, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C,T)
    const GrB_BinaryOp op,          // operator to apply to the entries
    const GrB_Matrix A,             // first input:  matrix A
    float y,                        // second input: scalar y
    const GrB_Descriptor desc       // descriptor for C, mask, and A
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_apply_BinaryOp2nd_FP64      // C<M>=accum(C,op(x,A))
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Matrix Mask,          // optional mask for C, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C,T)
    const GrB_BinaryOp op,          // operator to apply to the entries
    const GrB_Matrix A,             // first input:  matrix A
    double y,                       // second input: scalar y
    const GrB_Descriptor desc       // descriptor for C, mask, and A
) ;

GB_PUBLIC
GrB_Info GxB_Matrix_apply_BinaryOp2nd_FC32      // C<M>=accum(C,op(x,A))
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Matrix Mask,          // optional mask for C, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C,T)
    const GrB_BinaryOp op,          // operator to apply to the entries
    const GrB_Matrix A,             // first input:  matrix A
    GxB_FC32_t y,                   // second input: scalar y
    const GrB_Descriptor desc       // descriptor for C, mask, and A
) ;

GB_PUBLIC
GrB_Info GxB_Matrix_apply_BinaryOp2nd_FC64      // C<M>=accum(C,op(x,A))
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Matrix Mask,          // optional mask for C, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C,T)
    const GrB_BinaryOp op,          // operator to apply to the entries
    const GrB_Matrix A,             // first input:  matrix A
    GxB_FC64_t y,                   // second input: scalar y
    const GrB_Descriptor desc       // descriptor for C, mask, and A
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_apply_BinaryOp2nd_UDT       // C<M>=accum(C,op(x,A))
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Matrix Mask,          // optional mask for C, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C,T)
    const GrB_BinaryOp op,          // operator to apply to the entries
    const GrB_Matrix A,             // first input:  matrix A
    const void *y,                  // second input: scalar y
    const GrB_Descriptor desc       // descriptor for C, mask, and A
) ;

//------------------------------------------------------------------------------
// GrB_apply: generic matrix/vector apply
//------------------------------------------------------------------------------

// GrB_apply is a generic function for applying a unary operator to a matrix
// or vector and provides access to these functions:

// GrB_Vector_apply (w,mask,acc,op,u,d)  // w<mask> = accum (w, op(u))
// GrB_Matrix_apply (C,Mask,acc,op,A,d)  // C<Mask> = accum (C, op(A))

// GrB_Vector_apply                  (w,m,acc,unop ,u,d)
// GxB_Vector_apply_BinaryOp1st      (w,m,acc,binop,x,u,d)
// GrB_Vector_apply_BinaryOp1st_TYPE (w,m,acc,binop,x,u,d)
// GxB_Vector_apply_BinaryOp2nd      (w,m,acc,binop,u,y,d)
// GrB_Vector_apply_BinaryOp2nd_TYPE (w,m,acc,binop,u,y,d)

// GrB_Matrix_apply                  (C,M,acc,unop ,A,d)
// GxB_Matrix_apply_BinaryOp1st      (C,M,acc,binop,x,A,d)
// GrB_Matrix_apply_BinaryOp1st_TYPE (C,M,acc,binop,x,A,d)
// GxB_Matrix_apply_BinaryOp2nd      (C,M,acc,binop,A,y,d)
// GrB_Matrix_apply_BinaryOp2nd_TYPE (C,M,acc,binop,A,y,d)

#if GxB_STDC_VERSION >= 201112L

#define GB_BIND(kind,x,y,...)                                               \
    _Generic                                                                \
    (                                                                       \
        (x),                                                                \
        GxB_Scalar: GxB_ ## kind ## _apply_BinaryOp1st  ,                   \
              GB_(, GrB,    kind ## _apply_BinaryOp1st) ,                   \
        default :                                                           \
            _Generic                                                        \
            (                                                               \
                (y),                                                        \
                default : GxB_ ## kind ## _apply_BinaryOp2nd  ,             \
                    GB_(, GrB,    kind ## _apply_BinaryOp2nd)               \
            )                                                               \
    )

#define GrB_apply(C,Mask,accum,op,...)                                      \
    _Generic                                                                \
    (                                                                       \
        (C),                                                                \
        GrB_Vector :                                                        \
            _Generic                                                        \
            (                                                               \
                (op),                                                       \
                GrB_UnaryOp  : GrB_Vector_apply ,                           \
                GrB_BinaryOp : GB_BIND (Vector, __VA_ARGS__)                \
            ),                                                              \
        GrB_Matrix :                                                        \
            _Generic                                                        \
            (                                                               \
                (op),                                                       \
                GrB_UnaryOp  : GrB_Matrix_apply ,                           \
                GrB_BinaryOp : GB_BIND (Matrix, __VA_ARGS__)                \
            )                                                               \
    )                                                                       \
    (C, Mask, accum, op, __VA_ARGS__)
#endif

//==============================================================================
// GxB_select: matrix and vector selection
//==============================================================================

// Select a subset of entries from a matrix or vector.
// C<Mask> = accum (C, op (A,k)), where the entries of op(A,k) are a subset of
// the entries of A.

GB_PUBLIC
GrB_Info GxB_Vector_select          // w<mask> = accum (w, op(u,k))
(
    GrB_Vector w,                   // input/output vector for results
    const GrB_Vector mask,          // optional mask for w, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for z=accum(w,t)
    const GxB_SelectOp op,          // operator to apply to the entries
    const GrB_Vector u,             // first input:  vector u
    const GxB_Scalar Thunk,         // optional input for the select operator
    const GrB_Descriptor desc       // descriptor for w and mask
) ;

GB_PUBLIC
GrB_Info GxB_Matrix_select          // C<Mask> = accum (C, op(A,k)) or op(A',k)
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Matrix Mask,          // optional mask for C, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C,T)
    const GxB_SelectOp op,          // operator to apply to the entries
    const GrB_Matrix A,             // first input:  matrix A
    const GxB_Scalar Thunk,         // optional input for the select operator
    const GrB_Descriptor desc       // descriptor for C, mask, and A
) ;

//------------------------------------------------------------------------------
// GxB_select: generic matrix/vector select
//------------------------------------------------------------------------------

// GxB_select is a generic function for applying a select operator to a matrix
// or vector and provides access to these functions:

// GrB_Vector_select (w,mask,acc,op,u,k,d)  // w<mask> = accum (w, op(u,k))
// GrB_Matrix_select (C,Mask,acc,op,A,k,d)  // C<Mask> = accum (C, op(A,k))

#if GxB_STDC_VERSION >= 201112L
#define GxB_select(C,Mask,accum,op,A,Thunk,desc)    \
    _Generic                                        \
    (                                               \
        (C),                                        \
        GrB_Vector   : GxB_Vector_select ,          \
        GrB_Matrix   : GxB_Matrix_select            \
    )                                               \
    (C, Mask, accum, op, A, Thunk, desc)
#endif

//==============================================================================
// GrB_reduce: matrix and vector reduction
//==============================================================================

// Reduce the entries in a matrix to a vector, a column vector t such that
// t(i) = sum (A (i,:)), and where "sum" is a commutative and associative
// monoid with an identity value.  A can be transposed, which reduces down the
// columns instead of the rows.  This behavior is the transpose of the MATLAB
// convention, where r=sum(A) produces a row vector and sums each column.

// For GrB_Matrix_reduce_BinaryOp, the GrB_BinaryOp op must correspond to a
// known built-in GrB_Monoid.  User-defined binary operators are not supported.
// the use of GrB_Matrix_reduce_BinaryOp is discouraged; use GrB_reduce with a
// monoid instead.

GB_PUBLIC
GrB_Info GrB_Matrix_reduce_Monoid   // w<mask> = accum (w,reduce(A))
(
    GrB_Vector w,                   // input/output vector for results
    const GrB_Vector mask,          // optional mask for w, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for z=accum(w,t)
    const GrB_Monoid monoid,        // reduce operator for t=reduce(A)
    const GrB_Matrix A,             // first input:  matrix A
    const GrB_Descriptor desc       // descriptor for w, mask, and A
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_reduce_BinaryOp // w<mask> = accum (w,reduce(A))
(
    GrB_Vector w,                   // input/output vector for results
    const GrB_Vector mask,          // optional mask for w, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for z=accum(w,t)
    const GrB_BinaryOp op,          // reduce operator for t=reduce(A)
    const GrB_Matrix A,             // first input:  matrix A
    const GrB_Descriptor desc       // descriptor for w, mask, and A
) ;

//------------------------------------------------------------------------------
// reduce a vector to a scalar
//------------------------------------------------------------------------------

// Reduce entries in a vector to a scalar, c = accum (c, reduce_to_scalar(u))

GB_PUBLIC
GrB_Info GrB_Vector_reduce_BOOL     // c = accum (c, reduce_to_scalar (u))
(
    bool *c,                        // result scalar
    const GrB_BinaryOp accum,       // optional accum for c=accum(c,t)
    const GrB_Monoid monoid,        // monoid to do the reduction
    const GrB_Vector u,             // vector to reduce
    const GrB_Descriptor desc       // descriptor (currently unused)
) ;

GB_PUBLIC
GrB_Info GrB_Vector_reduce_INT8     // c = accum (c, reduce_to_scalar (u))
(
    int8_t *c,                      // result scalar
    const GrB_BinaryOp accum,       // optional accum for c=accum(c,t)
    const GrB_Monoid monoid,        // monoid to do the reduction
    const GrB_Vector u,             // vector to reduce
    const GrB_Descriptor desc       // descriptor (currently unused)
) ;

GB_PUBLIC
GrB_Info GrB_Vector_reduce_UINT8    // c = accum (c, reduce_to_scalar (u))
(
    uint8_t *c,                     // result scalar
    const GrB_BinaryOp accum,       // optional accum for c=accum(c,t)
    const GrB_Monoid monoid,        // monoid to do the reduction
    const GrB_Vector u,             // vector to reduce
    const GrB_Descriptor desc       // descriptor (currently unused)
) ;

GB_PUBLIC
GrB_Info GrB_Vector_reduce_INT16    // c = accum (c, reduce_to_scalar (u))
(
    int16_t *c,                     // result scalar
    const GrB_BinaryOp accum,       // optional accum for c=accum(c,t)
    const GrB_Monoid monoid,        // monoid to do the reduction
    const GrB_Vector u,             // vector to reduce
    const GrB_Descriptor desc       // descriptor (currently unused)
) ;

GB_PUBLIC
GrB_Info GrB_Vector_reduce_UINT16   // c = accum (c, reduce_to_scalar (u))
(
    uint16_t *c,                    // result scalar
    const GrB_BinaryOp accum,       // optional accum for c=accum(c,t)
    const GrB_Monoid monoid,        // monoid to do the reduction
    const GrB_Vector u,             // vector to reduce
    const GrB_Descriptor desc       // descriptor (currently unused)
) ;

GB_PUBLIC
GrB_Info GrB_Vector_reduce_INT32    // c = accum (c, reduce_to_scalar (u))
(
    int32_t *c,                     // result scalar
    const GrB_BinaryOp accum,       // optional accum for c=accum(c,t)
    const GrB_Monoid monoid,        // monoid to do the reduction
    const GrB_Vector u,             // vector to reduce
    const GrB_Descriptor desc       // descriptor (currently unused)
) ;

GB_PUBLIC
GrB_Info GrB_Vector_reduce_UINT32   // c = accum (c, reduce_to_scalar (u))
(
    uint32_t *c,                    // result scalar
    const GrB_BinaryOp accum,       // optional accum for c=accum(c,t)
    const GrB_Monoid monoid,        // monoid to do the reduction
    const GrB_Vector u,             // vector to reduce
    const GrB_Descriptor desc       // descriptor (currently unused)
) ;

GB_PUBLIC
GrB_Info GrB_Vector_reduce_INT64    // c = accum (c, reduce_to_scalar (u))
(
    int64_t *c,                     // result scalar
    const GrB_BinaryOp accum,       // optional accum for c=accum(c,t)
    const GrB_Monoid monoid,        // monoid to do the reduction
    const GrB_Vector u,             // vector to reduce
    const GrB_Descriptor desc       // descriptor (currently unused)
) ;

GB_PUBLIC
GrB_Info GrB_Vector_reduce_UINT64   // c = accum (c, reduce_to_scalar (u))
(
    uint64_t *c,                    // result scalar
    const GrB_BinaryOp accum,       // optional accum for c=accum(c,t)
    const GrB_Monoid monoid,        // monoid to do the reduction
    const GrB_Vector u,             // vector to reduce
    const GrB_Descriptor desc       // descriptor (currently unused)
) ;

GB_PUBLIC
GrB_Info GrB_Vector_reduce_FP32     // c = accum (c, reduce_to_scalar (u))
(
    float *c,                       // result scalar
    const GrB_BinaryOp accum,       // optional accum for c=accum(c,t)
    const GrB_Monoid monoid,        // monoid to do the reduction
    const GrB_Vector u,             // vector to reduce
    const GrB_Descriptor desc       // descriptor (currently unused)
) ;

GB_PUBLIC
GrB_Info GrB_Vector_reduce_FP64     // c = accum (c, reduce_to_scalar (u))
(
    double *c,                      // result scalar
    const GrB_BinaryOp accum,       // optional accum for c=accum(c,t)
    const GrB_Monoid monoid,        // monoid to do the reduction
    const GrB_Vector u,             // vector to reduce
    const GrB_Descriptor desc       // descriptor (currently unused)
) ;

GB_PUBLIC
GrB_Info GxB_Vector_reduce_FC32     // c = accum (c, reduce_to_scalar (u))
(
    GxB_FC32_t *c,                  // result scalar
    const GrB_BinaryOp accum,       // optional accum for c=accum(c,t)
    const GrB_Monoid monoid,        // monoid to do the reduction
    const GrB_Vector u,             // vector to reduce
    const GrB_Descriptor desc       // descriptor (currently unused)
) ;

GB_PUBLIC
GrB_Info GxB_Vector_reduce_FC64     // c = accum (c, reduce_to_scalar (u))
(
    GxB_FC64_t *c,                  // result scalar
    const GrB_BinaryOp accum,       // optional accum for c=accum(c,t)
    const GrB_Monoid monoid,        // monoid to do the reduction
    const GrB_Vector u,             // vector to reduce
    const GrB_Descriptor desc       // descriptor (currently unused)
) ;

GB_PUBLIC
GrB_Info GrB_Vector_reduce_UDT      // c = accum (c, reduce_to_scalar (u))
(
    void *c,                        // result scalar
    const GrB_BinaryOp accum,       // optional accum for c=accum(c,t)
    const GrB_Monoid monoid,        // monoid to do the reduction
    const GrB_Vector u,             // vector to reduce
    const GrB_Descriptor desc       // descriptor (currently unused)
) ;

//------------------------------------------------------------------------------
// reduce a matrix to a scalar
//------------------------------------------------------------------------------

// Reduce entries in a matrix to a scalar, c = accum (c, reduce_to_scalar(A))

GB_PUBLIC
GrB_Info GrB_Matrix_reduce_BOOL     // c = accum (c, reduce_to_scalar (A))
(
    bool *c,                        // result scalar
    const GrB_BinaryOp accum,       // optional accum for c=accum(c,t)
    const GrB_Monoid monoid,        // monoid to do the reduction
    const GrB_Matrix A,             // matrix to reduce
    const GrB_Descriptor desc       // descriptor (currently unused)
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_reduce_INT8     // c = accum (c, reduce_to_scalar (A))
(
    int8_t *c,                      // result scalar
    const GrB_BinaryOp accum,       // optional accum for c=accum(c,t)
    const GrB_Monoid monoid,        // monoid to do the reduction
    const GrB_Matrix A,             // matrix to reduce
    const GrB_Descriptor desc       // descriptor (currently unused)
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_reduce_UINT8    // c = accum (c, reduce_to_scalar (A))
(
    uint8_t *c,                     // result scalar
    const GrB_BinaryOp accum,       // optional accum for c=accum(c,t)
    const GrB_Monoid monoid,        // monoid to do the reduction
    const GrB_Matrix A,             // matrix to reduce
    const GrB_Descriptor desc       // descriptor (currently unused)
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_reduce_INT16    // c = accum (c, reduce_to_scalar (A))
(
    int16_t *c,                     // result scalar
    const GrB_BinaryOp accum,       // optional accum for c=accum(c,t)
    const GrB_Monoid monoid,        // monoid to do the reduction
    const GrB_Matrix A,             // matrix to reduce
    const GrB_Descriptor desc       // descriptor (currently unused)
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_reduce_UINT16   // c = accum (c, reduce_to_scalar (A))
(
    uint16_t *c,                    // result scalar
    const GrB_BinaryOp accum,       // optional accum for c=accum(c,t)
    const GrB_Monoid monoid,        // monoid to do the reduction
    const GrB_Matrix A,             // matrix to reduce
    const GrB_Descriptor desc       // descriptor (currently unused)
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_reduce_INT32    // c = accum (c, reduce_to_scalar (A))
(
    int32_t *c,                     // result scalar
    const GrB_BinaryOp accum,       // optional accum for c=accum(c,t)
    const GrB_Monoid monoid,        // monoid to do the reduction
    const GrB_Matrix A,             // matrix to reduce
    const GrB_Descriptor desc       // descriptor (currently unused)
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_reduce_UINT32   // c = accum (c, reduce_to_scalar (A))
(
    uint32_t *c,                    // result scalar
    const GrB_BinaryOp accum,       // optional accum for c=accum(c,t)
    const GrB_Monoid monoid,        // monoid to do the reduction
    const GrB_Matrix A,             // matrix to reduce
    const GrB_Descriptor desc       // descriptor (currently unused)
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_reduce_INT64    // c = accum (c, reduce_to_scalar (A))
(
    int64_t *c,                     // result scalar
    const GrB_BinaryOp accum,       // optional accum for c=accum(c,t)
    const GrB_Monoid monoid,        // monoid to do the reduction
    const GrB_Matrix A,             // matrix to reduce
    const GrB_Descriptor desc       // descriptor (currently unused)
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_reduce_UINT64   // c = accum (c, reduce_to_scalar (A))
(
    uint64_t *c,                    // result scalar
    const GrB_BinaryOp accum,       // optional accum for c=accum(c,t)
    const GrB_Monoid monoid,        // monoid to do the reduction
    const GrB_Matrix A,             // matrix to reduce
    const GrB_Descriptor desc       // descriptor (currently unused)
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_reduce_FP32     // c = accum (c, reduce_to_scalar (A))
(
    float *c,                       // result scalar
    const GrB_BinaryOp accum,       // optional accum for c=accum(c,t)
    const GrB_Monoid monoid,        // monoid to do the reduction
    const GrB_Matrix A,             // matrix to reduce
    const GrB_Descriptor desc       // descriptor (currently unused)
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_reduce_FP64     // c = accum (c, reduce_to_scalar (A))
(
    double *c,                      // result scalar
    const GrB_BinaryOp accum,       // optional accum for c=accum(c,t)
    const GrB_Monoid monoid,        // monoid to do the reduction
    const GrB_Matrix A,             // matrix to reduce
    const GrB_Descriptor desc       // descriptor (currently unused)
) ;

GB_PUBLIC
GrB_Info GxB_Matrix_reduce_FC32     // c = accum (c, reduce_to_scalar (A))
(
    GxB_FC32_t *c,                  // result scalar
    const GrB_BinaryOp accum,       // optional accum for c=accum(c,t)
    const GrB_Monoid monoid,        // monoid to do the reduction
    const GrB_Matrix A,             // matrix to reduce
    const GrB_Descriptor desc       // descriptor (currently unused)
) ;

GB_PUBLIC
GrB_Info GxB_Matrix_reduce_FC64     // c = accum (c, reduce_to_scalar (A))
(
    GxB_FC64_t *c,                  // result scalar
    const GrB_BinaryOp accum,       // optional accum for c=accum(c,t)
    const GrB_Monoid monoid,        // monoid to do the reduction
    const GrB_Matrix A,             // matrix to reduce
    const GrB_Descriptor desc       // descriptor (currently unused)
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_reduce_UDT      // c = accum (c, reduce_to_scalar (A))
(
    void *c,                        // result scalar
    const GrB_BinaryOp accum,       // optional accum for c=accum(c,t)
    const GrB_Monoid monoid,        // monoid to do the reduction
    const GrB_Matrix A,             // matrix to reduce
    const GrB_Descriptor desc       // descriptor (currently unused)
) ;

//------------------------------------------------------------------------------
// GrB_reduce: generic matrix/vector reduction to a vector or scalar
//------------------------------------------------------------------------------

// GrB_reduce is a generic function that provides access to all GrB_*reduce*
// functions:

// reduce matrix to vector:
// GrB_Matrix_reduce_Monoid   (w,mask,acc,mo,A,d) // w<mask> = acc (w,reduce(A))
// GrB_Matrix_reduce_BinaryOp (w,mask,acc,op,A,d) // w<mask> = acc (w,reduce(A))
// reduce matrix to scalar:
// GrB_Vector_reduce_[SCALAR] (c,acc,monoid,u,d)  // c = acc (c,reduce(u))
// GrB_Matrix_reduce_[SCALAR] (c,acc,monoid,A,d)  // c = acc (c,reduce(A))

#if GxB_STDC_VERSION >= 201112L
#define GB_REDUCE_TO_SCALAR(kind,c)                                 \
    _Generic                                                        \
    (                                                               \
        (c),                                                        \
          GB_(*, GrB,    kind ## _reduce),                          \
        default: GrB_ ## kind ## _reduce_UDT                        \
    )

#define GrB_reduce(arg1,arg2,arg3,arg4,...)                         \
    _Generic                                                        \
    (                                                               \
        (arg4),                                                     \
        const GrB_Vector   : GB_REDUCE_TO_SCALAR (Vector, arg1),    \
              GrB_Vector   : GB_REDUCE_TO_SCALAR (Vector, arg1),    \
        const GrB_Matrix   : GB_REDUCE_TO_SCALAR (Matrix, arg1),    \
              GrB_Matrix   : GB_REDUCE_TO_SCALAR (Matrix, arg1),    \
        const GrB_Monoid   : GrB_Matrix_reduce_Monoid   ,           \
              GrB_Monoid   : GrB_Matrix_reduce_Monoid   ,           \
        const GrB_BinaryOp : GrB_Matrix_reduce_BinaryOp ,           \
              GrB_BinaryOp : GrB_Matrix_reduce_BinaryOp             \
    )                                                               \
    (arg1, arg2, arg3, arg4, __VA_ARGS__)
#endif

//==============================================================================
// GrB_transpose: matrix transpose
//==============================================================================

GB_PUBLIC
GrB_Info GrB_transpose              // C<Mask> = accum (C, A')
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Matrix Mask,          // optional mask for C, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C,T)
    const GrB_Matrix A,             // first input:  matrix A
    const GrB_Descriptor desc       // descriptor for C, Mask, and A
) ;

//==============================================================================
// GrB_kronecker:  Kronecker product
//==============================================================================

// GxB_kron is deprecated; use GrB_kronecker instead
GB_PUBLIC
GrB_Info GxB_kron                   // C<Mask> = accum(C,kron(A,B)) (deprecated)
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Matrix Mask,          // optional mask for C, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C,T)
    const GrB_BinaryOp op,          // defines '*' for T=kron(A,B)
    const GrB_Matrix A,             // first input:  matrix A
    const GrB_Matrix B,             // second input: matrix B
    const GrB_Descriptor desc       // descriptor for C, Mask, A, and B
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_kronecker_BinaryOp  // C<M> = accum (C, kron(A,B))
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Matrix M,             // optional mask for C, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C,T)
    const GrB_BinaryOp op,          // defines '*' for T=kron(A,B)
    const GrB_Matrix A,             // first input:  matrix A
    const GrB_Matrix B,             // second input: matrix B
    const GrB_Descriptor desc       // descriptor for C, M, A, and B
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_kronecker_Monoid  // C<M> = accum (C, kron(A,B))
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Matrix M,             // optional mask for C, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C,T)
    const GrB_Monoid monoid,        // defines '*' for T=kron(A,B)
    const GrB_Matrix A,             // first input:  matrix A
    const GrB_Matrix B,             // second input: matrix B
    const GrB_Descriptor desc       // descriptor for C, M, A, and B
) ;

GB_PUBLIC
GrB_Info GrB_Matrix_kronecker_Semiring  // C<M> = accum (C, kron(A,B))
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Matrix M,             // optional mask for C, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C,T)
    const GrB_Semiring semiring,    // defines '*' for T=kron(A,B)
    const GrB_Matrix A,             // first input:  matrix A
    const GrB_Matrix B,             // second input: matrix B
    const GrB_Descriptor desc       // descriptor for C, M, A, and B
) ;

#if GxB_STDC_VERSION >= 201112L
#define GrB_kronecker(C,Mask,accum,op,A,B,desc)                 \
    _Generic                                                    \
    (                                                           \
        (op),                                                   \
        const GrB_Semiring : GrB_Matrix_kronecker_Semiring ,    \
              GrB_Semiring : GrB_Matrix_kronecker_Semiring ,    \
        const GrB_Monoid   : GrB_Matrix_kronecker_Monoid   ,    \
              GrB_Monoid   : GrB_Matrix_kronecker_Monoid   ,    \
        const GrB_BinaryOp : GrB_Matrix_kronecker_BinaryOp ,    \
              GrB_BinaryOp : GrB_Matrix_kronecker_BinaryOp      \
    )                                                           \
    (C, Mask, accum, op, A, B, desc)
#endif


//==============================================================================
// GrB_Monoid: built-in monoids
//==============================================================================

GB_PUBLIC GrB_Monoid

    //--------------------------------------------------------------------------
    // 10 MIN monoids: (not for complex types)
    //--------------------------------------------------------------------------

    // GxB_MIN monoids, deprecated, use GrB_MIN_MONOID_* instead:
    GxB_MIN_INT8_MONOID,        // identity: INT8_MAX     terminal: INT8_MIN
    GxB_MIN_INT16_MONOID,       // identity: INT16_MAX    terminal: INT16_MIN
    GxB_MIN_INT32_MONOID,       // identity: INT32_MAX    terminal: INT32_MIN
    GxB_MIN_INT64_MONOID,       // identity: INT64_MAX    terminal: INT32_MIN
    GxB_MIN_UINT8_MONOID,       // identity: UINT8_MAX    terminal: 0
    GxB_MIN_UINT16_MONOID,      // identity: UINT16_MAX   terminal: 0
    GxB_MIN_UINT32_MONOID,      // identity: UINT32_MAX   terminal: 0
    GxB_MIN_UINT64_MONOID,      // identity: UINT64_MAX   terminal: 0
    GxB_MIN_FP32_MONOID,        // identity: INFINITY     terminal: -INFINITY
    GxB_MIN_FP64_MONOID,        // identity: INFINITY     terminal: -INFINITY

    // preferred names from the v1.3 spec:
    GrB_MIN_MONOID_INT8,        // identity: INT8_MAX     terminal: INT8_MIN
    GrB_MIN_MONOID_INT16,       // identity: INT16_MAX    terminal: INT16_MIN
    GrB_MIN_MONOID_INT32,       // identity: INT32_MAX    terminal: INT32_MIN
    GrB_MIN_MONOID_INT64,       // identity: INT64_MAX    terminal: INT32_MIN
    GrB_MIN_MONOID_UINT8,       // identity: UINT8_MAX    terminal: 0
    GrB_MIN_MONOID_UINT16,      // identity: UINT16_MAX   terminal: 0
    GrB_MIN_MONOID_UINT32,      // identity: UINT32_MAX   terminal: 0
    GrB_MIN_MONOID_UINT64,      // identity: UINT64_MAX   terminal: 0
    GrB_MIN_MONOID_FP32,        // identity: INFINITY     terminal: -INFINITY
    GrB_MIN_MONOID_FP64,        // identity: INFINITY     terminal: -INFINITY

    //--------------------------------------------------------------------------
    // 10 MAX monoids:
    //--------------------------------------------------------------------------

    // GxB_MAX monoids, deprecated, use GrB_MAX_MONOID_* instead:
    GxB_MAX_INT8_MONOID,        // identity: INT8_MIN     terminal: INT8_MAX
    GxB_MAX_INT16_MONOID,       // identity: INT16_MIN    terminal: INT16_MAX
    GxB_MAX_INT32_MONOID,       // identity: INT32_MIN    terminal: INT32_MAX
    GxB_MAX_INT64_MONOID,       // identity: INT64_MIN    terminal: INT64_MAX
    GxB_MAX_UINT8_MONOID,       // identity: 0            terminal: UINT8_MAX
    GxB_MAX_UINT16_MONOID,      // identity: 0            terminal: UINT16_MAX
    GxB_MAX_UINT32_MONOID,      // identity: 0            terminal: UINT32_MAX
    GxB_MAX_UINT64_MONOID,      // identity: 0            terminal: UINT64_MAX
    GxB_MAX_FP32_MONOID,        // identity: -INFINITY    terminal: INFINITY
    GxB_MAX_FP64_MONOID,        // identity: -INFINITY    terminal: INFINITY

    // preferred names from the v1.3 spec:
    GrB_MAX_MONOID_INT8,        // identity: INT8_MIN     terminal: INT8_MAX
    GrB_MAX_MONOID_INT16,       // identity: INT16_MIN    terminal: INT16_MAX
    GrB_MAX_MONOID_INT32,       // identity: INT32_MIN    terminal: INT32_MAX
    GrB_MAX_MONOID_INT64,       // identity: INT64_MIN    terminal: INT64_MAX
    GrB_MAX_MONOID_UINT8,       // identity: 0            terminal: UINT8_MAX
    GrB_MAX_MONOID_UINT16,      // identity: 0            terminal: UINT16_MAX
    GrB_MAX_MONOID_UINT32,      // identity: 0            terminal: UINT32_MAX
    GrB_MAX_MONOID_UINT64,      // identity: 0            terminal: UINT64_MAX
    GrB_MAX_MONOID_FP32,        // identity: -INFINITY    terminal: INFINITY
    GrB_MAX_MONOID_FP64,        // identity: -INFINITY    terminal: INFINITY

    //--------------------------------------------------------------------------
    // 12 PLUS monoids:
    //--------------------------------------------------------------------------

    // GxB_PLUS monoids, deprecated, use GrB_PLUS_MONOID_* instead:
    GxB_PLUS_INT8_MONOID,       // identity: 0
    GxB_PLUS_INT16_MONOID,      // identity: 0
    GxB_PLUS_INT32_MONOID,      // identity: 0
    GxB_PLUS_INT64_MONOID,      // identity: 0
    GxB_PLUS_UINT8_MONOID,      // identity: 0
    GxB_PLUS_UINT16_MONOID,     // identity: 0
    GxB_PLUS_UINT32_MONOID,     // identity: 0
    GxB_PLUS_UINT64_MONOID,     // identity: 0
    GxB_PLUS_FP32_MONOID,       // identity: 0
    GxB_PLUS_FP64_MONOID,       // identity: 0

    // preferred names from the v1.3 spec:
    GrB_PLUS_MONOID_INT8,       // identity: 0
    GrB_PLUS_MONOID_INT16,      // identity: 0
    GrB_PLUS_MONOID_INT32,      // identity: 0
    GrB_PLUS_MONOID_INT64,      // identity: 0
    GrB_PLUS_MONOID_UINT8,      // identity: 0
    GrB_PLUS_MONOID_UINT16,     // identity: 0
    GrB_PLUS_MONOID_UINT32,     // identity: 0
    GrB_PLUS_MONOID_UINT64,     // identity: 0
    GrB_PLUS_MONOID_FP32,       // identity: 0
    GrB_PLUS_MONOID_FP64,       // identity: 0

    // complex monoids:
    GxB_PLUS_FC32_MONOID,       // identity: 0
    GxB_PLUS_FC64_MONOID,       // identity: 0

    //--------------------------------------------------------------------------
    // 12 TIMES monoids: identity value is 1, int* and uint* are terminal
    //--------------------------------------------------------------------------

    // GxB_TIMES monoids, deprecated, use GrB_TIMES_MONOID_* instead:
    GxB_TIMES_INT8_MONOID,      // identity: 1            terminal: 0
    GxB_TIMES_INT16_MONOID,     // identity: 1            terminal: 0
    GxB_TIMES_INT32_MONOID,     // identity: 1            terminal: 0
    GxB_TIMES_INT64_MONOID,     // identity: 1            terminal: 0
    GxB_TIMES_UINT8_MONOID,     // identity: 1            terminal: 0
    GxB_TIMES_UINT16_MONOID,    // identity: 1            terminal: 0
    GxB_TIMES_UINT32_MONOID,    // identity: 1            terminal: 0
    GxB_TIMES_UINT64_MONOID,    // identity: 1            terminal: 0
    GxB_TIMES_FP32_MONOID,      // identity: 1
    GxB_TIMES_FP64_MONOID,      // identity: 1

    // preferred names from the v1.3 spec:
    GrB_TIMES_MONOID_INT8,      // identity: 1            terminal: 0
    GrB_TIMES_MONOID_INT16,     // identity: 1            terminal: 0
    GrB_TIMES_MONOID_INT32,     // identity: 1            terminal: 0
    GrB_TIMES_MONOID_INT64,     // identity: 1            terminal: 0
    GrB_TIMES_MONOID_UINT8,     // identity: 1            terminal: 0
    GrB_TIMES_MONOID_UINT16,    // identity: 1            terminal: 0
    GrB_TIMES_MONOID_UINT32,    // identity: 1            terminal: 0
    GrB_TIMES_MONOID_UINT64,    // identity: 1            terminal: 0
    GrB_TIMES_MONOID_FP32,      // identity: 1
    GrB_TIMES_MONOID_FP64,      // identity: 1

    // complex monoids:
    GxB_TIMES_FC32_MONOID,      // identity: 1
    GxB_TIMES_FC64_MONOID,      // identity: 1

    //--------------------------------------------------------------------------
    // 13 ANY monoids:
    //--------------------------------------------------------------------------

    GxB_ANY_BOOL_MONOID,        // identity: any value    terminal: any value
    GxB_ANY_INT8_MONOID,        // identity: any value    terminal: any value
    GxB_ANY_INT16_MONOID,       // identity: any value    terminal: any value
    GxB_ANY_INT32_MONOID,       // identity: any value    terminal: any value
    GxB_ANY_INT64_MONOID,       // identity: any value    terminal: any value
    GxB_ANY_UINT8_MONOID,       // identity: any value    terminal: any value
    GxB_ANY_UINT16_MONOID,      // identity: any value    terminal: any value
    GxB_ANY_UINT32_MONOID,      // identity: any value    terminal: any value
    GxB_ANY_UINT64_MONOID,      // identity: any value    terminal: any value
    GxB_ANY_FP32_MONOID,        // identity: any value    terminal: any value
    GxB_ANY_FP64_MONOID,        // identity: any value    terminal: any value
    GxB_ANY_FC32_MONOID,        // identity: any value    terminal: any value
    GxB_ANY_FC64_MONOID,        // identity: any value    terminal: any value

    //--------------------------------------------------------------------------
    // 4 Boolean monoids: (see also the GxB_ANY_BOOL_MONOID above)
    //--------------------------------------------------------------------------

    // GxB boolean monoids, deprecated, use GrB instead:
    GxB_LOR_BOOL_MONOID,        // identity: false        terminal: true
    GxB_LAND_BOOL_MONOID,       // identity: true         terminal: false
    GxB_LXOR_BOOL_MONOID,       // identity: false
    GxB_LXNOR_BOOL_MONOID,      // identity: true
    GxB_EQ_BOOL_MONOID,         // (alternative name for GrB_LXNOR_MONOID_BOOL)

    // preferred names from the v1.3 spec:
    GrB_LOR_MONOID_BOOL,        // identity: false        terminal: true
    GrB_LAND_MONOID_BOOL,       // identity: true         terminal: false
    GrB_LXOR_MONOID_BOOL,       // identity: false
    GrB_LXNOR_MONOID_BOOL,      // identity: true

    //--------------------------------------------------------------------------
    // 16 Bitwise-or monoids:
    //--------------------------------------------------------------------------

    // BOR monoids (bitwise or):
    GxB_BOR_UINT8_MONOID,       // identity: 0   terminal: 0xFF
    GxB_BOR_UINT16_MONOID,      // identity: 0   terminal: 0xFFFF
    GxB_BOR_UINT32_MONOID,      // identity: 0   terminal: 0xFFFFFFFF
    GxB_BOR_UINT64_MONOID,      // identity: 0   terminal: 0xFFFFFFFFFFFFFFFF

    // BAND monoids (bitwise and):
    GxB_BAND_UINT8_MONOID,      // identity: 0xFF               terminal: 0
    GxB_BAND_UINT16_MONOID,     // identity: 0xFFFF             terminal: 0
    GxB_BAND_UINT32_MONOID,     // identity: 0xFFFFFFFF         terminal: 0
    GxB_BAND_UINT64_MONOID,     // identity: 0xFFFFFFFFFFFFFFFF terminal: 0

    // BXOR monoids (bitwise xor):
    GxB_BXOR_UINT8_MONOID,      // identity: 0
    GxB_BXOR_UINT16_MONOID,     // identity: 0
    GxB_BXOR_UINT32_MONOID,     // identity: 0
    GxB_BXOR_UINT64_MONOID,     // identity: 0

    // BXNOR monoids (bitwise xnor):
    GxB_BXNOR_UINT8_MONOID,     // identity: 0xFF
    GxB_BXNOR_UINT16_MONOID,    // identity: 0xFFFF
    GxB_BXNOR_UINT32_MONOID,    // identity: 0xFFFFFFFF
    GxB_BXNOR_UINT64_MONOID ;   // identity: 0xFFFFFFFFFFFFFFFF

//==============================================================================
// GrB_Semiring: built-in semirings
//==============================================================================

// Using built-in types and operators, SuiteSparse:GraphBLAS provides
// 1553 pre-defined, built-in semirings:

// 1000 semirings with a multiply operator TxT -> T where T is non-Boolean,
// from the complete cross product of:

//      5 monoids: MIN, MAX, PLUS, TIMES, ANY
//      20 multiply operators:
//          FIRST, SECOND, PAIR, MIN, MAX, PLUS, MINUS, TIMES, DIV, RDIV, RMINUS
//          ISEQ, ISNE, ISGT, ISLT, ISGE, ISLE,
//          LOR, LAND, LXOR
//      10 non-Boolean real types, T
//
//      Note that min_pair, max_pair, times_pair are all identical to any_pair.
//      These 30 semirings are named below, but are internally remapped to
//      their corresponding any_pair semiring.

// 300 semirings with a comparison operator TxT -> bool, where T is
// non-Boolean, from the complete cross product of:

//      5 Boolean monoids: LAND, LOR, LXOR, EQ (=LXNOR), ANY
//      6 multiply operators: EQ, NE, GT, LT, GE, LE
//      10 non-Boolean real types, T

// 55 semirings with purely Boolean types, bool x bool -> bool, from the
// complete cross product of:

//      5 Boolean monoids LAND, LOR, LXOR, EQ (=LXNOR), ANY
//      11 multiply operators:
//          FIRST, SECOND, LOR, LAND, LXOR, EQ (=LXNOR), GT, LT, GE, LE, PAIR
//
//      Note that lor_pair, land_pair, and eq_pair are all identical to
//      any_pair.  These 3 semirings are named below, but are internally
//      remapped to any_pair_bool semiring.

// 54 complex semirings: TxT -> T where T is float complex or double complex:

//      3 complex monoids: PLUS, TIMES, ANY
//      9 complex multiply operators:
//          FIRST, SECOND, PAIR, PLUS, MINUS, TIMES, DIV, RDIV, RMINUS
//      2 complex types
//
//      Note that times_pair is identical to any_pair.
//      These 2 semirings are named below, but are internally remapped to
//      their corresponding any_pair semiring.

// 64 bitwise semirings: TxT -> T where T is an unsigned integer:

//      4 bitwise monoids: BOR, BAND, BXOR, BXNOR
//      4 bitwise multiply operators: BOR, BAND, BXOR, BXNOR
//      4 unsigned integer types: UINT8, UINT16, UINT32, UINT64

// 80 positional semirings: XxX -> T where T is int64 or int32, and the type of
// X is ignored:

//      5 monoids: MIN, MAX, PLUS, TIMES, ANY
//      8 multiply operators:
//          FIRSTI, FIRSTI1, FIRSTJ, FIRSTJ1,
//          SECONDI, SECONDI1, SECONDJ, SECONDJ1
//      2 types: int32, int64

// The ANY operator is also valid to use as a multiplicative operator in a
// semiring, but serves no purpose in that case.  The ANY operator is meant as
// a fast additive operator for a monoid, that terminates, or short-circuits,
// as soon as any value is found.  A valid user semiring can be constructed
// with ANY as the multiply operator, but they are not predefined below.

// Likewise, additional built-in operators can be used as multiplicative
// operators for floating-point semirings (POW, ATAN2, HYPOT, ...) and many
// more semirings can be constructed from bitwise monoids and many integer
// binary (non-bitwise) multiplicative operators, but these are not
// pre-defined.

// In the names below, each semiring has a name of the form GxB_add_mult_T
// where add is the additive monoid, mult is the multiply operator, and T is
// the type.  The type T is always the type of x and y for the z=mult(x,y)
// operator.  The monoid's three types and the ztype of the mult operator are
// always the same.  This is the type T for the first set, and Boolean for
// the second and third sets of semirngs.

// 1553 = 1000 + 300 + 55 + 54 + 64 + 80 semirings are named below, but 35 = 30
// + 3 + 2 are identical to the corresponding any_pair semirings of the same
// type.  For positional semirings, the mulitiply ops FIRSTJ and SECONDI are
// identical, as are FIRSTJ1 and SECONDI1.  These semirings still appear as
// predefined, for convenience.

GB_PUBLIC GrB_Semiring

//------------------------------------------------------------------------------
// 1000 non-Boolean semirings where all types are the same, given by suffix _T
//------------------------------------------------------------------------------

    // semirings with multiply op: z = FIRST (x,y), all types x,y,z the same:
    GxB_MIN_FIRST_INT8     , GxB_MAX_FIRST_INT8     , GxB_PLUS_FIRST_INT8    , GxB_TIMES_FIRST_INT8   , GxB_ANY_FIRST_INT8     ,
    GxB_MIN_FIRST_INT16    , GxB_MAX_FIRST_INT16    , GxB_PLUS_FIRST_INT16   , GxB_TIMES_FIRST_INT16  , GxB_ANY_FIRST_INT16    ,
    GxB_MIN_FIRST_INT32    , GxB_MAX_FIRST_INT32    , GxB_PLUS_FIRST_INT32   , GxB_TIMES_FIRST_INT32  , GxB_ANY_FIRST_INT32    ,
    GxB_MIN_FIRST_INT64    , GxB_MAX_FIRST_INT64    , GxB_PLUS_FIRST_INT64   , GxB_TIMES_FIRST_INT64  , GxB_ANY_FIRST_INT64    ,
    GxB_MIN_FIRST_UINT8    , GxB_MAX_FIRST_UINT8    , GxB_PLUS_FIRST_UINT8   , GxB_TIMES_FIRST_UINT8  , GxB_ANY_FIRST_UINT8    ,
    GxB_MIN_FIRST_UINT16   , GxB_MAX_FIRST_UINT16   , GxB_PLUS_FIRST_UINT16  , GxB_TIMES_FIRST_UINT16 , GxB_ANY_FIRST_UINT16   ,
    GxB_MIN_FIRST_UINT32   , GxB_MAX_FIRST_UINT32   , GxB_PLUS_FIRST_UINT32  , GxB_TIMES_FIRST_UINT32 , GxB_ANY_FIRST_UINT32   ,
    GxB_MIN_FIRST_UINT64   , GxB_MAX_FIRST_UINT64   , GxB_PLUS_FIRST_UINT64  , GxB_TIMES_FIRST_UINT64 , GxB_ANY_FIRST_UINT64   ,
    GxB_MIN_FIRST_FP32     , GxB_MAX_FIRST_FP32     , GxB_PLUS_FIRST_FP32    , GxB_TIMES_FIRST_FP32   , GxB_ANY_FIRST_FP32     ,
    GxB_MIN_FIRST_FP64     , GxB_MAX_FIRST_FP64     , GxB_PLUS_FIRST_FP64    , GxB_TIMES_FIRST_FP64   , GxB_ANY_FIRST_FP64     ,

    // semirings with multiply op: z = SECOND (x,y), all types x,y,z the same:
    GxB_MIN_SECOND_INT8    , GxB_MAX_SECOND_INT8    , GxB_PLUS_SECOND_INT8   , GxB_TIMES_SECOND_INT8  , GxB_ANY_SECOND_INT8    ,
    GxB_MIN_SECOND_INT16   , GxB_MAX_SECOND_INT16   , GxB_PLUS_SECOND_INT16  , GxB_TIMES_SECOND_INT16 , GxB_ANY_SECOND_INT16   ,
    GxB_MIN_SECOND_INT32   , GxB_MAX_SECOND_INT32   , GxB_PLUS_SECOND_INT32  , GxB_TIMES_SECOND_INT32 , GxB_ANY_SECOND_INT32   ,
    GxB_MIN_SECOND_INT64   , GxB_MAX_SECOND_INT64   , GxB_PLUS_SECOND_INT64  , GxB_TIMES_SECOND_INT64 , GxB_ANY_SECOND_INT64   ,
    GxB_MIN_SECOND_UINT8   , GxB_MAX_SECOND_UINT8   , GxB_PLUS_SECOND_UINT8  , GxB_TIMES_SECOND_UINT8 , GxB_ANY_SECOND_UINT8   ,
    GxB_MIN_SECOND_UINT16  , GxB_MAX_SECOND_UINT16  , GxB_PLUS_SECOND_UINT16 , GxB_TIMES_SECOND_UINT16, GxB_ANY_SECOND_UINT16  ,
    GxB_MIN_SECOND_UINT32  , GxB_MAX_SECOND_UINT32  , GxB_PLUS_SECOND_UINT32 , GxB_TIMES_SECOND_UINT32, GxB_ANY_SECOND_UINT32  ,
    GxB_MIN_SECOND_UINT64  , GxB_MAX_SECOND_UINT64  , GxB_PLUS_SECOND_UINT64 , GxB_TIMES_SECOND_UINT64, GxB_ANY_SECOND_UINT64  ,
    GxB_MIN_SECOND_FP32    , GxB_MAX_SECOND_FP32    , GxB_PLUS_SECOND_FP32   , GxB_TIMES_SECOND_FP32  , GxB_ANY_SECOND_FP32    ,
    GxB_MIN_SECOND_FP64    , GxB_MAX_SECOND_FP64    , GxB_PLUS_SECOND_FP64   , GxB_TIMES_SECOND_FP64  , GxB_ANY_SECOND_FP64    ,

    // semirings with multiply op: z = PAIR (x,y), all types x,y,z the same:
    // (note that min_pair, max_pair, times_pair are all identical to any_pair, and are marked below)
    GxB_MIN_PAIR_INT8  /**/, GxB_MAX_PAIR_INT8  /**/, GxB_PLUS_PAIR_INT8     , GxB_TIMES_PAIR_INT8  /**/, GxB_ANY_PAIR_INT8    ,
    GxB_MIN_PAIR_INT16 /**/, GxB_MAX_PAIR_INT16 /**/, GxB_PLUS_PAIR_INT16    , GxB_TIMES_PAIR_INT16 /**/, GxB_ANY_PAIR_INT16   ,
    GxB_MIN_PAIR_INT32 /**/, GxB_MAX_PAIR_INT32 /**/, GxB_PLUS_PAIR_INT32    , GxB_TIMES_PAIR_INT32 /**/, GxB_ANY_PAIR_INT32   ,
    GxB_MIN_PAIR_INT64 /**/, GxB_MAX_PAIR_INT64 /**/, GxB_PLUS_PAIR_INT64    , GxB_TIMES_PAIR_INT64 /**/, GxB_ANY_PAIR_INT64   ,
    GxB_MIN_PAIR_UINT8 /**/, GxB_MAX_PAIR_UINT8 /**/, GxB_PLUS_PAIR_UINT8    , GxB_TIMES_PAIR_UINT8 /**/, GxB_ANY_PAIR_UINT8   ,
    GxB_MIN_PAIR_UINT16/**/, GxB_MAX_PAIR_UINT16/**/, GxB_PLUS_PAIR_UINT16   , GxB_TIMES_PAIR_UINT16/**/, GxB_ANY_PAIR_UINT16  ,
    GxB_MIN_PAIR_UINT32/**/, GxB_MAX_PAIR_UINT32/**/, GxB_PLUS_PAIR_UINT32   , GxB_TIMES_PAIR_UINT32/**/, GxB_ANY_PAIR_UINT32  ,
    GxB_MIN_PAIR_UINT64/**/, GxB_MAX_PAIR_UINT64/**/, GxB_PLUS_PAIR_UINT64   , GxB_TIMES_PAIR_UINT64/**/, GxB_ANY_PAIR_UINT64  ,
    GxB_MIN_PAIR_FP32  /**/, GxB_MAX_PAIR_FP32  /**/, GxB_PLUS_PAIR_FP32     , GxB_TIMES_PAIR_FP32  /**/, GxB_ANY_PAIR_FP32    ,
    GxB_MIN_PAIR_FP64  /**/, GxB_MAX_PAIR_FP64  /**/, GxB_PLUS_PAIR_FP64     , GxB_TIMES_PAIR_FP64  /**/, GxB_ANY_PAIR_FP64    ,

    // semirings with multiply op: z = MIN (x,y), all types x,y,z the same:
    GxB_MIN_MIN_INT8       , GxB_MAX_MIN_INT8       , GxB_PLUS_MIN_INT8      , GxB_TIMES_MIN_INT8     , GxB_ANY_MIN_INT8       ,
    GxB_MIN_MIN_INT16      , GxB_MAX_MIN_INT16      , GxB_PLUS_MIN_INT16     , GxB_TIMES_MIN_INT16    , GxB_ANY_MIN_INT16      ,
    GxB_MIN_MIN_INT32      , GxB_MAX_MIN_INT32      , GxB_PLUS_MIN_INT32     , GxB_TIMES_MIN_INT32    , GxB_ANY_MIN_INT32      ,
    GxB_MIN_MIN_INT64      , GxB_MAX_MIN_INT64      , GxB_PLUS_MIN_INT64     , GxB_TIMES_MIN_INT64    , GxB_ANY_MIN_INT64      ,
    GxB_MIN_MIN_UINT8      , GxB_MAX_MIN_UINT8      , GxB_PLUS_MIN_UINT8     , GxB_TIMES_MIN_UINT8    , GxB_ANY_MIN_UINT8      ,
    GxB_MIN_MIN_UINT16     , GxB_MAX_MIN_UINT16     , GxB_PLUS_MIN_UINT16    , GxB_TIMES_MIN_UINT16   , GxB_ANY_MIN_UINT16     ,
    GxB_MIN_MIN_UINT32     , GxB_MAX_MIN_UINT32     , GxB_PLUS_MIN_UINT32    , GxB_TIMES_MIN_UINT32   , GxB_ANY_MIN_UINT32     ,
    GxB_MIN_MIN_UINT64     , GxB_MAX_MIN_UINT64     , GxB_PLUS_MIN_UINT64    , GxB_TIMES_MIN_UINT64   , GxB_ANY_MIN_UINT64     ,
    GxB_MIN_MIN_FP32       , GxB_MAX_MIN_FP32       , GxB_PLUS_MIN_FP32      , GxB_TIMES_MIN_FP32     , GxB_ANY_MIN_FP32       ,
    GxB_MIN_MIN_FP64       , GxB_MAX_MIN_FP64       , GxB_PLUS_MIN_FP64      , GxB_TIMES_MIN_FP64     , GxB_ANY_MIN_FP64       ,

    // semirings with multiply op: z = MAX (x,y), all types x,y,z the same:
    GxB_MIN_MAX_INT8       , GxB_MAX_MAX_INT8       , GxB_PLUS_MAX_INT8      , GxB_TIMES_MAX_INT8     , GxB_ANY_MAX_INT8       ,
    GxB_MIN_MAX_INT16      , GxB_MAX_MAX_INT16      , GxB_PLUS_MAX_INT16     , GxB_TIMES_MAX_INT16    , GxB_ANY_MAX_INT16      ,
    GxB_MIN_MAX_INT32      , GxB_MAX_MAX_INT32      , GxB_PLUS_MAX_INT32     , GxB_TIMES_MAX_INT32    , GxB_ANY_MAX_INT32      ,
    GxB_MIN_MAX_INT64      , GxB_MAX_MAX_INT64      , GxB_PLUS_MAX_INT64     , GxB_TIMES_MAX_INT64    , GxB_ANY_MAX_INT64      ,
    GxB_MIN_MAX_UINT8      , GxB_MAX_MAX_UINT8      , GxB_PLUS_MAX_UINT8     , GxB_TIMES_MAX_UINT8    , GxB_ANY_MAX_UINT8      ,
    GxB_MIN_MAX_UINT16     , GxB_MAX_MAX_UINT16     , GxB_PLUS_MAX_UINT16    , GxB_TIMES_MAX_UINT16   , GxB_ANY_MAX_UINT16     ,
    GxB_MIN_MAX_UINT32     , GxB_MAX_MAX_UINT32     , GxB_PLUS_MAX_UINT32    , GxB_TIMES_MAX_UINT32   , GxB_ANY_MAX_UINT32     ,
    GxB_MIN_MAX_UINT64     , GxB_MAX_MAX_UINT64     , GxB_PLUS_MAX_UINT64    , GxB_TIMES_MAX_UINT64   , GxB_ANY_MAX_UINT64     ,
    GxB_MIN_MAX_FP32       , GxB_MAX_MAX_FP32       , GxB_PLUS_MAX_FP32      , GxB_TIMES_MAX_FP32     , GxB_ANY_MAX_FP32       ,
    GxB_MIN_MAX_FP64       , GxB_MAX_MAX_FP64       , GxB_PLUS_MAX_FP64      , GxB_TIMES_MAX_FP64     , GxB_ANY_MAX_FP64       ,

    // semirings with multiply op: z = PLUS (x,y), all types x,y,z the same:
    GxB_MIN_PLUS_INT8      , GxB_MAX_PLUS_INT8      , GxB_PLUS_PLUS_INT8     , GxB_TIMES_PLUS_INT8    , GxB_ANY_PLUS_INT8      ,
    GxB_MIN_PLUS_INT16     , GxB_MAX_PLUS_INT16     , GxB_PLUS_PLUS_INT16    , GxB_TIMES_PLUS_INT16   , GxB_ANY_PLUS_INT16     ,
    GxB_MIN_PLUS_INT32     , GxB_MAX_PLUS_INT32     , GxB_PLUS_PLUS_INT32    , GxB_TIMES_PLUS_INT32   , GxB_ANY_PLUS_INT32     ,
    GxB_MIN_PLUS_INT64     , GxB_MAX_PLUS_INT64     , GxB_PLUS_PLUS_INT64    , GxB_TIMES_PLUS_INT64   , GxB_ANY_PLUS_INT64     ,
    GxB_MIN_PLUS_UINT8     , GxB_MAX_PLUS_UINT8     , GxB_PLUS_PLUS_UINT8    , GxB_TIMES_PLUS_UINT8   , GxB_ANY_PLUS_UINT8     ,
    GxB_MIN_PLUS_UINT16    , GxB_MAX_PLUS_UINT16    , GxB_PLUS_PLUS_UINT16   , GxB_TIMES_PLUS_UINT16  , GxB_ANY_PLUS_UINT16    ,
    GxB_MIN_PLUS_UINT32    , GxB_MAX_PLUS_UINT32    , GxB_PLUS_PLUS_UINT32   , GxB_TIMES_PLUS_UINT32  , GxB_ANY_PLUS_UINT32    ,
    GxB_MIN_PLUS_UINT64    , GxB_MAX_PLUS_UINT64    , GxB_PLUS_PLUS_UINT64   , GxB_TIMES_PLUS_UINT64  , GxB_ANY_PLUS_UINT64    ,
    GxB_MIN_PLUS_FP32      , GxB_MAX_PLUS_FP32      , GxB_PLUS_PLUS_FP32     , GxB_TIMES_PLUS_FP32    , GxB_ANY_PLUS_FP32      ,
    GxB_MIN_PLUS_FP64      , GxB_MAX_PLUS_FP64      , GxB_PLUS_PLUS_FP64     , GxB_TIMES_PLUS_FP64    , GxB_ANY_PLUS_FP64      ,

    // semirings with multiply op: z = MINUS (x,y), all types x,y,z the same:
    GxB_MIN_MINUS_INT8     , GxB_MAX_MINUS_INT8     , GxB_PLUS_MINUS_INT8    , GxB_TIMES_MINUS_INT8   , GxB_ANY_MINUS_INT8     ,
    GxB_MIN_MINUS_INT16    , GxB_MAX_MINUS_INT16    , GxB_PLUS_MINUS_INT16   , GxB_TIMES_MINUS_INT16  , GxB_ANY_MINUS_INT16    ,
    GxB_MIN_MINUS_INT32    , GxB_MAX_MINUS_INT32    , GxB_PLUS_MINUS_INT32   , GxB_TIMES_MINUS_INT32  , GxB_ANY_MINUS_INT32    ,
    GxB_MIN_MINUS_INT64    , GxB_MAX_MINUS_INT64    , GxB_PLUS_MINUS_INT64   , GxB_TIMES_MINUS_INT64  , GxB_ANY_MINUS_INT64    ,
    GxB_MIN_MINUS_UINT8    , GxB_MAX_MINUS_UINT8    , GxB_PLUS_MINUS_UINT8   , GxB_TIMES_MINUS_UINT8  , GxB_ANY_MINUS_UINT8    ,
    GxB_MIN_MINUS_UINT16   , GxB_MAX_MINUS_UINT16   , GxB_PLUS_MINUS_UINT16  , GxB_TIMES_MINUS_UINT16 , GxB_ANY_MINUS_UINT16   ,
    GxB_MIN_MINUS_UINT32   , GxB_MAX_MINUS_UINT32   , GxB_PLUS_MINUS_UINT32  , GxB_TIMES_MINUS_UINT32 , GxB_ANY_MINUS_UINT32   ,
    GxB_MIN_MINUS_UINT64   , GxB_MAX_MINUS_UINT64   , GxB_PLUS_MINUS_UINT64  , GxB_TIMES_MINUS_UINT64 , GxB_ANY_MINUS_UINT64   ,
    GxB_MIN_MINUS_FP32     , GxB_MAX_MINUS_FP32     , GxB_PLUS_MINUS_FP32    , GxB_TIMES_MINUS_FP32   , GxB_ANY_MINUS_FP32     ,
    GxB_MIN_MINUS_FP64     , GxB_MAX_MINUS_FP64     , GxB_PLUS_MINUS_FP64    , GxB_TIMES_MINUS_FP64   , GxB_ANY_MINUS_FP64     ,

    // semirings with multiply op: z = TIMES (x,y), all types x,y,z the same:
    GxB_MIN_TIMES_INT8     , GxB_MAX_TIMES_INT8     , GxB_PLUS_TIMES_INT8    , GxB_TIMES_TIMES_INT8   , GxB_ANY_TIMES_INT8     ,
    GxB_MIN_TIMES_INT16    , GxB_MAX_TIMES_INT16    , GxB_PLUS_TIMES_INT16   , GxB_TIMES_TIMES_INT16  , GxB_ANY_TIMES_INT16    ,
    GxB_MIN_TIMES_INT32    , GxB_MAX_TIMES_INT32    , GxB_PLUS_TIMES_INT32   , GxB_TIMES_TIMES_INT32  , GxB_ANY_TIMES_INT32    ,
    GxB_MIN_TIMES_INT64    , GxB_MAX_TIMES_INT64    , GxB_PLUS_TIMES_INT64   , GxB_TIMES_TIMES_INT64  , GxB_ANY_TIMES_INT64    ,
    GxB_MIN_TIMES_UINT8    , GxB_MAX_TIMES_UINT8    , GxB_PLUS_TIMES_UINT8   , GxB_TIMES_TIMES_UINT8  , GxB_ANY_TIMES_UINT8    ,
    GxB_MIN_TIMES_UINT16   , GxB_MAX_TIMES_UINT16   , GxB_PLUS_TIMES_UINT16  , GxB_TIMES_TIMES_UINT16 , GxB_ANY_TIMES_UINT16   ,
    GxB_MIN_TIMES_UINT32   , GxB_MAX_TIMES_UINT32   , GxB_PLUS_TIMES_UINT32  , GxB_TIMES_TIMES_UINT32 , GxB_ANY_TIMES_UINT32   ,
    GxB_MIN_TIMES_UINT64   , GxB_MAX_TIMES_UINT64   , GxB_PLUS_TIMES_UINT64  , GxB_TIMES_TIMES_UINT64 , GxB_ANY_TIMES_UINT64   ,
    GxB_MIN_TIMES_FP32     , GxB_MAX_TIMES_FP32     , GxB_PLUS_TIMES_FP32    , GxB_TIMES_TIMES_FP32   , GxB_ANY_TIMES_FP32     ,
    GxB_MIN_TIMES_FP64     , GxB_MAX_TIMES_FP64     , GxB_PLUS_TIMES_FP64    , GxB_TIMES_TIMES_FP64   , GxB_ANY_TIMES_FP64     ,

    // semirings with multiply op: z = DIV (x,y), all types x,y,z the same:
    GxB_MIN_DIV_INT8       , GxB_MAX_DIV_INT8       , GxB_PLUS_DIV_INT8      , GxB_TIMES_DIV_INT8     , GxB_ANY_DIV_INT8       ,
    GxB_MIN_DIV_INT16      , GxB_MAX_DIV_INT16      , GxB_PLUS_DIV_INT16     , GxB_TIMES_DIV_INT16    , GxB_ANY_DIV_INT16      ,
    GxB_MIN_DIV_INT32      , GxB_MAX_DIV_INT32      , GxB_PLUS_DIV_INT32     , GxB_TIMES_DIV_INT32    , GxB_ANY_DIV_INT32      ,
    GxB_MIN_DIV_INT64      , GxB_MAX_DIV_INT64      , GxB_PLUS_DIV_INT64     , GxB_TIMES_DIV_INT64    , GxB_ANY_DIV_INT64      ,
    GxB_MIN_DIV_UINT8      , GxB_MAX_DIV_UINT8      , GxB_PLUS_DIV_UINT8     , GxB_TIMES_DIV_UINT8    , GxB_ANY_DIV_UINT8      ,
    GxB_MIN_DIV_UINT16     , GxB_MAX_DIV_UINT16     , GxB_PLUS_DIV_UINT16    , GxB_TIMES_DIV_UINT16   , GxB_ANY_DIV_UINT16     ,
    GxB_MIN_DIV_UINT32     , GxB_MAX_DIV_UINT32     , GxB_PLUS_DIV_UINT32    , GxB_TIMES_DIV_UINT32   , GxB_ANY_DIV_UINT32     ,
    GxB_MIN_DIV_UINT64     , GxB_MAX_DIV_UINT64     , GxB_PLUS_DIV_UINT64    , GxB_TIMES_DIV_UINT64   , GxB_ANY_DIV_UINT64     ,
    GxB_MIN_DIV_FP32       , GxB_MAX_DIV_FP32       , GxB_PLUS_DIV_FP32      , GxB_TIMES_DIV_FP32     , GxB_ANY_DIV_FP32       ,
    GxB_MIN_DIV_FP64       , GxB_MAX_DIV_FP64       , GxB_PLUS_DIV_FP64      , GxB_TIMES_DIV_FP64     , GxB_ANY_DIV_FP64       ,

    // semirings with multiply op: z = RDIV (x,y), all types x,y,z the same:
    GxB_MIN_RDIV_INT8      , GxB_MAX_RDIV_INT8      , GxB_PLUS_RDIV_INT8     , GxB_TIMES_RDIV_INT8    , GxB_ANY_RDIV_INT8      ,
    GxB_MIN_RDIV_INT16     , GxB_MAX_RDIV_INT16     , GxB_PLUS_RDIV_INT16    , GxB_TIMES_RDIV_INT16   , GxB_ANY_RDIV_INT16     ,
    GxB_MIN_RDIV_INT32     , GxB_MAX_RDIV_INT32     , GxB_PLUS_RDIV_INT32    , GxB_TIMES_RDIV_INT32   , GxB_ANY_RDIV_INT32     ,
    GxB_MIN_RDIV_INT64     , GxB_MAX_RDIV_INT64     , GxB_PLUS_RDIV_INT64    , GxB_TIMES_RDIV_INT64   , GxB_ANY_RDIV_INT64     ,
    GxB_MIN_RDIV_UINT8     , GxB_MAX_RDIV_UINT8     , GxB_PLUS_RDIV_UINT8    , GxB_TIMES_RDIV_UINT8   , GxB_ANY_RDIV_UINT8     ,
    GxB_MIN_RDIV_UINT16    , GxB_MAX_RDIV_UINT16    , GxB_PLUS_RDIV_UINT16   , GxB_TIMES_RDIV_UINT16  , GxB_ANY_RDIV_UINT16    ,
    GxB_MIN_RDIV_UINT32    , GxB_MAX_RDIV_UINT32    , GxB_PLUS_RDIV_UINT32   , GxB_TIMES_RDIV_UINT32  , GxB_ANY_RDIV_UINT32    ,
    GxB_MIN_RDIV_UINT64    , GxB_MAX_RDIV_UINT64    , GxB_PLUS_RDIV_UINT64   , GxB_TIMES_RDIV_UINT64  , GxB_ANY_RDIV_UINT64    ,
    GxB_MIN_RDIV_FP32      , GxB_MAX_RDIV_FP32      , GxB_PLUS_RDIV_FP32     , GxB_TIMES_RDIV_FP32    , GxB_ANY_RDIV_FP32      ,
    GxB_MIN_RDIV_FP64      , GxB_MAX_RDIV_FP64      , GxB_PLUS_RDIV_FP64     , GxB_TIMES_RDIV_FP64    , GxB_ANY_RDIV_FP64      ,

    // semirings with multiply op: z = RMINUS (x,y), all types x,y,z the same:
    GxB_MIN_RMINUS_INT8    , GxB_MAX_RMINUS_INT8    , GxB_PLUS_RMINUS_INT8   , GxB_TIMES_RMINUS_INT8  , GxB_ANY_RMINUS_INT8    ,
    GxB_MIN_RMINUS_INT16   , GxB_MAX_RMINUS_INT16   , GxB_PLUS_RMINUS_INT16  , GxB_TIMES_RMINUS_INT16 , GxB_ANY_RMINUS_INT16   ,
    GxB_MIN_RMINUS_INT32   , GxB_MAX_RMINUS_INT32   , GxB_PLUS_RMINUS_INT32  , GxB_TIMES_RMINUS_INT32 , GxB_ANY_RMINUS_INT32   ,
    GxB_MIN_RMINUS_INT64   , GxB_MAX_RMINUS_INT64   , GxB_PLUS_RMINUS_INT64  , GxB_TIMES_RMINUS_INT64 , GxB_ANY_RMINUS_INT64   ,
    GxB_MIN_RMINUS_UINT8   , GxB_MAX_RMINUS_UINT8   , GxB_PLUS_RMINUS_UINT8  , GxB_TIMES_RMINUS_UINT8 , GxB_ANY_RMINUS_UINT8   ,
    GxB_MIN_RMINUS_UINT16  , GxB_MAX_RMINUS_UINT16  , GxB_PLUS_RMINUS_UINT16 , GxB_TIMES_RMINUS_UINT16, GxB_ANY_RMINUS_UINT16  ,
    GxB_MIN_RMINUS_UINT32  , GxB_MAX_RMINUS_UINT32  , GxB_PLUS_RMINUS_UINT32 , GxB_TIMES_RMINUS_UINT32, GxB_ANY_RMINUS_UINT32  ,
    GxB_MIN_RMINUS_UINT64  , GxB_MAX_RMINUS_UINT64  , GxB_PLUS_RMINUS_UINT64 , GxB_TIMES_RMINUS_UINT64, GxB_ANY_RMINUS_UINT64  ,
    GxB_MIN_RMINUS_FP32    , GxB_MAX_RMINUS_FP32    , GxB_PLUS_RMINUS_FP32   , GxB_TIMES_RMINUS_FP32  , GxB_ANY_RMINUS_FP32    ,
    GxB_MIN_RMINUS_FP64    , GxB_MAX_RMINUS_FP64    , GxB_PLUS_RMINUS_FP64   , GxB_TIMES_RMINUS_FP64  , GxB_ANY_RMINUS_FP64    ,

    // semirings with multiply op: z = ISEQ (x,y), all types x,y,z the same:
    GxB_MIN_ISEQ_INT8      , GxB_MAX_ISEQ_INT8      , GxB_PLUS_ISEQ_INT8     , GxB_TIMES_ISEQ_INT8    , GxB_ANY_ISEQ_INT8      ,
    GxB_MIN_ISEQ_INT16     , GxB_MAX_ISEQ_INT16     , GxB_PLUS_ISEQ_INT16    , GxB_TIMES_ISEQ_INT16   , GxB_ANY_ISEQ_INT16     ,
    GxB_MIN_ISEQ_INT32     , GxB_MAX_ISEQ_INT32     , GxB_PLUS_ISEQ_INT32    , GxB_TIMES_ISEQ_INT32   , GxB_ANY_ISEQ_INT32     ,
    GxB_MIN_ISEQ_INT64     , GxB_MAX_ISEQ_INT64     , GxB_PLUS_ISEQ_INT64    , GxB_TIMES_ISEQ_INT64   , GxB_ANY_ISEQ_INT64     ,
    GxB_MIN_ISEQ_UINT8     , GxB_MAX_ISEQ_UINT8     , GxB_PLUS_ISEQ_UINT8    , GxB_TIMES_ISEQ_UINT8   , GxB_ANY_ISEQ_UINT8     ,
    GxB_MIN_ISEQ_UINT16    , GxB_MAX_ISEQ_UINT16    , GxB_PLUS_ISEQ_UINT16   , GxB_TIMES_ISEQ_UINT16  , GxB_ANY_ISEQ_UINT16    ,
    GxB_MIN_ISEQ_UINT32    , GxB_MAX_ISEQ_UINT32    , GxB_PLUS_ISEQ_UINT32   , GxB_TIMES_ISEQ_UINT32  , GxB_ANY_ISEQ_UINT32    ,
    GxB_MIN_ISEQ_UINT64    , GxB_MAX_ISEQ_UINT64    , GxB_PLUS_ISEQ_UINT64   , GxB_TIMES_ISEQ_UINT64  , GxB_ANY_ISEQ_UINT64    ,
    GxB_MIN_ISEQ_FP32      , GxB_MAX_ISEQ_FP32      , GxB_PLUS_ISEQ_FP32     , GxB_TIMES_ISEQ_FP32    , GxB_ANY_ISEQ_FP32      ,
    GxB_MIN_ISEQ_FP64      , GxB_MAX_ISEQ_FP64      , GxB_PLUS_ISEQ_FP64     , GxB_TIMES_ISEQ_FP64    , GxB_ANY_ISEQ_FP64      ,

    // semirings with multiply op: z = ISNE (x,y), all types x,y,z the same:
    GxB_MIN_ISNE_INT8      , GxB_MAX_ISNE_INT8      , GxB_PLUS_ISNE_INT8     , GxB_TIMES_ISNE_INT8    , GxB_ANY_ISNE_INT8      ,
    GxB_MIN_ISNE_INT16     , GxB_MAX_ISNE_INT16     , GxB_PLUS_ISNE_INT16    , GxB_TIMES_ISNE_INT16   , GxB_ANY_ISNE_INT16     ,
    GxB_MIN_ISNE_INT32     , GxB_MAX_ISNE_INT32     , GxB_PLUS_ISNE_INT32    , GxB_TIMES_ISNE_INT32   , GxB_ANY_ISNE_INT32     ,
    GxB_MIN_ISNE_INT64     , GxB_MAX_ISNE_INT64     , GxB_PLUS_ISNE_INT64    , GxB_TIMES_ISNE_INT64   , GxB_ANY_ISNE_INT64     ,
    GxB_MIN_ISNE_UINT8     , GxB_MAX_ISNE_UINT8     , GxB_PLUS_ISNE_UINT8    , GxB_TIMES_ISNE_UINT8   , GxB_ANY_ISNE_UINT8     ,
    GxB_MIN_ISNE_UINT16    , GxB_MAX_ISNE_UINT16    , GxB_PLUS_ISNE_UINT16   , GxB_TIMES_ISNE_UINT16  , GxB_ANY_ISNE_UINT16    ,
    GxB_MIN_ISNE_UINT32    , GxB_MAX_ISNE_UINT32    , GxB_PLUS_ISNE_UINT32   , GxB_TIMES_ISNE_UINT32  , GxB_ANY_ISNE_UINT32    ,
    GxB_MIN_ISNE_UINT64    , GxB_MAX_ISNE_UINT64    , GxB_PLUS_ISNE_UINT64   , GxB_TIMES_ISNE_UINT64  , GxB_ANY_ISNE_UINT64    ,
    GxB_MIN_ISNE_FP32      , GxB_MAX_ISNE_FP32      , GxB_PLUS_ISNE_FP32     , GxB_TIMES_ISNE_FP32    , GxB_ANY_ISNE_FP32      ,
    GxB_MIN_ISNE_FP64      , GxB_MAX_ISNE_FP64      , GxB_PLUS_ISNE_FP64     , GxB_TIMES_ISNE_FP64    , GxB_ANY_ISNE_FP64      ,

    // semirings with multiply op: z = ISGT (x,y), all types x,y,z the same:
    GxB_MIN_ISGT_INT8      , GxB_MAX_ISGT_INT8      , GxB_PLUS_ISGT_INT8     , GxB_TIMES_ISGT_INT8    , GxB_ANY_ISGT_INT8      ,
    GxB_MIN_ISGT_INT16     , GxB_MAX_ISGT_INT16     , GxB_PLUS_ISGT_INT16    , GxB_TIMES_ISGT_INT16   , GxB_ANY_ISGT_INT16     ,
    GxB_MIN_ISGT_INT32     , GxB_MAX_ISGT_INT32     , GxB_PLUS_ISGT_INT32    , GxB_TIMES_ISGT_INT32   , GxB_ANY_ISGT_INT32     ,
    GxB_MIN_ISGT_INT64     , GxB_MAX_ISGT_INT64     , GxB_PLUS_ISGT_INT64    , GxB_TIMES_ISGT_INT64   , GxB_ANY_ISGT_INT64     ,
    GxB_MIN_ISGT_UINT8     , GxB_MAX_ISGT_UINT8     , GxB_PLUS_ISGT_UINT8    , GxB_TIMES_ISGT_UINT8   , GxB_ANY_ISGT_UINT8     ,
    GxB_MIN_ISGT_UINT16    , GxB_MAX_ISGT_UINT16    , GxB_PLUS_ISGT_UINT16   , GxB_TIMES_ISGT_UINT16  , GxB_ANY_ISGT_UINT16    ,
    GxB_MIN_ISGT_UINT32    , GxB_MAX_ISGT_UINT32    , GxB_PLUS_ISGT_UINT32   , GxB_TIMES_ISGT_UINT32  , GxB_ANY_ISGT_UINT32    ,
    GxB_MIN_ISGT_UINT64    , GxB_MAX_ISGT_UINT64    , GxB_PLUS_ISGT_UINT64   , GxB_TIMES_ISGT_UINT64  , GxB_ANY_ISGT_UINT64    ,
    GxB_MIN_ISGT_FP32      , GxB_MAX_ISGT_FP32      , GxB_PLUS_ISGT_FP32     , GxB_TIMES_ISGT_FP32    , GxB_ANY_ISGT_FP32      ,
    GxB_MIN_ISGT_FP64      , GxB_MAX_ISGT_FP64      , GxB_PLUS_ISGT_FP64     , GxB_TIMES_ISGT_FP64    , GxB_ANY_ISGT_FP64      ,

    // semirings with multiply op: z = ISLT (x,y), all types x,y,z the same:
    GxB_MIN_ISLT_INT8      , GxB_MAX_ISLT_INT8      , GxB_PLUS_ISLT_INT8     , GxB_TIMES_ISLT_INT8    , GxB_ANY_ISLT_INT8      ,
    GxB_MIN_ISLT_INT16     , GxB_MAX_ISLT_INT16     , GxB_PLUS_ISLT_INT16    , GxB_TIMES_ISLT_INT16   , GxB_ANY_ISLT_INT16     ,
    GxB_MIN_ISLT_INT32     , GxB_MAX_ISLT_INT32     , GxB_PLUS_ISLT_INT32    , GxB_TIMES_ISLT_INT32   , GxB_ANY_ISLT_INT32     ,
    GxB_MIN_ISLT_INT64     , GxB_MAX_ISLT_INT64     , GxB_PLUS_ISLT_INT64    , GxB_TIMES_ISLT_INT64   , GxB_ANY_ISLT_INT64     ,
    GxB_MIN_ISLT_UINT8     , GxB_MAX_ISLT_UINT8     , GxB_PLUS_ISLT_UINT8    , GxB_TIMES_ISLT_UINT8   , GxB_ANY_ISLT_UINT8     ,
    GxB_MIN_ISLT_UINT16    , GxB_MAX_ISLT_UINT16    , GxB_PLUS_ISLT_UINT16   , GxB_TIMES_ISLT_UINT16  , GxB_ANY_ISLT_UINT16    ,
    GxB_MIN_ISLT_UINT32    , GxB_MAX_ISLT_UINT32    , GxB_PLUS_ISLT_UINT32   , GxB_TIMES_ISLT_UINT32  , GxB_ANY_ISLT_UINT32    ,
    GxB_MIN_ISLT_UINT64    , GxB_MAX_ISLT_UINT64    , GxB_PLUS_ISLT_UINT64   , GxB_TIMES_ISLT_UINT64  , GxB_ANY_ISLT_UINT64    ,
    GxB_MIN_ISLT_FP32      , GxB_MAX_ISLT_FP32      , GxB_PLUS_ISLT_FP32     , GxB_TIMES_ISLT_FP32    , GxB_ANY_ISLT_FP32      ,
    GxB_MIN_ISLT_FP64      , GxB_MAX_ISLT_FP64      , GxB_PLUS_ISLT_FP64     , GxB_TIMES_ISLT_FP64    , GxB_ANY_ISLT_FP64      ,

    // semirings with multiply op: z = ISGE (x,y), all types x,y,z the same:
    GxB_MIN_ISGE_INT8      , GxB_MAX_ISGE_INT8      , GxB_PLUS_ISGE_INT8     , GxB_TIMES_ISGE_INT8    , GxB_ANY_ISGE_INT8      ,
    GxB_MIN_ISGE_INT16     , GxB_MAX_ISGE_INT16     , GxB_PLUS_ISGE_INT16    , GxB_TIMES_ISGE_INT16   , GxB_ANY_ISGE_INT16     ,
    GxB_MIN_ISGE_INT32     , GxB_MAX_ISGE_INT32     , GxB_PLUS_ISGE_INT32    , GxB_TIMES_ISGE_INT32   , GxB_ANY_ISGE_INT32     ,
    GxB_MIN_ISGE_INT64     , GxB_MAX_ISGE_INT64     , GxB_PLUS_ISGE_INT64    , GxB_TIMES_ISGE_INT64   , GxB_ANY_ISGE_INT64     ,
    GxB_MIN_ISGE_UINT8     , GxB_MAX_ISGE_UINT8     , GxB_PLUS_ISGE_UINT8    , GxB_TIMES_ISGE_UINT8   , GxB_ANY_ISGE_UINT8     ,
    GxB_MIN_ISGE_UINT16    , GxB_MAX_ISGE_UINT16    , GxB_PLUS_ISGE_UINT16   , GxB_TIMES_ISGE_UINT16  , GxB_ANY_ISGE_UINT16    ,
    GxB_MIN_ISGE_UINT32    , GxB_MAX_ISGE_UINT32    , GxB_PLUS_ISGE_UINT32   , GxB_TIMES_ISGE_UINT32  , GxB_ANY_ISGE_UINT32    ,
    GxB_MIN_ISGE_UINT64    , GxB_MAX_ISGE_UINT64    , GxB_PLUS_ISGE_UINT64   , GxB_TIMES_ISGE_UINT64  , GxB_ANY_ISGE_UINT64    ,
    GxB_MIN_ISGE_FP32      , GxB_MAX_ISGE_FP32      , GxB_PLUS_ISGE_FP32     , GxB_TIMES_ISGE_FP32    , GxB_ANY_ISGE_FP32      ,
    GxB_MIN_ISGE_FP64      , GxB_MAX_ISGE_FP64      , GxB_PLUS_ISGE_FP64     , GxB_TIMES_ISGE_FP64    , GxB_ANY_ISGE_FP64      ,

    // semirings with multiply op: z = ISLE (x,y), all types x,y,z the same:
    GxB_MIN_ISLE_INT8      , GxB_MAX_ISLE_INT8      , GxB_PLUS_ISLE_INT8     , GxB_TIMES_ISLE_INT8    , GxB_ANY_ISLE_INT8      ,
    GxB_MIN_ISLE_INT16     , GxB_MAX_ISLE_INT16     , GxB_PLUS_ISLE_INT16    , GxB_TIMES_ISLE_INT16   , GxB_ANY_ISLE_INT16     ,
    GxB_MIN_ISLE_INT32     , GxB_MAX_ISLE_INT32     , GxB_PLUS_ISLE_INT32    , GxB_TIMES_ISLE_INT32   , GxB_ANY_ISLE_INT32     ,
    GxB_MIN_ISLE_INT64     , GxB_MAX_ISLE_INT64     , GxB_PLUS_ISLE_INT64    , GxB_TIMES_ISLE_INT64   , GxB_ANY_ISLE_INT64     ,
    GxB_MIN_ISLE_UINT8     , GxB_MAX_ISLE_UINT8     , GxB_PLUS_ISLE_UINT8    , GxB_TIMES_ISLE_UINT8   , GxB_ANY_ISLE_UINT8     ,
    GxB_MIN_ISLE_UINT16    , GxB_MAX_ISLE_UINT16    , GxB_PLUS_ISLE_UINT16   , GxB_TIMES_ISLE_UINT16  , GxB_ANY_ISLE_UINT16    ,
    GxB_MIN_ISLE_UINT32    , GxB_MAX_ISLE_UINT32    , GxB_PLUS_ISLE_UINT32   , GxB_TIMES_ISLE_UINT32  , GxB_ANY_ISLE_UINT32    ,
    GxB_MIN_ISLE_UINT64    , GxB_MAX_ISLE_UINT64    , GxB_PLUS_ISLE_UINT64   , GxB_TIMES_ISLE_UINT64  , GxB_ANY_ISLE_UINT64    ,
    GxB_MIN_ISLE_FP32      , GxB_MAX_ISLE_FP32      , GxB_PLUS_ISLE_FP32     , GxB_TIMES_ISLE_FP32    , GxB_ANY_ISLE_FP32      ,
    GxB_MIN_ISLE_FP64      , GxB_MAX_ISLE_FP64      , GxB_PLUS_ISLE_FP64     , GxB_TIMES_ISLE_FP64    , GxB_ANY_ISLE_FP64      ,

    // semirings with multiply op: z = LOR (x,y), all types x,y,z the same:
    GxB_MIN_LOR_INT8       , GxB_MAX_LOR_INT8       , GxB_PLUS_LOR_INT8      , GxB_TIMES_LOR_INT8     , GxB_ANY_LOR_INT8       ,
    GxB_MIN_LOR_INT16      , GxB_MAX_LOR_INT16      , GxB_PLUS_LOR_INT16     , GxB_TIMES_LOR_INT16    , GxB_ANY_LOR_INT16      ,
    GxB_MIN_LOR_INT32      , GxB_MAX_LOR_INT32      , GxB_PLUS_LOR_INT32     , GxB_TIMES_LOR_INT32    , GxB_ANY_LOR_INT32      ,
    GxB_MIN_LOR_INT64      , GxB_MAX_LOR_INT64      , GxB_PLUS_LOR_INT64     , GxB_TIMES_LOR_INT64    , GxB_ANY_LOR_INT64      ,
    GxB_MIN_LOR_UINT8      , GxB_MAX_LOR_UINT8      , GxB_PLUS_LOR_UINT8     , GxB_TIMES_LOR_UINT8    , GxB_ANY_LOR_UINT8      ,
    GxB_MIN_LOR_UINT16     , GxB_MAX_LOR_UINT16     , GxB_PLUS_LOR_UINT16    , GxB_TIMES_LOR_UINT16   , GxB_ANY_LOR_UINT16     ,
    GxB_MIN_LOR_UINT32     , GxB_MAX_LOR_UINT32     , GxB_PLUS_LOR_UINT32    , GxB_TIMES_LOR_UINT32   , GxB_ANY_LOR_UINT32     ,
    GxB_MIN_LOR_UINT64     , GxB_MAX_LOR_UINT64     , GxB_PLUS_LOR_UINT64    , GxB_TIMES_LOR_UINT64   , GxB_ANY_LOR_UINT64     ,
    GxB_MIN_LOR_FP32       , GxB_MAX_LOR_FP32       , GxB_PLUS_LOR_FP32      , GxB_TIMES_LOR_FP32     , GxB_ANY_LOR_FP32       ,
    GxB_MIN_LOR_FP64       , GxB_MAX_LOR_FP64       , GxB_PLUS_LOR_FP64      , GxB_TIMES_LOR_FP64     , GxB_ANY_LOR_FP64       ,

    // semirings with multiply op: z = LAND (x,y), all types x,y,z the same:
    GxB_MIN_LAND_INT8      , GxB_MAX_LAND_INT8      , GxB_PLUS_LAND_INT8     , GxB_TIMES_LAND_INT8    , GxB_ANY_LAND_INT8      ,
    GxB_MIN_LAND_INT16     , GxB_MAX_LAND_INT16     , GxB_PLUS_LAND_INT16    , GxB_TIMES_LAND_INT16   , GxB_ANY_LAND_INT16     ,
    GxB_MIN_LAND_INT32     , GxB_MAX_LAND_INT32     , GxB_PLUS_LAND_INT32    , GxB_TIMES_LAND_INT32   , GxB_ANY_LAND_INT32     ,
    GxB_MIN_LAND_INT64     , GxB_MAX_LAND_INT64     , GxB_PLUS_LAND_INT64    , GxB_TIMES_LAND_INT64   , GxB_ANY_LAND_INT64     ,
    GxB_MIN_LAND_UINT8     , GxB_MAX_LAND_UINT8     , GxB_PLUS_LAND_UINT8    , GxB_TIMES_LAND_UINT8   , GxB_ANY_LAND_UINT8     ,
    GxB_MIN_LAND_UINT16    , GxB_MAX_LAND_UINT16    , GxB_PLUS_LAND_UINT16   , GxB_TIMES_LAND_UINT16  , GxB_ANY_LAND_UINT16    ,
    GxB_MIN_LAND_UINT32    , GxB_MAX_LAND_UINT32    , GxB_PLUS_LAND_UINT32   , GxB_TIMES_LAND_UINT32  , GxB_ANY_LAND_UINT32    ,
    GxB_MIN_LAND_UINT64    , GxB_MAX_LAND_UINT64    , GxB_PLUS_LAND_UINT64   , GxB_TIMES_LAND_UINT64  , GxB_ANY_LAND_UINT64    ,
    GxB_MIN_LAND_FP32      , GxB_MAX_LAND_FP32      , GxB_PLUS_LAND_FP32     , GxB_TIMES_LAND_FP32    , GxB_ANY_LAND_FP32      ,
    GxB_MIN_LAND_FP64      , GxB_MAX_LAND_FP64      , GxB_PLUS_LAND_FP64     , GxB_TIMES_LAND_FP64    , GxB_ANY_LAND_FP64      ,

    // semirings with multiply op: z = LXOR (x,y), all types x,y,z the same:
    GxB_MIN_LXOR_INT8      , GxB_MAX_LXOR_INT8      , GxB_PLUS_LXOR_INT8     , GxB_TIMES_LXOR_INT8    , GxB_ANY_LXOR_INT8      ,
    GxB_MIN_LXOR_INT16     , GxB_MAX_LXOR_INT16     , GxB_PLUS_LXOR_INT16    , GxB_TIMES_LXOR_INT16   , GxB_ANY_LXOR_INT16     ,
    GxB_MIN_LXOR_INT32     , GxB_MAX_LXOR_INT32     , GxB_PLUS_LXOR_INT32    , GxB_TIMES_LXOR_INT32   , GxB_ANY_LXOR_INT32     ,
    GxB_MIN_LXOR_INT64     , GxB_MAX_LXOR_INT64     , GxB_PLUS_LXOR_INT64    , GxB_TIMES_LXOR_INT64   , GxB_ANY_LXOR_INT64     ,
    GxB_MIN_LXOR_UINT8     , GxB_MAX_LXOR_UINT8     , GxB_PLUS_LXOR_UINT8    , GxB_TIMES_LXOR_UINT8   , GxB_ANY_LXOR_UINT8     ,
    GxB_MIN_LXOR_UINT16    , GxB_MAX_LXOR_UINT16    , GxB_PLUS_LXOR_UINT16   , GxB_TIMES_LXOR_UINT16  , GxB_ANY_LXOR_UINT16    ,
    GxB_MIN_LXOR_UINT32    , GxB_MAX_LXOR_UINT32    , GxB_PLUS_LXOR_UINT32   , GxB_TIMES_LXOR_UINT32  , GxB_ANY_LXOR_UINT32    ,
    GxB_MIN_LXOR_UINT64    , GxB_MAX_LXOR_UINT64    , GxB_PLUS_LXOR_UINT64   , GxB_TIMES_LXOR_UINT64  , GxB_ANY_LXOR_UINT64    ,
    GxB_MIN_LXOR_FP32      , GxB_MAX_LXOR_FP32      , GxB_PLUS_LXOR_FP32     , GxB_TIMES_LXOR_FP32    , GxB_ANY_LXOR_FP32      ,
    GxB_MIN_LXOR_FP64      , GxB_MAX_LXOR_FP64      , GxB_PLUS_LXOR_FP64     , GxB_TIMES_LXOR_FP64    , GxB_ANY_LXOR_FP64      ,

//------------------------------------------------------------------------------
// 300 semirings with a comparison operator TxT -> bool, where T is non-Boolean
//------------------------------------------------------------------------------

    // In the 4th column the GxB_EQ_*_* semirings could also be called
    // GxB_LXNOR_*_*, since the EQ and LXNOR boolean operators are identical
    // but those names are not included.

    // semirings with multiply op: z = EQ (x,y), where z is boolean and x,y are given by the suffix:
    GxB_LOR_EQ_INT8        , GxB_LAND_EQ_INT8       , GxB_LXOR_EQ_INT8       , GxB_EQ_EQ_INT8         , GxB_ANY_EQ_INT8        ,
    GxB_LOR_EQ_INT16       , GxB_LAND_EQ_INT16      , GxB_LXOR_EQ_INT16      , GxB_EQ_EQ_INT16        , GxB_ANY_EQ_INT16       ,
    GxB_LOR_EQ_INT32       , GxB_LAND_EQ_INT32      , GxB_LXOR_EQ_INT32      , GxB_EQ_EQ_INT32        , GxB_ANY_EQ_INT32       ,
    GxB_LOR_EQ_INT64       , GxB_LAND_EQ_INT64      , GxB_LXOR_EQ_INT64      , GxB_EQ_EQ_INT64        , GxB_ANY_EQ_INT64       ,
    GxB_LOR_EQ_UINT8       , GxB_LAND_EQ_UINT8      , GxB_LXOR_EQ_UINT8      , GxB_EQ_EQ_UINT8        , GxB_ANY_EQ_UINT8       ,
    GxB_LOR_EQ_UINT16      , GxB_LAND_EQ_UINT16     , GxB_LXOR_EQ_UINT16     , GxB_EQ_EQ_UINT16       , GxB_ANY_EQ_UINT16      ,
    GxB_LOR_EQ_UINT32      , GxB_LAND_EQ_UINT32     , GxB_LXOR_EQ_UINT32     , GxB_EQ_EQ_UINT32       , GxB_ANY_EQ_UINT32      ,
    GxB_LOR_EQ_UINT64      , GxB_LAND_EQ_UINT64     , GxB_LXOR_EQ_UINT64     , GxB_EQ_EQ_UINT64       , GxB_ANY_EQ_UINT64      ,
    GxB_LOR_EQ_FP32        , GxB_LAND_EQ_FP32       , GxB_LXOR_EQ_FP32       , GxB_EQ_EQ_FP32         , GxB_ANY_EQ_FP32        ,
    GxB_LOR_EQ_FP64        , GxB_LAND_EQ_FP64       , GxB_LXOR_EQ_FP64       , GxB_EQ_EQ_FP64         , GxB_ANY_EQ_FP64        ,

    // semirings with multiply op: z = NE (x,y), where z is boolean and x,y are given by the suffix:
    GxB_LOR_NE_INT8        , GxB_LAND_NE_INT8       , GxB_LXOR_NE_INT8       , GxB_EQ_NE_INT8         , GxB_ANY_NE_INT8        ,
    GxB_LOR_NE_INT16       , GxB_LAND_NE_INT16      , GxB_LXOR_NE_INT16      , GxB_EQ_NE_INT16        , GxB_ANY_NE_INT16       ,
    GxB_LOR_NE_INT32       , GxB_LAND_NE_INT32      , GxB_LXOR_NE_INT32      , GxB_EQ_NE_INT32        , GxB_ANY_NE_INT32       ,
    GxB_LOR_NE_INT64       , GxB_LAND_NE_INT64      , GxB_LXOR_NE_INT64      , GxB_EQ_NE_INT64        , GxB_ANY_NE_INT64       ,
    GxB_LOR_NE_UINT8       , GxB_LAND_NE_UINT8      , GxB_LXOR_NE_UINT8      , GxB_EQ_NE_UINT8        , GxB_ANY_NE_UINT8       ,
    GxB_LOR_NE_UINT16      , GxB_LAND_NE_UINT16     , GxB_LXOR_NE_UINT16     , GxB_EQ_NE_UINT16       , GxB_ANY_NE_UINT16      ,
    GxB_LOR_NE_UINT32      , GxB_LAND_NE_UINT32     , GxB_LXOR_NE_UINT32     , GxB_EQ_NE_UINT32       , GxB_ANY_NE_UINT32      ,
    GxB_LOR_NE_UINT64      , GxB_LAND_NE_UINT64     , GxB_LXOR_NE_UINT64     , GxB_EQ_NE_UINT64       , GxB_ANY_NE_UINT64      ,
    GxB_LOR_NE_FP32        , GxB_LAND_NE_FP32       , GxB_LXOR_NE_FP32       , GxB_EQ_NE_FP32         , GxB_ANY_NE_FP32        ,
    GxB_LOR_NE_FP64        , GxB_LAND_NE_FP64       , GxB_LXOR_NE_FP64       , GxB_EQ_NE_FP64         , GxB_ANY_NE_FP64        ,

    // semirings with multiply op: z = GT (x,y), where z is boolean and x,y are given by the suffix:
    GxB_LOR_GT_INT8        , GxB_LAND_GT_INT8       , GxB_LXOR_GT_INT8       , GxB_EQ_GT_INT8         , GxB_ANY_GT_INT8        ,
    GxB_LOR_GT_INT16       , GxB_LAND_GT_INT16      , GxB_LXOR_GT_INT16      , GxB_EQ_GT_INT16        , GxB_ANY_GT_INT16       ,
    GxB_LOR_GT_INT32       , GxB_LAND_GT_INT32      , GxB_LXOR_GT_INT32      , GxB_EQ_GT_INT32        , GxB_ANY_GT_INT32       ,
    GxB_LOR_GT_INT64       , GxB_LAND_GT_INT64      , GxB_LXOR_GT_INT64      , GxB_EQ_GT_INT64        , GxB_ANY_GT_INT64       ,
    GxB_LOR_GT_UINT8       , GxB_LAND_GT_UINT8      , GxB_LXOR_GT_UINT8      , GxB_EQ_GT_UINT8        , GxB_ANY_GT_UINT8       ,
    GxB_LOR_GT_UINT16      , GxB_LAND_GT_UINT16     , GxB_LXOR_GT_UINT16     , GxB_EQ_GT_UINT16       , GxB_ANY_GT_UINT16      ,
    GxB_LOR_GT_UINT32      , GxB_LAND_GT_UINT32     , GxB_LXOR_GT_UINT32     , GxB_EQ_GT_UINT32       , GxB_ANY_GT_UINT32      ,
    GxB_LOR_GT_UINT64      , GxB_LAND_GT_UINT64     , GxB_LXOR_GT_UINT64     , GxB_EQ_GT_UINT64       , GxB_ANY_GT_UINT64      ,
    GxB_LOR_GT_FP32        , GxB_LAND_GT_FP32       , GxB_LXOR_GT_FP32       , GxB_EQ_GT_FP32         , GxB_ANY_GT_FP32        ,
    GxB_LOR_GT_FP64        , GxB_LAND_GT_FP64       , GxB_LXOR_GT_FP64       , GxB_EQ_GT_FP64         , GxB_ANY_GT_FP64        ,

    // semirings with multiply op: z = LT (x,y), where z is boolean and x,y are given by the suffix:
    GxB_LOR_LT_INT8        , GxB_LAND_LT_INT8       , GxB_LXOR_LT_INT8       , GxB_EQ_LT_INT8         , GxB_ANY_LT_INT8        ,
    GxB_LOR_LT_INT16       , GxB_LAND_LT_INT16      , GxB_LXOR_LT_INT16      , GxB_EQ_LT_INT16        , GxB_ANY_LT_INT16       ,
    GxB_LOR_LT_INT32       , GxB_LAND_LT_INT32      , GxB_LXOR_LT_INT32      , GxB_EQ_LT_INT32        , GxB_ANY_LT_INT32       ,
    GxB_LOR_LT_INT64       , GxB_LAND_LT_INT64      , GxB_LXOR_LT_INT64      , GxB_EQ_LT_INT64        , GxB_ANY_LT_INT64       ,
    GxB_LOR_LT_UINT8       , GxB_LAND_LT_UINT8      , GxB_LXOR_LT_UINT8      , GxB_EQ_LT_UINT8        , GxB_ANY_LT_UINT8       ,
    GxB_LOR_LT_UINT16      , GxB_LAND_LT_UINT16     , GxB_LXOR_LT_UINT16     , GxB_EQ_LT_UINT16       , GxB_ANY_LT_UINT16      ,
    GxB_LOR_LT_UINT32      , GxB_LAND_LT_UINT32     , GxB_LXOR_LT_UINT32     , GxB_EQ_LT_UINT32       , GxB_ANY_LT_UINT32      ,
    GxB_LOR_LT_UINT64      , GxB_LAND_LT_UINT64     , GxB_LXOR_LT_UINT64     , GxB_EQ_LT_UINT64       , GxB_ANY_LT_UINT64      ,
    GxB_LOR_LT_FP32        , GxB_LAND_LT_FP32       , GxB_LXOR_LT_FP32       , GxB_EQ_LT_FP32         , GxB_ANY_LT_FP32        ,
    GxB_LOR_LT_FP64        , GxB_LAND_LT_FP64       , GxB_LXOR_LT_FP64       , GxB_EQ_LT_FP64         , GxB_ANY_LT_FP64        ,

    // semirings with multiply op: z = GE (x,y), where z is boolean and x,y are given by the suffix:
    GxB_LOR_GE_INT8        , GxB_LAND_GE_INT8       , GxB_LXOR_GE_INT8       , GxB_EQ_GE_INT8         , GxB_ANY_GE_INT8        ,
    GxB_LOR_GE_INT16       , GxB_LAND_GE_INT16      , GxB_LXOR_GE_INT16      , GxB_EQ_GE_INT16        , GxB_ANY_GE_INT16       ,
    GxB_LOR_GE_INT32       , GxB_LAND_GE_INT32      , GxB_LXOR_GE_INT32      , GxB_EQ_GE_INT32        , GxB_ANY_GE_INT32       ,
    GxB_LOR_GE_INT64       , GxB_LAND_GE_INT64      , GxB_LXOR_GE_INT64      , GxB_EQ_GE_INT64        , GxB_ANY_GE_INT64       ,
    GxB_LOR_GE_UINT8       , GxB_LAND_GE_UINT8      , GxB_LXOR_GE_UINT8      , GxB_EQ_GE_UINT8        , GxB_ANY_GE_UINT8       ,
    GxB_LOR_GE_UINT16      , GxB_LAND_GE_UINT16     , GxB_LXOR_GE_UINT16     , GxB_EQ_GE_UINT16       , GxB_ANY_GE_UINT16      ,
    GxB_LOR_GE_UINT32      , GxB_LAND_GE_UINT32     , GxB_LXOR_GE_UINT32     , GxB_EQ_GE_UINT32       , GxB_ANY_GE_UINT32      ,
    GxB_LOR_GE_UINT64      , GxB_LAND_GE_UINT64     , GxB_LXOR_GE_UINT64     , GxB_EQ_GE_UINT64       , GxB_ANY_GE_UINT64      ,
    GxB_LOR_GE_FP32        , GxB_LAND_GE_FP32       , GxB_LXOR_GE_FP32       , GxB_EQ_GE_FP32         , GxB_ANY_GE_FP32        ,
    GxB_LOR_GE_FP64        , GxB_LAND_GE_FP64       , GxB_LXOR_GE_FP64       , GxB_EQ_GE_FP64         , GxB_ANY_GE_FP64        ,

    // semirings with multiply op: z = LE (x,y), where z is boolean and x,y are given by the suffix:
    GxB_LOR_LE_INT8        , GxB_LAND_LE_INT8       , GxB_LXOR_LE_INT8       , GxB_EQ_LE_INT8         , GxB_ANY_LE_INT8        ,
    GxB_LOR_LE_INT16       , GxB_LAND_LE_INT16      , GxB_LXOR_LE_INT16      , GxB_EQ_LE_INT16        , GxB_ANY_LE_INT16       ,
    GxB_LOR_LE_INT32       , GxB_LAND_LE_INT32      , GxB_LXOR_LE_INT32      , GxB_EQ_LE_INT32        , GxB_ANY_LE_INT32       ,
    GxB_LOR_LE_INT64       , GxB_LAND_LE_INT64      , GxB_LXOR_LE_INT64      , GxB_EQ_LE_INT64        , GxB_ANY_LE_INT64       ,
    GxB_LOR_LE_UINT8       , GxB_LAND_LE_UINT8      , GxB_LXOR_LE_UINT8      , GxB_EQ_LE_UINT8        , GxB_ANY_LE_UINT8       ,
    GxB_LOR_LE_UINT16      , GxB_LAND_LE_UINT16     , GxB_LXOR_LE_UINT16     , GxB_EQ_LE_UINT16       , GxB_ANY_LE_UINT16      ,
    GxB_LOR_LE_UINT32      , GxB_LAND_LE_UINT32     , GxB_LXOR_LE_UINT32     , GxB_EQ_LE_UINT32       , GxB_ANY_LE_UINT32      ,
    GxB_LOR_LE_UINT64      , GxB_LAND_LE_UINT64     , GxB_LXOR_LE_UINT64     , GxB_EQ_LE_UINT64       , GxB_ANY_LE_UINT64      ,
    GxB_LOR_LE_FP32        , GxB_LAND_LE_FP32       , GxB_LXOR_LE_FP32       , GxB_EQ_LE_FP32         , GxB_ANY_LE_FP32        ,
    GxB_LOR_LE_FP64        , GxB_LAND_LE_FP64       , GxB_LXOR_LE_FP64       , GxB_EQ_LE_FP64         , GxB_ANY_LE_FP64        ,

//------------------------------------------------------------------------------
// 55 semirings with purely Boolean types, bool x bool -> bool
//------------------------------------------------------------------------------

    // Note that lor_pair, land_pair, and eq_pair are all identical to any_pair.
    // These 3 are marked below.  GxB_EQ_*_BOOL could be called
    // GxB_LXNOR_*_BOOL, and GxB_*_EQ_BOOL could be called GxB_*_LXNOR_BOOL,
    // but those names are not included.

    // purely boolean semirings in the form GxB_(add monoid)_(multipy operator)_BOOL:
    GxB_LOR_FIRST_BOOL     , GxB_LAND_FIRST_BOOL    , GxB_LXOR_FIRST_BOOL    , GxB_EQ_FIRST_BOOL      , GxB_ANY_FIRST_BOOL     ,
    GxB_LOR_SECOND_BOOL    , GxB_LAND_SECOND_BOOL   , GxB_LXOR_SECOND_BOOL   , GxB_EQ_SECOND_BOOL     , GxB_ANY_SECOND_BOOL    ,
    GxB_LOR_PAIR_BOOL/**/  , GxB_LAND_PAIR_BOOL/**/ , GxB_LXOR_PAIR_BOOL     , GxB_EQ_PAIR_BOOL/**/   , GxB_ANY_PAIR_BOOL      ,
    GxB_LOR_LOR_BOOL       , GxB_LAND_LOR_BOOL      , GxB_LXOR_LOR_BOOL      , GxB_EQ_LOR_BOOL        , GxB_ANY_LOR_BOOL       ,
    GxB_LOR_LAND_BOOL      , GxB_LAND_LAND_BOOL     , GxB_LXOR_LAND_BOOL     , GxB_EQ_LAND_BOOL       , GxB_ANY_LAND_BOOL      ,
    GxB_LOR_LXOR_BOOL      , GxB_LAND_LXOR_BOOL     , GxB_LXOR_LXOR_BOOL     , GxB_EQ_LXOR_BOOL       , GxB_ANY_LXOR_BOOL      ,
    GxB_LOR_EQ_BOOL        , GxB_LAND_EQ_BOOL       , GxB_LXOR_EQ_BOOL       , GxB_EQ_EQ_BOOL         , GxB_ANY_EQ_BOOL        ,
    GxB_LOR_GT_BOOL        , GxB_LAND_GT_BOOL       , GxB_LXOR_GT_BOOL       , GxB_EQ_GT_BOOL         , GxB_ANY_GT_BOOL        ,
    GxB_LOR_LT_BOOL        , GxB_LAND_LT_BOOL       , GxB_LXOR_LT_BOOL       , GxB_EQ_LT_BOOL         , GxB_ANY_LT_BOOL        ,
    GxB_LOR_GE_BOOL        , GxB_LAND_GE_BOOL       , GxB_LXOR_GE_BOOL       , GxB_EQ_GE_BOOL         , GxB_ANY_GE_BOOL        ,
    GxB_LOR_LE_BOOL        , GxB_LAND_LE_BOOL       , GxB_LXOR_LE_BOOL       , GxB_EQ_LE_BOOL         , GxB_ANY_LE_BOOL        ,

//------------------------------------------------------------------------------
// 54 complex semirings
//------------------------------------------------------------------------------

    // 3 monoids (plus, times, any), 2 types (FC32 and FC64), and 9
    // multiplicative operators.

    // Note that times_pair is identical to any_pair.
    // These 2 are marked below.

    GxB_PLUS_FIRST_FC32    , GxB_TIMES_FIRST_FC32   , GxB_ANY_FIRST_FC32     ,
    GxB_PLUS_FIRST_FC64    , GxB_TIMES_FIRST_FC64   , GxB_ANY_FIRST_FC64     ,

    GxB_PLUS_SECOND_FC32   , GxB_TIMES_SECOND_FC32  , GxB_ANY_SECOND_FC32    ,
    GxB_PLUS_SECOND_FC64   , GxB_TIMES_SECOND_FC64  , GxB_ANY_SECOND_FC64    ,

    GxB_PLUS_PAIR_FC32     , GxB_TIMES_PAIR_FC32/**/, GxB_ANY_PAIR_FC32      ,
    GxB_PLUS_PAIR_FC64     , GxB_TIMES_PAIR_FC64/**/, GxB_ANY_PAIR_FC64      ,

    GxB_PLUS_PLUS_FC32     , GxB_TIMES_PLUS_FC32    , GxB_ANY_PLUS_FC32      ,
    GxB_PLUS_PLUS_FC64     , GxB_TIMES_PLUS_FC64    , GxB_ANY_PLUS_FC64      ,

    GxB_PLUS_MINUS_FC32    , GxB_TIMES_MINUS_FC32   , GxB_ANY_MINUS_FC32     ,
    GxB_PLUS_MINUS_FC64    , GxB_TIMES_MINUS_FC64   , GxB_ANY_MINUS_FC64     ,

    GxB_PLUS_TIMES_FC32    , GxB_TIMES_TIMES_FC32   , GxB_ANY_TIMES_FC32     ,
    GxB_PLUS_TIMES_FC64    , GxB_TIMES_TIMES_FC64   , GxB_ANY_TIMES_FC64     ,

    GxB_PLUS_DIV_FC32      , GxB_TIMES_DIV_FC32     , GxB_ANY_DIV_FC32       ,
    GxB_PLUS_DIV_FC64      , GxB_TIMES_DIV_FC64     , GxB_ANY_DIV_FC64       ,

    GxB_PLUS_RDIV_FC32     , GxB_TIMES_RDIV_FC32    , GxB_ANY_RDIV_FC32      ,
    GxB_PLUS_RDIV_FC64     , GxB_TIMES_RDIV_FC64    , GxB_ANY_RDIV_FC64      ,

    GxB_PLUS_RMINUS_FC32   , GxB_TIMES_RMINUS_FC32  , GxB_ANY_RMINUS_FC32    ,
    GxB_PLUS_RMINUS_FC64   , GxB_TIMES_RMINUS_FC64  , GxB_ANY_RMINUS_FC64    ,

//------------------------------------------------------------------------------
// 64 bitwise semirings
//------------------------------------------------------------------------------

    // monoids: (BOR, BAND, BXOR, BXNOR) x 
    // mult:    (BOR, BAND, BXOR, BXNOR) x 
    // types:   (UINT8, UINT16, UINT32, UINT64)

    GxB_BOR_BOR_UINT8      , GxB_BOR_BOR_UINT16     , GxB_BOR_BOR_UINT32     , GxB_BOR_BOR_UINT64     ,
    GxB_BOR_BAND_UINT8     , GxB_BOR_BAND_UINT16    , GxB_BOR_BAND_UINT32    , GxB_BOR_BAND_UINT64    ,
    GxB_BOR_BXOR_UINT8     , GxB_BOR_BXOR_UINT16    , GxB_BOR_BXOR_UINT32    , GxB_BOR_BXOR_UINT64    ,
    GxB_BOR_BXNOR_UINT8    , GxB_BOR_BXNOR_UINT16   , GxB_BOR_BXNOR_UINT32   , GxB_BOR_BXNOR_UINT64   ,

    GxB_BAND_BOR_UINT8     , GxB_BAND_BOR_UINT16    , GxB_BAND_BOR_UINT32    , GxB_BAND_BOR_UINT64    ,
    GxB_BAND_BAND_UINT8    , GxB_BAND_BAND_UINT16   , GxB_BAND_BAND_UINT32   , GxB_BAND_BAND_UINT64   ,
    GxB_BAND_BXOR_UINT8    , GxB_BAND_BXOR_UINT16   , GxB_BAND_BXOR_UINT32   , GxB_BAND_BXOR_UINT64   ,
    GxB_BAND_BXNOR_UINT8   , GxB_BAND_BXNOR_UINT16  , GxB_BAND_BXNOR_UINT32  , GxB_BAND_BXNOR_UINT64  ,

    GxB_BXOR_BOR_UINT8     , GxB_BXOR_BOR_UINT16    , GxB_BXOR_BOR_UINT32    , GxB_BXOR_BOR_UINT64    ,
    GxB_BXOR_BAND_UINT8    , GxB_BXOR_BAND_UINT16   , GxB_BXOR_BAND_UINT32   , GxB_BXOR_BAND_UINT64   ,
    GxB_BXOR_BXOR_UINT8    , GxB_BXOR_BXOR_UINT16   , GxB_BXOR_BXOR_UINT32   , GxB_BXOR_BXOR_UINT64   ,
    GxB_BXOR_BXNOR_UINT8   , GxB_BXOR_BXNOR_UINT16  , GxB_BXOR_BXNOR_UINT32  , GxB_BXOR_BXNOR_UINT64  ,

    GxB_BXNOR_BOR_UINT8    , GxB_BXNOR_BOR_UINT16   , GxB_BXNOR_BOR_UINT32   , GxB_BXNOR_BOR_UINT64   ,
    GxB_BXNOR_BAND_UINT8   , GxB_BXNOR_BAND_UINT16  , GxB_BXNOR_BAND_UINT32  , GxB_BXNOR_BAND_UINT64  ,
    GxB_BXNOR_BXOR_UINT8   , GxB_BXNOR_BXOR_UINT16  , GxB_BXNOR_BXOR_UINT32  , GxB_BXNOR_BXOR_UINT64  ,
    GxB_BXNOR_BXNOR_UINT8  , GxB_BXNOR_BXNOR_UINT16 , GxB_BXNOR_BXNOR_UINT32 , GxB_BXNOR_BXNOR_UINT64 ,

//------------------------------------------------------------------------------
// 80 positional semirings
//------------------------------------------------------------------------------

    // monoids: (MIN, MAX, ANY, PLUS, TIMES) x
    // mult:    (FIRSTI, FIRSTI1, FIRSTJ, FIRSTJ1, SECONDI, SECONDI1, SECONDJ, SECONDJ1)
    // types:   (INT32, INT64)

    GxB_MIN_FIRSTI_INT32,     GxB_MIN_FIRSTI_INT64,
    GxB_MAX_FIRSTI_INT32,     GxB_MAX_FIRSTI_INT64,
    GxB_ANY_FIRSTI_INT32,     GxB_ANY_FIRSTI_INT64,
    GxB_PLUS_FIRSTI_INT32,    GxB_PLUS_FIRSTI_INT64,
    GxB_TIMES_FIRSTI_INT32,   GxB_TIMES_FIRSTI_INT64,

    GxB_MIN_FIRSTI1_INT32,    GxB_MIN_FIRSTI1_INT64,
    GxB_MAX_FIRSTI1_INT32,    GxB_MAX_FIRSTI1_INT64,
    GxB_ANY_FIRSTI1_INT32,    GxB_ANY_FIRSTI1_INT64,
    GxB_PLUS_FIRSTI1_INT32,   GxB_PLUS_FIRSTI1_INT64,
    GxB_TIMES_FIRSTI1_INT32,  GxB_TIMES_FIRSTI1_INT64,

    GxB_MIN_FIRSTJ_INT32,     GxB_MIN_FIRSTJ_INT64,
    GxB_MAX_FIRSTJ_INT32,     GxB_MAX_FIRSTJ_INT64,
    GxB_ANY_FIRSTJ_INT32,     GxB_ANY_FIRSTJ_INT64,
    GxB_PLUS_FIRSTJ_INT32,    GxB_PLUS_FIRSTJ_INT64,
    GxB_TIMES_FIRSTJ_INT32,   GxB_TIMES_FIRSTJ_INT64,

    GxB_MIN_FIRSTJ1_INT32,    GxB_MIN_FIRSTJ1_INT64,
    GxB_MAX_FIRSTJ1_INT32,    GxB_MAX_FIRSTJ1_INT64,
    GxB_ANY_FIRSTJ1_INT32,    GxB_ANY_FIRSTJ1_INT64,
    GxB_PLUS_FIRSTJ1_INT32,   GxB_PLUS_FIRSTJ1_INT64,
    GxB_TIMES_FIRSTJ1_INT32,  GxB_TIMES_FIRSTJ1_INT64,

    GxB_MIN_SECONDI_INT32,    GxB_MIN_SECONDI_INT64,
    GxB_MAX_SECONDI_INT32,    GxB_MAX_SECONDI_INT64,
    GxB_ANY_SECONDI_INT32,    GxB_ANY_SECONDI_INT64,
    GxB_PLUS_SECONDI_INT32,   GxB_PLUS_SECONDI_INT64,
    GxB_TIMES_SECONDI_INT32,  GxB_TIMES_SECONDI_INT64,

    GxB_MIN_SECONDI1_INT32,   GxB_MIN_SECONDI1_INT64,
    GxB_MAX_SECONDI1_INT32,   GxB_MAX_SECONDI1_INT64,
    GxB_ANY_SECONDI1_INT32,   GxB_ANY_SECONDI1_INT64,
    GxB_PLUS_SECONDI1_INT32,  GxB_PLUS_SECONDI1_INT64,
    GxB_TIMES_SECONDI1_INT32, GxB_TIMES_SECONDI1_INT64,

    GxB_MIN_SECONDJ_INT32,    GxB_MIN_SECONDJ_INT64,
    GxB_MAX_SECONDJ_INT32,    GxB_MAX_SECONDJ_INT64,
    GxB_ANY_SECONDJ_INT32,    GxB_ANY_SECONDJ_INT64,
    GxB_PLUS_SECONDJ_INT32,   GxB_PLUS_SECONDJ_INT64,
    GxB_TIMES_SECONDJ_INT32,  GxB_TIMES_SECONDJ_INT64,

    GxB_MIN_SECONDJ1_INT32,   GxB_MIN_SECONDJ1_INT64,
    GxB_MAX_SECONDJ1_INT32,   GxB_MAX_SECONDJ1_INT64,
    GxB_ANY_SECONDJ1_INT32,   GxB_ANY_SECONDJ1_INT64,
    GxB_PLUS_SECONDJ1_INT32,  GxB_PLUS_SECONDJ1_INT64,
    GxB_TIMES_SECONDJ1_INT32, GxB_TIMES_SECONDJ1_INT64 ;

//------------------------------------------------------------------------------
// GrB_* semirings
//------------------------------------------------------------------------------

// The v1.3 C API for GraphBLAS adds the following 124 predefined semirings,
// with GrB* names.  They are identical to 124 GxB* semirings defined above,
// with the same name, except that GrB_LXNOR_LOR_SEMIRING_BOOL is identical to
// GxB_EQ_LOR_BOOL (since GrB_EQ_BOOL == GrB_LXNOR).  The old names are listed
// below alongside each new name; the new GrB* names are preferred.

// 12 kinds of GrB* semirings are available for all 10 real, non-boolean types:

    // PLUS_TIMES, PLUS_MIN,
    // MIN_PLUS, MIN_TIMES, MIN_FIRST, MIN_SECOND, MIN_MAX,
    // MAX_PLUS, MAX_TIMES, MAX_FIRST, MAX_SECOND, MAX_MIN

// and 4 semirings for boolean only: 

    // LOR_LAND, LAND_LOR, LXOR_LAND, LXNOR_LOR.

// GxB* semirings corresponding to the equivalent GrB* semiring are deprecated.

GB_PUBLIC GrB_Semiring

    //--------------------------------------------------------------------------
    // 20 semirings with PLUS monoids
    //--------------------------------------------------------------------------

    // PLUS_TIMES semirings for all 10 real, non-boolean types:
    GrB_PLUS_TIMES_SEMIRING_INT8,       // GxB_PLUS_TIMES_INT8
    GrB_PLUS_TIMES_SEMIRING_INT16,      // GxB_PLUS_TIMES_INT16
    GrB_PLUS_TIMES_SEMIRING_INT32,      // GxB_PLUS_TIMES_INT32
    GrB_PLUS_TIMES_SEMIRING_INT64,      // GxB_PLUS_TIMES_INT64
    GrB_PLUS_TIMES_SEMIRING_UINT8,      // GxB_PLUS_TIMES_UINT8
    GrB_PLUS_TIMES_SEMIRING_UINT16,     // GxB_PLUS_TIMES_UINT16
    GrB_PLUS_TIMES_SEMIRING_UINT32,     // GxB_PLUS_TIMES_UINT32
    GrB_PLUS_TIMES_SEMIRING_UINT64,     // GxB_PLUS_TIMES_UINT64
    GrB_PLUS_TIMES_SEMIRING_FP32,       // GxB_PLUS_TIMES_FP32  
    GrB_PLUS_TIMES_SEMIRING_FP64,       // GxB_PLUS_TIMES_FP64  

    // PLUS_MIN semirings for all 10 real, non-boolean types:
    GrB_PLUS_MIN_SEMIRING_INT8,         // GxB_PLUS_MIN_INT8
    GrB_PLUS_MIN_SEMIRING_INT16,        // GxB_PLUS_MIN_INT16
    GrB_PLUS_MIN_SEMIRING_INT32,        // GxB_PLUS_MIN_INT32
    GrB_PLUS_MIN_SEMIRING_INT64,        // GxB_PLUS_MIN_INT64
    GrB_PLUS_MIN_SEMIRING_UINT8,        // GxB_PLUS_MIN_UINT8
    GrB_PLUS_MIN_SEMIRING_UINT16,       // GxB_PLUS_MIN_UINT16
    GrB_PLUS_MIN_SEMIRING_UINT32,       // GxB_PLUS_MIN_UINT32
    GrB_PLUS_MIN_SEMIRING_UINT64,       // GxB_PLUS_MIN_UINT64
    GrB_PLUS_MIN_SEMIRING_FP32,         // GxB_PLUS_MIN_FP32  
    GrB_PLUS_MIN_SEMIRING_FP64,         // GxB_PLUS_MIN_FP64  

    //--------------------------------------------------------------------------
    // 50 semirings with MIN monoids
    //--------------------------------------------------------------------------

    // MIN_PLUS semirings for all 10 real, non-boolean types:
    GrB_MIN_PLUS_SEMIRING_INT8,         // GxB_MIN_PLUS_INT8
    GrB_MIN_PLUS_SEMIRING_INT16,        // GxB_MIN_PLUS_INT16
    GrB_MIN_PLUS_SEMIRING_INT32,        // GxB_MIN_PLUS_INT32
    GrB_MIN_PLUS_SEMIRING_INT64,        // GxB_MIN_PLUS_INT64
    GrB_MIN_PLUS_SEMIRING_UINT8,        // GxB_MIN_PLUS_UINT8
    GrB_MIN_PLUS_SEMIRING_UINT16,       // GxB_MIN_PLUS_UINT16
    GrB_MIN_PLUS_SEMIRING_UINT32,       // GxB_MIN_PLUS_UINT32
    GrB_MIN_PLUS_SEMIRING_UINT64,       // GxB_MIN_PLUS_UINT64
    GrB_MIN_PLUS_SEMIRING_FP32,         // GxB_MIN_PLUS_FP32  
    GrB_MIN_PLUS_SEMIRING_FP64,         // GxB_MIN_PLUS_FP64  

    // MIN_TIMES semirings for all 10 real, non-boolean types:
    GrB_MIN_TIMES_SEMIRING_INT8,        // GxB_MIN_TIMES_INT8
    GrB_MIN_TIMES_SEMIRING_INT16,       // GxB_MIN_TIMES_INT16
    GrB_MIN_TIMES_SEMIRING_INT32,       // GxB_MIN_TIMES_INT32
    GrB_MIN_TIMES_SEMIRING_INT64,       // GxB_MIN_TIMES_INT64
    GrB_MIN_TIMES_SEMIRING_UINT8,       // GxB_MIN_TIMES_UINT8
    GrB_MIN_TIMES_SEMIRING_UINT16,      // GxB_MIN_TIMES_UINT16
    GrB_MIN_TIMES_SEMIRING_UINT32,      // GxB_MIN_TIMES_UINT32
    GrB_MIN_TIMES_SEMIRING_UINT64,      // GxB_MIN_TIMES_UINT64
    GrB_MIN_TIMES_SEMIRING_FP32,        // GxB_MIN_TIMES_FP32  
    GrB_MIN_TIMES_SEMIRING_FP64,        // GxB_MIN_PLUS_FP64  

    // MIN_FIRST semirings for all 10 real, non-boolean types:
    GrB_MIN_FIRST_SEMIRING_INT8,        // GxB_MIN_FIRST_INT8
    GrB_MIN_FIRST_SEMIRING_INT16,       // GxB_MIN_FIRST_INT16
    GrB_MIN_FIRST_SEMIRING_INT32,       // GxB_MIN_FIRST_INT32
    GrB_MIN_FIRST_SEMIRING_INT64,       // GxB_MIN_FIRST_INT64
    GrB_MIN_FIRST_SEMIRING_UINT8,       // GxB_MIN_FIRST_UINT8
    GrB_MIN_FIRST_SEMIRING_UINT16,      // GxB_MIN_FIRST_UINT16
    GrB_MIN_FIRST_SEMIRING_UINT32,      // GxB_MIN_FIRST_UINT32
    GrB_MIN_FIRST_SEMIRING_UINT64,      // GxB_MIN_FIRST_UINT64
    GrB_MIN_FIRST_SEMIRING_FP32,        // GxB_MIN_FIRST_FP32  
    GrB_MIN_FIRST_SEMIRING_FP64,        // GxB_MIN_FIRST_FP64  

    // MIN_SECOND semirings for all 10 real, non-boolean types:
    GrB_MIN_SECOND_SEMIRING_INT8,       // GxB_MIN_SECOND_INT8
    GrB_MIN_SECOND_SEMIRING_INT16,      // GxB_MIN_SECOND_INT16
    GrB_MIN_SECOND_SEMIRING_INT32,      // GxB_MIN_SECOND_INT32
    GrB_MIN_SECOND_SEMIRING_INT64,      // GxB_MIN_SECOND_INT64
    GrB_MIN_SECOND_SEMIRING_UINT8,      // GxB_MIN_SECOND_UINT8
    GrB_MIN_SECOND_SEMIRING_UINT16,     // GxB_MIN_SECOND_UINT16
    GrB_MIN_SECOND_SEMIRING_UINT32,     // GxB_MIN_SECOND_UINT32
    GrB_MIN_SECOND_SEMIRING_UINT64,     // GxB_MIN_SECOND_UINT64
    GrB_MIN_SECOND_SEMIRING_FP32,       // GxB_MIN_SECOND_FP32  
    GrB_MIN_SECOND_SEMIRING_FP64,       // GxB_MIN_SECOND_FP64  

    // MIN_MAX semirings for all 10 real, non-boolean types:
    GrB_MIN_MAX_SEMIRING_INT8,          // GxB_MIN_MAX_INT8
    GrB_MIN_MAX_SEMIRING_INT16,         // GxB_MIN_MAX_INT16
    GrB_MIN_MAX_SEMIRING_INT32,         // GxB_MIN_MAX_INT32
    GrB_MIN_MAX_SEMIRING_INT64,         // GxB_MIN_MAX_INT64
    GrB_MIN_MAX_SEMIRING_UINT8,         // GxB_MIN_MAX_UINT8
    GrB_MIN_MAX_SEMIRING_UINT16,        // GxB_MIN_MAX_UINT16
    GrB_MIN_MAX_SEMIRING_UINT32,        // GxB_MIN_MAX_UINT32
    GrB_MIN_MAX_SEMIRING_UINT64,        // GxB_MIN_MAX_UINT64
    GrB_MIN_MAX_SEMIRING_FP32,          // GxB_MIN_MAX_FP32  
    GrB_MIN_MAX_SEMIRING_FP64,          // GxB_MIN_MAX_FP64  

    //--------------------------------------------------------------------------
    // 50 semirings with MAX monoids
    //--------------------------------------------------------------------------

    // MAX_PLUS semirings for all 10 real, non-boolean types
    GrB_MAX_PLUS_SEMIRING_INT8,         // GxB_MAX_PLUS_INT8
    GrB_MAX_PLUS_SEMIRING_INT16,        // GxB_MAX_PLUS_INT16
    GrB_MAX_PLUS_SEMIRING_INT32,        // GxB_MAX_PLUS_INT32
    GrB_MAX_PLUS_SEMIRING_INT64,        // GxB_MAX_PLUS_INT64
    GrB_MAX_PLUS_SEMIRING_UINT8,        // GxB_MAX_PLUS_UINT8
    GrB_MAX_PLUS_SEMIRING_UINT16,       // GxB_MAX_PLUS_UINT16
    GrB_MAX_PLUS_SEMIRING_UINT32,       // GxB_MAX_PLUS_UINT32
    GrB_MAX_PLUS_SEMIRING_UINT64,       // GxB_MAX_PLUS_UINT64
    GrB_MAX_PLUS_SEMIRING_FP32,         // GxB_MAX_PLUS_FP32  
    GrB_MAX_PLUS_SEMIRING_FP64,         // GxB_MAX_PLUS_FP64  

    // MAX_TIMES semirings for all 10 real, non-boolean types:
    GrB_MAX_TIMES_SEMIRING_INT8,        // GxB_MAX_TIMES_INT8
    GrB_MAX_TIMES_SEMIRING_INT16,       // GxB_MAX_TIMES_INT16
    GrB_MAX_TIMES_SEMIRING_INT32,       // GxB_MAX_TIMES_INT32
    GrB_MAX_TIMES_SEMIRING_INT64,       // GxB_MAX_TIMES_INT64
    GrB_MAX_TIMES_SEMIRING_UINT8,       // GxB_MAX_TIMES_UINT8
    GrB_MAX_TIMES_SEMIRING_UINT16,      // GxB_MAX_TIMES_UINT16
    GrB_MAX_TIMES_SEMIRING_UINT32,      // GxB_MAX_TIMES_UINT32
    GrB_MAX_TIMES_SEMIRING_UINT64,      // GxB_MAX_TIMES_UINT64
    GrB_MAX_TIMES_SEMIRING_FP32,        // GxB_MAX_TIMES_FP32  
    GrB_MAX_TIMES_SEMIRING_FP64,        // GxB_MAX_TIMES_FP64  

    // MAX_FIRST semirings for all 10 real, non-boolean types:
    GrB_MAX_FIRST_SEMIRING_INT8,        // GxB_MAX_FIRST_INT8
    GrB_MAX_FIRST_SEMIRING_INT16,       // GxB_MAX_FIRST_INT16
    GrB_MAX_FIRST_SEMIRING_INT32,       // GxB_MAX_FIRST_INT32
    GrB_MAX_FIRST_SEMIRING_INT64,       // GxB_MAX_FIRST_INT64
    GrB_MAX_FIRST_SEMIRING_UINT8,       // GxB_MAX_FIRST_UINT8
    GrB_MAX_FIRST_SEMIRING_UINT16,      // GxB_MAX_FIRST_UINT16
    GrB_MAX_FIRST_SEMIRING_UINT32,      // GxB_MAX_FIRST_UINT32
    GrB_MAX_FIRST_SEMIRING_UINT64,      // GxB_MAX_FIRST_UINT64
    GrB_MAX_FIRST_SEMIRING_FP32,        // GxB_MAX_FIRST_FP32  
    GrB_MAX_FIRST_SEMIRING_FP64,        // GxB_MAX_FIRST_FP64  

    // MAX_SECOND semirings for all 10 real, non-boolean types:
    GrB_MAX_SECOND_SEMIRING_INT8,       // GxB_MAX_SECOND_INT8
    GrB_MAX_SECOND_SEMIRING_INT16,      // GxB_MAX_SECOND_INT16
    GrB_MAX_SECOND_SEMIRING_INT32,      // GxB_MAX_SECOND_INT32
    GrB_MAX_SECOND_SEMIRING_INT64,      // GxB_MAX_SECOND_INT64
    GrB_MAX_SECOND_SEMIRING_UINT8,      // GxB_MAX_SECOND_UINT8
    GrB_MAX_SECOND_SEMIRING_UINT16,     // GxB_MAX_SECOND_UINT16
    GrB_MAX_SECOND_SEMIRING_UINT32,     // GxB_MAX_SECOND_UINT32
    GrB_MAX_SECOND_SEMIRING_UINT64,     // GxB_MAX_SECOND_UINT64
    GrB_MAX_SECOND_SEMIRING_FP32,       // GxB_MAX_SECOND_FP32  
    GrB_MAX_SECOND_SEMIRING_FP64,       // GxB_MAX_SECOND_FP64  

    // MAX_MIN semirings for all 10 real, non-boolean types:
    GrB_MAX_MIN_SEMIRING_INT8,          // GxB_MAX_MIN_INT8
    GrB_MAX_MIN_SEMIRING_INT16,         // GxB_MAX_MIN_INT16
    GrB_MAX_MIN_SEMIRING_INT32,         // GxB_MAX_MIN_INT32
    GrB_MAX_MIN_SEMIRING_INT64,         // GxB_MAX_MIN_INT64
    GrB_MAX_MIN_SEMIRING_UINT8,         // GxB_MAX_MIN_UINT8
    GrB_MAX_MIN_SEMIRING_UINT16,        // GxB_MAX_MIN_UINT16
    GrB_MAX_MIN_SEMIRING_UINT32,        // GxB_MAX_MIN_UINT32
    GrB_MAX_MIN_SEMIRING_UINT64,        // GxB_MAX_MIN_UINT64
    GrB_MAX_MIN_SEMIRING_FP32,          // GxB_MAX_MIN_FP32  
    GrB_MAX_MIN_SEMIRING_FP64,          // GxB_MAX_MIN_FP64  

    //--------------------------------------------------------------------------
    // 4 boolean semirings:
    //--------------------------------------------------------------------------

    GrB_LOR_LAND_SEMIRING_BOOL,         // GxB_LOR_LAND_BOOL
    GrB_LAND_LOR_SEMIRING_BOOL,         // GxB_LAND_LOR_BOOL
    GrB_LXOR_LAND_SEMIRING_BOOL,        // GxB_LXOR_LAND_BOOL
    GrB_LXNOR_LOR_SEMIRING_BOOL ;       // GxB_EQ_LOR_BOOL (note EQ == LXNOR)

//==============================================================================
// GrB_*_resize:  change the size of a matrix or vector
//==============================================================================

// If the dimensions decrease, entries that fall outside the resized matrix or
// vector are deleted.

GB_PUBLIC
GrB_Info GrB_Matrix_resize      // change the size of a matrix
(
    GrB_Matrix C,               // matrix to modify
    GrB_Index nrows_new,        // new number of rows in matrix
    GrB_Index ncols_new         // new number of columns in matrix
) ;

GB_PUBLIC
GrB_Info GrB_Vector_resize      // change the size of a vector
(
    GrB_Vector w,               // vector to modify
    GrB_Index nrows_new         // new number of rows in vector
) ;

// GxB_*_resize are identical to the GrB*resize methods above
GB_PUBLIC
GrB_Info GxB_Matrix_resize      // change the size of a matrix (deprecated)
(
    GrB_Matrix C,               // matrix to modify
    GrB_Index nrows_new,        // new number of rows in matrix
    GrB_Index ncols_new         // new number of columns in matrix
) ;

GB_PUBLIC
GrB_Info GxB_Vector_resize      // change the size of a vector (deprecated)
(
    GrB_Vector w,               // vector to modify
    GrB_Index nrows_new         // new number of rows in vector
) ;

// GxB_resize is a generic function for resizing a matrix or vector:

// GrB_Vector_resize (u,nrows_new)
// GrB_Matrix_resize (A,nrows_new,ncols_new)

#if GxB_STDC_VERSION >= 201112L
#define GxB_resize(arg1,...)                                \
    _Generic                                                \
    (                                                       \
        (arg1),                                             \
              GrB_Vector : GrB_Vector_resize ,              \
              GrB_Matrix : GrB_Matrix_resize                \
    )                                                       \
    (arg1, __VA_ARGS__)
#endif


//------------------------------------------------------------------------------
// GxB_Matrix_Pending:  Checks to see if matrix has pending operations
//------------------------------------------------------------------------------

GrB_Info GxB_Matrix_Pending
(
	GrB_Matrix A,                   // matrix to query
	bool *pending                   // are there any pending operations
 ) ;

//------------------------------------------------------------------------------
// GxB_MatrixTupleIter:  Iterates over all none zero values of a matrix
//------------------------------------------------------------------------------

// TuplesIter maintains information required
// to iterate over a matrix
typedef struct
{
	GrB_Matrix A ;          // Matrix being iterated
	GrB_Index nvals ;       // Number of none zero values in matrix
	GrB_Index nnz_idx ;     // Index of current none zero value
	int64_t p ;             // Number of none zero values in current column
	int64_t row_idx ;       // Index of current row
	GrB_Index nrows ;       // Total number of rows in matrix
} GxB_MatrixTupleIter ;

// Create a new matrix iterator
GrB_Info GxB_MatrixTupleIter_new
(
	GxB_MatrixTupleIter **iter, // iterator to create
	GrB_Matrix A                // matrix being iterated
) ;

// Iterate over specific row
GrB_Info GxB_MatrixTupleIter_iterate_row
(
	GxB_MatrixTupleIter *iter,  // iterator to use
	GrB_Index rowIdx            // row index to iterate over
) ;

// Move iterator to a specific row
GrB_Info GxB_MatrixTupleIter_jump_to_row
(
	GxB_MatrixTupleIter *iter,  // iterator to use
	GrB_Index rowIdx            // row index to move iterator to
) ;

// Move iterator over specific rows range
GrB_Info GxB_MatrixTupleIter_iterate_range
(
	GxB_MatrixTupleIter *iter,  // iterator to use
	GrB_Index startRowIdx,      // row index to start with
	GrB_Index endRowIdx         // row index to finish with
) ;

// Advance iterator to the next none zero value
GrB_Info GxB_MatrixTupleIter_next
(
	GxB_MatrixTupleIter *iter,      // iterator to consume
	GrB_Index *row,                 // optional row index of current NNZ
	GrB_Index *col,                 // optional column index of current NNZ
	bool *depleted                  // indicate if iterator depleted
) ;

// Reset iterator
GrB_Info GxB_MatrixTupleIter_reset
(
	GxB_MatrixTupleIter *iter        // iterator to reset
) ;

// Reuse iterator to scan given matrix
GrB_Info GxB_MatrixTupleIter_reuse
(
	GxB_MatrixTupleIter *iter,  // iterator to update
	GrB_Matrix A				// matrix to scan
) ;

// Release every resource consumed by iterator
GrB_Info GxB_MatrixTupleIter_free
(
	GxB_MatrixTupleIter *iter   // iterator to free
) ;


//==============================================================================
// GxB_fprint and GxB_print: print the contents of a GraphBLAS object
//==============================================================================

// GxB_fprint (object, GxB_Print_Level pr, FILE *f) prints the contents of any
// of the 9 GraphBLAS objects to the file f, and also does an extensive test on
// the object to determine if it is valid.  It returns one of the following
// error conditions:
//
//      GrB_SUCCESS               object is valid
//      GrB_UNINITIALIZED_OBJECT  object is not initialized
//      GrB_INVALID_OBJECT        object is not valid
//      GrB_NULL_POINTER          object is a NULL pointer
//      GrB_INVALID_VALUE         fprintf returned an I/O error; see the ANSI C
//                                errno or GrB_error( )for details.
//
// GxB_fprint does not modify the status of any object.  If a matrix or vector
// has not been completed, the pending computations are guaranteed to *not* be
// performed by GxB_fprint.  The reason is simple.  It is possible for a bug in
// the user application (such as accessing memory outside the bounds of an
// array) to mangle the internal content of a GraphBLAS object, and GxB_fprint
// can be a helpful tool to track down this bug.  If GxB_fprint attempted to
// complete any computations prior to printing or checking the contents of the
// matrix or vector, then further errors could occur, including a segfault.
//
// The type-specific functions include an additional argument, the name string.
// The name is printed at the beginning of the display (assuming pr is not
// GxB_SILENT) so that the object can be more easily identified in the output.
// For the type-generic methods GxB_fprint and GxB_print, the name string is
// the variable name of the object itself.
//
// If f is NULL, nothing is printed (pr is effectively GxB_SILENT); this is not
// an error condition.  If pr is outside the bounds 0 to 3, negative values are
// treated as GxB_SILENT, and values > 3 are treated as GxB_COMPLETE.  If name
// is NULL, it is treated as the empty string.
//
// GxB_print (object, GxB_Print_Level pr) is the same as GxB_fprint, except
// that it prints the contents with printf instead of fprintf to a file f.
//
// The exact content and format of what is printed is implementation-dependent,
// and will change from version to version of SuiteSparse:GraphBLAS.  Do not
// attempt to rely on the exact content or format by trying to parse the
// resulting output via another program.  The intent of these functions is to
// produce a report of the object for visual inspection.

typedef enum
{
    GxB_SILENT = 0,     // nothing is printed, just check the object
    GxB_SUMMARY = 1,    // print a terse summary
    GxB_SHORT = 2,      // short description, about 30 entries of a matrix
    GxB_COMPLETE = 3,   // print the entire contents of the object
    GxB_SHORT_VERBOSE = 4,    // GxB_SHORT but with "%.15g" for doubles
    GxB_COMPLETE_VERBOSE = 5  // GxB_COMPLETE but with "%.15g" for doubles
}
GxB_Print_Level ;

GB_PUBLIC
GrB_Info GxB_Type_fprint            // print and check a GrB_Type
(
    GrB_Type type,                  // object to print and check
    const char *name,               // name of the object
    GxB_Print_Level pr,             // print level
    FILE *f                         // file for output
) ;

GB_PUBLIC
GrB_Info GxB_UnaryOp_fprint         // print and check a GrB_UnaryOp
(
    GrB_UnaryOp unaryop,            // object to print and check
    const char *name,               // name of the object
    GxB_Print_Level pr,             // print level
    FILE *f                         // file for output
) ;

GB_PUBLIC
GrB_Info GxB_BinaryOp_fprint        // print and check a GrB_BinaryOp
(
    GrB_BinaryOp binaryop,          // object to print and check
    const char *name,               // name of the object
    GxB_Print_Level pr,             // print level
    FILE *f                         // file for output
) ;

GB_PUBLIC
GrB_Info GxB_SelectOp_fprint        // print and check a GxB_SelectOp
(
    GxB_SelectOp selectop,          // object to print and check
    const char *name,               // name of the object
    GxB_Print_Level pr,             // print level
    FILE *f                         // file for output
) ;

GB_PUBLIC
GrB_Info GxB_Monoid_fprint          // print and check a GrB_Monoid
(
    GrB_Monoid monoid,              // object to print and check
    const char *name,               // name of the object
    GxB_Print_Level pr,             // print level
    FILE *f                         // file for output
) ;

GB_PUBLIC
GrB_Info GxB_Semiring_fprint        // print and check a GrB_Semiring
(
    GrB_Semiring semiring,          // object to print and check
    const char *name,               // name of the object
    GxB_Print_Level pr,             // print level
    FILE *f                         // file for output
) ;

GB_PUBLIC
GrB_Info GxB_Descriptor_fprint      // print and check a GrB_Descriptor
(
    GrB_Descriptor descriptor,      // object to print and check
    const char *name,               // name of the object
    GxB_Print_Level pr,             // print level
    FILE *f                         // file for output
) ;

GB_PUBLIC
GrB_Info GxB_Matrix_fprint          // print and check a GrB_Matrix
(
    GrB_Matrix A,                   // object to print and check
    const char *name,               // name of the object
    GxB_Print_Level pr,             // print level
    FILE *f                         // file for output
) ;

GB_PUBLIC
GrB_Info GxB_Vector_fprint          // print and check a GrB_Vector
(
    GrB_Vector v,                   // object to print and check
    const char *name,               // name of the object
    GxB_Print_Level pr,             // print level
    FILE *f                         // file for output
) ;

GB_PUBLIC
GrB_Info GxB_Scalar_fprint          // print and check a GxB_Scalar
(
    GxB_Scalar s,                   // object to print and check
    const char *name,               // name of the object
    GxB_Print_Level pr,             // print level
    FILE *f                         // file for output
) ;

#if GxB_STDC_VERSION >= 201112L
#define GxB_fprint(object,pr,f)                         \
    _Generic                                            \
    (                                                   \
        (object),                                       \
        const GrB_Type       : GxB_Type_fprint       ,  \
              GrB_Type       : GxB_Type_fprint       ,  \
        const GrB_UnaryOp    : GxB_UnaryOp_fprint    ,  \
              GrB_UnaryOp    : GxB_UnaryOp_fprint    ,  \
        const GrB_BinaryOp   : GxB_BinaryOp_fprint   ,  \
              GrB_BinaryOp   : GxB_BinaryOp_fprint   ,  \
        const GxB_SelectOp   : GxB_SelectOp_fprint   ,  \
              GxB_SelectOp   : GxB_SelectOp_fprint   ,  \
        const GrB_Monoid     : GxB_Monoid_fprint     ,  \
              GrB_Monoid     : GxB_Monoid_fprint     ,  \
        const GrB_Semiring   : GxB_Semiring_fprint   ,  \
              GrB_Semiring   : GxB_Semiring_fprint   ,  \
        const GxB_Scalar     : GxB_Scalar_fprint     ,  \
              GxB_Scalar     : GxB_Scalar_fprint     ,  \
        const GrB_Vector     : GxB_Vector_fprint     ,  \
              GrB_Vector     : GxB_Vector_fprint     ,  \
        const GrB_Matrix     : GxB_Matrix_fprint     ,  \
              GrB_Matrix     : GxB_Matrix_fprint     ,  \
        const GrB_Descriptor : GxB_Descriptor_fprint ,  \
              GrB_Descriptor : GxB_Descriptor_fprint    \
    )                                                   \
    (object, GB_STR(object), pr, f)
#endif

#if GxB_STDC_VERSION >= 201112L
#define GxB_print(object,pr) GxB_fprint(object,pr,NULL)
#endif

//==============================================================================
// GxB_import* and GxB_export*: Matrix and vector import/export
//==============================================================================

// The import/export functions allow the user application to create a
// GrB_Matrix or GrB_Vector object, and to extract its contents, faster and
// with less memory overhead than the GrB_*_build and GrB_*_extractTuples
// functions.

// The semantics of import/export are the same as the "move constructor" in
// C++.  On import, the user provides a set of arrays that have been previously
// allocated via the ANSI C malloc function.  The arrays define the content of
// the matrix or vector.  Unlike GrB_*_build, the GraphBLAS library then takes
// ownership of the user's input arrays and may either (a) incorporate them
// into its internal data structure for the new GrB_Matrix or GrB_Vector,
// potentially creating the GrB_Matrix or GrB_Vector in constant time with no
// memory copying performed, or (b) if the library does not support the import
// format directly, then it may convert the input to its internal format, and
// then free the user's input arrays.  GraphBLAS may also choose to use a mix
// of the two strategies.  In either case, the input arrays are no longer
// "owned" by the user application.  If A is a GrB_Matrix created by an import,
// the user input arrays are freed no later than GrB_free (&A), and may be
// freed earlier, at the discretion of the GraphBLAS library.  The data
// structure of the GrB_Matrix and GrB_Vector remain opaque.

// The export of a GrB_Matrix or GrB_Vector is symmetric with the import
// operation.  It is a destructive export, where the GrB_Matrix or GrB_Vector
// no longer exists when the export completes, and instead the user is returned
// several arrays that contain the matrix or vector in the requested format.
// Ownership of these arrays is given to the user application, which is then
// responsible for freeing them via the ANSI C free function.  If the output
// format is supported by the GraphBLAS library, then these arrays may be
// returned to the user application in O(1) time and with no memory copying
// performed.  Otherwise, the GraphBLAS library will create the output arrays
// for the user (via the ANSI C malloc function), fill them with the GrB_Matrix
// or GrB_Vector data, and then return the newly allocated arrays to the user.

// Eight different formats are provided for import/export.  For each format,
// the Ax array has a C-type <type> corresponding to one of the 13 built-in
// types in GraphBLAS (bool, int*_t, uint*_t, float, double, float complex, or
// double complex), or a user-defined type.

// On import, the required user arrays Ah, Ap, Ab, Ai, Aj, and/or Ax must be
// non-NULL pointers to memory space allocated by the ANSI C malloc (or calloc,
// or realloc), unless nzmax is zero (in which case the Ab, Ai, Aj, Ax, vb, vi,
// and vx arrays may all be NULL).  Just like GrB_*_new, the GrB_Matrix A (or
// GrB_Vector v) is undefined on input.  If the import is successful, the
// GrB_Matrix A or GrB_Vector v is created, and the pointers to the user input
// arrays have been set to NULL.  These user arrays have either been
// incorporated directly into the GrB_Matrix A or GrB_Vector v, in which case
// the user input arrays will eventually be freed by GrB_free (&A), or their
// contents have been copied and the arrays freed.  This decision is made by
// the GraphBLAS library itself, and the user application has no control over
// this decision.

// If any of the arrays Ab, Aj, Ai, Ax, vb, vi, or vx have zero size (with
// nzmax of zero), they are allowed to be be NULL pointers on input.

// No error checking is performed on the content of the user input arrays.  If
// the user input arrays do not conform to the precise specifications above,
// results are undefined.  No typecasting of the values of the matrix or vector
// entries is performed on import or export.

// SuiteSparse:GraphBLAS supports all eight formats natively (CSR, CSC,
// HyperCSR, and HyperCSC, BitmapR, BitmapC, FullR, FullC).  For vectors, only
// CSC, BitmapC, and FullC formats are used.  On import, the all eight formats
// take O(1) time and memory to import.  On export, if the GrB_Matrix or
// GrB_Vector is already in this particular format, then the export takes O(1)
// time and no memory copying is performed.

// If the import is not successful, the GxB_Matrix_import_* functions return A
// as NULL, GxB_Vector_import returns v as NULL, and the user input arrays are
// neither modified nor freed.  They are still owned by the user application.

//------------------------------------------------------------------------------
// GxB_Matrix_import_CSR: import a CSR matrix
//------------------------------------------------------------------------------

GB_PUBLIC
GrB_Info GxB_Matrix_import_CSR      // import a CSR matrix
(
    GrB_Matrix *A,      // handle of matrix to create
    GrB_Type type,      // type of matrix to create
    GrB_Index nrows,    // number of rows of the matrix
    GrB_Index ncols,    // number of columns of the matrix
    GrB_Index **Ap,     // row "pointers", Ap_size >= nrows+1
    GrB_Index **Aj,     // row indices, Aj_size >= nvals(A)
    void **Ax,          // values, Ax_size >= nvals(A)
    GrB_Index Ap_size,  // size of Ap
    GrB_Index Aj_size,  // size of Aj
    GrB_Index Ax_size,  // size of Ax
    bool jumbled,       // if true, indices in each row may be unsorted
    const GrB_Descriptor desc
) ;

    // CSR:  an nrows-by-ncols matrix with nvals entries in CSR format consists
    // of 3 arrays, where nvals = Ap [nrows]:
    //
    //          GrB_Index Ap [nrows+1], Aj [nvals] ; <type> Ax [nvals] ;
    //
    //      The column indices of entries in the ith row of the matrix are held
    //      in Aj [Ap [i] ... Ap[i+1]], and the corresponding values are held
    //      in the same positions in Ax.  Column indices must be in the range 0
    //      to ncols-1.  If jumbled is false, the column indices must appear in
    //      sorted order within each row.  No duplicate column indices may
    //      appear in any row.  Ap [0] must equal zero, and Ap [nrows] must
    //      equal nvals.  The Ap array must be of size nrows+1 (or larger), and
    //      the Aj and Ax arrays must have size at least nvals.  If nvals is
    //      zero, then the Aj and Ax arrays need not be present and can be
    //      NULL.

//------------------------------------------------------------------------------
// GxB_Matrix_import_CSC: import a CSC matrix
//------------------------------------------------------------------------------

GB_PUBLIC
GrB_Info GxB_Matrix_import_CSC      // import a CSC matrix
(
    GrB_Matrix *A,      // handle of matrix to create
    GrB_Type type,      // type of matrix to create
    GrB_Index nrows,    // number of rows of the matrix
    GrB_Index ncols,    // number of columns of the matrix
    GrB_Index **Ap,     // column "pointers", Ap_size >= ncols+1
    GrB_Index **Ai,     // row indices, Ai_size >= nvals(A)
    void **Ax,          // values, Ax_size >= nvals(A)
    GrB_Index Ap_size,  // size of Ap
    GrB_Index Ai_size,  // size of Ai
    GrB_Index Ax_size,  // size of Ax
    bool jumbled,       // if true, indices in each column may be unsorted
    const GrB_Descriptor desc
) ;

    // CSC:  an nrows-by-ncols matrix with nvals entries in CSC format consists
    // of 3 arrays, where nvals = Ap [ncols]:
    //
    //          GrB_Index Ap [ncols+1], Ai [nvals] ; <type> Ax [nvals] ;
    //
    //      The row indices of entries in the jth column of the matrix are held
    //      in Ai [Ap [j] ... Ap[j+1]], and the corresponding values are held
    //      in the same positions in Ax.  Row indices must be in the range 0 to
    //      nrows-1.  If jumbled is false, the row indices must appear in
    //      sorted order within each column.  No duplicate row indices may
    //      appear in any column.  Ap [0] must equal zero, and Ap [ncols] must
    //      equal nvals.  The Ap array must be of size ncols+1 (or larger), and
    //      the Ai and Ax arrays must have size at least nvals.  If nvals is
    //      zero, then the Ai and Ax arrays need not be present and can be
    //      NULL.

//------------------------------------------------------------------------------
// GxB_Matrix_import_HyperCSR: import a hypersparse CSR matrix
//------------------------------------------------------------------------------

GB_PUBLIC
GrB_Info GxB_Matrix_import_HyperCSR      // import a hypersparse CSR matrix
(
    GrB_Matrix *A,      // handle of matrix to create
    GrB_Type type,      // type of matrix to create
    GrB_Index nrows,    // number of rows of the matrix
    GrB_Index ncols,    // number of columns of the matrix
    GrB_Index **Ap,     // row "pointers", Ap_size >= nvec+1
    GrB_Index **Ah,     // row indices, Ah_size >= nvec
    GrB_Index **Aj,     // column indices, Aj_size >= nvals(A)
    void **Ax,          // values, Ax_size >= nvals(A)
    GrB_Index Ap_size,  // size of Ap
    GrB_Index Ah_size,  // size of Ah
    GrB_Index Aj_size,  // size of Aj
    GrB_Index Ax_size,  // size of Ax
    GrB_Index nvec,     // number of rows that appear in Ah
    bool jumbled,       // if true, indices in each row may be unsorted
    const GrB_Descriptor desc
) ;

    // HyperCSR: an nrows-by-ncols matrix with nvals entries and nvec
    // rows that may have entries in HyperCSR format consists of 4 arrays,
    // where nvals = Ap [nvec]:
    //
    //          GrB_Index Ah [nvec], Ap [nvec+1], Aj [nvals] ;
    //          <type> Ax [nvals] ;
    //
    //      The Aj and Ax arrays are the same for a matrix in CSR or HyperCSR
    //      format.  Only Ap and Ah differ.
    //
    //      The Ah array is a list of the row indices of rows that appear in
    //      the matrix.  It
    //      must appear in sorted order, and no duplicates may appear.  If i =
    //      Ah [k] is the kth row, then the column indices of the ith
    //      row appear in Aj [Ap [k] ... Ap [k+1]], and the corresponding
    //      values appear in the same locations in Ax.  Column indices must be
    //      in the range 0 to ncols-1, and must appear in sorted order within
    //      each row.  No duplicate column indices may appear in any row.  nvec
    //      may be zero, to denote an array with no entries.  The Ah array must
    //      be of size at least nvec, Ap must be of size at least nvec+1, and
    //      Aj and Ax must be at least of size nvals.  If nvals is zero, then
    //      the Aj and Ax arrays need not be present and can be NULL.

//------------------------------------------------------------------------------
// GxB_Matrix_import_HyperCSC: import a hypersparse CSC matrix
//------------------------------------------------------------------------------

GB_PUBLIC
GrB_Info GxB_Matrix_import_HyperCSC      // import a hypersparse CSC matrix
(
    GrB_Matrix *A,      // handle of matrix to create
    GrB_Type type,      // type of matrix to create
    GrB_Index nrows,    // number of rows of the matrix
    GrB_Index ncols,    // number of columns of the matrix
    GrB_Index **Ap,     // column "pointers", Ap_size >= nvec+1
    GrB_Index **Ah,     // column indices, Ah_size >= nvec
    GrB_Index **Ai,     // row indices, Ai_size >= nvals(A)
    void **Ax,          // values, Ax_size >= nvals(A)
    GrB_Index Ap_size,  // size of Ap
    GrB_Index Ah_size,  // size of Ah
    GrB_Index Ai_size,  // size of Ai
    GrB_Index Ax_size,  // size of Ax
    GrB_Index nvec,     // number of columns that appear in Ah
    bool jumbled,       // if true, indices in each column may be unsorted
    const GrB_Descriptor desc
) ;

    // HyperCSC: an nrows-by-ncols matrix with nvals entries and nvec
    // columns that may have entries in HyperCSC format consists of 4 arrays,
    // where nvals = Ap [nvec]:
    //
    //
    //          GrB_Index Ah [nvec], Ap [nvec+1], Ai [nvals] ;
    //          <type> Ax [nvals] ;
    //
    //      The Ai and Ax arrays are the same for a matrix in CSC or HyperCSC
    //      format.  Only Ap and Ah differ.
    //
    //      The Ah array is a list of the column indices of non-empty columns.
    //      It must appear in sorted order, and no duplicates may appear.  If j
    //      = Ah [k] is the kth non-empty column, then the row indices of the
    //      jth column appear in Ai [Ap [k] ... Ap [k+1]], and the
    //      corresponding values appear in the same locations in Ax.  Row
    //      indices must be in the range 0 to nrows-1, and must appear in
    //      sorted order within each column.  No duplicate row indices may
    //      appear in any column.  nvec may be zero, to denote an array with no
    //      entries.  The Ah array must be of size at least nvec, Ap must be of
    //      size at least nvec+1, and Ai and Ax must be at least of size nvals.
    //      If nvals is zero, then the Ai and Ax arrays need not be present and
    //      can be NULL.

//------------------------------------------------------------------------------
// GxB_Matrix_import_BitmapR: import a bitmap matrix, held by row
//------------------------------------------------------------------------------

GB_PUBLIC
GrB_Info GxB_Matrix_import_BitmapR  // import a bitmap matrix, held by row
(
    GrB_Matrix *A,      // handle of matrix to create
    GrB_Type type,      // type of matrix to create
    GrB_Index nrows,    // number of rows of the matrix
    GrB_Index ncols,    // number of columns of the matrix
    int8_t **Ab,        // bitmap, Ab_size >= nrows*ncols
    void **Ax,          // values, Ax_size >= nrows*ncols
    GrB_Index Ab_size,  // size of Ab
    GrB_Index Ax_size,  // size of Ax
    GrB_Index nvals,    // # of entries in bitmap
    const GrB_Descriptor desc
) ;

    // BitmapR: a dense format, but able to represent sparsity structure of A.
    //
    //          int8_t Ab [nrows*ncols] ;
    //          <type> Ax [nrows*ncols] ;
    //
    //      Ab and Ax are both of size nrows*ncols.  Ab [i*ncols+j] = 1 if the
    //      A(i,j) entry is present with value Ax [i*ncols+j], or 0 if A(i,j)
    //      is not present.  nvals must equal the number of 1's in the Ab
    //      array.

//------------------------------------------------------------------------------
// GxB_Matrix_import_BitmapC: import a bitmap matrix, held by column
//------------------------------------------------------------------------------

GB_PUBLIC
GrB_Info GxB_Matrix_import_BitmapC  // import a bitmap matrix, held by column
(
    GrB_Matrix *A,      // handle of matrix to create
    GrB_Type type,      // type of matrix to create
    GrB_Index nrows,    // number of rows of the matrix
    GrB_Index ncols,    // number of columns of the matrix
    int8_t **Ab,        // bitmap, Ab_size >= nrows*ncols
    void **Ax,          // values, Ax_size >= nrows*ncols
    GrB_Index Ab_size,  // size of Ab
    GrB_Index Ax_size,  // size of Ax
    GrB_Index nvals,    // # of entries in bitmap
    const GrB_Descriptor desc
) ;

    // BitmapC: a dense format, but able to represent sparsity structure of A.
    //
    //          int8_t Ab [nrows*ncols] ;
    //          <type> Ax [nrows*ncols] ;
    //
    //      Ab and Ax are both of size nrows*ncols.  Ab [i+j*nrows] = 1 if the
    //      A(i,j) entry is present with value Ax [i+j*nrows], or 0 if A(i,j)
    //      is not present.  nvals must equal the number of 1's in the Ab
    //      array.

//------------------------------------------------------------------------------
// GxB_Matrix_import_FullR:  import a full matrix, held by row
//------------------------------------------------------------------------------

GB_PUBLIC
GrB_Info GxB_Matrix_import_FullR  // import a full matrix, held by row
(
    GrB_Matrix *A,      // handle of matrix to create
    GrB_Type type,      // type of matrix to create
    GrB_Index nrows,    // number of rows of the matrix
    GrB_Index ncols,    // number of columns of the matrix
    void **Ax,          // values, Ax_size >= nrows*ncols
    GrB_Index Ax_size,  // size of Ax
    const GrB_Descriptor desc
) ;

    // FullR: an nrows-by-ncols full matrix held in row-major order:
    //
    //  <type> Ax [nrows*ncols] ;
    //
    //      Ax is an array of size nrows*ncols, where A(i,j) is held in
    //      Ax [i*ncols+j].  All entries in A are present.

//------------------------------------------------------------------------------
// GxB_Matrix_import_FullC: import a full matrix, held by column
//------------------------------------------------------------------------------

GB_PUBLIC
GrB_Info GxB_Matrix_import_FullC  // import a full matrix, held by column
(
    GrB_Matrix *A,      // handle of matrix to create
    GrB_Type type,      // type of matrix to create
    GrB_Index nrows,    // number of rows of the matrix
    GrB_Index ncols,    // number of columns of the matrix
    void **Ax,          // values, Ax_size >= nrows*ncols
    GrB_Index Ax_size,  // size of Ax
    const GrB_Descriptor desc
) ;

    // FullC: an nrows-by-ncols full matrix held in column-major order:
    //
    //  <type> Ax [nrows*ncols] ;
    //
    //      Ax is an array of size nrows*ncols, where A(i,j) is held in
    //      Ax [i+j*nrows].  All entries in A are present.

//------------------------------------------------------------------------------
// GxB_Vector_import_CSC: import a vector in CSC format
//------------------------------------------------------------------------------

GB_PUBLIC
GrB_Info GxB_Vector_import_CSC  // import a vector in CSC format
(
    GrB_Vector *v,      // handle of vector to create
    GrB_Type type,      // type of vector to create
    GrB_Index n,        // vector length
    GrB_Index **vi,     // indices, vi_size >= nvals(v)
    void **vx,          // values, vx_size >= nvals(v)
    GrB_Index vi_size,  // size of vi
    GrB_Index vx_size,  // size of vx
    GrB_Index nvals,    // # of entries in vector
    bool jumbled,       // if true, indices may be unsorted
    const GrB_Descriptor desc
) ;

    // The GrB_Vector is treated as if it was a single column of an n-by-1
    // matrix in CSC format, except that no vp array is required.  If nvals is
    // zero, then the vi and vx arrays need not be present and can be NULL.

//------------------------------------------------------------------------------
// GxB_Vector_import_Bitmap: import a vector in bitmap format
//------------------------------------------------------------------------------

GB_PUBLIC
GrB_Info GxB_Vector_import_Bitmap // import a bitmap vector
(
    GrB_Vector *v,      // handle of vector to create
    GrB_Type type,      // type of vector to create
    GrB_Index n,        // vector length
    int8_t **vb,        // bitmap, vb_size >= n
    void **vx,          // values, vx_size >= n
    GrB_Index vb_size,  // size of vb
    GrB_Index vx_size,  // size of vx
    GrB_Index nvals,    // # of entries in bitmap
    const GrB_Descriptor desc
) ;

    // The GrB_Vector is treated as if it was a single column of an n-by-1
    // matrix in BitmapC format.

//------------------------------------------------------------------------------
// GxB_Vector_import_Full: import a vector in full format
//------------------------------------------------------------------------------

GB_PUBLIC
GrB_Info GxB_Vector_import_Full // import a full vector
(
    GrB_Vector *v,      // handle of vector to create
    GrB_Type type,      // type of vector to create
    GrB_Index n,        // vector length
    void **vx,          // values, vx_size >= nvals(v)
    GrB_Index vx_size,  // size of vx
    const GrB_Descriptor desc
) ;

    // The GrB_Vector is treated as if it was a single column of an n-by-1
    // matrix in FullC format.

//------------------------------------------------------------------------------

// The GxB_*_export functions are symmetric with the GxB_*_import functions.
// GxB_*export* functions force completion of any pending operations, prior to
// the export, except if the only pending operation is to unjumble the matrix.
//
// If there are no entries in the matrix or vector, then the index arrays
// (Ai, Aj, or vi) and value arrays (Ax or vx) are returned as NULL.  This is
// not an error condition.
//
// GxB_Matrix_export:
//
//      A GrB_Matrix may be exported in any one of four different formats.  On
//      successful export, the input GrB_Matrix A is freed, and the output
//      arrays Ah, Ap, Ai, Aj, and/or Ax are returned to the user application
//      as arrays allocated by the ANSI C malloc function.  The four formats
//      are the same as the import formats for GrB_Matrix_import_*.
//
//      If jumbled is NULL on input, this indicates to GxB_*export* that the
//      exported matrix cannot be returned in a jumbled format.  In this case,
//      if the matrix is jumbled, it is sorted before exporting it to the
//      caller.

GB_PUBLIC
GrB_Info GxB_Matrix_export_CSR  // export and free a CSR matrix
(
    GrB_Matrix *A,      // handle of matrix to export and free
    GrB_Type *type,     // type of matrix exported
    GrB_Index *nrows,   // number of rows of the matrix
    GrB_Index *ncols,   // number of columns of the matrix
    GrB_Index **Ap,     // row "pointers", Ap_size >= nrows+1
    GrB_Index **Aj,     // row indices, Aj_size >= nvals(A)
    void **Ax,          // values, Ax_size >= nvals(A)
    GrB_Index *Ap_size, // size of Ap
    GrB_Index *Aj_size, // size of Aj
    GrB_Index *Ax_size, // size of Ax
    bool *jumbled,      // if true, indices in each row may be unsorted
    const GrB_Descriptor desc
) ;

GB_PUBLIC
GrB_Info GxB_Matrix_export_CSC  // export and free a CSC matrix
(
    GrB_Matrix *A,      // handle of matrix to export and free
    GrB_Type *type,     // type of matrix exported
    GrB_Index *nrows,   // number of rows of the matrix
    GrB_Index *ncols,   // number of columns of the matrix
    GrB_Index **Ap,     // column "pointers", Ap_size >= ncols+1
    GrB_Index **Ai,     // row indices, Ai_size >= nvals(A)
    void **Ax,          // values, Ax_size >= nvals(A)
    GrB_Index *Ap_size, // size of Ap
    GrB_Index *Ai_size, // size of Ai
    GrB_Index *Ax_size, // size of Ax
    bool *jumbled,      // if true, indices in each column may be unsorted
    const GrB_Descriptor desc
) ;

GB_PUBLIC
GrB_Info GxB_Matrix_export_HyperCSR  // export and free a hypersparse CSR matrix
(
    GrB_Matrix *A,      // handle of matrix to export and free
    GrB_Type *type,     // type of matrix exported
    GrB_Index *nrows,   // number of rows of the matrix
    GrB_Index *ncols,   // number of columns of the matrix
    GrB_Index **Ap,     // row "pointers", Ap_size >= nvec+1
    GrB_Index **Ah,     // row indices, Ah_size >= nvec
    GrB_Index **Aj,     // column indices, Aj_size >= nvals(A)
    void **Ax,          // values, Ax_size >= nvals(A)
    GrB_Index *Ap_size, // size of Ap
    GrB_Index *Ah_size, // size of Ah
    GrB_Index *Aj_size, // size of Aj
    GrB_Index *Ax_size, // size of Ax
    GrB_Index *nvec,    // number of rows that appear in Ah
    bool *jumbled,      // if true, indices in each row may be unsorted
    const GrB_Descriptor desc
) ;

GB_PUBLIC
GrB_Info GxB_Matrix_export_HyperCSC  // export and free a hypersparse CSC matrix
(
    GrB_Matrix *A,      // handle of matrix to export and free
    GrB_Type *type,     // type of matrix exported
    GrB_Index *nrows,   // number of rows of the matrix
    GrB_Index *ncols,   // number of columns of the matrix
    GrB_Index **Ap,     // column "pointers", Ap_size >= nvec+1
    GrB_Index **Ah,     // column indices, Ah_size >= nvec
    GrB_Index **Ai,     // row indices, Ai_size >= nvals(A)
    void **Ax,          // values, Ax_size >= nvals(A)
    GrB_Index *Ap_size, // size of Ap
    GrB_Index *Ah_size, // size of Ah
    GrB_Index *Ai_size, // size of Ai
    GrB_Index *Ax_size, // size of Ax
    GrB_Index *nvec,    // number of columns that appear in Ah
    bool *jumbled,      // if true, indices in each column may be unsorted
    const GrB_Descriptor desc
) ;

GB_PUBLIC
GrB_Info GxB_Matrix_export_BitmapR  // export and free a bitmap matrix, by row
(
    GrB_Matrix *A,      // handle of matrix to export and free
    GrB_Type *type,     // type of matrix exported
    GrB_Index *nrows,   // number of rows of the matrix
    GrB_Index *ncols,   // number of columns of the matrix
    int8_t **Ab,        // bitmap, Ab_size >= nrows*ncols
    void **Ax,          // values, Ax_size >= nrows*ncols
    GrB_Index *Ab_size, // size of Ab
    GrB_Index *Ax_size, // size of Ax
    GrB_Index *nvals,   // # of entries in bitmap
    const GrB_Descriptor desc
) ;

GB_PUBLIC
GrB_Info GxB_Matrix_export_BitmapC  // export and free a bitmap matrix, by col
(
    GrB_Matrix *A,      // handle of matrix to export and free
    GrB_Type *type,     // type of matrix exported
    GrB_Index *nrows,   // number of rows of the matrix
    GrB_Index *ncols,   // number of columns of the matrix
    int8_t **Ab,        // bitmap, Ab_size >= nrows*ncols
    void **Ax,          // values, Ax_size >= nrows*ncols
    GrB_Index *Ab_size, // size of Ab
    GrB_Index *Ax_size, // size of Ax
    GrB_Index *nvals,   // # of entries in bitmap
    const GrB_Descriptor desc
) ;

GB_PUBLIC
GrB_Info GxB_Matrix_export_FullR  // export and free a full matrix, by row
(
    GrB_Matrix *A,      // handle of matrix to export and free
    GrB_Type *type,     // type of matrix exported
    GrB_Index *nrows,   // number of rows of the matrix
    GrB_Index *ncols,   // number of columns of the matrix
    void **Ax,          // values, Ax_size >= nrows*ncols
    GrB_Index *Ax_size, // size of Ax
    const GrB_Descriptor desc
) ;

GB_PUBLIC
GrB_Info GxB_Matrix_export_FullC  // export and free a full matrix, by column
(
    GrB_Matrix *A,      // handle of matrix to export and free
    GrB_Type *type,     // type of matrix exported
    GrB_Index *nrows,   // number of rows of the matrix
    GrB_Index *ncols,   // number of columns of the matrix
    void **Ax,          // values, Ax_size >= nrows*ncols
    GrB_Index *Ax_size, // size of Ax
    const GrB_Descriptor desc
) ;

    // For GxB_Matrix_export_Full*, all entries in A must be present.  That is,
    // GrB_Matrix_nvals must report nvals equal to nrows*ncols.  If this
    // condition does not hold, the matrix is not exported, and
    // GrB_INVALID_VALUE is returned.

GB_PUBLIC
GrB_Info GxB_Vector_export_CSC  // export and free a CSC vector
(
    GrB_Vector *v,      // handle of vector to export and free
    GrB_Type *type,     // type of vector exported
    GrB_Index *n,       // length of the vector
    GrB_Index **vi,     // indices, vi_size >= nvals(v)
    void **vx,          // values, vx_size >= nvals(v)
    GrB_Index *vi_size, // size of vi
    GrB_Index *vx_size, // size of vx
    GrB_Index *nvals,   // # of entries in vector
    bool *jumbled,      // if true, indices may be unsorted
    const GrB_Descriptor desc
) ;

GB_PUBLIC
GrB_Info GxB_Vector_export_Bitmap   // export and free a bitmap vector
(
    GrB_Vector *v,      // handle of vector to export and free
    GrB_Type *type,     // type of vector exported
    GrB_Index *n,       // length of the vector
    int8_t **vb,        // bitmap, vb_size >= n
    void **vx,          // values, vx_size >= n
    GrB_Index *vb_size, // size of vb
    GrB_Index *vx_size, // size of vx
    GrB_Index *nvals,    // # of entries in bitmap
    const GrB_Descriptor desc
) ;

GB_PUBLIC
GrB_Info GxB_Vector_export_Full   // export and free a full vector
(
    GrB_Vector *v,      // handle of vector to export and free
    GrB_Type *type,     // type of vector exported
    GrB_Index *n,       // length of the vector
    void **vx,          // values, vx_size >= nvals(v)
    GrB_Index *vx_size, // size of vx
    const GrB_Descriptor desc
) ;

// If the export is not successful, the GxB_Matrix_export_* functions do not
// modify A, the GxB_Vector_export does not modify v, and the user arrays are
// returned as NULL.

//==============================================================================
// CUDA memory management (DRAFT: in progress, do not use)
//==============================================================================

// These functions are made available to the user application, since the
// GxB_import/export functions require the user application and the GraphBLAS
// library to rely on the same malloc/calloc/realloc/free functions.  If
// GraphBLAS is using CUDA Unified Memory Management and GxB_cuda_init is used
// to initialize GraphBLAS, then all of its memory allocations rely on these
// functions.

// If GraphBLAS is compiled with CUDA enabled, these functions map to
// cudaMallocManaged and cudaFree.  Otherwise, they map to the ANSI C malloc,
// calloc, and free functions.

// Note that there is no cudaReallocManaged function, and in this case
// GraphBLAS makes do without it.  As a result, the user application cannot use
// realloc either, for memory blocks passed to/from GraphBLAS via
// import/export.

void *GxB_cuda_malloc (size_t size) ;           // standard malloc signature
void *GxB_cuda_calloc (size_t n, size_t size) ; // standard calloc signature
void  GxB_cuda_free (void *p) ;                 // standard free signature

#endif

