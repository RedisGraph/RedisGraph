//------------------------------------------------------------------------------
// LAGraph.h:  include file for user applications that use LAGraph
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

// TODO: add more comments to this file.

#ifndef LAGRAPH_INCLUDE
#define LAGRAPH_INCLUDE

//------------------------------------------------------------------------------
// GraphBLAS.h and version control
//------------------------------------------------------------------------------

#include "GraphBLAS.h"

// the C API v1.3 now has GRB_VERSION and GRB_SUBVERSION.  If this macro
// is not defined, assume the spec is v1.2.
#ifndef GRB_VERSION
#define GRB_VERSION 1
#define GRB_SUBVERSION 2
#endif

// LAGRAPH_SPEC_VERSION: a single integer for comparing spec and version levels
// for the GraphBLAS C API.
#define LAGRAPH_SPEC_VERSION(major,minor,sub)       \
    (((major)*1000ULL + (minor))*1000ULL + (sub))

// LAGRAPH_SPEC: the current version of the GraphBLAS C API being used
#define LAGRAPH_SPEC LAGRAPH_SPEC_VERSION (GRB_VERSION, GRB_SUBVERSION, 0)

#if (LAGRAPH_SPEC >= LAGRAPH_SPEC_VERSION (1,3,0))
// GrB_error has changed in the v1.3 C API Specification of GraphBLAS
#define GRB_ERROR(string,object) GrB_error (&string, object)
#else
#define GRB_ERROR(string,object) (string = GrB_error ( ))
#endif

//------------------------------------------------------------------------------

#include <complex.h>
#include <ctype.h>

// "I" is defined by <complex.h>, but is used in LAGraph and GraphBLAS to
// denote a list of row indices; remove it here.
#undef I

#include <time.h>

#if defined ( __linux__ )
#include <sys/time.h>
#endif

#if defined ( _OPENMP )
#include <omp.h>
#endif

#if defined ( __MACH__ )
#include <mach/clock.h>
#include <mach/mach.h>
#endif

#if defined __INTEL_COMPILER
// disable icc warnings
//  161:  unrecognized pragma
#pragma warning (disable: 161)
#endif

#if !defined(__cplusplus)
#define LA_RESTRICT restrict
#elif defined(_MSC_BUILD) || defined(__clang__) || defined(__GNUC__) || defined(__INTEL_COMPILER)
#define LA_RESTRICT __restrict
#else
#define LA_RESTRICT
#endif

#define LAGRAPH_RAND_MAX 32767

// suitable for integers, and non-NaN floating point:
#define LAGRAPH_MAX(x,y) (((x) > (y)) ? (x) : (y))
#define LAGRAPH_MIN(x,y) (((x) < (y)) ? (x) : (y))

// free a block of memory and set the pointer to NULL
#define LAGRAPH_FREE(p)     \
{                           \
    LAGraph_free (p) ;      \
    p = NULL ;              \
}

//------------------------------------------------------------------------------
// memory management functions
//------------------------------------------------------------------------------

// use the ANSI C functions by default (or mx* functions if the #ifdef
// above redefines them).  See Source/Utility/LAGraph_malloc.c.

extern void *(* LAGraph_malloc_function)(size_t)         ;
extern void *(* LAGraph_calloc_function)(size_t, size_t) ;
extern void *(* LAGraph_realloc_function)(void *, size_t) ;
extern void (* LAGraph_free_function)(void *)         ;
extern bool LAGraph_malloc_is_thread_safe ;

//------------------------------------------------------------------------------
// LAGr wrappers: call GraphBLAS in a defined LAGraph context
//------------------------------------------------------------------------------

// Algebra Methods /////////////////////////////////////////////////////////////

#define LAGr_Type_new(...)                                                  \
{                                                                           \
    LAGRAPH_TRY_CATCH(GrB_Type_new(__VA_ARGS__));                           \
}

#define LAGr_UnaryOp_new(...)                                               \
{                                                                           \
    LAGRAPH_TRY_CATCH(GrB_UnaryOp_new(__VA_ARGS__));                        \
}

#define LAGr_BinaryOp_new(...)                                              \
{                                                                           \
    LAGRAPH_TRY_CATCH(GrB_BinaryOp_new(__VA_ARGS__));                       \
}

#define LAGr_SelectOp_new(...)                                              \
{                                                                           \
    LAGRAPH_TRY_CATCH(GxB_SelectOp_new(__VA_ARGS__));                       \
}

#define LAGr_Monoid_new(...)                                                \
{                                                                           \
    LAGRAPH_TRY_CATCH(GrB_Monoid_new(__VA_ARGS__));                         \
}

#define LAGr_Semiring_new(...)                                              \
{                                                                           \
    LAGRAPH_TRY_CATCH(GrB_Semiring_new(__VA_ARGS__));                       \
}

// Scalar Methods //////////////////////////////////////////////////////////////

#define LAGr_Scalar_new(...)                                                \
{                                                                           \
    LAGRAPH_TRY_CATCH (GxB_Scalar_new(__VA_ARGS__)) ;                       \
}

#define LAGr_Scalar_setElement(...)                                         \
{                                                                           \
    LAGRAPH_TRY_CATCH (GxB_Scalar_setElement(__VA_ARGS__)) ;                \
}

// Vector Methods //////////////////////////////////////////////////////////////

#define LAGr_Vector_new(...)                                                \
{                                                                           \
    LAGRAPH_TRY_CATCH(GrB_Vector_new(__VA_ARGS__));                         \
}

#define LAGr_Vector_dup(...)                                                \
{                                                                           \
    LAGRAPH_TRY_CATCH(GrB_Vector_dup(__VA_ARGS__));                         \
}

#define LAGr_Vector_resize(...)                                             \
{                                                                           \
    LAGRAPH_TRY_CATCH(GrB_Vector_resize(__VA_ARGS__));                      \
}

#define LAGr_Vector_clear(...)                                              \
{                                                                           \
    LAGRAPH_TRY_CATCH(GrB_Vector_clear(__VA_ARGS__));                       \
}

#define LAGr_Vector_size(...)                                               \
{                                                                           \
    LAGRAPH_TRY_CATCH(GrB_Vector_size(__VA_ARGS__));                        \
}

#define LAGr_Vector_nvals(...)                                              \
{                                                                           \
    LAGRAPH_TRY_CATCH(GrB_Vector_nvals(__VA_ARGS__));                       \
}

