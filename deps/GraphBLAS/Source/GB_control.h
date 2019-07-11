//------------------------------------------------------------------------------
// GB_control.h:  disable hard-coded functions to reduce code size
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// The installer of SuiteSparse:GraphBLAS can edit this file to reduce the code
// size of the compiled library, by disabling the corresonding hard-coded
// functions in Source/Generated.  For example, if SuiteSparse:GraphBLAS is
// integrated into an application that makes no use of the GrB_INT16 data type,
// or just occassional use where performance is not a concern, then uncomment
// the line "#define GxB_NO_INT16 1".  Alternatively, SuiteSparse:GraphBLAS can
// be compiled with a list of options, such as -DGxB_NO_INT16=1, which does the
// same thing.

// GraphBLAS will still work as expected.  It will simply use a generic method
// in place of the type- or operator-specific code.  It will be slower, by
// about 2x or 3x, depending on the operation. but its results will be the
// same.  A few operations will be 10x slower, such as GrB_reduce to scalar
// using the GrB_MAX_FP64 operator.

// Enabling the "#define GBCOMPACT" option is the same as uncommenting this
// entire file.  This file provides a more concise control over which
// types, operators, and semirings are given fast hard-coded versions in
// Source/Generated, and which use the slower generic methods.

// However, the code size can be reduced significantly.  Uncommenting all of
// the options below cuts the code from 55MB to under 2.7MB, on a MacBook Pro
// using gcc 8.2.0 (as of the draft V3.0.0 version, June 18, 2019).  Disabling
// all types except GxB_NO_FP64 results in a code size of 7.8MB.

// Note that some semirings are renamed.  For example, C=A*B when all matrices
// are in CSC format, uses the semiring as-is.  If all matrices are in CSR
// format instead, then C'=B'*A' is computed, treating the internal matrices
// as if they are in CSC format.  To accomplish this, the semiring may be
// "flipped", if the multiply operator is not commutative.  That is,
// the GxB_PLUS_FIRST_* semiring is replaced with GxB_PLUS_SECOND_*.  Below
// is a list of all multiplicative operators and their "flipped" pair.

// As a result of the "flip", if the FIRST operator is disabled, it may disable
// some uses of the GxB_*_SECOND_* semirings, and visa versa, depending on the
// matrix formats.  I recommend that if you want to use the FIRST operator with
// fast hard-coded semirings, then do not disable FIRST or SECOND.  The
// following is a complete list of all pairs of operators that may be replaced
// with the other.  I recommend either keeping both of each pair, or disabling
// both.

        // FIRST and SECOND
        // LT and GT
        // LE and GE
        // ISLT and ISGT
        // ISLE and ISGE
        // DIV and RDIV
        // MINUS and RMINUS

// In addition, many of the Boolean operators are not unique, and are renamed
// internally.  The operators listed here are just the unique ones.  In
// particular, GrB_DIV_BOOL is identical to GrB_FIRST_BOOL, so the GrB_DIV_BOOL
// operator is replaced internally with GrB_FIRST_BOOL.

        // FIRST and DIV                        : FIRST is used for both
        // SECOND and RDIV                      : SECOND
        // MIN, TIMES, and LAND                 : LAND
        // MAX, PLUS, and LOR                   : LOR
        // ISNE, NE, MINUS, RMINUS, and LXOR    : LXOR
        // ISEQ, EQ                             : EQ
        // ISGT, GT                             : GT
        // ISLT, LT                             : LT
        // ISGE, GE                             : GE
        // ISLE, LE                             : LE

// Thus, below there is a #define GxB_NO_LAND_FIRST_BOOL, but no #define
// GxB_NO_LAND_DIV_BOOL.

// Note that there are no macros that disable the hard-coded functions for
// GxB_select (Generated/GB_sel__*), since they have no generic equivalents.

//------------------------------------------------------------------------------
// uncomment any of these lines to disable the types
//------------------------------------------------------------------------------

// These disable all semirings with the corresponding type of x,y for the
// multiplicative operator, for GrB_mxm, GrB_vxm, and GrB_mxv.

// They also disable the hard-coded functions for GrB_eWiseAdd, GrB_eWiseMult,
// GrB_reduce, GrB_*_build, GrB_apply, and GrB_transpose for this type.

// #define GxB_NO_BOOL      1
#define GxB_NO_FP32      1
#define GxB_NO_FP64      1
#define GxB_NO_INT16     1
#define GxB_NO_INT32     1
#define GxB_NO_INT64     1
#define GxB_NO_INT8      1
#define GxB_NO_UINT16    1
#define GxB_NO_UINT32    1
// #define GxB_NO_UINT64    1
#define GxB_NO_UINT8     1

//------------------------------------------------------------------------------
// uncomment any of these lines to disable the unary operators
//------------------------------------------------------------------------------

// These disable all unary operators for GrB_apply.  These options have no
// effect on GrB_mxm, GrB_vxm, GrB_mxv, GrB_eWiseAdd, GrB_eWiseMult,
// GrB_reduce, or GrB_*_build.

// #define GxB_NO_ABS       1
// #define GxB_NO_AINV      1
// #define GxB_NO_IDENTITY  1
// #define GxB_NO_LNOT      1
// #define GxB_NO_MINV      1
// #define GxB_NO_ONE       1

//------------------------------------------------------------------------------
// uncomment any of these lines to disable the binary operators for all types
//------------------------------------------------------------------------------

// These disable all semirings with the corresponding additive or
// multiplicative operator, for GrB_mxm, GrB_vxm, and GrB_mxv.

// They also disable the hard-coded functions for GrB_eWiseAdd, GrB_eWiseMult,
// GrB_reduce, and GrB_*_build for this binary operator.

// #define GxB_NO_FIRST     1
// #define GxB_NO_SECOND    1
// #define GxB_NO_MIN       1
// #define GxB_NO_MAX       1
// #define GxB_NO_PLUS      1
// #define GxB_NO_MINUS     1
// #define GxB_NO_RMINUS    1
// #define GxB_NO_TIMES     1
// #define GxB_NO_DIV       1
// #define GxB_NO_RDIV      1
// #define GxB_NO_ISEQ      1
// #define GxB_NO_ISNE      1
// #define GxB_NO_ISGT      1
// #define GxB_NO_ISGE      1
// #define GxB_NO_ISLT      1
// #define GxB_NO_ISLE      1
// #define GxB_NO_EQ        1
// #define GxB_NO_NE        1
// #define GxB_NO_GT        1
// #define GxB_NO_LT        1
// #define GxB_NO_LE        1
// #define GxB_NO_GE        1
// #define GxB_NO_LAND      1
// #define GxB_NO_LOR       1
// #define GxB_NO_LXOR      1

//------------------------------------------------------------------------------
// uncomment any of these lines to disable the binary operators for each type
//------------------------------------------------------------------------------

// These disable all semirings with the corresponding additive or
// multiplicative operator with the particular type, for GrB_mxm, GrB_vxm, and
// GrB_mxv.

// They also disable the hard-coded functions for GrB_eWiseAdd, GrB_eWiseMult,
// GrB_reduce, and GrB_*_build for this binary operator, for the particular
// type.

// These options are more precise than the ones above.  For example,
// GxB_NO_TIMES, above, disables the GrB_TIMES_<type> operator for all types
// (except bool, which is renamed as GrB_LAND).  GxB_NO_INT16, above, disables
// all operators of the form GrB_<op>_INT16.  But GxB_NO_TIMES_INT16, listed
// below, disables just the single GrB_TIMES_INT16 operator, and all semirings
// that use it.  It has no effect on GrB_TIMES_FP64, for example, or any other
// operator.

// Note that some boolean operators do not appear.  GrB_MIN_BOOL and
// GrB_TIMES_BOOL are renamed internally to GrB_LAND_BOOL, for example, so
// uncommenting GxB_NO_LAND_BOOL disables all three operators.

// #define GxB_NO_FIRST_INT8    1
// #define GxB_NO_FIRST_INT16   1
// #define GxB_NO_FIRST_INT32   1
// #define GxB_NO_FIRST_INT64   1
// #define GxB_NO_FIRST_UINT8   1
// #define GxB_NO_FIRST_UINT16  1
// #define GxB_NO_FIRST_UINT32  1
// #define GxB_NO_FIRST_UINT64  1
// #define GxB_NO_FIRST_FP32    1
// #define GxB_NO_FIRST_FP64    1
// #define GxB_NO_FIRST_BOOL    1

// #define GxB_NO_SECOND_INT8   1
// #define GxB_NO_SECOND_INT16  1
// #define GxB_NO_SECOND_INT32  1
// #define GxB_NO_SECOND_INT64  1
// #define GxB_NO_SECOND_UINT8  1
// #define GxB_NO_SECOND_UINT16 1
// #define GxB_NO_SECOND_UINT32 1
// #define GxB_NO_SECOND_UINT64 1
// #define GxB_NO_SECOND_FP32   1
// #define GxB_NO_SECOND_FP64   1
// #define GxB_NO_SECOND_BOOL   1

// #define GxB_NO_MIN_INT8      1
// #define GxB_NO_MIN_INT16     1
// #define GxB_NO_MIN_INT32     1
// #define GxB_NO_MIN_INT64     1
// #define GxB_NO_MIN_UINT8     1
// #define GxB_NO_MIN_UINT16    1
// #define GxB_NO_MIN_UINT32    1
// #define GxB_NO_MIN_UINT64    1
// #define GxB_NO_MIN_FP32      1
// #define GxB_NO_MIN_FP64      1

// #define GxB_NO_MAX_INT8      1
// #define GxB_NO_MAX_INT16     1
// #define GxB_NO_MAX_INT32     1
// #define GxB_NO_MAX_INT64     1
// #define GxB_NO_MAX_UINT8     1
// #define GxB_NO_MAX_UINT16    1
// #define GxB_NO_MAX_UINT32    1
// #define GxB_NO_MAX_UINT64    1
// #define GxB_NO_MAX_FP32      1
// #define GxB_NO_MAX_FP64      1

// #define GxB_NO_PLUS_INT8     1
// #define GxB_NO_PLUS_INT16    1
// #define GxB_NO_PLUS_INT32    1
// #define GxB_NO_PLUS_INT64    1
// #define GxB_NO_PLUS_UINT8    1
// #define GxB_NO_PLUS_UINT16   1
// #define GxB_NO_PLUS_UINT32   1
// #define GxB_NO_PLUS_UINT64   1
// #define GxB_NO_PLUS_FP32     1
// #define GxB_NO_PLUS_FP64     1

// #define GxB_NO_MINUS_INT8    1
// #define GxB_NO_MINUS_INT16   1
// #define GxB_NO_MINUS_INT32   1
// #define GxB_NO_MINUS_INT64   1
// #define GxB_NO_MINUS_UINT8   1
// #define GxB_NO_MINUS_UINT16  1
// #define GxB_NO_MINUS_UINT32  1
// #define GxB_NO_MINUS_UINT64  1
// #define GxB_NO_MINUS_FP32    1
// #define GxB_NO_MINUS_FP64    1

