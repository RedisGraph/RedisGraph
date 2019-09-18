//------------------------------------------------------------------------------
// GB.h: definitions visible only inside GraphBLAS
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// These defintions are not visible to the user.  They are used only inside
// GraphBLAS itself.

#ifndef GB_H
#define GB_H

//------------------------------------------------------------------------------
// code development settings
//------------------------------------------------------------------------------

// to turn on Debug for a single file of GraphBLAS, add:
// #define GB_DEBUG
// just before the statement:
// #include "GB.h"

// to turn on Debug for all of GraphBLAS, uncomment this line:
// #define GB_DEBUG

// to reduce code size and for faster time to compile, uncomment this line;
// GraphBLAS will be slower:
// #define GBCOMPACT 1

// set these via cmake, or uncomment to select the user-thread model:

// #define USER_POSIX_THREADS
// #define USER_OPENMP_THREADS
// #define USER_NO_THREADS

//------------------------------------------------------------------------------
// manage compiler warnings
//------------------------------------------------------------------------------

#if defined __INTEL_COMPILER

// disable icc -w2 warnings
//  191:  type qualifier meangingless
//  193:  zero used for undefined #define
//  589:  bypass initialization
#pragma warning (disable: 191 193 )

// disable icc -w3 warnings
//  144:  initialize with incompatible pointer
//  181:  format
//  869:  unused parameters
//  1572: floating point comparisons
//  1599: shadow
//  2259: typecasting may lose bits
//  2282: unrecognized pragma
//  2557: sign compare
#pragma warning (disable: 144 181 869 1572 1599 2259 2282 2557 )

// See GB_unused.h, for warnings 177 and 593, which are not globally 
// disabled, but selectively by #include'ing GB_unused.h as needed.

// resolved (warnings no longer disabled globally):
//  58:   sign compare
//  167:  incompatible pointer
//  177:  declared but unused
//  186:  useless comparison
//  188:  mixing enum types
//  593:  set but not used
//  981:  unspecified order
//  1418: no external declaration
//  1419: external declaration in source file
//  2330: const incompatible
//  2547: remark about include files
//  3280: shadow

#elif defined __GNUC__

// disable warnings for gcc 8.2:
#pragma GCC diagnostic ignored "-Wunknown-warning-option"
#pragma GCC diagnostic ignored "-Wint-in-bool-context"
#pragma GCC diagnostic ignored "-Wformat-truncation="

// disable warnings from -Wall -Wextra -Wpendantic
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#pragma GCC diagnostic ignored "-Wincompatible-pointer-types"

// See GB_unused.h, where these two pragmas are used:
// #pragma GCC diagnostic ignored "-Wunused-but-set-variable"
// #pragma GCC diagnostic ignored "-Wunused-variable"

// resolved (warnings no longer disabled globally):
// #pragma GCC diagnostic ignored "-Wunknown-pragmas"
// #pragma GCC diagnostic ignored "-Wtype-limits"
// #pragma GCC diagnostic ignored "-Wunused-result"
// #pragma GCC diagnostic ignored "-Wdiscarded-qualifiers"

// enable these warnings as errors
#pragma GCC diagnostic error "-Wmisleading-indentation"
#pragma GCC diagnostic error "-Wswitch-default"
#pragma GCC diagnostic error "-Wmissing-prototypes"

// #pragma GCC diagnostic error "-Wdouble-promotion"

#endif

//------------------------------------------------------------------------------
// include GraphBLAS.h (depends on user threading model)
//------------------------------------------------------------------------------

#include "GraphBLAS.h"

//------------------------------------------------------------------------------
// min, max, and NaN handling
//------------------------------------------------------------------------------

// For floating-point computations, SuiteSparse:GraphBLAS relies on the IEEE
// 754 standard for the basic operations (+ - / *).  Comparison operators also
// work as they should; any comparison with NaN is always false, even
// eq(NaN,NaN) is false.  This follows the IEEE 754 standard.

// For integer MIN and MAX, SuiteSparse:GraphBLAS relies on one comparison:

// z = min(x,y) = (x < y) ? x : y
// z = max(x,y) = (x > y) ? x : y

// However, this is not suitable for floating-point x and y.  Comparisons with
// NaN always return false, so if either x or y are NaN, then z = y, for both
// min(x,y) and max(x,y).  In MATLAB, min(3,NaN), min(NaN,3), max(3,NaN), and
// max(NaN,3) are all 3, which is another interpretation.  The MATLAB min and
// max functions have a 3rd argument that specifies how NaNs are handled:
// 'omitnan' (default) and 'includenan'.  In SuiteSparse:GraphBLAS 2.2.* and
// earlier, the min and max functions were the same as 'includenan' in MATLAB.
// As of version 2.3 and later, they are 'omitnan', to facilitate the terminal
// exit of the MIN and MAX monoids for floating-point values.

// The ANSI C11 fmin, fminf, fmax, and fmaxf functions have the 'omitnan'
// behavior.  These are used in SuiteSparse:GraphBLAS v2.3.0 and later.

// Below is a complete comparison of MATLAB and GraphBLAS.  Both tables are the
// results for both min and max (they return the same results in these cases):

//   x    y  MATLAB    MATLAB   (x<y)?x:y   SuiteSparse:    SuiteSparse:    ANSI
//           omitnan includenan             GraphBLAS       GraphBLAS       fmin
//                                          v 2.2.x         this version
//
//   3    3     3        3          3        3              3               3
//   3   NaN    3       NaN        NaN      NaN             3               3
//  NaN   3     3       NaN         3       NaN             3               3
//  NaN  NaN   NaN      NaN        NaN      NaN             NaN             NaN

// for integers only:
#define GB_IABS(x) (((x) >= 0) ? (x) : (-(x)))

// suitable for integers, and non-NaN floating point:
#define GB_IMAX(x,y) (((x) > (y)) ? (x) : (y))
#define GB_IMIN(x,y) (((x) < (y)) ? (x) : (y))

//------------------------------------------------------------------------------
// for coverage tests in Tcov/
//------------------------------------------------------------------------------

#ifdef GBCOVER
#define GBCOVER_MAX 10000
extern int64_t GB_cov [GBCOVER_MAX] ;
extern int GB_cover_max ;
#endif

//------------------------------------------------------------------------------
// internal typedefs, not visible at all to the GraphBLAS user
//------------------------------------------------------------------------------

typedef unsigned char GB_void ;

typedef void (*GB_cast_function) (void *, const void *, size_t) ;

#define GB_LEN 128

typedef struct GB_Sauna_struct *GB_Sauna ;

//------------------------------------------------------------------------------
// pending tuples
//------------------------------------------------------------------------------

// Pending tuples are a list of unsorted (i,j,x) tuples that have not yet been
// added to a matrix.  The data structure is defined in GB_Pending.h.

typedef struct GB_Pending_struct *GB_Pending ;

//------------------------------------------------------------------------------
// type codes for GrB_Type
//------------------------------------------------------------------------------

typedef enum
{
    // the 12 scalar types
    GB_ignore_code  = 0,
    GB_BOOL_code    = 0,
    GB_INT8_code    = 1,
    GB_UINT8_code   = 2,
    GB_INT16_code   = 3,
    GB_UINT16_code  = 4,
    GB_INT32_code   = 5,
    GB_UINT32_code  = 6,
    GB_INT64_code   = 7,
    GB_UINT64_code  = 8,
    GB_FP32_code    = 9,
    GB_FP64_code    = 10,
    GB_UCT_code     = 11,       // void *, compile-time user-defined type
    GB_UDT_code     = 12        // void *, run-time user-defined type
}
GB_Type_code ;                  // enumerated type code

//------------------------------------------------------------------------------
// operator codes used in GrB_BinaryOp and GrB_UnaryOp
//------------------------------------------------------------------------------

typedef enum
{
    //--------------------------------------------------------------------------
    // NOP
    //--------------------------------------------------------------------------

    // a placeholder; not an actual operator
    GB_NOP_opcode,      //  0: nop

    //--------------------------------------------------------------------------
    // T -> T
    //--------------------------------------------------------------------------

    // 6 unary operators x=f(x) that return the same type as their input
    GB_ONE_opcode,      //  1: z = 1
    GB_IDENTITY_opcode, //  2: z = x
    GB_AINV_opcode,     //  3: z = -x
    GB_ABS_opcode,      //  4: z = abs(x)
    GB_MINV_opcode,     //  5: z = 1/x ; special cases for bool and integers
    GB_LNOT_opcode,     //  6: z = !x

    //--------------------------------------------------------------------------
    // TxT -> T
    //--------------------------------------------------------------------------

    // 10 binary operators z=f(x,y) that return the same type as their inputs
    GB_FIRST_opcode,    //  7: z = x
    GB_SECOND_opcode,   //  8: z = y
    GB_MIN_opcode,      //  9: z = min(x,y)
    GB_MAX_opcode,      // 10: z = max(x,y)
    GB_PLUS_opcode,     // 11: z = x + y
    GB_MINUS_opcode,    // 12: z = x - y
    GB_RMINUS_opcode,   // 13: z = y - x
    GB_TIMES_opcode,    // 14: z = x * y
    GB_DIV_opcode,      // 15: z = x / y ; special cases for bool and ints
    GB_RDIV_opcode,     // 16: z = y / x ; special cases for bool and ints

    // 6 binary operators z=f(x,y), x,y,z all the same type
    GB_ISEQ_opcode,     // 17: z = (x == y)
    GB_ISNE_opcode,     // 18: z = (x != y)
    GB_ISGT_opcode,     // 19: z = (x >  y)
    GB_ISLT_opcode,     // 20: z = (x <  y)
    GB_ISGE_opcode,     // 21: z = (x >= y)
    GB_ISLE_opcode,     // 22: z = (x <= y)

    // 3 binary operators that work on purely boolean values
    GB_LOR_opcode,      // 23: z = (x != 0) || (y != 0)
    GB_LAND_opcode,     // 23: z = (x != 0) && (y != 0)
    GB_LXOR_opcode,     // 25: z = (x != 0) != (y != 0)

    //--------------------------------------------------------------------------
    // TxT -> bool
    //--------------------------------------------------------------------------

    // 6 binary operators z=f(x,y) that return bool (TxT -> bool)
    GB_EQ_opcode,       // 26: z = (x == y)
    GB_NE_opcode,       // 27: z = (x != y)
    GB_GT_opcode,       // 28: z = (x >  y)
    GB_LT_opcode,       // 29: z = (x <  y)
    GB_GE_opcode,       // 30: z = (x >= y)
    GB_LE_opcode,       // 31: z = (x <= y)

    //--------------------------------------------------------------------------
    // user-defined: unary and binary operators
    //--------------------------------------------------------------------------

    GB_USER_C_opcode,   // 32: compile-time user-defined operator
    GB_USER_R_opcode    // 33: run-time user-defined operator
}
GB_Opcode ;

//------------------------------------------------------------------------------
// select opcodes
//------------------------------------------------------------------------------

// operator codes used in GrB_SelectOp structures
typedef enum
{
    // built-in select operators: thunk optional; defaults to zero
    GB_TRIL_opcode      = 0,
    GB_TRIU_opcode      = 1,
    GB_DIAG_opcode      = 2,
    GB_OFFDIAG_opcode   = 3,
    GB_RESIZE_opcode    = 4,

    // built-in select operators, no thunk used
    GB_NONZOMBIE_opcode = 5,
    GB_NONZERO_opcode   = 6,
    GB_EQ_ZERO_opcode   = 7,
    GB_GT_ZERO_opcode   = 8,
    GB_GE_ZERO_opcode   = 9,
    GB_LT_ZERO_opcode   = 10,
    GB_LE_ZERO_opcode   = 11,

    // built-in select operators, thunk optional; defaults to zero
    GB_NE_THUNK_opcode  = 12,
    GB_EQ_THUNK_opcode  = 13,
    GB_GT_THUNK_opcode  = 14,
    GB_GE_THUNK_opcode  = 15,
    GB_LT_THUNK_opcode  = 16,
    GB_LE_THUNK_opcode  = 17,

    // for all user-defined select operators:  thunk is optional
    GB_USER_SELECT_C_opcode = 18,   // defined at compile-time
    GB_USER_SELECT_R_opcode = 19    // defined at run-time
}
GB_Select_Opcode ;


//------------------------------------------------------------------------------
// opaque content of GraphBLAS objects
//------------------------------------------------------------------------------

struct GB_Type_opaque       // content of GrB_Type
{
    int64_t magic ;         // for detecting uninitialized objects
    size_t size ;           // size of the type
    GB_Type_code code ;     // the type code
    char name [GB_LEN] ;    // name of the type
} ;

struct GB_UnaryOp_opaque    // content of GrB_UnaryOp
{
    int64_t magic ;         // for detecting uninitialized objects
    GrB_Type xtype ;        // type of x
    GrB_Type ztype ;        // type of z
    GxB_unary_function function ;        // a pointer to the unary function
    char name [GB_LEN] ;    // name of the unary operator
    GB_Opcode opcode ;      // operator opcode
} ;

struct GB_BinaryOp_opaque   // content of GrB_BinaryOp
{
    int64_t magic ;         // for detecting uninitialized objects
    GrB_Type xtype ;        // type of x
    GrB_Type ytype ;        // type of y
    GrB_Type ztype ;        // type of z
    GxB_binary_function function ;        // a pointer to the binary function
    char name [GB_LEN] ;    // name of the binary operator
    GB_Opcode opcode ;      // operator opcode
} ;