#define LAGr_Vector_build(...)                                              \
{                                                                           \
    LAGRAPH_TRY_CATCH(GrB_Vector_build(__VA_ARGS__));                       \
}

#define LAGr_Vector_setElement(...)                                         \
{                                                                           \
    LAGRAPH_TRY_CATCH(GrB_Vector_setElement(__VA_ARGS__));                  \
}

#define LAGr_Vector_removeElement(...)                                      \
{                                                                           \
    LAGRAPH_TRY_CATCH(GrB_Vector_removeElement(__VA_ARGS__));               \
}

#define LAGr_Vector_extractElement(...)                                     \
{                                                                           \
    LAGRAPH_TRY_CATCH(GrB_Vector_extractElement(__VA_ARGS__));              \
}

#define LAGr_Vector_extractTuples(...)                                      \
{                                                                           \
    LAGRAPH_TRY_CATCH(GrB_Vector_extractTuples(__VA_ARGS__));               \
}

#define LAGr_Vector_import(...)                                             \
{                                                                           \
    LAGRAPH_TRY_CATCH(GxB_Vector_import (__VA_ARGS__)) ;                    \
}

#define LAGr_Vector_export(...)                                             \
{                                                                           \
    LAGRAPH_TRY_CATCH(GxB_Vector_export (__VA_ARGS__)) ;                    \
}

// Matrix Methods //////////////////////////////////////////////////////////////

#define LAGr_Matrix_new(...)                                                \
{                                                                           \
    LAGRAPH_TRY_CATCH(GrB_Matrix_new(__VA_ARGS__));                         \
}

#define LAGr_Matrix_type(...)                                               \
{                                                                           \
    LAGRAPH_TRY_CATCH(GxB_Matrix_type(__VA_ARGS__));                        \
}

#define LAGr_Matrix_dup(...)                                                \
{                                                                           \
    LAGRAPH_TRY_CATCH(GrB_Matrix_dup(__VA_ARGS__));                         \
}

#define LAGr_Matrix_resize(...)                                             \
{                                                                           \
    LAGRAPH_TRY_CATCH(GrB_Matrix_resize(__VA_ARGS__));                      \
}

#define LAGr_Matrix_clear(...)                                              \
{                                                                           \
    LAGRAPH_TRY_CATCH(GrB_Matrix_clear(__VA_ARGS__));                       \
}

#define LAGr_Matrix_nrows(...)                                              \
{                                                                           \
    LAGRAPH_TRY_CATCH(GrB_Matrix_nrows(__VA_ARGS__));                       \
}

#define LAGr_Matrix_ncols(...)                                              \
{                                                                           \
    LAGRAPH_TRY_CATCH(GrB_Matrix_ncols(__VA_ARGS__));                       \
}

#define LAGr_Matrix_nvals(...)                                              \
{                                                                           \
    LAGRAPH_TRY_CATCH(GrB_Matrix_nvals(__VA_ARGS__));                       \
}

#define LAGr_Matrix_build(...)                                              \
{                                                                           \
    LAGRAPH_TRY_CATCH(GrB_Matrix_build(__VA_ARGS__));                       \
}

#define LAGr_Matrix_setElement(...)                                         \
{                                                                           \
    LAGRAPH_TRY_CATCH(GrB_Matrix_setElement(__VA_ARGS__));                  \
}

#define LAGr_Matrix_removeElement(...)                                      \
{                                                                           \
    LAGRAPH_TRY_CATCH(GrB_Matrix_removeElement(__VA_ARGS__));               \
}

#define LAGr_Matrix_extractElement(...)                                     \
{                                                                           \
    LAGRAPH_TRY_CATCH(GrB_Matrix_extractElement(__VA_ARGS__));              \
}

#define LAGr_Matrix_extractTuples(...)                                      \
{                                                                           \
    LAGRAPH_TRY_CATCH(GrB_Matrix_extractTuples(__VA_ARGS__));               \
}

// Descriptor Methods //////////////////////////////////////////////////////////

#define LAGr_Descriptor_new(...)                                            \
{                                                                           \
    LAGRAPH_TRY_CATCH(GrB_Descriptor_new(__VA_ARGS__));                     \
}

#define LAGr_Descriptor_set(...)                                            \
{                                                                           \
    LAGRAPH_TRY_CATCH(GrB_Descriptor_set(__VA_ARGS__));                     \
}

#define LAGr_get(...)                                                       \
{                                                                           \
    LAGRAPH_TRY_CATCH(GxB_get(__VA_ARGS__));                                \
}

#define LAGr_set(...)                                                       \
{                                                                           \
    LAGRAPH_TRY_CATCH(GxB_set(__VA_ARGS__));                                \
}

// Free Method /////////////////////////////////////////////////////////////////

// TODO: For now, LAGr_free is simply a wrapper for GrB_free with no error
//       handling. In the future, there may be more happening here.
#define LAGr_free(...)                                                      \
{                                                                           \
    GrB_free(__VA_ARGS__);                                                  \
}

// GraphBLAS Operations ////////////////////////////////////////////////////////

#define LAGr_mxm(...)                                                       \
{                                                                           \
    LAGRAPH_TRY_CATCH(GrB_mxm(__VA_ARGS__));                                \
}

#define LAGr_vxm(...)                                                       \
{                                                                           \
    LAGRAPH_TRY_CATCH(GrB_vxm(__VA_ARGS__));                                \
}

#define LAGr_mxv(...)                                                       \
{                                                                           \
    LAGRAPH_TRY_CATCH(GrB_mxv(__VA_ARGS__));                                \
}

#define LAGr_eWiseMult(...)                                                 \
{                                                                           \
    LAGRAPH_TRY_CATCH(GrB_eWiseMult(__VA_ARGS__));                          \
}

#define LAGr_eWiseAdd(...)                                                  \
{                                                                           \
    LAGRAPH_TRY_CATCH(GrB_eWiseAdd(__VA_ARGS__));                           \
}

#define LAGr_extract(...)                                                   \
{                                                                           \
    LAGRAPH_TRY_CATCH(GrB_extract(__VA_ARGS__));                            \
}

#define LAGr_assign(...)                                                    \
{                                                                           \
    LAGRAPH_TRY_CATCH(GrB_assign(__VA_ARGS__));                             \
}

#define LAGr_apply(...)                                                     \
{                                                                           \
    LAGRAPH_TRY_CATCH(GrB_apply(__VA_ARGS__));                              \
}

#define LAGr_reduce(...)                                                    \
{                                                                           \
    LAGRAPH_TRY_CATCH(GrB_reduce(__VA_ARGS__));                             \
}

