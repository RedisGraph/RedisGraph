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
// the method for multithreading in the user application must be known:
//------------------------------------------------------------------------------

// set these via cmake, or uncomment to select the user-thread model:

// #define USER_POSIX_THREADS
// #define USER_OPENMP_THREADS
// #define USER_NO_THREADS

// These are in draft status, and not yet implemented:
// #define USER_WINDOWS_THREADS         not yet supported
// #define USER_ANSI_THREADS            not yet supported

//------------------------------------------------------------------------------
// manage compiler warnings
//------------------------------------------------------------------------------

// These warnings are spurious.  SuiteSparse:GraphBLAS uses many template-style
// code generation mechanisms, and these can generate unused results that an
// optimizing compiler can safely discard as dead code.

#if defined __INTEL_COMPILER
// disable icc warnings
//  58:   sign compare
//  167:  incompatible pointer
//  144:  initialize with incompatible pointer
//  177:  declared but unused
//  181:  format
//  186:  useless comparison
//  188:  mixing enum types
//  589:  bypass initialization
//  593:  set but not used
//  869:  unused parameters
//  981:  unspecified order
//  1418: no external declaration
//  1419: external declaration in source file
//  1572: floating point comparisons
//  1599: shadow
//  2259: typecasting
//  2282: unrecognized pragma
//  2557: sign compare
//  2547: remark about include files
//  3280: shadow
#pragma warning (disable: 58 167 144 177 181 186 188 589 593 869 981 1418 1419 1572 1599 2259 2282 2557 2547 3280 )

#elif defined __GNUC__

#pragma GCC diagnostic ignored "-Wunknown-pragmas"
#pragma GCC diagnostic ignored "-Wunknown-warning-option"
#pragma GCC diagnostic ignored "-Wformat-truncation="
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-result"
#pragma GCC diagnostic ignored "-Wint-in-bool-context"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wtype-limits"
#pragma GCC diagnostic ignored "-Wincompatible-pointer-types"
#pragma GCC diagnostic ignored "-Wdiscarded-qualifiers"

// enable these warnings as errors
#pragma GCC diagnostic error "-Wmisleading-indentation"
#pragma GCC diagnostic error "-Wswitch-default"

#endif

//------------------------------------------------------------------------------
// include GraphBLAS.h (depends on user threading model)
//------------------------------------------------------------------------------

#include "GraphBLAS.h"

//------------------------------------------------------------------------------
// code development settings
//------------------------------------------------------------------------------

// turn off debugging; do not edit these three lines
#ifndef NDEBUG
#define NDEBUG
#endif

// These flags are used for code development.  Uncomment them as needed.

// to turn on debugging, uncomment this line:
// #undef NDEBUG

// to turn on memory usage debug printing, uncomment this line:
// #define GB_PRINT_MALLOC 1

// to reduce code size and for faster time to compile, uncomment this line;
// GraphBLAS will be slower:
// #define GBCOMPACT 1

// uncomment this for code development (additional diagnostics are printed):
// #define GB_DEVELOPER 1

// for coverage tests
#ifdef GBCOVER
#define GBCOVER_MAX 10000
extern int64_t GB_cov [GBCOVER_MAX] ;
extern int GB_cover_max ;
#endif

//------------------------------------------------------------------------------
// internal typedefs, not visible at all to the GraphBLAS user
//------------------------------------------------------------------------------

typedef unsigned char GB_void ;

typedef void (*GB_cast_function)   (void *, void *, size_t) ;

#define GB_LEN 128

struct GB_Sauna_struct      // sparse accumulator
{
    int64_t Sauna_hiwater ; // Sauna_Mark [0..Sauna_n-1] < hiwater holds when
                            // the Sauna_Mark is clear.
    int64_t Sauna_n ;       // size of Sauna_Mark and Sauna_Work
    int64_t *Sauna_Mark ;   // array of size Sauna_n
    void    *Sauna_Work ;   // array of size Sauna_n, each entry Sauna_size
    size_t Sauna_size ;     // size of each entry in Sauna_Work
} ;

typedef struct GB_Sauna_struct *GB_Sauna ;

//------------------------------------------------------------------------------
// opaque content of GraphBLAS objects
//------------------------------------------------------------------------------

struct GB_Type_opaque       // content of GrB_Type
{
    int64_t magic ;         // for detecting uninitialized objects
    size_t size ;           // size of the type
    int code ;              // the type code
    char name [GB_LEN] ;    // name of the type
} ;

struct GB_UnaryOp_opaque    // content of GrB_UnaryOp
{
    int64_t magic ;         // for detecting uninitialized objects
    GrB_Type xtype ;        // type of x
    GrB_Type ztype ;        // type of z
    GxB_unary_function function ;        // a pointer to the unary function
    char name [GB_LEN] ;    // name of the unary operator
    int opcode ;            // operator opcode
} ;

struct GB_BinaryOp_opaque   // content of GrB_BinaryOp
{
    int64_t magic ;         // for detecting uninitialized objects
    GrB_Type xtype ;        // type of x
    GrB_Type ytype ;        // type of y
    GrB_Type ztype ;        // type of z
    GxB_binary_function function ;        // a pointer to the binary function
    char name [GB_LEN] ;    // name of the binary operator
    int opcode ;            // operator opcode
} ;

struct GB_SelectOp_opaque   // content of GxB_SelectOp
{
    int64_t magic ;         // for detecting uninitialized objects
    GrB_Type xtype ;        // type of x, or NULL if generic
    GxB_select_function function ;        // a pointer to the select function
    char name [GB_LEN] ;    // name of the select operator
    int opcode ;            // operator opcode
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
    int nthreads ;          // # threads to use in this call to GraphBLAS
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
    ((v)->h == NULL) &&             \
    ((v)->j_pending == NULL)        \
)                                   \

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
// debugging definitions
//------------------------------------------------------------------------------

#undef ASSERT
#undef ASSERT_OK
#undef ASSERT_OK_OR_NULL
#undef ASSERT_OK_OR_JUMBLED

#ifndef NDEBUG

    #define ASSERT(x)                                                       \
    {                                                                       \
        if (!(x))                                                           \
        {                                                                   \
            printf ("assertion failed: " __FILE__ " line %d\n", __LINE__) ; \
            printf ("[%s]\n", GB_STR (x)) ;                                 \
            GB_Global_abort_function_call ( ) ;                             \
        }                                                                   \
    }

    #define ASSERT_OK(X)                                                    \
    {                                                                       \
        GrB_Info Info = X ;                                                 \
        ASSERT (Info == GrB_SUCCESS) ;                                      \
    }

    #define ASSERT_OK_OR_NULL(X)                                            \
    {                                                                       \
        GrB_Info Info = X ;                                                 \
        ASSERT (Info == GrB_SUCCESS || Info == GrB_NULL_POINTER) ;          \
    }

    #define ASSERT_OK_OR_JUMBLED(X)                                         \
    {                                                                       \
        GrB_Info Info = X ;                                                 \
        ASSERT (Info == GrB_SUCCESS || Info == GrB_INDEX_OUT_OF_BOUNDS) ;   \
    }

#else

    // debugging disabled
    #define ASSERT(x)
    #define ASSERT_OK(X)
    #define ASSERT_OK_OR_NULL(X)
    #define ASSERT_OK_OR_JUMBLED(X)

#endif

#define GB_IMPLIES(p,q) (!(p) || (q))

// for finding tests that trigger statement coverage
#define GB_GOTCHA                                           \
{                                                           \
    printf ("gotcha: " __FILE__ " line: %d\n", __LINE__) ;  \
    GB_Global_abort_function_call ( ) ;                     \
}

#define GB_HERE printf (" Here: " __FILE__ " line: %d\n",  __LINE__) ;

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

// true if pointers p1 and p2 are aliased and not NULL
#define GB_POINTER_ALIASED(p1,p2)   ((p1) == (p2) && (p1) != NULL)

// C may not be aliased with X, Y, or Z.  But X, Y and/or Z may be aliased
// with each other.
#define GB_NOT_ALIASED(C,X)        (!GB_aliased (C,X))
#define GB_NOT_ALIASED_2(C,X,Y)    \
    (GB_NOT_ALIASED (C,X) && GB_NOT_ALIASED  (C,Y))
#define GB_NOT_ALIASED_3(C,X,Y,Z)  \
    (GB_NOT_ALIASED (C,X) && GB_NOT_ALIASED_2 (C,Y,Z))

// these macros are always true but are used just for commenting the code
#define GB_ALIAS_OK(C,X)           (GB_aliased (C,X) || GB_NOT_ALIASED (C,X))
#define GB_ALIAS_OK2(C,X,Y)        (GB_ALIAS_OK (C,X) && GB_ALIAS_OK  (C,Y))
#define GB_ALIAS_OK3(C,X,Y,Z)      (GB_ALIAS_OK (C,X) && GB_ALIAS_OK2 (C,Y,Z))

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

typedef enum
{
    // the 12 scalar types
    GB_BOOL_code,               // 0: bool
    GB_INT8_code,               // 1: int8_t
    GB_UINT8_code,              // 2: uint8_t
    GB_INT16_code,              // 3: int16_t
    GB_UINT16_code,             // 4: uint16_t
    GB_INT32_code,              // 5: int32_t
    GB_UINT32_code,             // 6: uint32_t
    GB_INT64_code,              // 7: int64_t
    GB_UINT64_code,             // 8: uint64_t
    GB_FP32_code,               // 9: float
    GB_FP64_code,               // 10: double
    GB_UCT_code,                // 11: void *, compile-time user-defined type
    GB_UDT_code                 // 12: void *, run-time user-defined type
}
GB_Type_code ;                  // enumerated type code

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

// operator codes used in GrB_BinaryOp and GrB_UnaryOp
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

    // 8 binary operators z=f(x,y) that return the same type as their inputs
    GB_FIRST_opcode,    //  7: z = x
    GB_SECOND_opcode,   //  8: z = y
    GB_MIN_opcode,      //  9: z = min(x,y)
    GB_MAX_opcode,      // 10: z = max(x,y)
    GB_PLUS_opcode,     // 11: z = x + y
    GB_MINUS_opcode,    // 12: z = x - y
    GB_TIMES_opcode,    // 13: z = x * y
    GB_DIV_opcode,      // 14: z = x / y ; special cases for bool and ints

    // 6 binary operators z=f(x,y), x,y,z all the same type
    GB_ISEQ_opcode,     // 15: z = (x == y)
    GB_ISNE_opcode,     // 16: z = (x != y)
    GB_ISGT_opcode,     // 17: z = (x >  y)
    GB_ISLT_opcode,     // 18: z = (x <  y)
    GB_ISGE_opcode,     // 19: z = (x >= y)
    GB_ISLE_opcode,     // 20: z = (x <= y)

    // 3 binary operators that work on purely boolean values
    GB_LOR_opcode,      // 21: z = (x != 0) || (y != 0)
    GB_LAND_opcode,     // 22: z = (x != 0) && (y != 0)
    GB_LXOR_opcode,     // 23: z = (x != 0) != (y != 0)

    //--------------------------------------------------------------------------
    // TxT -> bool
    //--------------------------------------------------------------------------

    // 6 binary operators z=f(x,y) that return bool (TxT -> bool)
    GB_EQ_opcode,       // 24: z = (x == y)
    GB_NE_opcode,       // 25: z = (x != y)
    GB_GT_opcode,       // 26: z = (x >  y)
    GB_LT_opcode,       // 27: z = (x <  y)
    GB_GE_opcode,       // 28: z = (x >= y)
    GB_LE_opcode,       // 29: z = (x <= y)

    //--------------------------------------------------------------------------
    // user-defined: unary and binary operators
    //--------------------------------------------------------------------------

    GB_USER_C_opcode,   // 30: compile-time user-defined operator
    GB_USER_R_opcode    // 31: run-time user-defined operator
}
GB_Opcode ;

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
// built-in semirings
//------------------------------------------------------------------------------

// Using built-in types and operators, 960 unique semirings can be built.  This
// count excludes redundant Boolean operators (for example GxB_TIMES_BOOL and
// GxB_LAND_BOOL are different operators but they are redundant since they
// always return the same result):

// 680 semirings with a multiply operator TxT -> T where T is non-Boolean, from


//------------------------------------------------------------------------------
// select opcodes
//------------------------------------------------------------------------------

// operator codes used in GrB_SelectOp structures
typedef enum
{
    // built-in select operators:
    GB_TRIL_opcode,
    GB_TRIU_opcode,
    GB_DIAG_opcode,
    GB_OFFDIAG_opcode,
    GB_NONZERO_opcode,

    // for all user-defined select operators:
    GB_USER_SELECT_C_opcode,    // defined at compile-time
    GB_USER_SELECT_R_opcode     // defined at run-time
}
GB_Select_Opcode ;

extern struct GB_SelectOp_opaque
    GB_opaque_GxB_TRIL,
    GB_opaque_GxB_TRIU,
    GB_opaque_GxB_DIAG,
    GB_opaque_GxB_OFFDIAG,
    GB_opaque_GxB_NONZERO ;

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
// nthreads = GxB_DEFAULT (that is, zero).  The default rule is to let
// GraphBLAS determine the number of threads automatically by selecting a
// number of threads between 1 and nthreads_max.  GrB_init initializes
// nthreads_max to omp_get_max_threads ( ).  Both the global value and the
// value in a descriptor can set/queried by GxB_set / GxB_get.

// Some GrB_Matrix and GrB_Vector methods do not take a descriptor, however
// (GrB_*_dup, _build, _exportTuples, _clear, _nvals, _wait, and GxB_*_resize).
// For those methods the default rule is always used (nthreads = GxB_DEFAULT),
// which then relies on the global nthreads_max.

#define GB_RLEN 384
#define GB_DLEN 256

typedef struct
{
    int nthreads ;              // number of threads to use
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

#define GB_WHERE(where_string)                      \
    GB_Context_struct Context_struct ;              \
    GB_Context Context = &Context_struct ;          \
    Context->where = where_string ;                 \
    Context->nthreads = GB_Global_nthreads_max_get ( ) ;

// GB_GET_NTHREADS:  determine number of threads for OpenMP parallelism.
//
//      GB_GET_NTHREADS obtains the # of threads to use, from the Context.  If
//      Context is NULL then a single thread *must* be used (this is only used
//      for GB_qsort_*, calloc, and realloc, for problems that are small or
//      where the calling function is already being done by one thread in a
//      larger parallel construct).  If Context->nthreads is <= GxB_DEFAULT,
//      then select automatically: between 1 and nthreads_max, depending on the
//      problem size.  Below is the default rule.  Any function can use its own
//      rule instead, based on Context, nthreads_max, and the problem size.  No
//      rule can exceed nthreads_max.

#if defined ( _OPENMP )
    #define GB_GET_NTHREADS(nthreads,Context)                               \
        int nthreads = (Context == NULL) ? 1 : Context->nthreads ;          \
        if (nthreads <= GxB_DEFAULT) nthreads = GB_Global_nthreads_max_get ( ) ;
#else
    // OpenMP is not available; SuiteSparse:GraphBLAS is sequential.
    #define GB_GET_NTHREADS(nthreads,Context)                               \
        int nthreads = 1 ;
#endif

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

// check object->magic code
#ifdef GB_DEVELOPER
#define GBPR_MAGIC(pcode)                                               \
{                                                                       \
    char *p = (char *) &(pcode) ;                                       \
    if (pr > 0) GBPR (" magic: [ %d1 %s ] ", p [0], p) ;                \
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
            if (pr > 0) GBPR ("already freed!\n") ;                     \
            return (GB_ERROR (GrB_UNINITIALIZED_OBJECT, (GB_LOG,        \
                "%s is freed: [%s]", kind, name))) ;                    \
                                                                        \
        case GB_MAGIC2 :                                                \
            /* invalid */                                               \
            GBPR_MAGIC (object->magic) ;                                \
            if (pr > 0) GBPR ("invalid\n") ;                            \
            return (GB_ERROR (GrB_INVALID_OBJECT, (GB_LOG,              \
                "%s is invalid: [%s]", kind, name))) ;                  \
                                                                        \
        default :                                                       \
            /* uninitialized */                                         \
            if (pr > 0) GBPR ("uninititialized\n") ;                    \
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
    GB_Context Context
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

#ifndef GB_PANIC
#define GB_PANIC { printf ("panic %s %d\n", __FILE__, __LINE__) ; return (GrB_PANIC) ; }
#endif

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

GrB_Info GB_transpose_bucket    // bucket transpose; typecast and apply op
(
    GrB_Matrix *Chandle,        // output matrix (unallocated on input)
    const GrB_Type ctype,       // type of output matrix C
    const bool C_is_csc,        // format of output matrix C
    const GrB_Matrix A,         // input matrix
    const GrB_UnaryOp op,       // operator to apply, NULL if no operator
    GB_Context Context
) ;

void GB_transpose_ix        // transpose the pattern and values of a matrix
(
    int64_t *Rp,            // size m+1, input: row pointers, shifted on output
    int64_t *Ri,            // size cnz, output column indices
    GB_void *Rx,            // size cnz, output numerical values, type R_type
    const GrB_Type R_type,  // type of output R (do typecasting into R)
    const GrB_Matrix A,     // input matrix
    GB_Context Context
) ;

void GB_transpose_op        // transpose and apply an operator to a matrix
(
    int64_t *Rp,            // size m+1, input: row pointers, shifted on output
    int64_t *Ri,            // size cnz, output column indices
    GB_void *Rx,            // size cnz, output values, type op->ztype
    const GrB_UnaryOp op,   // operator to apply, NULL if no operator
    const GrB_Matrix A,     // input matrix
    GB_Context Context
) ;

GrB_Info GB_apply                   // C<M> = accum (C, op(A)) or op(A')
(
    GrB_Matrix C,                   // input/output matrix for results
    const bool C_replace,           // C descriptor
    const GrB_Matrix M,             // optional mask for C, unused if NULL
    const bool Mask_comp,           // M descriptor
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C,T)
    const GrB_UnaryOp op,           // operator to apply to the entries
    const GrB_Matrix A,             // first input:  matrix A
    bool A_transpose,               // A matrix descriptor
    GB_Context Context
) ;

GrB_Info GB_select          // C<M> = accum (C, select(A,k)) or select(A',k)
(
    GrB_Matrix C,                   // input/output matrix for results
    const bool C_replace,           // C descriptor
    const GrB_Matrix M,             // optional mask for C, unused if NULL
    const bool Mask_comp,           // descriptor for M
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C,T)
    const GxB_SelectOp op,          // operator to select the entries
    const GrB_Matrix A,             // input matrix
    const void *k,                  // optional input for select operator
    const bool A_transpose,         // A matrix descriptor
    GB_Context Context
) ;

GrB_Info GB_shallow_cast    // create a shallow typecasted matrix
(
    GrB_Matrix *Chandle,    // output matrix C, of type op->ztype
    const GrB_Type ctype,   // type of the output matrix C
    const bool C_is_csc,    // desired CSR/CSC format of C
    const GrB_Matrix A,     // input matrix to typecast
    GB_Context Context
) ;

GrB_Info GB_shallow_op      // create shallow matrix and apply operator
(
    GrB_Matrix *Chandle,    // output matrix C, of type op->ztype
    const bool C_is_csc,    // desired CSR/CSC format of C
    const GrB_UnaryOp op,   // operator to apply
    const GrB_Matrix A,     // input matrix to typecast
    GB_Context Context
) ;

void GB_cast_array              // typecast an array
(
    void *C,                    // output array
    const GB_Type_code code1,   // type code for C
    const void *A,              // input array
    const GB_Type_code code2,   // type code for A
    const int64_t n,            // number of entries in C and A
    GB_Context Context
) ;

GB_cast_function GB_cast_factory   // returns pointer to function to cast x to z
(
    const GB_Type_code code1,      // the type of z, the output value
    const GB_Type_code code2       // the type of x, the input value
) ;

GrB_Info GB_add             // C = A+B
(
    GrB_Matrix *Chandle,    // output matrix (unallocated on input)
    const GrB_Type ctype,   // type of output matrix C
    const bool C_is_csc,    // format of output matrix C
    const GrB_Matrix A,     // input A matrix
    const GrB_Matrix B,     // input B matrix
    const GrB_BinaryOp op,  // op to perform C = op (A,B)
    GB_Context Context
) ;

GrB_Info GB_emult           // C = A.*B
(
    GrB_Matrix *Chandle,    // output matrix (unallocated on input)
    const GrB_Type ctype,   // type of output matrix C
    const bool C_is_csc,    // format of output matrix C
    const GrB_Matrix A,     // input A matrix
    const GrB_Matrix B,     // input B matrix
    const GrB_BinaryOp op,  // op to perform C = op (A,B)
    GB_Context Context
) ;

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
    size_t size_of_item,    // sizeof each item
    GB_Context Context
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
    bool *ok,               // true if successful, false otherwise
    GB_Context Context
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

// if GB_PRINT_MALLOC is define above, these macros print diagnostic
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

#define GB_CALLOC_MEMORY(p,n,s,Context)                                        \
    printf ("\nCalloc:                       "                                \
    "%s = calloc (%s = "GBd", %s = "GBd") line %d file %s\n",                 \
    GB_STR(p), GB_STR(n), (int64_t) n, GB_STR(s), (int64_t) s,                \
    __LINE__,__FILE__) ;                                                      \
    p = GB_calloc_memory (n, s, Context) ;

#define GB_MALLOC_MEMORY(p,n,s)                                               \
    printf ("\nMalloc:                       "                                \
    "%s = malloc (%s = "GBd", %s = "GBd") line %d file %s\n",                 \
    GB_STR(p), GB_STR(n), (int64_t) n, GB_STR(s), (int64_t) s,                \
    __LINE__,__FILE__) ;                                                      \
    p = GB_malloc_memory (n, s) ;

#define GB_REALLOC_MEMORY(p,nnew,nold,s,ok,Context)                            \
{                                                                             \
    printf ("\nRealloc: %14p       "                                          \
    "%s = realloc (%s = "GBd", %s = "GBd", %s = "GBd") line %d file %s\n",    \
    p, GB_STR(p), GB_STR(nnew), (int64_t) nnew, GB_STR(nold), (int64_t) nold, \
    GB_STR(s), (int64_t) s, __LINE__,__FILE__) ;                              \
    p = GB_realloc_memory (nnew, nold, s, p, ok, Context) ;                   \
}

