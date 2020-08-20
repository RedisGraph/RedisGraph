//------------------------------------------------------------------------------
// GB_semiring_template.c: built-in unary and binary functions and operators
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// This file is #include'd many times in GB_ops.c to define the built-in
// semirings.  That file has defined either GB_BOOLEAN, or GB_TYPE as one of
// the 10 non-boolean types.

// Using built-in types and operators, 1355 unique semirings can be built.
// This count excludes redundant Boolean operators (for example GrB_TIMES_BOOL
// and GrB_LAND are different operators but they are redundant since they
// always return the same result):

// 1000 semirings with a multiply operator TxT -> T where T is non-Boolean, from
// the complete cross product of:

//      5 add monoids: MIN, MAX, PLUS, TIMES, ANY
//      20 multiply operators:
//          FIRST, SECOND, PAIR, MIN, MAX, PLUS, MINUS, RMINUS, TIMES, DIV, RDIV
//          ISEQ, ISNE, ISGT, ISLT, ISGE, ISLE,
//          LOR, LAND, LXOR
//      10 non-Boolean types, T

//      a single instance of this file creates 100 semirings of this
//      form, of one type T, when T is not BOOL

// 300 semirings with a comparison operator TxT -> bool, where T is
// non-Boolean, from the complete cross product of:

//      5 Boolean add monoids: LAND, LOR, LXOR, EQ, ANY
//      6 multiply operators: EQ, NE, GT, LT, GE, LE
//      10 non-Boolean types, T

//      a single instance of this file creates 30 semirings of this form,
//      of one type T, when T is not BOOL

// 55 semirings with purely Boolean types, bool x bool -> bool, from the
// complete cross product of:

//      5 Boolean add monoids: LAND, LOR, LXOR, EQ, ANY
//      11 multiply operators:
//          FIRST, SECOND, PAIR, LOR, LAND, LXOR, EQ, GT, LT, GE, LE

//      a single instance of this file creates all 5*11 = 55 purely Boolean
//      semirings, when T is BOOL and GB_BOOLEAN is defined

//------------------------------------------------------------------------------

#ifdef GB_BOOLEAN

//------------------------------------------------------------------------------
// 55 purely Boolean semirings
//------------------------------------------------------------------------------

// All types in these 44 semirings are BOOL

// 11 semirings with LOR monoid; the 2nd argument is the multiply operator
GB_SEMIRING_DEFINE ( LOR   , GrB_, FIRST  )
GB_SEMIRING_DEFINE ( LOR   , GrB_, SECOND )
GB_SEMIRING_DEFINE ( LOR   , GxB_, PAIR   )
GB_SEMIRING_DEFINE ( LOR   , GxB_, LOR    )
GB_SEMIRING_DEFINE ( LOR   , GxB_, LAND   )
GB_SEMIRING_DEFINE ( LOR   , GxB_, LXOR   )
GB_SEMIRING_DEFINE ( LOR   , GrB_, EQ     )
GB_SEMIRING_DEFINE ( LOR   , GrB_, GT     )
GB_SEMIRING_DEFINE ( LOR   , GrB_, LT     )
GB_SEMIRING_DEFINE ( LOR   , GrB_, GE     )
GB_SEMIRING_DEFINE ( LOR   , GrB_, LE     )

// 11 semirings with LAND monoid; the 2nd argument is the multiply operator
GB_SEMIRING_DEFINE ( LAND  , GrB_, FIRST  )
GB_SEMIRING_DEFINE ( LAND  , GrB_, SECOND )
GB_SEMIRING_DEFINE ( LAND  , GxB_, PAIR   )
GB_SEMIRING_DEFINE ( LAND  , GxB_, LOR    )
GB_SEMIRING_DEFINE ( LAND  , GxB_, LAND   )
GB_SEMIRING_DEFINE ( LAND  , GxB_, LXOR   )
GB_SEMIRING_DEFINE ( LAND  , GrB_, EQ     )
GB_SEMIRING_DEFINE ( LAND  , GrB_, GT     )
GB_SEMIRING_DEFINE ( LAND  , GrB_, LT     )
GB_SEMIRING_DEFINE ( LAND  , GrB_, GE     )
GB_SEMIRING_DEFINE ( LAND  , GrB_, LE     )