#define LAGr_transpose(...)                                                 \
{                                                                           \
    LAGRAPH_TRY_CATCH(GrB_transpose(__VA_ARGS__));                          \
}

#define LAGr_kronecker(...)                                                 \
{                                                                           \
    LAGRAPH_TRY_CATCH(GrB_kronecker(__VA_ARGS__));                          \
}

#define LAGr_select(...)                                                    \
{                                                                           \
    LAGRAPH_TRY_CATCH (GxB_select (__VA_ARGS__)) ;                          \
}

// Sequence Termination ////////////////////////////////////////////////////////

#define LAGr_wait(...)                                                      \
{                                                                           \
    LAGRAPH_TRY_CATCH(GrB_wait(__VA_ARGS__));                               \
}

#define LAGr_UnaryOp_free(...)                                              \
{                                                                           \
    LAGRAPH_TRY_CATCH(GrB_UnaryOp_free(__VA_ARGS__));                       \
}

#define LAGRAPH_TRY_CATCH(method)                                           \
{                                                                           \
    GrB_Info info = (method) ;                                              \
    if (!(info == GrB_SUCCESS || info == GrB_NO_VALUE))                     \
    {                                                                       \
        LAGRAPH_ERROR ("", info) ;                                          \
    }                                                                       \
}

//------------------------------------------------------------------------------
// LAGraph methods
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// LAGRAPH_OK: call LAGraph or GraphBLAS and check the result
//------------------------------------------------------------------------------

// To use LAGRAPH_OK, the #include'ing file must declare a scalar GrB_Info
// info, and must define LAGRAPH_FREE_ALL as a macro that frees all workspace
// if an error occurs.  The method can be a GrB_Info scalar as well, so that
// LAGRAPH_OK(info) works.  The function that uses this macro must return
// GrB_Info, or int.

#define LAGRAPH_ERROR(message,info)                                         \
{                                                                           \
    fprintf (stderr, "LAGraph error: %s\n[%d]\nFile: %s Line: %d\n",        \
        message, info, __FILE__, __LINE__) ;                                \
    LAGRAPH_FREE_ALL ;                                                      \
    return (info) ;                                                         \
}

#define LAGRAPH_OK(method)                                                  \
{                                                                           \
    info = method ;                                                         \
    if (! (info == GrB_SUCCESS || info == GrB_NO_VALUE))                    \
    {                                                                       \
        LAGRAPH_ERROR ("", info) ;                                          \
    }                                                                       \
}

GrB_Info LAGraph_log
(
	char *caller,           // calling function
	char *message1,         // message to include (may be NULL)
	char *message2,         // message to include (may be NULL)
	int nthreads,           // # of threads used
	double t                // time taken by the test
) ;

// LAGr_log (message1, message2, nthreads, time)
#define LAGr_log(...)                                                       \
{                                                                           \
    LAGRAPH_TRY_CATCH (LAGraph_log (__FILE__, __VA_ARGS__)) ;               \
}

//------------------------------------------------------------------------------
// LAGraph_Context:
//------------------------------------------------------------------------------

// All LAGraph functions will use a Context for global parameters, error
// status, and the like.  So far, the parameter is only for LAGraph_random.

typedef struct {
	int nthreads ;          // # of threads to use.  If <= 0, use defaults
	// (from omp_get_max_threads)
}
LAGraph_Context ;

//------------------------------------------------------------------------------
// global objects
//------------------------------------------------------------------------------

// LAGraph_ComplexFP64 operators
//------------------------------------------------------------------------------
// 10 binary functions, z=f(x,y), where CxC -> C
//------------------------------------------------------------------------------

extern
GrB_BinaryOp
LAGraph_FIRST_ComplexFP64,
LAGraph_SECOND_ComplexFP64,
LAGraph_MIN_ComplexFP64,
LAGraph_MAX_ComplexFP64,
LAGraph_PLUS_ComplexFP64,
LAGraph_MINUS_ComplexFP64,
LAGraph_TIMES_ComplexFP64,
LAGraph_DIV_ComplexFP64,
LAGraph_RDIV_ComplexFP64,
LAGraph_RMINUS_ComplexFP64,
LAGraph_SKEW_ComplexFP64,
LAGraph_PAIR_ComplexFP64,
LAGraph_ANY_ComplexFP64,
LAGraph_HERMITIAN_ComplexFP64       ;

//------------------------------------------------------------------------------
// 6 binary comparison functions, z = f(x,y), where CxC -> C
//------------------------------------------------------------------------------

extern
GrB_BinaryOp
LAGraph_ISEQ_ComplexFP64,
LAGraph_ISNE_ComplexFP64,
LAGraph_ISGT_ComplexFP64,
LAGraph_ISLT_ComplexFP64,
LAGraph_ISGE_ComplexFP64,
LAGraph_ISLE_ComplexFP64              ;

//------------------------------------------------------------------------------
// 3 binary boolean functions, z=f(x,y), where CxC -> C
//------------------------------------------------------------------------------

extern
GrB_BinaryOp
LAGraph_OR_ComplexFP64,
LAGraph_AND_ComplexFP64,
LAGraph_XOR_ComplexFP64               ;

//------------------------------------------------------------------------------
// 6 binary comparison functions, z=f(x,y), where CxC -> bool
//------------------------------------------------------------------------------

extern
GrB_BinaryOp
LAGraph_EQ_ComplexFP64,
LAGraph_NE_ComplexFP64,
LAGraph_GT_ComplexFP64,
LAGraph_LT_ComplexFP64,
LAGraph_GE_ComplexFP64,
LAGraph_LE_ComplexFP64                ;

//------------------------------------------------------------------------------
// 1 binary function, z=f(x,y), where double x double -> C
//------------------------------------------------------------------------------

extern GrB_BinaryOp LAGraph_COMPLEX_ComplexFP64 ;

//------------------------------------------------------------------------------
// 5 unary functions, z=f(x) where C -> C
//------------------------------------------------------------------------------

extern
GrB_UnaryOp
LAGraph_IDENTITY_ComplexFP64,
LAGraph_AINV_ComplexFP64,
LAGraph_MINV_ComplexFP64,
LAGraph_NOT_ComplexFP64,
LAGraph_CONJ_ComplexFP64,
LAGraph_ONE_ComplexFP64,
LAGraph_ISONE_ComplexFP64,
LAGraph_ABS_ComplexFP64,
LAGraph_TRUE_BOOL_ComplexFP64         ;