// #define GxB_NO_RMINUS_INT8   1
// #define GxB_NO_RMINUS_INT16  1
// #define GxB_NO_RMINUS_INT32  1
// #define GxB_NO_RMINUS_INT64  1
// #define GxB_NO_RMINUS_UINT8  1
// #define GxB_NO_RMINUS_UINT16 1
// #define GxB_NO_RMINUS_UINT32 1
// #define GxB_NO_RMINUS_UINT64 1
// #define GxB_NO_RMINUS_FP32   1
// #define GxB_NO_RMINUS_FP64   1

// #define GxB_NO_TIMES_INT8    1
// #define GxB_NO_TIMES_INT16   1
// #define GxB_NO_TIMES_INT32   1
// #define GxB_NO_TIMES_INT64   1
// #define GxB_NO_TIMES_UINT8   1
// #define GxB_NO_TIMES_UINT16  1
// #define GxB_NO_TIMES_UINT32  1
// #define GxB_NO_TIMES_UINT64  1
// #define GxB_NO_TIMES_FP32    1
// #define GxB_NO_TIMES_FP64    1

// #define GxB_NO_DIV_INT8      1
// #define GxB_NO_DIV_INT16     1
// #define GxB_NO_DIV_INT32     1
// #define GxB_NO_DIV_INT64     1
// #define GxB_NO_DIV_UINT8     1
// #define GxB_NO_DIV_UINT16    1
// #define GxB_NO_DIV_UINT32    1
// #define GxB_NO_DIV_UINT64    1
// #define GxB_NO_DIV_FP32      1
// #define GxB_NO_DIV_FP64      1

// #define GxB_NO_RDIV_INT8     1
// #define GxB_NO_RDIV_INT16    1
// #define GxB_NO_RDIV_INT32    1
// #define GxB_NO_RDIV_INT64    1
// #define GxB_NO_RDIV_UINT8    1
// #define GxB_NO_RDIV_UINT16   1
// #define GxB_NO_RDIV_UINT32   1
// #define GxB_NO_RDIV_UINT64   1
// #define GxB_NO_RDIV_FP32     1
// #define GxB_NO_RDIV_FP64     1

// #define GxB_NO_ISEQ_INT8     1
// #define GxB_NO_ISEQ_INT16    1
// #define GxB_NO_ISEQ_INT32    1
// #define GxB_NO_ISEQ_INT64    1
// #define GxB_NO_ISEQ_UINT8    1
// #define GxB_NO_ISEQ_UINT16   1
// #define GxB_NO_ISEQ_UINT32   1
// #define GxB_NO_ISEQ_UINT64   1
// #define GxB_NO_ISEQ_FP32     1
// #define GxB_NO_ISEQ_FP64     1

// #define GxB_NO_ISNE_INT8     1
// #define GxB_NO_ISNE_INT16    1
// #define GxB_NO_ISNE_INT32    1
// #define GxB_NO_ISNE_INT64    1
// #define GxB_NO_ISNE_UINT8    1
// #define GxB_NO_ISNE_UINT16   1
// #define GxB_NO_ISNE_UINT32   1
// #define GxB_NO_ISNE_UINT64   1
// #define GxB_NO_ISNE_FP32     1
// #define GxB_NO_ISNE_FP64     1

// #define GxB_NO_ISGT_INT8     1
// #define GxB_NO_ISGT_INT16    1
// #define GxB_NO_ISGT_INT32    1
// #define GxB_NO_ISGT_INT64    1
// #define GxB_NO_ISGT_UINT8    1
// #define GxB_NO_ISGT_UINT16   1
// #define GxB_NO_ISGT_UINT32   1
// #define GxB_NO_ISGT_UINT64   1
// #define GxB_NO_ISGT_FP32     1
// #define GxB_NO_ISGT_FP64     1

// #define GxB_NO_ISLT_INT8     1
// #define GxB_NO_ISLT_INT16    1
// #define GxB_NO_ISLT_INT32    1
// #define GxB_NO_ISLT_INT64    1
// #define GxB_NO_ISLT_UINT8    1
// #define GxB_NO_ISLT_UINT16   1
// #define GxB_NO_ISLT_UINT32   1
// #define GxB_NO_ISLT_UINT64   1
// #define GxB_NO_ISLT_FP32     1
// #define GxB_NO_ISLT_FP64     1

// #define GxB_NO_ISGE_INT8     1
// #define GxB_NO_ISGE_INT16    1
// #define GxB_NO_ISGE_INT32    1
// #define GxB_NO_ISGE_INT64    1
// #define GxB_NO_ISGE_UINT8    1
// #define GxB_NO_ISGE_UINT16   1
// #define GxB_NO_ISGE_UINT32   1
// #define GxB_NO_ISGE_UINT64   1
// #define GxB_NO_ISGE_FP32     1
// #define GxB_NO_ISGE_FP64     1

// #define GxB_NO_ISLE_INT8     1
// #define GxB_NO_ISLE_INT16    1
// #define GxB_NO_ISLE_INT32    1
// #define GxB_NO_ISLE_INT64    1
// #define GxB_NO_ISLE_UINT8    1
// #define GxB_NO_ISLE_UINT16   1
// #define GxB_NO_ISLE_UINT32   1
// #define GxB_NO_ISLE_UINT64   1
// #define GxB_NO_ISLE_FP32     1
// #define GxB_NO_ISLE_FP64     1

// #define GxB_NO_EQ_INT8       1
// #define GxB_NO_EQ_INT16      1
// #define GxB_NO_EQ_INT32      1
// #define GxB_NO_EQ_INT64      1
// #define GxB_NO_EQ_UINT8      1
// #define GxB_NO_EQ_UINT16     1
// #define GxB_NO_EQ_UINT32     1
// #define GxB_NO_EQ_UINT64     1
// #define GxB_NO_EQ_FP32       1
// #define GxB_NO_EQ_FP64       1
// #define GxB_NO_EQ_BOOL       1

// #define GxB_NO_NE_INT8       1
// #define GxB_NO_NE_INT16      1
// #define GxB_NO_NE_INT32      1
// #define GxB_NO_NE_INT64      1
// #define GxB_NO_NE_UINT8      1
// #define GxB_NO_NE_UINT16     1
// #define GxB_NO_NE_UINT32     1
// #define GxB_NO_NE_UINT64     1
// #define GxB_NO_NE_FP32       1
// #define GxB_NO_NE_FP64       1

// #define GxB_NO_GT_INT8       1
// #define GxB_NO_GT_INT16      1
// #define GxB_NO_GT_INT32      1
// #define GxB_NO_GT_INT64      1
// #define GxB_NO_GT_UINT8      1
// #define GxB_NO_GT_UINT16     1
// #define GxB_NO_GT_UINT32     1
// #define GxB_NO_GT_UINT64     1
// #define GxB_NO_GT_FP32       1
// #define GxB_NO_GT_FP64       1
// #define GxB_NO_GT_BOOL       1

// #define GxB_NO_LT_INT8       1
// #define GxB_NO_LT_INT16      1
// #define GxB_NO_LT_INT32      1
// #define GxB_NO_LT_INT64      1
// #define GxB_NO_LT_UINT8      1
// #define GxB_NO_LT_UINT16     1
// #define GxB_NO_LT_UINT32     1
// #define GxB_NO_LT_UINT64     1
// #define GxB_NO_LT_FP32       1
// #define GxB_NO_LT_FP64       1
// #define GxB_NO_LT_BOOL       1

// #define GxB_NO_GE_INT8       1
// #define GxB_NO_GE_INT16      1
// #define GxB_NO_GE_INT32      1
// #define GxB_NO_GE_INT64      1
// #define GxB_NO_GE_UINT8      1
// #define GxB_NO_GE_UINT16     1
// #define GxB_NO_GE_UINT32     1
// #define GxB_NO_GE_UINT64     1
// #define GxB_NO_GE_FP32       1
// #define GxB_NO_GE_FP64       1
// #define GxB_NO_GE_BOOL       1

// #define GxB_NO_LE_INT8       1
// #define GxB_NO_LE_INT16      1
// #define GxB_NO_LE_INT32      1
// #define GxB_NO_LE_INT64      1
// #define GxB_NO_LE_UINT8      1
// #define GxB_NO_LE_UINT16     1
// #define GxB_NO_LE_UINT32     1
// #define GxB_NO_LE_UINT64     1
// #define GxB_NO_LE_FP32       1
// #define GxB_NO_LE_FP64       1
// #define GxB_NO_LE_BOOL

// #define GxB_NO_LOR_INT8      1
// #define GxB_NO_LOR_INT16     1
// #define GxB_NO_LOR_INT32     1
// #define GxB_NO_LOR_INT64     1
// #define GxB_NO_LOR_UINT8     1
// #define GxB_NO_LOR_UINT16    1
// #define GxB_NO_LOR_UINT32    1
// #define GxB_NO_LOR_UINT64    1
// #define GxB_NO_LOR_FP32      1
// #define GxB_NO_LOR_FP64      1
// #define GxB_NO_LOR_BOOL      1

// #define GxB_NO_LAND_INT8     1
// #define GxB_NO_LAND_INT16    1
// #define GxB_NO_LAND_INT32    1
// #define GxB_NO_LAND_INT64    1
// #define GxB_NO_LAND_UINT8    1
// #define GxB_NO_LAND_UINT16   1
// #define GxB_NO_LAND_UINT32   1
// #define GxB_NO_LAND_UINT64   1
// #define GxB_NO_LAND_FP32     1
// #define GxB_NO_LAND_FP64     1
// #define GxB_NO_LAND_BOOL     1

// #define GxB_NO_LXOR_INT8     1
// #define GxB_NO_LXOR_INT16    1
// #define GxB_NO_LXOR_INT32    1
// #define GxB_NO_LXOR_INT64    1
// #define GxB_NO_LXOR_UINT8    1
// #define GxB_NO_LXOR_UINT16   1
// #define GxB_NO_LXOR_UINT32   1
// #define GxB_NO_LXOR_UINT64   1
// #define GxB_NO_LXOR_FP32     1
// #define GxB_NO_LXOR_FP64     1
// #define GxB_NO_LXOR_BOOL     1

//------------------------------------------------------------------------------
// uncomment any of these lines to disable the corresponding semiring
//------------------------------------------------------------------------------

// These options are still more precise.  They disable all semirings with the
// corresponding semiring, for GrB_mxm, GrB_vxm, and GrB_mxv.  For example, if
// GrB_NO_FP64 is enabled above, then all semirings of the form
// GxB_<addop>_<multop>_FP64 are disabled, for any addop or multop operators.
// If GrB_NO_PLUS is enabled above, then all semirings of the form
// GxB_PLUS_<multop>_<type> and GxB_<multop>_<PLUS>_<type> are disabled.

