//------------------------------------------------------------------------------
// GB_opaque.h: definitions of opaque objects
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#ifndef GB_OPAQUE_H
#define GB_OPAQUE_H

#define GB_OPAQUE(x) GB (GB_EVAL2 (_opaque__, x))

//------------------------------------------------------------------------------
// GB_void: like void, but valid for pointer arithmetic
//------------------------------------------------------------------------------

typedef unsigned char GB_void ;

//------------------------------------------------------------------------------
// type codes for GrB_Type
//------------------------------------------------------------------------------

typedef enum
{
    // the 14 scalar types: 13 built-in types, and one user-defined type code
    GB_ignore_code  = 0,
    GB_BOOL_code    = 1,        // 'logical' in @GrB interface
    GB_INT8_code    = 2,
    GB_UINT8_code   = 3,
    GB_INT16_code   = 4,
    GB_UINT16_code  = 5,
    GB_INT32_code   = 6,
    GB_UINT32_code  = 7,
    GB_INT64_code   = 8,
    GB_UINT64_code  = 9,
    GB_FP32_code    = 10,       // float ('single' in @GrB interface)
    GB_FP64_code    = 11,       // double
    GB_FC32_code    = 12,       // float complex ('single complex' in @GrB)
    GB_FC64_code    = 13,       // double complex
    GB_UDT_code     = 14        // void *, user-defined type
}
GB_Type_code ;                  // enumerated type code

//------------------------------------------------------------------------------
// opcodes for all operators
//------------------------------------------------------------------------------