//------------------------------------------------------------------------------
// 4 unary functions, z=f(x) where C -> double
//------------------------------------------------------------------------------

extern
GrB_UnaryOp
LAGraph_REAL_ComplexFP64,
LAGraph_IMAG_ComplexFP64,
LAGraph_CABS_ComplexFP64,
LAGraph_ANGLE_ComplexFP64             ;

//------------------------------------------------------------------------------
// 2 unary functions, z=f(x) where double -> C
//------------------------------------------------------------------------------

extern GrB_UnaryOp
LAGraph_COMPLEX_REAL_ComplexFP64,
LAGraph_COMPLEX_IMAG_ComplexFP64      ;

//------------------------------------------------------------------------------
// Complex type, scalars, monoids, and semiring
//------------------------------------------------------------------------------

extern GrB_Type LAGraph_ComplexFP64 ;

extern GrB_Monoid
LAGraph_PLUS_ComplexFP64_MONOID,
LAGraph_TIMES_ComplexFP64_MONOID      ;

extern GrB_Semiring LAGraph_PLUS_TIMES_ComplexFP64 ;

extern double _Complex LAGraph_ComplexFP64_1 ;
extern double _Complex LAGraph_ComplexFP64_0 ;

GrB_Info LAGraph_Complex_init() ;
GrB_Info LAGraph_Complex_finalize() ;

extern GrB_BinaryOp

// binary operators to test for symmetry, skew-symmetry
// and Hermitian property
LAGraph_SKEW_INT8,
LAGraph_SKEW_INT16,
LAGraph_SKEW_INT32,
LAGraph_SKEW_INT64,
LAGraph_SKEW_FP32,
LAGraph_SKEW_FP64,
LAGraph_LOR_UINT32,
LAGraph_LOR_INT64           ;

extern GrB_UnaryOp

// unary operators to check if the entry is equal to 1
LAGraph_ISONE_INT8,
LAGraph_ISONE_INT16,
LAGraph_ISONE_INT32,
LAGraph_ISONE_INT64,
LAGraph_ISONE_UINT8,
LAGraph_ISONE_UINT16,
LAGraph_ISONE_UINT32,
LAGraph_ISONE_UINT64,
LAGraph_ISONE_FP32,
LAGraph_ISONE_FP64,

// unary operators to check if the entry is equal to 2
LAGraph_ISTWO_UINT32,
LAGraph_ISTWO_INT64,

// unary operators that decrement by 1
LAGraph_DECR_INT32,
LAGraph_DECR_INT64,

// unary operators for lcc
LAGraph_COMB_DIR_FP64,
LAGraph_COMB_UNDIR_FP64,

// unary ops to check if greater than zero
LAGraph_GT0_FP32,
LAGraph_GT0_FP64,

// unary YMAX ops for DNN
LAGraph_YMAX_FP32,
LAGraph_YMAX_FP64,

// unary operators that return 1
LAGraph_ONE_UINT32,
LAGraph_ONE_INT64,
LAGraph_ONE_FP64,
LAGraph_TRUE_BOOL           ;

// monoids and semirings
extern GrB_Monoid

LAGraph_PLUS_INT64_MONOID,
LAGraph_MAX_INT32_MONOID,
LAGraph_LAND_MONOID,
LAGraph_LOR_MONOID,
LAGraph_MIN_INT32_MONOID,
LAGraph_MIN_INT64_MONOID,
LAGraph_PLUS_UINT32_MONOID,
LAGraph_PLUS_FP32_MONOID,
LAGraph_PLUS_FP64_MONOID,
LAGraph_DIV_FP64_MONOID     ;

extern GrB_Semiring

LAGraph_LOR_LAND_BOOL,
LAGraph_LOR_SECOND_BOOL,
LAGraph_LOR_FIRST_BOOL,
LAGraph_MIN_SECOND_INT32,
LAGraph_MIN_FIRST_INT32,
LAGraph_MIN_SECOND_INT64,
LAGraph_MIN_FIRST_INT64,
LAGraph_PLUS_TIMES_UINT32,
LAGraph_PLUS_TIMES_INT64,
LAGraph_PLUS_TIMES_FP64,
LAGraph_PLUS_PLUS_FP64,
LAGraph_PLUS_TIMES_FP32,
LAGraph_PLUS_PLUS_FP32      ;

// all 16 descriptors
// syntax: 4 characters define the following.  'o' is the default:
// 1: o or t: A transpose
// 2: o or t: B transpose
// 3: o or c: complemented mask
// 4: o or r: replace
extern GrB_Descriptor

LAGraph_desc_oooo,    // default (NULL)
LAGraph_desc_ooor,    // replace
LAGraph_desc_ooco,    // compl mask
LAGraph_desc_oocr,    // compl mask, replace

LAGraph_desc_tooo,    // A'
LAGraph_desc_toor,    // A', replace
LAGraph_desc_toco,    // A', compl mask
LAGraph_desc_tocr,    // A', compl mask, replace

LAGraph_desc_otoo,    // B'
LAGraph_desc_otor,    // B', replace
LAGraph_desc_otco,    // B', compl mask
LAGraph_desc_otcr,    // B', compl mask, replace

LAGraph_desc_ttoo,    // A', B'
LAGraph_desc_ttor,    // A', B', replace
LAGraph_desc_ttco,    // A', B', compl mask
LAGraph_desc_ttcr ;   // A', B', compl mask, replace

#if defined ( GxB_SUITESPARSE_GRAPHBLAS ) \
    && GxB_IMPLEMENTATION >= GxB_VERSION (3,0,1)
// requires SuiteSparse:GraphBLAS v3.0.1 or later
extern GxB_SelectOp LAGraph_support ;
#endif

//------------------------------------------------------------------------------
// user-callable utility functions
//------------------------------------------------------------------------------

typedef void (*LAGraph_binary_function)(void *, const void *, const void *) ;

GrB_Info LAGraph_init() ;           // start LAGraph

GrB_Info LAGraph_xinit              // start LAGraph (alternative method)
(
	// pointers to memory management functions
	void *(* user_malloc_function)(size_t),
	void *(* user_calloc_function)(size_t, size_t),
	void *(* user_realloc_function)(void *, size_t),
	void (* user_free_function)(void *),
	bool user_malloc_is_thread_safe
) ;

GrB_Info LAGraph_finalize() ;       // end LAGraph

GrB_Info LAGraph_mmread
(
	GrB_Matrix *A,      // handle of matrix to create
	FILE *f             // file to read from, already open
) ;