// These options have no effect on GrB_eWiseAdd, GrB_eWiseMult, GrB_reduce,
// GrB_*_build, or GrB_apply.

// #define GxB_NO_EQ_EQ_BOOL            1
// #define GxB_NO_EQ_EQ_FP32            1
// #define GxB_NO_EQ_EQ_FP64            1
// #define GxB_NO_EQ_EQ_INT16           1
// #define GxB_NO_EQ_EQ_INT32           1
// #define GxB_NO_EQ_EQ_INT64           1
// #define GxB_NO_EQ_EQ_INT8            1
// #define GxB_NO_EQ_EQ_UINT16          1
// #define GxB_NO_EQ_EQ_UINT32          1
// #define GxB_NO_EQ_EQ_UINT64          1
// #define GxB_NO_EQ_EQ_UINT8           1

// #define GxB_NO_EQ_FIRST_BOOL         1

// #define GxB_NO_EQ_GE_BOOL            1
// #define GxB_NO_EQ_GE_FP32            1
// #define GxB_NO_EQ_GE_FP64            1
// #define GxB_NO_EQ_GE_INT16           1
// #define GxB_NO_EQ_GE_INT32           1
// #define GxB_NO_EQ_GE_INT64           1
// #define GxB_NO_EQ_GE_INT8            1
// #define GxB_NO_EQ_GE_UINT16          1
// #define GxB_NO_EQ_GE_UINT32          1
// #define GxB_NO_EQ_GE_UINT64          1
// #define GxB_NO_EQ_GE_UINT8           1

// #define GxB_NO_EQ_GT_BOOL            1

// #define GxB_NO_EQ_GT_FP32            1
// #define GxB_NO_EQ_GT_FP64            1
// #define GxB_NO_EQ_GT_INT16           1
// #define GxB_NO_EQ_GT_INT32           1
// #define GxB_NO_EQ_GT_INT64           1
// #define GxB_NO_EQ_GT_INT8            1
// #define GxB_NO_EQ_GT_UINT16          1
// #define GxB_NO_EQ_GT_UINT32          1
// #define GxB_NO_EQ_GT_UINT64          1
// #define GxB_NO_EQ_GT_UINT8           1

// #define GxB_NO_EQ_LAND_BOOL          1

// #define GxB_NO_EQ_LE_BOOL            1
// #define GxB_NO_EQ_LE_FP32            1
// #define GxB_NO_EQ_LE_FP64            1
// #define GxB_NO_EQ_LE_INT16           1
// #define GxB_NO_EQ_LE_INT32           1
// #define GxB_NO_EQ_LE_INT64           1
// #define GxB_NO_EQ_LE_INT8            1
// #define GxB_NO_EQ_LE_UINT16          1
// #define GxB_NO_EQ_LE_UINT32          1
// #define GxB_NO_EQ_LE_UINT64          1
// #define GxB_NO_EQ_LE_UINT8           1

// #define GxB_NO_EQ_LOR_BOOL           1

// #define GxB_NO_EQ_LT_BOOL            1
// #define GxB_NO_EQ_LT_FP32            1
// #define GxB_NO_EQ_LT_FP64            1
// #define GxB_NO_EQ_LT_INT16           1
// #define GxB_NO_EQ_LT_INT32           1
// #define GxB_NO_EQ_LT_INT64           1
// #define GxB_NO_EQ_LT_INT8            1
// #define GxB_NO_EQ_LT_UINT16          1
// #define GxB_NO_EQ_LT_UINT32          1
// #define GxB_NO_EQ_LT_UINT64          1
// #define GxB_NO_EQ_LT_UINT8           1

// #define GxB_NO_EQ_LXOR_BOOL          1

// #define GxB_NO_EQ_NE_FP32            1
// #define GxB_NO_EQ_NE_FP64            1
// #define GxB_NO_EQ_NE_INT16           1
// #define GxB_NO_EQ_NE_INT32           1
// #define GxB_NO_EQ_NE_INT64           1
// #define GxB_NO_EQ_NE_INT8            1
// #define GxB_NO_EQ_NE_UINT16          1
// #define GxB_NO_EQ_NE_UINT32          1
// #define GxB_NO_EQ_NE_UINT64          1
// #define GxB_NO_EQ_NE_UINT8           1

// #define GxB_NO_EQ_SECOND_BOOL        1

// #define GxB_NO_LAND_EQ_BOOL          1
// #define GxB_NO_LAND_EQ_FP32          1
// #define GxB_NO_LAND_EQ_FP64          1
// #define GxB_NO_LAND_EQ_INT16         1
// #define GxB_NO_LAND_EQ_INT32         1
// #define GxB_NO_LAND_EQ_INT64         1
// #define GxB_NO_LAND_EQ_INT8          1
// #define GxB_NO_LAND_EQ_UINT16        1
// #define GxB_NO_LAND_EQ_UINT32        1
// #define GxB_NO_LAND_EQ_UINT64        1
// #define GxB_NO_LAND_EQ_UINT8         1

// #define GxB_NO_LAND_FIRST_BOOL       1

// #define GxB_NO_LAND_GE_BOOL          1
// #define GxB_NO_LAND_GE_FP32          1
// #define GxB_NO_LAND_GE_FP64          1
// #define GxB_NO_LAND_GE_INT16         1
// #define GxB_NO_LAND_GE_INT32         1
// #define GxB_NO_LAND_GE_INT64         1
// #define GxB_NO_LAND_GE_INT8          1
// #define GxB_NO_LAND_GE_UINT16        1
// #define GxB_NO_LAND_GE_UINT32        1
// #define GxB_NO_LAND_GE_UINT64        1
// #define GxB_NO_LAND_GE_UINT8         1

// #define GxB_NO_LAND_GT_BOOL          1

// #define GxB_NO_LAND_GT_FP32          1
// #define GxB_NO_LAND_GT_FP64          1
// #define GxB_NO_LAND_GT_INT16         1
// #define GxB_NO_LAND_GT_INT32         1
// #define GxB_NO_LAND_GT_INT64         1
// #define GxB_NO_LAND_GT_INT8          1
// #define GxB_NO_LAND_GT_UINT16        1
// #define GxB_NO_LAND_GT_UINT32        1
// #define GxB_NO_LAND_GT_UINT64        1
// #define GxB_NO_LAND_GT_UINT8         1

// #define GxB_NO_LAND_LAND_BOOL        1

// #define GxB_NO_LAND_LE_BOOL          1
// #define GxB_NO_LAND_LE_FP32          1
// #define GxB_NO_LAND_LE_FP64          1
// #define GxB_NO_LAND_LE_INT16         1
// #define GxB_NO_LAND_LE_INT32         1
// #define GxB_NO_LAND_LE_INT64         1
// #define GxB_NO_LAND_LE_INT8          1
// #define GxB_NO_LAND_LE_UINT16        1
// #define GxB_NO_LAND_LE_UINT32        1
// #define GxB_NO_LAND_LE_UINT64        1
// #define GxB_NO_LAND_LE_UINT8         1

// #define GxB_NO_LAND_LOR_BOOL         1

// #define GxB_NO_LAND_LT_BOOL          1
// #define GxB_NO_LAND_LT_FP32          1
// #define GxB_NO_LAND_LT_FP64          1
// #define GxB_NO_LAND_LT_INT16         1
// #define GxB_NO_LAND_LT_INT32         1
// #define GxB_NO_LAND_LT_INT64         1
// #define GxB_NO_LAND_LT_INT8          1
// #define GxB_NO_LAND_LT_UINT16        1
// #define GxB_NO_LAND_LT_UINT32        1
// #define GxB_NO_LAND_LT_UINT64        1
// #define GxB_NO_LAND_LT_UINT8         1

// #define GxB_NO_LAND_LXOR_BOOL        1

// #define GxB_NO_LAND_NE_FP32          1
// #define GxB_NO_LAND_NE_FP64          1
// #define GxB_NO_LAND_NE_INT16         1
// #define GxB_NO_LAND_NE_INT32         1
// #define GxB_NO_LAND_NE_INT64         1
// #define GxB_NO_LAND_NE_INT8          1
// #define GxB_NO_LAND_NE_UINT16        1
// #define GxB_NO_LAND_NE_UINT32        1
// #define GxB_NO_LAND_NE_UINT64        1
// #define GxB_NO_LAND_NE_UINT8         1

// #define GxB_NO_LAND_SECOND_BOOL      1

// #define GxB_NO_LOR_EQ_BOOL           1
// #define GxB_NO_LOR_EQ_FP32           1
// #define GxB_NO_LOR_EQ_FP64           1
// #define GxB_NO_LOR_EQ_INT16          1
// #define GxB_NO_LOR_EQ_INT32          1
// #define GxB_NO_LOR_EQ_INT64          1
// #define GxB_NO_LOR_EQ_INT8           1
// #define GxB_NO_LOR_EQ_UINT16         1
// #define GxB_NO_LOR_EQ_UINT32         1
// #define GxB_NO_LOR_EQ_UINT64         1
// #define GxB_NO_LOR_EQ_UINT8          1

// #define GxB_NO_LOR_FIRST_BOOL        1

// #define GxB_NO_LOR_GE_BOOL           1
// #define GxB_NO_LOR_GE_FP32           1
// #define GxB_NO_LOR_GE_FP64           1
// #define GxB_NO_LOR_GE_INT16          1
// #define GxB_NO_LOR_GE_INT32          1
// #define GxB_NO_LOR_GE_INT64          1
// #define GxB_NO_LOR_GE_INT8           1
// #define GxB_NO_LOR_GE_UINT16         1
// #define GxB_NO_LOR_GE_UINT32         1
// #define GxB_NO_LOR_GE_UINT64         1
// #define GxB_NO_LOR_GE_UINT8          1

// #define GxB_NO_LOR_GT_BOOL           1
// #define GxB_NO_LOR_GT_FP32           1
// #define GxB_NO_LOR_GT_FP64           1
// #define GxB_NO_LOR_GT_INT16          1
// #define GxB_NO_LOR_GT_INT32          1
// #define GxB_NO_LOR_GT_INT64          1
// #define GxB_NO_LOR_GT_INT8           1
// #define GxB_NO_LOR_GT_UINT16         1
// #define GxB_NO_LOR_GT_UINT32         1
// #define GxB_NO_LOR_GT_UINT64         1
// #define GxB_NO_LOR_GT_UINT8          1

// #define GxB_NO_LOR_LAND_BOOL         1

// #define GxB_NO_LOR_LE_BOOL           1
// #define GxB_NO_LOR_LE_FP32           1
// #define GxB_NO_LOR_LE_FP64           1
// #define GxB_NO_LOR_LE_INT16          1
// #define GxB_NO_LOR_LE_INT32          1
// #define GxB_NO_LOR_LE_INT64          1
// #define GxB_NO_LOR_LE_INT8           1
// #define GxB_NO_LOR_LE_UINT16         1
// #define GxB_NO_LOR_LE_UINT32         1
// #define GxB_NO_LOR_LE_UINT64         1
// #define GxB_NO_LOR_LE_UINT8          1