// 11 semirings with LXOR monoid; the 2nd argument is the multiply operator
GB_SEMIRING_DEFINE ( LXOR , GrB_, FIRST  )
GB_SEMIRING_DEFINE ( LXOR , GrB_, SECOND )
GB_SEMIRING_DEFINE ( LXOR , GxB_, PAIR   )
GB_SEMIRING_DEFINE ( LXOR , GxB_, LOR    )
GB_SEMIRING_DEFINE ( LXOR , GxB_, LAND   )
GB_SEMIRING_DEFINE ( LXOR , GxB_, LXOR   )
GB_SEMIRING_DEFINE ( LXOR , GrB_, EQ     )
GB_SEMIRING_DEFINE ( LXOR , GrB_, GT     )
GB_SEMIRING_DEFINE ( LXOR , GrB_, LT     )
GB_SEMIRING_DEFINE ( LXOR , GrB_, GE     )
GB_SEMIRING_DEFINE ( LXOR , GrB_, LE     )

// 11 semirings with EQ monoid; the 2nd argument is the multiply operator
GB_SEMIRING_DEFINE ( EQ   , GrB_, FIRST  )
GB_SEMIRING_DEFINE ( EQ   , GrB_, SECOND )
GB_SEMIRING_DEFINE ( EQ   , GxB_, PAIR   )
GB_SEMIRING_DEFINE ( EQ   , GxB_, LOR    )
GB_SEMIRING_DEFINE ( EQ   , GxB_, LAND   )
GB_SEMIRING_DEFINE ( EQ   , GxB_, LXOR   )
GB_SEMIRING_DEFINE ( EQ   , GrB_, EQ     )
GB_SEMIRING_DEFINE ( EQ   , GrB_, GT     )
GB_SEMIRING_DEFINE ( EQ   , GrB_, LT     )
GB_SEMIRING_DEFINE ( EQ   , GrB_, GE     )
GB_SEMIRING_DEFINE ( EQ   , GrB_, LE     )

// 11 semirings with ANY monoid; the 2nd argument is the multiply operator
GB_SEMIRING_DEFINE ( ANY  , GrB_, FIRST  )
GB_SEMIRING_DEFINE ( ANY  , GrB_, SECOND )
GB_SEMIRING_DEFINE ( ANY  , GxB_, PAIR   )
GB_SEMIRING_DEFINE ( ANY  , GxB_, LOR    )
GB_SEMIRING_DEFINE ( ANY  , GxB_, LAND   )
GB_SEMIRING_DEFINE ( ANY  , GxB_, LXOR   )
GB_SEMIRING_DEFINE ( ANY  , GrB_, EQ     )
GB_SEMIRING_DEFINE ( ANY  , GrB_, GT     )
GB_SEMIRING_DEFINE ( ANY  , GrB_, LT     )
GB_SEMIRING_DEFINE ( ANY  , GrB_, GE     )
GB_SEMIRING_DEFINE ( ANY  , GrB_, LE     )

#else

//------------------------------------------------------------------------------
// 100 semirings of the form TxT->T
//------------------------------------------------------------------------------

// All types in these semirings are the same.  These are defined for
// the 10 non-Boolean types, not when T is BOOL.

