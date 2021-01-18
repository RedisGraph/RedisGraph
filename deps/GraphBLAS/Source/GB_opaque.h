//------------------------------------------------------------------------------
// GB_opaque.h: definitions of opaque objects
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#ifndef GB_OPAQUE_H
#define GB_OPAQUE_H

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
    GB_BOOL_code    = 1,        // 'logical' in MATLAB
    GB_INT8_code    = 2,
    GB_UINT8_code   = 3,
    GB_INT16_code   = 4,
    GB_UINT16_code  = 5,
    GB_INT32_code   = 6,
    GB_UINT32_code  = 7,
    GB_INT64_code   = 8,
    GB_UINT64_code  = 9,
    GB_FP32_code    = 10,       // float ('single' in MATLAB)
    GB_FP64_code    = 11,       // double
    GB_FC32_code    = 12,       // float complex ('single complex' in MATLAB)
    GB_FC64_code    = 13,       // double complex
    GB_UDT_code     = 14        // void *, user-defined type
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

    GB_NOP_opcode       = 0,    // no operation

    //==========================================================================
    // binary operators
    //==========================================================================

    //--------------------------------------------------------------------------
    // primary unary operators x=f(x)
    //--------------------------------------------------------------------------

    GB_ONE_opcode       = 1,    // z = 1
    GB_IDENTITY_opcode  = 2,    // z = x
    GB_AINV_opcode      = 3,    // z = -x
    GB_ABS_opcode       = 4,    // z = abs(x) ; except z is real if x is complex
    GB_MINV_opcode      = 5,    // z = 1/x ; special cases for bool and integers
    GB_LNOT_opcode      = 6,    // z = !x
    GB_BNOT_opcode      = 7,    // z = ~x (bitwise complement)

    //--------------------------------------------------------------------------
    // unary operators for floating-point types (real and complex)
    //--------------------------------------------------------------------------

    GB_SQRT_opcode      = 8,    // z = sqrt (x)
    GB_LOG_opcode       = 9,    // z = log (x)
    GB_EXP_opcode       = 10,   // z = exp (x)

    GB_SIN_opcode       = 11,   // z = sin (x)
    GB_COS_opcode       = 12,   // z = cos (x)
    GB_TAN_opcode       = 13,   // z = tan (x)

    GB_ASIN_opcode      = 14,   // z = asin (x)
    GB_ACOS_opcode      = 15,   // z = acos (x)
    GB_ATAN_opcode      = 16,   // z = atan (x)

    GB_SINH_opcode      = 17,   // z = sinh (x)
    GB_COSH_opcode      = 18,   // z = cosh (x)
    GB_TANH_opcode      = 19,   // z = tanh (x)

    GB_ASINH_opcode     = 20,   // z = asinh (x)
    GB_ACOSH_opcode     = 21,   // z = acosh (x)
    GB_ATANH_opcode     = 22,   // z = atanh (x)

    GB_SIGNUM_opcode    = 23,   // z = signum (x)
    GB_CEIL_opcode      = 24,   // z = ceil (x)
    GB_FLOOR_opcode     = 25,   // z = floor (x)
    GB_ROUND_opcode     = 26,   // z = round (x)
    GB_TRUNC_opcode     = 27,   // z = trunc (x)

    GB_EXP2_opcode      = 28,   // z = exp2 (x)
    GB_EXPM1_opcode     = 29,   // z = expm1 (x)
    GB_LOG10_opcode     = 30,   // z = log10 (x)
    GB_LOG1P_opcode     = 31,   // z = log1P (x)
    GB_LOG2_opcode      = 32,   // z = log2 (x)

    //--------------------------------------------------------------------------
    // unary operators for real floating-point types
    //--------------------------------------------------------------------------

    GB_LGAMMA_opcode    = 33,   // z = lgamma (x)
    GB_TGAMMA_opcode    = 34,   // z = tgamma (x)
    GB_ERF_opcode       = 35,   // z = erf (x)
    GB_ERFC_opcode      = 36,   // z = erfc (x)
    GB_FREXPX_opcode    = 37,   // z = frexpx (x), mantissa from ANSI C11 frexp
    GB_FREXPE_opcode    = 38,   // z = frexpe (x), exponent from ANSI C11 frexp

    //--------------------------------------------------------------------------
    // unary operators for complex types only
    //--------------------------------------------------------------------------

    GB_CONJ_opcode      = 39,   // z = conj (x)

    //--------------------------------------------------------------------------
    // unary operators where z is real and x is complex
    //--------------------------------------------------------------------------

    GB_CREAL_opcode     = 40,   // z = creal (x)
    GB_CIMAG_opcode     = 41,   // z = cimag (x)
    GB_CARG_opcode      = 42,   // z = carg (x)

    //--------------------------------------------------------------------------
    // unary operators where z is bool and x is any floating-point type
    //--------------------------------------------------------------------------

    GB_ISINF_opcode     = 43,   // z = isinf (x)
    GB_ISNAN_opcode     = 44,   // z = isnan (x)
    GB_ISFINITE_opcode  = 45,   // z = isfinite (x)

    //--------------------------------------------------------------------------
    // positional unary operators: z is int64, x is ignored
    //--------------------------------------------------------------------------

    GB_POSITIONI_opcode     = 46,   // z = position_i(A(i,j)) == i
    GB_POSITIONI1_opcode    = 47,   // z = position_i1(A(i,j)) == i+1
    GB_POSITIONJ_opcode     = 48,   // z = position_j(A(i,j)) == j
    GB_POSITIONJ1_opcode    = 49,   // z = position_j1(A(i,j)) == j+1

    //==========================================================================
    // binary operators
    //==========================================================================

    //--------------------------------------------------------------------------
    // binary operators z=f(x,y) that return the same type as their inputs
    //--------------------------------------------------------------------------

    GB_FIRST_opcode     = 50,   // z = x
    GB_SECOND_opcode    = 51,   // z = y
    GB_ANY_opcode       = 52,   // z = x or y, selected arbitrarily
    GB_PAIR_opcode      = 53,   // z = 1
    GB_MIN_opcode       = 54,   // z = min(x,y)
    GB_MAX_opcode       = 55,   // z = max(x,y)
    GB_PLUS_opcode      = 56,   // z = x + y
    GB_MINUS_opcode     = 57,   // z = x - y
    GB_RMINUS_opcode    = 58,   // z = y - x
    GB_TIMES_opcode     = 59,   // z = x * y
    GB_DIV_opcode       = 60,   // z = x / y ; special cases for bool and ints
    GB_RDIV_opcode      = 61,   // z = y / x ; special cases for bool and ints
    GB_POW_opcode       = 62,   // z = pow (x,y)

    GB_ISEQ_opcode      = 63,   // z = (x == y)
    GB_ISNE_opcode      = 64,   // z = (x != y)
    GB_ISGT_opcode      = 65,   // z = (x >  y)
    GB_ISLT_opcode      = 66,   // z = (x <  y)
    GB_ISGE_opcode      = 67,   // z = (x >= y)
    GB_ISLE_opcode      = 68,   // z = (x <= y)

    GB_LOR_opcode       = 69,   // z = (x != 0) || (y != 0)
    GB_LAND_opcode      = 70,   // z = (x != 0) && (y != 0)
    GB_LXOR_opcode      = 71,   // z = (x != 0) != (y != 0)

    GB_BOR_opcode       = 72,   // z = (x | y), bitwise or
    GB_BAND_opcode      = 73,   // z = (x & y), bitwise and
    GB_BXOR_opcode      = 74,   // z = (x ^ y), bitwise xor
    GB_BXNOR_opcode     = 75,   // z = ~(x ^ y), bitwise xnor
    GB_BGET_opcode      = 76,   // z = bitget (x,y)
    GB_BSET_opcode      = 77,   // z = bitset (x,y)
    GB_BCLR_opcode      = 78,   // z = bitclr (x,y)
    GB_BSHIFT_opcode    = 79,   // z = bitshift (x,y)

    //--------------------------------------------------------------------------
    // binary operators z=f(x,y) that return bool (TxT -> bool)
    //--------------------------------------------------------------------------

    GB_EQ_opcode        = 80,   // z = (x == y), same as LXNOR operator for bool
    GB_NE_opcode        = 81,   // z = (x != y)
    GB_GT_opcode        = 82,   // z = (x >  y)
    GB_LT_opcode        = 83,   // z = (x <  y)
    GB_GE_opcode        = 84,   // z = (x >= y)
    GB_LE_opcode        = 85,   // z = (x <= y)

    //--------------------------------------------------------------------------
    // binary operators for real floating-point types (TxT -> T)
    //--------------------------------------------------------------------------

    GB_ATAN2_opcode     = 86,   // z = atan2 (x,y)
    GB_HYPOT_opcode     = 87,   // z = hypot (x,y)
    GB_FMOD_opcode      = 88,   // z = fmod (x,y)
    GB_REMAINDER_opcode = 89,   // z = remainder (x,y)
    GB_COPYSIGN_opcode  = 90,   // z = copysign (x,y)
    GB_LDEXP_opcode     = 91,   // z = ldexp (x,y)

    //--------------------------------------------------------------------------
    // binary operator z=f(x,y) where z is complex, x,y real:
    //--------------------------------------------------------------------------

    GB_CMPLX_opcode     = 92,   // z = cmplx (x,y)

    //--------------------------------------------------------------------------
    // positional binary operators: z is int64, x and y are ignored
    //--------------------------------------------------------------------------

    GB_FIRSTI_opcode    = 93,   // z = first_i(A(i,j),y) == i
    GB_FIRSTI1_opcode   = 94,   // z = first_i1(A(i,j),y) == i+1
    GB_FIRSTJ_opcode    = 95,   // z = first_j(A(i,j),y) == j
    GB_FIRSTJ1_opcode   = 96,   // z = first_j1(A(i,j),y) == j+1

    GB_SECONDI_opcode   = 97,   // z = second_i(x,B(i,j)) == i
    GB_SECONDI1_opcode  = 98,   // z = second_i1(x,B(i,j)) == i+1
    GB_SECONDJ_opcode   = 99,   // z = second_j(x,B(i,j)) == j
    GB_SECONDJ1_opcode  = 100,  // z = second_j1(x,B(i,j)) == j+1

    //==========================================================================
    // user-defined: unary and binary operators
    //==========================================================================

    GB_USER_opcode = 101        // user-defined operator (unary or binary)
}
GB_Opcode ;