struct GB_SelectOp_opaque   // content of GxB_SelectOp
{
    int64_t magic ;         // for detecting uninitialized objects
    GrB_Type xtype ;        // type of x, or NULL if generic
    GrB_Type ttype ;        // type of thunk, or NULL if not used or generic
    GxB_select_function function ;        // a pointer to the select function
    char name [GB_LEN] ;    // name of the select operator
    GB_Select_Opcode opcode ;   // operator opcode
} ;

// codes used in GrB_Monoid and GrB_Semiring objects
typedef enum
{
    GB_BUILTIN,             // 0: built-in monoid or semiring
    GB_USER_COMPILED,       // 1: pre-compiled user monoid or semiring
    GB_USER_RUNTIME         // 2: user monoid or semiring created a run-time
}
GB_object_code ;

struct GB_Monoid_opaque     // content of GrB_Monoid
{
    int64_t magic ;         // for detecting uninitialized objects
    GrB_BinaryOp op ;       // binary operator of the monoid
    void *identity ;        // identity of the monoid
    size_t op_ztype_size ;  // size of the type (also is op->ztype->size)
    GB_object_code object_kind ;   // built-in, user pre-compiled, or run-time
    void *terminal ;        // value that triggers early-exit (NULL if no value)
} ;

struct GB_Semiring_opaque   // content of GrB_Semiring
{
    int64_t magic ;         // for detecting uninitialized objects
    GrB_Monoid add ;        // add operator of the semiring
    GrB_BinaryOp multiply ; // multiply operator of the semiring
    GB_object_code object_kind ;   // built-in, user pre-compiled, or run-time
} ;

struct GB_Scalar_opaque     // content of GxB_Scalar: 1-by-1 standard CSC matrix
{
    #include "GB_matrix.h"
} ;

struct GB_Vector_opaque     // content of GrB_Vector: m-by-1 standard CSC matrix
{
    #include "GB_matrix.h"
} ;

struct GB_Matrix_opaque     // content of GrB_Matrix
{
    #include "GB_matrix.h"
} ;

struct GB_Descriptor_opaque // content of GrB_Descriptor
{
    int64_t magic ;         // for detecting uninitialized objects
    GrB_Desc_Value out ;    // output descriptor
    GrB_Desc_Value mask ;   // mask descriptor
    GrB_Desc_Value in0 ;    // first input descriptor (A for C=A*B, for example)
    GrB_Desc_Value in1 ;    // second input descriptor (B for C=A*B)
    GrB_Desc_Value axb ;    // for selecting the method for C=A*B
    int nthreads_max ;      // max # threads to use in this call to GraphBLAS
    double chunk ;          // chunk size for # of threads for small problems
} ;

//------------------------------------------------------------------------------
// default options
//------------------------------------------------------------------------------

// These parameters define the content of extern const values that can be
// used as inputs to GxB_*Option_set.

// The default format is by row (CSR), with a hyper_ratio of 1/16.
// In Versions 2.1 and earlier, the default was GxB_BY_COL (CSC format).

#define GB_HYPER_DEFAULT (0.0625)

// compile SuiteSparse:GraphBLAS with "-DBYCOL" to make GxB_BY_COL the default
// format
#ifdef BYCOL
#define GB_FORMAT_DEFAULT GxB_BY_COL
#else
#define GB_FORMAT_DEFAULT GxB_BY_ROW
#endif

// these parameters define the hyper_ratio needed to ensure matrix stays
// either always hypersparse, or never hypersparse.
#define GB_ALWAYS_HYPER (1.0)
#define GB_NEVER_HYPER  (-1.0)

#define GB_FORCE_HYPER 1
#define GB_FORCE_NONHYPER 0
#define GB_AUTO_HYPER (-1)

#define GB_SAME_HYPER_AS(A_is_hyper) \
    ((A_is_hyper) ? GB_FORCE_HYPER : GB_FORCE_NONHYPER)

// if A is hypersparse but all vectors are present, then
// treat A as if it were non-hypersparse
#define GB_IS_HYPER(A) \
    (((A) != NULL) && ((A)->is_hyper && ((A)->nvec < (A)->vdim)))

//------------------------------------------------------------------------------
// macros for matrices and vectors
//------------------------------------------------------------------------------

// If A->nzmax is zero, then A->p might not be allocated.  Note that this
// function does not count pending tuples; use GB_WAIT(A) first, if needed.
// For sparse or hypersparse matrix, Ap [0] == 0.  For a slice or hyperslice,
// Ap [0] >= 0 points to the first entry in the slice.  For all 4 cases
// (sparse, hypersparse, slice, hyperslice), nnz(A) = Ap [nvec] - Ap [0].
#define GB_NNZ(A) (((A)->nzmax > 0) ? ((A)->p [(A)->nvec] - (A)->p [0]) : 0 )

// Upper bound on nnz(A) when the matrix has zombies and pending tuples;
// does not need GB_WAIT(A) first.
#define GB_NNZ_UPPER_BOUND(A) ((GB_NNZ (A) - A->nzombies) + GB_Pending_n (A))

int64_t GB_Pending_n        // return # of pending tuples in A
(
    GrB_Matrix A
) ;

// A is nrows-by-ncols, in either CSR or CSC format
#define GB_NROWS(A) ((A)->is_csc ? (A)->vlen : (A)->vdim)
#define GB_NCOLS(A) ((A)->is_csc ? (A)->vdim : (A)->vlen)

// The internal content of a GrB_Matrix and GrB_Vector are identical, and
// inside SuiteSparse:GraphBLAS, they can be typecasted between each other.
// This typecasting feature should not be done in user code, however, since it
// is not supported in the API.  All GrB_Vector objects can be safely
// typecasted into a GrB_Matrix, but not the other way around.  The GrB_Vector
// object is more restrictive.  The GB_VECTOR_OK(v) macro defines the content
// that all GrB_Vector objects must have.

// GB_VECTOR_OK(v) is used mainly for assertions, but also to determine when it
// is safe to typecast an n-by-1 GrB_Matrix (in standard CSC format) into a
// GrB_Vector.  This is not done in the main SuiteSparse:GraphBLAS library, but
// in the GraphBLAS/Test directory only.  The macro is also used in
// GB_Vector_check, to ensure the content of a GrB_Vector is valid.

#define GB_VECTOR_OK(v)             \
(                                   \
    ((v) != NULL) &&                \
    ((v)->is_hyper == false) &&     \
    ((v)->is_csc == true) &&        \
    ((v)->plen == 1) &&             \
    ((v)->vdim == 1) &&             \
    ((v)->nvec == 1) &&             \
    ((v)->h == NULL)                \
)

// A GxB_Vector is a GrB_Vector of length 1
#define GB_SCALAR_OK(v) (GB_VECTOR_OK(v) && ((v)->vlen == 1))

//------------------------------------------------------------------------------
// GB_INDEX_MAX
//------------------------------------------------------------------------------

// The largest valid dimension permitted in this implementation is 2^60.
// Matrices with that many rows and/or columns can be actually be easily
// created, particularly if they are hypersparse since in that case O(nrows) or
// O(ncols) memory is not needed.  For the standard formats, O(ncols) space is
// needed for CSC and O(nrows) space is needed for CSR.  For hypersparse
// matrices, the time complexity does not depend on O(nrows) or O(ncols).

#ifndef GB_INDEX_MAX
#define GB_INDEX_MAX ((GrB_Index) (1ULL << 60))
#endif

// format strings, normally %llu and %lld, for GrB_Index values
#define GBu "%" PRIu64
#define GBd "%" PRId64

//------------------------------------------------------------------------------
// Global access functions
//------------------------------------------------------------------------------

// These functions are available to all internal functions in
// SuiteSparse:GraphBLAS, but the GB_Global struct is accessible only inside
// GB_Global.c.

#include "GB_Global.h"

//------------------------------------------------------------------------------
// debugging definitions
//------------------------------------------------------------------------------

#undef ASSERT
#undef ASSERT_OK
#undef ASSERT_OK_OR_NULL
#undef ASSERT_OK_OR_JUMBLED

#ifdef GB_DEBUG

    // assert X is true
    #define ASSERT(X)                                                       \
    {                                                                       \
        if (!(X))                                                           \
        {                                                                   \
            printf ("assert(" GB_STR(X) ") failed: "                        \
                __FILE__ " line %d\n", __LINE__) ;                          \
            GB_Global_abort_function ( ) ;                                  \
        }                                                                   \
    }

    // call a GraphBLAS method and assert that it returns GrB_SUCCESS
    #define ASSERT_OK(X)                                                    \
    {                                                                       \
        GrB_Info Info = (X) ;                                               \
        ASSERT (Info == GrB_SUCCESS) ;                                      \
    }

    // call a GraphBLAS method and assert that it returns GrB_SUCCESS
    // or GrB_NULL_POINTER.
    #define ASSERT_OK_OR_NULL(X)                                            \
    {                                                                       \
        GrB_Info Info = (X) ;                                               \
        ASSERT (Info == GrB_SUCCESS || Info == GrB_NULL_POINTER) ;          \
    }

    // call a GraphBLAS method and assert that it returns GrB_SUCCESS
    // or GrB_INDEX_OUT_OF_BOUNDS.  Used by GB_check(A,...) when the indices
    // in the vectors of A may be jumbled.
    #define ASSERT_OK_OR_JUMBLED(X)                                         \
    {                                                                       \
        GrB_Info Info = (X) ;                                               \
        ASSERT (Info == GrB_SUCCESS || Info == GrB_INDEX_OUT_OF_BOUNDS) ;   \
    }

#else

    // debugging disabled
    #define ASSERT(X)
    #define ASSERT_OK(X)
    #define ASSERT_OK_OR_NULL(X)
    #define ASSERT_OK_OR_JUMBLED(X)

#endif

#define GB_IMPLIES(p,q) (!(p) || (q))

// for finding tests that trigger statement coverage.  If running a test
// in GraphBLAS/Tcov, the test does not terminate.
#ifdef GBTESTCOV
#define GB_GOTCHA                                                   \
{                                                                   \
    fprintf (stderr, "gotcha: " __FILE__ " line: %d\n", __LINE__) ; \
    printf ("gotcha: " __FILE__ " line: %d\n", __LINE__) ;          \
}
#else
#define GB_GOTCHA                                                   \
{                                                                   \
    fprintf (stderr, "gotcha: " __FILE__ " line: %d\n", __LINE__) ; \
    printf ("gotcha: " __FILE__ " line: %d\n", __LINE__) ;          \
    GB_Global_abort_function ( ) ;                                  \
}
#endif

#define GB_HERE printf ("%2d: Here: " __FILE__ " line: %d\n",  \
    GB_OPENMP_THREAD_ID, __LINE__) ;

// ASSERT (GB_DEAD_CODE) marks code that is intentionally dead, leftover from
// prior versions of SuiteSparse:GraphBLAS but no longer used in the current
// version.  The code is kept in case it is required for future versions (in
// which case, the ASSERT (GB_DEAD_CODE) statement would be removed).
#define GB_DEAD_CODE 0

//------------------------------------------------------------------------------
// aliased objects
//------------------------------------------------------------------------------

// GraphBLAS allows all inputs to all user-accessible objects to be aliased, as
// in GrB_mxm (C, C, accum, C, C, ...), which is valid.  Internal routines are
// more restrictive.

// GB_aliased also checks the content of A and B
bool GB_aliased             // determine if A and B are aliased
(
    GrB_Matrix A,           // input A matrix
    GrB_Matrix B            // input B matrix
) ;

//------------------------------------------------------------------------------
// GraphBLAS memory manager
//------------------------------------------------------------------------------

#define GBYTES(n,s)  ((((double) (n)) * ((double) (s))) / 1e9)

//------------------------------------------------------------------------------
// internal GraphBLAS type and operator codes
//------------------------------------------------------------------------------

// GB_MAGIC is an arbitrary number that is placed inside each object when it is
// initialized, as a way of detecting uninitialized objects.
#define GB_MAGIC  0x72657473786f62ULL

// The magic number is set to GB_FREED when the object is freed, as a way of
// helping to detect dangling pointers.
#define GB_FREED  0x6c6c756e786f62ULL

// The value is set to GB_MAGIC2 when the object has been allocated but cannot
// yet be used in most methods and operations.  Currently this is used only for
// when A->p array is allocated but not initialized.
#define GB_MAGIC2 0x7265745f786f62ULL

// predefined type objects
extern struct GB_Type_opaque
    GB_opaque_GrB_BOOL   ,  // GrB_BOOL is a pointer to this object, etc.
    GB_opaque_GrB_INT8   ,
    GB_opaque_GrB_UINT8  ,
    GB_opaque_GrB_INT16  ,
    GB_opaque_GrB_UINT16 ,
    GB_opaque_GrB_INT32  ,
    GB_opaque_GrB_UINT32 ,
    GB_opaque_GrB_INT64  ,
    GB_opaque_GrB_UINT64 ,
    GB_opaque_GrB_FP32   ,
    GB_opaque_GrB_FP64   ;

//------------------------------------------------------------------------------
// monoid structs
//------------------------------------------------------------------------------