typedef enum
{

    GB_NOP_code = 0,    // no operation

    //==========================================================================
    // unary operators
    //==========================================================================

    //--------------------------------------------------------------------------
    // primary unary operators x=f(x)
    //--------------------------------------------------------------------------

    GB_ONE_unop_code       = 1,    // z = 1
    GB_IDENTITY_unop_code  = 2,    // z = x
    GB_AINV_unop_code      = 3,    // z = -x
    GB_ABS_unop_code       = 4,    // z = abs(x) ; z is real if x is complex
    GB_MINV_unop_code      = 5,    // z = 1/x ; special cases for bool and ints
    GB_LNOT_unop_code      = 6,    // z = !x
    GB_BNOT_unop_code      = 7,    // z = ~x (bitwise complement)

    //--------------------------------------------------------------------------
    // unary operators for floating-point types (real and complex)
    //--------------------------------------------------------------------------

    GB_SQRT_unop_code      = 8,    // z = sqrt (x)
    GB_LOG_unop_code       = 9,    // z = log (x)
    GB_EXP_unop_code       = 10,   // z = exp (x)
    GB_SIN_unop_code       = 11,   // z = sin (x)
    GB_COS_unop_code       = 12,   // z = cos (x)
    GB_TAN_unop_code       = 13,   // z = tan (x)
    GB_ASIN_unop_code      = 14,   // z = asin (x)
    GB_ACOS_unop_code      = 15,   // z = acos (x)
    GB_ATAN_unop_code      = 16,   // z = atan (x)
    GB_SINH_unop_code      = 17,   // z = sinh (x)
    GB_COSH_unop_code      = 18,   // z = cosh (x)
    GB_TANH_unop_code      = 19,   // z = tanh (x)
    GB_ASINH_unop_code     = 20,   // z = asinh (x)
    GB_ACOSH_unop_code     = 21,   // z = acosh (x)
    GB_ATANH_unop_code     = 22,   // z = atanh (x)
    GB_SIGNUM_unop_code    = 23,   // z = signum (x)
    GB_CEIL_unop_code      = 24,   // z = ceil (x)
    GB_FLOOR_unop_code     = 25,   // z = floor (x)
    GB_ROUND_unop_code     = 26,   // z = round (x)
    GB_TRUNC_unop_code     = 27,   // z = trunc (x)
    GB_EXP2_unop_code      = 28,   // z = exp2 (x)
    GB_EXPM1_unop_code     = 29,   // z = expm1 (x)
    GB_LOG10_unop_code     = 30,   // z = log10 (x)
    GB_LOG1P_unop_code     = 31,   // z = log1P (x)
    GB_LOG2_unop_code      = 32,   // z = log2 (x)

    //--------------------------------------------------------------------------
    // unary operators for real floating-point types
    //--------------------------------------------------------------------------

    GB_LGAMMA_unop_code    = 33,   // z = lgamma (x)
    GB_TGAMMA_unop_code    = 34,   // z = tgamma (x)
    GB_ERF_unop_code       = 35,   // z = erf (x)
    GB_ERFC_unop_code      = 36,   // z = erfc (x)
    GB_CBRT_unop_code      = 37,   // z = cbrt (x)
    GB_FREXPX_unop_code    = 38,   // z = frexpx (x), mantissa of ANSI C11 frexp
    GB_FREXPE_unop_code    = 39,   // z = frexpe (x), exponent of ANSI C11 frexp

    //--------------------------------------------------------------------------
    // unary operators for complex types only
    //--------------------------------------------------------------------------

    GB_CONJ_unop_code      = 40,   // z = conj (x)

    //--------------------------------------------------------------------------
    // unary operators where z is real and x is complex
    //--------------------------------------------------------------------------

    GB_CREAL_unop_code     = 41,   // z = creal (x)
    GB_CIMAG_unop_code     = 42,   // z = cimag (x)
    GB_CARG_unop_code      = 43,   // z = carg (x)

    //--------------------------------------------------------------------------
    // unary operators where z is bool and x is any floating-point type
    //--------------------------------------------------------------------------

    GB_ISINF_unop_code     = 44,   // z = isinf (x)
    GB_ISNAN_unop_code     = 45,   // z = isnan (x)
    GB_ISFINITE_unop_code  = 46,   // z = isfinite (x)

    //--------------------------------------------------------------------------
    // positional unary operators: z is int32 or int64, x is ignored
    //--------------------------------------------------------------------------

    GB_POSITIONI_unop_code     = 47,   // z = position_i(A(i,j)) == i
    GB_POSITIONI1_unop_code    = 48,   // z = position_i1(A(i,j)) == i+1
    GB_POSITIONJ_unop_code     = 49,   // z = position_j(A(i,j)) == j
    GB_POSITIONJ1_unop_code    = 50,   // z = position_j1(A(i,j)) == j+1

    GB_USER_unop_code = 51,

    // true if opcode is for a GrB_UnaryOp
    #define GB_IS_UNARYOP_CODE(opcode) \
        ((opcode) >= GB_ONE_unop_code && \
         (opcode) <= GB_USER_unop_code)

    // true if opcode is for a GrB_UnaryOp positional operator
    #define GB_IS_UNARYOP_CODE_POSITIONAL(opcode) \
        ((opcode) >= GB_POSITIONI_unop_code && \
         (opcode) <= GB_POSITIONJ1_unop_code)

    //==========================================================================
    // index_unary operators
    //==========================================================================

    // operator codes used in GrB_IndexUnaryOp structures

    // Result is INT32 or INT64, depending on i and/or j, and thunk:
    GB_ROWINDEX_idxunop_code  = 52,   // (i+thunk): row index - thunk
    GB_COLINDEX_idxunop_code  = 53,   // (j+thunk): col index - thunk
    GB_DIAGINDEX_idxunop_code = 54,   // (j-(i+thunk)): diag index + thunk
    GB_FLIPDIAGINDEX_idxunop_code = 55,   // (i-(j+thunk)), internal use only

    // Result is BOOL, depending on i and/or j, and thunk:
    GB_TRIL_idxunop_code      = 56,   // (j <= (i+thunk)): tril (A,thunk)
    GB_TRIU_idxunop_code      = 57,   // (j >= (i+thunk)): triu (A,thunk)
    GB_DIAG_idxunop_code      = 58,   // (j == (i+thunk)): diag(A,thunk)
    GB_OFFDIAG_idxunop_code   = 59,   // (j != (i+thunk)): offdiag(A,thunk)
    GB_COLLE_idxunop_code     = 60,   // (j <= thunk): A (:,0:thunk)
    GB_COLGT_idxunop_code     = 61,   // (j > thunk): A (:,thunk+1:ncols-1)
    GB_ROWLE_idxunop_code     = 62,   // (i <= thunk): A (0:thunk,:)
    GB_ROWGT_idxunop_code     = 63,   // (i > thunk): A (thunk+1:nrows-1,:)

    // Result is BOOL, depending on the value aij and thunk:
    GB_VALUENE_idxunop_code   = 64,   // (aij != thunk)
    GB_VALUEEQ_idxunop_code   = 65,   // (aij == thunk)
    GB_VALUEGT_idxunop_code   = 66,   // (aij > thunk)
    GB_VALUEGE_idxunop_code   = 67,   // (aij >= thunk)
    GB_VALUELT_idxunop_code   = 68,   // (aij < thunk)
    GB_VALUELE_idxunop_code   = 69,   // (aij <= thunk)

    GB_USER_idxunop_code = 70,

    // true if opcode is for a GrB_IndexUnaryOp
    #define GB_IS_INDEXUNARYOP_CODE(opcode) \
        ((opcode) >= GB_ROWINDEX_idxunop_code && \
         (opcode) <= GB_USER_idxunop_code)

    // true if opcode is for a GrB_IndexUnaryOp positional operator
    #define GB_IS_INDEXUNARYOP_CODE_POSITIONAL(opcode) \
        ((opcode) >= GB_ROWINDEX_idxunop_code && \
         (opcode) <= GB_ROWGT_idxunop_code)

    //==========================================================================
    // binary operators
    //==========================================================================

    //--------------------------------------------------------------------------
    // binary operators z=f(x,y) that return the same type as their inputs
    //--------------------------------------------------------------------------

    GB_FIRST_binop_code     = 71,   // z = x
    GB_SECOND_binop_code    = 72,   // z = y
    GB_ANY_binop_code       = 73,   // z = x or y, selected arbitrarily
    GB_PAIR_binop_code      = 74,   // z = 1
    GB_MIN_binop_code       = 75,   // z = min(x,y)
    GB_MAX_binop_code       = 76,   // z = max(x,y)
    GB_PLUS_binop_code      = 77,   // z = x + y
    GB_MINUS_binop_code     = 78,   // z = x - y
    GB_RMINUS_binop_code    = 79,   // z = y - x
    GB_TIMES_binop_code     = 80,   // z = x * y
    GB_DIV_binop_code       = 81,   // z = x / y
    GB_RDIV_binop_code      = 82,   // z = y / x
    GB_POW_binop_code       = 83,   // z = pow (x,y)

    GB_ISEQ_binop_code      = 84,   // z = (x == y)
    GB_ISNE_binop_code      = 85,   // z = (x != y)
    GB_ISGT_binop_code      = 86,   // z = (x >  y)
    GB_ISLT_binop_code      = 87,   // z = (x <  y)
    GB_ISGE_binop_code      = 88,   // z = (x >= y)
    GB_ISLE_binop_code      = 89,   // z = (x <= y)

    GB_LOR_binop_code       = 90,   // z = (x != 0) || (y != 0)
    GB_LAND_binop_code      = 91,   // z = (x != 0) && (y != 0)
    GB_LXOR_binop_code      = 92,   // z = (x != 0) != (y != 0)

    GB_BOR_binop_code       = 93,   // z = (x | y), bitwise or
    GB_BAND_binop_code      = 94,   // z = (x & y), bitwise and
    GB_BXOR_binop_code      = 95,   // z = (x ^ y), bitwise xor
    GB_BXNOR_binop_code     = 96,   // z = ~(x ^ y), bitwise xnor
    GB_BGET_binop_code      = 97,   // z = bitget (x,y)
    GB_BSET_binop_code      = 98,   // z = bitset (x,y)
    GB_BCLR_binop_code      = 99,   // z = bitclr (x,y)
    GB_BSHIFT_binop_code    =100,   // z = bitshift (x,y)

    //--------------------------------------------------------------------------
    // binary operators z=f(x,y) that return bool (TxT -> bool)
    //--------------------------------------------------------------------------

    GB_EQ_binop_code        = 101,  // z = (x == y), is LXNOR for bool
    GB_NE_binop_code        = 102,  // z = (x != y)
    GB_GT_binop_code        = 103,  // z = (x >  y)
    GB_LT_binop_code        = 104,  // z = (x <  y)
    GB_GE_binop_code        = 105,  // z = (x >= y)
    GB_LE_binop_code        = 106,  // z = (x <= y)

    //--------------------------------------------------------------------------
    // binary operators for real floating-point types (TxT -> T)
    //--------------------------------------------------------------------------

    GB_ATAN2_binop_code     = 107,  // z = atan2 (x,y)
    GB_HYPOT_binop_code     = 108,  // z = hypot (x,y)
    GB_FMOD_binop_code      = 109,  // z = fmod (x,y)
    GB_REMAINDER_binop_code = 110,  // z = remainder (x,y)
    GB_COPYSIGN_binop_code  = 111,  // z = copysign (x,y)
    GB_LDEXP_binop_code     = 112,  // z = ldexp (x,y)

    //--------------------------------------------------------------------------
    // binary operator z=f(x,y) where z is complex, x,y real:
    //--------------------------------------------------------------------------

    GB_CMPLX_binop_code     = 113,  // z = cmplx (x,y)

    //--------------------------------------------------------------------------
    // positional binary operators: z is int64, x and y are ignored
    //--------------------------------------------------------------------------

    GB_FIRSTI_binop_code    = 114,  // z = first_i(A(i,j),y) == i
    GB_FIRSTI1_binop_code   = 115,  // z = first_i1(A(i,j),y) == i+1
    GB_FIRSTJ_binop_code    = 116,  // z = first_j(A(i,j),y) == j
    GB_FIRSTJ1_binop_code   = 117,  // z = first_j1(A(i,j),y) == j+1
    GB_SECONDI_binop_code   = 118,  // z = second_i(x,B(i,j)) == i
    GB_SECONDI1_binop_code  = 119,  // z = second_i1(x,B(i,j)) == i+1
    GB_SECONDJ_binop_code   = 120,  // z = second_j(x,B(i,j)) == j
    GB_SECONDJ1_binop_code  = 121,  // z = second_j1(x,B(i,j)) == j+1

    GB_USER_binop_code = 122,

    // true if opcode is for a GrB_BinaryOp
    #define GB_IS_BINARYOP_CODE(opcode) \
        ((opcode) >= GB_FIRST_binop_code && (opcode) <= GB_USER_binop_code)

    // true if opcode is for a GrB_BinaryOp positional operator
    #define GB_IS_BINARYOP_CODE_POSITIONAL(opcode) \
        ((opcode) >= GB_FIRSTI_binop_code && \
         (opcode) <= GB_SECONDJ1_binop_code)

    //==========================================================================
    // built-in Select operators
    //==========================================================================

    // built-in positional select operators: thunk optional; defaults to zero
    GB_TRIL_selop_code      = 123,
    GB_TRIU_selop_code      = 124,
    GB_DIAG_selop_code      = 125,
    GB_OFFDIAG_selop_code   = 126,

    // built-in select operators, no thunk used
    GB_NONZOMBIE_selop_code = 127,
    GB_NONZERO_selop_code   = 128,
    GB_EQ_ZERO_selop_code   = 129,
    GB_GT_ZERO_selop_code   = 130,
    GB_GE_ZERO_selop_code   = 131,
    GB_LT_ZERO_selop_code   = 132,
    GB_LE_ZERO_selop_code   = 133,

    // built-in select operators, thunk optional; defaults to zero
    GB_NE_THUNK_selop_code  = 134,
    GB_EQ_THUNK_selop_code  = 135,
    GB_GT_THUNK_selop_code  = 136,
    GB_GE_THUNK_selop_code  = 137,
    GB_LT_THUNK_selop_code  = 138,
    GB_LE_THUNK_selop_code  = 139,

    GB_USER_selop_code = 140

    // true if opcode is for a GxB_SelectOp
    #define GB_IS_SELECTOP_CODE(opcode) \
        ((opcode) >= GB_TRIL_selop_code && (opcode) <= GB_USER_selop_code)

    // true if opcode is for a GxB_SelectOp positional operator
    #define GB_IS_SELECTOP_CODE_POSITIONAL(opcode) \
        ((opcode) >= GB_TRIL_selop_code && \
         (opcode) <= GB_OFFDIAG_selop_code)

}
GB_Opcode ;