// true if the opcode is for a unary or binary positional operator
#define GB_OPCODE_IS_POSITIONAL(opcode) \
    (((opcode) >= GB_POSITIONI_opcode && (opcode) <= GB_POSITIONJ1_opcode) \
    || ((opcode) >= GB_FIRSTI_opcode && (opcode) <= GB_SECONDJ1_opcode))

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

int64_t GB_positional_offset        // return 0 or 1
(
    GB_Opcode opcode                // opcode of positional operator
) ;

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
    GB_USER_SELECT_opcode = 18
}
GB_Select_Opcode ;

#define GB_SELECTOP_IS_POSITIONAL(opcode) \
    ((opcode) >= GB_TRIL_opcode && (opcode) <= GB_OFFDIAG_opcode)

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

// string length for names of opaque objects
#define GB_LEN 128

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

struct GB_Monoid_opaque     // content of GrB_Monoid
{
    int64_t magic ;         // for detecting uninitialized objects
    GrB_BinaryOp op ;       // binary operator of the monoid
    void *identity ;        // identity of the monoid; type is op->ztype
    void *terminal ;        // early-exit (NULL if no value); type is op->ztype
    bool monoid_is_builtin ;       // built-in or user defined
} ;

struct GB_Semiring_opaque   // content of GrB_Semiring
{
    int64_t magic ;         // for detecting uninitialized objects
    GrB_Monoid add ;        // add operator of the semiring
    GrB_BinaryOp multiply ; // multiply operator of the semiring
    bool semiring_is_builtin ;       // built-in or user defined
} ;