extern struct GB_Monoid_opaque

    // MIN monoids:
    GB_opaque_GxB_MIN_INT8_MONOID,          // identity: INT8_MAX
    GB_opaque_GxB_MIN_UINT8_MONOID,         // identity: UINT8_MAX
    GB_opaque_GxB_MIN_INT16_MONOID,         // identity: INT16_MAX
    GB_opaque_GxB_MIN_UINT16_MONOID,        // identity: UINT16_MAX
    GB_opaque_GxB_MIN_INT32_MONOID,         // identity: INT32_MAX
    GB_opaque_GxB_MIN_UINT32_MONOID,        // identity: UINT32_MAX
    GB_opaque_GxB_MIN_INT64_MONOID,         // identity: INT64_MAX
    GB_opaque_GxB_MIN_UINT64_MONOID,        // identity: UINT64_MAX
    GB_opaque_GxB_MIN_FP32_MONOID,          // identity: INFINITY
    GB_opaque_GxB_MIN_FP64_MONOID,          // identity: INFINITY

    // MAX monoids:
    GB_opaque_GxB_MAX_INT8_MONOID,          // identity: INT8_MIN
    GB_opaque_GxB_MAX_UINT8_MONOID,         // identity: 0
    GB_opaque_GxB_MAX_INT16_MONOID,         // identity: INT16_MIN
    GB_opaque_GxB_MAX_UINT16_MONOID,        // identity: 0
    GB_opaque_GxB_MAX_INT32_MONOID,         // identity: INT32_MIN
    GB_opaque_GxB_MAX_UINT32_MONOID,        // identity: 0
    GB_opaque_GxB_MAX_INT64_MONOID,         // identity: INT64_MIN
    GB_opaque_GxB_MAX_UINT64_MONOID,        // identity: 0
    GB_opaque_GxB_MAX_FP32_MONOID,          // identity: -INFINITY
    GB_opaque_GxB_MAX_FP64_MONOID,          // identity: -INFINITY

    // PLUS monoids:
    GB_opaque_GxB_PLUS_INT8_MONOID,         // identity: 0
    GB_opaque_GxB_PLUS_UINT8_MONOID,        // identity: 0
    GB_opaque_GxB_PLUS_INT16_MONOID,        // identity: 0
    GB_opaque_GxB_PLUS_UINT16_MONOID,       // identity: 0
    GB_opaque_GxB_PLUS_INT32_MONOID,        // identity: 0
    GB_opaque_GxB_PLUS_UINT32_MONOID,       // identity: 0
    GB_opaque_GxB_PLUS_INT64_MONOID,        // identity: 0
    GB_opaque_GxB_PLUS_UINT64_MONOID,       // identity: 0
    GB_opaque_GxB_PLUS_FP32_MONOID,         // identity: 0
    GB_opaque_GxB_PLUS_FP64_MONOID,         // identity: 0

    // TIMES monoids:
    GB_opaque_GxB_TIMES_INT8_MONOID,        // identity: 1
    GB_opaque_GxB_TIMES_UINT8_MONOID,       // identity: 1
    GB_opaque_GxB_TIMES_INT16_MONOID,       // identity: 1
    GB_opaque_GxB_TIMES_UINT16_MONOID,      // identity: 1
    GB_opaque_GxB_TIMES_INT32_MONOID,       // identity: 1
    GB_opaque_GxB_TIMES_UINT32_MONOID,      // identity: 1
    GB_opaque_GxB_TIMES_INT64_MONOID,       // identity: 1
    GB_opaque_GxB_TIMES_UINT64_MONOID,      // identity: 1
    GB_opaque_GxB_TIMES_FP32_MONOID,        // identity: 1
    GB_opaque_GxB_TIMES_FP64_MONOID,        // identity: 1

    // Boolean monoids:
    GB_opaque_GxB_LOR_BOOL_MONOID,          // identity: false
    GB_opaque_GxB_LAND_BOOL_MONOID,         // identity: true
    GB_opaque_GxB_LXOR_BOOL_MONOID,         // identity: false
    GB_opaque_GxB_EQ_BOOL_MONOID ;          // identity: true

//------------------------------------------------------------------------------
// select structs
//------------------------------------------------------------------------------

extern struct GB_SelectOp_opaque
    GB_opaque_GxB_TRIL,
    GB_opaque_GxB_TRIU,
    GB_opaque_GxB_DIAG,
    GB_opaque_GxB_OFFDIAG,
    GB_opaque_GxB_NONZERO,
    GB_opaque_GxB_EQ_ZERO,
    GB_opaque_GxB_GT_ZERO,
    GB_opaque_GxB_GE_ZERO,
    GB_opaque_GxB_LT_ZERO,
    GB_opaque_GxB_LE_ZERO,
    GB_opaque_GxB_NE_THUNK,
    GB_opaque_GxB_EQ_THUNK,
    GB_opaque_GxB_GT_THUNK,
    GB_opaque_GxB_GE_THUNK,
    GB_opaque_GxB_LT_THUNK,
    GB_opaque_GxB_LE_THUNK ;

//------------------------------------------------------------------------------
// error logging and parallel thread control
//------------------------------------------------------------------------------

// Error messages are logged in GB_DLEN, on the stack, and then copied into
// thread-local storage of size GB_RLEN.  If the user-defined data types,
// operators, etc have really long names, the error messages are safely
// truncated (via snprintf).  This is intentional, but gcc with
// -Wformat-truncation will print a warning (see pragmas above).  Ignore the
// warning.

// The Context also contains the number of threads to use in the operation.  It
// is normally determined from the user's descriptor, with a default of
// nthreads_max = GxB_DEFAULT (that is, zero).  The default rule is to let
// GraphBLAS determine the number of threads automatically by selecting a
// number of threads between 1 and nthreads_max.  GrB_init initializes
// nthreads_max to omp_get_max_threads.  Both the global value and the value in
// a descriptor can set/queried by GxB_set / GxB_get.

// Some GrB_Matrix and GrB_Vector methods do not take a descriptor, however
// (GrB_*_dup, _build, _exportTuples, _clear, _nvals, _wait, and GxB_*_resize).
// For those methods the default rule is always used (nthreads_max =
// GxB_DEFAULT), which then relies on the global nthreads_max.

#define GB_RLEN 384
#define GB_DLEN 256

typedef struct
{
    double chunk ;              // chunk size for small problems
    int nthreads_max ;          // max # of threads to use
    const char *where ;         // GraphBLAS function where error occurred
    char details [GB_DLEN] ;    // error report
}
GB_Context_struct ;

typedef GB_Context_struct *GB_Context ;

// GB_WHERE keeps track of the currently running user-callable function.
// User-callable functions in this implementation are written so that they do
// not call other unrelated user-callable functions (except for GrB_*free).
// Related user-callable functions can call each other since they all report
// the same type-generic name.  Internal functions can be called by many
// different user-callable functions, directly or indirectly.  It would not be
// helpful to report the name of an internal function that flagged an error
// condition.  Thus, each time a user-callable function is entered (except
// GrB_*free), it logs the name of the function with the GB_WHERE macro.
// GrB_*free does not encounter error conditions so it doesn't need to be
// logged by the GB_WHERE macro.

#ifndef GB_PANIC
#define GB_PANIC return (GrB_PANIC) ;
#endif

#define GB_CONTEXT(where_string)                                    \
    /* construct the Context */                                     \
    GB_Context_struct Context_struct ;                              \
    GB_Context Context = &Context_struct ;                          \
    /* set Context->where so GrB_error can report it if needed */   \
    Context->where = where_string ;                                 \
    /* get the default max # of threads and default chunk size */   \
    Context->nthreads_max = GB_Global_nthreads_max_get ( ) ;        \
    Context->chunk = GB_Global_chunk_get ( ) ;

#define GB_WHERE(where_string)                                      \
    if (!GB_Global_GrB_init_called_get ( ))                         \
    {                                                               \
        /* GrB_init (or GxB_init) has not been called! */           \
        GB_PANIC ;                                                  \
    }                                                               \
    GB_CONTEXT (where_string) ;

//------------------------------------------------------------------------------
// GB_GET_NTHREADS_MAX:  determine max # of threads for OpenMP parallelism.
//------------------------------------------------------------------------------

//      GB_GET_NTHREADS_MAX obtains the max # of threads to use and the chunk
//      size from the Context.  If Context is NULL then a single thread *must*
//      be used.  If Context->nthreads_max is <= GxB_DEFAULT, then select
//      automatically: between 1 and nthreads_max, depending on the problem
//      size.  Below is the default rule.  Any function can use its own rule
//      instead, based on Context, chunk, nthreads_max, and the problem size.
//      No rule can exceed nthreads_max.

#define GB_GET_NTHREADS_MAX(nthreads_max,chunk,Context)                     \
    int nthreads_max = (Context == NULL) ? 1 : Context->nthreads_max ;      \
    if (nthreads_max <= GxB_DEFAULT)                                        \
    {                                                                       \
        nthreads_max = GB_Global_nthreads_max_get ( ) ;                     \
    }                                                                       \
    double chunk = (Context == NULL) ? GxB_DEFAULT : Context->chunk ;       \
    if (chunk <= GxB_DEFAULT)                                               \
    {                                                                       \
        chunk = GB_Global_chunk_get ( ) ;                                   \
    }

//------------------------------------------------------------------------------
// GB_nthreads: determine # of threads to use for a parallel loop or region
//------------------------------------------------------------------------------

// If work < 2*chunk, then only one thread is used.
// else if work < 3*chunk, then two threads are used, and so on.

static inline int GB_nthreads   // return # of threads to use
(
    double work,                // total work to do
    double chunk,               // give each thread at least this much work
    int nthreads_max            // max # of threads to use
)
{
    work  = GB_IMAX (work, 1) ;
    chunk = GB_IMAX (chunk, 1) ;
    int64_t nthreads = (int64_t) floor (work / chunk) ;
    nthreads = GB_IMIN (nthreads, nthreads_max) ;
    nthreads = GB_IMAX (nthreads, 1) ;
    return ((int) nthreads) ;
}

//------------------------------------------------------------------------------
// error logging
//------------------------------------------------------------------------------

// The GB_ERROR and GB_LOG macros work together.  If an error occurs, the
// GB_ERROR macro records the details in the Context.details, and returns the
// GrB_info to its 'caller'.  This value can then be returned, or set to an
// info variable of type GrB_Info.  For example:
//
//  if (i >= nrows)
//  {
//      return (GB_ERROR (GrB_INDEX_OUT_OF_BOUNDS, (GB_LOG,
//          "Row index %d out of bounds; must be < %d", i, nrows))) ;
//  }
//
// The user can then do:
//
//  printf ("%s", GrB_error ( )) ;
//
// To print details of the error, which includes: which user-callable function
// encountered the error, the error status (GrB_INDEX_OUT_OF_BOUNDS), the
// details ("Row index 102 out of bounds, must be < 100").

const char *GB_status_code (GrB_Info info) ;

GrB_Info GB_error           // log an error in thread-local-storage
(
    GrB_Info info,          // error return code from a GraphBLAS function
    GB_Context Context      // pointer to a Context struct, on the stack
) ;

// GB_LOG becomes the snprintf_args for GB_ERROR.  Unused if Context is NULL.
#define GB_LOG Context->details, GB_DLEN

// if Context is NULL, do not log the error string in Context->details
#define GB_ERROR(info,snprintf_args)                                \
(                                                                   \
    ((Context == NULL) ? 0 : snprintf snprintf_args),               \
    GB_error (info, Context)                                        \
)

// return (GB_OUT_OF_MEMORY) ; reports an out-of-memory error
#define GB_OUT_OF_MEMORY GB_ERROR (GrB_OUT_OF_MEMORY, (GB_LOG, "out of memory"))

//------------------------------------------------------------------------------
// GraphBLAS check functions: check and optionally print an object
//------------------------------------------------------------------------------

// pr values for *_check functions
#define GB0 GxB_SILENT
#define GB1 GxB_SUMMARY
#define GB2 GxB_SHORT
#define GB3 GxB_COMPLETE

// a NULL name is treated as the empty string
#define GB_NAME ((name != NULL) ? name : "")

// print to a file f, and check the result
#define GBPR(...)                                                           \
{                                                                           \
    if (f != NULL)                                                          \
    {                                                                       \
        if (fprintf (f, __VA_ARGS__) < 0)                                   \
        {                                                                   \
            int err = errno ;                                               \
            return (GB_ERROR (GrB_INVALID_VALUE, (GB_LOG,                   \
                "File output error (%d): %s", err, strerror (err)))) ;      \
        }                                                                   \
    }                                                                       \
}

#define GBPR0(...)                  \
{                                   \
    if (pr > 0)                     \
    {                               \
        GBPR (__VA_ARGS__) ;        \
    }                               \
}

// check object->magic code
#ifdef GB_DEVELOPER
#define GBPR_MAGIC(pcode)                                               \
{                                                                       \
    char *p = (char *) &(pcode) ;                                       \
    GBPR0 (" pcode: [ %d1 %s ] ", p [0], p) ;                           \
}
#else
#define GBPR_MAGIC(pcode) ;
#endif