// 20 semirings with MIN monoid; the 2nd argument is the multiply operator
GB_SEMIRING_DEFINE ( MIN   , GrB_, FIRST  )
GB_SEMIRING_DEFINE ( MIN   , GrB_, SECOND )
GB_SEMIRING_DEFINE ( MIN   , GxB_, PAIR   )
GB_SEMIRING_DEFINE ( MIN   , GrB_, MIN    )
GB_SEMIRING_DEFINE ( MIN   , GrB_, MAX    )
GB_SEMIRING_DEFINE ( MIN   , GrB_, PLUS   )
GB_SEMIRING_DEFINE ( MIN   , GrB_, MINUS  )
GB_SEMIRING_DEFINE ( MIN   , GxB_, RMINUS )
GB_SEMIRING_DEFINE ( MIN   , GrB_, TIMES  )
GB_SEMIRING_DEFINE ( MIN   , GrB_, DIV    )
GB_SEMIRING_DEFINE ( MIN   , GxB_, RDIV   )
GB_SEMIRING_DEFINE ( MIN   , GxB_, ISEQ   )
GB_SEMIRING_DEFINE ( MIN   , GxB_, ISNE   )
GB_SEMIRING_DEFINE ( MIN   , GxB_, ISGT   )
GB_SEMIRING_DEFINE ( MIN   , GxB_, ISLT   )
GB_SEMIRING_DEFINE ( MIN   , GxB_, ISGE   )
GB_SEMIRING_DEFINE ( MIN   , GxB_, ISLE   )
GB_SEMIRING_DEFINE ( MIN   , GxB_, LOR    )
GB_SEMIRING_DEFINE ( MIN   , GxB_, LAND   )
GB_SEMIRING_DEFINE ( MIN   , GxB_, LXOR   )

// 20 semirings with MAX monoid; the 2nd argument is the multiply operator
GB_SEMIRING_DEFINE ( MAX   , GrB_, FIRST  )
GB_SEMIRING_DEFINE ( MAX   , GrB_, SECOND )
GB_SEMIRING_DEFINE ( MAX   , GxB_, PAIR   )
GB_SEMIRING_DEFINE ( MAX   , GrB_, MIN    )
GB_SEMIRING_DEFINE ( MAX   , GrB_, MAX    )
GB_SEMIRING_DEFINE ( MAX   , GrB_, PLUS   )
GB_SEMIRING_DEFINE ( MAX   , GrB_, MINUS  )
GB_SEMIRING_DEFINE ( MAX   , GxB_, RMINUS )
GB_SEMIRING_DEFINE ( MAX   , GrB_, TIMES  )
GB_SEMIRING_DEFINE ( MAX   , GrB_, DIV    )
GB_SEMIRING_DEFINE ( MAX   , GxB_, RDIV   )
GB_SEMIRING_DEFINE ( MAX   , GxB_, ISEQ   )
GB_SEMIRING_DEFINE ( MAX   , GxB_, ISNE   )
GB_SEMIRING_DEFINE ( MAX   , GxB_, ISGT   )
GB_SEMIRING_DEFINE ( MAX   , GxB_, ISLT   )
GB_SEMIRING_DEFINE ( MAX   , GxB_, ISGE   )
GB_SEMIRING_DEFINE ( MAX   , GxB_, ISLE   )
GB_SEMIRING_DEFINE ( MAX   , GxB_, LOR    )
GB_SEMIRING_DEFINE ( MAX   , GxB_, LAND   )
GB_SEMIRING_DEFINE ( MAX   , GxB_, LXOR   )