// true if the opcode is a positional operator of any kind
#define GB_OPCODE_IS_POSITIONAL(opcode)             \
    (GB_IS_UNARYOP_CODE_POSITIONAL (opcode) ||      \
     GB_IS_INDEXUNARYOP_CODE_POSITIONAL (opcode) || \
     GB_IS_BINARYOP_CODE_POSITIONAL (opcode) ||     \
     GB_IS_SELECTOP_CODE_POSITIONAL (opcode))

// true if the op is a unary or binary positional operator
#define GB_OP_IS_POSITIONAL(op) \
    (((op) == NULL) ? false : GB_OPCODE_IS_POSITIONAL ((op)->opcode))

GrB_UnaryOp GB_positional_unop_ijflip   // return flipped operator
(
    GrB_UnaryOp op                      // operator to flip
) ;

GrB_BinaryOp GB_positional_binop_ijflip // return flipped operator
(
    GrB_BinaryOp op                     // operator to flip
) ;

GrB_IndexUnaryOp GB_positional_idxunop_ijflip   // return flipped operator
(
    int64_t *ithunk,            // input/output: revised value of thunk
    GrB_IndexUnaryOp op         // operator to flip
) ;

int64_t GB_positional_offset        // return the positional thunk
(
    GB_Opcode opcode,               // opcode of positional operator
    GrB_Scalar Thunk                // thunk for idxunops, or NULL
) ;