GrB_Info LAGraph_mmwrite
(
	GrB_Matrix A,           // matrix to write to the file
	FILE *f                 // file to write it to
	// TODO , FILE *fcomments         // optional file with extra comments
) ;

// ascii header prepended to all *.grb files
#define LAGRAPH_BIN_HEADER 512

GrB_Info LAGraph_binwrite
(
	GrB_Matrix *A,          // matrix to write to the file
	char *filename,         // file to write it to
	const char *comments    // comments to add to the file, up to 220 characters
	// in length, not including the terminating null
	// byte. Ignored if NULL.  Characters past
	// the 220 limit are silently ignored.
) ;

GrB_Info LAGraph_binread
(
	GrB_Matrix *A,          // matrix to read from the file
	char *filename          // file to read it from
) ;

GrB_Info LAGraph_tsvread        // returns GrB_SUCCESS if successful
(
	GrB_Matrix *Chandle,        // C, created on output
	FILE *f,                    // file to read from (already open)
	GrB_Type type,              // the type of C to create
	GrB_Index nrows,            // C is nrows-by-ncols
	GrB_Index ncols
) ;

GrB_Info LAGraph_ispattern  // return GrB_SUCCESS if successful
(
	bool *result,           // true if A is all one, false otherwise
	GrB_Matrix A,
	GrB_UnaryOp userop      // for A with arbitrary user-defined type.
	// Ignored if A and B are of built-in types or
	// LAGraph_ComplexFP64.
) ;

GrB_Info LAGraph_pattern    // return GrB_SUCCESS if successful
(
	GrB_Matrix *C,          // a boolean matrix with the pattern of A
	GrB_Matrix A,
	GrB_Type T              // return type for C
) ;

GrB_Info LAGraph_isequal    // return GrB_SUCCESS if successful
(
	bool *result,           // true if A == B, false if A != B or error
	GrB_Matrix A,
	GrB_Matrix B,
	GrB_BinaryOp userop     // for A and B with arbitrary user-defined types.
	// Ignored if A and B are of built-in types or
	// LAGraph_ComplexFP64.
) ;

GrB_Info LAGraph_Vector_isequal    // return GrB_SUCCESS if successful
(
	bool *result,           // true if A == B, false if A != B or error
	GrB_Vector A,
	GrB_Vector B,
	GrB_BinaryOp userop     // for A and B with arbitrary user-defined types.
	// Ignored if A and B are of built-in types or
	// LAGraph_ComplexFP64.
) ;

GrB_Info LAGraph_isall      // return GrB_SUCCESS if successful
(
	bool *result,           // true if A == B, false if A != B or error
	GrB_Matrix A,
	GrB_Matrix B,
	GrB_BinaryOp op         // GrB_EQ_<type>, for the type of A and B,
	// to check for equality.  Or use any desired
	// operator.  The operator should return GrB_BOOL.
) ;

GrB_Info LAGraph_Vector_isall      // return GrB_SUCCESS if successful
(
	bool *result,           // true if A == B, false if A != B or error
	GrB_Vector A,
	GrB_Vector B,
	GrB_BinaryOp op         // GrB_EQ_<type>, for the type of A and B,
	// to check for equality.  Or use any desired
	// operator.  The operator should return GrB_BOOL.
) ;

uint64_t LAGraph_rand(uint64_t *seed) ;

uint64_t LAGraph_rand64(uint64_t *seed) ;

double LAGraph_randx(uint64_t *seed) ;

GrB_Info LAGraph_random         // create a random matrix
(
	GrB_Matrix *A,              // handle of matrix to create
	GrB_Type type,              // built-in type, or LAGraph_ComplexFP64
	GrB_Index nrows,            // number of rows
	GrB_Index ncols,            // number of columns
	GrB_Index nvals,            // number of values
	bool make_pattern,          // if true, A is a pattern
	bool make_symmetric,        // if true, A is symmetric
	bool make_skew_symmetric,   // if true, A is skew-symmetric
	bool make_hermitian,        // if true, A is hermitian
	bool no_diagonal,           // if true, A has no entries on the diagonal
	uint64_t *seed              // random number seed; modified on return
) ;

#define LAGr_random(...)                                \
{                                                       \
    LAGRAPH_TRY_CATCH (LAGraph_random (__VA_ARGS__)) ;  \
}

GrB_Info LAGraph_alloc_global() ;

GrB_Info LAGraph_free_global() ;

void *LAGraph_malloc        // wrapper for malloc
(
	size_t nitems,          // number of items
	size_t size_of_item     // size of each item
) ;

void *LAGraph_calloc        // wrapper for calloc
(
	size_t nitems,          // number of items
	size_t size_of_item     // size of each item
) ;

void LAGraph_free           // wrapper for free
(
	void *p
) ;

void LAGraph_tic            // gets current time in seconds and nanoseconds
(
	double tic [2]          // tic [0]: seconds, tic [1]: nanoseconds
) ;

double LAGraph_toc          // returns time since last LAGraph_tic
(
	const double tic [2]    // tic from last call to LAGraph_tic
) ;

GrB_Info LAGraph_prune_diag // remove all entries from the diagonal
(
	GrB_Matrix A
) ;

GrB_Info LAGraph_Vector_to_dense
(
	GrB_Vector *vdense,     // output vector
	GrB_Vector v,           // input vector
	void *id                // pointer to value to fill vdense with
) ;

int LAGraph_set_nthreads        // returns # threads set, 0 if nothing done
(
	int nthreads
) ;

int LAGraph_get_nthreads        // returns # threads to use, 1 if unknown
(
	void
) ;

GrB_Info LAGraph_grread     // read a matrix from a binary file
(
	GrB_Matrix *G,          // handle of matrix to create
	uint64_t *G_version,    // the version in the file
	const char *filename,   // name of file to open
	GrB_Type gtype          // type of matrix to read
) ;

GrB_Info LAGraph_1_to_n     // create an integer vector v = 1:n
(
	GrB_Vector *v_handle,   // vector to create
	GrB_Index n             // size of vector to create
) ;

//------------------------------------------------------------------------------
// user-callable algorithms
//------------------------------------------------------------------------------

GrB_Info LAGraph_bc     // betweeness centrality
(
	GrB_Vector *delta, // delta(i) is the betweeness centrality of node i
	GrB_Matrix A,      // input graph, treated as if boolean in semiring
	GrB_Index s        // source vertex from which to compute shortest paths
);