#define GB_FREE_MEMORY(p,n,s)                                                 \
{                                                                             \
    if (p)                                                                    \
    printf ("\nFree:               "                                          \
    "(%s, %s = "GBd", %s = "GBd") line %d file %s\n",                         \
    GB_STR(p), GB_STR(n), (int64_t) n, GB_STR(s), (int64_t) s,                \
    __LINE__,__FILE__) ;                                                      \
    GB_free_memory (p, n, s) ;                                                \
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

#define GB_CALLOC_MEMORY(p,n,s,Context)                                       \
    p = GB_calloc_memory (n, s ,Context) ;

#define GB_MALLOC_MEMORY(p,n,s)                                               \
    p = GB_malloc_memory (n, s) ;

#define GB_REALLOC_MEMORY(p,nnew,nold,s,ok,Context)                           \
    p = GB_realloc_memory (nnew, nold, s, p, ok, Context) ;

#define GB_FREE_MEMORY(p,n,s)                                                 \
{                                                                             \
    GB_free_memory (p, n, s) ;                                                \
    (p) = NULL ;                                                              \
}

#endif

//------------------------------------------------------------------------------

GrB_Type GB_code_type           // return the GrB_Type corresponding to the code
(
    const GB_Type_code code,    // type code to convert
    const GrB_Type type         // user type if code is GB_UDT_code
) ;

GrB_Info GB_AxB_alloc           // estimate nnz(C) and allocate C for C=A*B
(
    GrB_Matrix *Chandle,        // output matrix
    const GrB_Type ctype,       // type of C
    const GrB_Index cvlen,      // vector length of C
    const GrB_Index cvdim,      // # of vectors of C
    const GrB_Matrix M,         // optional mask
    const GrB_Matrix A,         // input matrix A (transposed for dot product)
    const GrB_Matrix B,         // input matrix B
    const bool numeric,         // if true, allocate A->x, else A->x is NULL
    const int64_t rough_guess   // rough estimate of nnz(C)
) ;

GrB_Info GB_AxB_Gustavson           // C=A*B or C<M>=A*B, Gustavson's method
(
    GrB_Matrix *Chandle,            // output matrix
    const GrB_Matrix M_in,          // optional matrix
    const bool Mask_comp,           // if true, use ~M
    const GrB_Matrix A,             // input matrix A
    const GrB_Matrix B,             // input matrix B
    const GrB_Semiring semiring,    // semiring that defines C=A*B
    const bool flipxy,              // if true, do z=fmult(b,a) vs fmult(a,b)
    bool *mask_applied,             // if true, mask was applied
    const int Sauna_id              // Sauna to use
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

GrB_Info GB_AxB_heap                // C<M>=A*B or C=A*B using a heap
(
    GrB_Matrix *Chandle,            // output matrix
    const GrB_Matrix M_in,          // mask matrix for C<M>=A*B
    const bool Mask_comp,           // if true, use ~M
    const GrB_Matrix A,             // input matrix
    const GrB_Matrix B,             // input matrix
    const GrB_Semiring semiring,    // semiring that defines C=A*B
    const bool flipxy,              // if true, do z=fmult(b,a) vs fmult(a,b)
    bool *mask_applied,             // if true, mask was applied
    const int64_t bjnz_max          // max # entries in any vector of B
) ;

void GB_AxB_select                  // select method for A*B or A'*B
(
    const GrB_Matrix A,             // input matrix A
    const GrB_Matrix B,             // input matrix B
    const GrB_Semiring semiring,    // semiring that defines C=A*B
    const bool do_adotb,            // if true then do A'*B
    const GrB_Desc_Value AxB_method,// for auto vs user selection of methods
    // output
    GrB_Desc_Value *AxB_method_used,        // method to use
    int64_t *bjnz_max                       // # entries in densest col of B
) ;

GrB_Info GB_AxB_meta                // C<M>=A*B meta algorithm
(
    GrB_Matrix *Chandle,            // output matrix C
    const bool C_is_csc,            // desired CSR/CSC format of C
    GrB_Matrix *MT_handle,          // return MT = M' to caller, if computed
    const GrB_Matrix M_in,          // mask for C<M> (not complemented)
    const bool Mask_comp,           // if true, use ~M
    const GrB_Matrix A_in,          // input matrix
    const GrB_Matrix B_in,          // input matrix
    const GrB_Semiring semiring,    // semiring that defines C=A*B
    bool A_transpose,               // if true, use A', else A
    bool B_transpose,               // if true, use B', else B
    bool flipxy,                    // if true, do z=fmult(b,a) vs fmult(a,b)
    bool *mask_applied,             // if true, mask was applied
    const GrB_Desc_Value AxB_method,// for auto vs user selection of methods
    GrB_Desc_Value *AxB_method_used,// method selected
    GB_Context Context
) ;

GrB_Info GB_AxB_parallel            // parallel matrix-matrix multiply
(
    GrB_Matrix *Chandle,            // output matrix, NULL on input
    GrB_Matrix M,                   // optional mask matrix
    const bool Mask_comp,           // if true, use ~M
    const GrB_Matrix A,             // input matrix A
    const GrB_Matrix B,             // input matrix B
    const GrB_Semiring semiring,    // semiring that defines C=A*B
    const bool flipxy,              // if true, do z=fmult(b,a) vs fmult(a,b)
    const bool do_adotb,            // if true, do A'*B via dot products
    const GrB_Desc_Value AxB_method,// for auto vs user selection of methods
//  const GrB_Desc_Value AxB_slice, // how to slice B or A'
    GrB_Desc_Value *AxB_method_used,// method selected
    bool *mask_applied,             // if true, mask was applied
    GB_Context Context
) ;

GrB_Info GB_AxB_sequential          // single-threaded matrix-matrix multiply
(
    GrB_Matrix *Chandle,            // output matrix, NULL on input
    GrB_Matrix M,                   // optional mask matrix
    const bool Mask_comp,           // if true, use ~M
    const GrB_Matrix A,             // input matrix A
    const GrB_Matrix B,             // input matrix B
    const GrB_Semiring semiring,    // semiring that defines C=A*B
    const bool flipxy,              // if true, do z=fmult(b,a) vs fmult(a,b)
    const GrB_Desc_Value AxB_method,// already chosen
    const int64_t bjnz_max,         // for heap method only
    bool *mask_applied,             // if true, mask was applied
    const int Sauna_id              // Sauna to use, for Gustavson method only
) ;

bool GB_semiring_builtin            // true if semiring is builtin
(
    // inputs:
    const GrB_Matrix A,
    const GrB_Matrix B,
    const GrB_Semiring semiring,    // semiring that defines C=A*B
    const bool flipxy,              // true if z=fmult(y,x), flipping x and y
    // outputs, unused by caller if this function returns false
    GB_Opcode *mult_opcode,         // multiply opcode
    GB_Opcode *add_opcode,          // add opcode
    GB_Type_code *xycode,           // type code for x and y inputs
    GB_Type_code *zcode             // type code for z output
) ;

GrB_Info GB_AxB_Gustavson_builtin
(
    GrB_Matrix C,                   // output matrix
    const GrB_Matrix M,             // M matrix for C<M> (not complemented)
    const GrB_Matrix A,             // input matrix
    const GrB_Matrix B,             // input matrix
    const GrB_Semiring semiring,    // semiring that defines C=A*B
    const bool flipxy,              // if true, do z=fmult(b,a) vs fmult(a,b)
    bool *done,                     // true if C=A*B has been computed
    GB_Sauna Sauna                  // sparse accumulator
) ;

GrB_Info GB_AxB_dot                 // C = A'*B using dot product method
(
    GrB_Matrix *Chandle,            // output matrix
    const GrB_Matrix M_in,          // mask matrix for C<M>=A'*B
    const bool Mask_comp,           // if true, use ~M
    const GrB_Matrix A,             // input matrix
    const GrB_Matrix B,             // input matrix
    const GrB_Semiring semiring,    // semiring that defines C=A*B
    const bool flipxy,              // if true, do z=fmult(b,a) vs fmult(a,b)
    bool *mask_applied              // if true, mask was applied
) ;

bool GB_AxB_flopcount           // compute flops for C<M>=A*B or C=A*B
(
    int64_t *Bflops,            // size B->nvec+1, if present
    GrB_Matrix M,               // optional mask matrix
    GrB_Matrix A,
    GrB_Matrix B,
    int64_t floplimit,          // maximumm flops to compute if Bflops NULL
    GB_Context Context
) ;

GrB_Info GB_mxm                     // C<M> = A*B
(
    GrB_Matrix C,                   // input/output matrix for results
    const bool C_replace,           // if true, clear C before writing to it
    const GrB_Matrix M,             // optional mask for C, unused if NULL
    const bool Mask_comp,           // if true, use ~M
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C,T)
    const GrB_Semiring semiring,    // defines '+' and '*' for C=A*B
    const GrB_Matrix A,             // input matrix
    const bool A_transpose,         // if true, use A' instead of A
    const GrB_Matrix B,             // input matrix
    const bool B_transpose,         // if true, use B' instead of B
    const bool flipxy,              // if true, do z=fmult(b,a) vs fmult(a,b)
    const GrB_Desc_Value AxB_method,// for auto vs user selection of methods
    GB_Context Context
) ;

void GB_cumsum                  // compute the cumulative sum of an array
(
    int64_t *count,             // size n+1, input/output
    const int64_t n,
    int64_t *kresult,           // return k, if needed by the caller
    GB_Context Context
) ;

GrB_Info GB_mask                // C<M> = Z
(
    GrB_Matrix C_result,        // both input C and result matrix
    const GrB_Matrix M,         // optional mask matrix, can be NULL
    GrB_Matrix *Zhandle,        // Z = results of computation, might be shallow
                                // or can even be NULL if M is empty and
                                // complemented.  Z is freed when done.
    const bool C_replace,       // true if clear(C) to be done first
    const bool Mask_complement, // true if M is to be complemented
    GB_Context Context
) ;

GrB_Info GB_accum_mask          // C<M> = accum (C,T)
(
    GrB_Matrix C,               // input/output matrix for results
    const GrB_Matrix M_in,      // optional mask for C, unused if NULL
    const GrB_Matrix MT_in,     // MT=M' if computed already in the caller
    const GrB_BinaryOp accum,   // optional accum for Z=accum(C,results)
    GrB_Matrix *Thandle,        // results of computation, freed when done
    const bool C_replace,       // if true, clear C first
    const bool Mask_complement, // if true, complement the mask
    GB_Context Context
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

void GB_qsort_1         // sort array A of size 1-by-n
(
    int64_t A_0 [ ],    // size-n array
    const int64_t n,
    GB_Context Context  // for # of threads; use one thread if NULL
) ;

void GB_qsort_2a        // sort array A of size 2-by-n, using 1 key (A [0][])
(
    int64_t A_0 [ ],    // size n array
    int64_t A_1 [ ],    // size n array
    const int64_t n,
    GB_Context Context  // for # of threads; use one thread if NULL
) ;

void GB_qsort_2b        // sort array A of size 2-by-n, using 2 keys (A [0:1][])
(
    int64_t A_0 [ ],    // size n array
    int64_t A_1 [ ],    // size n array
    const int64_t n,
    GB_Context Context  // for # of threads; use one thread if NULL
) ;

void GB_qsort_3         // sort array A of size 3-by-n, using 3 keys (A [0:2][])
(
    int64_t A_0 [ ],    // size n array
    int64_t A_1 [ ],    // size n array
    int64_t A_2 [ ],    // size n array
    const int64_t n,
    GB_Context Context  // for # of threads; use one thread if NULL
) ;

GrB_Info GB_subref_numeric      // C = A (I,J), extract the values
(
    GrB_Matrix *Chandle,        // output C
    const bool C_is_csc,        // requested format of C
    const GrB_Matrix A,         // input matrix
    const GrB_Index *I,         // list of indices, duplicates OK
    int64_t ni,                 // length of the I array, or special
    const GrB_Index *J,         // list of vector indices, duplicates OK
    int64_t nj,                 // length of the J array, or special
    const bool must_sort,       // if true, must return C sorted
    GB_Context Context
) ;

GrB_Info GB_subref_symbolic     // C = A (I,J), extract the pattern
(
    GrB_Matrix *Chandle,        // output C
    const bool C_is_csc,        // requested format of C
    const GrB_Matrix A,         // input matrix
    const GrB_Index *I,         // list of indices, duplicates OK
    int64_t ni,                 // length of the I array, or special
    const GrB_Index *J,         // list of vector indices, duplicates OK
    int64_t nj,                 // length of the J array, or special
    GB_Context Context
) ;

GrB_Info GB_I_inverse           // invert the I list for GB_subref_template
(
    const GrB_Index *I,         // list of indices, duplicates OK
    int64_t nI,                 // length of I
    int64_t avlen,              // length of the vectors of A
    bool need_Iwork1,           // true if Iwork1 of size nI needed for sorting
    // outputs:
    int64_t **p_Mark,           // head pointers for buckets, size avlen
    int64_t **p_Inext,          // next pointers for buckets, size nI
    int64_t **p_Iwork1,         // workspace of size nI, if needed
    int64_t *p_nduplicates,     // number of duplicate entries in I
    int64_t *p_flag,            // Mark [0:avlen-1] < flag
    GB_Context Context
) ;

GrB_Info GB_extract                 // C<M> = accum (C, A(I,J))
(
    GrB_Matrix C,                   // input/output matrix for results
    const bool C_replace,           // C matrix descriptor
    const GrB_Matrix M,             // optional mask for C, unused if NULL
    const bool Mask_comp,           // mask descriptor
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C,T)
    const GrB_Matrix A,             // input matrix
    const bool A_transpose,         // A matrix descriptor
    const GrB_Index *Rows,          // row indices
    const GrB_Index nRows_in,       // number of row indices
    const GrB_Index *Cols,          // column indices
    const GrB_Index nCols_in,       // number of column indices
    GB_Context Context
) ;

GrB_Info GB_eWise                   // C<M> = accum (C, A+B) or A.*B
(
    GrB_Matrix C,                   // input/output matrix for results
    const bool C_replace,           // if true, clear C before writing to it
    const GrB_Matrix M,             // optional mask for C, unused if NULL
    const bool Mask_comp,           // if true, complement the mask M
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C,T)
    const GrB_BinaryOp op,          // defines '+' for C=A+B, or .* for A.*B
    const GrB_Matrix A,             // input matrix
    bool A_transpose,               // if true, use A' instead of A
    const GrB_Matrix B,             // input matrix
    bool B_transpose,               // if true, use B' instead of B
    const bool eWiseAdd,            // if true, do set union (like A+B),
                                    // otherwise do intersection (like A.*B)
    GB_Context Context
) ;

GrB_Info GB_reduce_to_column        // C<M> = accum (C,reduce(A))
(
    GrB_Matrix C,                   // input/output for results, size n-by-1
    const GrB_Matrix M,             // optional M for C, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for z=accum(C,T)
    const GrB_BinaryOp reduce,      // reduce operator for T=reduce(A)
    const GB_void *terminal,        // for early exit (NULL if none)
    const GrB_Matrix A,             // first input:  matrix A
    const GrB_Descriptor desc,      // descriptor for C, M, and A
    GB_Context Context
) ;

GrB_Info GB_reduce_to_scalar    // twork = reduce_to_scalar (A)
(
    void *c,                    // result scalar
    const GrB_Type ctype,       // the type of scalar, c
    const GrB_BinaryOp accum,   // for c = accum(c,twork)
    const GrB_Monoid reduce,    // monoid to do the reduction
    const GrB_Matrix A,         // matrix to reduce
    GB_Context Context
) ;

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

GrB_Info GB_user_build          // check inputs then build matrix
(
    GrB_Matrix C,               // matrix to build
    const GrB_Index *I,         // row indices of tuples
    const GrB_Index *J,         // col indices of tuples
    const void *S,              // array of values of tuples
    const GrB_Index nvals,      // number of tuples
    const GrB_BinaryOp dup,     // binary function to assemble duplicates
    const GB_Type_code scode,   // GB_Type_code of S array
    const bool is_matrix,       // true if C is a matrix, false if GrB_Vector
    GB_Context Context
) ;

GrB_Info GB_build               // build matrix
(
    GrB_Matrix C,               // matrix to build
    const GrB_Index *I_in,      // row indices of tuples
    const GrB_Index *J_in,      // col indices of tuples
    const void *S,              // array of values of tuples
    const GrB_Index nvals,      // number of tuples
    const GrB_BinaryOp dup,     // binary function to assemble duplicates
    const GB_Type_code scode,   // GB_Type_code of S array
    const bool is_matrix,       // true if C is a matrix, false if GrB_Vector
    const bool ijcheck,         // true if I and J are to be checked
    GB_Context Context
) ;

GrB_Info GB_builder
(
    GrB_Matrix *Thandle,            // matrix T to build
    const GrB_Type ttype,           // type of output matrix T
    const int64_t vlen,             // length of each vector of T
    const int64_t vdim,             // number of vectors in T
    const bool is_csc,              // true if T is CSC, false if CSR
    int64_t **iwork_handle,         // for (i,k) or (j,i,k) tuples
    int64_t **jwork_handle,         // for (j,i,k) tuples
    const bool already_sorted,      // true if tuples already sorted on input
    const void *S,                  // array of values of tuples
    const int64_t len,              // number of tuples, and size of kwork
    const int64_t ijlen,            // size of iwork and jwork arrays
    const GrB_BinaryOp dup,         // binary function to assemble duplicates,
                                    // if NULL use the "SECOND" function to
                                    // keep the most recent duplicate.
    const GB_Type_code scode,       // GB_Type_code of S array
    GB_Context Context
) ;

GrB_Info GB_build_factory           // build a matrix
(
    GrB_Matrix *Thandle,            // matrix T to build
    const int64_t tnz0,             // final nnz(T)
    int64_t **iwork_handle,         // for (i,k) or (j,i,k) tuples
    int64_t **kwork_handle,         // for (i,k) or (j,i,k) tuples
    const GB_void *S,               // array of values of tuples
    const int64_t len,              // number of tuples and size of kwork
    const int64_t ijlen,            // size of iwork array
    const GrB_BinaryOp dup,         // binary function to assemble duplicates,
                                    // if NULL use the "SECOND" function to
                                    // keep the most recent duplicate.
    const GB_Type_code scode,       // GB_Type_code of S array
    GB_Context Context
) ;

GrB_Info GB_wait                // finish all pending computations
(
    GrB_Matrix A,               // matrix with pending computations
    GB_Context Context
) ;

GrB_Info GB_pending_add             // add a pending tuple A(i,j) to a matrix
(
    GrB_Matrix A,                   // matrix to modify
    const void *scalar,             // scalar to add to the pending tuples of A
    const GrB_Type stype,           // scalar type
    const GrB_BinaryOp pop,         // new A->operator_pending, if 1st pending
    const int64_t i,                // index into vector
    const int64_t j,                // vector index
    GB_Context Context
) ;

void GB_pending_free            // free all pending tuples
(
    GrB_Matrix A                // matrix with pending tuples to free
) ;

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

GrB_Info GB_subassign               // C(Rows,Cols)<M> += A or A'
(
    GrB_Matrix C,                   // input/output matrix for results
    const bool C_replace,           // descriptor for C
    const GrB_Matrix M_in,          // optional mask for C(Rows,Cols)
    const bool Mask_comp,           // true if mask is complemented
    bool M_transpose,               // true if the mask should be transposed
    const GrB_BinaryOp accum,       // optional accum for accum(C,T)
    const GrB_Matrix A_in,          // input matrix
    bool A_transpose,               // true if A is transposed
    const GrB_Index *Rows,          // row indices
    const GrB_Index nRows_in,       // number of row indices
    const GrB_Index *Cols,          // column indices
    const GrB_Index nCols_in,       // number of column indices
    const bool scalar_expansion,    // if true, expand scalar to A
    const void *scalar,             // scalar to be expanded
    const GB_Type_code scalar_code, // type code of scalar to expand
    GB_Context Context
) ;

GrB_Info GB_assign                  // C<M>(Rows,Cols) += A or A'
(
    GrB_Matrix C,                   // input/output matrix for results
    const bool C_replace,           // descriptor for C
    const GrB_Matrix M_in,          // optional mask for C
    const bool Mask_comp,           // true if mask is complemented
    bool M_transpose,               // true if the mask should be transposed
    const GrB_BinaryOp accum,       // optional accum for accum(C,T)
    const GrB_Matrix A_in,          // input matrix
    bool A_transpose,               // true if A is transposed
    const GrB_Index *Rows,          // row indices
    const GrB_Index nRows_in,       // number of row indices
    const GrB_Index *Cols,          // column indices
    const GrB_Index nCols_in,       // number of column indices
    const bool scalar_expansion,    // if true, expand scalar to A
    const void *scalar,             // scalar to be expanded
    const GB_Type_code scalar_code, // type code of scalar to expand
    const bool col_assign,          // true for GrB_Col_assign
    const bool row_assign,          // true for GrB_Row_assign
    GB_Context Context
) ;

GrB_Info GB_subassign_kernel        // C(I,J)<M> = A or accum (C (I,J), A)
(
    GrB_Matrix C,                   // input/output matrix for results
    bool C_replace,                 // C matrix descriptor
    const GrB_Matrix M,             // optional mask for C(I,J), unused if NULL
    const bool Mask_comp,           // mask descriptor
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C(I,J),A)
    const GrB_Matrix A,             // input matrix (NULL for scalar expansion)
    const GrB_Index *I,             // list of indices
    const int64_t   ni,             // number of indices
    const GrB_Index *J,             // list of vector indices
    const int64_t   nj,             // number of column indices
    const bool scalar_expansion,    // if true, expand scalar to A
    const void *scalar,             // scalar to be expanded
    const GB_Type_code scalar_code, // type code of scalar to expand
    GB_Context Context
) ;

GrB_Info GB_subassign_scalar        // C(Rows,Cols)<M> += x
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Matrix M,             // mask for C(Rows,Cols), unused if NULL
    const GrB_BinaryOp accum,       // accum for Z=accum(C(Rows,Cols),T)
    const void *scalar,             // scalar to assign to C(Rows,Cols)
    const GB_Type_code scalar_code, // type code of scalar to assign
    const GrB_Index *Rows,          // row indices
    const GrB_Index nRows,          // number of row indices
    const GrB_Index *Cols,          // column indices
    const GrB_Index nCols,          // number of column indices
    const GrB_Descriptor desc,      // descriptor for C(Rows,Cols) and M
    GB_Context Context
) ;

GrB_Info GB_assign_scalar           // C<M>(Rows,Cols) += x
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Matrix M,             // mask for C(Rows,Cols), unused if NULL
    const GrB_BinaryOp accum,       // accum for Z=accum(C(Rows,Cols),T)
    const void *scalar,             // scalar to assign to C(Rows,Cols)
    const GB_Type_code scalar_code, // type code of scalar to assign
    const GrB_Index *Rows,          // row indices
    const GrB_Index nRows,          // number of row indices
    const GrB_Index *Cols,          // column indices
    const GrB_Index nCols,          // number of column indices
    const GrB_Descriptor desc,      // descriptor for C and M
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

GrB_Info GB_kron                    // C<M> = accum (C, kron(A,B))
(
    GrB_Matrix C,                   // input/output matrix for results
    const bool C_replace,           // if true, clear C before writing to it
    const GrB_Matrix M,             // optional mask for C, unused if NULL
    const bool Mask_comp,           // if true, use ~M
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C,T)
    const GrB_BinaryOp op,          // defines '*' for kron(A,B)
    const GrB_Matrix A,             // input matrix
    bool A_transpose,               // if true, use A' instead of A
    const GrB_Matrix B,             // input matrix
    bool B_transpose,               // if true, use B' instead of B
    GB_Context Context
) ;

GrB_Info GB_kron_kernel             // C = kron (A,B)
(
    GrB_Matrix *Chandle,            // output matrix
    const bool C_is_csc,            // desired format of C
    const GrB_BinaryOp op,          // multiply operator
    const GrB_Matrix A,             // input matrix
    const GrB_Matrix B,             // input matrix
    GB_Context Context
) ;

void GB_apply_op            // apply a unary operator, Cx = op ((xtype) Ax)
(
    GB_void *Cx,            // output array, of type op->ztype
    const GrB_UnaryOp op,   // operator to apply
    const GB_void *Ax,      // input array, of type atype
    const GrB_Type atype,   // type of Ax
    const int64_t anz,      // size of Ax and Cx
    GB_Context Context
) ;

GrB_Info GB_transpose           // C=A', C=(ctype)A or C=op(A')
(
    GrB_Matrix *Chandle,        // output matrix C, possibly modified in place
    GrB_Type ctype,             // desired type of C; if NULL use A->type.
                                // ignored if op is present (cast to op->ztype)
    const bool C_is_csc,        // desired CSR/CSC format of C
    const GrB_Matrix A_in,      // input matrix
    const GrB_UnaryOp op_in,    // optional operator to apply to the values
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

//------------------------------------------------------------------------------
// Global storage: for all threads in a user application that uses GraphBLAS
//------------------------------------------------------------------------------

// Global storage is used to record a list of matrices with pending operations
// (for GrB_wait), and to keep track of the GraphBLAS mode (blocking or
// non-blocking).

typedef struct
{

    //--------------------------------------------------------------------------
    // queue of matrices with work to do
    //--------------------------------------------------------------------------

    // In non-blocking mode, GraphBLAS needs to keep track of all matrices that
    // have pending operations that have not yet been finished.  In the current
    // implementation, these are matrices with pending tuples from
    // GrB_setElement, GxB_subassign, and GrB_assign that haven't been added to
    // the matrix yet.

    // A matrix with no pending tuples is not in the list.  When a matrix gets
    // its first pending tuple, it is added to the list.  A matrix is removed
    // from the list when another operation needs to use the matrix; in that
    // case the pending tuples are assembled for just that one matrix.  The
    // GrB_wait operation iterates through the entire list and assembles all
    // the pending tuples for all the matrices in the list, leaving the list
    // emtpy.  A simple link list suffices for the list.  The links are in the
    // matrices themselves so no additional memory needs to be allocated.  The
    // list never needs to be searched; if a particular matrix is to be removed
    // from the list, GraphBLAS has already been given the matrix handle, and
    // the prev & next pointers it contains.  All of these operations can thus
    // be done in O(1) time, except for GrB_wait which needs to traverse the
    // whole list once and then the list is empty afterwards.

    // The access of these variables must be protected in a critical section,
    // if the user application is multithreaded.

    bool user_multithreaded ;   // true if user application may be multithreaded

    void *queue_head ;          // head pointer to matrix queue

    GrB_Mode mode ;             // GrB_NONBLOCKING or GrB_BLOCKING

    bool GrB_init_called ;      // true if GrB_init already called

    int nthreads_max ;          // max number of threads to use

    //--------------------------------------------------------------------------
    // Sauna: thread workspace for Gustavson's method
    //--------------------------------------------------------------------------

    GB_Sauna Saunas   [GxB_NTHREADS_MAX] ;
    bool Sauna_in_use [GxB_NTHREADS_MAX] ;

    //--------------------------------------------------------------------------
    // hypersparsity and CSR/CSC format control
    //--------------------------------------------------------------------------

    // See GB_matrix.h and GraphBLAS.h for a description.  It is safe for
    // a multi-threaded application to modify these in parallel, and thus
    // they do not need to be protected by a critical section.

    double hyper_ratio ;        // default hyper_ratio for new matrices
    bool is_csc ;               // default CSR/CSC format for new matrices

    //--------------------------------------------------------------------------
    // abort function: only used for debugging
    //--------------------------------------------------------------------------

    void (* abort_function ) (void) ;

    //--------------------------------------------------------------------------
    // malloc/calloc/realloc/free: memory management functions
    //--------------------------------------------------------------------------

    // All threads must use the same malloc/calloc/realloc/free functions.
    // They default to the ANSI C11 functions, but can be defined by GxB_init.

    void * (* malloc_function  ) (size_t)         ;
    void * (* calloc_function  ) (size_t, size_t) ;
    void * (* realloc_function ) (void *, size_t) ;
    void   (* free_function    ) (void *)         ;

    //--------------------------------------------------------------------------
    // memory usage tracking: for testing and debugging only
    //--------------------------------------------------------------------------

    // NOTE: these statistics are not thread-safe, and used only for testing.

    // malloc_tracking:  default is false.  There is no user-accessible API for
    // setting this to true.  If true, the following statistics are computed.
    // If false, all of the following are unused.

    // nmalloc:  To aid in searching for memory leaks, GraphBLAS keeps track of
    // the number of blocks of allocated that have not yet been freed.  The
    // count starts at zero.  GB_malloc_memory and GB_calloc_memory increment
    // this count, and free (of a non-NULL pointer) decrements it.  realloc
    // increments the count it if is allocating a new block, but it does this
    // by calling GB_malloc_memory.

    // inuse: the # of bytes currently in use by all threads

    // maxused: the max value of inuse since the call to GrB_init

    // malloc_debug: this is used for testing only (GraphBLAS/Tcov).  If true,
    // then use malloc_debug_count for testing memory allocation and
    // out-of-memory conditions.  If malloc_debug_count > 0, the value is
    // decremented after each allocation of memory.  If malloc_debug_count <=
    // 0, the GB_*_memory routines pretend to fail; returning NULL and not
    // allocating anything.

    bool malloc_tracking ;          // true if allocations are being tracked
    int64_t nmalloc ;               // number of blocks allocated but not freed
    bool malloc_debug ;             // if true, test memory handling
    int64_t malloc_debug_count ;    // for testing memory handling
    int64_t inuse ;                 // memory space current in use
    int64_t maxused ;               // high water memory usage

}
GB_Global_struct ;

extern GB_Global_struct GB_Global ;

//------------------------------------------------------------------------------
// GB_Global access functions
//------------------------------------------------------------------------------

int      GB_Global_nthreads_max_get ( ) ;
int64_t  GB_Global_nmalloc_get ( ) ;
void     GB_Global_nmalloc_clear ( ) ;
int64_t  GB_Global_nmalloc_decrement ( ) ;
int64_t  GB_Global_nmalloc_increment ( ) ;
void     GB_Global_abort_function_set (void (* abort_function) (void)) ;
void     GB_Global_abort_function_call ( ) ;
void     GB_Global_GrB_init_called_set (bool GrB_init_called) ;
bool     GB_Global_malloc_tracking_get ( ) ;
void     GB_Global_malloc_tracking_set (bool malloc_tracking) ;
void     GB_Global_malloc_debug_set (bool malloc_debug) ;
bool     GB_Global_malloc_debug_get ( ) ;
void     GB_Global_malloc_debug_count_set (int64_t malloc_debug_count) ;
int64_t  GB_Global_inuse_get ( ) ;
void     GB_Global_inuse_clear ( ) ;
void     GB_Global_inuse_increment (int64_t s) ;
void     GB_Global_inuse_decrement (int64_t s) ;
int64_t  GB_Global_maxused_get ( ) ;
void  *  GB_Global_queue_head_get ( ) ;
void     GB_Global_queue_head_set (void *p) ;
void     GB_Global_mode_set (GrB_Mode mode) ;
GB_Sauna GB_Global_Saunas_get (int id) ;
void     GB_Global_user_multithreaded_set (bool user_multithreaded) ;

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
// Thread local storage
//------------------------------------------------------------------------------

// Thread local storage is used to to record the details of the last error
// encountered (for GrB_error).  If the user application is multi-threaded,
// each thread that calls GraphBLAS needs its own private copy of these
// variables.  Thus, this method must match the user-thread model.

#if defined (USER_POSIX_THREADS)
// thread-local storage for POSIX THREADS
extern pthread_key_t GB_thread_local_key ;

#elif defined (USER_WINDOWS_THREADS)
// for user applications that use Windows threads:
#error "Windows threads not yet supported"

#elif defined (USER_ANSI_THREADS)
// for user applications that use ANSI C11 threads:
// (this should work per the ANSI C11 specification but is not yet supported)
_Thread_local

#else
// _OPENMP, USER_OPENMP_THREADS, or USER_NO_THREADS
// This is the default.

#endif

extern char GB_thread_local_report [GB_RLEN+1] ;

// return pointer to thread-local storage
char *GB_thread_local_access ( ) ;

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

// check the descriptor and extract its contents (also gets nthreads)
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
        /* C<~NULL>=NULL since result does not depend on computing Z */ \
        return (C_replace ? GB_clear (C, Context) : GrB_SUCCESS) ;      \
    }

//------------------------------------------------------------------------------
// random number generator
//------------------------------------------------------------------------------

// return a random GrB_Index, in range 0 to 2^60
#define GB_RAND_MAX 32767

// return a random number between 0 and GB_RAND_MAX
static inline GrB_Index GB_rand15 (uint64_t *seed)
{ 
   (*seed) = (*seed) * 1103515245 + 12345 ;
   return (((*seed) / 65536) % (GB_RAND_MAX + 1)) ;
}

// return a random GrB_Index, in range 0 to 2^60
static inline GrB_Index GB_rand (uint64_t *seed)
{ 
    GrB_Index i = GB_rand15 (seed) ;
    i = GB_RAND_MAX * i + GB_rand15 (seed) ;
    i = GB_RAND_MAX * i + GB_rand15 (seed) ;
    i = GB_RAND_MAX * i + GB_rand15 (seed) ;
    return (i) ;
}

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
#define GB_PENDING(A) ((A) != NULL && (A)->n_pending > 0)