// #define GxB_NO_LOR_LOR_BOOL          1

// #define GxB_NO_LOR_LT_BOOL           1
// #define GxB_NO_LOR_LT_FP32           1
// #define GxB_NO_LOR_LT_FP64           1
// #define GxB_NO_LOR_LT_INT16          1
// #define GxB_NO_LOR_LT_INT32          1
// #define GxB_NO_LOR_LT_INT64          1
// #define GxB_NO_LOR_LT_INT8           1
// #define GxB_NO_LOR_LT_UINT16         1
// #define GxB_NO_LOR_LT_UINT32         1
// #define GxB_NO_LOR_LT_UINT64         1
// #define GxB_NO_LOR_LT_UINT8          1

// #define GxB_NO_LOR_LXOR_BOOL         1

// #define GxB_NO_LOR_NE_FP32           1
// #define GxB_NO_LOR_NE_FP64           1
// #define GxB_NO_LOR_NE_INT16          1
// #define GxB_NO_LOR_NE_INT32          1
// #define GxB_NO_LOR_NE_INT64          1
// #define GxB_NO_LOR_NE_INT8           1
// #define GxB_NO_LOR_NE_UINT16         1
// #define GxB_NO_LOR_NE_UINT32         1
// #define GxB_NO_LOR_NE_UINT64         1
// #define GxB_NO_LOR_NE_UINT8          1

// #define GxB_NO_LOR_SECOND_BOOL       1

// #define GxB_NO_LXOR_EQ_BOOL          1
// #define GxB_NO_LXOR_EQ_FP32          1
// #define GxB_NO_LXOR_EQ_FP64          1
// #define GxB_NO_LXOR_EQ_INT16         1
// #define GxB_NO_LXOR_EQ_INT32         1
// #define GxB_NO_LXOR_EQ_INT64         1
// #define GxB_NO_LXOR_EQ_INT8          1
// #define GxB_NO_LXOR_EQ_UINT16        1
// #define GxB_NO_LXOR_EQ_UINT32        1
// #define GxB_NO_LXOR_EQ_UINT64        1
// #define GxB_NO_LXOR_EQ_UINT8         1

// #define GxB_NO_LXOR_FIRST_BOOL       1

// #define GxB_NO_LXOR_GE_BOOL          1
// #define GxB_NO_LXOR_GE_FP32          1
// #define GxB_NO_LXOR_GE_FP64          1
// #define GxB_NO_LXOR_GE_INT16         1
// #define GxB_NO_LXOR_GE_INT32         1
// #define GxB_NO_LXOR_GE_INT64         1
// #define GxB_NO_LXOR_GE_INT8          1
// #define GxB_NO_LXOR_GE_UINT16        1
// #define GxB_NO_LXOR_GE_UINT32        1
// #define GxB_NO_LXOR_GE_UINT64        1
// #define GxB_NO_LXOR_GE_UINT8         1

// #define GxB_NO_LXOR_GT_BOOL          1
// #define GxB_NO_LXOR_GT_FP32          1
// #define GxB_NO_LXOR_GT_FP64          1
// #define GxB_NO_LXOR_GT_INT16         1
// #define GxB_NO_LXOR_GT_INT32         1
// #define GxB_NO_LXOR_GT_INT64         1
// #define GxB_NO_LXOR_GT_INT8          1
// #define GxB_NO_LXOR_GT_UINT16        1
// #define GxB_NO_LXOR_GT_UINT32        1
// #define GxB_NO_LXOR_GT_UINT64        1
// #define GxB_NO_LXOR_GT_UINT8         1

// #define GxB_NO_LXOR_LAND_BOOL        1

// #define GxB_NO_LXOR_LE_BOOL          1
// #define GxB_NO_LXOR_LE_FP32          1
// #define GxB_NO_LXOR_LE_FP64          1
// #define GxB_NO_LXOR_LE_INT16         1
// #define GxB_NO_LXOR_LE_INT32         1
// #define GxB_NO_LXOR_LE_INT64         1
// #define GxB_NO_LXOR_LE_INT8          1
// #define GxB_NO_LXOR_LE_UINT16        1
// #define GxB_NO_LXOR_LE_UINT32        1
// #define GxB_NO_LXOR_LE_UINT64        1
// #define GxB_NO_LXOR_LE_UINT8         1

// #define GxB_NO_LXOR_LOR_BOOL         1

// #define GxB_NO_LXOR_LT_BOOL          1
// #define GxB_NO_LXOR_LT_FP32          1
// #define GxB_NO_LXOR_LT_FP64          1
// #define GxB_NO_LXOR_LT_INT16         1
// #define GxB_NO_LXOR_LT_INT32         1
// #define GxB_NO_LXOR_LT_INT64         1
// #define GxB_NO_LXOR_LT_INT8          1
// #define GxB_NO_LXOR_LT_UINT16        1
// #define GxB_NO_LXOR_LT_UINT32        1
// #define GxB_NO_LXOR_LT_UINT64        1
// #define GxB_NO_LXOR_LT_UINT8         1

// #define GxB_NO_LXOR_LXOR_BOOL        1

// #define GxB_NO_LXOR_NE_FP32          1
// #define GxB_NO_LXOR_NE_FP64          1
// #define GxB_NO_LXOR_NE_INT16         1
// #define GxB_NO_LXOR_NE_INT32         1
// #define GxB_NO_LXOR_NE_INT64         1
// #define GxB_NO_LXOR_NE_INT8          1
// #define GxB_NO_LXOR_NE_UINT16        1
// #define GxB_NO_LXOR_NE_UINT32        1
// #define GxB_NO_LXOR_NE_UINT64        1
// #define GxB_NO_LXOR_NE_UINT8         1

// #define GxB_NO_LXOR_SECOND_BOOL      1

// #define GxB_NO_MAX_DIV_FP32          1
// #define GxB_NO_MAX_DIV_FP64          1
// #define GxB_NO_MAX_DIV_INT16         1
// #define GxB_NO_MAX_DIV_INT32         1
// #define GxB_NO_MAX_DIV_INT64         1
// #define GxB_NO_MAX_DIV_INT8          1
// #define GxB_NO_MAX_DIV_UINT16        1
// #define GxB_NO_MAX_DIV_UINT32        1
// #define GxB_NO_MAX_DIV_UINT64        1
// #define GxB_NO_MAX_DIV_UINT8         1

// #define GxB_NO_MAX_FIRST_FP32        1
// #define GxB_NO_MAX_FIRST_FP64        1
// #define GxB_NO_MAX_FIRST_INT16       1
// #define GxB_NO_MAX_FIRST_INT32       1
// #define GxB_NO_MAX_FIRST_INT64       1
// #define GxB_NO_MAX_FIRST_INT8        1
// #define GxB_NO_MAX_FIRST_UINT16      1
// #define GxB_NO_MAX_FIRST_UINT32      1
// #define GxB_NO_MAX_FIRST_UINT64      1
// #define GxB_NO_MAX_FIRST_UINT8       1

// #define GxB_NO_MAX_ISEQ_FP32         1
// #define GxB_NO_MAX_ISEQ_FP64         1
// #define GxB_NO_MAX_ISEQ_INT16        1
// #define GxB_NO_MAX_ISEQ_INT32        1
// #define GxB_NO_MAX_ISEQ_INT64        1
// #define GxB_NO_MAX_ISEQ_INT8         1
// #define GxB_NO_MAX_ISEQ_UINT16       1
// #define GxB_NO_MAX_ISEQ_UINT32       1
// #define GxB_NO_MAX_ISEQ_UINT64       1
// #define GxB_NO_MAX_ISEQ_UINT8        1

// #define GxB_NO_MAX_ISGE_FP32         1
// #define GxB_NO_MAX_ISGE_FP64         1
// #define GxB_NO_MAX_ISGE_INT16        1
// #define GxB_NO_MAX_ISGE_INT32        1
// #define GxB_NO_MAX_ISGE_INT64        1
// #define GxB_NO_MAX_ISGE_INT8         1
// #define GxB_NO_MAX_ISGE_UINT16       1
// #define GxB_NO_MAX_ISGE_UINT32       1
// #define GxB_NO_MAX_ISGE_UINT64       1
// #define GxB_NO_MAX_ISGE_UINT8        1

// #define GxB_NO_MAX_ISGT_FP32         1
// #define GxB_NO_MAX_ISGT_FP64         1
// #define GxB_NO_MAX_ISGT_INT16        1
// #define GxB_NO_MAX_ISGT_INT32        1
// #define GxB_NO_MAX_ISGT_INT64        1
// #define GxB_NO_MAX_ISGT_INT8         1
// #define GxB_NO_MAX_ISGT_UINT16       1
// #define GxB_NO_MAX_ISGT_UINT32       1
// #define GxB_NO_MAX_ISGT_UINT64       1
// #define GxB_NO_MAX_ISGT_UINT8        1

// #define GxB_NO_MAX_ISLE_FP32         1
// #define GxB_NO_MAX_ISLE_FP64         1
// #define GxB_NO_MAX_ISLE_INT16        1
// #define GxB_NO_MAX_ISLE_INT32        1
// #define GxB_NO_MAX_ISLE_INT64        1
// #define GxB_NO_MAX_ISLE_INT8         1
// #define GxB_NO_MAX_ISLE_UINT16       1
// #define GxB_NO_MAX_ISLE_UINT32       1
// #define GxB_NO_MAX_ISLE_UINT64       1
// #define GxB_NO_MAX_ISLE_UINT8        1

// #define GxB_NO_MAX_ISLT_FP32         1
// #define GxB_NO_MAX_ISLT_FP64         1
// #define GxB_NO_MAX_ISLT_INT16        1
// #define GxB_NO_MAX_ISLT_INT32        1
// #define GxB_NO_MAX_ISLT_INT64        1
// #define GxB_NO_MAX_ISLT_INT8         1
// #define GxB_NO_MAX_ISLT_UINT16       1
// #define GxB_NO_MAX_ISLT_UINT32       1
// #define GxB_NO_MAX_ISLT_UINT64       1
// #define GxB_NO_MAX_ISLT_UINT8        1

// #define GxB_NO_MAX_ISNE_FP32         1
// #define GxB_NO_MAX_ISNE_FP64         1
// #define GxB_NO_MAX_ISNE_INT16        1
// #define GxB_NO_MAX_ISNE_INT32        1
// #define GxB_NO_MAX_ISNE_INT64        1
// #define GxB_NO_MAX_ISNE_INT8         1
// #define GxB_NO_MAX_ISNE_UINT16       1
// #define GxB_NO_MAX_ISNE_UINT32       1
// #define GxB_NO_MAX_ISNE_UINT64       1
// #define GxB_NO_MAX_ISNE_UINT8        1