struct GB_Descriptor_opaque // content of GrB_Descriptor
{
    int64_t magic ;         // for detecting uninitialized objects
    char *logger ;          // error logger string
    GrB_Desc_Value out ;    // output descriptor
    GrB_Desc_Value mask ;   // mask descriptor
    GrB_Desc_Value in0 ;    // first input descriptor (A for C=A*B, for example)
    GrB_Desc_Value in1 ;    // second input descriptor (B for C=A*B)
    GrB_Desc_Value axb ;    // for selecting the method for C=A*B
    int nthreads_max ;      // max # threads to use in this call to GraphBLAS
    double chunk ;          // chunk size for # of threads for small problems
    bool predefined ;       // if true, descriptor is predefined
    bool do_sort ;          // if nonzero, do the sort in GrB_mxm
    // #include "GB_Descriptor_opaque_mkl_template.h"
} ;

//------------------------------------------------------------------------------
// GB_Pending data structure: for scalars, vectors, and matrices
//------------------------------------------------------------------------------

// Pending tuples are a list of unsorted (i,j,x) tuples that have not yet been
// added to a matrix.  The data structure is defined in GB_Pending.h.

struct GB_Pending_struct    // list of pending tuples for a matrix
{
    int64_t n ;         // number of pending tuples to add to matrix
    int64_t nmax ;      // size of i,j,x
    bool sorted ;       // true if pending tuples are in sorted order
    int64_t *i ;        // row indices of pending tuples
    int64_t *j ;        // col indices of pending tuples; NULL if A->vdim <= 1
    GB_void *x ;        // values of pending tuples
    GrB_Type type ;     // the type of s
    size_t size ;       // type->size
    GrB_BinaryOp op ;   // operator to assemble pending tuples
} ;

typedef struct GB_Pending_struct *GB_Pending ;

//------------------------------------------------------------------------------
// scalar, vector, and matrix types
//------------------------------------------------------------------------------

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

//------------------------------------------------------------------------------
// Accessing the content of a scalar, vector, or matrix
//------------------------------------------------------------------------------

#define GBP(Ap,k,avlen) ((Ap == NULL) ? ((k) * (avlen)) : Ap [k])
#define GBH(Ah,k)       ((Ah == NULL) ? (k) : Ah [k])
#define GBI(Ai,p,avlen) ((Ai == NULL) ? ((p) % (avlen)) : Ai [p])
#define GBB(Ab,p)       ((Ab == NULL) ? 1 : Ab [p])
// #define GBX(...)     TODO: constant-valued matrices

#endif