// true if a matrix is allowed to have pending tuples
#define GB_PENDING_OK(A) ((A) != NULL && (A)->n_pending >= 0)

// true if a matrix has zombies
#define GB_ZOMBIES(A) ((A) != NULL && (A)->nzombies > 0)

// true if a matrix is allowed to have zombies
#define GB_ZOMBIES_OK(A) ((A) != NULL && (A)->nzombies >= 0)

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
    if (GB_PENDING (A) || GB_ZOMBIES (A)) GB_WAIT_MATRIX (A) ;          \
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
// type-specific macros
//------------------------------------------------------------------------------

// returns the largest possible value for a given type
#define GB_PLUS_INF(type)         \
    _Generic                      \
    (                             \
        (type),                   \
        bool     : true         , \
        int8_t   : INT8_MAX     , \
        uint8_t  : UINT8_MAX    , \
        int16_t  : INT16_MAX    , \
        uint16_t : UINT16_MAX   , \
        int32_t  : INT32_MAX    , \
        uint32_t : UINT32_MAX   , \
        int64_t  : INT64_MAX    , \
        uint64_t : UINT64_MAX   , \
        float    : INFINITY     , \
        double   : INFINITY       \
    )

// returns the smallest possible value for a given type
#define GB_MINUS_INF(type)        \
    _Generic                      \
    (                             \
        (type),                   \
        bool     : false        , \
        int8_t   : INT8_MIN     , \
        uint8_t  : 0            , \
        int16_t  : INT16_MIN    , \
        uint16_t : 0            , \
        int32_t  : INT32_MIN    , \
        uint32_t : 0            , \
        int64_t  : INT64_MIN    , \
        uint64_t : 0            , \
        float    : -INFINITY    , \
        double   : -INFINITY      \
    )

// true if the type is signed
#define GB_IS_SIGNED(type)        \
    _Generic                      \
    (                             \
        (type),                   \
        bool     : false        , \
        int8_t   : true         , \
        uint8_t  : false        , \
        int16_t  : true         , \
        uint16_t : false        , \
        int32_t  : true         , \
        uint32_t : false        , \
        int64_t  : true         , \
        uint64_t : false        , \
        float    : true         , \
        double   : true           \
    )

// true if the type is integer (boolean is not integer)
#define GB_IS_INTEGER(type)       \
    _Generic                      \
    (                             \
        (type),                   \
        bool     : false        , \
        int8_t   : true         , \
        uint8_t  : true         , \
        int16_t  : true         , \
        uint16_t : true         , \
        int32_t  : true         , \
        uint32_t : true         , \
        int64_t  : true         , \
        uint64_t : true         , \
        float    : false        , \
        double   : false          \
    )

// true if the type is boolean
#define GB_IS_BOOLEAN(type)       \
    _Generic                      \
    (                             \
        (type),                   \
        bool     : true         , \
        int8_t   : false        , \
        uint8_t  : false        , \
        int16_t  : false        , \
        uint16_t : false        , \
        int32_t  : false        , \
        uint32_t : false        , \
        int64_t  : false        , \
        uint64_t : false        , \
        float    : false        , \
        double   : false          \
    )

// true if the type is float or double
#define GB_IS_FLOAT(type)         \
    _Generic                      \
    (                             \
        (type),                   \
        bool     : false        , \
        int8_t   : false        , \
        uint8_t  : false        , \
        int16_t  : false        , \
        uint16_t : false        , \
        int32_t  : false        , \
        uint32_t : false        , \
        int64_t  : false        , \
        uint64_t : false        , \
        float    : true         , \
        double   : true           \
    )

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

// signed integer division x/zero: special cases for 0/0, -(neg)/0, (pos)/0
#define GB_SIGNED_INTEGER_DIVISION_BY_ZERO(x)                               \
(                                                                           \
    ((x) == 0) ?                                                            \
    (                                                                       \
        /* zero divided by zero gives 'Nan' */                              \
        0                                                                   \
    )                                                                       \
    :                                                                       \
    (                                                                       \
        /* x/0 and x is nonzero */                                          \
        ((x) < 0) ?                                                         \
        (                                                                   \
            /* x is negative: x/0 gives '-Inf' */                           \
            GB_MINUS_INF (x)                                                \
        )                                                                   \
        :                                                                   \
        (                                                                   \
            /* x is positive: x/0 gives '+Inf' */                           \
            GB_PLUS_INF (x)                                                 \
        )                                                                   \
    )                                                                       \
)

// signed integer division x/y: special cases for y=-1 and y=0
#define GB_SIGNED_INTEGER_DIVISION(x,y)                                     \
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
            GB_SIGNED_INTEGER_DIVISION_BY_ZERO (x)                          \
        )                                                                   \
        :                                                                   \
        (                                                                   \
            /* normal case for signed integer division */                   \
            (x) / (y)                                                       \
        )                                                                   \
    )                                                                       \
)

// unsigned integer division x/zero: special cases for 0/0 and (pos)/0
#define GB_UNSIGNED_INTEGER_DIVISION_BY_ZERO(x)                             \
(                                                                           \
    ((x) == 0) ?                                                            \
    (                                                                       \
        /* zero divided by zero gives 'Nan' */                              \
        0                                                                   \
    )                                                                       \
    :                                                                       \
    (                                                                       \
        /* x is positive: x/0 gives '+Inf' */                               \
        GB_PLUS_INF (x)                                                     \
    )                                                                       \
)                                                                           \

// unsigned integer division x/y: special case for y=0
#define GB_UNSIGNED_INTEGER_DIVISION(x,y)                                   \
(                                                                           \
    ((y) == 0) ?                                                            \
    (                                                                       \
        /* x/0 */                                                           \
        GB_UNSIGNED_INTEGER_DIVISION_BY_ZERO (x)                            \
    )                                                                       \
    :                                                                       \
    (                                                                       \
        /* normal case for unsigned integer division */                     \
        (x) / (y)                                                           \
    )                                                                       \
)

// x/y when x and y are signed or unsigned integers
#define GB_IDIV(x,y)                                                        \
(                                                                           \
    (GB_IS_SIGNED (x)) ?                                                    \
    (                                                                       \
        GB_SIGNED_INTEGER_DIVISION (x,y)                                    \
    )                                                                       \
    :                                                                       \
    (                                                                       \
        GB_UNSIGNED_INTEGER_DIVISION (x,y)                                  \
    )                                                                       \
)

// 1/y when y is signed or unsigned integer
#define GB_IMINV(y)                                                         \
(                                                                           \
    ((y) == 0) ?                                                            \
    (                                                                       \
        /* 1/0 */                                                           \
        GB_PLUS_INF (y)                                                     \
    )                                                                       \
    :                                                                       \
    (                                                                       \
        /* normal case for integer minv, signed or unsigned */              \
        1 / (y)                                                             \
    )                                                                       \
)

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