// #define GxB_NO_MAX_LAND_FP32         1
// #define GxB_NO_MAX_LAND_FP64         1
// #define GxB_NO_MAX_LAND_INT16        1
// #define GxB_NO_MAX_LAND_INT32        1
// #define GxB_NO_MAX_LAND_INT64        1
// #define GxB_NO_MAX_LAND_INT8         1
// #define GxB_NO_MAX_LAND_UINT16       1
// #define GxB_NO_MAX_LAND_UINT32       1
// #define GxB_NO_MAX_LAND_UINT64       1
// #define GxB_NO_MAX_LAND_UINT8        1

// #define GxB_NO_MAX_LOR_FP32          1
// #define GxB_NO_MAX_LOR_FP64          1
// #define GxB_NO_MAX_LOR_INT16         1
// #define GxB_NO_MAX_LOR_INT32         1
// #define GxB_NO_MAX_LOR_INT64         1
// #define GxB_NO_MAX_LOR_INT8          1
// #define GxB_NO_MAX_LOR_UINT16        1
// #define GxB_NO_MAX_LOR_UINT32        1
// #define GxB_NO_MAX_LOR_UINT64        1
// #define GxB_NO_MAX_LOR_UINT8         1

// #define GxB_NO_MAX_LXOR_FP32         1
// #define GxB_NO_MAX_LXOR_FP64         1
// #define GxB_NO_MAX_LXOR_INT16        1
// #define GxB_NO_MAX_LXOR_INT32        1
// #define GxB_NO_MAX_LXOR_INT64        1
// #define GxB_NO_MAX_LXOR_INT8         1
// #define GxB_NO_MAX_LXOR_UINT16       1
// #define GxB_NO_MAX_LXOR_UINT32       1
// #define GxB_NO_MAX_LXOR_UINT64       1
// #define GxB_NO_MAX_LXOR_UINT8        1

// #define GxB_NO_MAX_MAX_FP32          1
// #define GxB_NO_MAX_MAX_FP64          1
// #define GxB_NO_MAX_MAX_INT16         1
// #define GxB_NO_MAX_MAX_INT32         1
// #define GxB_NO_MAX_MAX_INT64         1
// #define GxB_NO_MAX_MAX_INT8          1
// #define GxB_NO_MAX_MAX_UINT16        1
// #define GxB_NO_MAX_MAX_UINT32        1
// #define GxB_NO_MAX_MAX_UINT64        1
// #define GxB_NO_MAX_MAX_UINT8         1

// #define GxB_NO_MAX_MINUS_FP32        1
// #define GxB_NO_MAX_MINUS_FP64        1
// #define GxB_NO_MAX_MINUS_INT16       1
// #define GxB_NO_MAX_MINUS_INT32       1
// #define GxB_NO_MAX_MINUS_INT64       1
// #define GxB_NO_MAX_MINUS_INT8        1
// #define GxB_NO_MAX_MINUS_UINT16      1
// #define GxB_NO_MAX_MINUS_UINT32      1
// #define GxB_NO_MAX_MINUS_UINT64      1
// #define GxB_NO_MAX_MINUS_UINT8       1

// #define GxB_NO_MAX_MIN_FP32          1
// #define GxB_NO_MAX_MIN_FP64          1
// #define GxB_NO_MAX_MIN_INT16         1
// #define GxB_NO_MAX_MIN_INT32         1
// #define GxB_NO_MAX_MIN_INT64         1
// #define GxB_NO_MAX_MIN_INT8          1
// #define GxB_NO_MAX_MIN_UINT16        1
// #define GxB_NO_MAX_MIN_UINT32        1
// #define GxB_NO_MAX_MIN_UINT64        1
// #define GxB_NO_MAX_MIN_UINT8         1

// #define GxB_NO_MAX_PLUS_FP32         1
// #define GxB_NO_MAX_PLUS_FP64         1
// #define GxB_NO_MAX_PLUS_INT16        1
// #define GxB_NO_MAX_PLUS_INT32        1
// #define GxB_NO_MAX_PLUS_INT64        1
// #define GxB_NO_MAX_PLUS_INT8         1
// #define GxB_NO_MAX_PLUS_UINT16       1
// #define GxB_NO_MAX_PLUS_UINT32       1
// #define GxB_NO_MAX_PLUS_UINT64       1
// #define GxB_NO_MAX_PLUS_UINT8        1

// #define GxB_NO_MAX_RDIV_FP32         1
// #define GxB_NO_MAX_RDIV_FP64         1
// #define GxB_NO_MAX_RDIV_INT16        1
// #define GxB_NO_MAX_RDIV_INT32        1
// #define GxB_NO_MAX_RDIV_INT64        1
// #define GxB_NO_MAX_RDIV_INT8         1
// #define GxB_NO_MAX_RDIV_UINT16       1
// #define GxB_NO_MAX_RDIV_UINT32       1
// #define GxB_NO_MAX_RDIV_UINT64       1
// #define GxB_NO_MAX_RDIV_UINT8        1

// #define GxB_NO_MAX_RMINUS_FP32       1
// #define GxB_NO_MAX_RMINUS_FP64       1
// #define GxB_NO_MAX_RMINUS_INT16      1
// #define GxB_NO_MAX_RMINUS_INT32      1
// #define GxB_NO_MAX_RMINUS_INT64      1
// #define GxB_NO_MAX_RMINUS_INT8       1
// #define GxB_NO_MAX_RMINUS_UINT16     1
// #define GxB_NO_MAX_RMINUS_UINT32     1
// #define GxB_NO_MAX_RMINUS_UINT64     1
// #define GxB_NO_MAX_RMINUS_UINT8      1

// #define GxB_NO_MAX_SECOND_FP32       1
// #define GxB_NO_MAX_SECOND_FP64       1
// #define GxB_NO_MAX_SECOND_INT16      1
// #define GxB_NO_MAX_SECOND_INT32      1
// #define GxB_NO_MAX_SECOND_INT64      1
// #define GxB_NO_MAX_SECOND_INT8       1
// #define GxB_NO_MAX_SECOND_UINT16     1
// #define GxB_NO_MAX_SECOND_UINT32     1
// #define GxB_NO_MAX_SECOND_UINT64     1
// #define GxB_NO_MAX_SECOND_UINT8      1

// #define GxB_NO_MAX_TIMES_FP32        1
// #define GxB_NO_MAX_TIMES_FP64        1
// #define GxB_NO_MAX_TIMES_INT16       1
// #define GxB_NO_MAX_TIMES_INT32       1
// #define GxB_NO_MAX_TIMES_INT64       1
// #define GxB_NO_MAX_TIMES_INT8        1
// #define GxB_NO_MAX_TIMES_UINT16      1
// #define GxB_NO_MAX_TIMES_UINT32      1
// #define GxB_NO_MAX_TIMES_UINT64      1
// #define GxB_NO_MAX_TIMES_UINT8       1

// #define GxB_NO_MIN_DIV_FP32          1
// #define GxB_NO_MIN_DIV_FP64          1
// #define GxB_NO_MIN_DIV_INT16         1
// #define GxB_NO_MIN_DIV_INT32         1
// #define GxB_NO_MIN_DIV_INT64         1
// #define GxB_NO_MIN_DIV_INT8          1
// #define GxB_NO_MIN_DIV_UINT16        1
// #define GxB_NO_MIN_DIV_UINT32        1
// #define GxB_NO_MIN_DIV_UINT64        1
// #define GxB_NO_MIN_DIV_UINT8         1

// #define GxB_NO_MIN_FIRST_FP32        1
// #define GxB_NO_MIN_FIRST_FP64        1
// #define GxB_NO_MIN_FIRST_INT16       1
// #define GxB_NO_MIN_FIRST_INT32       1
// #define GxB_NO_MIN_FIRST_INT64       1
// #define GxB_NO_MIN_FIRST_INT8        1
// #define GxB_NO_MIN_FIRST_UINT16      1
// #define GxB_NO_MIN_FIRST_UINT32      1
// #define GxB_NO_MIN_FIRST_UINT64      1
// #define GxB_NO_MIN_FIRST_UINT8       1

// #define GxB_NO_MIN_ISEQ_FP32         1
// #define GxB_NO_MIN_ISEQ_FP64         1
// #define GxB_NO_MIN_ISEQ_INT16        1
// #define GxB_NO_MIN_ISEQ_INT32        1
// #define GxB_NO_MIN_ISEQ_INT64        1
// #define GxB_NO_MIN_ISEQ_INT8         1
// #define GxB_NO_MIN_ISEQ_UINT16       1
// #define GxB_NO_MIN_ISEQ_UINT32       1
// #define GxB_NO_MIN_ISEQ_UINT64       1
// #define GxB_NO_MIN_ISEQ_UINT8        1

// #define GxB_NO_MIN_ISGE_FP32         1
// #define GxB_NO_MIN_ISGE_FP64         1
// #define GxB_NO_MIN_ISGE_INT16        1
// #define GxB_NO_MIN_ISGE_INT32        1
// #define GxB_NO_MIN_ISGE_INT64        1
// #define GxB_NO_MIN_ISGE_INT8         1
// #define GxB_NO_MIN_ISGE_UINT16       1
// #define GxB_NO_MIN_ISGE_UINT32       1
// #define GxB_NO_MIN_ISGE_UINT64       1
// #define GxB_NO_MIN_ISGE_UINT8        1

// #define GxB_NO_MIN_ISGT_FP32         1
// #define GxB_NO_MIN_ISGT_FP64         1
// #define GxB_NO_MIN_ISGT_INT16        1
// #define GxB_NO_MIN_ISGT_INT32        1
// #define GxB_NO_MIN_ISGT_INT64        1
// #define GxB_NO_MIN_ISGT_INT8         1
// #define GxB_NO_MIN_ISGT_UINT16       1
// #define GxB_NO_MIN_ISGT_UINT32       1
// #define GxB_NO_MIN_ISGT_UINT64       1
// #define GxB_NO_MIN_ISGT_UINT8        1

// #define GxB_NO_MIN_ISLE_FP32         1
// #define GxB_NO_MIN_ISLE_FP64         1
// #define GxB_NO_MIN_ISLE_INT16        1
// #define GxB_NO_MIN_ISLE_INT32        1
// #define GxB_NO_MIN_ISLE_INT64        1
// #define GxB_NO_MIN_ISLE_INT8         1
// #define GxB_NO_MIN_ISLE_UINT16       1
// #define GxB_NO_MIN_ISLE_UINT32       1
// #define GxB_NO_MIN_ISLE_UINT64       1
// #define GxB_NO_MIN_ISLE_UINT8        1