// for internal use only
GB_PUBLIC GrB_IndexUnaryOp GxB_FLIPDIAGINDEX_INT32, GxB_FLIPDIAGINDEX_INT64 ;

//------------------------------------------------------------------------------
// opaque content of GraphBLAS objects
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

struct GB_Type_opaque       // content of GrB_Type
{
    int64_t magic ;         // for detecting uninitialized objects
    size_t header_size ;    // size of the malloc'd block for this struct, or 0
    size_t size ;           // size of the type
    GB_Type_code code ;     // the type code
    char name [GxB_MAX_NAME_LEN] ;       // name of the type
    char *defn ;            // type definition
    size_t defn_size ;      // size of the definition
} ;

struct GB_UnaryOp_opaque    // content of GrB_UnaryOp
{
    #include "GB_Operator.h"
} ;

struct GB_IndexUnaryOp_opaque   // content of GrB_IndexUnaryOp
{
    #include "GB_Operator.h"
} ;

struct GB_BinaryOp_opaque   // content of GrB_BinaryOp
{
    #include "GB_Operator.h"
} ;

struct GB_SelectOp_opaque   // content of GxB_SelectOp
{
    #include "GB_Operator.h"
} ;

struct GB_Operator_opaque   // content of GB_Operator
{
    #include "GB_Operator.h"
} ;