GrB_Info LAGraph_bc2     // betweeness centrality
(
	GrB_Vector *centrality, // centrality(i): betweeness centrality of node i
	GrB_Matrix A_matrix,    // input graph
	GrB_Index source        // source vertex
) ;

GrB_Info LAGraph_bc_batch // betweeness centrality, batch algorithm
(
	GrB_Vector *delta,  // delta(i) is the betweeness centrality of node i
	const GrB_Matrix A, // input graph, treated as if boolean in semiring
	const GrB_Index *s, // source vertices from which to compute shortest paths
	const int32_t nsver // number of source vertices (length of s)
);

GrB_Info LAGraphX_bc_batch // betweeness centrality, batch algorithm
(
	GrB_Vector *delta,  // delta(i) is the betweeness centrality of node i
	const GrB_Matrix A, // input graph, treated as if boolean in semiring
	const GrB_Index *s, // source vertices from which to compute shortest paths
	const int32_t nsver // number of source vertices (length of s)
);

GrB_Info LAGraphX_bc_batch2 // betweeness centrality, batch algorithm
(
	GrB_Vector *delta,  // delta(i) is the betweeness centrality of node i
	const GrB_Matrix A, // input graph, treated as if boolean in semiring
	const GrB_Index *s, // source vertices from which to compute shortest paths
	const int32_t nsver // number of source vertices (length of s)
);

GrB_Info LAGraphX_bc_batch3 // betweeness centrality, batch algorithm
(
	GrB_Vector *delta,  // delta(i) is the betweeness centrality of node i
	const GrB_Matrix A, // input graph, treated as if boolean in semiring
	const GrB_Matrix AT, // A'
	const GrB_Index *s, // source vertices from which to compute shortest paths
	const int32_t nsver, // number of source vertices (length of s)
	double timing [3]
);

GrB_Info LAGraph_bc_batch3 // betweeness centrality, batch algorithm
(
	GrB_Vector *centrality,    // centrality(i) is the betweeness centrality of node i
	const GrB_Matrix A,        // input graph, treated as if boolean in semiring
	const GrB_Matrix AT,       // A'
	const GrB_Index *sources,  // source vertices from which to compute shortest paths
	int32_t num_sources        // number of source vertices (length of s)
) ;

GrB_Info LAGraph_bc_batch4      // betweeness centrality, batch algorithm
(
	GrB_Vector *centrality,    // centrality(i) is the betweeness centrality of node i
	const GrB_Matrix A_matrix, // input graph, treated as if boolean in semiring
	const GrB_Matrix AT_matrix, // A'
	const GrB_Index *sources,  // source vertices from which to compute shortest paths
	int32_t num_sources        // number of source vertices (length of s)
) ;

GrB_Info LAGraph_bfs_pushpull   // push-pull BFS, or push-only if AT = NULL
(
	GrB_Vector *v_output,   // v(i) is the BFS level of node i in the graph
	GrB_Vector *pi_output,  // pi(i) is the parent of node i in the graph.
	// if NULL, the parent is not computed
	GrB_Matrix A,           // input graph, treated as if boolean in semiring
	GrB_Matrix AT,          // transpose of A (optional; push-only if NULL)
	int64_t s,              // starting node of the BFS (s < 0: whole graph)
	int64_t max_level,      // optional limit of # levels to search
	bool vsparse            // if true, v is expected to be very sparse
) ;

GrB_Info LAGraph_bfs_parent // push-pull BFS, compute the tree only
(
	// output:
	GrB_Vector *pi_output,  // pi(i) = p+1 if p is the parent of node i
	// inputs:
	GrB_Matrix A,           // input graph, any type
	GrB_Matrix AT,          // transpose of A (optional; push-only if NULL)
	GrB_Vector Degree,      // Degree(i) is the out-degree of node i
	// (optional: push-only if NULL)
	int64_t source          // starting node of the BFS
) ;

GrB_Info bfs_log                // push-pull BFS, or push-only if AT = NULL
(
	GrB_Vector *v_output,   // v(i) is the BFS level of node i in the graph
	GrB_Vector *pi_output,  // pi(i) = p if p is the parent of node i.
	// if NULL, the parent is not computed.
	GrB_Matrix A,           // input graph, treated as if boolean in semiring
	GrB_Matrix AT,          // transpose of A (optional; push-only if NULL)
	int64_t s,              // starting node of the BFS
	int64_t max_level,      // optional limit of # levels to search
	bool vsparse            // if true, v is expected to be very sparse
	, FILE *file
)  ;

GrB_Info LAGraph_bfs_simple     // push-only BFS
(
	GrB_Vector *v_output,   // v(i) is the BFS level of node i in the graph
	const GrB_Matrix A,     // input graph, treated as if boolean in semiring
	GrB_Index s             // starting node of the BFS
) ;

GrB_Info LAGraph_cc_lacc(
	GrB_Vector *result,     // output: array of component identifiers
	GrB_Matrix A,           // input matrix
	bool sanitize           // if true, ensure A is symmetric
) ;

GrB_Info LAGraph_cc_fastsv(
	GrB_Vector *result,     // output: array of component identifiers
	GrB_Matrix A,           // input matrix
	bool sanitize           // if true, ensure A is symmetric
) ;

GrB_Info LAGraph_cc_fastsv2(
	GrB_Vector *result,     // output: array of component identifiers
	GrB_Matrix A,           // input matrix
	bool sanitize           // if true, ensure A is symmetric
) ;

GrB_Info LAGraph_cc_fastsv3(
	GrB_Vector *result,     // output: array of component identifiers
	GrB_Matrix A,           // input matrix
	bool sanitize           // if true, ensure A is symmetric
) ;

GrB_Info LAGraph_cc_fastsv4(
	GrB_Vector *result,     // output: array of component identifiers
	GrB_Matrix A,           // input matrix
	bool sanitize           // if true, ensure A is symmetric
) ;

GrB_Info LAGraph_cc_fastsv5(
	GrB_Vector *result,     // output: array of component identifiers
	GrB_Matrix A,           // input matrix
	bool sanitize           // if true, ensure A is symmetric
) ;

GrB_Info LAGraph_cc_fastsv5a(
	GrB_Vector *result,     // output: array of component identifiers
	GrB_Matrix *A,          // input matrix
	//   content remains the same, but pointer changes
	bool sanitize           // if true, ensure A is symmetric
) ;