// #define GxB_NO_MIN_ISLT_FP32         1
// #define GxB_NO_MIN_ISLT_FP64         1
// #define GxB_NO_MIN_ISLT_INT16        1
// #define GxB_NO_MIN_ISLT_INT32        1
// #define GxB_NO_MIN_ISLT_INT64        1
// #define GxB_NO_MIN_ISLT_INT8         1
// #define GxB_NO_MIN_ISLT_UINT16       1
// #define GxB_NO_MIN_ISLT_UINT32       1
// #define GxB_NO_MIN_ISLT_UINT64       1
// #define GxB_NO_MIN_ISLT_UINT8        1

// #define GxB_NO_MIN_ISNE_FP32         1
// #define GxB_NO_MIN_ISNE_FP64         1
// #define GxB_NO_MIN_ISNE_INT16        1
// #define GxB_NO_MIN_ISNE_INT32        1
// #define GxB_NO_MIN_ISNE_INT64        1
// #define GxB_NO_MIN_ISNE_INT8         1
// #define GxB_NO_MIN_ISNE_UINT16       1
// #define GxB_NO_MIN_ISNE_UINT32       1
// #define GxB_NO_MIN_ISNE_UINT64       1
// #define GxB_NO_MIN_ISNE_UINT8        1

// #define GxB_NO_MIN_LAND_FP32         1
// #define GxB_NO_MIN_LAND_FP64         1
// #define GxB_NO_MIN_LAND_INT16        1
// #define GxB_NO_MIN_LAND_INT32        1
// #define GxB_NO_MIN_LAND_INT64        1
// #define GxB_NO_MIN_LAND_INT8         1
// #define GxB_NO_MIN_LAND_UINT16       1
// #define GxB_NO_MIN_LAND_UINT32       1
// #define GxB_NO_MIN_LAND_UINT64       1
// #define GxB_NO_MIN_LAND_UINT8        1

// #define GxB_NO_MIN_LOR_FP32          1
// #define GxB_NO_MIN_LOR_FP64          1
// #define GxB_NO_MIN_LOR_INT16         1
// #define GxB_NO_MIN_LOR_INT32         1
// #define GxB_NO_MIN_LOR_INT64         1
// #define GxB_NO_MIN_LOR_INT8          1
// #define GxB_NO_MIN_LOR_UINT16        1
// #define GxB_NO_MIN_LOR_UINT32        1
// #define GxB_NO_MIN_LOR_UINT64        1
// #define GxB_NO_MIN_LOR_UINT8         1

// #define GxB_NO_MIN_LXOR_FP32         1
// #define GxB_NO_MIN_LXOR_FP64         1
// #define GxB_NO_MIN_LXOR_INT16        1
// #define GxB_NO_MIN_LXOR_INT32        1
// #define GxB_NO_MIN_LXOR_INT64        1
// #define GxB_NO_MIN_LXOR_INT8         1
// #define GxB_NO_MIN_LXOR_UINT16       1
// #define GxB_NO_MIN_LXOR_UINT32       1
// #define GxB_NO_MIN_LXOR_UINT64       1
// #define GxB_NO_MIN_LXOR_UINT8        1

// #define GxB_NO_MIN_MAX_FP32          1
// #define GxB_NO_MIN_MAX_FP64          1
// #define GxB_NO_MIN_MAX_INT16         1
// #define GxB_NO_MIN_MAX_INT32         1
// #define GxB_NO_MIN_MAX_INT64         1
// #define GxB_NO_MIN_MAX_INT8          1
// #define GxB_NO_MIN_MAX_UINT16        1
// #define GxB_NO_MIN_MAX_UINT32        1
// #define GxB_NO_MIN_MAX_UINT64        1
// #define GxB_NO_MIN_MAX_UINT8         1

// #define GxB_NO_MIN_MINUS_FP32        1
// #define GxB_NO_MIN_MINUS_FP64        1
// #define GxB_NO_MIN_MINUS_INT16       1
// #define GxB_NO_MIN_MINUS_INT32       1
// #define GxB_NO_MIN_MINUS_INT64       1
// #define GxB_NO_MIN_MINUS_INT8        1
// #define GxB_NO_MIN_MINUS_UINT16      1
// #define GxB_NO_MIN_MINUS_UINT32      1
// #define GxB_NO_MIN_MINUS_UINT64      1
// #define GxB_NO_MIN_MINUS_UINT8       1

// #define GxB_NO_MIN_MIN_FP32          1
// #define GxB_NO_MIN_MIN_FP64          1
// #define GxB_NO_MIN_MIN_INT16         1
// #define GxB_NO_MIN_MIN_INT32         1
// #define GxB_NO_MIN_MIN_INT64         1
// #define GxB_NO_MIN_MIN_INT8          1
// #define GxB_NO_MIN_MIN_UINT16        1
// #define GxB_NO_MIN_MIN_UINT32        1
// #define GxB_NO_MIN_MIN_UINT64        1
// #define GxB_NO_MIN_MIN_UINT8         1

// #define GxB_NO_MIN_PLUS_FP32         1
// #define GxB_NO_MIN_PLUS_FP64         1
// #define GxB_NO_MIN_PLUS_INT16        1
// #define GxB_NO_MIN_PLUS_INT32        1
// #define GxB_NO_MIN_PLUS_INT64        1
// #define GxB_NO_MIN_PLUS_INT8         1
// #define GxB_NO_MIN_PLUS_UINT16       1
// #define GxB_NO_MIN_PLUS_UINT32       1
// #define GxB_NO_MIN_PLUS_UINT64       1
// #define GxB_NO_MIN_PLUS_UINT8        1

// #define GxB_NO_MIN_RDIV_FP32         1
// #define GxB_NO_MIN_RDIV_FP64         1
// #define GxB_NO_MIN_RDIV_INT16        1
// #define GxB_NO_MIN_RDIV_INT32        1
// #define GxB_NO_MIN_RDIV_INT64        1
// #define GxB_NO_MIN_RDIV_INT8         1
// #define GxB_NO_MIN_RDIV_UINT16       1
// #define GxB_NO_MIN_RDIV_UINT32       1
// #define GxB_NO_MIN_RDIV_UINT64       1
// #define GxB_NO_MIN_RDIV_UINT8        1

// #define GxB_NO_MIN_RMINUS_FP32       1
// #define GxB_NO_MIN_RMINUS_FP64       1
// #define GxB_NO_MIN_RMINUS_INT16      1
// #define GxB_NO_MIN_RMINUS_INT32      1
// #define GxB_NO_MIN_RMINUS_INT64      1
// #define GxB_NO_MIN_RMINUS_INT8       1
// #define GxB_NO_MIN_RMINUS_UINT16     1
// #define GxB_NO_MIN_RMINUS_UINT32     1
// #define GxB_NO_MIN_RMINUS_UINT64     1
// #define GxB_NO_MIN_RMINUS_UINT8      1

// #define GxB_NO_MIN_SECOND_FP32       1
// #define GxB_NO_MIN_SECOND_FP64       1
// #define GxB_NO_MIN_SECOND_INT16      1
// #define GxB_NO_MIN_SECOND_INT32      1
// #define GxB_NO_MIN_SECOND_INT64      1
// #define GxB_NO_MIN_SECOND_INT8       1
// #define GxB_NO_MIN_SECOND_UINT16     1
// #define GxB_NO_MIN_SECOND_UINT32     1
// #define GxB_NO_MIN_SECOND_UINT64     1
// #define GxB_NO_MIN_SECOND_UINT8      1

// #define GxB_NO_MIN_TIMES_FP32        1
// #define GxB_NO_MIN_TIMES_FP64        1
// #define GxB_NO_MIN_TIMES_INT16       1
// #define GxB_NO_MIN_TIMES_INT32       1
// #define GxB_NO_MIN_TIMES_INT64       1
// #define GxB_NO_MIN_TIMES_INT8        1
// #define GxB_NO_MIN_TIMES_UINT16      1
// #define GxB_NO_MIN_TIMES_UINT32      1
// #define GxB_NO_MIN_TIMES_UINT64      1
// #define GxB_NO_MIN_TIMES_UINT8       1

// #define GxB_NO_PLUS_DIV_FP32         1
// #define GxB_NO_PLUS_DIV_FP64         1
// #define GxB_NO_PLUS_DIV_INT16        1
// #define GxB_NO_PLUS_DIV_INT32        1
// #define GxB_NO_PLUS_DIV_INT64        1
// #define GxB_NO_PLUS_DIV_INT8         1
// #define GxB_NO_PLUS_DIV_UINT16       1
// #define GxB_NO_PLUS_DIV_UINT32       1
// #define GxB_NO_PLUS_DIV_UINT64       1
// #define GxB_NO_PLUS_DIV_UINT8        1

// #define GxB_NO_PLUS_FIRST_FP32       1
// #define GxB_NO_PLUS_FIRST_FP64       1
// #define GxB_NO_PLUS_FIRST_INT16      1
// #define GxB_NO_PLUS_FIRST_INT32      1
// #define GxB_NO_PLUS_FIRST_INT64      1
// #define GxB_NO_PLUS_FIRST_INT8       1
// #define GxB_NO_PLUS_FIRST_UINT16     1
// #define GxB_NO_PLUS_FIRST_UINT32     1
// #define GxB_NO_PLUS_FIRST_UINT64     1
// #define GxB_NO_PLUS_FIRST_UINT8      1

// #define GxB_NO_PLUS_ISEQ_FP32        1
// #define GxB_NO_PLUS_ISEQ_FP64        1
// #define GxB_NO_PLUS_ISEQ_INT16       1
// #define GxB_NO_PLUS_ISEQ_INT32       1
// #define GxB_NO_PLUS_ISEQ_INT64       1
// #define GxB_NO_PLUS_ISEQ_INT8        1
// #define GxB_NO_PLUS_ISEQ_UINT16      1
// #define GxB_NO_PLUS_ISEQ_UINT32      1
// #define GxB_NO_PLUS_ISEQ_UINT64      1
// #define GxB_NO_PLUS_ISEQ_UINT8       1

// #define GxB_NO_PLUS_ISGE_FP32        1
// #define GxB_NO_PLUS_ISGE_FP64        1
// #define GxB_NO_PLUS_ISGE_INT16       1
// #define GxB_NO_PLUS_ISGE_INT32       1
// #define GxB_NO_PLUS_ISGE_INT64       1
// #define GxB_NO_PLUS_ISGE_INT8        1
// #define GxB_NO_PLUS_ISGE_UINT16      1
// #define GxB_NO_PLUS_ISGE_UINT32      1
// #define GxB_NO_PLUS_ISGE_UINT64      1
// #define GxB_NO_PLUS_ISGE_UINT8       1