// 20 semirings with PLUS monoid; the 2nd argument is the multiply operator
GB_SEMIRING_DEFINE ( PLUS  , GrB_, FIRST  )
GB_SEMIRING_DEFINE ( PLUS  , GrB_, SECOND )
GB_SEMIRING_DEFINE ( PLUS  , GxB_, PAIR   )
GB_SEMIRING_DEFINE ( PLUS  , GrB_, MIN    )
GB_SEMIRING_DEFINE ( PLUS  , GrB_, MAX    )
GB_SEMIRING_DEFINE ( PLUS  , GrB_, PLUS   )
GB_SEMIRING_DEFINE ( PLUS  , GrB_, MINUS  )
GB_SEMIRING_DEFINE ( PLUS  , GxB_, RMINUS )
GB_SEMIRING_DEFINE ( PLUS  , GrB_, TIMES  )
GB_SEMIRING_DEFINE ( PLUS  , GrB_, DIV    )
GB_SEMIRING_DEFINE ( PLUS  , GxB_, RDIV   )
GB_SEMIRING_DEFINE ( PLUS  , GxB_, ISEQ   )
GB_SEMIRING_DEFINE ( PLUS  , GxB_, ISNE   )
GB_SEMIRING_DEFINE ( PLUS  , GxB_, ISGT   )
GB_SEMIRING_DEFINE ( PLUS  , GxB_, ISLT   )
GB_SEMIRING_DEFINE ( PLUS  , GxB_, ISGE   )
GB_SEMIRING_DEFINE ( PLUS  , GxB_, ISLE   )
GB_SEMIRING_DEFINE ( PLUS  , GxB_, LOR    )
GB_SEMIRING_DEFINE ( PLUS  , GxB_, LAND   )
GB_SEMIRING_DEFINE ( PLUS  , GxB_, LXOR   )

// 20 semirings with TIMES monoid; the 2nd argument is the multiply operator
GB_SEMIRING_DEFINE ( TIMES , GrB_, FIRST  )
GB_SEMIRING_DEFINE ( TIMES , GrB_, SECOND )
GB_SEMIRING_DEFINE ( TIMES , GxB_, PAIR   )
GB_SEMIRING_DEFINE ( TIMES , GrB_, MIN    )
GB_SEMIRING_DEFINE ( TIMES , GrB_, MAX    )
GB_SEMIRING_DEFINE ( TIMES , GrB_, PLUS   )
GB_SEMIRING_DEFINE ( TIMES , GrB_, MINUS  )
GB_SEMIRING_DEFINE ( TIMES , GxB_, RMINUS )
GB_SEMIRING_DEFINE ( TIMES , GrB_, TIMES  )
GB_SEMIRING_DEFINE ( TIMES , GrB_, DIV    )
GB_SEMIRING_DEFINE ( TIMES , GxB_, RDIV   )
GB_SEMIRING_DEFINE ( TIMES , GxB_, ISEQ   )
GB_SEMIRING_DEFINE ( TIMES , GxB_, ISNE   )
GB_SEMIRING_DEFINE ( TIMES , GxB_, ISGT   )
GB_SEMIRING_DEFINE ( TIMES , GxB_, ISLT   )
GB_SEMIRING_DEFINE ( TIMES , GxB_, ISGE   )
GB_SEMIRING_DEFINE ( TIMES , GxB_, ISLE   )
GB_SEMIRING_DEFINE ( TIMES , GxB_, LOR    )
GB_SEMIRING_DEFINE ( TIMES , GxB_, LAND   )
GB_SEMIRING_DEFINE ( TIMES , GxB_, LXOR   )

// 20 semirings with ANY monoid; the 2nd argument is the multiply operator
GB_SEMIRING_DEFINE ( ANY   , GrB_, FIRST  )
GB_SEMIRING_DEFINE ( ANY   , GrB_, SECOND )
GB_SEMIRING_DEFINE ( ANY   , GxB_, PAIR   )
GB_SEMIRING_DEFINE ( ANY   , GrB_, MIN    )
GB_SEMIRING_DEFINE ( ANY   , GrB_, MAX    )
GB_SEMIRING_DEFINE ( ANY   , GrB_, PLUS   )
GB_SEMIRING_DEFINE ( ANY   , GrB_, MINUS  )
GB_SEMIRING_DEFINE ( ANY   , GxB_, RMINUS )
GB_SEMIRING_DEFINE ( ANY   , GrB_, TIMES  )
GB_SEMIRING_DEFINE ( ANY   , GrB_, DIV    )
GB_SEMIRING_DEFINE ( ANY   , GxB_, RDIV   )
GB_SEMIRING_DEFINE ( ANY   , GxB_, ISEQ   )
GB_SEMIRING_DEFINE ( ANY   , GxB_, ISNE   )
GB_SEMIRING_DEFINE ( ANY   , GxB_, ISGT   )
GB_SEMIRING_DEFINE ( ANY   , GxB_, ISLT   )
GB_SEMIRING_DEFINE ( ANY   , GxB_, ISGE   )
GB_SEMIRING_DEFINE ( ANY   , GxB_, ISLE   )
GB_SEMIRING_DEFINE ( ANY   , GxB_, LOR    )
GB_SEMIRING_DEFINE ( ANY   , GxB_, LAND   )
GB_SEMIRING_DEFINE ( ANY   , GxB_, LXOR   )