GrB_Info LAGraph_cc_fastsv5b(
	GrB_Vector *result,     // output: array of component identifiers
	GrB_Matrix *A,          // input matrix
	//   content remains the same, but pointer changes
	bool sanitize           // if true, ensure A is symmetric
) ;

GrB_Info LAGraph_cc_boruvka(
	GrB_Vector *result,     // output: array of component identifiers
	GrB_Matrix A,           // input matrix
	bool sanitize           // if true, ensure A is symmetric
) ;

GrB_Info LAGraph_msf(
	GrB_Matrix *result,     // output: an unsymmetrical matrix, the spanning forest
	GrB_Matrix A,           // input matrix
	bool sanitize           // if true, ensure A is symmetric
) ;

GrB_Info LAGraph_scc(
	GrB_Vector *result,     // output: array of component identifiers
	GrB_Matrix A            // input matrix
) ;

// LAGraph_pagerank computes an array of structs for its result
typedef struct {
	double pagerank ;   // the pagerank of a node
	GrB_Index page ;    // the node number itself
}
LAGraph_PageRank ;

GrB_Info LAGraph_pagerank       // GrB_SUCCESS or error condition
(
	LAGraph_PageRank **Phandle, // output: array of LAGraph_PageRank structs
	GrB_Matrix A,               // binary input graph, not modified
	int itermax,                // max number of iterations
	double tol,                 // stop when norm (r-rnew,2) < tol
	int *iters                  // number of iterations taken
) ;

GrB_Info LAGraph_pagerank2      // second PageRank definition
(
	GrB_Vector *result,         // output: array of LAGraph_PageRank structs
	GrB_Matrix A,               // binary input graph, not modified
	double damping_factor,      // damping factor
	unsigned long itermax       // number of iterations
) ;

GrB_Info LAGraph_pagerank3a // PageRank definition
(
	GrB_Vector *result,     // output: array of LAGraph_PageRank structs
	GrB_Matrix A,           // binary input graph, not modified
	GrB_Vector d_out,       // outbound degree of all nodes
	float damping,          // damping factor (typically 0.85)
	int itermax,            // maximum number of iterations
	int *iters              // output: number of iterations taken
) ;

GrB_Info LAGraph_pagerank3b     // PageRank definition
(
	GrB_Vector *result,         // output: array of LAGraph_PageRank structs
	GrB_Matrix A,               // binary input graph, not modified
	float damping_factor,       // damping factor
	unsigned long itermax,      // maximum number of iterations
	int *iters                  // number of iterations taken
) ;

GrB_Info LAGraph_pagerank3c         // PageRank definition
(
	GrB_Vector *result,             // output: array of LAGraph_PageRank structs
	GrB_Matrix A,                   // binary input graph, not modified
	const float *LA_RESTRICT d_out, // out degree of each node (GrB_FP32, size n)
	float damping,                  // damping factor (typically 0.85)
	int itermax,                    // maximum number of iterations
	int *iters                      // output: number of iterations taken
) ;

GrB_Info LAGraph_pagerank3d // PageRank definition
(
	GrB_Vector *result,     // output: array of LAGraph_PageRank structs
	GrB_Matrix A,           // binary input graph, not modified
	GrB_Vector d_out,       // outbound degree of all nodes (not modified)
	float damping,          // damping factor (typically 0.85)
	int itermax,            // maximum number of iterations
	int *iters              // output: number of iterations taken
) ;

GrB_Info LAGraph_pagerank3e // PageRank definition
(
	GrB_Vector *result,     // output: array of LAGraph_PageRank structs
	GrB_Matrix A,           // binary input graph, not modified
	GrB_Vector d_out,       // outbound degree of all nodes (not modified)
	float damping,          // damping factor (typically 0.85)
	int itermax,            // maximum number of iterations
	int *iters              // output: number of iterations taken
) ;

GrB_Info LAGraph_pagerank3f // PageRank definition
(
	GrB_Vector *result,     // output: array of LAGraph_PageRank structs
	GrB_Matrix A,           // binary input graph, not modified
	GrB_Vector d_out,       // outbound degree of all nodes (not modified)
	float damping,          // damping factor (typically 0.85)
	int itermax,            // maximum number of iterations
	int *iters              // output: number of iterations taken
) ;

GrB_Info LAGraph_pagerankx4 // PageRank definition
(
	GrB_Vector *result,     // output: array of LAGraph_PageRank structs
	GrB_Matrix A,           // binary input graph, not modified
	const float *LA_RESTRICT d_out, // out degree of each node (GrB_FP32, size n)
	float damping,          // damping factor (typically 0.85)
	int itermax,            // maximum number of iterations
	int *iters              // output: number of iterations taken
) ;

GrB_Info LAGraph_tricount   // count # of triangles
(
	int64_t *ntri,          // # of triangles
	const int method,       // 1 to 6, see above
	const int sorting,      //  0: no sort
	//  1: sort by degree, ascending order
	// -1: sort by degree, descending order
	const int64_t *degree,  // degree of each node, may be NULL if sorting==0.
	// of size n, unmodified.
	const GrB_Matrix A_in   // input matrix, must be symmetric, no diag entries
) ;

GrB_Info LAGraph_ktruss         // compute the k-truss of a graph
(
	GrB_Matrix *Chandle,        // output k-truss subgraph, C
	const GrB_Matrix A,         // input adjacency matrix, A, not modified
	const uint32_t k,           // find the k-truss, where k >= 3
	int32_t *nsteps             // # of steps taken (ignored if NULL)
) ;

GrB_Info LAGraph_allktruss      // compute all k-trusses of a graph
(
	GrB_Matrix *Cset,           // size n, output k-truss subgraphs (optional)
	GrB_Matrix A,               // input adjacency matrix, A, not modified
	// output statistics
	int64_t *kmax,              // smallest k where k-truss is empty
	int64_t *ntris,             // size n, ntris [k] is #triangles in k-truss
	int64_t *nedges,            // size n, nedges [k] is #edges in k-truss
	int64_t *nstepss            // size n, nstepss [k] is #steps for k-truss
) ;

GrB_Info LAGraph_BF_full
(
	GrB_Vector *pd,             //the pointer to the vector of distance
	GrB_Vector *ppi,            //the pointer to the vector of parent
	GrB_Vector *ph,             //the pointer to the vector of hops
	const GrB_Matrix A,         //matrix for the graph
	const GrB_Index s           //given index of the source
) ;

GrB_Info LAGraph_BF_basic
(
	GrB_Vector *pd,             //the pointer to the vector of distance
	const GrB_Matrix A,         //matrix for the graph
	const GrB_Index s           //given index of the source
) ;