// check object->magic and print an error if invalid
#define GB_CHECK_MAGIC(object,kind)                                     \
{                                                                       \
    switch (object->magic)                                              \
    {                                                                   \
        case GB_MAGIC :                                                 \
            /* the object is valid */                                   \
            GBPR_MAGIC (object->magic) ;                                \
            break ;                                                     \
                                                                        \
        case GB_FREED :                                                 \
            /* dangling pointer! */                                     \
            GBPR_MAGIC (object->magic) ;                                \
            GBPR0 ("already freed!\n") ;                                \
            return (GB_ERROR (GrB_UNINITIALIZED_OBJECT, (GB_LOG,        \
                "%s is freed: [%s]", kind, name))) ;                    \
                                                                        \
        case GB_MAGIC2 :                                                \
            /* invalid */                                               \
            GBPR_MAGIC (object->magic) ;                                \
            GBPR0 ("invalid\n") ;                                       \
            return (GB_ERROR (GrB_INVALID_OBJECT, (GB_LOG,              \
                "%s is invalid: [%s]", kind, name))) ;                  \
                                                                        \
        default :                                                       \
            /* uninitialized */                                         \
            GBPR0 ("uninititialized\n") ;                               \
            return (GB_ERROR (GrB_UNINITIALIZED_OBJECT, (GB_LOG,        \
                "%s is uninitialized: [%s]", kind, name))) ;            \
    }                                                                   \
}

GrB_Info GB_entry_check     // print a single value
(
    const GrB_Type type,    // type of value to print
    const void *x,          // value to print
    FILE *f,                // file to print to
    GB_Context Context
) ;

GrB_Info GB_code_check          // print and check an entry using a type code
(
    const GB_Type_code code,    // type code of value to print
    const void *x,              // entry to print
    FILE *f,                    // file to print to
    GB_Context Context
) ;

GrB_Info GB_Type_check      // check a GraphBLAS Type
(
    const GrB_Type type,    // GraphBLAS type to print and check
    const char *name,       // name of the type from the caller; optional
    int pr,                 // 0: print nothing, 1: print header and errors,
                            // 2: print brief, 3: print all
    FILE *f,                // file for output
    GB_Context Context
) ;

GrB_Info GB_BinaryOp_check  // check a GraphBLAS binary operator
(
    const GrB_BinaryOp op,  // GraphBLAS operator to print and check
    const char *name,       // name of the operator
    int pr,                 // 0: print nothing, 1: print header and errors,
                            // 2: print brief, 3: print all
    FILE *f,                // file for output
    GB_Context Context
) ;

GrB_Info GB_UnaryOp_check   // check a GraphBLAS unary operator
(
    const GrB_UnaryOp op,   // GraphBLAS operator to print and check
    const char *name,       // name of the operator
    int pr,                 // 0: print nothing, 1: print header and errors,
                            // 2: print brief, 3: print all
    FILE *f,                // file for output
    GB_Context Context
) ;

GrB_Info GB_SelectOp_check  // check a GraphBLAS select operator
(
    const GxB_SelectOp op,  // GraphBLAS operator to print and check
    const char *name,       // name of the operator
    int pr,                 // 0: print nothing, 1: print header and errors,
                            // 2: print brief, 3: print all
    FILE *f,                // file for output
    GB_Context Context
) ;

GrB_Info GB_Monoid_check        // check a GraphBLAS monoid
(
    const GrB_Monoid monoid,    // GraphBLAS monoid to print and check
    const char *name,           // name of the monoid, optional
    int pr,                     // 0: print nothing, 1: print header and errors,
                                // 2: print brief, 3: print all
    FILE *f,                    // file for output
    GB_Context Context
) ;

GrB_Info GB_Semiring_check          // check a GraphBLAS semiring
(
    const GrB_Semiring semiring,    // GraphBLAS semiring to print and check
    const char *name,               // name of the semiring, optional
    int pr,                         // 0: print nothing, 1: print header and
                                    // errors, 2: print brief, 3: print all
    FILE *f,                        // file for output
    GB_Context Context
) ;

GrB_Info GB_Descriptor_check    // check a GraphBLAS descriptor
(
    const GrB_Descriptor D,     // GraphBLAS descriptor to print and check
    const char *name,           // name of the descriptor, optional
    int pr,                     // 0: print nothing, 1: print header and
                                // errors, 2: print brief, 3: print all
    FILE *f,                    // file for output
    GB_Context Context
) ;

GrB_Info GB_matvec_check    // check a GraphBLAS matrix or vector
(
    const GrB_Matrix A,     // GraphBLAS matrix to print and check
    const char *name,       // name of the matrix, optional
    int pr,                 // 0: print nothing, 1: print header and errors,
                            // 2: print brief, 3: print all
                            // if negative, ignore queue conditions
                            // and use GB_FLIP(pr) for diagnostic printing.
    FILE *f,                // file for output
    const char *kind,       // "matrix" or "vector"
    GB_Context Context
) ;

GrB_Info GB_Matrix_check    // check a GraphBLAS matrix
(
    const GrB_Matrix A,     // GraphBLAS matrix to print and check
    const char *name,       // name of the matrix
    int pr,                 // 0: print nothing, 1: print header and errors,
                            // 2: print brief, 3: print all
    FILE *f,                // file for output
    GB_Context Context
) ;

GrB_Info GB_Vector_check    // check a GraphBLAS vector
(
    const GrB_Vector v,     // GraphBLAS vector to print and check
    const char *name,       // name of the vector
    int pr,                 // 0: print nothing, 1: print header and errors,
                            // 2: print brief, 3: print all
    FILE *f,                // file for output
    GB_Context Context
) ;

GrB_Info GB_Scalar_check    // check a GraphBLAS GxB_Scalar
(
    const GxB_Scalar v,     // GraphBLAS GxB_Scalar to print and check
    const char *name,       // name of the GxB_Scalar
    int pr,                 // 0: print nothing, 1: print header and errors,
                            // 2: print brief, 3: print all
    FILE *f,                // file for output
    GB_Context Context
) ;

#define GB_check(x,name,pr)                             \
    _Generic                                            \
    (                                                   \
        (x),                                            \
        const GrB_Type       : GB_Type_check       ,    \
              GrB_Type       : GB_Type_check       ,    \
        const GrB_BinaryOp   : GB_BinaryOp_check   ,    \
              GrB_BinaryOp   : GB_BinaryOp_check   ,    \
        const GxB_SelectOp   : GB_SelectOp_check   ,    \
              GxB_SelectOp   : GB_SelectOp_check   ,    \
        const GrB_UnaryOp    : GB_UnaryOp_check    ,    \
              GrB_UnaryOp    : GB_UnaryOp_check    ,    \
        const GrB_Monoid     : GB_Monoid_check     ,    \
              GrB_Monoid     : GB_Monoid_check     ,    \
        const GrB_Semiring   : GB_Semiring_check   ,    \
              GrB_Semiring   : GB_Semiring_check   ,    \
        const GrB_Matrix     : GB_Matrix_check     ,    \
              GrB_Matrix     : GB_Matrix_check     ,    \
        const GrB_Vector     : GB_Vector_check     ,    \
              GrB_Vector     : GB_Vector_check     ,    \
        const GxB_Scalar     : GB_Scalar_check     ,    \
              GxB_Scalar     : GB_Scalar_check     ,    \
        const GrB_Descriptor : GB_Descriptor_check ,    \
              GrB_Descriptor : GB_Descriptor_check      \
    ) (x, name, pr, stdout, Context)

//------------------------------------------------------------------------------
// internal GraphBLAS functions
//------------------------------------------------------------------------------

GrB_Info GB_init            // start up GraphBLAS
(
    const GrB_Mode mode,    // blocking or non-blocking mode

    // pointers to memory management functions.  Must be non-NULL.
    void * (* malloc_function  ) (size_t),
    void * (* calloc_function  ) (size_t, size_t),
    void * (* realloc_function ) (void *, size_t),
    void   (* free_function    ) (void *),
    bool malloc_is_thread_safe,

    GB_Context Context      // from GrB_init or GxB_init
) ;

typedef enum                    // input parameter to GB_new and GB_create
{
    GB_Ap_calloc,               // 0: calloc A->p, malloc A->h if hypersparse
    GB_Ap_malloc,               // 1: malloc A->p, malloc A->h if hypersparse
    GB_Ap_null                  // 2: do not allocate A->p or A->h
}
GB_Ap_code ;

GrB_Info GB_new                 // create matrix, except for indices & values
(
    GrB_Matrix *Ahandle,        // handle of matrix to create
    const GrB_Type type,        // matrix type
    const int64_t vlen,         // length of each vector
    const int64_t vdim,         // number of vectors
    const GB_Ap_code Ap_option, // allocate A->p and A->h, or leave NULL
    const bool is_csc,          // true if CSC, false if CSR
    const int hyper_option,     // 1:hyper, 0:nonhyper, -1:auto
    const double hyper_ratio,   // A->hyper_ratio, unless auto
    const int64_t plen,         // size of A->p and A->h, if A hypersparse.
                                // Ignored if A is not hypersparse.
    GB_Context Context
) ;

GrB_Info GB_create              // create a new matrix, including A->i and A->x
(
    GrB_Matrix *Ahandle,        // output matrix to create
    const GrB_Type type,        // type of output matrix
    const int64_t vlen,         // length of each vector
    const int64_t vdim,         // number of vectors
    const GB_Ap_code Ap_option, // allocate A->p and A->h, or leave NULL
    const bool is_csc,          // true if CSC, false if CSR
    const int hyper_option,     // 1:hyper, 0:nonhyper, -1:auto
    const double hyper_ratio,   // A->hyper_ratio, unless auto
    const int64_t plen,         // size of A->p and A->h, if hypersparse
    const int64_t anz,          // number of nonzeros the matrix must hold
    const bool numeric,         // if true, allocate A->x, else A->x is NULL
    GB_Context Context
) ;

GrB_Info GB_hyper_realloc
(
    GrB_Matrix A,               // matrix with hyperlist to reallocate
    int64_t plen_new,           // new size of A->p and A->h
    GB_Context Context
) ;

GrB_Info GB_clear           // clear a matrix, type and dimensions unchanged
(
    GrB_Matrix A,           // matrix to clear
    GB_Context Context
) ;

GrB_Info GB_dup             // make an exact copy of a matrix
(
    GrB_Matrix *Chandle,    // handle of output matrix to create
    const GrB_Matrix A,     // input matrix to copy
    const bool numeric,     // if true, duplicate the numeric values
    const GrB_Type ctype,   // type of C, if numeric is false
    GB_Context Context
) ;

void GB_memcpy                  // parallel memcpy
(
    void *dest,                 // destination
    const void *src,            // source
    size_t n,                   // # of bytes to copy
    int nthreads                // # of threads to use
) ;

GrB_Info GB_nvals           // get the number of entries in a matrix
(
    GrB_Index *nvals,       // matrix has nvals entries
    const GrB_Matrix A,     // matrix to query
    GB_Context Context
) ;

GrB_Info GB_type            // get the type of a matrix
(
    GrB_Type *type,         // returns the type of the matrix
    const GrB_Matrix A,     // matrix to query
    GB_Context Context
) ;

GrB_Info GB_ix_alloc        // allocate A->i and A->x space in a matrix
(
    GrB_Matrix A,           // matrix to allocate space for
    const GrB_Index nzmax,  // number of entries the matrix can hold
    const bool numeric,     // if true, allocate A->x, otherwise A->x is NULL
    GB_Context Context
) ;

GrB_Info GB_ix_realloc      // reallocate space in a matrix
(
    GrB_Matrix A,           // matrix to allocate space for
    const GrB_Index nzmax,  // new number of entries the matrix can hold
    const bool numeric,     // if true, reallocate A->x, otherwise A->x is NULL
    GB_Context Context
) ;

GrB_Info GB_ix_resize           // resize a matrix
(
    GrB_Matrix A,
    const int64_t anz_new,      // required new nnz(A)
    GB_Context Context
) ;

// free A->i and A->x and return if critical section fails
#define GB_IX_FREE(A)                                                       \
{                                                                           \
    if (GB_ix_free (A) == GrB_PANIC) GB_PANIC ;                             \
}

GrB_Info GB_ix_free             // free A->i and A->x of a matrix
(
    GrB_Matrix A                // matrix with content to free
) ;

void GB_ph_free                 // free A->p and A->h of a matrix
(
    GrB_Matrix A                // matrix with content to free
) ;

// free all content, and return if critical section fails
#define GB_PHIX_FREE(A)                                                     \
{                                                                           \
    if (GB_phix_free (A) == GrB_PANIC) GB_PANIC ;                           \
}

GrB_Info GB_phix_free           // free all content of a matrix
(
    GrB_Matrix A                // matrix with content to free
) ;

GrB_Info GB_free                // free a matrix
(
    GrB_Matrix *matrix_handle   // handle of matrix to free
) ;

bool GB_Type_compatible         // check if two types can be typecast
(
    const GrB_Type atype,
    const GrB_Type btype
) ;

bool GB_code_compatible         // check if two types can be typecast
(
    const GB_Type_code acode,
    const GB_Type_code bcode
) ;

void GB_cast_array              // typecast an array
(
    GB_void *restrict Cx,       // output array
    const GB_Type_code code1,   // type code for Cx
    const GB_void *restrict Ax, // input array
    const GB_Type_code code2,   // type code for Ax
    const int64_t anz,          // number of entries in Cx and Ax
    GB_Context Context
) ;

GB_cast_function GB_cast_factory   // returns pointer to function to cast x to z
(
    const GB_Type_code code1,      // the type of z, the output value
    const GB_Type_code code2       // the type of x, the input value
) ;

void GB_copy_user_user (void *z, const void *x, size_t s) ;

//------------------------------------------------------------------------------
// GB_task_struct: parallel task descriptor
//------------------------------------------------------------------------------