// #define GxB_NO_PLUS_ISGT_FP32        1
// #define GxB_NO_PLUS_ISGT_FP64        1
// #define GxB_NO_PLUS_ISGT_INT16       1
// #define GxB_NO_PLUS_ISGT_INT32       1
// #define GxB_NO_PLUS_ISGT_INT64       1
// #define GxB_NO_PLUS_ISGT_INT8        1
// #define GxB_NO_PLUS_ISGT_UINT16      1
// #define GxB_NO_PLUS_ISGT_UINT32      1
// #define GxB_NO_PLUS_ISGT_UINT64      1
// #define GxB_NO_PLUS_ISGT_UINT8       1

// #define GxB_NO_PLUS_ISLE_FP32        1
// #define GxB_NO_PLUS_ISLE_FP64        1
// #define GxB_NO_PLUS_ISLE_INT16       1
// #define GxB_NO_PLUS_ISLE_INT32       1
// #define GxB_NO_PLUS_ISLE_INT64       1
// #define GxB_NO_PLUS_ISLE_INT8        1
// #define GxB_NO_PLUS_ISLE_UINT16      1
// #define GxB_NO_PLUS_ISLE_UINT32      1
// #define GxB_NO_PLUS_ISLE_UINT64      1
// #define GxB_NO_PLUS_ISLE_UINT8       1

// #define GxB_NO_PLUS_ISLT_FP32        1
// #define GxB_NO_PLUS_ISLT_FP64        1
// #define GxB_NO_PLUS_ISLT_INT16       1
// #define GxB_NO_PLUS_ISLT_INT32       1
// #define GxB_NO_PLUS_ISLT_INT64       1
// #define GxB_NO_PLUS_ISLT_INT8        1
// #define GxB_NO_PLUS_ISLT_UINT16      1
// #define GxB_NO_PLUS_ISLT_UINT32      1
// #define GxB_NO_PLUS_ISLT_UINT64      1
// #define GxB_NO_PLUS_ISLT_UINT8       1

// #define GxB_NO_PLUS_ISNE_FP32        1
// #define GxB_NO_PLUS_ISNE_FP64        1
// #define GxB_NO_PLUS_ISNE_INT16       1
// #define GxB_NO_PLUS_ISNE_INT32       1
// #define GxB_NO_PLUS_ISNE_INT64       1
// #define GxB_NO_PLUS_ISNE_INT8        1
// #define GxB_NO_PLUS_ISNE_UINT16      1
// #define GxB_NO_PLUS_ISNE_UINT32      1
// #define GxB_NO_PLUS_ISNE_UINT64      1
// #define GxB_NO_PLUS_ISNE_UINT8       1

// #define GxB_NO_PLUS_LAND_FP32        1
// #define GxB_NO_PLUS_LAND_FP64        1
// #define GxB_NO_PLUS_LAND_INT16       1
// #define GxB_NO_PLUS_LAND_INT32       1
// #define GxB_NO_PLUS_LAND_INT64       1
// #define GxB_NO_PLUS_LAND_INT8        1
// #define GxB_NO_PLUS_LAND_UINT16      1
// #define GxB_NO_PLUS_LAND_UINT32      1
// #define GxB_NO_PLUS_LAND_UINT64      1
// #define GxB_NO_PLUS_LAND_UINT8       1

// #define GxB_NO_PLUS_LOR_FP32         1
// #define GxB_NO_PLUS_LOR_FP64         1
// #define GxB_NO_PLUS_LOR_INT16        1
// #define GxB_NO_PLUS_LOR_INT32        1
// #define GxB_NO_PLUS_LOR_INT64        1
// #define GxB_NO_PLUS_LOR_INT8         1
// #define GxB_NO_PLUS_LOR_UINT16       1
// #define GxB_NO_PLUS_LOR_UINT32       1
// #define GxB_NO_PLUS_LOR_UINT64       1
// #define GxB_NO_PLUS_LOR_UINT8        1

// #define GxB_NO_PLUS_LXOR_FP32        1
// #define GxB_NO_PLUS_LXOR_FP64        1
// #define GxB_NO_PLUS_LXOR_INT16       1
// #define GxB_NO_PLUS_LXOR_INT32       1
// #define GxB_NO_PLUS_LXOR_INT64       1
// #define GxB_NO_PLUS_LXOR_INT8        1
// #define GxB_NO_PLUS_LXOR_UINT16      1
// #define GxB_NO_PLUS_LXOR_UINT32      1
// #define GxB_NO_PLUS_LXOR_UINT64      1
// #define GxB_NO_PLUS_LXOR_UINT8       1

// #define GxB_NO_PLUS_MAX_FP32         1
// #define GxB_NO_PLUS_MAX_FP64         1
// #define GxB_NO_PLUS_MAX_INT16        1
// #define GxB_NO_PLUS_MAX_INT32        1
// #define GxB_NO_PLUS_MAX_INT64        1
// #define GxB_NO_PLUS_MAX_INT8         1
// #define GxB_NO_PLUS_MAX_UINT16       1
// #define GxB_NO_PLUS_MAX_UINT32       1
// #define GxB_NO_PLUS_MAX_UINT64       1
// #define GxB_NO_PLUS_MAX_UINT8        1

// #define GxB_NO_PLUS_MINUS_FP32       1
// #define GxB_NO_PLUS_MINUS_FP64       1
// #define GxB_NO_PLUS_MINUS_INT16      1
// #define GxB_NO_PLUS_MINUS_INT32      1
// #define GxB_NO_PLUS_MINUS_INT64      1
// #define GxB_NO_PLUS_MINUS_INT8       1
// #define GxB_NO_PLUS_MINUS_UINT16     1
// #define GxB_NO_PLUS_MINUS_UINT32     1
// #define GxB_NO_PLUS_MINUS_UINT64     1
// #define GxB_NO_PLUS_MINUS_UINT8      1

// #define GxB_NO_PLUS_MIN_FP32         1
// #define GxB_NO_PLUS_MIN_FP64         1
// #define GxB_NO_PLUS_MIN_INT16        1
// #define GxB_NO_PLUS_MIN_INT32        1
// #define GxB_NO_PLUS_MIN_INT64        1
// #define GxB_NO_PLUS_MIN_INT8         1
// #define GxB_NO_PLUS_MIN_UINT16       1
// #define GxB_NO_PLUS_MIN_UINT32       1
// #define GxB_NO_PLUS_MIN_UINT64       1
// #define GxB_NO_PLUS_MIN_UINT8        1

// #define GxB_NO_PLUS_PLUS_FP32        1
// #define GxB_NO_PLUS_PLUS_FP64        1
// #define GxB_NO_PLUS_PLUS_INT16       1
// #define GxB_NO_PLUS_PLUS_INT32       1
// #define GxB_NO_PLUS_PLUS_INT64       1
// #define GxB_NO_PLUS_PLUS_INT8        1
// #define GxB_NO_PLUS_PLUS_UINT16      1
// #define GxB_NO_PLUS_PLUS_UINT32      1
// #define GxB_NO_PLUS_PLUS_UINT64      1
// #define GxB_NO_PLUS_PLUS_UINT8       1

// #define GxB_NO_PLUS_RDIV_FP32        1
// #define GxB_NO_PLUS_RDIV_FP64        1
// #define GxB_NO_PLUS_RDIV_INT16       1
// #define GxB_NO_PLUS_RDIV_INT32       1
// #define GxB_NO_PLUS_RDIV_INT64       1
// #define GxB_NO_PLUS_RDIV_INT8        1
// #define GxB_NO_PLUS_RDIV_UINT16      1
// #define GxB_NO_PLUS_RDIV_UINT32      1
// #define GxB_NO_PLUS_RDIV_UINT64      1
// #define GxB_NO_PLUS_RDIV_UINT8       1

// #define GxB_NO_PLUS_RMINUS_FP32      1
// #define GxB_NO_PLUS_RMINUS_FP64      1
// #define GxB_NO_PLUS_RMINUS_INT16     1
// #define GxB_NO_PLUS_RMINUS_INT32     1
// #define GxB_NO_PLUS_RMINUS_INT64     1
// #define GxB_NO_PLUS_RMINUS_INT8      1
// #define GxB_NO_PLUS_RMINUS_UINT16    1
// #define GxB_NO_PLUS_RMINUS_UINT32    1
// #define GxB_NO_PLUS_RMINUS_UINT64    1
// #define GxB_NO_PLUS_RMINUS_UINT8     1

// #define GxB_NO_PLUS_SECOND_FP32      1
// #define GxB_NO_PLUS_SECOND_FP64      1
// #define GxB_NO_PLUS_SECOND_INT16     1
// #define GxB_NO_PLUS_SECOND_INT32     1
// #define GxB_NO_PLUS_SECOND_INT64     1
// #define GxB_NO_PLUS_SECOND_INT8      1
// #define GxB_NO_PLUS_SECOND_UINT16    1
// #define GxB_NO_PLUS_SECOND_UINT32    1
// #define GxB_NO_PLUS_SECOND_UINT64    1
// #define GxB_NO_PLUS_SECOND_UINT8     1

// #define GxB_NO_PLUS_TIMES_FP32       1
// #define GxB_NO_PLUS_TIMES_FP64       1
// #define GxB_NO_PLUS_TIMES_INT16      1
// #define GxB_NO_PLUS_TIMES_INT32      1
// #define GxB_NO_PLUS_TIMES_INT64      1
// #define GxB_NO_PLUS_TIMES_INT8       1
// #define GxB_NO_PLUS_TIMES_UINT16     1
// #define GxB_NO_PLUS_TIMES_UINT32     1
// #define GxB_NO_PLUS_TIMES_UINT64     1
// #define GxB_NO_PLUS_TIMES_UINT8      1

// #define GxB_NO_TIMES_DIV_FP32        1
// #define GxB_NO_TIMES_DIV_FP64        1
// #define GxB_NO_TIMES_DIV_INT16       1
// #define GxB_NO_TIMES_DIV_INT32       1
// #define GxB_NO_TIMES_DIV_INT64       1
// #define GxB_NO_TIMES_DIV_INT8        1
// #define GxB_NO_TIMES_DIV_UINT16      1
// #define GxB_NO_TIMES_DIV_UINT32      1
// #define GxB_NO_TIMES_DIV_UINT64      1
// #define GxB_NO_TIMES_DIV_UINT8       1

// #define GxB_NO_TIMES_FIRST_FP32      1
// #define GxB_NO_TIMES_FIRST_FP64      1
// #define GxB_NO_TIMES_FIRST_INT16     1
// #define GxB_NO_TIMES_FIRST_INT32     1
// #define GxB_NO_TIMES_FIRST_INT64     1
// #define GxB_NO_TIMES_FIRST_INT8      1
// #define GxB_NO_TIMES_FIRST_UINT16    1
// #define GxB_NO_TIMES_FIRST_UINT32    1
// #define GxB_NO_TIMES_FIRST_UINT64    1
// #define GxB_NO_TIMES_FIRST_UINT8     1