GrB_Info LAGraph_BF_basic_pushpull
(
	GrB_Vector *pd,             //the pointer to the vector of distance
	const GrB_Matrix A,         //matrix for the graph
	const GrB_Matrix AT,        //transpose of A (optional)
	const GrB_Index s           //given index of the source
) ;

GrB_Info LAGraph_lcc            // compute lcc for all nodes in A
(
	GrB_Vector *LCC_handle,     // output vector
	const GrB_Matrix A,         // input matrix
	bool symmetric,             // if true, the matrix is symmetric
	bool sanitize,              // if true, ensure A is binary
	double t [2]                // t [0] = sanitize time, t [1] = lcc time,
	// in seconds
) ;

GrB_Info LAGraph_cdlp           // compute cdlp for all nodes in A
(
	GrB_Vector *CDLP_handle,    // output vector
	const GrB_Matrix A,         // input matrix
	bool symmetric,             // denote whether the matrix is symmetric
	bool sanitize,              // if true, ensure A is binary
	int itermax,                // max number of iterations,
	double *t                   // t [0] = sanitize time, t [1] = cdlp time,
	// in seconds
) ;

GrB_Info LAGraph_dense_relabel   // relabel sparse IDs to dense row/column indices
(
	GrB_Matrix *Id2index_handle, // output matrix: A(id, index)=1 (unfilled if NULL)
	GrB_Matrix *Index2id_handle, // output matrix: B(index, id)=1 (unfilled if NULL)
	GrB_Vector *id2index_handle, // output vector: v(id)=index (unfilled if NULL)
	const GrB_Index *ids,        // array of unique identifiers (under GxB_INDEX_MAX)
	GrB_Index nids,              // number of identifiers
	GrB_Index *id_dimension      // number of rows in Id2index matrix, id2index vector (unfilled if NULL)
) ;

GrB_Info LAGraph_dnn    // returns GrB_SUCCESS if successful
(
	// output
	GrB_Matrix *Yhandle,    // Y, created on output
	// input: not modified
	GrB_Matrix *W,      // W [0..nlayers-1], each nneurons-by-nneurons
	GrB_Matrix *Bias,   // Bias [0..nlayers-1], diagonal nneurons-by-nneurons
	int nlayers,        // # of layers
	GrB_Matrix Y0       // input features: nfeatures-by-nneurons
) ;

GrB_Info LAGraph_sssp // single source shortest paths
(
	GrB_Vector *path_length,   // path_length(i) is the length of the shortest
	// path from the source vertex to vertex i
	const GrB_Matrix graph,    // input graph, treated as if boolean in semiring
	const GrB_Index source,    // source vertex from which to compute shortest paths
	double delta               // delta value for delta stepping
) ;

GrB_Info LAGraph_sssp1 // single source shortest paths
(
	GrB_Vector *path_length,   // path_length(i) is the length of the shortest
	// path from the source vertex to vertex i
	GrB_Matrix graph,          // input graph, treated as if boolean in semiring
	GrB_Index source,          // source vertex from which to compute shortest paths
	int32_t delta               // delta value for delta stepping
);


GrB_Info LAGraph_BF_pure_c
(
	int32_t **pd,    // pointer to distance vector d, d(k) = shorstest distance
	// between s and k if k is reachable from s
	int64_t **ppi,   // pointer to parent index vector pi, pi(k) = parent of
	// node k in the shortest path tree
	const int64_t s, // given source node index
	const int64_t n, // number of nodes
	const int64_t nz,// number of edges
	const int64_t *I,// row index vector
	const int64_t *J,// column index vector
	const int32_t *W // weight vector, W(i) = weight of edge (I(i),J(i))
);

GrB_Info LAGraph_sssp11         // single source shortest paths
(
	GrB_Vector *path_length,   // path_length(i) is the length of the shortest
	// path from the source vertex to vertex i
	GrB_Matrix A,              // input graph, treated as if boolean in
	// semiring (INT32)
	GrB_Index source,          // source vertex from which to compute
	// shortest paths
	int32_t delta,             // delta value for delta stepping
	bool AIsAllPositive        // A boolean indicating whether the entries of
	// matrix A are all positive
);

GrB_Info LAGraph_sssp12         // single source shortest paths
(
	GrB_Vector *path_length,   // path_length(i) is the length of the shortest
	// path from the source vertex to vertex i
	GrB_Matrix A,              // input graph, treated as if boolean in
	// semiring (INT32)
	GrB_Index source,          // source vertex from which to compute
	// shortest paths
	int32_t delta,             // delta value for delta stepping
	bool AIsAllPositive        // A boolean indicating whether the entries of
	// matrix A are all positive
);


GrB_Info LAGraph_sssp12c        // single source shortest paths
(
	GrB_Vector *path_length,   // path_length(i) is the length of the shortest
	// path from the source vertex to vertex i
	GrB_Matrix A,              // input graph, treated as if boolean in
	// semiring (INT32)
	GrB_Index source,          // source vertex from which to compute
	// shortest paths
	int32_t delta,             // delta value for delta stepping

	// TODO: make this an enum:
	//      case 0: A can have negative, zero, or positive entries
	//      case 1: A can have zero or positive entries
	//      case 2: A only has positive entries (see FIXME below)
	bool AIsAllPositive        // A boolean indicating whether the entries of
	// matrix A are all positive
);


GrB_Info LAGraph_bfs_both       // push-pull BFS, or push-only if AT = NULL
(
	GrB_Vector *v_output,   // v(i) is the BFS level of node i in the graph
	GrB_Vector *pi_output,  // pi(i) = p+1 if p is the parent of node i.
	// if NULL, the parent is not computed.
	GrB_Matrix A,           // input graph, treated as if boolean in semiring
	GrB_Matrix AT,          // transpose of A (optional; push-only if NULL)
	int64_t source,         // starting node of the BFS
	int64_t max_level,      // optional limit of # levels to search
	bool vsparse            // if true, v is expected to be very sparse
	, FILE *logfile
) ;


GrB_Info LAGraph_Matrix_extract_keep_dimensions // extract submatrix but keep
// the dimensions of the
// original matrix
(
	GrB_Matrix *Chandle,         // output matrix
	const GrB_Matrix A,          // input matrix
	const GrB_Index *Vsparse,    // sorted list of vertex indices
	const bool *Vdense,          // boolean array of vertices
	GrB_Index nv                 // number of vertex indices
) ;

#endif