//------------------------------------------------------------------------------
// 30 semirings of the form TxT->bool
//------------------------------------------------------------------------------

// The multiply operator has the form z=compare(x,y), where x and y are of
// type T, and z is Boolean.  These operators are combined with the four
// Boolean monoids.

// These are defined when T is one of the 10 non-Boolean types, not when T is
// BOOL

// 6 semrings with LOR monoid; the 2nd argument is the comparison operator
GB_SEMIRING_COMPARE_DEFINE ( LOR  , EQ )
GB_SEMIRING_COMPARE_DEFINE ( LOR  , NE )
GB_SEMIRING_COMPARE_DEFINE ( LOR  , GT )
GB_SEMIRING_COMPARE_DEFINE ( LOR  , LT )
GB_SEMIRING_COMPARE_DEFINE ( LOR  , GE )
GB_SEMIRING_COMPARE_DEFINE ( LOR  , LE )

// 6 semrings with LAND monoid; the 2nd argument is the comparison operator
GB_SEMIRING_COMPARE_DEFINE ( LAND , EQ )
GB_SEMIRING_COMPARE_DEFINE ( LAND , NE )
GB_SEMIRING_COMPARE_DEFINE ( LAND , GT )
GB_SEMIRING_COMPARE_DEFINE ( LAND , LT )
GB_SEMIRING_COMPARE_DEFINE ( LAND , GE )
GB_SEMIRING_COMPARE_DEFINE ( LAND , LE )

// 6 semrings with LXOR monoid; the 2nd argument is the comparison operator
GB_SEMIRING_COMPARE_DEFINE ( LXOR , EQ )
GB_SEMIRING_COMPARE_DEFINE ( LXOR , NE )
GB_SEMIRING_COMPARE_DEFINE ( LXOR , GT )
GB_SEMIRING_COMPARE_DEFINE ( LXOR , LT )
GB_SEMIRING_COMPARE_DEFINE ( LXOR , GE )
GB_SEMIRING_COMPARE_DEFINE ( LXOR , LE )

// 6 semrings with EQ monoid; the 2nd argument is the comparison operator
GB_SEMIRING_COMPARE_DEFINE ( EQ   , EQ )
GB_SEMIRING_COMPARE_DEFINE ( EQ   , NE )
GB_SEMIRING_COMPARE_DEFINE ( EQ   , GT )
GB_SEMIRING_COMPARE_DEFINE ( EQ   , LT )
GB_SEMIRING_COMPARE_DEFINE ( EQ   , GE )
GB_SEMIRING_COMPARE_DEFINE ( EQ   , LE )

// 6 semrings with ANY monoid; the 2nd argument is the comparison operator
GB_SEMIRING_COMPARE_DEFINE ( ANY  , EQ )
GB_SEMIRING_COMPARE_DEFINE ( ANY  , NE )
GB_SEMIRING_COMPARE_DEFINE ( ANY  , GT )
GB_SEMIRING_COMPARE_DEFINE ( ANY  , LT )
GB_SEMIRING_COMPARE_DEFINE ( ANY  , GE )
GB_SEMIRING_COMPARE_DEFINE ( ANY  , LE )

#endif

#undef GB
#undef GB_MONOID
#undef GxB_NAME
#undef GB_BOOLEAN