// Any GrB_UnaryOp, GrB_IndexUnaryOp, GrB_BinaryOp, or GxB_SelectOp can be
// typecasted to a generic GB_Operator object, which is only used internally.
typedef struct GB_Operator_opaque *GB_Operator ;

struct GB_Monoid_opaque     // content of GrB_Monoid
{
    int64_t magic ;         // for detecting uninitialized objects
    size_t header_size ;    // size of the malloc'd block for this struct, or 0
    GrB_BinaryOp op ;       // binary operator of the monoid
    void *identity ;        // identity of the monoid; type is op->ztype
    void *terminal ;        // early-exit (NULL if no value); type is op->ztype
    size_t identity_size ;  // size of the malloc'd block for identity, or 0
    size_t terminal_size ;  // size of the malloc'd block for terminal, or 0
} ;

struct GB_Semiring_opaque   // content of GrB_Semiring
{
    int64_t magic ;         // for detecting uninitialized objects
    size_t header_size ;    // size of the malloc'd block for this struct, or 0
    GrB_Monoid add ;        // add operator of the semiring
    GrB_BinaryOp multiply ; // multiply operator of the semiring
} ;

struct GB_Descriptor_opaque // content of GrB_Descriptor
{
    // first 4 items exactly match GrB_Matrix, GrB_Vector, GrB_Scalar structs:
    int64_t magic ;         // for detecting uninitialized objects
    size_t header_size ;    // size of the malloc'd block for this struct, or 0
    char *logger ;          // error logger string
    size_t logger_size ;    // size of the malloc'd block for logger, or 0
    // specific to the descriptor struct:
    double chunk ;          // chunk size for # of threads for small problems
    GrB_Desc_Value out ;    // output descriptor
    GrB_Desc_Value mask ;   // mask descriptor
    GrB_Desc_Value in0 ;    // first input descriptor (A for C=A*B, for example)
    GrB_Desc_Value in1 ;    // second input descriptor (B for C=A*B)
    GrB_Desc_Value axb ;    // for selecting the method for C=A*B
    int nthreads_max ;      // max # threads to use in this call to GraphBLAS
    int compression ;       // compression method for GxB_Matrix_serialize
    bool do_sort ;          // if nonzero, do the sort in GrB_mxm
    int import ;            // if zero (default), trust input data
} ;