// #define GxB_NO_TIMES_ISEQ_FP32       1
// #define GxB_NO_TIMES_ISEQ_FP64       1
// #define GxB_NO_TIMES_ISEQ_INT16      1
// #define GxB_NO_TIMES_ISEQ_INT32      1
// #define GxB_NO_TIMES_ISEQ_INT64      1
// #define GxB_NO_TIMES_ISEQ_INT8       1
// #define GxB_NO_TIMES_ISEQ_UINT16     1
// #define GxB_NO_TIMES_ISEQ_UINT32     1
// #define GxB_NO_TIMES_ISEQ_UINT64     1
// #define GxB_NO_TIMES_ISEQ_UINT8      1

// #define GxB_NO_TIMES_ISGE_FP32       1
// #define GxB_NO_TIMES_ISGE_FP64       1
// #define GxB_NO_TIMES_ISGE_INT16      1
// #define GxB_NO_TIMES_ISGE_INT32      1
// #define GxB_NO_TIMES_ISGE_INT64      1
// #define GxB_NO_TIMES_ISGE_INT8       1
// #define GxB_NO_TIMES_ISGE_UINT16     1
// #define GxB_NO_TIMES_ISGE_UINT32     1
// #define GxB_NO_TIMES_ISGE_UINT64     1
// #define GxB_NO_TIMES_ISGE_UINT8      1

// #define GxB_NO_TIMES_ISGT_FP32       1
// #define GxB_NO_TIMES_ISGT_FP64       1
// #define GxB_NO_TIMES_ISGT_INT16      1
// #define GxB_NO_TIMES_ISGT_INT32      1
// #define GxB_NO_TIMES_ISGT_INT64      1
// #define GxB_NO_TIMES_ISGT_INT8       1
// #define GxB_NO_TIMES_ISGT_UINT16     1
// #define GxB_NO_TIMES_ISGT_UINT32     1
// #define GxB_NO_TIMES_ISGT_UINT64     1
// #define GxB_NO_TIMES_ISGT_UINT8      1

// #define GxB_NO_TIMES_ISLE_FP32       1
// #define GxB_NO_TIMES_ISLE_FP64       1
// #define GxB_NO_TIMES_ISLE_INT16      1
// #define GxB_NO_TIMES_ISLE_INT32      1
// #define GxB_NO_TIMES_ISLE_INT64      1
// #define GxB_NO_TIMES_ISLE_INT8       1
// #define GxB_NO_TIMES_ISLE_UINT16     1
// #define GxB_NO_TIMES_ISLE_UINT32     1
// #define GxB_NO_TIMES_ISLE_UINT64     1
// #define GxB_NO_TIMES_ISLE_UINT8      1

// #define GxB_NO_TIMES_ISLT_FP32       1
// #define GxB_NO_TIMES_ISLT_FP64       1
// #define GxB_NO_TIMES_ISLT_INT16      1
// #define GxB_NO_TIMES_ISLT_INT32      1
// #define GxB_NO_TIMES_ISLT_INT64      1
// #define GxB_NO_TIMES_ISLT_INT8       1
// #define GxB_NO_TIMES_ISLT_UINT16     1
// #define GxB_NO_TIMES_ISLT_UINT32     1
// #define GxB_NO_TIMES_ISLT_UINT64     1
// #define GxB_NO_TIMES_ISLT_UINT8      1

// #define GxB_NO_TIMES_ISNE_FP32       1
// #define GxB_NO_TIMES_ISNE_FP64       1
// #define GxB_NO_TIMES_ISNE_INT16      1
// #define GxB_NO_TIMES_ISNE_INT32      1
// #define GxB_NO_TIMES_ISNE_INT64      1
// #define GxB_NO_TIMES_ISNE_INT8       1
// #define GxB_NO_TIMES_ISNE_UINT16     1
// #define GxB_NO_TIMES_ISNE_UINT32     1
// #define GxB_NO_TIMES_ISNE_UINT64     1
// #define GxB_NO_TIMES_ISNE_UINT8      1

// #define GxB_NO_TIMES_LAND_FP32       1
// #define GxB_NO_TIMES_LAND_FP64       1
// #define GxB_NO_TIMES_LAND_INT16      1
// #define GxB_NO_TIMES_LAND_INT32      1
// #define GxB_NO_TIMES_LAND_INT64      1
// #define GxB_NO_TIMES_LAND_INT8       1
// #define GxB_NO_TIMES_LAND_UINT16     1
// #define GxB_NO_TIMES_LAND_UINT32     1
// #define GxB_NO_TIMES_LAND_UINT64     1
// #define GxB_NO_TIMES_LAND_UINT8      1

// #define GxB_NO_TIMES_LOR_FP32        1
// #define GxB_NO_TIMES_LOR_FP64        1
// #define GxB_NO_TIMES_LOR_INT16       1
// #define GxB_NO_TIMES_LOR_INT32       1
// #define GxB_NO_TIMES_LOR_INT64       1
// #define GxB_NO_TIMES_LOR_INT8        1
// #define GxB_NO_TIMES_LOR_UINT16      1
// #define GxB_NO_TIMES_LOR_UINT32      1
// #define GxB_NO_TIMES_LOR_UINT64      1
// #define GxB_NO_TIMES_LOR_UINT8       1

// #define GxB_NO_TIMES_LXOR_FP32       1
// #define GxB_NO_TIMES_LXOR_FP64       1
// #define GxB_NO_TIMES_LXOR_INT16      1
// #define GxB_NO_TIMES_LXOR_INT32      1
// #define GxB_NO_TIMES_LXOR_INT64      1
// #define GxB_NO_TIMES_LXOR_INT8       1
// #define GxB_NO_TIMES_LXOR_UINT16     1
// #define GxB_NO_TIMES_LXOR_UINT32     1
// #define GxB_NO_TIMES_LXOR_UINT64     1
// #define GxB_NO_TIMES_LXOR_UINT8      1

// #define GxB_NO_TIMES_MAX_FP32        1
// #define GxB_NO_TIMES_MAX_FP64        1
// #define GxB_NO_TIMES_MAX_INT16       1
// #define GxB_NO_TIMES_MAX_INT32       1
// #define GxB_NO_TIMES_MAX_INT64       1
// #define GxB_NO_TIMES_MAX_INT8        1
// #define GxB_NO_TIMES_MAX_UINT16      1
// #define GxB_NO_TIMES_MAX_UINT32      1
// #define GxB_NO_TIMES_MAX_UINT64      1
// #define GxB_NO_TIMES_MAX_UINT8       1

// #define GxB_NO_TIMES_MINUS_FP32      1
// #define GxB_NO_TIMES_MINUS_FP64      1
// #define GxB_NO_TIMES_MINUS_INT16     1
// #define GxB_NO_TIMES_MINUS_INT32     1
// #define GxB_NO_TIMES_MINUS_INT64     1
// #define GxB_NO_TIMES_MINUS_INT8      1
// #define GxB_NO_TIMES_MINUS_UINT16    1
// #define GxB_NO_TIMES_MINUS_UINT32    1
// #define GxB_NO_TIMES_MINUS_UINT64    1
// #define GxB_NO_TIMES_MINUS_UINT8     1

// #define GxB_NO_TIMES_MIN_FP32        1
// #define GxB_NO_TIMES_MIN_FP64        1
// #define GxB_NO_TIMES_MIN_INT16       1
// #define GxB_NO_TIMES_MIN_INT32       1
// #define GxB_NO_TIMES_MIN_INT64       1
// #define GxB_NO_TIMES_MIN_INT8        1
// #define GxB_NO_TIMES_MIN_UINT16      1
// #define GxB_NO_TIMES_MIN_UINT32      1
// #define GxB_NO_TIMES_MIN_UINT64      1
// #define GxB_NO_TIMES_MIN_UINT8       1

// #define GxB_NO_TIMES_PLUS_FP32       1
// #define GxB_NO_TIMES_PLUS_FP64       1
// #define GxB_NO_TIMES_PLUS_INT16      1
// #define GxB_NO_TIMES_PLUS_INT32      1
// #define GxB_NO_TIMES_PLUS_INT64      1
// #define GxB_NO_TIMES_PLUS_INT8       1
// #define GxB_NO_TIMES_PLUS_UINT16     1
// #define GxB_NO_TIMES_PLUS_UINT32     1
// #define GxB_NO_TIMES_PLUS_UINT64     1
// #define GxB_NO_TIMES_PLUS_UINT8      1

// #define GxB_NO_TIMES_RDIV_FP32       1
// #define GxB_NO_TIMES_RDIV_FP64       1
// #define GxB_NO_TIMES_RDIV_INT16      1
// #define GxB_NO_TIMES_RDIV_INT32      1
// #define GxB_NO_TIMES_RDIV_INT64      1
// #define GxB_NO_TIMES_RDIV_INT8       1
// #define GxB_NO_TIMES_RDIV_UINT16     1
// #define GxB_NO_TIMES_RDIV_UINT32     1
// #define GxB_NO_TIMES_RDIV_UINT64     1
// #define GxB_NO_TIMES_RDIV_UINT8      1

// #define GxB_NO_TIMES_RMINUS_FP32     1
// #define GxB_NO_TIMES_RMINUS_FP64     1
// #define GxB_NO_TIMES_RMINUS_INT16    1
// #define GxB_NO_TIMES_RMINUS_INT32    1
// #define GxB_NO_TIMES_RMINUS_INT64    1
// #define GxB_NO_TIMES_RMINUS_INT8     1
// #define GxB_NO_TIMES_RMINUS_UINT16   1
// #define GxB_NO_TIMES_RMINUS_UINT32   1
// #define GxB_NO_TIMES_RMINUS_UINT64   1
// #define GxB_NO_TIMES_RMINUS_UINT8    1

// #define GxB_NO_TIMES_SECOND_FP32     1
// #define GxB_NO_TIMES_SECOND_FP64     1
// #define GxB_NO_TIMES_SECOND_INT16    1
// #define GxB_NO_TIMES_SECOND_INT32    1
// #define GxB_NO_TIMES_SECOND_INT64    1
// #define GxB_NO_TIMES_SECOND_INT8     1
// #define GxB_NO_TIMES_SECOND_UINT16   1
// #define GxB_NO_TIMES_SECOND_UINT32   1
// #define GxB_NO_TIMES_SECOND_UINT64   1
// #define GxB_NO_TIMES_SECOND_UINT8    1

// #define GxB_NO_TIMES_TIMES_FP32      1
// #define GxB_NO_TIMES_TIMES_FP64      1
// #define GxB_NO_TIMES_TIMES_INT16     1
// #define GxB_NO_TIMES_TIMES_INT32     1
// #define GxB_NO_TIMES_TIMES_INT64     1
// #define GxB_NO_TIMES_TIMES_INT8      1
// #define GxB_NO_TIMES_TIMES_UINT16    1
// #define GxB_NO_TIMES_TIMES_UINT32    1
// #define GxB_NO_TIMES_TIMES_UINT64    1
// #define GxB_NO_TIMES_TIMES_UINT8     1