// cast a value x to the type of z
#define GB_CAST(z,x)                                                        \
{                                                                           \
    if (GB_IS_INTEGER (z) && GB_IS_FLOAT (x))                               \
    {                                                                       \
        switch (fpclassify ((double) (x)))                                  \
        {                                                                   \
            case FP_NAN:                                                    \
                z = 0 ;                                                     \
                break ;                                                     \
            case FP_INFINITE:                                               \
                z = ((x) > 0) ? GB_PLUS_INF (z) : GB_MINUS_INF (z) ;        \
                break ;                                                     \
            default:                                                        \
                z = (x) ;                                                   \
                break ;                                                     \
        }                                                                   \
    }                                                                       \
    else                                                                    \
    {                                                                       \
        /* trust the built-in typecast */                                   \
        z = (x) ;                                                           \
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
//    X [pleft ... original_pright-1] > i holds, and pleft-1 == pright
// If X has no duplicates, then whether or not i is found,
//    X [original_pleft ... pleft-1] < i and
//    X [pleft ... original_pright-1] >= i holds.
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

//------------------------------------------------------------------------------
// index lists I and J
//------------------------------------------------------------------------------

// kind of index list, Ikind and Jkind:
#define GB_ALL 0
#define GB_RANGE 1
#define GB_STRIDE 2
#define GB_LIST 4

void GB_ijlength            // get the length and kind of an index list I
(
    const GrB_Index *I,     // list of indices (actual or implicit)
    const int64_t ni,       // length I, or special
    const int64_t limit,    // indices must be in the range 0 to limit-1
    int64_t *nI,            // actual length of I
    int *Ikind,             // kind of I: GB_ALL, GB_RANGE, GB_STRIDE, GB_LIST
    int64_t Icolon [3]      // begin:inc:end for all but GB_LIST
) ;

GrB_Info GB_ijproperties        // check I and determine its properties
(
    const GrB_Index *I,         // list of indices, or special
    const int64_t ni,           // length I, or special
    const int64_t nI,           // actual length from GB_ijlength
    const int64_t limit,        // I must be in the range 0 to limit-1
    const int64_t Ikind,        // kind of I, from GB_ijlength
    const int64_t Icolon [3],   // begin:inc:end from GB_ijlength
    bool *I_is_unsorted,        // true if I is out of order
    bool *I_is_contig,          // true if I is a contiguous list, imin:imax
    int64_t *imin_result,       // min (I)
    int64_t *imax_result,       // max (I)
    bool is_I,                  // true if I, false if J (debug only)
    GB_Context Context
) ;

GrB_Info GB_ijsort
(
    const GrB_Index *I, // index array of size ni
    int64_t *p_ni,      // input: size of I, output: number of indices in I2
    GrB_Index **p_I2,   // output array of size ni, where I2 [0..ni2-1]
                        // contains the sorted indices with duplicates removed.
    GB_Context Context
) ;

// given k, return the kth item i = I [k] in the list
static inline int64_t GB_ijlist     // get the kth item in a list of indices
(
    const GrB_Index *I,         // list of indices
    const int64_t k,            // return i = I [k], the kth item in the list
    const int Ikind,            // GB_ALL, GB_RANGE, GB_STRIDE, or GB_LIST
    const int64_t Icolon [3]    // begin:inc:end for all but GB_LIST
)
{
    if (Ikind == GB_ALL)
    { 
        // I is ":"
        return (k) ;
    }
    else if (Ikind == GB_RANGE)
    { 
        // I is begin:end
        return (Icolon [GxB_BEGIN] + k) ;
    }
    else if (Ikind == GB_STRIDE)
    { 
        // I is begin:inc:end
        // note that iinc can be negative or even zero
        return (Icolon [GxB_BEGIN] + k * Icolon [GxB_INC]) ;
    }
    else // Ikind == GB_LIST
    { 
        ASSERT (Ikind == GB_LIST) ;
        ASSERT (I != NULL) ;
        return (I [k]) ;
    }
}

// given i and I, return true there is a k so that i is the kth item in I
static inline bool GB_ij_is_in_list // determine if i is in the list I
(
    const GrB_Index *I,         // list of indices for GB_LIST
    const int64_t nI,           // length of I
    int64_t i,                  // find i = I [k] in the list
    const int Ikind,            // GB_ALL, GB_RANGE, GB_STRIDE, or GB_LIST
    const int64_t Icolon [3]    // begin:inc:end for all but GB_LIST
)
{
    if (Ikind == GB_ALL)
    { 
        // I is ":", all indices are in the sequence
        return (true) ;
    }
    else if (Ikind == GB_RANGE)
    { 
        // I is begin:end
        int64_t b = Icolon [GxB_BEGIN] ;
        int64_t e = Icolon [GxB_END] ;
        if (i < b) return (false) ;
        if (i > e) return (false) ;
        return (true) ;
    }
    else if (Ikind == GB_STRIDE)
    {
        // I is begin:inc:end
        // note that inc can be negative or even zero
        int64_t b   = Icolon [GxB_BEGIN] ;
        int64_t inc = Icolon [GxB_INC] ;
        int64_t e   = Icolon [GxB_END] ;
        if (inc == 0)
        { 
            // I is empty if inc is zero, so i is not in I
            return (false) ;
        }
        else if (inc > 0)
        { 
            // forward direction, increment is positive
            // I = b:inc:e = [b, b+inc, b+2*inc, ..., e]
            if (i < b) return (false) ;
            if (i > e) return (false) ;
            // now i is in the range [b ... e]
            ASSERT (b <= i && i <= e) ;
            i = i - b ;
            ASSERT (0 <= i && i <= (e-b)) ;
            // the sequence I-b = [0, inc, 2*inc, ... e-b].
            // i is in the sequence if i % inc == 0
            return (i % inc == 0) ;
        }
        else // inc < 0
        { 
            // backwards direction, increment is negative
            inc = -inc ;
            // now inc is positive
            ASSERT (inc > 0) ;
            // I = b:(-inc):e = [b, b-inc, b-2*inc, ... e]
            if (i > b) return (false) ;
            if (i < e) return (false) ;
            // now i is in the range of the sequence, [b down to e]
            ASSERT (e <= i && i <= b) ;
            i = b - i ;
            ASSERT (0 <= i && i <= (b-e)) ;
            // b-I = 0:(inc):(b-e) = [0, inc, 2*inc, ... (b-e)]
            // i is in the sequence if i % inc == 0
            return (i % inc == 0) ;
        }
    }
    else // Ikind == GB_LIST
    { 
        ASSERT (Ikind == GB_LIST) ;
        ASSERT (I != NULL) ;
        // search for i in the sorted list I
        bool found ;
        int64_t pleft = 0 ;
        int64_t pright = nI-1 ;
        GB_BINARY_SEARCH (i, I, pleft, pright, found) ;
        return (found) ;
    }
}

//------------------------------------------------------------------------------
// GB_bracket_left
//------------------------------------------------------------------------------

// Given a sorted list X [kleft:kright], and a range imin:..., modify kleft so
// that the smaller sublist X [kleft:kright] contains the range imin:...

static inline void GB_bracket_left
(
    const int64_t imin,
    const int64_t *restrict X,  // input list is in X [kleft:kright]
    int64_t *kleft,
    const int64_t kright
)
{
    // tighten kleft
    int64_t len = kright - (*kleft) + 1 ;
    if (len > 0 && X [(*kleft)] < imin)
    { 
        // search for imin in X [kleft:kright]
        int64_t pleft = (*kleft) ;
        int64_t pright = kright ;
        GB_BINARY_TRIM_SEARCH (imin, X, pleft, pright) ;
        (*kleft) = pleft ;
    }
}

//------------------------------------------------------------------------------
// GB_bracket_right
//------------------------------------------------------------------------------

// Given a sorted list X [kleft:kright], and a range ...:imax, modify kright so
// that the smaller sublist X [kleft:kright] contains the range ...:imax.

static inline void GB_bracket_right
(
    const int64_t imax,
    const int64_t *restrict X,  // input list is in X [kleft:kright]
    const int64_t kleft,
    int64_t *kright
)
{
    // tighten kright
    int64_t len = (*kright) - kleft + 1 ;
    if (len > 0 && imax < X [(*kright)])
    { 
        // search for imax in X [kleft:kright]
        int64_t pleft = kleft ;
        int64_t pright = (*kright) ;
        GB_BINARY_TRIM_SEARCH (imax, X, pleft, pright) ;
        (*kright) = pleft ;
    }
}

//------------------------------------------------------------------------------
// GB_bracket
//------------------------------------------------------------------------------

// Given a sorted list X [kleft:kright], and a range imin:imax, find the
// kleft_new and kright_new so that the smaller sublist X
// [kleft_new:kright_new] contains the range imin:imax.

// Zombies are not tolerated.

static inline void GB_bracket
(
    const int64_t imin,         // search for entries in the range imin:imax
    const int64_t imax,
    const int64_t *restrict X,  // input list is in X [kleft:kright]
    const int64_t kleft_in,
    const int64_t kright_in,
    int64_t *kleft_new,         // output list is in X [kleft_new:kright_new]
    int64_t *kright_new
)
{ 

    int64_t kleft  = kleft_in ;
    int64_t kright = kright_in ;

    if (imin > imax)
    { 
        // imin:imax is empty, make X [kleft:kright] empty
        (*kleft_new ) = kleft ;
        (*kright_new) = kleft-1 ;
        return ;
    }

    // Find kleft and kright so that X [kleft:kright] contains all of imin:imax

    // tighten kleft
    GB_bracket_left (imin, X, &kleft, kright) ;

    // tighten kright
    GB_bracket_right (imax, X, kleft, &kright) ;

    // list has been trimmed
    ASSERT (GB_IMPLIES (kleft > kleft_in, X [kleft-1] < imin)) ;
    ASSERT (GB_IMPLIES (kright < kright_in, imax < X [kright+1])) ;

    // return result
    (*kleft_new ) = kleft ;
    (*kright_new) = kright ;
}

//------------------------------------------------------------------------------
// NaN handling
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

// for floating point:
// #define GB_FABS(x) (((x) >= 0) ? (x) : (-(x)))

// suitable for integers, and non-NaN floating point:
#define GB_IMAX(x,y) (((x) > (y)) ? (x) : (y))
#define GB_IMIN(x,y) (((x) < (y)) ? (x) : (y))

// for floating-point, same as min(x,y,'includenan') and max(...) in MATLAB
// #define GB_FMIN(x,y) ((isnan (x) || isnan (y)) ? NAN : GB_IMIN (x,y))
// #define GB_FMAX(x,y) ((isnan (x) || isnan (y)) ? NAN : GB_IMAX (x,y))

// for floating-point, same as min(x,y,'omitnan') and max(...) in MATLAB
// #define GB_FMAX(x,y) ((isnan (x)) ? (y) : ((isnan (y)) ? (x) : GB_IMAX(x,y)))
// #define GB_FMIN(x,y) ((isnan (x)) ? (y) : ((isnan (y)) ? (x) : GB_IMIN(x,y)))

//------------------------------------------------------------------------------
// GB_lookup: find k so that j == Ah [k]
//------------------------------------------------------------------------------

// Given a standard or hypersparse matrix, find k so that j == Ah [k], if it
// appears in the list.  k is not needed by the caller, just the variables
// pstart, pend, pleft, and found.

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


//==============================================================================
// GrB_Matrix iterators and related functions
//==============================================================================

// There are two distinct kinds of matrix iterators.

// GBI_single_iterator:

//      The simplest one is a GBI_single_iterator.  It controls the iteration
//      over the vectors of a single matrix, which can be in any format
//      (standard, hypersparse, slice, or hyperslice).  It is easily
//      parallelizable if the iterations are independent, or for
//      reduction-style loops via the appropriate #pragmas.

// GBIk_multi_iterator:

//      The GBIk_multi_iterator controls the interation over 2 or 3 objects, one
//      of which may be an expanded scalar, and the others are standard or
//      hypersparse matrices, in any combination.  It iterates over the set
//      union of 2 or more objects, and is not suitable for a parallel for
//      loop.  This iterator cannot be used to iterate over the vectors of a
//      slice or hyperslice.

//------------------------------------------------------------------------------
// GBI_single_iterator: iterate over the vectors of a matrix
//------------------------------------------------------------------------------

// The Iter->* content of a GBIk_multi_iterator or GBI_single_iterator is
// accessed only in this file.  All typedefs, functions, and macros that
// operate on the SuiteSparse:GraphBLAS iterator have names that start with the
// GBI prefix.  For both kinds of iterators, the A->h and A->p components of
// the matrices may not change during the iteration.

// The single-matrix iterator, GBI_for_each_vector (A) can handle any of the
// four cases: standard, hypersparse, slice, or hyperslice.  The comments below
// assume A is in CSC format.

#ifdef for_comments_only    // only so vim will add color to the code below:

    // The GBI_for_each_vector (A) macro, which uses the GBI_single_iterator,
    // the two functions GBI1_init and GBI1_start, and the macro
    // GBI_jth_iteration can do any one of the 4 following for loops, depending
    // on whether A is standard, hypersparse, a slice, or a hyperslice.

    // A->vdim: the vector dimension of A (ncols(A))
    // A->nvec: # of vectors that appear in A.  For the hypersparse case,
    //          these are the number of column indices in Ah [0..nvec-1], since
    //          A is CSC.  For all cases, Ap [0...nvec] are the pointers.

    //--------------------
    // (1) standard     // A->is_hyper == false, A->is_slice == false
                        // A->nvec == A->vdim

        for (k = 0 ; k < A->nvec ; k++)
        {
            j = k ;
            // operate on column A(:,j)
            for (p = Ap [j] ; p < Ap [j+1] ; p++)
            {
                // A(i,j) has row i = Ai [p], value aij = Ax [p]
            }
        }

    //--------------------
    // (2) hypersparse  // A->is_hyper == true, A->is_slice == false
                        // A->nvec <= A->dim

        for (k = 0 ; k < A->nvec ; k++)
        {
            j = A->h [k]
            // operate on column A(:,j)
            for (p = Ap [k] ; p < Ap [k+1] ; p++)
            {
                // A(i,j) has row i = Ai [p], value aij = Ax [p]
            }
        }

    //--------------------
    // (3) slice, of another standard matrix S. (FUTURE)
                        // A->i == S->i, A->x == S->x
                        // A->p = S->p + A->hfirst, A->h is NULL
                        // A->nvec <= A->vdim == S->vdim

        for (k = 0 ; k < A->nvec ; k++)
        {
            j = A->hfirst + k ;
            ASSERT (S->p [j] == Ap [k]) ;
            // operate on column A(:,j), which is also S (:,j)
            for (p = Ap [k] ; p < Ap [k+1] ; p++)
            {
                // A(i,j) has row i = Ai [p], value aij = Ax [p]
                // This is identical to S(i,j)
            }
        }

    //--------------------
    // (4) hyperslice, of another hypersparse matrix S (FUTURE)
                        // A->i == S->i, A->x == S->x, A->p = S->p + kfirst,
                        // A->h == S->h + kfirst where A(:,0) is the same
                        // column as S->h [kfirst].  kfirst is not kept.
                        // A->nvec <= A->vdim == S->vdim

        for (k = 0 ; k < A->nvec ; k++)
        {
            j = A->h [k] ;
            // operate on column A(:,j), which is also S (:,j)
            for (p = Ap [k] ; p < Ap [k+1] ; p++)
            {
                // A(i,j) has row i = Ai [p], value aij = Ax [p].
                // This is identical to S(i,j)
            }
        }

    //--------------------
    // all of the above: via GBI_for_each_vector (A)
                        // are done with a single iterator that selects
                        // the iteration method based on the format of A.

        GBI_for_each_vector (A)
        {
            // get A(:,j)
            GBI_jth_iteration (j, pstart, pend)
            // operate on column A(:,j)
            for (p = pstart ; p < pend ; p++)
            {
                // A(i,j) has row i = Ai [p], value aij = Ax [p].
            }
        }

#endif

//------------------------------------------------------------------------------
// GBI_single_iterator: iterate over the vectors of a single matrix
//------------------------------------------------------------------------------

// The matrix may be standard sparse, hypersparse, slice, or hyperslice.

typedef struct
{
    const int64_t *restrict p ; // vector pointer A->p of A
    const int64_t *restrict h ; // A->h: hyperlist of vectors in A
    int64_t nvec ;              // A->nvec: number of vectors in A
    bool is_hyper ;             // true if A  is hypersparse
//  FUTURE:: slice
//  bool is_slice ;             // true if A [0] is a slice
//  int64_t hfirst ;            // A->hfirst: first vector in slice A

} GBI_single_iterator ;

//----------------------------------------
// GBI1_init: initialize a GBI_single_iterator
//----------------------------------------

static inline void GBI1_init
(
    GBI_single_iterator *Iter,
    const GrB_Matrix A
)
{ 
    // load the content of A into the iterator
    Iter->is_hyper = A->is_hyper ;
    Iter->p = A->p ;
    Iter->h = A->h ;
    Iter->nvec = A->nvec ;
//  Iter->is_slice = A->is_slice ;          // FUTURE::
//  Iter->hfirst = A->hfirst ;
}

//----------------------------------------
// GBI1_start: start the kth iteration for GBI_single_iterator
//----------------------------------------

static inline void GBI1_start
(
    int64_t Iter_k,
    GBI_single_iterator *Iter,
    int64_t *j,
    int64_t *pstart,
    int64_t *pend
)
{

#if 0
    // FUTURE:: slice and hyperslice
    if (Iter->is_slice)
    {
        // get next vector from A
        if (Iter->is_hyper)
        {
            // A is a hyperslice of a hypersparse matrix
            (*j) = Iter->h [Iter_k] ;
        }
        else
        {
            // A is a slice of a standard matrix
            (*j) = (Iter->hfirst) + Iter_k ;
        }
    }
    else
#endif
    {
        if (Iter->is_hyper)
        { 
            // A is a hypersparse matrix
            // get next vector from A
            (*j) = Iter->h [Iter_k] ;
        }
        else
        { 
            // A is a standard matrix
            // get next vector from A
            (*j) = Iter_k ;
        }
    }

    // get the start and end of the next vector of A
    (*pstart) = Iter->p [Iter_k  ] ;
    (*pend)   = Iter->p [Iter_k+1] ;
}

// iterate over one matrix A (sparse, hypersparse, slice, or hyperslice)
// with a named iterator
#define GBI_for_each_vector_with_iter(Iter,A)                               \
    GBI_single_iterator Iter ;                                              \
    GBI1_init (&Iter, A) ;                                                  \
    for (int64_t Iter ## _k = 0 ; Iter ## _k < Iter.nvec ; Iter ## _k++)

// iterate over one matrix A (sparse, hypersparse, slice, or hyperslice)
// with the iterator named "Iter"
#define GBI_for_each_vector(A) GBI_for_each_vector_with_iter (Iter,A)

// get the column at the current iteration, and the start/end pointers
// of column j in the matrix A
#define GBI_jth_iteration_with_iter(Iter,j0,pstart0,pend0)                  \
    int64_t j0, pstart0, pend0 ;                                            \
    GBI1_start (Iter ## _k, &Iter, &j0, &pstart0, &pend0) ;

#define GBI_jth_iteration(j0,pstart0,pend0)                                 \
    GBI_jth_iteration_with_iter(Iter,j0,pstart0,pend0)

// iterate over a vector of a single matrix
#define GBI_for_each_entry(j,p,pend)                                        \
    GBI_jth_iteration (j, p, pend) ;                                        \
    for ( ; (p) < (pend) ; (p)++)

//------------------------------------------------------------------------------
// GBIk_multi_iterator: iterate over vectors of multiple matrices
//------------------------------------------------------------------------------

// Any of the matrices may be standard sparse or hypersparse.  None can be a
// slice or hyperslice.

typedef struct
{
    bool any_hyper ;            // true if any matrix is hypersparse
    bool is_hyper [3] ;         // true if A [0..2] is hypersparse

    int64_t j ;                 // name of current vector
    int64_t vdim ;              // A->vdim: number of vectors in A [0..2]

    const int64_t *p [3] ;      // vector pointers A->p of A [0..2]
    int64_t pstart [3] ;        // start of current vector
    int64_t pend [3] ;          // one past the end of current vector

    int64_t nvec [3] ;          // A->nvec: number of vectors in A [0..2]
    const int64_t *h [3] ;      // A->h: hyperlist of vectors in A [0..2]
    int64_t k [3] ;             // current index into hyperlist h [0..2]
    int64_t jj [3] ;            // current vector jj [.] = h [k [.]]

} GBIk_multi_iterator ;

//------------------------------------------------------------------------------
// GBI#_init
//------------------------------------------------------------------------------

// Initialize an iterator for two or three matrices (one of which may be an
// expanded scalar).  Extracts the vector content (is_hyper, vdim, p, h, and
// nvec) of A, B, and/or C and caches them in the GBIk_multi_iterator Iter, and
// initializes the counters Iter->j and Iter->k [0..2].  The GBI#_init
// functions are the only inline functions that directly access the matrix
// content A->(...) for A, B, and/or C.  All other iterator functions and
// macros access the cached copies in Iter.

static inline void GBI3_init
(
    GBIk_multi_iterator *Iter,
    const GrB_Matrix A,
    const GrB_Matrix B,
    const GrB_Matrix C
)
{ 
    // A, B, and C must have the same vdim.  vlen can vary
    ASSERT (A->vdim == B->vdim) ;
    ASSERT (A->vdim == C->vdim) ;

    // GBI3_for_each_vector (A,B,C) is not used if any matrix is a slice
    // ASSERT (!A->is_slice && !B->is_slice && !C->is_slice) ;

    Iter->any_hyper = (A->is_hyper || B->is_hyper || C->is_hyper) ;
    Iter->j = 0 ;
    Iter->vdim = A->vdim ;

    Iter->p [0] = A->p ;
    Iter->p [1] = B->p ;
    Iter->p [2] = C->p ;

    Iter->h [0] = A->h ;
    Iter->h [1] = B->h ;
    Iter->h [2] = C->h ;

    Iter->jj [0] = 0 ;
    Iter->jj [1] = 0 ;
    Iter->jj [2] = 0 ;

    Iter->k [0] = 0 ;
    Iter->k [1] = 0 ;
    Iter->k [2] = 0 ;

    Iter->pstart [0] = 0 ;
    Iter->pstart [1] = 0 ;
    Iter->pstart [2] = 0 ;

    Iter->pend [0] = 0 ;
    Iter->pend [1] = 0 ;
    Iter->pend [2] = 0 ;

    Iter->nvec [0] = A->nvec ;
    Iter->nvec [1] = B->nvec ;
    Iter->nvec [2] = C->nvec ;

    Iter->is_hyper [0] = A->is_hyper ;
    Iter->is_hyper [1] = B->is_hyper ;
    Iter->is_hyper [2] = C->is_hyper ;

}

static inline void GBI2_init
(
    GBIk_multi_iterator *Iter,
    const GrB_Matrix A,
    const GrB_Matrix B
)
{ 
    // A and B must have the same vdim.  vlen can vary
    ASSERT (A->vdim == B->vdim) ;

    // GBI2_for_each_vector (A,B) is not used if any matrix is a slice
    // ASSERT (!A->is_slice && !B->is_slice) ;

    Iter->any_hyper = (A->is_hyper || B->is_hyper) ;
    Iter->j = 0 ;
    Iter->vdim = A->vdim ;

    Iter->p [0] = A->p ;
    Iter->p [1] = B->p ;
//  Iter->p [2] = NULL ;

    Iter->h [0] = A->h ;
    Iter->h [1] = B->h ;
//  Iter->h [2] = NULL ;

    Iter->jj [0] = 0 ;
    Iter->jj [1] = 0 ;
//  Iter->jj [2] = 0 ;

    Iter->k [0] = 0 ;
    Iter->k [1] = 0 ;
//  Iter->k [2] = 0 ;

    Iter->pstart [0] = 0 ;
    Iter->pstart [1] = 0 ;
//  Iter->pstart [2] = 0 ;

    Iter->pend [0] = 0 ;
    Iter->pend [1] = 0 ;
//  Iter->pend [2] = 0 ;

    Iter->nvec [0] = A->nvec ;
    Iter->nvec [1] = B->nvec ;
//  Iter->nvec [2] = 0 ;

    Iter->is_hyper [0] = A->is_hyper ;
    Iter->is_hyper [1] = B->is_hyper ;
//  Iter->is_hyper [2] = false ;

}

static inline void GBI3s_init
(
    GBIk_multi_iterator *Iter,
    const GrB_Matrix A,
    const GrB_Matrix B
)
{ 
    GBI2_init (Iter, A, B) ;
}

static inline void GBI2s_init
(
    GBIk_multi_iterator *Iter,
    const GrB_Matrix A
)
{ 

    // GBI2s_for_each_vector (A,scalar) is not used if the matrix is a slice
    // ASSERT (!A->is_slice)

    Iter->any_hyper = (A->is_hyper) ;
    Iter->j = 0 ;
    Iter->vdim = A->vdim ;

    Iter->p [0] = A->p ;
//  Iter->p [1] = NULL ;
//  Iter->p [2] = NULL ;

    Iter->h [0] = A->h ;
//  Iter->h [1] = NULL ;
//  Iter->h [2] = NULL ;

    Iter->jj [0] = 0 ;
//  Iter->jj [1] = 0 ;
//  Iter->jj [2] = 0 ;

    Iter->k [0] = 0 ;
//  Iter->k [1] = 0 ;
//  Iter->k [2] = 0 ;

    Iter->pstart [0] = 0 ;
//  Iter->pstart [1] = 0 ;
//  Iter->pstart [2] = 0 ;

    Iter->pend [0] = 0 ;
//  Iter->pend [1] = 0 ;
//  Iter->pend [2] = 0 ;

    Iter->nvec [0] = A->nvec ;
//  Iter->nvec [1] = 0 ;
//  Iter->nvec [2] = 0 ;

    Iter->is_hyper [0] = A->is_hyper ;
//  Iter->is_hyper [1] = false ;
//  Iter->is_hyper [2] = false ;

}

//------------------------------------------------------------------------------
// GBI#_while for GBIk_multi_iterator
//------------------------------------------------------------------------------

// The GBI#_while functions test the condition for a loop over one (plus an
// expanded scalar), two, or three matrices and prepare to access the vector at
// each matrix.  The inline GBI#_while functions are only used in the GB_*each*
// macros.  These macros are not used for iterating over slices.

// get the index of the next vector in a given matrix
#define GBIk_jjget(Iter,matrix)                                              \
{                                                                           \
    if (!(Iter->is_hyper [matrix]))                                         \
    {                                                                       \
        /* the matrix is not hypersparse */                                 \
        Iter->jj [matrix] = Iter->k [matrix] ;                              \
    }                                                                       \
    else if (Iter->k [matrix] < Iter->nvec [matrix])                        \
    {                                                                       \
        /* jj is the kth non-empty vector in the matrix */                  \
        Iter->jj [matrix] = (Iter->h [matrix]) [Iter->k [matrix]] ;         \
    }                                                                       \
    else                                                                    \
    {                                                                       \
        /* list of non-empty vectors exhausted; return sentinel value */    \
        Iter->jj [matrix] = Iter->vdim ;                                    \
    }                                                                       \
}

// Get the pointers to the next vector A [matrix] (:,j), sparse or hypersparse.
// The vector appears in Ai and Ax [Iter->pstart [matrix]...Iter->pend
// [matrix]-1].
#define GBIk_phget(Iter,matrix)                                              \
{                                                                           \
    if (!(Iter->is_hyper [matrix]))                                         \
    {                                                                       \
        /* A (:,j) is the jth vector in the non-hypersparse matrix */       \
        GBIk_pget (Iter, matrix) ;                                           \
    }                                                                       \
    else if (Iter->j == Iter->jj [matrix])                                  \
    {                                                                       \
        /* A (:,j) is the kth vector in the hypersparse matrix */           \
        GBIk_pget_hyper (Iter, matrix) ;                                     \
    }                                                                       \
    else                                                                    \
    {                                                                       \
        /* A (:,j) does not appear in the hypersparse matrix */             \
        Iter->pstart [matrix] = -1 ;                                        \
        Iter->pend   [matrix] = -1 ;                                        \
    }                                                                       \
}

//----------------------------------------
// get the start and end of A [matrix] (:,j)
//----------------------------------------

// get the pointers to the next vector A (:,j), not hypersparse
#define GBIk_pget(Iter,matrix)                                               \
{                                                                           \
    Iter->pstart [matrix] = (Iter->p [matrix]) [Iter->j  ] ;                \
    Iter->pend   [matrix] = (Iter->p [matrix]) [Iter->j+1] ;                \
}

// get the pointers to the kth vector A (:,j), hypersparse
#define GBIk_pget_hyper(Iter,matrix)                                         \
{                                                                           \
    Iter->pstart [matrix] = (Iter->p [matrix]) [Iter->k [matrix]  ] ;       \
    Iter->pend   [matrix] = (Iter->p [matrix]) [Iter->k [matrix]+1] ;       \
}

// The GBI2* and GBI3* methods can iterate over standard or hypersparse
// matrices, one of which may be an expanded scalar.  They cannot iterate over
// any slice or hyperslice matrix.

//----------------------------------------
// while iterating over 3 matrices
//----------------------------------------

static inline bool GBI3_while
(
    GBIk_multi_iterator *Iter
)
{

    if (Iter->any_hyper)
    { 
        // get next vector from A, B and/or C
        GBIk_jjget (Iter, 0) ;
        GBIk_jjget (Iter, 1) ;
        GBIk_jjget (Iter, 2) ;
        Iter->j = GB_IMIN (Iter->jj [0], Iter->jj [1]) ;
        Iter->j = GB_IMIN (Iter->j     , Iter->jj [2]) ;

        // test if the end of the matrix has been reached
        if (Iter->j >= Iter->vdim) return (false) ;

        // get the start and end of each vector of A, B, and C
        GBIk_phget (Iter, 0) ;
        GBIk_phget (Iter, 1) ;
        GBIk_phget (Iter, 2) ;
    }
    else
    { 
        // test if the end of the matrix has been reached
        if (Iter->j >= Iter->vdim) return (false) ;

        // get the start and end of the jth vector of A, B, and C
        GBIk_pget (Iter, 0) ;
        GBIk_pget (Iter, 1) ;
        GBIk_pget (Iter, 2) ;
    }

    // end of matrix has not yet been reached
    return (true) ;
}

//----------------------------------------
// while iterating over 2 matrices and a scalar
//----------------------------------------

static inline bool GBI3s_while
(
    GBIk_multi_iterator *Iter
)
{

    // test if the end of the matrix has been reached
    if (Iter->j >= Iter->vdim) return (false) ;

    if (Iter->any_hyper)
    { 
        // get next vector from A and/or B
        GBIk_jjget (Iter, 0) ;
        GBIk_jjget (Iter, 1) ;
        GBIk_phget (Iter, 0) ;
        GBIk_phget (Iter, 1) ;
    }
    else
    { 
        // get the start and end of the jth vector of A and B
        GBIk_pget (Iter, 0) ;
        GBIk_pget (Iter, 1) ;
    }

    // end of matrix has not yet been reached
    return (true) ;
}

//----------------------------------------
// while iterating over 2 matrices
//----------------------------------------

static inline bool GBI2_while
(
    GBIk_multi_iterator *Iter
)
{

    if (Iter->any_hyper)
    { 
        // get next vector from A and/or B
        GBIk_jjget (Iter, 0) ;
        GBIk_jjget (Iter, 1) ;
        Iter->j = GB_IMIN (Iter->jj [0], Iter->jj [1]) ;

        // test if the end of the matrix has been reached
        if (Iter->j >= Iter->vdim) return (false) ;

        // get the start and end of each vector of A and B
        GBIk_phget (Iter, 0) ;
        GBIk_phget (Iter, 1) ;
    }
    else
    { 
        // test if the end of the matrix has been reached
        if (Iter->j >= Iter->vdim) return (false) ;

        // get the start and end of the jth vector of A and B
        GBIk_pget (Iter, 0) ;
        GBIk_pget (Iter, 1) ;
    }

    // end of matrix has not yet been reached
    return (true) ;
}

//----------------------------------------
// while iterating over a matrix and a scalar
//----------------------------------------

static inline bool GBI2s_while
(
    GBIk_multi_iterator *Iter
)
{

    // test if the end of the matrix has been reached
    if (Iter->j >= Iter->vdim) return (false) ;

    if (Iter->any_hyper)
    { 
        // get next vector from A
        GBIk_jjget (Iter, 0) ;
        GBIk_phget (Iter, 0) ;
    }
    else
    { 
        // get the start and end of the jth vector of A and B
        GBIk_pget (Iter, 0) ;
    }

    // end of matrix has not yet been reached
    return (true) ;
}

//----------------------------------------
// only used by GBI#_while:
//----------------------------------------

#undef GBIk_jjget
#undef GBIk_phget
#undef GBIk_pget
#undef GBIk_pget_hyper

//------------------------------------------------------------------------------
// GBI#_next
//------------------------------------------------------------------------------

// Advance to the next vector of one, two, or three matrices.  The inline
// GBI#_next functions are only used in the GB_*each* macros.

#define GBIk_jnext(Iter,matrix)                                                \
{                                                                             \
    if (!(Iter->is_hyper [matrix]))                                           \
    {                                                                         \
        /* matrix is not hypersparse, so always advance to next vector */     \
        Iter->k [matrix]++ ;                                                  \
    }                                                                         \
    else if (Iter->j == Iter->jj [matrix])                                    \
    {                                                                         \
        /* matrix is hypersparse, and Ah [k] == j; advance to next vector */  \
        ASSERT (Iter->j == (Iter->h [matrix])[Iter->k [matrix]]) ;            \
        Iter->k [matrix]++ ;                                                  \
    }                                                                         \
}

static inline void GBI3_next
(
    GBIk_multi_iterator *Iter
)
{
    if (Iter->any_hyper)
    { 
        // advance to the next vector of A, B, and/or C
        GBIk_jnext (Iter, 0) ;
        GBIk_jnext (Iter, 1) ;
        GBIk_jnext (Iter, 2) ;
    }
    else
    { 
        // A, B, and C are non-hypersparse
        Iter->j++ ;
    }
}

static inline void GBI3s_next
(
    GBIk_multi_iterator *Iter
)
{
    if (Iter->any_hyper)
    { 
        // advance to the next vector of A, and/or B
        GBIk_jnext (Iter, 0) ;
        GBIk_jnext (Iter, 1) ;
    }
    // C is an expanded scalar, so it always advances
    Iter->j++ ;
}

static inline void GBI2_next
(
    GBIk_multi_iterator *Iter
)
{
    if (Iter->any_hyper)
    { 
        // advance to the next vector of A and/or B
        GBIk_jnext (Iter, 0) ;
        GBIk_jnext (Iter, 1) ;
    }
    else
    { 
        // both A and B are non-hypersparse
        Iter->j++ ;
    }
}

static inline void GBI2s_next
(
    GBIk_multi_iterator *Iter
)
{
    if (Iter->any_hyper)
    { 
        // advance to the next vector of A
        GBIk_jnext (Iter, 0) ;
    }
    // B is an expanded scalar, so it always advances
    Iter->j++ ;
}

// the GBIk_jnext* macros are only used in the GBI#_next functions above.
#undef GBIk_jnext

//------------------------------------------------------------------------------
// GBI#_jth_iteration
//------------------------------------------------------------------------------

// Get the column at the current iteration, and the start/end pointers
// of column j in the three matrices A, B, and C.
#define GBI3_jth_iteration(Iter,j_,pstart0,pend0,pstart1,pend1,pstart2,pend2)  \
    int64_t                                                            \
    j_      = Iter.j,                                                  \
    pstart0 = Iter.pstart [0],                                         \
    pend0   = Iter.pend   [0],                                         \
    pstart1 = Iter.pstart [1],                                         \
    pend1   = Iter.pend   [1],                                         \
    pstart2 = Iter.pstart [2],                                         \
    pend2   = Iter.pend   [2]

// get the column at the current iteration, and the start/end pointers
// of column j in the two matrices A and B
#define GBI2_jth_iteration(Iter,j_,pstart0,pend0,pstart1,pend1)        \
    int64_t                                                            \
    j_      = Iter.j,                                                  \
    pstart0 = Iter.pstart [0],                                         \
    pend0   = Iter.pend   [0],                                         \
    pstart1 = Iter.pstart [1],                                         \
    pend1   = Iter.pend   [1]

// these are the same:
#define GBI3s_jth_iteration(Iter,j_,pstart0,pend0,pstart1,pend1)       \
         GBI2_jth_iteration(Iter,j_,pstart0,pend0,pstart1,pend1)

#define GBI2s_jth_iteration(Iter,j_,pstart0,pend0)                     \
    int64_t                                                            \
    j_      = Iter.j,                                                  \
    pstart0 = Iter.pstart [0],                                         \
    pend0   = Iter.pend   [0]                                          \

//------------------------------------------------------------------------------
// for-loop control: iterate over the vectors and entries of 1, 2, or 3 matrices
//------------------------------------------------------------------------------

// iterate over the vectors in the union of two matrices A and B
#define GBI2_for_each_vector(A,B)                                              \
    GBIk_multi_iterator Iter ;                                                  \
    for (GBI2_init (&Iter,A,B) ; GBI2_while (&Iter) ; GBI2_next (&Iter))

// iterate over the vectors in the union of three matrices A, B, and C
#define GBI3_for_each_vector(A,B,C)                                            \
    GBIk_multi_iterator Iter ;                                                  \
    for (GBI3_init (&Iter,A,B,C) ; GBI3_while (&Iter) ; GBI3_next (&Iter))

// iterate over the vectors of a matrix A and an expanded scalar
// (note the scalar arg is not used; it is for code readability only):
#define GBI2s_for_each_vector(A,scalar)                                        \
    GBIk_multi_iterator Iter ;                                                  \
    for (GBI2s_init (&Iter,A) ; GBI2s_while (&Iter) ; GBI2s_next (&Iter))

// iterate over the vectors of two matrices A and B, and an expanded scalar
// (note the scalar arg is not used; it is for code readability only):
#define GBI3s_for_each_vector(A,B,scalar)                                      \
    GBIk_multi_iterator Iter ;                                                  \
    for (GBI3s_init (&Iter,A,B) ; GBI3s_while (&Iter) ; GBI3s_next (&Iter))

//------------------------------------------------------------------------------
// GB_jstartup:  start the formation of a matrix
//------------------------------------------------------------------------------

// GB_jstartup is used with GB_jappend and GB_jwrapup to create the
// hyperlist and vector pointers of a new matrix, one at a time.

// GB_jstartup logs the start of C(:,0); it also acts as if it logs the end of
// the sentinal vector C(:,-1).

static inline void GB_jstartup
(
    GrB_Matrix C,           // matrix to start creating
    int64_t *jlast,         // last vector appended, set to -1
    int64_t *cnz,           // set to zero
    int64_t *cnz_last       // set to zero
)
{
    C->p [0] = 0 ;          // log the start of C(:,0)
    (*cnz) = 0 ;            //
    (*cnz_last) = 0 ;
    (*jlast) = -1 ;         // last sentinal vector is -1
    if (C->is_hyper)
    { 
        C->nvec = 0 ;       // clear all existing vectors from C
    }
    C->nvec_nonempty = 0 ;  // # of non-empty vectors will be counted
}

//------------------------------------------------------------------------------
// GB_jappend:  append a new vector to the end of a matrix
//------------------------------------------------------------------------------

// Append a new vector to the end of a matrix C.

// If C->is_hyper is true, C is in hypersparse form with
// C->nvec <= C->plen <= C->vdim.  C->h has size C->plen.
// If C->is_hyper is false, C is in non-hypersparse form with
// C->nvec == C->plen == C->vdim.  C->h is NULL.
// In both cases, C->p has size C->plen+1.

// For both hypersparse and non-hypersparse, C->nvec_nonemty <= C->nvec
// is the number of vectors with at least one entry.

static inline GrB_Info GB_jappend
(
    GrB_Matrix C,           // matrix to append vector j to
    int64_t j,              // new vector to append
    int64_t *jlast,         // last vector appended, -1 if none
    int64_t cnz,            // nnz(C) after adding this vector j
    int64_t *cnz_last,      // nnz(C) before adding this vector j
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (C != NULL) ;
    ASSERT (!C->p_shallow) ;
    ASSERT (!C->h_shallow) ;
    ASSERT (C->p != NULL) ;

    if (cnz <= (*cnz_last))
    { 
        // nothing to do
        return (GrB_SUCCESS) ;
    }

    // one more non-empty vector
    C->nvec_nonempty++ ;

    if (C->is_hyper)
    { 

        //----------------------------------------------------------------------
        // C is hypersparse; make sure space exists in the hyperlist
        //----------------------------------------------------------------------

        ASSERT (C->p [C->nvec] == (*cnz_last)) ;
        ASSERT (C->h != NULL) ;

        // check if space exists
        if (C->nvec == C->plen)
        { 
            // double the size of C->h and C->p
            GrB_Info info ;
            info = GB_hyper_realloc (C, GB_IMIN (C->vdim, 2*(C->plen+1)),
                Context) ;
            if (info != GrB_SUCCESS)
            { 
                return (info) ;
            }
        }

        ASSERT (C->nvec >= 0) ;
        ASSERT (C->nvec < C->plen) ;
        ASSERT (C->plen <= C->vdim) ;
        ASSERT (C->p [C->nvec] == (*cnz_last)) ;

        C->h [C->nvec] = j ;            // add j to the hyperlist
        C->p [C->nvec+1] = cnz ;        // mark the end of C(:,j)
        C->nvec++ ;                     // one more vector in the hyperlist

    }
    else
    {

        //----------------------------------------------------------------------
        // C is non-hypersparse
        //----------------------------------------------------------------------

        int64_t *restrict Cp = C->p ;

        ASSERT (C->nvec == C->plen && C->plen == C->vdim) ;
        ASSERT (C->h == NULL) ;
        ASSERT (Cp [(*jlast)+1] == (*cnz_last)) ;

        // Even if C is non-hypersparse, the iteration that uses this function
        // may iterate over a hypersparse input matrix, so not every vector j
        // will be traversed.  So when j is seen, the end of vectors jlast+1 to
        // j must logged in Cp.

        for (int64_t jprior = (*jlast)+1 ; jprior < j ; jprior++)
        { 
            Cp [jprior+1] = (*cnz_last) ;   // mark the end of C(:,jprior)
        }
        Cp [j+1] = cnz ;                    // mark the end of C(:,j)
    }

    // record the last vector added to C
    (*cnz_last) = cnz ;
    (*jlast) = j ;

    return (GrB_SUCCESS) ;
}

//------------------------------------------------------------------------------
// GB_jwrapup:  finish contructing a new matrix
//------------------------------------------------------------------------------

// Log the end of any vectors in C that are not yet terminated.  Nothing
// happens if C is hypersparse (except for setting C->magic).

static inline void GB_jwrapup
(
    GrB_Matrix C,           // matrix to finish
    int64_t jlast,          // last vector appended, -1 if none
    int64_t cnz             // final nnz(C)
)
{

    if (!C->is_hyper)
    {

        //----------------------------------------------------------------------
        // C is non-hypersparse
        //----------------------------------------------------------------------

        // log the end of C(:,jlast+1) to C(:,n-1), in case the last vector
        // j=n-1 has not yet been seen, or has been seen but was empty.

        int64_t *restrict Cp = C->p ;
        int64_t j = C->vdim - 1 ;

        for (int64_t jprior = jlast+1 ; jprior <= j ; jprior++)
        { 
            Cp [jprior+1] = cnz ;           // mark the end of C(:,jprior)
        }
    }

    // C->p and C->h are now valid
    C->magic = GB_MAGIC ;
}

//------------------------------------------------------------------------------
// built-in unary and binary operators
//------------------------------------------------------------------------------

#define GB_TYPE             bool
#define GB_BOOLEAN
#define GB(x)               GB_ ## x ## _BOOL
#define GB_CAST_NAME(x)     GB_cast_bool_ ## x
#include "GB_ops_template.h"

#define GB_TYPE             int8_t
#define GB(x)               GB_ ## x ## _INT8
#define GB_CAST_NAME(x)     GB_cast_int8_t_ ## x
#include "GB_ops_template.h"

#define GB_TYPE             uint8_t
#define GB_UNSIGNED
#define GB(x)               GB_ ## x ## _UINT8
#define GB_CAST_NAME(x)     GB_cast_uint8_t_ ## x
#include "GB_ops_template.h"

#define GB_TYPE             int16_t
#define GB(x)               GB_ ## x ## _INT16
#define GB_CAST_NAME(x)     GB_cast_int16_t_ ## x
#include "GB_ops_template.h"

#define GB_TYPE             uint16_t
#define GB_UNSIGNED
#define GB(x)               GB_ ## x ## _UINT16
#define GB_CAST_NAME(x)     GB_cast_uint16_t_ ## x
#include "GB_ops_template.h"

#define GB_TYPE             int32_t
#define GB(x)               GB_ ## x ## _INT32
#define GB_CAST_NAME(x)     GB_cast_int32_t_ ## x
#include "GB_ops_template.h"

#define GB_TYPE             uint32_t
#define GB_UNSIGNED
#define GB(x)               GB_ ## x ## _UINT32
#define GB_CAST_NAME(x)     GB_cast_uint32_t_ ## x
#include "GB_ops_template.h"

#define GB_TYPE             int64_t
#define GB(x)               GB_ ## x ## _INT64
#define GB_CAST_NAME(x)     GB_cast_int64_t_ ## x
#include "GB_ops_template.h"

#define GB_TYPE             uint64_t
#define GB_UNSIGNED
#define GB(x)               GB_ ## x ## _UINT64
#define GB_CAST_NAME(x)     GB_cast_uint64_t_ ## x
#include "GB_ops_template.h"

#define GB_TYPE             float
#define GB_FLOATING_POINT
#define GB(x)               GB_ ## x ## _FP32
#define GB_CAST_NAME(x)     GB_cast_float_ ## x
#include "GB_ops_template.h"

#define GB_TYPE             double
#define GB_FLOATING_POINT
#define GB_FLOATING_POINT_DOUBLE
#define GB(x)               GB_ ## x ## _FP64
#define GB_CAST_NAME(x)     GB_cast_double_ ## x
#include "GB_ops_template.h"

inline void GB_copy_user_user (void *z, void *x, size_t s)
{ 
    memcpy (z, x, s) ;
}

//------------------------------------------------------------------------------
// definitions of built-in types and functions, used by user-defined objects
//------------------------------------------------------------------------------

//------------------------------------------------------
// built-in types
//------------------------------------------------------

#define GB_DEF_GrB_BOOL_type bool
#define GB_DEF_GrB_INT8_type int8_t
#define GB_DEF_GrB_UINT8_type uint8_t
#define GB_DEF_GrB_INT16_type int16_t
#define GB_DEF_GrB_UINT16_type uint16_t
#define GB_DEF_GrB_INT32_type int32_t
#define GB_DEF_GrB_UINT32_type uint32_t
#define GB_DEF_GrB_INT64_type int64_t
#define GB_DEF_GrB_UINT64_type uint64_t
#define GB_DEF_GrB_FP32_type float
#define GB_DEF_GrB_FP64_type double

//------------------------------------------------------
// built-in unary operators
//------------------------------------------------------

// op: IDENTITY
#define GB_DEF_GrB_IDENTITY_BOOL_function GB_IDENTITY_f_BOOL
#define GB_DEF_GrB_IDENTITY_BOOL_ztype bool
#define GB_DEF_GrB_IDENTITY_BOOL_xtype bool

#define GB_DEF_GrB_IDENTITY_INT8_function GB_IDENTITY_f_INT8
#define GB_DEF_GrB_IDENTITY_INT8_ztype int8_t
#define GB_DEF_GrB_IDENTITY_INT8_xtype int8_t

#define GB_DEF_GrB_IDENTITY_UINT8_function GB_IDENTITY_f_UINT8
#define GB_DEF_GrB_IDENTITY_UINT8_ztype uint8_t
#define GB_DEF_GrB_IDENTITY_UINT8_xtype uint8_t

#define GB_DEF_GrB_IDENTITY_INT16_function GB_IDENTITY_f_INT16
#define GB_DEF_GrB_IDENTITY_INT16_ztype int16_t
#define GB_DEF_GrB_IDENTITY_INT16_xtype int16_t

#define GB_DEF_GrB_IDENTITY_UINT16_function GB_IDENTITY_f_UINT16
#define GB_DEF_GrB_IDENTITY_UINT16_ztype uint16_t
#define GB_DEF_GrB_IDENTITY_UINT16_xtype uint16_t

#define GB_DEF_GrB_IDENTITY_INT32_function GB_IDENTITY_f_INT32
#define GB_DEF_GrB_IDENTITY_INT32_ztype int32_t
#define GB_DEF_GrB_IDENTITY_INT32_xtype int32_t

#define GB_DEF_GrB_IDENTITY_UINT32_function GB_IDENTITY_f_UINT32
#define GB_DEF_GrB_IDENTITY_UINT32_ztype uint32_t
#define GB_DEF_GrB_IDENTITY_UINT32_xtype uint32_t

#define GB_DEF_GrB_IDENTITY_INT64_function GB_IDENTITY_f_INT64
#define GB_DEF_GrB_IDENTITY_INT64_ztype int64_t
#define GB_DEF_GrB_IDENTITY_INT64_xtype int64_t

#define GB_DEF_GrB_IDENTITY_UINT64_function GB_IDENTITY_f_UINT64
#define GB_DEF_GrB_IDENTITY_UINT64_ztype uint64_t
#define GB_DEF_GrB_IDENTITY_UINT64_xtype uint64_t

#define GB_DEF_GrB_IDENTITY_FP32_function GB_IDENTITY_f_FP32
#define GB_DEF_GrB_IDENTITY_FP32_ztype float
#define GB_DEF_GrB_IDENTITY_FP32_xtype float

#define GB_DEF_GrB_IDENTITY_FP64_function GB_IDENTITY_f_FP64
#define GB_DEF_GrB_IDENTITY_FP64_ztype double
#define GB_DEF_GrB_IDENTITY_FP64_xtype double

// op: AINV
#define GB_DEF_GrB_AINV_BOOL_function GB_AINV_f_BOOL
#define GB_DEF_GrB_AINV_BOOL_ztype bool
#define GB_DEF_GrB_AINV_BOOL_xtype bool

#define GB_DEF_GrB_AINV_INT8_function GB_AINV_f_INT8
#define GB_DEF_GrB_AINV_INT8_ztype int8_t
#define GB_DEF_GrB_AINV_INT8_xtype int8_t

#define GB_DEF_GrB_AINV_UINT8_function GB_AINV_f_UINT8
#define GB_DEF_GrB_AINV_UINT8_ztype uint8_t
#define GB_DEF_GrB_AINV_UINT8_xtype uint8_t

#define GB_DEF_GrB_AINV_INT16_function GB_AINV_f_INT16
#define GB_DEF_GrB_AINV_INT16_ztype int16_t
#define GB_DEF_GrB_AINV_INT16_xtype int16_t

#define GB_DEF_GrB_AINV_UINT16_function GB_AINV_f_UINT16
#define GB_DEF_GrB_AINV_UINT16_ztype uint16_t
#define GB_DEF_GrB_AINV_UINT16_xtype uint16_t

#define GB_DEF_GrB_AINV_INT32_function GB_AINV_f_INT32
#define GB_DEF_GrB_AINV_INT32_ztype int32_t
#define GB_DEF_GrB_AINV_INT32_xtype int32_t

#define GB_DEF_GrB_AINV_UINT32_function GB_AINV_f_UINT32
#define GB_DEF_GrB_AINV_UINT32_ztype uint32_t
#define GB_DEF_GrB_AINV_UINT32_xtype uint32_t

#define GB_DEF_GrB_AINV_INT64_function GB_AINV_f_INT64
#define GB_DEF_GrB_AINV_INT64_ztype int64_t
#define GB_DEF_GrB_AINV_INT64_xtype int64_t

#define GB_DEF_GrB_AINV_UINT64_function GB_AINV_f_UINT64
#define GB_DEF_GrB_AINV_UINT64_ztype uint64_t
#define GB_DEF_GrB_AINV_UINT64_xtype uint64_t

#define GB_DEF_GrB_AINV_FP32_function GB_AINV_f_FP32
#define GB_DEF_GrB_AINV_FP32_ztype float
#define GB_DEF_GrB_AINV_FP32_xtype float

#define GB_DEF_GrB_AINV_FP64_function GB_AINV_f_FP64
#define GB_DEF_GrB_AINV_FP64_ztype double
#define GB_DEF_GrB_AINV_FP64_xtype double

// op: MINV
#define GB_DEF_GrB_MINV_BOOL_function GB_MINV_f_BOOL
#define GB_DEF_GrB_MINV_BOOL_ztype bool
#define GB_DEF_GrB_MINV_BOOL_xtype bool

#define GB_DEF_GrB_MINV_INT8_function GB_MINV_f_INT8
#define GB_DEF_GrB_MINV_INT8_ztype int8_t
#define GB_DEF_GrB_MINV_INT8_xtype int8_t

#define GB_DEF_GrB_MINV_UINT8_function GB_MINV_f_UINT8
#define GB_DEF_GrB_MINV_UINT8_ztype uint8_t
#define GB_DEF_GrB_MINV_UINT8_xtype uint8_t

#define GB_DEF_GrB_MINV_INT16_function GB_MINV_f_INT16
#define GB_DEF_GrB_MINV_INT16_ztype int16_t
#define GB_DEF_GrB_MINV_INT16_xtype int16_t

#define GB_DEF_GrB_MINV_UINT16_function GB_MINV_f_UINT16
#define GB_DEF_GrB_MINV_UINT16_ztype uint16_t
#define GB_DEF_GrB_MINV_UINT16_xtype uint16_t

#define GB_DEF_GrB_MINV_INT32_function GB_MINV_f_INT32
#define GB_DEF_GrB_MINV_INT32_ztype int32_t
#define GB_DEF_GrB_MINV_INT32_xtype int32_t

#define GB_DEF_GrB_MINV_UINT32_function GB_MINV_f_UINT32
#define GB_DEF_GrB_MINV_UINT32_ztype uint32_t
#define GB_DEF_GrB_MINV_UINT32_xtype uint32_t

#define GB_DEF_GrB_MINV_INT64_function GB_MINV_f_INT64
#define GB_DEF_GrB_MINV_INT64_ztype int64_t
#define GB_DEF_GrB_MINV_INT64_xtype int64_t

#define GB_DEF_GrB_MINV_UINT64_function GB_MINV_f_UINT64
#define GB_DEF_GrB_MINV_UINT64_ztype uint64_t
#define GB_DEF_GrB_MINV_UINT64_xtype uint64_t

#define GB_DEF_GrB_MINV_FP32_function GB_MINV_f_FP32
#define GB_DEF_GrB_MINV_FP32_ztype float
#define GB_DEF_GrB_MINV_FP32_xtype float

#define GB_DEF_GrB_MINV_FP64_function GB_MINV_f_FP64
#define GB_DEF_GrB_MINV_FP64_ztype double
#define GB_DEF_GrB_MINV_FP64_xtype double

// op: LNOT
#define GB_DEF_GxB_LNOT_BOOL_function GB_LNOT_f_BOOL
#define GB_DEF_GxB_LNOT_BOOL_ztype bool
#define GB_DEF_GxB_LNOT_BOOL_xtype bool

#define GB_DEF_GxB_LNOT_INT8_function GB_LNOT_f_INT8
#define GB_DEF_GxB_LNOT_INT8_ztype int8_t
#define GB_DEF_GxB_LNOT_INT8_xtype int8_t

#define GB_DEF_GxB_LNOT_UINT8_function GB_LNOT_f_UINT8
#define GB_DEF_GxB_LNOT_UINT8_ztype uint8_t
#define GB_DEF_GxB_LNOT_UINT8_xtype uint8_t

#define GB_DEF_GxB_LNOT_INT16_function GB_LNOT_f_INT16
#define GB_DEF_GxB_LNOT_INT16_ztype int16_t
#define GB_DEF_GxB_LNOT_INT16_xtype int16_t

#define GB_DEF_GxB_LNOT_UINT16_function GB_LNOT_f_UINT16
#define GB_DEF_GxB_LNOT_UINT16_ztype uint16_t
#define GB_DEF_GxB_LNOT_UINT16_xtype uint16_t

#define GB_DEF_GxB_LNOT_INT32_function GB_LNOT_f_INT32
#define GB_DEF_GxB_LNOT_INT32_ztype int32_t
#define GB_DEF_GxB_LNOT_INT32_xtype int32_t

#define GB_DEF_GxB_LNOT_UINT32_function GB_LNOT_f_UINT32
#define GB_DEF_GxB_LNOT_UINT32_ztype uint32_t
#define GB_DEF_GxB_LNOT_UINT32_xtype uint32_t

#define GB_DEF_GxB_LNOT_INT64_function GB_LNOT_f_INT64
#define GB_DEF_GxB_LNOT_INT64_ztype int64_t
#define GB_DEF_GxB_LNOT_INT64_xtype int64_t

#define GB_DEF_GxB_LNOT_UINT64_function GB_LNOT_f_UINT64
#define GB_DEF_GxB_LNOT_UINT64_ztype uint64_t
#define GB_DEF_GxB_LNOT_UINT64_xtype uint64_t

#define GB_DEF_GxB_LNOT_FP32_function GB_LNOT_f_FP32
#define GB_DEF_GxB_LNOT_FP32_ztype float
#define GB_DEF_GxB_LNOT_FP32_xtype float

#define GB_DEF_GxB_LNOT_FP64_function GB_LNOT_f_FP64
#define GB_DEF_GxB_LNOT_FP64_ztype double
#define GB_DEF_GxB_LNOT_FP64_xtype double

// op: ONE
#define GB_DEF_GxB_ONE_BOOL_function GB_ONE_f_BOOL
#define GB_DEF_GxB_ONE_BOOL_ztype bool
#define GB_DEF_GxB_ONE_BOOL_xtype bool

#define GB_DEF_GxB_ONE_INT8_function GB_ONE_f_INT8
#define GB_DEF_GxB_ONE_INT8_ztype int8_t
#define GB_DEF_GxB_ONE_INT8_xtype int8_t

#define GB_DEF_GxB_ONE_UINT8_function GB_ONE_f_UINT8
#define GB_DEF_GxB_ONE_UINT8_ztype uint8_t
#define GB_DEF_GxB_ONE_UINT8_xtype uint8_t

#define GB_DEF_GxB_ONE_INT16_function GB_ONE_f_INT16
#define GB_DEF_GxB_ONE_INT16_ztype int16_t
#define GB_DEF_GxB_ONE_INT16_xtype int16_t

#define GB_DEF_GxB_ONE_UINT16_function GB_ONE_f_UINT16
#define GB_DEF_GxB_ONE_UINT16_ztype uint16_t
#define GB_DEF_GxB_ONE_UINT16_xtype uint16_t

#define GB_DEF_GxB_ONE_INT32_function GB_ONE_f_INT32
#define GB_DEF_GxB_ONE_INT32_ztype int32_t
#define GB_DEF_GxB_ONE_INT32_xtype int32_t

#define GB_DEF_GxB_ONE_UINT32_function GB_ONE_f_UINT32
#define GB_DEF_GxB_ONE_UINT32_ztype uint32_t
#define GB_DEF_GxB_ONE_UINT32_xtype uint32_t

#define GB_DEF_GxB_ONE_INT64_function GB_ONE_f_INT64
#define GB_DEF_GxB_ONE_INT64_ztype int64_t
#define GB_DEF_GxB_ONE_INT64_xtype int64_t

#define GB_DEF_GxB_ONE_UINT64_function GB_ONE_f_UINT64
#define GB_DEF_GxB_ONE_UINT64_ztype uint64_t
#define GB_DEF_GxB_ONE_UINT64_xtype uint64_t

#define GB_DEF_GxB_ONE_FP32_function GB_ONE_f_FP32
#define GB_DEF_GxB_ONE_FP32_ztype float
#define GB_DEF_GxB_ONE_FP32_xtype float

#define GB_DEF_GxB_ONE_FP64_function GB_ONE_f_FP64
#define GB_DEF_GxB_ONE_FP64_ztype double
#define GB_DEF_GxB_ONE_FP64_xtype double

// op: ABS
#define GB_DEF_GxB_ABS_BOOL_function GB_ABS_f_BOOL
#define GB_DEF_GxB_ABS_BOOL_ztype bool
#define GB_DEF_GxB_ABS_BOOL_xtype bool

#define GB_DEF_GxB_ABS_INT8_function GB_ABS_f_INT8
#define GB_DEF_GxB_ABS_INT8_ztype int8_t
#define GB_DEF_GxB_ABS_INT8_xtype int8_t

#define GB_DEF_GxB_ABS_UINT8_function GB_ABS_f_UINT8
#define GB_DEF_GxB_ABS_UINT8_ztype uint8_t
#define GB_DEF_GxB_ABS_UINT8_xtype uint8_t

#define GB_DEF_GxB_ABS_INT16_function GB_ABS_f_INT16
#define GB_DEF_GxB_ABS_INT16_ztype int16_t
#define GB_DEF_GxB_ABS_INT16_xtype int16_t

#define GB_DEF_GxB_ABS_UINT16_function GB_ABS_f_UINT16
#define GB_DEF_GxB_ABS_UINT16_ztype uint16_t
#define GB_DEF_GxB_ABS_UINT16_xtype uint16_t

#define GB_DEF_GxB_ABS_INT32_function GB_ABS_f_INT32
#define GB_DEF_GxB_ABS_INT32_ztype int32_t
#define GB_DEF_GxB_ABS_INT32_xtype int32_t

#define GB_DEF_GxB_ABS_UINT32_function GB_ABS_f_UINT32
#define GB_DEF_GxB_ABS_UINT32_ztype uint32_t
#define GB_DEF_GxB_ABS_UINT32_xtype uint32_t

#define GB_DEF_GxB_ABS_INT64_function GB_ABS_f_INT64
#define GB_DEF_GxB_ABS_INT64_ztype int64_t
#define GB_DEF_GxB_ABS_INT64_xtype int64_t

#define GB_DEF_GxB_ABS_UINT64_function GB_ABS_f_UINT64
#define GB_DEF_GxB_ABS_UINT64_ztype uint64_t
#define GB_DEF_GxB_ABS_UINT64_xtype uint64_t

#define GB_DEF_GxB_ABS_FP32_function GB_ABS_f_FP32
#define GB_DEF_GxB_ABS_FP32_ztype float
#define GB_DEF_GxB_ABS_FP32_xtype float

#define GB_DEF_GxB_ABS_FP64_function GB_ABS_f_FP64
#define GB_DEF_GxB_ABS_FP64_ztype double
#define GB_DEF_GxB_ABS_FP64_xtype double

#define GB_DEF_GrB_LNOT_function GB_LNOT_f_BOOL
#define GB_DEF_GrB_LNOT_ztype bool
#define GB_DEF_GrB_LNOT_xtype bool

//------------------------------------------------------
// binary operators of the form z=f(x,y): TxT -> T
//------------------------------------------------------

// op: FIRST
#define GB_DEF_GrB_FIRST_BOOL_function GB_FIRST_f_BOOL
#define GB_DEF_GrB_FIRST_BOOL_ztype bool
#define GB_DEF_GrB_FIRST_BOOL_xtype bool
#define GB_DEF_GrB_FIRST_BOOL_ytype bool

#define GB_DEF_GrB_FIRST_INT8_function GB_FIRST_f_INT8
#define GB_DEF_GrB_FIRST_INT8_ztype int8_t
#define GB_DEF_GrB_FIRST_INT8_xtype int8_t
#define GB_DEF_GrB_FIRST_INT8_ytype int8_t

#define GB_DEF_GrB_FIRST_UINT8_function GB_FIRST_f_UINT8
#define GB_DEF_GrB_FIRST_UINT8_ztype uint8_t
#define GB_DEF_GrB_FIRST_UINT8_xtype uint8_t
#define GB_DEF_GrB_FIRST_UINT8_ytype uint8_t

#define GB_DEF_GrB_FIRST_INT16_function GB_FIRST_f_INT16
#define GB_DEF_GrB_FIRST_INT16_ztype int16_t
#define GB_DEF_GrB_FIRST_INT16_xtype int16_t
#define GB_DEF_GrB_FIRST_INT16_ytype int16_t

#define GB_DEF_GrB_FIRST_UINT16_function GB_FIRST_f_UINT16
#define GB_DEF_GrB_FIRST_UINT16_ztype uint16_t
#define GB_DEF_GrB_FIRST_UINT16_xtype uint16_t
#define GB_DEF_GrB_FIRST_UINT16_ytype uint16_t

#define GB_DEF_GrB_FIRST_INT32_function GB_FIRST_f_INT32
#define GB_DEF_GrB_FIRST_INT32_ztype int32_t
#define GB_DEF_GrB_FIRST_INT32_xtype int32_t
#define GB_DEF_GrB_FIRST_INT32_ytype int32_t

#define GB_DEF_GrB_FIRST_UINT32_function GB_FIRST_f_UINT32
#define GB_DEF_GrB_FIRST_UINT32_ztype uint32_t
#define GB_DEF_GrB_FIRST_UINT32_xtype uint32_t
#define GB_DEF_GrB_FIRST_UINT32_ytype uint32_t

#define GB_DEF_GrB_FIRST_INT64_function GB_FIRST_f_INT64
#define GB_DEF_GrB_FIRST_INT64_ztype int64_t
#define GB_DEF_GrB_FIRST_INT64_xtype int64_t
#define GB_DEF_GrB_FIRST_INT64_ytype int64_t

#define GB_DEF_GrB_FIRST_UINT64_function GB_FIRST_f_UINT64
#define GB_DEF_GrB_FIRST_UINT64_ztype uint64_t
#define GB_DEF_GrB_FIRST_UINT64_xtype uint64_t
#define GB_DEF_GrB_FIRST_UINT64_ytype uint64_t

#define GB_DEF_GrB_FIRST_FP32_function GB_FIRST_f_FP32
#define GB_DEF_GrB_FIRST_FP32_ztype float
#define GB_DEF_GrB_FIRST_FP32_xtype float
#define GB_DEF_GrB_FIRST_FP32_ytype float

#define GB_DEF_GrB_FIRST_FP64_function GB_FIRST_f_FP64
#define GB_DEF_GrB_FIRST_FP64_ztype double
#define GB_DEF_GrB_FIRST_FP64_xtype double
#define GB_DEF_GrB_FIRST_FP64_ytype double

// op: SECOND
#define GB_DEF_GrB_SECOND_BOOL_function GB_SECOND_f_BOOL
#define GB_DEF_GrB_SECOND_BOOL_ztype bool
#define GB_DEF_GrB_SECOND_BOOL_xtype bool
#define GB_DEF_GrB_SECOND_BOOL_ytype bool

#define GB_DEF_GrB_SECOND_INT8_function GB_SECOND_f_INT8
#define GB_DEF_GrB_SECOND_INT8_ztype int8_t
#define GB_DEF_GrB_SECOND_INT8_xtype int8_t
#define GB_DEF_GrB_SECOND_INT8_ytype int8_t

#define GB_DEF_GrB_SECOND_UINT8_function GB_SECOND_f_UINT8
#define GB_DEF_GrB_SECOND_UINT8_ztype uint8_t
#define GB_DEF_GrB_SECOND_UINT8_xtype uint8_t
#define GB_DEF_GrB_SECOND_UINT8_ytype uint8_t

#define GB_DEF_GrB_SECOND_INT16_function GB_SECOND_f_INT16
#define GB_DEF_GrB_SECOND_INT16_ztype int16_t
#define GB_DEF_GrB_SECOND_INT16_xtype int16_t
#define GB_DEF_GrB_SECOND_INT16_ytype int16_t

#define GB_DEF_GrB_SECOND_UINT16_function GB_SECOND_f_UINT16
#define GB_DEF_GrB_SECOND_UINT16_ztype uint16_t
#define GB_DEF_GrB_SECOND_UINT16_xtype uint16_t
#define GB_DEF_GrB_SECOND_UINT16_ytype uint16_t

#define GB_DEF_GrB_SECOND_INT32_function GB_SECOND_f_INT32
#define GB_DEF_GrB_SECOND_INT32_ztype int32_t
#define GB_DEF_GrB_SECOND_INT32_xtype int32_t
#define GB_DEF_GrB_SECOND_INT32_ytype int32_t

#define GB_DEF_GrB_SECOND_UINT32_function GB_SECOND_f_UINT32
#define GB_DEF_GrB_SECOND_UINT32_ztype uint32_t
#define GB_DEF_GrB_SECOND_UINT32_xtype uint32_t
#define GB_DEF_GrB_SECOND_UINT32_ytype uint32_t

#define GB_DEF_GrB_SECOND_INT64_function GB_SECOND_f_INT64
#define GB_DEF_GrB_SECOND_INT64_ztype int64_t
#define GB_DEF_GrB_SECOND_INT64_xtype int64_t
#define GB_DEF_GrB_SECOND_INT64_ytype int64_t

#define GB_DEF_GrB_SECOND_UINT64_function GB_SECOND_f_UINT64
#define GB_DEF_GrB_SECOND_UINT64_ztype uint64_t
#define GB_DEF_GrB_SECOND_UINT64_xtype uint64_t
#define GB_DEF_GrB_SECOND_UINT64_ytype uint64_t

#define GB_DEF_GrB_SECOND_FP32_function GB_SECOND_f_FP32
#define GB_DEF_GrB_SECOND_FP32_ztype float
#define GB_DEF_GrB_SECOND_FP32_xtype float
#define GB_DEF_GrB_SECOND_FP32_ytype float

#define GB_DEF_GrB_SECOND_FP64_function GB_SECOND_f_FP64
#define GB_DEF_GrB_SECOND_FP64_ztype double
#define GB_DEF_GrB_SECOND_FP64_xtype double
#define GB_DEF_GrB_SECOND_FP64_ytype double

// op: MIN
#define GB_DEF_GrB_MIN_BOOL_function GB_MIN_f_BOOL
#define GB_DEF_GrB_MIN_BOOL_ztype bool
#define GB_DEF_GrB_MIN_BOOL_xtype bool
#define GB_DEF_GrB_MIN_BOOL_ytype bool

#define GB_DEF_GrB_MIN_INT8_function GB_MIN_f_INT8
#define GB_DEF_GrB_MIN_INT8_ztype int8_t
#define GB_DEF_GrB_MIN_INT8_xtype int8_t
#define GB_DEF_GrB_MIN_INT8_ytype int8_t

#define GB_DEF_GrB_MIN_UINT8_function GB_MIN_f_UINT8
#define GB_DEF_GrB_MIN_UINT8_ztype uint8_t
#define GB_DEF_GrB_MIN_UINT8_xtype uint8_t
#define GB_DEF_GrB_MIN_UINT8_ytype uint8_t

#define GB_DEF_GrB_MIN_INT16_function GB_MIN_f_INT16
#define GB_DEF_GrB_MIN_INT16_ztype int16_t
#define GB_DEF_GrB_MIN_INT16_xtype int16_t
#define GB_DEF_GrB_MIN_INT16_ytype int16_t

#define GB_DEF_GrB_MIN_UINT16_function GB_MIN_f_UINT16
#define GB_DEF_GrB_MIN_UINT16_ztype uint16_t
#define GB_DEF_GrB_MIN_UINT16_xtype uint16_t
#define GB_DEF_GrB_MIN_UINT16_ytype uint16_t

#define GB_DEF_GrB_MIN_INT32_function GB_MIN_f_INT32
#define GB_DEF_GrB_MIN_INT32_ztype int32_t
#define GB_DEF_GrB_MIN_INT32_xtype int32_t
#define GB_DEF_GrB_MIN_INT32_ytype int32_t

#define GB_DEF_GrB_MIN_UINT32_function GB_MIN_f_UINT32
#define GB_DEF_GrB_MIN_UINT32_ztype uint32_t
#define GB_DEF_GrB_MIN_UINT32_xtype uint32_t
#define GB_DEF_GrB_MIN_UINT32_ytype uint32_t

#define GB_DEF_GrB_MIN_INT64_function GB_MIN_f_INT64
#define GB_DEF_GrB_MIN_INT64_ztype int64_t
#define GB_DEF_GrB_MIN_INT64_xtype int64_t
#define GB_DEF_GrB_MIN_INT64_ytype int64_t

#define GB_DEF_GrB_MIN_UINT64_function GB_MIN_f_UINT64
#define GB_DEF_GrB_MIN_UINT64_ztype uint64_t
#define GB_DEF_GrB_MIN_UINT64_xtype uint64_t
#define GB_DEF_GrB_MIN_UINT64_ytype uint64_t

#define GB_DEF_GrB_MIN_FP32_function GB_MIN_f_FP32
#define GB_DEF_GrB_MIN_FP32_ztype float
#define GB_DEF_GrB_MIN_FP32_xtype float
#define GB_DEF_GrB_MIN_FP32_ytype float

#define GB_DEF_GrB_MIN_FP64_function GB_MIN_f_FP64
#define GB_DEF_GrB_MIN_FP64_ztype double
#define GB_DEF_GrB_MIN_FP64_xtype double
#define GB_DEF_GrB_MIN_FP64_ytype double

// op: MAX
#define GB_DEF_GrB_MAX_BOOL_function GB_MAX_f_BOOL
#define GB_DEF_GrB_MAX_BOOL_ztype bool
#define GB_DEF_GrB_MAX_BOOL_xtype bool
#define GB_DEF_GrB_MAX_BOOL_ytype bool

#define GB_DEF_GrB_MAX_INT8_function GB_MAX_f_INT8
#define GB_DEF_GrB_MAX_INT8_ztype int8_t
#define GB_DEF_GrB_MAX_INT8_xtype int8_t
#define GB_DEF_GrB_MAX_INT8_ytype int8_t

#define GB_DEF_GrB_MAX_UINT8_function GB_MAX_f_UINT8
#define GB_DEF_GrB_MAX_UINT8_ztype uint8_t
#define GB_DEF_GrB_MAX_UINT8_xtype uint8_t
#define GB_DEF_GrB_MAX_UINT8_ytype uint8_t

#define GB_DEF_GrB_MAX_INT16_function GB_MAX_f_INT16
#define GB_DEF_GrB_MAX_INT16_ztype int16_t
#define GB_DEF_GrB_MAX_INT16_xtype int16_t
#define GB_DEF_GrB_MAX_INT16_ytype int16_t

#define GB_DEF_GrB_MAX_UINT16_function GB_MAX_f_UINT16
#define GB_DEF_GrB_MAX_UINT16_ztype uint16_t
#define GB_DEF_GrB_MAX_UINT16_xtype uint16_t
#define GB_DEF_GrB_MAX_UINT16_ytype uint16_t

#define GB_DEF_GrB_MAX_INT32_function GB_MAX_f_INT32
#define GB_DEF_GrB_MAX_INT32_ztype int32_t
#define GB_DEF_GrB_MAX_INT32_xtype int32_t
#define GB_DEF_GrB_MAX_INT32_ytype int32_t

#define GB_DEF_GrB_MAX_UINT32_function GB_MAX_f_UINT32
#define GB_DEF_GrB_MAX_UINT32_ztype uint32_t
#define GB_DEF_GrB_MAX_UINT32_xtype uint32_t
#define GB_DEF_GrB_MAX_UINT32_ytype uint32_t

#define GB_DEF_GrB_MAX_INT64_function GB_MAX_f_INT64
#define GB_DEF_GrB_MAX_INT64_ztype int64_t
#define GB_DEF_GrB_MAX_INT64_xtype int64_t
#define GB_DEF_GrB_MAX_INT64_ytype int64_t

#define GB_DEF_GrB_MAX_UINT64_function GB_MAX_f_UINT64
#define GB_DEF_GrB_MAX_UINT64_ztype uint64_t
#define GB_DEF_GrB_MAX_UINT64_xtype uint64_t
#define GB_DEF_GrB_MAX_UINT64_ytype uint64_t

#define GB_DEF_GrB_MAX_FP32_function GB_MAX_f_FP32
#define GB_DEF_GrB_MAX_FP32_ztype float
#define GB_DEF_GrB_MAX_FP32_xtype float
#define GB_DEF_GrB_MAX_FP32_ytype float

#define GB_DEF_GrB_MAX_FP64_function GB_MAX_f_FP64
#define GB_DEF_GrB_MAX_FP64_ztype double
#define GB_DEF_GrB_MAX_FP64_xtype double
#define GB_DEF_GrB_MAX_FP64_ytype double

// op: PLUS
#define GB_DEF_GrB_PLUS_BOOL_function GB_PLUS_f_BOOL
#define GB_DEF_GrB_PLUS_BOOL_ztype bool
#define GB_DEF_GrB_PLUS_BOOL_xtype bool
#define GB_DEF_GrB_PLUS_BOOL_ytype bool

#define GB_DEF_GrB_PLUS_INT8_function GB_PLUS_f_INT8
#define GB_DEF_GrB_PLUS_INT8_ztype int8_t
#define GB_DEF_GrB_PLUS_INT8_xtype int8_t
#define GB_DEF_GrB_PLUS_INT8_ytype int8_t

#define GB_DEF_GrB_PLUS_UINT8_function GB_PLUS_f_UINT8
#define GB_DEF_GrB_PLUS_UINT8_ztype uint8_t
#define GB_DEF_GrB_PLUS_UINT8_xtype uint8_t
#define GB_DEF_GrB_PLUS_UINT8_ytype uint8_t

#define GB_DEF_GrB_PLUS_INT16_function GB_PLUS_f_INT16
#define GB_DEF_GrB_PLUS_INT16_ztype int16_t
#define GB_DEF_GrB_PLUS_INT16_xtype int16_t
#define GB_DEF_GrB_PLUS_INT16_ytype int16_t

#define GB_DEF_GrB_PLUS_UINT16_function GB_PLUS_f_UINT16
#define GB_DEF_GrB_PLUS_UINT16_ztype uint16_t
#define GB_DEF_GrB_PLUS_UINT16_xtype uint16_t
#define GB_DEF_GrB_PLUS_UINT16_ytype uint16_t

#define GB_DEF_GrB_PLUS_INT32_function GB_PLUS_f_INT32
#define GB_DEF_GrB_PLUS_INT32_ztype int32_t
#define GB_DEF_GrB_PLUS_INT32_xtype int32_t
#define GB_DEF_GrB_PLUS_INT32_ytype int32_t

#define GB_DEF_GrB_PLUS_UINT32_function GB_PLUS_f_UINT32
#define GB_DEF_GrB_PLUS_UINT32_ztype uint32_t
#define GB_DEF_GrB_PLUS_UINT32_xtype uint32_t
#define GB_DEF_GrB_PLUS_UINT32_ytype uint32_t

#define GB_DEF_GrB_PLUS_INT64_function GB_PLUS_f_INT64
#define GB_DEF_GrB_PLUS_INT64_ztype int64_t
#define GB_DEF_GrB_PLUS_INT64_xtype int64_t
#define GB_DEF_GrB_PLUS_INT64_ytype int64_t

#define GB_DEF_GrB_PLUS_UINT64_function GB_PLUS_f_UINT64
#define GB_DEF_GrB_PLUS_UINT64_ztype uint64_t
#define GB_DEF_GrB_PLUS_UINT64_xtype uint64_t
#define GB_DEF_GrB_PLUS_UINT64_ytype uint64_t

#define GB_DEF_GrB_PLUS_FP32_function GB_PLUS_f_FP32
#define GB_DEF_GrB_PLUS_FP32_ztype float
#define GB_DEF_GrB_PLUS_FP32_xtype float
#define GB_DEF_GrB_PLUS_FP32_ytype float

#define GB_DEF_GrB_PLUS_FP64_function GB_PLUS_f_FP64
#define GB_DEF_GrB_PLUS_FP64_ztype double
#define GB_DEF_GrB_PLUS_FP64_xtype double
#define GB_DEF_GrB_PLUS_FP64_ytype double

// op: MINUS
#define GB_DEF_GrB_MINUS_BOOL_function GB_MINUS_f_BOOL
#define GB_DEF_GrB_MINUS_BOOL_ztype bool
#define GB_DEF_GrB_MINUS_BOOL_xtype bool
#define GB_DEF_GrB_MINUS_BOOL_ytype bool

#define GB_DEF_GrB_MINUS_INT8_function GB_MINUS_f_INT8
#define GB_DEF_GrB_MINUS_INT8_ztype int8_t
#define GB_DEF_GrB_MINUS_INT8_xtype int8_t
#define GB_DEF_GrB_MINUS_INT8_ytype int8_t

#define GB_DEF_GrB_MINUS_UINT8_function GB_MINUS_f_UINT8
#define GB_DEF_GrB_MINUS_UINT8_ztype uint8_t
#define GB_DEF_GrB_MINUS_UINT8_xtype uint8_t
#define GB_DEF_GrB_MINUS_UINT8_ytype uint8_t

#define GB_DEF_GrB_MINUS_INT16_function GB_MINUS_f_INT16
#define GB_DEF_GrB_MINUS_INT16_ztype int16_t
#define GB_DEF_GrB_MINUS_INT16_xtype int16_t
#define GB_DEF_GrB_MINUS_INT16_ytype int16_t

#define GB_DEF_GrB_MINUS_UINT16_function GB_MINUS_f_UINT16
#define GB_DEF_GrB_MINUS_UINT16_ztype uint16_t
#define GB_DEF_GrB_MINUS_UINT16_xtype uint16_t
#define GB_DEF_GrB_MINUS_UINT16_ytype uint16_t

#define GB_DEF_GrB_MINUS_INT32_function GB_MINUS_f_INT32
#define GB_DEF_GrB_MINUS_INT32_ztype int32_t
#define GB_DEF_GrB_MINUS_INT32_xtype int32_t
#define GB_DEF_GrB_MINUS_INT32_ytype int32_t

#define GB_DEF_GrB_MINUS_UINT32_function GB_MINUS_f_UINT32
#define GB_DEF_GrB_MINUS_UINT32_ztype uint32_t
#define GB_DEF_GrB_MINUS_UINT32_xtype uint32_t
#define GB_DEF_GrB_MINUS_UINT32_ytype uint32_t

#define GB_DEF_GrB_MINUS_INT64_function GB_MINUS_f_INT64
#define GB_DEF_GrB_MINUS_INT64_ztype int64_t
#define GB_DEF_GrB_MINUS_INT64_xtype int64_t
#define GB_DEF_GrB_MINUS_INT64_ytype int64_t

#define GB_DEF_GrB_MINUS_UINT64_function GB_MINUS_f_UINT64
#define GB_DEF_GrB_MINUS_UINT64_ztype uint64_t
#define GB_DEF_GrB_MINUS_UINT64_xtype uint64_t
#define GB_DEF_GrB_MINUS_UINT64_ytype uint64_t

#define GB_DEF_GrB_MINUS_FP32_function GB_MINUS_f_FP32
#define GB_DEF_GrB_MINUS_FP32_ztype float
#define GB_DEF_GrB_MINUS_FP32_xtype float
#define GB_DEF_GrB_MINUS_FP32_ytype float

#define GB_DEF_GrB_MINUS_FP64_function GB_MINUS_f_FP64
#define GB_DEF_GrB_MINUS_FP64_ztype double
#define GB_DEF_GrB_MINUS_FP64_xtype double
#define GB_DEF_GrB_MINUS_FP64_ytype double

// op: TIMES
#define GB_DEF_GrB_TIMES_BOOL_function GB_TIMES_f_BOOL
#define GB_DEF_GrB_TIMES_BOOL_ztype bool
#define GB_DEF_GrB_TIMES_BOOL_xtype bool
#define GB_DEF_GrB_TIMES_BOOL_ytype bool

#define GB_DEF_GrB_TIMES_INT8_function GB_TIMES_f_INT8
#define GB_DEF_GrB_TIMES_INT8_ztype int8_t
#define GB_DEF_GrB_TIMES_INT8_xtype int8_t
#define GB_DEF_GrB_TIMES_INT8_ytype int8_t

#define GB_DEF_GrB_TIMES_UINT8_function GB_TIMES_f_UINT8
#define GB_DEF_GrB_TIMES_UINT8_ztype uint8_t
#define GB_DEF_GrB_TIMES_UINT8_xtype uint8_t
#define GB_DEF_GrB_TIMES_UINT8_ytype uint8_t

#define GB_DEF_GrB_TIMES_INT16_function GB_TIMES_f_INT16
#define GB_DEF_GrB_TIMES_INT16_ztype int16_t
#define GB_DEF_GrB_TIMES_INT16_xtype int16_t
#define GB_DEF_GrB_TIMES_INT16_ytype int16_t

#define GB_DEF_GrB_TIMES_UINT16_function GB_TIMES_f_UINT16
#define GB_DEF_GrB_TIMES_UINT16_ztype uint16_t
#define GB_DEF_GrB_TIMES_UINT16_xtype uint16_t
#define GB_DEF_GrB_TIMES_UINT16_ytype uint16_t

#define GB_DEF_GrB_TIMES_INT32_function GB_TIMES_f_INT32
#define GB_DEF_GrB_TIMES_INT32_ztype int32_t
#define GB_DEF_GrB_TIMES_INT32_xtype int32_t
#define GB_DEF_GrB_TIMES_INT32_ytype int32_t

#define GB_DEF_GrB_TIMES_UINT32_function GB_TIMES_f_UINT32
#define GB_DEF_GrB_TIMES_UINT32_ztype uint32_t
#define GB_DEF_GrB_TIMES_UINT32_xtype uint32_t
#define GB_DEF_GrB_TIMES_UINT32_ytype uint32_t

#define GB_DEF_GrB_TIMES_INT64_function GB_TIMES_f_INT64
#define GB_DEF_GrB_TIMES_INT64_ztype int64_t
#define GB_DEF_GrB_TIMES_INT64_xtype int64_t
#define GB_DEF_GrB_TIMES_INT64_ytype int64_t

#define GB_DEF_GrB_TIMES_UINT64_function GB_TIMES_f_UINT64
#define GB_DEF_GrB_TIMES_UINT64_ztype uint64_t
#define GB_DEF_GrB_TIMES_UINT64_xtype uint64_t
#define GB_DEF_GrB_TIMES_UINT64_ytype uint64_t

#define GB_DEF_GrB_TIMES_FP32_function GB_TIMES_f_FP32
#define GB_DEF_GrB_TIMES_FP32_ztype float
#define GB_DEF_GrB_TIMES_FP32_xtype float
#define GB_DEF_GrB_TIMES_FP32_ytype float

#define GB_DEF_GrB_TIMES_FP64_function GB_TIMES_f_FP64
#define GB_DEF_GrB_TIMES_FP64_ztype double
#define GB_DEF_GrB_TIMES_FP64_xtype double
#define GB_DEF_GrB_TIMES_FP64_ytype double

// op: DIV
#define GB_DEF_GrB_DIV_BOOL_function GB_DIV_f_BOOL
#define GB_DEF_GrB_DIV_BOOL_ztype bool
#define GB_DEF_GrB_DIV_BOOL_xtype bool
#define GB_DEF_GrB_DIV_BOOL_ytype bool

#define GB_DEF_GrB_DIV_INT8_function GB_DIV_f_INT8
#define GB_DEF_GrB_DIV_INT8_ztype int8_t
#define GB_DEF_GrB_DIV_INT8_xtype int8_t
#define GB_DEF_GrB_DIV_INT8_ytype int8_t

#define GB_DEF_GrB_DIV_UINT8_function GB_DIV_f_UINT8
#define GB_DEF_GrB_DIV_UINT8_ztype uint8_t
#define GB_DEF_GrB_DIV_UINT8_xtype uint8_t
#define GB_DEF_GrB_DIV_UINT8_ytype uint8_t

#define GB_DEF_GrB_DIV_INT16_function GB_DIV_f_INT16
#define GB_DEF_GrB_DIV_INT16_ztype int16_t
#define GB_DEF_GrB_DIV_INT16_xtype int16_t
#define GB_DEF_GrB_DIV_INT16_ytype int16_t

#define GB_DEF_GrB_DIV_UINT16_function GB_DIV_f_UINT16
#define GB_DEF_GrB_DIV_UINT16_ztype uint16_t
#define GB_DEF_GrB_DIV_UINT16_xtype uint16_t
#define GB_DEF_GrB_DIV_UINT16_ytype uint16_t

#define GB_DEF_GrB_DIV_INT32_function GB_DIV_f_INT32
#define GB_DEF_GrB_DIV_INT32_ztype int32_t
#define GB_DEF_GrB_DIV_INT32_xtype int32_t
#define GB_DEF_GrB_DIV_INT32_ytype int32_t

#define GB_DEF_GrB_DIV_UINT32_function GB_DIV_f_UINT32
#define GB_DEF_GrB_DIV_UINT32_ztype uint32_t
#define GB_DEF_GrB_DIV_UINT32_xtype uint32_t
#define GB_DEF_GrB_DIV_UINT32_ytype uint32_t

#define GB_DEF_GrB_DIV_INT64_function GB_DIV_f_INT64
#define GB_DEF_GrB_DIV_INT64_ztype int64_t
#define GB_DEF_GrB_DIV_INT64_xtype int64_t
#define GB_DEF_GrB_DIV_INT64_ytype int64_t

#define GB_DEF_GrB_DIV_UINT64_function GB_DIV_f_UINT64
#define GB_DEF_GrB_DIV_UINT64_ztype uint64_t
#define GB_DEF_GrB_DIV_UINT64_xtype uint64_t
#define GB_DEF_GrB_DIV_UINT64_ytype uint64_t

#define GB_DEF_GrB_DIV_FP32_function GB_DIV_f_FP32
#define GB_DEF_GrB_DIV_FP32_ztype float
#define GB_DEF_GrB_DIV_FP32_xtype float
#define GB_DEF_GrB_DIV_FP32_ytype float

#define GB_DEF_GrB_DIV_FP64_function GB_DIV_f_FP64
#define GB_DEF_GrB_DIV_FP64_ztype double
#define GB_DEF_GrB_DIV_FP64_xtype double
#define GB_DEF_GrB_DIV_FP64_ytype double

// op: ISEQ
#define GB_DEF_GxB_ISEQ_BOOL_function GB_ISEQ_f_BOOL
#define GB_DEF_GxB_ISEQ_BOOL_ztype bool
#define GB_DEF_GxB_ISEQ_BOOL_xtype bool
#define GB_DEF_GxB_ISEQ_BOOL_ytype bool

#define GB_DEF_GxB_ISEQ_INT8_function GB_ISEQ_f_INT8
#define GB_DEF_GxB_ISEQ_INT8_ztype int8_t
#define GB_DEF_GxB_ISEQ_INT8_xtype int8_t
#define GB_DEF_GxB_ISEQ_INT8_ytype int8_t

#define GB_DEF_GxB_ISEQ_UINT8_function GB_ISEQ_f_UINT8
#define GB_DEF_GxB_ISEQ_UINT8_ztype uint8_t
#define GB_DEF_GxB_ISEQ_UINT8_xtype uint8_t
#define GB_DEF_GxB_ISEQ_UINT8_ytype uint8_t

#define GB_DEF_GxB_ISEQ_INT16_function GB_ISEQ_f_INT16
#define GB_DEF_GxB_ISEQ_INT16_ztype int16_t
#define GB_DEF_GxB_ISEQ_INT16_xtype int16_t
#define GB_DEF_GxB_ISEQ_INT16_ytype int16_t

#define GB_DEF_GxB_ISEQ_UINT16_function GB_ISEQ_f_UINT16
#define GB_DEF_GxB_ISEQ_UINT16_ztype uint16_t
#define GB_DEF_GxB_ISEQ_UINT16_xtype uint16_t
#define GB_DEF_GxB_ISEQ_UINT16_ytype uint16_t

#define GB_DEF_GxB_ISEQ_INT32_function GB_ISEQ_f_INT32
#define GB_DEF_GxB_ISEQ_INT32_ztype int32_t
#define GB_DEF_GxB_ISEQ_INT32_xtype int32_t
#define GB_DEF_GxB_ISEQ_INT32_ytype int32_t

#define GB_DEF_GxB_ISEQ_UINT32_function GB_ISEQ_f_UINT32
#define GB_DEF_GxB_ISEQ_UINT32_ztype uint32_t
#define GB_DEF_GxB_ISEQ_UINT32_xtype uint32_t
#define GB_DEF_GxB_ISEQ_UINT32_ytype uint32_t

#define GB_DEF_GxB_ISEQ_INT64_function GB_ISEQ_f_INT64
#define GB_DEF_GxB_ISEQ_INT64_ztype int64_t
#define GB_DEF_GxB_ISEQ_INT64_xtype int64_t
#define GB_DEF_GxB_ISEQ_INT64_ytype int64_t

#define GB_DEF_GxB_ISEQ_UINT64_function GB_ISEQ_f_UINT64
#define GB_DEF_GxB_ISEQ_UINT64_ztype uint64_t
#define GB_DEF_GxB_ISEQ_UINT64_xtype uint64_t
#define GB_DEF_GxB_ISEQ_UINT64_ytype uint64_t

#define GB_DEF_GxB_ISEQ_FP32_function GB_ISEQ_f_FP32
#define GB_DEF_GxB_ISEQ_FP32_ztype float
#define GB_DEF_GxB_ISEQ_FP32_xtype float
#define GB_DEF_GxB_ISEQ_FP32_ytype float

#define GB_DEF_GxB_ISEQ_FP64_function GB_ISEQ_f_FP64
#define GB_DEF_GxB_ISEQ_FP64_ztype double
#define GB_DEF_GxB_ISEQ_FP64_xtype double
#define GB_DEF_GxB_ISEQ_FP64_ytype double

// op: ISNE
#define GB_DEF_GxB_ISNE_BOOL_function GB_ISNE_f_BOOL
#define GB_DEF_GxB_ISNE_BOOL_ztype bool
#define GB_DEF_GxB_ISNE_BOOL_xtype bool
#define GB_DEF_GxB_ISNE_BOOL_ytype bool

#define GB_DEF_GxB_ISNE_INT8_function GB_ISNE_f_INT8
#define GB_DEF_GxB_ISNE_INT8_ztype int8_t
#define GB_DEF_GxB_ISNE_INT8_xtype int8_t
#define GB_DEF_GxB_ISNE_INT8_ytype int8_t

#define GB_DEF_GxB_ISNE_UINT8_function GB_ISNE_f_UINT8
#define GB_DEF_GxB_ISNE_UINT8_ztype uint8_t
#define GB_DEF_GxB_ISNE_UINT8_xtype uint8_t
#define GB_DEF_GxB_ISNE_UINT8_ytype uint8_t

#define GB_DEF_GxB_ISNE_INT16_function GB_ISNE_f_INT16
#define GB_DEF_GxB_ISNE_INT16_ztype int16_t
#define GB_DEF_GxB_ISNE_INT16_xtype int16_t
#define GB_DEF_GxB_ISNE_INT16_ytype int16_t

#define GB_DEF_GxB_ISNE_UINT16_function GB_ISNE_f_UINT16
#define GB_DEF_GxB_ISNE_UINT16_ztype uint16_t
#define GB_DEF_GxB_ISNE_UINT16_xtype uint16_t
#define GB_DEF_GxB_ISNE_UINT16_ytype uint16_t

#define GB_DEF_GxB_ISNE_INT32_function GB_ISNE_f_INT32
#define GB_DEF_GxB_ISNE_INT32_ztype int32_t
#define GB_DEF_GxB_ISNE_INT32_xtype int32_t
#define GB_DEF_GxB_ISNE_INT32_ytype int32_t

#define GB_DEF_GxB_ISNE_UINT32_function GB_ISNE_f_UINT32
#define GB_DEF_GxB_ISNE_UINT32_ztype uint32_t
#define GB_DEF_GxB_ISNE_UINT32_xtype uint32_t
#define GB_DEF_GxB_ISNE_UINT32_ytype uint32_t

#define GB_DEF_GxB_ISNE_INT64_function GB_ISNE_f_INT64
#define GB_DEF_GxB_ISNE_INT64_ztype int64_t
#define GB_DEF_GxB_ISNE_INT64_xtype int64_t
#define GB_DEF_GxB_ISNE_INT64_ytype int64_t

#define GB_DEF_GxB_ISNE_UINT64_function GB_ISNE_f_UINT64
#define GB_DEF_GxB_ISNE_UINT64_ztype uint64_t
#define GB_DEF_GxB_ISNE_UINT64_xtype uint64_t
#define GB_DEF_GxB_ISNE_UINT64_ytype uint64_t

#define GB_DEF_GxB_ISNE_FP32_function GB_ISNE_f_FP32
#define GB_DEF_GxB_ISNE_FP32_ztype float
#define GB_DEF_GxB_ISNE_FP32_xtype float
#define GB_DEF_GxB_ISNE_FP32_ytype float

#define GB_DEF_GxB_ISNE_FP64_function GB_ISNE_f_FP64
#define GB_DEF_GxB_ISNE_FP64_ztype double
#define GB_DEF_GxB_ISNE_FP64_xtype double
#define GB_DEF_GxB_ISNE_FP64_ytype double

// op: ISGT
#define GB_DEF_GxB_ISGT_BOOL_function GB_ISGT_f_BOOL
#define GB_DEF_GxB_ISGT_BOOL_ztype bool
#define GB_DEF_GxB_ISGT_BOOL_xtype bool
#define GB_DEF_GxB_ISGT_BOOL_ytype bool

#define GB_DEF_GxB_ISGT_INT8_function GB_ISGT_f_INT8
#define GB_DEF_GxB_ISGT_INT8_ztype int8_t
#define GB_DEF_GxB_ISGT_INT8_xtype int8_t
#define GB_DEF_GxB_ISGT_INT8_ytype int8_t

#define GB_DEF_GxB_ISGT_UINT8_function GB_ISGT_f_UINT8
#define GB_DEF_GxB_ISGT_UINT8_ztype uint8_t
#define GB_DEF_GxB_ISGT_UINT8_xtype uint8_t
#define GB_DEF_GxB_ISGT_UINT8_ytype uint8_t

#define GB_DEF_GxB_ISGT_INT16_function GB_ISGT_f_INT16
#define GB_DEF_GxB_ISGT_INT16_ztype int16_t
#define GB_DEF_GxB_ISGT_INT16_xtype int16_t
#define GB_DEF_GxB_ISGT_INT16_ytype int16_t

#define GB_DEF_GxB_ISGT_UINT16_function GB_ISGT_f_UINT16
#define GB_DEF_GxB_ISGT_UINT16_ztype uint16_t
#define GB_DEF_GxB_ISGT_UINT16_xtype uint16_t
#define GB_DEF_GxB_ISGT_UINT16_ytype uint16_t

#define GB_DEF_GxB_ISGT_INT32_function GB_ISGT_f_INT32
#define GB_DEF_GxB_ISGT_INT32_ztype int32_t
#define GB_DEF_GxB_ISGT_INT32_xtype int32_t
#define GB_DEF_GxB_ISGT_INT32_ytype int32_t

#define GB_DEF_GxB_ISGT_UINT32_function GB_ISGT_f_UINT32
#define GB_DEF_GxB_ISGT_UINT32_ztype uint32_t
#define GB_DEF_GxB_ISGT_UINT32_xtype uint32_t
#define GB_DEF_GxB_ISGT_UINT32_ytype uint32_t

#define GB_DEF_GxB_ISGT_INT64_function GB_ISGT_f_INT64
#define GB_DEF_GxB_ISGT_INT64_ztype int64_t
#define GB_DEF_GxB_ISGT_INT64_xtype int64_t
#define GB_DEF_GxB_ISGT_INT64_ytype int64_t

#define GB_DEF_GxB_ISGT_UINT64_function GB_ISGT_f_UINT64
#define GB_DEF_GxB_ISGT_UINT64_ztype uint64_t
#define GB_DEF_GxB_ISGT_UINT64_xtype uint64_t
#define GB_DEF_GxB_ISGT_UINT64_ytype uint64_t

#define GB_DEF_GxB_ISGT_FP32_function GB_ISGT_f_FP32
#define GB_DEF_GxB_ISGT_FP32_ztype float
#define GB_DEF_GxB_ISGT_FP32_xtype float
#define GB_DEF_GxB_ISGT_FP32_ytype float

#define GB_DEF_GxB_ISGT_FP64_function GB_ISGT_f_FP64
#define GB_DEF_GxB_ISGT_FP64_ztype double
#define GB_DEF_GxB_ISGT_FP64_xtype double
#define GB_DEF_GxB_ISGT_FP64_ytype double

// op: ISLT
#define GB_DEF_GxB_ISLT_BOOL_function GB_ISLT_f_BOOL
#define GB_DEF_GxB_ISLT_BOOL_ztype bool
#define GB_DEF_GxB_ISLT_BOOL_xtype bool
#define GB_DEF_GxB_ISLT_BOOL_ytype bool

#define GB_DEF_GxB_ISLT_INT8_function GB_ISLT_f_INT8
#define GB_DEF_GxB_ISLT_INT8_ztype int8_t
#define GB_DEF_GxB_ISLT_INT8_xtype int8_t
#define GB_DEF_GxB_ISLT_INT8_ytype int8_t

#define GB_DEF_GxB_ISLT_UINT8_function GB_ISLT_f_UINT8
#define GB_DEF_GxB_ISLT_UINT8_ztype uint8_t
#define GB_DEF_GxB_ISLT_UINT8_xtype uint8_t
#define GB_DEF_GxB_ISLT_UINT8_ytype uint8_t

#define GB_DEF_GxB_ISLT_INT16_function GB_ISLT_f_INT16
#define GB_DEF_GxB_ISLT_INT16_ztype int16_t
#define GB_DEF_GxB_ISLT_INT16_xtype int16_t
#define GB_DEF_GxB_ISLT_INT16_ytype int16_t

#define GB_DEF_GxB_ISLT_UINT16_function GB_ISLT_f_UINT16
#define GB_DEF_GxB_ISLT_UINT16_ztype uint16_t
#define GB_DEF_GxB_ISLT_UINT16_xtype uint16_t
#define GB_DEF_GxB_ISLT_UINT16_ytype uint16_t

#define GB_DEF_GxB_ISLT_INT32_function GB_ISLT_f_INT32
#define GB_DEF_GxB_ISLT_INT32_ztype int32_t
#define GB_DEF_GxB_ISLT_INT32_xtype int32_t
#define GB_DEF_GxB_ISLT_INT32_ytype int32_t

#define GB_DEF_GxB_ISLT_UINT32_function GB_ISLT_f_UINT32
#define GB_DEF_GxB_ISLT_UINT32_ztype uint32_t
#define GB_DEF_GxB_ISLT_UINT32_xtype uint32_t
#define GB_DEF_GxB_ISLT_UINT32_ytype uint32_t

#define GB_DEF_GxB_ISLT_INT64_function GB_ISLT_f_INT64
#define GB_DEF_GxB_ISLT_INT64_ztype int64_t
#define GB_DEF_GxB_ISLT_INT64_xtype int64_t
#define GB_DEF_GxB_ISLT_INT64_ytype int64_t

#define GB_DEF_GxB_ISLT_UINT64_function GB_ISLT_f_UINT64
#define GB_DEF_GxB_ISLT_UINT64_ztype uint64_t
#define GB_DEF_GxB_ISLT_UINT64_xtype uint64_t
#define GB_DEF_GxB_ISLT_UINT64_ytype uint64_t

#define GB_DEF_GxB_ISLT_FP32_function GB_ISLT_f_FP32
#define GB_DEF_GxB_ISLT_FP32_ztype float
#define GB_DEF_GxB_ISLT_FP32_xtype float
#define GB_DEF_GxB_ISLT_FP32_ytype float

#define GB_DEF_GxB_ISLT_FP64_function GB_ISLT_f_FP64
#define GB_DEF_GxB_ISLT_FP64_ztype double
#define GB_DEF_GxB_ISLT_FP64_xtype double
#define GB_DEF_GxB_ISLT_FP64_ytype double

// op: ISGE
#define GB_DEF_GxB_ISGE_BOOL_function GB_ISGE_f_BOOL
#define GB_DEF_GxB_ISGE_BOOL_ztype bool
#define GB_DEF_GxB_ISGE_BOOL_xtype bool
#define GB_DEF_GxB_ISGE_BOOL_ytype bool

#define GB_DEF_GxB_ISGE_INT8_function GB_ISGE_f_INT8
#define GB_DEF_GxB_ISGE_INT8_ztype int8_t
#define GB_DEF_GxB_ISGE_INT8_xtype int8_t
#define GB_DEF_GxB_ISGE_INT8_ytype int8_t

#define GB_DEF_GxB_ISGE_UINT8_function GB_ISGE_f_UINT8
#define GB_DEF_GxB_ISGE_UINT8_ztype uint8_t
#define GB_DEF_GxB_ISGE_UINT8_xtype uint8_t
#define GB_DEF_GxB_ISGE_UINT8_ytype uint8_t

#define GB_DEF_GxB_ISGE_INT16_function GB_ISGE_f_INT16
#define GB_DEF_GxB_ISGE_INT16_ztype int16_t
#define GB_DEF_GxB_ISGE_INT16_xtype int16_t
#define GB_DEF_GxB_ISGE_INT16_ytype int16_t

#define GB_DEF_GxB_ISGE_UINT16_function GB_ISGE_f_UINT16
#define GB_DEF_GxB_ISGE_UINT16_ztype uint16_t
#define GB_DEF_GxB_ISGE_UINT16_xtype uint16_t
#define GB_DEF_GxB_ISGE_UINT16_ytype uint16_t

#define GB_DEF_GxB_ISGE_INT32_function GB_ISGE_f_INT32
#define GB_DEF_GxB_ISGE_INT32_ztype int32_t
#define GB_DEF_GxB_ISGE_INT32_xtype int32_t
#define GB_DEF_GxB_ISGE_INT32_ytype int32_t

#define GB_DEF_GxB_ISGE_UINT32_function GB_ISGE_f_UINT32
#define GB_DEF_GxB_ISGE_UINT32_ztype uint32_t
#define GB_DEF_GxB_ISGE_UINT32_xtype uint32_t
#define GB_DEF_GxB_ISGE_UINT32_ytype uint32_t

#define GB_DEF_GxB_ISGE_INT64_function GB_ISGE_f_INT64
#define GB_DEF_GxB_ISGE_INT64_ztype int64_t
#define GB_DEF_GxB_ISGE_INT64_xtype int64_t
#define GB_DEF_GxB_ISGE_INT64_ytype int64_t

#define GB_DEF_GxB_ISGE_UINT64_function GB_ISGE_f_UINT64
#define GB_DEF_GxB_ISGE_UINT64_ztype uint64_t
#define GB_DEF_GxB_ISGE_UINT64_xtype uint64_t
#define GB_DEF_GxB_ISGE_UINT64_ytype uint64_t

#define GB_DEF_GxB_ISGE_FP32_function GB_ISGE_f_FP32
#define GB_DEF_GxB_ISGE_FP32_ztype float
#define GB_DEF_GxB_ISGE_FP32_xtype float
#define GB_DEF_GxB_ISGE_FP32_ytype float

#define GB_DEF_GxB_ISGE_FP64_function GB_ISGE_f_FP64
#define GB_DEF_GxB_ISGE_FP64_ztype double
#define GB_DEF_GxB_ISGE_FP64_xtype double
#define GB_DEF_GxB_ISGE_FP64_ytype double

// op: ISLE
#define GB_DEF_GxB_ISLE_BOOL_function GB_ISLE_f_BOOL
#define GB_DEF_GxB_ISLE_BOOL_ztype bool
#define GB_DEF_GxB_ISLE_BOOL_xtype bool
#define GB_DEF_GxB_ISLE_BOOL_ytype bool

#define GB_DEF_GxB_ISLE_INT8_function GB_ISLE_f_INT8
#define GB_DEF_GxB_ISLE_INT8_ztype int8_t
#define GB_DEF_GxB_ISLE_INT8_xtype int8_t
#define GB_DEF_GxB_ISLE_INT8_ytype int8_t

#define GB_DEF_GxB_ISLE_UINT8_function GB_ISLE_f_UINT8
#define GB_DEF_GxB_ISLE_UINT8_ztype uint8_t
#define GB_DEF_GxB_ISLE_UINT8_xtype uint8_t
#define GB_DEF_GxB_ISLE_UINT8_ytype uint8_t

#define GB_DEF_GxB_ISLE_INT16_function GB_ISLE_f_INT16
#define GB_DEF_GxB_ISLE_INT16_ztype int16_t
#define GB_DEF_GxB_ISLE_INT16_xtype int16_t
#define GB_DEF_GxB_ISLE_INT16_ytype int16_t

#define GB_DEF_GxB_ISLE_UINT16_function GB_ISLE_f_UINT16
#define GB_DEF_GxB_ISLE_UINT16_ztype uint16_t
#define GB_DEF_GxB_ISLE_UINT16_xtype uint16_t
#define GB_DEF_GxB_ISLE_UINT16_ytype uint16_t

#define GB_DEF_GxB_ISLE_INT32_function GB_ISLE_f_INT32
#define GB_DEF_GxB_ISLE_INT32_ztype int32_t
#define GB_DEF_GxB_ISLE_INT32_xtype int32_t
#define GB_DEF_GxB_ISLE_INT32_ytype int32_t

#define GB_DEF_GxB_ISLE_UINT32_function GB_ISLE_f_UINT32
#define GB_DEF_GxB_ISLE_UINT32_ztype uint32_t
#define GB_DEF_GxB_ISLE_UINT32_xtype uint32_t
#define GB_DEF_GxB_ISLE_UINT32_ytype uint32_t

#define GB_DEF_GxB_ISLE_INT64_function GB_ISLE_f_INT64
#define GB_DEF_GxB_ISLE_INT64_ztype int64_t
#define GB_DEF_GxB_ISLE_INT64_xtype int64_t
#define GB_DEF_GxB_ISLE_INT64_ytype int64_t

#define GB_DEF_GxB_ISLE_UINT64_function GB_ISLE_f_UINT64
#define GB_DEF_GxB_ISLE_UINT64_ztype uint64_t
#define GB_DEF_GxB_ISLE_UINT64_xtype uint64_t
#define GB_DEF_GxB_ISLE_UINT64_ytype uint64_t

#define GB_DEF_GxB_ISLE_FP32_function GB_ISLE_f_FP32
#define GB_DEF_GxB_ISLE_FP32_ztype float
#define GB_DEF_GxB_ISLE_FP32_xtype float
#define GB_DEF_GxB_ISLE_FP32_ytype float

#define GB_DEF_GxB_ISLE_FP64_function GB_ISLE_f_FP64
#define GB_DEF_GxB_ISLE_FP64_ztype double
#define GB_DEF_GxB_ISLE_FP64_xtype double
#define GB_DEF_GxB_ISLE_FP64_ytype double

// op: LOR
#define GB_DEF_GxB_LOR_BOOL_function GB_LOR_f_BOOL
#define GB_DEF_GxB_LOR_BOOL_ztype bool
#define GB_DEF_GxB_LOR_BOOL_xtype bool
#define GB_DEF_GxB_LOR_BOOL_ytype bool

#define GB_DEF_GxB_LOR_INT8_function GB_LOR_f_INT8
#define GB_DEF_GxB_LOR_INT8_ztype int8_t
#define GB_DEF_GxB_LOR_INT8_xtype int8_t
#define GB_DEF_GxB_LOR_INT8_ytype int8_t

#define GB_DEF_GxB_LOR_UINT8_function GB_LOR_f_UINT8
#define GB_DEF_GxB_LOR_UINT8_ztype uint8_t
#define GB_DEF_GxB_LOR_UINT8_xtype uint8_t
#define GB_DEF_GxB_LOR_UINT8_ytype uint8_t

#define GB_DEF_GxB_LOR_INT16_function GB_LOR_f_INT16
#define GB_DEF_GxB_LOR_INT16_ztype int16_t
#define GB_DEF_GxB_LOR_INT16_xtype int16_t
#define GB_DEF_GxB_LOR_INT16_ytype int16_t

#define GB_DEF_GxB_LOR_UINT16_function GB_LOR_f_UINT16
#define GB_DEF_GxB_LOR_UINT16_ztype uint16_t
#define GB_DEF_GxB_LOR_UINT16_xtype uint16_t
#define GB_DEF_GxB_LOR_UINT16_ytype uint16_t

#define GB_DEF_GxB_LOR_INT32_function GB_LOR_f_INT32
#define GB_DEF_GxB_LOR_INT32_ztype int32_t
#define GB_DEF_GxB_LOR_INT32_xtype int32_t
#define GB_DEF_GxB_LOR_INT32_ytype int32_t

#define GB_DEF_GxB_LOR_UINT32_function GB_LOR_f_UINT32
#define GB_DEF_GxB_LOR_UINT32_ztype uint32_t
#define GB_DEF_GxB_LOR_UINT32_xtype uint32_t
#define GB_DEF_GxB_LOR_UINT32_ytype uint32_t

#define GB_DEF_GxB_LOR_INT64_function GB_LOR_f_INT64
#define GB_DEF_GxB_LOR_INT64_ztype int64_t
#define GB_DEF_GxB_LOR_INT64_xtype int64_t
#define GB_DEF_GxB_LOR_INT64_ytype int64_t

#define GB_DEF_GxB_LOR_UINT64_function GB_LOR_f_UINT64
#define GB_DEF_GxB_LOR_UINT64_ztype uint64_t
#define GB_DEF_GxB_LOR_UINT64_xtype uint64_t
#define GB_DEF_GxB_LOR_UINT64_ytype uint64_t

#define GB_DEF_GxB_LOR_FP32_function GB_LOR_f_FP32
#define GB_DEF_GxB_LOR_FP32_ztype float
#define GB_DEF_GxB_LOR_FP32_xtype float
#define GB_DEF_GxB_LOR_FP32_ytype float

#define GB_DEF_GxB_LOR_FP64_function GB_LOR_f_FP64
#define GB_DEF_GxB_LOR_FP64_ztype double
#define GB_DEF_GxB_LOR_FP64_xtype double
#define GB_DEF_GxB_LOR_FP64_ytype double

// op: LAND
#define GB_DEF_GxB_LAND_BOOL_function GB_LAND_f_BOOL
#define GB_DEF_GxB_LAND_BOOL_ztype bool
#define GB_DEF_GxB_LAND_BOOL_xtype bool
#define GB_DEF_GxB_LAND_BOOL_ytype bool

#define GB_DEF_GxB_LAND_INT8_function GB_LAND_f_INT8
#define GB_DEF_GxB_LAND_INT8_ztype int8_t
#define GB_DEF_GxB_LAND_INT8_xtype int8_t
#define GB_DEF_GxB_LAND_INT8_ytype int8_t

#define GB_DEF_GxB_LAND_UINT8_function GB_LAND_f_UINT8
#define GB_DEF_GxB_LAND_UINT8_ztype uint8_t
#define GB_DEF_GxB_LAND_UINT8_xtype uint8_t
#define GB_DEF_GxB_LAND_UINT8_ytype uint8_t

#define GB_DEF_GxB_LAND_INT16_function GB_LAND_f_INT16
#define GB_DEF_GxB_LAND_INT16_ztype int16_t
#define GB_DEF_GxB_LAND_INT16_xtype int16_t
#define GB_DEF_GxB_LAND_INT16_ytype int16_t

#define GB_DEF_GxB_LAND_UINT16_function GB_LAND_f_UINT16
#define GB_DEF_GxB_LAND_UINT16_ztype uint16_t
#define GB_DEF_GxB_LAND_UINT16_xtype uint16_t
#define GB_DEF_GxB_LAND_UINT16_ytype uint16_t

#define GB_DEF_GxB_LAND_INT32_function GB_LAND_f_INT32
#define GB_DEF_GxB_LAND_INT32_ztype int32_t
#define GB_DEF_GxB_LAND_INT32_xtype int32_t
#define GB_DEF_GxB_LAND_INT32_ytype int32_t

#define GB_DEF_GxB_LAND_UINT32_function GB_LAND_f_UINT32
#define GB_DEF_GxB_LAND_UINT32_ztype uint32_t
#define GB_DEF_GxB_LAND_UINT32_xtype uint32_t
#define GB_DEF_GxB_LAND_UINT32_ytype uint32_t

#define GB_DEF_GxB_LAND_INT64_function GB_LAND_f_INT64
#define GB_DEF_GxB_LAND_INT64_ztype int64_t
#define GB_DEF_GxB_LAND_INT64_xtype int64_t
#define GB_DEF_GxB_LAND_INT64_ytype int64_t

#define GB_DEF_GxB_LAND_UINT64_function GB_LAND_f_UINT64
#define GB_DEF_GxB_LAND_UINT64_ztype uint64_t
#define GB_DEF_GxB_LAND_UINT64_xtype uint64_t
#define GB_DEF_GxB_LAND_UINT64_ytype uint64_t

#define GB_DEF_GxB_LAND_FP32_function GB_LAND_f_FP32
#define GB_DEF_GxB_LAND_FP32_ztype float
#define GB_DEF_GxB_LAND_FP32_xtype float
#define GB_DEF_GxB_LAND_FP32_ytype float

#define GB_DEF_GxB_LAND_FP64_function GB_LAND_f_FP64
#define GB_DEF_GxB_LAND_FP64_ztype double
#define GB_DEF_GxB_LAND_FP64_xtype double
#define GB_DEF_GxB_LAND_FP64_ytype double

// op: LXOR
#define GB_DEF_GxB_LXOR_BOOL_function GB_LXOR_f_BOOL
#define GB_DEF_GxB_LXOR_BOOL_ztype bool
#define GB_DEF_GxB_LXOR_BOOL_xtype bool
#define GB_DEF_GxB_LXOR_BOOL_ytype bool

#define GB_DEF_GxB_LXOR_INT8_function GB_LXOR_f_INT8
#define GB_DEF_GxB_LXOR_INT8_ztype int8_t
#define GB_DEF_GxB_LXOR_INT8_xtype int8_t
#define GB_DEF_GxB_LXOR_INT8_ytype int8_t

#define GB_DEF_GxB_LXOR_UINT8_function GB_LXOR_f_UINT8
#define GB_DEF_GxB_LXOR_UINT8_ztype uint8_t
#define GB_DEF_GxB_LXOR_UINT8_xtype uint8_t
#define GB_DEF_GxB_LXOR_UINT8_ytype uint8_t

#define GB_DEF_GxB_LXOR_INT16_function GB_LXOR_f_INT16
#define GB_DEF_GxB_LXOR_INT16_ztype int16_t
#define GB_DEF_GxB_LXOR_INT16_xtype int16_t
#define GB_DEF_GxB_LXOR_INT16_ytype int16_t

#define GB_DEF_GxB_LXOR_UINT16_function GB_LXOR_f_UINT16
#define GB_DEF_GxB_LXOR_UINT16_ztype uint16_t
#define GB_DEF_GxB_LXOR_UINT16_xtype uint16_t
#define GB_DEF_GxB_LXOR_UINT16_ytype uint16_t

#define GB_DEF_GxB_LXOR_INT32_function GB_LXOR_f_INT32
#define GB_DEF_GxB_LXOR_INT32_ztype int32_t
#define GB_DEF_GxB_LXOR_INT32_xtype int32_t
#define GB_DEF_GxB_LXOR_INT32_ytype int32_t

#define GB_DEF_GxB_LXOR_UINT32_function GB_LXOR_f_UINT32
#define GB_DEF_GxB_LXOR_UINT32_ztype uint32_t
#define GB_DEF_GxB_LXOR_UINT32_xtype uint32_t
#define GB_DEF_GxB_LXOR_UINT32_ytype uint32_t

#define GB_DEF_GxB_LXOR_INT64_function GB_LXOR_f_INT64
#define GB_DEF_GxB_LXOR_INT64_ztype int64_t
#define GB_DEF_GxB_LXOR_INT64_xtype int64_t
#define GB_DEF_GxB_LXOR_INT64_ytype int64_t

#define GB_DEF_GxB_LXOR_UINT64_function GB_LXOR_f_UINT64
#define GB_DEF_GxB_LXOR_UINT64_ztype uint64_t
#define GB_DEF_GxB_LXOR_UINT64_xtype uint64_t
#define GB_DEF_GxB_LXOR_UINT64_ytype uint64_t

#define GB_DEF_GxB_LXOR_FP32_function GB_LXOR_f_FP32
#define GB_DEF_GxB_LXOR_FP32_ztype float
#define GB_DEF_GxB_LXOR_FP32_xtype float
#define GB_DEF_GxB_LXOR_FP32_ytype float

#define GB_DEF_GxB_LXOR_FP64_function GB_LXOR_f_FP64
#define GB_DEF_GxB_LXOR_FP64_ztype double
#define GB_DEF_GxB_LXOR_FP64_xtype double
#define GB_DEF_GxB_LXOR_FP64_ytype double


//------------------------------------------------------
// binary operators of the form z=f(x,y): TxT -> bool
//------------------------------------------------------

// op: EQ
#define GB_DEF_GrB_EQ_BOOL_function GB_EQ_f_BOOL
#define GB_DEF_GrB_EQ_BOOL_ztype bool
#define GB_DEF_GrB_EQ_BOOL_xtype bool
#define GB_DEF_GrB_EQ_BOOL_ytype bool

#define GB_DEF_GrB_EQ_INT8_function GB_EQ_f_INT8
#define GB_DEF_GrB_EQ_INT8_ztype bool
#define GB_DEF_GrB_EQ_INT8_xtype int8_t
#define GB_DEF_GrB_EQ_INT8_ytype int8_t

#define GB_DEF_GrB_EQ_UINT8_function GB_EQ_f_UINT8
#define GB_DEF_GrB_EQ_UINT8_ztype bool
#define GB_DEF_GrB_EQ_UINT8_xtype uint8_t
#define GB_DEF_GrB_EQ_UINT8_ytype uint8_t

#define GB_DEF_GrB_EQ_INT16_function GB_EQ_f_INT16
#define GB_DEF_GrB_EQ_INT16_ztype bool
#define GB_DEF_GrB_EQ_INT16_xtype int16_t
#define GB_DEF_GrB_EQ_INT16_ytype int16_t

#define GB_DEF_GrB_EQ_UINT16_function GB_EQ_f_UINT16
#define GB_DEF_GrB_EQ_UINT16_ztype bool
#define GB_DEF_GrB_EQ_UINT16_xtype uint16_t
#define GB_DEF_GrB_EQ_UINT16_ytype uint16_t

#define GB_DEF_GrB_EQ_INT32_function GB_EQ_f_INT32
#define GB_DEF_GrB_EQ_INT32_ztype bool
#define GB_DEF_GrB_EQ_INT32_xtype int32_t
#define GB_DEF_GrB_EQ_INT32_ytype int32_t

#define GB_DEF_GrB_EQ_UINT32_function GB_EQ_f_UINT32
#define GB_DEF_GrB_EQ_UINT32_ztype bool
#define GB_DEF_GrB_EQ_UINT32_xtype uint32_t
#define GB_DEF_GrB_EQ_UINT32_ytype uint32_t

#define GB_DEF_GrB_EQ_INT64_function GB_EQ_f_INT64
#define GB_DEF_GrB_EQ_INT64_ztype bool
#define GB_DEF_GrB_EQ_INT64_xtype int64_t
#define GB_DEF_GrB_EQ_INT64_ytype int64_t

#define GB_DEF_GrB_EQ_UINT64_function GB_EQ_f_UINT64
#define GB_DEF_GrB_EQ_UINT64_ztype bool
#define GB_DEF_GrB_EQ_UINT64_xtype uint64_t
#define GB_DEF_GrB_EQ_UINT64_ytype uint64_t

#define GB_DEF_GrB_EQ_FP32_function GB_EQ_f_FP32
#define GB_DEF_GrB_EQ_FP32_ztype bool
#define GB_DEF_GrB_EQ_FP32_xtype float
#define GB_DEF_GrB_EQ_FP32_ytype float

#define GB_DEF_GrB_EQ_FP64_function GB_EQ_f_FP64
#define GB_DEF_GrB_EQ_FP64_ztype bool
#define GB_DEF_GrB_EQ_FP64_xtype double
#define GB_DEF_GrB_EQ_FP64_ytype double

// op: NE
#define GB_DEF_GrB_NE_BOOL_function GB_NE_f_BOOL
#define GB_DEF_GrB_NE_BOOL_ztype bool
#define GB_DEF_GrB_NE_BOOL_xtype bool
#define GB_DEF_GrB_NE_BOOL_ytype bool

#define GB_DEF_GrB_NE_INT8_function GB_NE_f_INT8
#define GB_DEF_GrB_NE_INT8_ztype bool
#define GB_DEF_GrB_NE_INT8_xtype int8_t
#define GB_DEF_GrB_NE_INT8_ytype int8_t

#define GB_DEF_GrB_NE_UINT8_function GB_NE_f_UINT8
#define GB_DEF_GrB_NE_UINT8_ztype bool
#define GB_DEF_GrB_NE_UINT8_xtype uint8_t
#define GB_DEF_GrB_NE_UINT8_ytype uint8_t

#define GB_DEF_GrB_NE_INT16_function GB_NE_f_INT16
#define GB_DEF_GrB_NE_INT16_ztype bool
#define GB_DEF_GrB_NE_INT16_xtype int16_t
#define GB_DEF_GrB_NE_INT16_ytype int16_t

#define GB_DEF_GrB_NE_UINT16_function GB_NE_f_UINT16
#define GB_DEF_GrB_NE_UINT16_ztype bool
#define GB_DEF_GrB_NE_UINT16_xtype uint16_t
#define GB_DEF_GrB_NE_UINT16_ytype uint16_t

#define GB_DEF_GrB_NE_INT32_function GB_NE_f_INT32
#define GB_DEF_GrB_NE_INT32_ztype bool
#define GB_DEF_GrB_NE_INT32_xtype int32_t
#define GB_DEF_GrB_NE_INT32_ytype int32_t

#define GB_DEF_GrB_NE_UINT32_function GB_NE_f_UINT32
#define GB_DEF_GrB_NE_UINT32_ztype bool
#define GB_DEF_GrB_NE_UINT32_xtype uint32_t
#define GB_DEF_GrB_NE_UINT32_ytype uint32_t

#define GB_DEF_GrB_NE_INT64_function GB_NE_f_INT64
#define GB_DEF_GrB_NE_INT64_ztype bool
#define GB_DEF_GrB_NE_INT64_xtype int64_t
#define GB_DEF_GrB_NE_INT64_ytype int64_t

#define GB_DEF_GrB_NE_UINT64_function GB_NE_f_UINT64
#define GB_DEF_GrB_NE_UINT64_ztype bool
#define GB_DEF_GrB_NE_UINT64_xtype uint64_t
#define GB_DEF_GrB_NE_UINT64_ytype uint64_t

#define GB_DEF_GrB_NE_FP32_function GB_NE_f_FP32
#define GB_DEF_GrB_NE_FP32_ztype bool
#define GB_DEF_GrB_NE_FP32_xtype float
#define GB_DEF_GrB_NE_FP32_ytype float

#define GB_DEF_GrB_NE_FP64_function GB_NE_f_FP64
#define GB_DEF_GrB_NE_FP64_ztype bool
#define GB_DEF_GrB_NE_FP64_xtype double
#define GB_DEF_GrB_NE_FP64_ytype double

// op: GT
#define GB_DEF_GrB_GT_BOOL_function GB_GT_f_BOOL
#define GB_DEF_GrB_GT_BOOL_ztype bool
#define GB_DEF_GrB_GT_BOOL_xtype bool
#define GB_DEF_GrB_GT_BOOL_ytype bool

#define GB_DEF_GrB_GT_INT8_function GB_GT_f_INT8
#define GB_DEF_GrB_GT_INT8_ztype bool
#define GB_DEF_GrB_GT_INT8_xtype int8_t
#define GB_DEF_GrB_GT_INT8_ytype int8_t

#define GB_DEF_GrB_GT_UINT8_function GB_GT_f_UINT8
#define GB_DEF_GrB_GT_UINT8_ztype bool
#define GB_DEF_GrB_GT_UINT8_xtype uint8_t
#define GB_DEF_GrB_GT_UINT8_ytype uint8_t

#define GB_DEF_GrB_GT_INT16_function GB_GT_f_INT16
#define GB_DEF_GrB_GT_INT16_ztype bool
#define GB_DEF_GrB_GT_INT16_xtype int16_t
#define GB_DEF_GrB_GT_INT16_ytype int16_t

#define GB_DEF_GrB_GT_UINT16_function GB_GT_f_UINT16
#define GB_DEF_GrB_GT_UINT16_ztype bool
#define GB_DEF_GrB_GT_UINT16_xtype uint16_t
#define GB_DEF_GrB_GT_UINT16_ytype uint16_t

#define GB_DEF_GrB_GT_INT32_function GB_GT_f_INT32
#define GB_DEF_GrB_GT_INT32_ztype bool
#define GB_DEF_GrB_GT_INT32_xtype int32_t
#define GB_DEF_GrB_GT_INT32_ytype int32_t

#define GB_DEF_GrB_GT_UINT32_function GB_GT_f_UINT32
#define GB_DEF_GrB_GT_UINT32_ztype bool
#define GB_DEF_GrB_GT_UINT32_xtype uint32_t
#define GB_DEF_GrB_GT_UINT32_ytype uint32_t

#define GB_DEF_GrB_GT_INT64_function GB_GT_f_INT64
#define GB_DEF_GrB_GT_INT64_ztype bool
#define GB_DEF_GrB_GT_INT64_xtype int64_t
#define GB_DEF_GrB_GT_INT64_ytype int64_t

#define GB_DEF_GrB_GT_UINT64_function GB_GT_f_UINT64
#define GB_DEF_GrB_GT_UINT64_ztype bool
#define GB_DEF_GrB_GT_UINT64_xtype uint64_t
#define GB_DEF_GrB_GT_UINT64_ytype uint64_t

#define GB_DEF_GrB_GT_FP32_function GB_GT_f_FP32
#define GB_DEF_GrB_GT_FP32_ztype bool
#define GB_DEF_GrB_GT_FP32_xtype float
#define GB_DEF_GrB_GT_FP32_ytype float

#define GB_DEF_GrB_GT_FP64_function GB_GT_f_FP64
#define GB_DEF_GrB_GT_FP64_ztype bool
#define GB_DEF_GrB_GT_FP64_xtype double
#define GB_DEF_GrB_GT_FP64_ytype double

// op: LT
#define GB_DEF_GrB_LT_BOOL_function GB_LT_f_BOOL
#define GB_DEF_GrB_LT_BOOL_ztype bool
#define GB_DEF_GrB_LT_BOOL_xtype bool
#define GB_DEF_GrB_LT_BOOL_ytype bool

#define GB_DEF_GrB_LT_INT8_function GB_LT_f_INT8
#define GB_DEF_GrB_LT_INT8_ztype bool
#define GB_DEF_GrB_LT_INT8_xtype int8_t
#define GB_DEF_GrB_LT_INT8_ytype int8_t

#define GB_DEF_GrB_LT_UINT8_function GB_LT_f_UINT8
#define GB_DEF_GrB_LT_UINT8_ztype bool
#define GB_DEF_GrB_LT_UINT8_xtype uint8_t
#define GB_DEF_GrB_LT_UINT8_ytype uint8_t

#define GB_DEF_GrB_LT_INT16_function GB_LT_f_INT16
#define GB_DEF_GrB_LT_INT16_ztype bool
#define GB_DEF_GrB_LT_INT16_xtype int16_t
#define GB_DEF_GrB_LT_INT16_ytype int16_t

#define GB_DEF_GrB_LT_UINT16_function GB_LT_f_UINT16
#define GB_DEF_GrB_LT_UINT16_ztype bool
#define GB_DEF_GrB_LT_UINT16_xtype uint16_t
#define GB_DEF_GrB_LT_UINT16_ytype uint16_t

#define GB_DEF_GrB_LT_INT32_function GB_LT_f_INT32
#define GB_DEF_GrB_LT_INT32_ztype bool
#define GB_DEF_GrB_LT_INT32_xtype int32_t
#define GB_DEF_GrB_LT_INT32_ytype int32_t

#define GB_DEF_GrB_LT_UINT32_function GB_LT_f_UINT32
#define GB_DEF_GrB_LT_UINT32_ztype bool
#define GB_DEF_GrB_LT_UINT32_xtype uint32_t
#define GB_DEF_GrB_LT_UINT32_ytype uint32_t

#define GB_DEF_GrB_LT_INT64_function GB_LT_f_INT64
#define GB_DEF_GrB_LT_INT64_ztype bool
#define GB_DEF_GrB_LT_INT64_xtype int64_t
#define GB_DEF_GrB_LT_INT64_ytype int64_t

#define GB_DEF_GrB_LT_UINT64_function GB_LT_f_UINT64
#define GB_DEF_GrB_LT_UINT64_ztype bool
#define GB_DEF_GrB_LT_UINT64_xtype uint64_t
#define GB_DEF_GrB_LT_UINT64_ytype uint64_t

#define GB_DEF_GrB_LT_FP32_function GB_LT_f_FP32
#define GB_DEF_GrB_LT_FP32_ztype bool
#define GB_DEF_GrB_LT_FP32_xtype float
#define GB_DEF_GrB_LT_FP32_ytype float

#define GB_DEF_GrB_LT_FP64_function GB_LT_f_FP64
#define GB_DEF_GrB_LT_FP64_ztype bool
#define GB_DEF_GrB_LT_FP64_xtype double
#define GB_DEF_GrB_LT_FP64_ytype double

// op: GE
#define GB_DEF_GrB_GE_BOOL_function GB_GE_f_BOOL
#define GB_DEF_GrB_GE_BOOL_ztype bool
#define GB_DEF_GrB_GE_BOOL_xtype bool
#define GB_DEF_GrB_GE_BOOL_ytype bool

#define GB_DEF_GrB_GE_INT8_function GB_GE_f_INT8
#define GB_DEF_GrB_GE_INT8_ztype bool
#define GB_DEF_GrB_GE_INT8_xtype int8_t
#define GB_DEF_GrB_GE_INT8_ytype int8_t

#define GB_DEF_GrB_GE_UINT8_function GB_GE_f_UINT8
#define GB_DEF_GrB_GE_UINT8_ztype bool
#define GB_DEF_GrB_GE_UINT8_xtype uint8_t
#define GB_DEF_GrB_GE_UINT8_ytype uint8_t

#define GB_DEF_GrB_GE_INT16_function GB_GE_f_INT16
#define GB_DEF_GrB_GE_INT16_ztype bool
#define GB_DEF_GrB_GE_INT16_xtype int16_t
#define GB_DEF_GrB_GE_INT16_ytype int16_t

#define GB_DEF_GrB_GE_UINT16_function GB_GE_f_UINT16
#define GB_DEF_GrB_GE_UINT16_ztype bool
#define GB_DEF_GrB_GE_UINT16_xtype uint16_t
#define GB_DEF_GrB_GE_UINT16_ytype uint16_t

#define GB_DEF_GrB_GE_INT32_function GB_GE_f_INT32
#define GB_DEF_GrB_GE_INT32_ztype bool
#define GB_DEF_GrB_GE_INT32_xtype int32_t
#define GB_DEF_GrB_GE_INT32_ytype int32_t

#define GB_DEF_GrB_GE_UINT32_function GB_GE_f_UINT32
#define GB_DEF_GrB_GE_UINT32_ztype bool
#define GB_DEF_GrB_GE_UINT32_xtype uint32_t
#define GB_DEF_GrB_GE_UINT32_ytype uint32_t

#define GB_DEF_GrB_GE_INT64_function GB_GE_f_INT64
#define GB_DEF_GrB_GE_INT64_ztype bool
#define GB_DEF_GrB_GE_INT64_xtype int64_t
#define GB_DEF_GrB_GE_INT64_ytype int64_t

#define GB_DEF_GrB_GE_UINT64_function GB_GE_f_UINT64
#define GB_DEF_GrB_GE_UINT64_ztype bool
#define GB_DEF_GrB_GE_UINT64_xtype uint64_t
#define GB_DEF_GrB_GE_UINT64_ytype uint64_t

#define GB_DEF_GrB_GE_FP32_function GB_GE_f_FP32
#define GB_DEF_GrB_GE_FP32_ztype bool
#define GB_DEF_GrB_GE_FP32_xtype float
#define GB_DEF_GrB_GE_FP32_ytype float

#define GB_DEF_GrB_GE_FP64_function GB_GE_f_FP64
#define GB_DEF_GrB_GE_FP64_ztype bool
#define GB_DEF_GrB_GE_FP64_xtype double
#define GB_DEF_GrB_GE_FP64_ytype double

// op: LE
#define GB_DEF_GrB_LE_BOOL_function GB_LE_f_BOOL
#define GB_DEF_GrB_LE_BOOL_ztype bool
#define GB_DEF_GrB_LE_BOOL_xtype bool
#define GB_DEF_GrB_LE_BOOL_ytype bool

#define GB_DEF_GrB_LE_INT8_function GB_LE_f_INT8
#define GB_DEF_GrB_LE_INT8_ztype bool
#define GB_DEF_GrB_LE_INT8_xtype int8_t
#define GB_DEF_GrB_LE_INT8_ytype int8_t

#define GB_DEF_GrB_LE_UINT8_function GB_LE_f_UINT8
#define GB_DEF_GrB_LE_UINT8_ztype bool
#define GB_DEF_GrB_LE_UINT8_xtype uint8_t
#define GB_DEF_GrB_LE_UINT8_ytype uint8_t

#define GB_DEF_GrB_LE_INT16_function GB_LE_f_INT16
#define GB_DEF_GrB_LE_INT16_ztype bool
#define GB_DEF_GrB_LE_INT16_xtype int16_t
#define GB_DEF_GrB_LE_INT16_ytype int16_t

#define GB_DEF_GrB_LE_UINT16_function GB_LE_f_UINT16
#define GB_DEF_GrB_LE_UINT16_ztype bool
#define GB_DEF_GrB_LE_UINT16_xtype uint16_t
#define GB_DEF_GrB_LE_UINT16_ytype uint16_t

#define GB_DEF_GrB_LE_INT32_function GB_LE_f_INT32
#define GB_DEF_GrB_LE_INT32_ztype bool
#define GB_DEF_GrB_LE_INT32_xtype int32_t
#define GB_DEF_GrB_LE_INT32_ytype int32_t

#define GB_DEF_GrB_LE_UINT32_function GB_LE_f_UINT32
#define GB_DEF_GrB_LE_UINT32_ztype bool
#define GB_DEF_GrB_LE_UINT32_xtype uint32_t
#define GB_DEF_GrB_LE_UINT32_ytype uint32_t

#define GB_DEF_GrB_LE_INT64_function GB_LE_f_INT64
#define GB_DEF_GrB_LE_INT64_ztype bool
#define GB_DEF_GrB_LE_INT64_xtype int64_t
#define GB_DEF_GrB_LE_INT64_ytype int64_t

#define GB_DEF_GrB_LE_UINT64_function GB_LE_f_UINT64
#define GB_DEF_GrB_LE_UINT64_ztype bool
#define GB_DEF_GrB_LE_UINT64_xtype uint64_t
#define GB_DEF_GrB_LE_UINT64_ytype uint64_t

#define GB_DEF_GrB_LE_FP32_function GB_LE_f_FP32
#define GB_DEF_GrB_LE_FP32_ztype bool
#define GB_DEF_GrB_LE_FP32_xtype float
#define GB_DEF_GrB_LE_FP32_ytype float

#define GB_DEF_GrB_LE_FP64_function GB_LE_f_FP64
#define GB_DEF_GrB_LE_FP64_ztype bool
#define GB_DEF_GrB_LE_FP64_xtype double
#define GB_DEF_GrB_LE_FP64_ytype double


//------------------------------------------------------
// binary operators of the form z=f(x,y): bool x bool -> bool
//------------------------------------------------------

#define GB_DEF_GrB_LOR_function GB_LOR_f_BOOL
#define GB_DEF_GrB_LOR_ztype bool
#define GB_DEF_GrB_LOR_xtype bool
#define GB_DEF_GrB_LOR_ytype bool

#define GB_DEF_GrB_LAND_function GB_LAND_f_BOOL
#define GB_DEF_GrB_LAND_ztype bool
#define GB_DEF_GrB_LAND_xtype bool
#define GB_DEF_GrB_LAND_ytype bool

#define GB_DEF_GrB_LXOR_function GB_LXOR_f_BOOL
#define GB_DEF_GrB_LXOR_ztype bool
#define GB_DEF_GrB_LXOR_xtype bool
#define GB_DEF_GrB_LXOR_ytype bool


//------------------------------------------------------
// built-in monoids
//------------------------------------------------------

// op: MIN
#define GB_DEF_GxB_MIN_BOOL_MONOID_add GB_MIN_f_BOOL
#define GB_DEF_GxB_MIN_INT8_MONOID_add GB_MIN_f_INT8
#define GB_DEF_GxB_MIN_UINT8_MONOID_add GB_MIN_f_UINT8
#define GB_DEF_GxB_MIN_INT16_MONOID_add GB_MIN_f_INT16
#define GB_DEF_GxB_MIN_UINT16_MONOID_add GB_MIN_f_UINT16
#define GB_DEF_GxB_MIN_INT32_MONOID_add GB_MIN_f_INT32
#define GB_DEF_GxB_MIN_UINT32_MONOID_add GB_MIN_f_UINT32
#define GB_DEF_GxB_MIN_INT64_MONOID_add GB_MIN_f_INT64
#define GB_DEF_GxB_MIN_UINT64_MONOID_add GB_MIN_f_UINT64
#define GB_DEF_GxB_MIN_FP32_MONOID_add GB_MIN_f_FP32
#define GB_DEF_GxB_MIN_FP64_MONOID_add GB_MIN_f_FP64
// op: MAX
#define GB_DEF_GxB_MAX_BOOL_MONOID_add GB_MAX_f_BOOL
#define GB_DEF_GxB_MAX_INT8_MONOID_add GB_MAX_f_INT8
#define GB_DEF_GxB_MAX_UINT8_MONOID_add GB_MAX_f_UINT8
#define GB_DEF_GxB_MAX_INT16_MONOID_add GB_MAX_f_INT16
#define GB_DEF_GxB_MAX_UINT16_MONOID_add GB_MAX_f_UINT16
#define GB_DEF_GxB_MAX_INT32_MONOID_add GB_MAX_f_INT32
#define GB_DEF_GxB_MAX_UINT32_MONOID_add GB_MAX_f_UINT32
#define GB_DEF_GxB_MAX_INT64_MONOID_add GB_MAX_f_INT64
#define GB_DEF_GxB_MAX_UINT64_MONOID_add GB_MAX_f_UINT64
#define GB_DEF_GxB_MAX_FP32_MONOID_add GB_MAX_f_FP32
#define GB_DEF_GxB_MAX_FP64_MONOID_add GB_MAX_f_FP64
// op: PLUS
#define GB_DEF_GxB_PLUS_BOOL_MONOID_add GB_PLUS_f_BOOL
#define GB_DEF_GxB_PLUS_INT8_MONOID_add GB_PLUS_f_INT8
#define GB_DEF_GxB_PLUS_UINT8_MONOID_add GB_PLUS_f_UINT8
#define GB_DEF_GxB_PLUS_INT16_MONOID_add GB_PLUS_f_INT16
#define GB_DEF_GxB_PLUS_UINT16_MONOID_add GB_PLUS_f_UINT16
#define GB_DEF_GxB_PLUS_INT32_MONOID_add GB_PLUS_f_INT32
#define GB_DEF_GxB_PLUS_UINT32_MONOID_add GB_PLUS_f_UINT32
#define GB_DEF_GxB_PLUS_INT64_MONOID_add GB_PLUS_f_INT64
#define GB_DEF_GxB_PLUS_UINT64_MONOID_add GB_PLUS_f_UINT64
#define GB_DEF_GxB_PLUS_FP32_MONOID_add GB_PLUS_f_FP32
#define GB_DEF_GxB_PLUS_FP64_MONOID_add GB_PLUS_f_FP64
// op: TIMES
#define GB_DEF_GxB_TIMES_BOOL_MONOID_add GB_TIMES_f_BOOL
#define GB_DEF_GxB_TIMES_INT8_MONOID_add GB_TIMES_f_INT8
#define GB_DEF_GxB_TIMES_UINT8_MONOID_add GB_TIMES_f_UINT8
#define GB_DEF_GxB_TIMES_INT16_MONOID_add GB_TIMES_f_INT16
#define GB_DEF_GxB_TIMES_UINT16_MONOID_add GB_TIMES_f_UINT16
#define GB_DEF_GxB_TIMES_INT32_MONOID_add GB_TIMES_f_INT32
#define GB_DEF_GxB_TIMES_UINT32_MONOID_add GB_TIMES_f_UINT32
#define GB_DEF_GxB_TIMES_INT64_MONOID_add GB_TIMES_f_INT64
#define GB_DEF_GxB_TIMES_UINT64_MONOID_add GB_TIMES_f_UINT64
#define GB_DEF_GxB_TIMES_FP32_MONOID_add GB_TIMES_f_FP32
#define GB_DEF_GxB_TIMES_FP64_MONOID_add GB_TIMES_f_FP64

// op: Boolean
#define GB_DEF_GxB_LOR_BOOL_MONOID_add   GB_LOR_f_BOOL
#define GB_DEF_GxB_LAND_BOOL_MONOID_add  GB_LAND_f_BOOL
#define GB_DEF_GxB_LXOR_BOOL_MONOID_add  GB_LXOR_f_BOOL
#define GB_DEF_GxB_EQ_BOOL_MONOID_add    GB_EQ_f_BOOL

// monoid identity values
#define GB_DEF_GxB_MIN_INT8_MONOID_identity   INT8_MAX
#define GB_DEF_GxB_MIN_UINT8_MONOID_identity  UINT8_MAX
#define GB_DEF_GxB_MIN_INT16_MONOID_identity  INT16_MAX
#define GB_DEF_GxB_MIN_UINT16_MONOID_identity UINT16_MAX
#define GB_DEF_GxB_MIN_INT32_MONOID_identity  INT32_MAX
#define GB_DEF_GxB_MIN_UINT32_MONOID_identity UINT32_MAX
#define GB_DEF_GxB_MIN_INT64_MONOID_identity  INT64_MAX
#define GB_DEF_GxB_MIN_UINT64_MONOID_identity UINT64_MAX
#define GB_DEF_GxB_MIN_FP32_MONOID_identity   INFINITY
#define GB_DEF_GxB_MIN_FP64_MONOID_identity   INFINITY

#define GB_DEF_GxB_MAX_INT8_MONOID_identity   INT8_MIN
#define GB_DEF_GxB_MAX_UINT8_MONOID_identity  0
#define GB_DEF_GxB_MAX_INT16_MONOID_identity  INT16_MIN
#define GB_DEF_GxB_MAX_UINT16_MONOID_identity 0
#define GB_DEF_GxB_MAX_INT32_MONOID_identity  INT32_MIN
#define GB_DEF_GxB_MAX_UINT32_MONOID_identity 0
#define GB_DEF_GxB_MAX_INT64_MONOID_identity  INT64_MIN
#define GB_DEF_GxB_MAX_UINT64_MONOID_identity 0
#define GB_DEF_GxB_MAX_FP32_MONOID_identity   (-INFINITY)
#define GB_DEF_GxB_MAX_FP64_MONOID_identity   (-INFINITY)

#define GB_DEF_GxB_PLUS_INT8_MONOID_identity   0
#define GB_DEF_GxB_PLUS_UINT8_MONOID_identity  0
#define GB_DEF_GxB_PLUS_INT16_MONOID_identity  0
#define GB_DEF_GxB_PLUS_UINT16_MONOID_identity 0
#define GB_DEF_GxB_PLUS_INT32_MONOID_identity  0
#define GB_DEF_GxB_PLUS_UINT32_MONOID_identity 0
#define GB_DEF_GxB_PLUS_INT64_MONOID_identity  0
#define GB_DEF_GxB_PLUS_UINT64_MONOID_identity 0
#define GB_DEF_GxB_PLUS_FP32_MONOID_identity   0
#define GB_DEF_GxB_PLUS_FP64_MONOID_identity   0

#define GB_DEF_GxB_TIMES_INT8_MONOID_identity   1
#define GB_DEF_GxB_TIMES_UINT8_MONOID_identity  1
#define GB_DEF_GxB_TIMES_INT16_MONOID_identity  1
#define GB_DEF_GxB_TIMES_UINT16_MONOID_identity 1
#define GB_DEF_GxB_TIMES_INT32_MONOID_identity  1
#define GB_DEF_GxB_TIMES_UINT32_MONOID_identity 1
#define GB_DEF_GxB_TIMES_INT64_MONOID_identity  1
#define GB_DEF_GxB_TIMES_UINT64_MONOID_identity 1
#define GB_DEF_GxB_TIMES_FP32_MONOID_identity   1
#define GB_DEF_GxB_TIMES_FP64_MONOID_identity   1

#define GB_DEF_GxB_LOR_BOOL_MONOID_identity    false
#define GB_DEF_GxB_LAND_BOOL_MONOID_identity   true
#define GB_DEF_GxB_LXOR_BOOL_MONOID_identity   false
#define GB_DEF_GxB_EQ_BOOL_MONOID_identity     true

// monoid terminal values
#define GB_DEF_GxB_MIN_INT8_MONOID_terminal   INT8_MIN
#define GB_DEF_GxB_MIN_UINT8_MONOID_terminal  0
#define GB_DEF_GxB_MIN_INT16_MONOID_terminal  INT16_MIN
#define GB_DEF_GxB_MIN_UINT16_MONOID_terminal 0
#define GB_DEF_GxB_MIN_INT32_MONOID_terminal  INT32_MIN
#define GB_DEF_GxB_MIN_UINT32_MONOID_terminal 0
#define GB_DEF_GxB_MIN_INT64_MONOID_terminal  INT64_MIN
#define GB_DEF_GxB_MIN_UINT64_MONOID_terminal 0
#define GB_DEF_GxB_MIN_FP32_MONOID_terminal   (-INFINITY)
#define GB_DEF_GxB_MIN_FP64_MONOID_terminal   (-INFINITY)

#define GB_DEF_GxB_MAX_INT8_MONOID_terminal   INT8_MAX
#define GB_DEF_GxB_MAX_UINT8_MONOID_terminal  UINT8_MAX
#define GB_DEF_GxB_MAX_INT16_MONOID_terminal  INT16_MAX
#define GB_DEF_GxB_MAX_UINT16_MONOID_terminal UINT16_MAX
#define GB_DEF_GxB_MAX_INT32_MONOID_terminal  INT32_MAX
#define GB_DEF_GxB_MAX_UINT32_MONOID_terminal UINT32_MAX
#define GB_DEF_GxB_MAX_INT64_MONOID_terminal  INT64_MAX
#define GB_DEF_GxB_MAX_UINT64_MONOID_terminal UINT64_MAX
#define GB_DEF_GxB_MAX_FP32_MONOID_terminal   INFINITY
#define GB_DEF_GxB_MAX_FP64_MONOID_terminal   INFINITY

// no #define GB_DEF_GxB_PLUS_INT8_MONOID_terminal
// no #define GB_DEF_GxB_PLUS_UINT8_MONOID_terminal
// no #define GB_DEF_GxB_PLUS_INT16_MONOID_terminal
// no #define GB_DEF_GxB_PLUS_UINT16_MONOID_terminal
// no #define GB_DEF_GxB_PLUS_INT32_MONOID_terminal
// no #define GB_DEF_GxB_PLUS_UINT32_MONOID_terminal
// no #define GB_DEF_GxB_PLUS_INT64_MONOID_terminal
// no #define GB_DEF_GxB_PLUS_UINT64_MONOID_terminal
// no #define GB_DEF_GxB_PLUS_FP32_MONOID_terminal
// no #define GB_DEF_GxB_PLUS_FP64_MONOID_terminal

#define GB_DEF_GxB_TIMES_INT8_MONOID_terminal   0
#define GB_DEF_GxB_TIMES_UINT8_MONOID_terminal  0
#define GB_DEF_GxB_TIMES_INT16_MONOID_terminal  0
#define GB_DEF_GxB_TIMES_UINT16_MONOID_terminal 0
#define GB_DEF_GxB_TIMES_INT32_MONOID_terminal  0
#define GB_DEF_GxB_TIMES_UINT32_MONOID_terminal 0
#define GB_DEF_GxB_TIMES_INT64_MONOID_terminal  0
#define GB_DEF_GxB_TIMES_UINT64_MONOID_terminal 0
// no #define GB_DEF_GxB_TIMES_FP32_MONOID_terminal
// no #define GB_DEF_GxB_TIMES_FP64_MONOID_terminal

#define GB_DEF_GxB_LOR_BOOL_MONOID_terminal    true
#define GB_DEF_GxB_LAND_BOOL_MONOID_terminal   false
// no #define GB_DEF_GxB_LXOR_BOOL_MONOID_terminal
// no #define GB_DEF_GxB_EQ_BOOL_MONOID_terminal

//------------------------------------------------------------------------------

GrB_Info GB_AxB_user
(
    const GrB_Desc_Value AxB_method,
    const GrB_Semiring s,

    GrB_Matrix *Chandle,
    const GrB_Matrix M,
    const GrB_Matrix A,
    const GrB_Matrix B,
    bool flipxy,

    // for dot method only:
    const bool GB_mask_comp,

    // for heap method only:
    int64_t *restrict List,
    GB_pointer_pair *restrict pA_pair,
    GB_Element *restrict Heap,
    const int64_t bjnz_max,

    // for Gustavson's method only:
    GB_Sauna Sauna
) ;

//------------------------------------------------------------------------------
// Sauna methods: the sparse accumulator
//------------------------------------------------------------------------------

void GB_Sauna_free                  // free a Sauna
(
    int Sauna_id                    // id of Sauna to free
) ;

GrB_Info GB_Sauna_alloc             // create a Sauna
(
    int Sauna_id,                   // id of Sauna to create
    int64_t Sauna_n,                // size of the Sauna
    size_t Sauna_size               // size of each entry in the Sauna
) ;

GrB_Info GB_Sauna_acquire
(
    int nthreads,           // number of internal threads that need a Sauna
    int *Sauna_ids,         // size nthreads, the Sauna id's acquired
    GrB_Desc_Value *AxB_methods_used,       // size nthreads
    GB_Context Context
) ;

GrB_Info GB_Sauna_release
(
    int nthreads,           // number of internal threads that have a Sauna
    int *Sauna_ids          // size nthreads, the Sauna id's to release
) ;

#ifndef NDEBUG
    #define ASSERT_SAUNA_IS_RESET                                           \
    {                                                                       \
        for (int64_t i = 0 ; i < Sauna->Sauna_n ; i++)                      \
        {                                                                   \
            ASSERT (Sauna->Sauna_Mark [i] < Sauna->Sauna_hiwater) ;         \
        }                                                                   \
    }
#else
    #define ASSERT_SAUNA_IS_RESET
#endif

// GB_Sauna_reset: increment the Sauna_hiwater and clear Sauna_Mark if needed
static inline int64_t GB_Sauna_reset
(
    GB_Sauna Sauna,     // Sauna to reset
    int64_t reset,      // does Sauna_hiwater += reset
    int64_t range       // clear Mark if Sauna_hiwater+reset+range overflows
)
{ 

    ASSERT (Sauna != NULL) ;
    Sauna->Sauna_hiwater += reset ;     // increment the Sauna_hiwater

    if (Sauna->Sauna_hiwater + range <= 0 || reset == 0)
    { 
        // integer overflow has occurred; clear all of the Sauna_Mark array
        for (int64_t i = 0 ; i < Sauna->Sauna_n ; i++)
        { 
            Sauna->Sauna_Mark [i] = 0 ;
        }
        Sauna->Sauna_hiwater = 1 ;
    }

    // assertion for debugging only:
    ASSERT_SAUNA_IS_RESET ;         // assert that Sauna_Mark [...] < hiwater

    return (Sauna->Sauna_hiwater) ;
}

//------------------------------------------------------------------------------
// import/export
//------------------------------------------------------------------------------

#define GB_IMPORT_CHECK                                         \
    GB_RETURN_IF_NULL (A) ;                                     \
    (*A) = NULL ;                                               \
    GB_RETURN_IF_NULL_OR_FAULTY (type) ;                        \
    if (nrows > GB_INDEX_MAX)                                   \
    {                                                           \
        return (GB_ERROR (GrB_INVALID_VALUE, (GB_LOG,           \
            "problem too large: nrows "GBu" exceeds "GBu,       \
            nrows, GB_INDEX_MAX))) ;                            \
    }                                                           \
    if (ncols > GB_INDEX_MAX)                                   \
    {                                                           \
        return (GB_ERROR (GrB_INVALID_VALUE, (GB_LOG,           \
            "problem too large: ncols "GBu" exceeds "GBu,       \
            ncols, GB_INDEX_MAX))) ;                            \
    }                                                           \
    if (nvals > GB_INDEX_MAX)                                   \
    {                                                           \
        return (GB_ERROR (GrB_INVALID_VALUE, (GB_LOG,           \
            "problem too large: nvals "GBu" exceeds "GBu,       \
            nvals, GB_INDEX_MAX))) ;                            \
    }                                                           \
    /* get the descriptor */                                    \
    GB_GET_DESCRIPTOR (info, desc, xx1, xx2, xx3, xx4, xx5) ;

#define GB_EXPORT_CHECK                                         \
    GB_RETURN_IF_NULL (A) ;                                     \
    GB_RETURN_IF_NULL_OR_FAULTY (*A) ;                          \
    ASSERT_OK (GB_check (*A, "A to export", GB0)) ;             \
    /* finish any pending work */                               \
    GB_WAIT (*A) ;                                              \
    /* check these after forcing completion */                  \
    GB_RETURN_IF_NULL (type) ;                                  \
    GB_RETURN_IF_NULL (nrows) ;                                 \
    GB_RETURN_IF_NULL (ncols) ;                                 \
    GB_RETURN_IF_NULL (nvals) ;                                 \
    GB_RETURN_IF_NULL (nonempty) ;                              \
    /* get the descriptor */                                    \
    GB_GET_DESCRIPTOR (info, desc, xx1, xx2, xx3, xx4, xx5) ;   \
    /* export basic attributes */                               \
    (*type) = (*A)->type ;                                      \
    (*nrows) = GB_NROWS (*A) ;                                  \
    (*ncols) = GB_NCOLS (*A) ;                                  \
    (*nvals) = GB_NNZ (*A) ;

#endif