//------------------------------------------------------------------------------
// GB_Pending data structure: for scalars, vectors, and matrices
//------------------------------------------------------------------------------

// Pending tuples are a list of unsorted (i,j,x) tuples that have not yet been
// added to a matrix.  The data structure is defined in GB_Pending.h.

struct GB_Pending_struct    // list of pending tuples for a matrix
{
    size_t header_size ;    // size of the malloc'd block for this struct, or 0
    int64_t n ;         // number of pending tuples to add to matrix
    int64_t nmax ;      // size of i,j,x
    bool sorted ;       // true if pending tuples are in sorted order
    int64_t *i ;        // row indices of pending tuples
    size_t i_size ;
    int64_t *j ;        // col indices of pending tuples; NULL if A->vdim <= 1
    size_t j_size ;
    GB_void *x ;        // values of pending tuples
    size_t x_size ;
    GrB_Type type ;     // the type of s
    size_t size ;       // type->size
    GrB_BinaryOp op ;   // operator to assemble pending tuples
} ;

typedef struct GB_Pending_struct *GB_Pending ;

//------------------------------------------------------------------------------
// scalar, vector, and matrix types
//------------------------------------------------------------------------------

struct GB_Scalar_opaque     // content of GrB_Scalar: 1-by-1 standard CSC matrix
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

static inline GrB_Matrix GB_clear_static_header // clear a static header
(
    GrB_Matrix C    // static header to clear
)
{
    ASSERT (C != NULL) ;
    memset (C, 0, sizeof (struct GB_Matrix_opaque)) ;
    C->static_header = true ;
    return (C) ;
}

//------------------------------------------------------------------------------
// Accessing the content of a scalar, vector, or matrix
//------------------------------------------------------------------------------

#define GBP(Ap,k,avlen) ((Ap == NULL) ? ((k) * (avlen)) : Ap [k])
#define GBH(Ah,k)       ((Ah == NULL) ? (k) : Ah [k])
#define GBI(Ai,p,avlen) ((Ai == NULL) ? ((p) % (avlen)) : Ai [p])
#define GBB(Ab,p)       ((Ab == NULL) ? 1 : Ab [p])
#define GBX(Ax,p,A_iso) (Ax [(A_iso) ? 0 : (p)])

#endif