// The element-wise computations (GB_add, GB_emult, and GB_mask) compute
// C(:,j)<M(:,j)> = op (A (:,j), B(:,j)).  They are parallelized by slicing the
// work into tasks, described by the GB_task_struct.

// There are two kinds of tasks.  For a coarse task, kfirst <= klast, and the
// task computes all vectors in C(:,kfirst:klast), inclusive.  None of the
// vectors are sliced and computed by other tasks.  For a fine task, klast is
// -1.  The task computes part of the single vector C(:,kfirst).  It starts at
// pA in Ai,Ax, at pB in Bi,Bx, and (if M is present) at pM in Mi,Mx.  It
// computes C(:,kfirst), starting at pC in Ci,Cx.

// GB_subref also uses the TaskList.  It has 12 kinds of fine tasks,
// corresponding to each of the 12 methods used in GB_subref_template.  For
// those fine tasks, method = -TaskList [taskid].klast defines the method to
// use.

// The GB_subassign functions use the TaskList, in many different ways.

typedef struct          // task descriptor
{
    int64_t kfirst ;    // C(:,kfirst) is the first vector in this task.
    int64_t klast  ;    // C(:,klast) is the last vector in this task.
    int64_t pC ;        // fine task starts at Ci, Cx [pC]
    int64_t pC_end ;    // fine task ends at Ci, Cx [pC_end-1]
    int64_t pM ;        // fine task starts at Mi, Mx [pM]
    int64_t pM_end ;    // fine task ends at Mi, Mx [pM_end-1]
    int64_t pA ;        // fine task starts at Ai, Ax [pA]
    int64_t pA_end ;    // fine task ends at Ai, Ax [pA_end-1]
    int64_t pB ;        // fine task starts at Bi, Bx [pB]
    int64_t pB_end ;    // fine task ends at Bi, Bx [pB_end-1]
    int64_t len ;       // fine task handles a subvector of this length
}
GB_task_struct ;

// GB_REALLOC_TASK_LIST: Allocate or reallocate the TaskList so that it can
// hold at least ntasks.  Double the size if it's too small.

#define GB_REALLOC_TASK_LIST(TaskList,ntasks,max_ntasks)                \
{                                                                       \
    if ((ntasks) >= max_ntasks)                                         \
    {                                                                   \
        bool ok ;                                                       \
        int nold = (max_ntasks == 0) ? 0 : (max_ntasks + 1) ;           \
        int nnew = 2 * (ntasks) + 1 ;                                   \
        GB_REALLOC_MEMORY (TaskList, nnew, nold,                        \
            sizeof (GB_task_struct), &ok) ;                             \
        if (!ok)                                                        \
        {                                                               \
            /* out of memory */                                         \
            GB_FREE_ALL ;                                               \
            return (GB_OUT_OF_MEMORY) ;                                 \
        }                                                               \
        for (int t = nold ; t < nnew ; t++)                             \
        {                                                               \
            TaskList [t].kfirst = -1 ;                                  \
            TaskList [t].klast  = INT64_MIN ;                           \
            TaskList [t].pA     = INT64_MIN ;                           \
            TaskList [t].pA_end = INT64_MIN ;                           \
            TaskList [t].pB     = INT64_MIN ;                           \
            TaskList [t].pB_end = INT64_MIN ;                           \
            TaskList [t].pC     = INT64_MIN ;                           \
            TaskList [t].pC_end = INT64_MIN ;                           \
            TaskList [t].pM     = INT64_MIN ;                           \
            TaskList [t].pM_end = INT64_MIN ;                           \
            TaskList [t].len    = INT64_MIN ;                           \
        }                                                               \
        max_ntasks = 2 * (ntasks) ;                                     \
    }                                                                   \
    ASSERT ((ntasks) < max_ntasks) ;                                    \
}

GrB_Info GB_ewise_slice
(
    // output:
    GB_task_struct **p_TaskList,    // array of structs, of size max_ntasks
    int *p_max_ntasks,              // size of TaskList
    int *p_ntasks,                  // # of tasks constructed
    int *p_nthreads,                // # of threads to use
    // input:
    const int64_t Cnvec,            // # of vectors of C
    const int64_t *restrict Ch,     // vectors of C, if hypersparse
    const int64_t *restrict C_to_M, // mapping of C to M
    const int64_t *restrict C_to_A, // mapping of C to A
    const int64_t *restrict C_to_B, // mapping of C to B
    bool Ch_is_Mh,                  // if true, then Ch == Mh; GB_add only
    const GrB_Matrix M,             // mask matrix to slice (optional)
    const GrB_Matrix A,             // matrix to slice
    const GrB_Matrix B,             // matrix to slice
    GB_Context Context
) ;

void GB_slice_vector
(
    // output: return i, pA, and pB
    int64_t *p_i,                   // work starts at A(i,kA) and B(i,kB)
    int64_t *p_pM,                  // M(i:end,kM) starts at pM
    int64_t *p_pA,                  // A(i:end,kA) starts at pA
    int64_t *p_pB,                  // B(i:end,kB) starts at pB
    // input:
    const int64_t pM_start,         // M(:,kM) starts at pM_start in Mi,Mx
    const int64_t pM_end,           // M(:,kM) ends at pM_end-1 in Mi,Mx
    const int64_t *restrict Mi,     // indices of M (or NULL)
    const int64_t pA_start,         // A(:,kA) starts at pA_start in Ai,Ax
    const int64_t pA_end,           // A(:,kA) ends at pA_end-1 in Ai,Ax
    const int64_t *restrict Ai,     // indices of A
    const int64_t A_hfirst,         // if Ai is an implicit hyperlist
    const int64_t pB_start,         // B(:,kB) starts at pB_start in Bi,Bx
    const int64_t pB_end,           // B(:,kB) ends at pB_end-1 in Bi,Bx
    const int64_t *restrict Bi,     // indices of B
    const int64_t vlen,             // A->vlen and B->vlen
    const double target_work        // target work
) ;

void GB_task_cumsum
(
    int64_t *Cp,                        // size Cnvec+1
    const int64_t Cnvec,
    int64_t *Cnvec_nonempty,            // # of non-empty vectors in C
    GB_task_struct *restrict TaskList,  // array of structs
    const int ntasks,                   // # of tasks
    const int nthreads                  // # of threads
) ;

//------------------------------------------------------------------------------
// GB_GET_VECTOR: get the content of a vector for a coarse/fine task
//------------------------------------------------------------------------------

#define GB_GET_VECTOR(pX_start, pX_fini, pX, pX_end, Xp, kX)                \
    int64_t pX_start, pX_fini ;                                             \
    if (fine_task)                                                          \
    {                                                                       \
        /* A fine task operates on a slice of X(:,k) */                     \
        pX_start = TaskList [taskid].pX ;                                   \
        pX_fini  = TaskList [taskid].pX_end ;                               \
    }                                                                       \
    else                                                                    \
    {                                                                       \
        /* vectors are never sliced for a coarse task */                    \
        pX_start = Xp [kX] ;                                                \
        pX_fini  = Xp [kX+1] ;                                              \
    }

//------------------------------------------------------------------------------

GrB_Info GB_transplant          // transplant one matrix into another
(
    GrB_Matrix C,               // output matrix to overwrite with A
    const GrB_Type ctype,       // new type of C
    GrB_Matrix *Ahandle,        // input matrix to copy from and free
    GB_Context Context
) ;

GrB_Info GB_transplant_conform      // transplant and conform hypersparsity
(
    GrB_Matrix C,                   // destination matrix to transplant into
    GrB_Type ctype,                 // type to cast into
    GrB_Matrix *Thandle,            // source matrix
    GB_Context Context
) ;

size_t GB_code_size             // return the size of a type, given its code
(
    const GB_Type_code code,    // input code of the type to find the size of
    const size_t usize          // known size of user-defined type
) ;

void *GB_calloc_memory      // pointer to allocated block of memory
(
    size_t nitems,          // number of items to allocate
    size_t size_of_item     // sizeof each item
) ;

void *GB_malloc_memory      // pointer to allocated block of memory
(
    size_t nitems,          // number of items to allocate
    size_t size_of_item     // sizeof each item
) ;

void *GB_realloc_memory     // pointer to reallocated block of memory, or
                            // to original block if the realloc failed.
(
    size_t nitems_new,      // new number of items in the object
    size_t nitems_old,      // old number of items in the object
    size_t size_of_item,    // sizeof each item
    void *p,                // old object to reallocate
    bool *ok                // true if successful, false otherwise
) ;

void GB_free_memory
(
    void *p,                // pointer to allocated block of memory to free
    size_t nitems,          // number of items to free
    size_t size_of_item     // sizeof each item
) ;

//------------------------------------------------------------------------------
// macros to create/free matrices, vectors, and generic memory
//------------------------------------------------------------------------------

// if GB_PRINT_MALLOC is defined, these macros print diagnostic
// information, meant for development of SuiteSparse:GraphBLAS only

#ifdef GB_PRINT_MALLOC

#define GB_NEW(A,type,vlen,vdim,Ap_option,is_csc,hopt,h,plen,Context)         \
{                                                                             \
    printf ("\nmatrix new:                   %s = new (%s, vlen = "GBd        \
        ", vdim = "GBd", Ap:%d, csc:%d, hyper:%d %g, plen:"GBd")"             \
        " line %d file %s\n", GB_STR(A), GB_STR(type),                        \
        (int64_t) vlen, (int64_t) vdim, Ap_option, is_csc, hopt, h,           \
        (int64_t) plen, __LINE__, __FILE__) ;                                 \
    info = GB_new (A, type, vlen, vdim, Ap_option, is_csc, hopt, h, plen,     \
        Context) ;                                                            \
}

#define GB_CREATE(A,type,vlen,vdim,Ap_option,is_csc,hopt,h,plen,anz,numeric,Context)  \
{                                                                             \
    printf ("\nmatrix create:                %s = new (%s, vlen = "GBd        \
        ", vdim = "GBd", Ap:%d, csc:%d, hyper:%d %g, plen:"GBd", anz:"GBd     \
        " numeric:%d) line %d file %s\n", GB_STR(A), GB_STR(type),            \
        (int64_t) vlen, (int64_t) vdim, Ap_option, is_csc, hopt, h,           \
        (int64_t) plen, (int64_t) anz, numeric, __LINE__, __FILE__) ;         \
    info = GB_create (A, type, vlen, vdim, Ap_option, is_csc, hopt, h, plen,  \
        anz, numeric, Context) ;                                              \
}

#define GB_MATRIX_FREE(A)                                                     \
{                                                                             \
    if (A != NULL && *(A) != NULL)                                            \
        printf ("\nmatrix free:                  "                            \
        "matrix_free (%s) line %d file %s\n", GB_STR(A), __LINE__, __FILE__) ;\
    if (GB_free (A) == GrB_PANIC) GB_PANIC ;                                  \
}

#define GB_VECTOR_FREE(v)                                                     \
{                                                                             \
    if (v != NULL && *(v) != NULL)                                            \
        printf ("\nvector free:                  "                            \
        "vector_free (%s) line %d file %s\n", GB_STR(v), __LINE__, __FILE__) ;\
    if (GB_free ((GrB_Matrix *) v) == GrB_PANIC) GB_PANIC ;                   \
}

#define GB_SCALAR_FREE(v)                                                     \
{                                                                             \
    if (v != NULL && *(v) != NULL)                                            \
        printf ("\nscalar free:                  "                            \
        "scalar_free (%s) line %d file %s\n", GB_STR(v), __LINE__, __FILE__) ;\
    if (GB_free ((GrB_Matrix *) v) == GrB_PANIC) GB_PANIC ;                   \
}

#define GB_CALLOC_MEMORY(p,n,s)                                               \
    printf ("\nCalloc:                       "                                \
    "%s = calloc (%s = "GBd", %s = "GBd") line %d file %s\n",                 \
    GB_STR(p), GB_STR(n), (int64_t) n, GB_STR(s), (int64_t) s,                \
    __LINE__,__FILE__) ;                                                      \
    p = GB_calloc_memory (n, s) ;

#define GB_MALLOC_MEMORY(p,n,s)                                               \
    printf ("\nMalloc:                       "                                \
    "%s = malloc (%s = "GBd", %s = "GBd") line %d file %s\n",                 \
    GB_STR(p), GB_STR(n), (int64_t) n, GB_STR(s), (int64_t) s,                \
    __LINE__,__FILE__) ;                                                      \
    p = GB_malloc_memory (n, s) ;

#define GB_REALLOC_MEMORY(p,nnew,nold,s,ok)                                    \
{                                                                             \
    printf ("\nRealloc: %14p       "                                          \
    "%s = realloc (%s = "GBd", %s = "GBd", %s = "GBd") line %d file %s\n",    \
    p, GB_STR(p), GB_STR(nnew), (int64_t) nnew, GB_STR(nold), (int64_t) nold, \
    GB_STR(s), (int64_t) s, __LINE__,__FILE__) ;                              \
    p = GB_realloc_memory (nnew, nold, s, (void *) p, ok) ;                   \
}

#define GB_FREE_MEMORY(p,n,s)                                                 \
{                                                                             \
    if (p)                                                                    \
    printf ("\nFree:               "                                          \
    "(%s, %s = "GBd", %s = "GBd") line %d file %s\n",                         \
    GB_STR(p), GB_STR(n), (int64_t) n, GB_STR(s), (int64_t) s,                \
    __LINE__,__FILE__) ;                                                      \
    GB_free_memory ((void *) p, n, s) ;                                       \
    (p) = NULL ;                                                              \
}

#else

#define GB_NEW(A,type,vlen,vdim,Ap_option,is_csc,hopt,h,plen,Context)         \
{                                                                             \
    info = GB_new (A, type, vlen, vdim, Ap_option, is_csc, hopt, h, plen,     \
        Context) ;                                                            \
}

#define GB_CREATE(A,type,vlen,vdim,Ap_option,is_csc,hopt,h,plen,anz,numeric,Context)  \
{                                                                             \
    info = GB_create (A, type, vlen, vdim, Ap_option, is_csc, hopt, h, plen,  \
        anz, numeric, Context) ;                                              \
}

#define GB_MATRIX_FREE(A)                                                     \
{                                                                             \
    if (GB_free (A) == GrB_PANIC) GB_PANIC ;                                  \
}

#define GB_VECTOR_FREE(v) GB_MATRIX_FREE ((GrB_Matrix *) v)

#define GB_SCALAR_FREE(v) GB_MATRIX_FREE ((GrB_Matrix *) v)

#define GB_CALLOC_MEMORY(p,n,s)                                               \
    p = GB_calloc_memory (n, s) ;

#define GB_MALLOC_MEMORY(p,n,s)                                               \
    p = GB_malloc_memory (n, s) ;

#define GB_REALLOC_MEMORY(p,nnew,nold,s,ok)                                   \
    p = GB_realloc_memory (nnew, nold, s, (void *) p, ok) ;

#define GB_FREE_MEMORY(p,n,s)                                                 \
{                                                                             \
    GB_free_memory ((void *) p, n, s) ;                                       \
    (p) = NULL ;                                                              \
}

#endif

//------------------------------------------------------------------------------

GrB_Type GB_code_type           // return the GrB_Type corresponding to the code
(
    const GB_Type_code code,    // type code to convert
    const GrB_Type type         // user type if code is GB_UDT_code
) ;

// used in GB_AxB_heap for temporary workspace
typedef struct
{
    int64_t start ;                 // first entry of A(:,k) is at Ai [start]
    int64_t end ;                   // last entry of A(:,k) is at Ai [end-1]
}
GB_pointer_pair ;

// used in GB_heap_*
typedef struct
{
    int64_t key ;       // the key for this element, for ordering in the Heap
    int64_t name ;      // the name of the element; not used in these functions
                        // but required by the caller
}
GB_Element ;

GrB_Info GB_slice       // slice B into nthreads slices or hyperslices
(
    GrB_Matrix B,       // matrix to slice
    int nthreads,       // # of slices to create
    int64_t *Slice,     // array of size nthreads+1 that defines the slice
    GrB_Matrix *Bslice, // array of output slices, of size nthreads
    GB_Context Context
) ;

void GB_pslice                      // find how to slice Ap
(
    int64_t *Slice,                 // size ntasks+1
    const int64_t *restrict Ap,     // array of size n+1
    const int64_t n,
    const int ntasks                // # of tasks
) ;

void GB_eslice
(
    // output:
    int64_t *Slice,         // array of size ntasks+1
    // input:
    int64_t e,              // number items to partition amongst the tasks
    const int ntasks        // # of tasks
) ;

bool GB_binop_builtin               // true if binary operator is builtin
(
    // inputs:
    const GrB_Matrix A,
    const bool A_is_pattern,        // true if only the pattern of A is used
    const GrB_Matrix B,
    const bool B_is_pattern,        // true if only the pattern of B is used
    const GrB_BinaryOp op,          // binary operator
    const bool flipxy,              // true if z=op(y,x), flipping x and y
    // outputs, unused by caller if this function returns false
    GB_Opcode *opcode,              // opcode for the binary operator
    GB_Type_code *xycode,           // type code for x and y inputs
    GB_Type_code *zcode             // type code for z output
) ;

void GB_cumsum                  // compute the cumulative sum of an array
(
    int64_t *restrict count,    // size n+1, input/output
    const int64_t n,
    int64_t *restrict kresult,  // return k, if needed by the caller
    int nthreads
) ;

GrB_Info GB_Descriptor_get      // get the contents of a descriptor
(
    const GrB_Descriptor desc,  // descriptor to query, may be NULL
    bool *C_replace,            // if true replace C before C<M>=Z
    bool *Mask_comp,            // if true use logical negation of M
    bool *In0_transpose,        // if true transpose first input
    bool *In1_transpose,        // if true transpose second input
    GrB_Desc_Value *AxB_method, // method for C=A*B
    GB_Context Context
) ;

GrB_Info GB_compatible          // SUCCESS if all is OK, *_MISMATCH otherwise
(
    const GrB_Type ctype,       // the type of C (matrix or scalar)
    const GrB_Matrix C,         // the output matrix C; NULL if C is a scalar
    const GrB_Matrix M,         // optional mask, NULL if no mask
    const GrB_BinaryOp accum,   // C<M> = accum(C,T) is computed
    const GrB_Type ttype,       // type of T
    GB_Context Context
) ;

GrB_Info GB_Mask_compatible     // check type and dimensions of mask
(
    const GrB_Matrix M,         // mask to check
    const GrB_Matrix C,         // C<M>= ...
    const GrB_Index nrows,      // size of output if C is NULL (see GB*assign)
    const GrB_Index ncols,
    GB_Context Context
) ;

GrB_Info GB_BinaryOp_compatible     // check for domain mismatch
(
    const GrB_BinaryOp op,          // binary operator to check
    const GrB_Type ctype,           // C must be compatible with op->ztype
    const GrB_Type atype,           // A must be compatible with op->xtype
    const GrB_Type btype,           // B must be compatible with op->ytype
    const GB_Type_code bcode,       // B may not have a type, just a code
    GB_Context Context
) ;

// Several methods can use choose between a qsort-based method that takes
// O(anz*log(anz)) time, or a bucket-sort method that takes O(anz+n) time.
// The qsort method is choosen if the following condition is true:
#define GB_CHOOSE_QSORT_INSTEAD_OF_BUCKET(anz,n) ((16 * (anz)) < (n))

GB_Opcode GB_boolean_rename     // renamed opcode
(
    const GB_Opcode opcode      // opcode to rename
) ;

bool GB_Index_multiply      // true if ok, false if overflow
(
    GrB_Index *c,           // c = a*b, or zero if overflow occurs
    const int64_t a,
    const int64_t b
) ;

bool GB_size_t_multiply     // true if ok, false if overflow
(
    size_t *c,              // c = a*b, or zero if overflow occurs
    const size_t a,
    const size_t b
) ;

void GB_extract_vector_list
(
    // output:
    int64_t *restrict J,        // size nnz(A) or more
    // input
    const GrB_Matrix A,
    int nthreads
) ;

GrB_Info GB_extractTuples       // extract all tuples from a matrix
(
    GrB_Index *I_out,           // array for returning row indices of tuples
    GrB_Index *J_out,           // array for returning col indices of tuples
    void *X,                    // array for returning values of tuples
    GrB_Index *p_nvals,         // I,J,X size on input; # tuples on output
    const GB_Type_code xcode,   // type of array X
    const GrB_Matrix A,         // matrix to extract tuples from
    GB_Context Context
) ;

GrB_Info GB_extractElement      // extract a single entry, x = A(row,col)
(
    void *x,                    // scalar to extract, not modified if not found
    const GB_Type_code xcode,   // type of the scalar x
    const GrB_Matrix A,         // matrix to extract a scalar from
    const GrB_Index row,        // row index
    const GrB_Index col,        // column index
    GB_Context Context
) ;

GrB_Info GB_Monoid_new          // create a monoid
(
    GrB_Monoid *monoid,         // handle of monoid to create
    GrB_BinaryOp op,            // binary operator of the monoid
    const void *identity,       // identity value
    const void *terminal,       // terminal value, if any (may be NULL)
    const GB_Type_code idcode,  // identity and terminal type code
    GB_Context Context
) ;

GrB_Info GB_wait                // finish all pending computations
(
    GrB_Matrix A,               // matrix with pending computations
    GB_Context Context
) ;

//------------------------------------------------------------------------------
// OpenMP definitions
//------------------------------------------------------------------------------

// GB_PART and GB_PARTITION:  divide the index range 0:n-1 uniformly
// for nthreads.  GB_PART(tid,n,nthreads) is the first index for thread tid.
#define GB_PART(tid,n,nthreads)  \
    (((tid) * ((double) (n))) / ((double) (nthreads)))

// thread tid will operate on the range k1:(k2-1)
#define GB_PARTITION(k1,k2,n,tid,nthreads)                                  \
    k1 = ((tid) ==  0          ) ?  0  : GB_PART ((tid),  n, nthreads) ;    \
    k2 = ((tid) == (nthreads)-1) ? (n) : GB_PART ((tid)+1,n, nthreads) ;


#if defined ( _OPENMP )

    #include <omp.h>
    #define GB_OPENMP_THREAD_ID         omp_get_thread_num ( )
    #define GB_OPENMP_MAX_THREADS       omp_get_max_threads ( )
    #define GB_OPENMP_GET_NUM_THREADS   omp_get_num_threads ( )

#else

    #define GB_OPENMP_THREAD_ID         (0)
    #define GB_OPENMP_MAX_THREADS       (1)
    #define GB_OPENMP_GET_NUM_THREADS   (1)

#endif

// by default, give each thread at least 4096 units of work to do
#define GB_CHUNK_DEFAULT 4096

//------------------------------------------------------------------------------
// GB_queue operations
//------------------------------------------------------------------------------

// GB_queue_* can fail if the critical section fails.  This is an unrecoverable
// error, so return a panic if it fails.  All GB_queue_* operations are used
// via the GB_CRITICAL macro.  GrB_init uses GB_CRITICAL as well.

// GB_CRITICAL: GB_queue_* inside a critical section, which 'cannot' fail
#define GB_CRITICAL(op) if (!(op)) GB_PANIC ;

bool GB_queue_remove            // remove matrix from queue
(
    GrB_Matrix A                // matrix to remove
) ;

bool GB_queue_insert            // insert matrix at the head of queue
(
    GrB_Matrix A                // matrix to insert
) ;

bool GB_queue_remove_head       // remove matrix at the head of queue
(
    GrB_Matrix *Ahandle         // return matrix or NULL if queue empty
) ;

bool GB_queue_status            // get the queue status of a matrix
(
    GrB_Matrix A,               // matrix to check
    GrB_Matrix *p_head,         // head of the queue
    GrB_Matrix *p_prev,         // prev from A
    GrB_Matrix *p_next,         // next after A
    bool *p_enqd                // true if A is in the queue
) ;

//------------------------------------------------------------------------------

GrB_Info GB_setElement              // set a single entry, C(row,col) = scalar
(
    GrB_Matrix C,                   // matrix to modify
    const void *scalar,             // scalar to set
    const GrB_Index row,            // row index
    const GrB_Index col,            // column index
    const GB_Type_code scalar_code, // type of the scalar
    GB_Context Context
) ;

GrB_Info GB_block   // apply all pending computations if blocking mode enabled
(
    GrB_Matrix A,
    GB_Context Context
) ;

bool GB_op_is_second    // return true if op is SECOND, of the right type
(
    GrB_BinaryOp op,
    GrB_Type type
) ;


//------------------------------------------------------------------------------

char *GB_code_string            // return a static string for a type name
(
    const GB_Type_code code     // code to convert to string
) ;

GrB_Info GB_resize              // change the size of a matrix
(
    GrB_Matrix A,               // matrix to modify
    const GrB_Index nrows_new,  // new number of rows in matrix
    const GrB_Index ncols_new,  // new number of columns in matrix
    GB_Context Context
) ;

int64_t GB_nvec_nonempty        // return # of non-empty vectors
(
    const GrB_Matrix A,         // input matrix to examine
    GB_Context Context
) ;

GrB_Info GB_to_nonhyper     // convert a matrix to non-hypersparse
(
    GrB_Matrix A,           // matrix to convert to non-hypersparse
    GB_Context Context
) ;

GrB_Info GB_to_hyper        // convert a matrix to hypersparse
(
    GrB_Matrix A,           // matrix to convert to hypersparse
    GB_Context Context
) ;

bool GB_to_nonhyper_test    // test for conversion to hypersparse
(
    GrB_Matrix A,           // matrix to test
    int64_t k,              // # of non-empty vectors of A, an estimate is OK,
                            // but normally A->nvec_nonempty
    int64_t vdim            // normally A->vdim
) ;

bool GB_to_hyper_test       // test for conversion to hypersparse
(
    GrB_Matrix A,           // matrix to test
    int64_t k,              // # of non-empty vectors of A, an estimate is OK,
                            // but normally A->nvec_nonempty
    int64_t vdim            // normally A->vdim
) ;

GrB_Info GB_to_hyper_conform    // conform a matrix to its desired format
(
    GrB_Matrix A,               // matrix to conform
    GB_Context Context
) ;

GrB_Info GB_hyper_prune
(
    // output, not allocated on input:
    int64_t *restrict *p_Ap,        // size nvec+1
    int64_t *restrict *p_Ah,        // size nvec
    int64_t *p_nvec,                // # of vectors, all nonempty
    // input, not modified
    const int64_t *Ap_old,          // size nvec_old+1
    const int64_t *Ah_old,          // size nvec_old
    const int64_t nvec_old,         // original number of vectors
    GB_Context Context
) ;

//------------------------------------------------------------------------------
// critical section for user threads
//------------------------------------------------------------------------------

// User-level threads may call GraphBLAS in parallel, so the access to the
// global queue for GrB_wait must be protected by a critical section.  The
// critical section method should match the user threading model.

#if defined (USER_POSIX_THREADS)
// for user applications that use POSIX pthreads
extern pthread_mutex_t GB_sync ;

#elif defined (USER_WINDOWS_THREADS)
// for user applications that use Windows threads (not yet supported)
extern CRITICAL_SECTION GB_sync ;

#elif defined (USER_ANSI_THREADS)
// for user applications that use ANSI C11 threads (not yet supported)
extern mtx_t GB_sync ;

#else // USER_OPENMP_THREADS, or USER_NO_THREADS
// nothing to do for OpenMP, or for no user threading

#endif

//------------------------------------------------------------------------------
// boiler plate macros for checking inputs and returning if an error occurs
//------------------------------------------------------------------------------

// Functions use these macros to check/get their inputs and return an error
// if something has gone wrong.

#define GB_OK(method)                       \
{                                           \
    info = method ;                         \
    if (info != GrB_SUCCESS)                \
    {                                       \
        GB_FREE_ALL ;                       \
        return (info) ;                     \
    }                                       \
}

// check if a required arg is NULL
#define GB_RETURN_IF_NULL(arg)                                          \
    if ((arg) == NULL)                                                  \
    {                                                                   \
        /* the required arg is NULL */                                  \
        return (GB_ERROR (GrB_NULL_POINTER, (GB_LOG,                    \
            "Required argument is null: [%s]", GB_STR(arg)))) ;         \
    }

// arg may be NULL, but if non-NULL then it must be initialized
#define GB_RETURN_IF_FAULTY(arg)                                        \
    if ((arg) != NULL && (arg)->magic != GB_MAGIC)                      \
    {                                                                   \
        if ((arg)->magic == GB_MAGIC2)                                  \
        {                                                               \
            /* optional arg is not NULL, but invalid */                 \
            return (GB_ERROR (GrB_INVALID_OBJECT, (GB_LOG,              \
                "Argument is invalid: [%s]", GB_STR(arg)))) ;           \
        }                                                               \
        else                                                            \
        {                                                               \
            /* optional arg is not NULL, but not initialized */         \
            return (GB_ERROR (GrB_UNINITIALIZED_OBJECT, (GB_LOG,        \
                "Argument is uninitialized: [%s]", GB_STR(arg)))) ;     \
        }                                                               \
    }

// arg must not be NULL, and it must be initialized
#define GB_RETURN_IF_NULL_OR_FAULTY(arg)                                \
    GB_RETURN_IF_NULL (arg) ;                                           \
    GB_RETURN_IF_FAULTY (arg) ;

// check the descriptor and extract its contents; also copies
// nthreads_max and chunk from the descriptor to the Context
#define GB_GET_DESCRIPTOR(info,desc,dout,dm,d0,d1,dalgo)                     \
    GrB_Info info ;                                                          \
    bool dout, dm, d0, d1 ;                                                  \
    GrB_Desc_Value dalgo ;                                                   \
    /* if desc is NULL then defaults are used.  This is OK */                \
    info = GB_Descriptor_get (desc, &dout, &dm, &d0, &d1, &dalgo, Context) ; \
    if (info != GrB_SUCCESS)                                                 \
    {                                                                        \
        /* desc not NULL, but uninitialized or an invalid object */          \
        return (info) ;                                                      \
    }

// C<M>=Z ignores Z if an empty mask is complemented, so return from
// the method without computing anything.  But do apply the mask.
#define GB_RETURN_IF_QUICK_MASK(C, C_replace, M, Mask_comp)             \
    if (Mask_comp && M == NULL)                                         \
    {                                                                   \
        /* C<!NULL>=NULL since result does not depend on computing Z */ \
        return (C_replace ? GB_clear (C, Context) : GrB_SUCCESS) ;      \
    }

// GB_MASK_VERY_SPARSE is true if C<M>=A+B or C<M>=accum(C,T) is being
// computed, and the mask M is very sparse compared with A and B.
#define GB_MASK_VERY_SPARSE(M,A,B) (8 * GB_NNZ (M) < GB_NNZ (A) + GB_NNZ (B))

//------------------------------------------------------------------------------
// Pending upddate and zombies
//------------------------------------------------------------------------------

// GB_FLIP is a kind of  "negation" about (-1) of a zero-based index.
// If i >= 0 then it is not flipped.
// If i < 0 then it has been flipped.
// Like negation, GB_FLIP is its own inverse: GB_FLIP (GB_FLIP (i)) == i.
// The "nil" value, -1, doesn't change when flipped: GB_FLIP (-1) = -1.
// GB_UNFLIP(i) is like taking an absolute value, undoing any GB_FLIP(i).

// An entry A(i,j) in a matrix can be marked as a "zombie".  A zombie is an
// entry that has been marked for deletion, but hasn't been deleted yet because
// it's more efficient to delete all zombies all at once, instead of one at a
// time.  Zombies are created by submatrix assignment, C(I,J)=A which copies
// not only new entries into C, but it also deletes entries already present in
// C.  If an entry appears in A but not C(I,J), it is a new entry; new entries
// placed in the pending tuple lists to be added later.  If an entry appear in
// C(I,J) but NOT in A, then it is marked for deletion by flipping its row
// index, marking it as a zombie.

// Zombies can be restored as regular entries by GrB_*assign.  If an assignment
// C(I,J)=A finds an entry in A that is a zombie in C, the zombie becomes a
// regular entry, taking on the value from A.  The row index is unflipped.

// Zombies are deleted and pending tuples are added into the matrix all at
// once, by GB_wait.

#define GB_FLIP(i)             (-(i)-2)
#define GB_IS_FLIPPED(i)       ((i) < 0)
#define GB_IS_ZOMBIE(i)        ((i) < 0)
#define GB_IS_NOT_FLIPPED(i)   ((i) >= 0)
#define GB_IS_NOT_ZOMBIE(i)    ((i) >= 0)
#define GB_UNFLIP(i)           (((i) < 0) ? GB_FLIP(i) : (i))

// true if a matrix has pending tuples
#define GB_PENDING(A) ((A) != NULL && (A)->Pending != NULL)

// true if a matrix is allowed to have pending tuples
#define GB_PENDING_OK(A) (GB_PENDING (A) || !GB_PENDING (A))

// true if a matrix has zombies
#define GB_ZOMBIES(A) ((A) != NULL && (A)->nzombies > 0)

// true if a matrix is allowed to have zombies
#define GB_ZOMBIES_OK(A) (((A) == NULL) || ((A) != NULL && (A)->nzombies >= 0))

// true if a matrix has pending tuples or zombies
#define GB_PENDING_OR_ZOMBIES(A) (GB_PENDING (A) || GB_ZOMBIES (A))

// do all pending updates:  delete zombies and assemble any pending tuples
#define GB_WAIT_MATRIX(A)                                               \
{                                                                       \
    GrB_Info info = GB_wait ((GrB_Matrix) A, Context) ;                 \
    if (info != GrB_SUCCESS)                                            \
    {                                                                   \
        /* out of memory; no other error possible */                    \
        ASSERT (info == GrB_OUT_OF_MEMORY) ;                            \
        return (info) ;                                                 \
    }                                                                   \
    ASSERT (!GB_ZOMBIES (A)) ;                                          \
    ASSERT (!GB_PENDING (A)) ;                                          \
}

// wait for any pending operations: both pending tuples and zombies
#define GB_WAIT(A)                                                      \
{                                                                       \
    if (GB_PENDING_OR_ZOMBIES (A)) GB_WAIT_MATRIX (A) ;                 \
}

// just wait for pending tuples; zombies are OK but removed anyway if the
// matrix also has pending tuples.  They are left in place if the matrix has
// zombies but no pending tuples.
#define GB_WAIT_PENDING(A)                                              \
{                                                                       \
    if (GB_PENDING (A)) GB_WAIT_MATRIX (A) ;                            \
    ASSERT (GB_ZOMBIES_OK (A)) ;                                        \
}

// true if a matrix has no entries; zombies OK
#define GB_EMPTY(A) ((GB_NNZ (A) == 0) && !GB_PENDING (A))

//------------------------------------------------------------------------------
// division by zero
//------------------------------------------------------------------------------

// Integer division by zero is done the same way it's done in MATLAB.  This
// approach allows GraphBLAS to not terminate the user's application on
// divide-by-zero, and allows GraphBLAS results to be tested against MATLAB.
// To compute X/0: if X is zero, the result is zero (like NaN).  if X is
// negative, the result is the negative integer with biggest magnitude (like
// -infinity).  if X is positive, the result is the biggest positive integer
// (like +infinity).

// For places affected by this decision in the code do:
// grep "integer division"

// Signed and unsigned integer division, z = x/y.  The bits parameter can be 8,
// 16, 32, or 64.
#define GB_INT_MIN(bits)  INT ## bits ## _MIN
#define GB_INT_MAX(bits)  INT ## bits ## _MAX
#define GB_UINT_MAX(bits) UINT ## bits ## _MAX

// x/y when x and y are signed integers
#define GB_IDIV_SIGNED(x,y,bits)                                            \
(                                                                           \
    ((y) == -1) ?                                                           \
    (                                                                       \
        /* INT32_MIN/(-1) causes floating point exception; avoid it  */     \
        -(x)                                                                \
    )                                                                       \
    :                                                                       \
    (                                                                       \
        ((y) == 0) ?                                                        \
        (                                                                   \
            /* x/0 */                                                       \
            ((x) == 0) ?                                                    \
            (                                                               \
                /* zero divided by zero gives 'Nan' */                      \
                0                                                           \
            )                                                               \
            :                                                               \
            (                                                               \
                /* x/0 and x is nonzero */                                  \
                ((x) < 0) ?                                                 \
                (                                                           \
                    /* x is negative: x/0 gives '-Inf' */                   \
                    GB_INT_MIN (bits)                                       \
                )                                                           \
                :                                                           \
                (                                                           \
                    /* x is positive: x/0 gives '+Inf' */                   \
                    GB_INT_MAX (bits)                                       \
                )                                                           \
            )                                                               \
        )                                                                   \
        :                                                                   \
        (                                                                   \
            /* normal case for signed integer division */                   \
            (x) / (y)                                                       \
        )                                                                   \
    )                                                                       \
)

// x/y when x and y are unsigned integers
#define GB_IDIV_UNSIGNED(x,y,bits)                                          \
(                                                                           \
    ((y) == 0) ?                                                            \
    (                                                                       \
        /* x/0 */                                                           \
        ((x) == 0) ?                                                        \
        (                                                                   \
            /* zero divided by zero gives 'Nan' */                          \
            0                                                               \
        )                                                                   \
        :                                                                   \
        (                                                                   \
            /* x is positive: x/0 gives '+Inf' */                           \
            GB_UINT_MAX (bits)                                              \
        )                                                                   \
    )                                                                       \
    :                                                                       \
    (                                                                       \
        /* normal case for unsigned integer division */                     \
        (x) / (y)                                                           \
    )                                                                       \
)

// 1/y when y is a signed integer
#define GB_IMINV_SIGNED(y,bits)                                             \
(                                                                           \
    ((y) == -1) ?                                                           \
    (                                                                       \
        -1                                                                  \
    )                                                                       \
    :                                                                       \
    (                                                                       \
        ((y) == 0) ?                                                        \
        (                                                                   \
            GB_INT_MAX (bits)                                               \
        )                                                                   \
        :                                                                   \
        (                                                                   \
            ((y) == 1) ?                                                    \
            (                                                               \
                1                                                           \
            )                                                               \
            :                                                               \
            (                                                               \
                0                                                           \
            )                                                               \
        )                                                                   \
    )                                                                       \
)

// 1/y when y is an unsigned integer
#define GB_IMINV_UNSIGNED(y,bits)                                           \
(                                                                           \
    ((y) == 0) ?                                                            \
    (                                                                       \
        GB_UINT_MAX (bits)                                                  \
    )                                                                       \
    :                                                                       \
    (                                                                       \
        ((y) == 1) ?                                                        \
        (                                                                   \
            1                                                               \
        )                                                                   \
        :                                                                   \
        (                                                                   \
            0                                                               \
        )                                                                   \
    )                                                                       \
)                                                                           \

// GraphBLAS includes a built-in GrB_DIV_BOOL operator, so boolean division
// must be defined.  There is no MATLAB equivalent since x/y for logical x and
// y is not permitted in MATLAB.  ANSI C11 does not provide a definition
// either, and dividing by zero (boolean false) will typically terminate an
// application.  In this GraphBLAS implementation, boolean division is treated
// as if it were int1, where 1/1 = 1, 0/1 = 0, 0/0 = integer NaN = 0, 1/0 =
// +infinity = 1.  Thus Z=X/Y is Z=X.  This is arbitrary, but it allows all
// operators to work on all types without causing run time exceptions.  It also
// means that GrB_DIV(x,y) is the same as GrB_FIRST(x,y) for boolean x and y.
// See for example GB_boolean_rename and Template/GB_ops_template.c.
// Similarly, GrB_MINV_BOOL, which is 1/x, is simply 'true' for all x.

//------------------------------------------------------------------------------
// typecasting
//------------------------------------------------------------------------------

// The ANSI C11 language specification states that results are undefined when
// typecasting a float or double to an integer value that is outside the range
// of the integer type.  However, most implementations provide a reasonable and
// repeatable result using modular arithmetic, just like conversions between
// integer types, so this is not changed here.  MATLAB uses a different
// strategy; any value outside the range is maxed out the largest or smallest
// integer.  Users of a C library would not expect this behavior.  They would
// expect instead whatever their C compiler would do in this case, even though
// it is technically undefined by the C11 standard.  Thus, this implementation
// of GraphBLAS does not attempt to second-guess the compiler when converting
// large floating-point values to integers.

// However, Inf's and NaN's are very unpredictable.  The same C compiler can
// generate different results with different optimization levels.  Only bool is
// defined by the ANSI C11 standard (NaN converts to true, since Nan != 0 is
// true).

// This unpredictability with Inf's and NaN's causes the GraphBLAS tests to
// fail in unpredictable ways.  Therefore, in this implementation of GraphBLAS,
// a float or double +Inf is converted to the largest integer, -Inf to the
// smallest integer, and NaN to zero.  This is the same behavior as MATLAB.

// typecast a float or double x to a signed integer z
#define GB_CAST_SIGNED(z,x,bits)                                            \
{                                                                           \
    switch (fpclassify (x))                                                 \
    {                                                                       \
        case FP_NAN:                                                        \
            z = 0 ;                                                         \
            break ;                                                         \
        case FP_INFINITE:                                                   \
            z = ((x) > 0) ? GB_INT_MAX (bits) : GB_INT_MIN (bits) ;         \
            break ;                                                         \
        default:                                                            \
            z = (x) ;                                                       \
            break ;                                                         \
    }                                                                       \
}

// typecast a float or double x to a unsigned integer z
#define GB_CAST_UNSIGNED(z,x,bits)                                          \
{                                                                           \
    switch (fpclassify (x))                                                 \
    {                                                                       \
        case FP_NAN:                                                        \
            z = 0 ;                                                         \
            break ;                                                         \
        case FP_INFINITE:                                                   \
            z = ((x) > 0) ? GB_UINT_MAX (bits) : 0 ;                        \
            break ;                                                         \
        default:                                                            \
            z = (x) ;                                                       \
            break ;                                                         \
    }                                                                       \
}

//------------------------------------------------------------------------------
// GB_BINARY_SEARCH
//------------------------------------------------------------------------------

// search for integer i in the list X [pleft...pright]; no zombies.
// The list X [pleft ... pright] is in ascending order.  It may have
// duplicates.

#define GB_BINARY_TRIM_SEARCH(i,X,pleft,pright)                             \
{                                                                           \
    /* binary search of X [pleft ... pright] for integer i */               \
    while (pleft < pright)                                                  \
    {                                                                       \
        int64_t pmiddle = (pleft + pright) / 2 ;                            \
        if (X [pmiddle] < i)                                                \
        {                                                                   \
            /* if in the list, it appears in [pmiddle+1..pright] */         \
            pleft = pmiddle + 1 ;                                           \
        }                                                                   \
        else                                                                \
        {                                                                   \
            /* if in the list, it appears in [pleft..pmiddle] */            \
            pright = pmiddle ;                                              \
        }                                                                   \
    }                                                                       \
    /* binary search is narrowed down to a single item */                   \
    /* or it has found the list is empty */                                 \
    ASSERT (pleft == pright || pleft == pright + 1) ;                       \
}

// GB_BINARY_SEARCH:
// If found is true then X [pleft == pright] == i.  If duplicates appear then
// X [pleft] is any one of the entries with value i in the list.
// If found is false then
//    X [original_pleft ... pleft-1] < i and
//    X [pleft+1 ... original_pright] > i holds.
// The value X [pleft] may be either < or > i.
#define GB_BINARY_SEARCH(i,X,pleft,pright,found)                            \
{                                                                           \
    GB_BINARY_TRIM_SEARCH (i, X, pleft, pright) ;                           \
    found = (pleft == pright && X [pleft] == i) ;                           \
}

// GB_BINARY_SPLIT_SEARCH
// If found is true then X [pleft] == i.  If duplicates appear then X [pleft]
//    is any one of the entries with value i in the list.
// If found is false then
//    X [original_pleft ... pleft-1] < i and
//    X [pleft ... original_pright] > i holds, and pleft-1 == pright
// If X has no duplicates, then whether or not i is found,
//    X [original_pleft ... pleft-1] < i and
//    X [pleft ... original_pright] >= i holds.
#define GB_BINARY_SPLIT_SEARCH(i,X,pleft,pright,found)                      \
{                                                                           \
    GB_BINARY_SEARCH (i, X, pleft, pright, found)                           \
    if (!found && (pleft == pright))                                        \
    {                                                                       \
        if (i > X [pleft])                                                  \
        {                                                                   \
            pleft++ ;                                                       \
        }                                                                   \
        else                                                                \
        {                                                                   \
            pright++ ;                                                      \
        }                                                                   \
    }                                                                       \
}

//------------------------------------------------------------------------------
// GB_BINARY_ZOMBIE
//------------------------------------------------------------------------------

#define GB_BINARY_TRIM_ZOMBIE(i,X,pleft,pright)                             \
{                                                                           \
    /* binary search of X [pleft ... pright] for integer i */               \
    while (pleft < pright)                                                  \
    {                                                                       \
        int64_t pmiddle = (pleft + pright) / 2 ;                            \
        if (i > GB_UNFLIP (X [pmiddle]))                                    \
        {                                                                   \
            /* if in the list, it appears in [pmiddle+1..pright] */         \
            pleft = pmiddle + 1 ;                                           \
        }                                                                   \
        else                                                                \
        {                                                                   \
            /* if in the list, it appears in [pleft..pmiddle] */            \
            pright = pmiddle ;                                              \
        }                                                                   \
    }                                                                       \
    /* binary search is narrowed down to a single item */                   \
    /* or it has found the list is empty */                                 \
    ASSERT (pleft == pright || pleft == pright + 1) ;                       \
}

#define GB_BINARY_ZOMBIE(i,X,pleft,pright,found,nzombies,is_zombie)         \
{                                                                           \
    if (nzombies > 0)                                                       \
    {                                                                       \
        GB_BINARY_TRIM_ZOMBIE (i, X, pleft, pright) ;                       \
        found = false ;                                                     \
        is_zombie = false ;                                                 \
        if (pleft == pright)                                                \
        {                                                                   \
            int64_t i2 = X [pleft] ;                                        \
            is_zombie = GB_IS_ZOMBIE (i2) ;                                 \
            if (is_zombie)                                                  \
            {                                                               \
                i2 = GB_FLIP (i2) ;                                         \
            }                                                               \
            found = (i == i2) ;                                             \
        }                                                                   \
    }                                                                       \
    else                                                                    \
    {                                                                       \
        is_zombie = false ;                                                 \
        GB_BINARY_SEARCH(i,X,pleft,pright,found)                            \
    }                                                                       \
}

#define GB_BINARY_SPLIT_ZOMBIE(i,X,pleft,pright,found,nzombies,is_zombie)   \
{                                                                           \
    if (nzombies > 0)                                                       \
    {                                                                       \
        GB_BINARY_TRIM_ZOMBIE (i, X, pleft, pright) ;                       \
        found = false ;                                                     \
        is_zombie = false ;                                                 \
        if (pleft == pright)                                                \
        {                                                                   \
            int64_t i2 = X [pleft] ;                                        \
            is_zombie = GB_IS_ZOMBIE (i2) ;                                 \
            if (is_zombie)                                                  \
            {                                                               \
                i2 = GB_FLIP (i2) ;                                         \
            }                                                               \
            found = (i == i2) ;                                             \
            if (!found)                                                     \
            {                                                               \
                if (i > i2)                                                 \
                {                                                           \
                    pleft++ ;                                               \
                }                                                           \
                else                                                        \
                {                                                           \
                    pright++ ;                                              \
                }                                                           \
            }                                                               \
        }                                                                   \
    }                                                                       \
    else                                                                    \
    {                                                                       \
        is_zombie = false ;                                                 \
        GB_BINARY_SPLIT_SEARCH(i,X,pleft,pright,found)                      \
    }                                                                       \
}

//------------------------------------------------------------------------------
// GB_lookup: find k so that j == Ah [k]
//------------------------------------------------------------------------------

// Given a sparse, hypersparse, or hyperslice matrix, find k so that j == Ah
// [k], if it appears in the list.  k is not needed by the caller, just the
// variables pstart, pend, pleft, and found.  GB_lookup cannot be used if
// A is a slice (it could be extended to handle this case).

static inline bool GB_lookup        // find j = Ah [k] in a hyperlist
(
    const bool A_is_hyper,          // true if A is hypersparse
    const int64_t *restrict Ah,     // A->h [0..A->nvec-1]: list of vectors
    const int64_t *restrict Ap,     // A->p [0..A->nvec  ]: pointers to vectors
    int64_t *restrict pleft,        // look only in A->h [pleft..pright]
    int64_t pright,                 // normally A->nvec-1, but can be trimmed
//  const int64_t nvec,             // A->nvec: number of vectors
    const int64_t j,                // vector to find, as j = Ah [k]
    int64_t *restrict pstart,       // start of vector: Ap [k]
    int64_t *restrict pend          // end of vector: Ap [k+1]
)
{
    if (A_is_hyper)
    {
        // to search the whole Ah list, use on input:
        // pleft = 0 ; pright = nvec-1 ;
        bool found ;
        GB_BINARY_SEARCH (j, Ah, (*pleft), pright, found) ;
        if (found)
        { 
            // j appears in the hyperlist at Ah [pleft]
            // k = (*pleft)
            (*pstart) = Ap [(*pleft)] ;
            (*pend)   = Ap [(*pleft)+1] ;
        }
        else
        { 
            // j does not appear in the hyperlist Ah
            // k = -1
            (*pstart) = -1 ;
            (*pend)   = -1 ;
        }
        return (found) ;
    }
    else
    { 
        // A is not hypersparse; j always appears
        // k = j
        (*pstart) = Ap [j] ;
        (*pend)   = Ap [j+1] ;
        return (true) ;
    }
}



#define GB_PRAGMA(x) _Pragma (#x)

#define GB_PRAGMA_SIMD GB_PRAGMA (omp simd)

//------------------------------------------------------------------------------
// built-in unary and binary operators
//------------------------------------------------------------------------------

#define GB_TYPE             bool
#define GB_BOOLEAN
#define GB(x)               GB_ ## x ## _BOOL
#define GB_CAST_NAME(x)     GB_cast_bool_ ## x
#define GB_BITS             1
#include "GB_ops_template.h"

#define GB_TYPE             int8_t
#define GB(x)               GB_ ## x ## _INT8
#define GB_CAST_NAME(x)     GB_cast_int8_t_ ## x
#define GB_BITS             8
#include "GB_ops_template.h"

#define GB_TYPE             uint8_t
#define GB_UNSIGNED
#define GB(x)               GB_ ## x ## _UINT8
#define GB_CAST_NAME(x)     GB_cast_uint8_t_ ## x
#define GB_BITS             8
#include "GB_ops_template.h"

#define GB_TYPE             int16_t
#define GB(x)               GB_ ## x ## _INT16
#define GB_CAST_NAME(x)     GB_cast_int16_t_ ## x
#define GB_BITS             16
#include "GB_ops_template.h"

#define GB_TYPE             uint16_t
#define GB_UNSIGNED
#define GB(x)               GB_ ## x ## _UINT16
#define GB_CAST_NAME(x)     GB_cast_uint16_t_ ## x
#define GB_BITS             16
#include "GB_ops_template.h"

#define GB_TYPE             int32_t
#define GB(x)               GB_ ## x ## _INT32
#define GB_CAST_NAME(x)     GB_cast_int32_t_ ## x
#define GB_BITS             32
#include "GB_ops_template.h"

#define GB_TYPE             uint32_t
#define GB_UNSIGNED
#define GB(x)               GB_ ## x ## _UINT32
#define GB_CAST_NAME(x)     GB_cast_uint32_t_ ## x
#define GB_BITS             32
#include "GB_ops_template.h"

#define GB_TYPE             int64_t
#define GB(x)               GB_ ## x ## _INT64
#define GB_CAST_NAME(x)     GB_cast_int64_t_ ## x
#define GB_BITS             64
#include "GB_ops_template.h"

#define GB_TYPE             uint64_t
#define GB_UNSIGNED
#define GB(x)               GB_ ## x ## _UINT64
#define GB_CAST_NAME(x)     GB_cast_uint64_t_ ## x
#define GB_BITS             64
#include "GB_ops_template.h"

#define GB_TYPE             float
#define GB_FLOATING_POINT
#define GB(x)               GB_ ## x ## _FP32
#define GB_CAST_NAME(x)     GB_cast_float_ ## x
#define GB_BITS             32
#include "GB_ops_template.h"

#define GB_TYPE             double
#define GB_FLOATING_POINT
#define GB_FLOATING_POINT_DOUBLE
#define GB(x)               GB_ ## x ## _FP64
#define GB_CAST_NAME(x)     GB_cast_double_ ## x
#define GB_BITS             64
#include "GB_ops_template.h"

#define GB_opaque_GrB_LNOT GB_opaque_GxB_LNOT_BOOL
#define GB_opaque_GrB_LOR  GB_opaque_GxB_LOR_BOOL
#define GB_opaque_GrB_LAND GB_opaque_GxB_LAND_BOOL
#define GB_opaque_GrB_LXOR GB_opaque_GxB_LXOR_BOOL

#endif

