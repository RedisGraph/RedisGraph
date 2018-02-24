//------------------------------------------------------------------------------
// GB_semiring_template.c: built-in unary and binary functions and operators
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// This file is #include'd many times in GB_builtin.c to define the built-in
// semirings.  That file has #define'd either BOOLEAN, or TYPE as one of the
// 10 non-boolean types.

// Using built-in types and operators, 960 unique semirings can be built.  This
// count excludes redundant Boolean operators (for example GrB_TIMES_BOOL and
// GrB_LAND are different operators but they are redundant since they always
// return the same result):

// 680 semirings with a multiply operator TxT -> T where T is non-Boolean, from
// the complete cross product of:

//      4 add monoids (MIN, MAX, PLUS, TIMES)
//      17 multiply operators:
//          (FIRST, SECOND, MIN, MAX, PLUS, MINUS, TIMES, DIV,
//           ISEQ, ISNE, ISGT, ISLT, ISGE, ISLE,
//           LOR, LAND, LXOR)
//      10 non-Boolean types, T

//      a single instance of this file creates 4*17 = 68 semirings of this
//      form, of one type T, when T is not BOOL

// 240 semirings with a comparison operator TxT -> bool, where T is
// non-Boolean, from the complete cross product of:

//      4 Boolean add monoids: (LAND, LOR, LXOR, EQ)
//      6 multiply operators: (EQ, NE, GT, LT, GE, LE)
//      10 non-Boolean types, T

//      a single instance of this file creates 4*6 = 24 semirings of this form,
//      of one type T, when T is not BOOL

// 40 semirings with purely Boolean types, bool x bool -> bool, from the
// complete cross product of:

//      4 Boolean add monoids (LAND, LOR, LXOR, EQ)
//      10 multiply operators:
//          (FIRST, SECOND, LOR, LAND, LXOR, EQ, GT, LT, GE, LE)

//      a single instance of this file creates all 4*10 = 40 purely Boolean
//      semirings, when T is BOOL and BOOLEAN is #define'd.

//------------------------------------------------------------------------------

#ifdef BOOLEAN

//------------------------------------------------------------------------------
// 40 purely Boolean semirings
//------------------------------------------------------------------------------

// All types in these 40 semirings are BOOL

// 10 semirings with LOR monoid; the 2nd argument is the multiply operator
SEMIRING ( LOR   , FIRST  ) ;
SEMIRING ( LOR   , SECOND ) ;
SEMIRING ( LOR   , LOR    ) ;
SEMIRING ( LOR   , LAND   ) ;
SEMIRING ( LOR   , LXOR   ) ;
SEMIRING ( LOR   , EQ     ) ;
SEMIRING ( LOR   , NE     ) ;
SEMIRING ( LOR   , GT     ) ;
SEMIRING ( LOR   , LT     ) ;
SEMIRING ( LOR   , GE     ) ;
SEMIRING ( LOR   , LE     ) ;

// 10 semirings with LAND monoid; the 2nd argument is the multiply operator
SEMIRING ( LAND  , FIRST  ) ;
SEMIRING ( LAND  , SECOND ) ;
SEMIRING ( LAND  , LOR    ) ;
SEMIRING ( LAND  , LAND   ) ;
SEMIRING ( LAND  , LXOR   ) ;
SEMIRING ( LAND  , EQ     ) ;
SEMIRING ( LAND  , NE     ) ;
SEMIRING ( LAND  , GT     ) ;
SEMIRING ( LAND  , LT     ) ;
SEMIRING ( LAND  , GE     ) ;
SEMIRING ( LAND  , LE     ) ;

// 10 semirings with LXOR monoid; the 2nd argument is the multiply operator
SEMIRING ( LXOR , FIRST  ) ;
SEMIRING ( LXOR , SECOND ) ;
SEMIRING ( LXOR , LOR    ) ;
SEMIRING ( LXOR , LAND   ) ;
SEMIRING ( LXOR , LXOR   ) ;
SEMIRING ( LXOR , EQ     ) ;
SEMIRING ( LXOR , NE     ) ;
SEMIRING ( LXOR , GT     ) ;
SEMIRING ( LXOR , LT     ) ;
SEMIRING ( LXOR , GE     ) ;
SEMIRING ( LXOR , LE     ) ;

// 10 semirings with EQ monoid; the 2nd argument is the multiply operator
SEMIRING ( EQ   , FIRST  ) ;
SEMIRING ( EQ   , SECOND ) ;
SEMIRING ( EQ   , LOR    ) ;
SEMIRING ( EQ   , LAND   ) ;
SEMIRING ( EQ   , LXOR   ) ;
SEMIRING ( EQ   , EQ     ) ;
SEMIRING ( EQ   , NE     ) ;
SEMIRING ( EQ   , GT     ) ;
SEMIRING ( EQ   , LT     ) ;
SEMIRING ( EQ   , GE     ) ;
SEMIRING ( EQ   , LE     ) ;

#else

//------------------------------------------------------------------------------
// 68 semirings of the form TxT->T
//------------------------------------------------------------------------------

// All types in these 68 semirings are the same.  These are defined for
// the 10 non-Boolean types, not when T is BOOL.

// 17 semirings with MIN monoid; the 2nd argument is the multiply operator
SEMIRING ( MIN   , FIRST  ) ;
SEMIRING ( MIN   , SECOND ) ;
SEMIRING ( MIN   , MIN    ) ;
SEMIRING ( MIN   , MAX    ) ;
SEMIRING ( MIN   , PLUS   ) ;
SEMIRING ( MIN   , MINUS  ) ;
SEMIRING ( MIN   , TIMES  ) ;
SEMIRING ( MIN   , DIV    ) ;
SEMIRING ( MIN   , ISEQ   ) ;
SEMIRING ( MIN   , ISNE   ) ;
SEMIRING ( MIN   , ISGT   ) ;
SEMIRING ( MIN   , ISLT   ) ;
SEMIRING ( MIN   , ISGE   ) ;
SEMIRING ( MIN   , ISLE   ) ;
SEMIRING ( MIN   , LOR    ) ;
SEMIRING ( MIN   , LAND   ) ;
SEMIRING ( MIN   , LXOR   ) ;

// 17 semirings with MAX monoid; the 2nd argument is the multiply operator
SEMIRING ( MAX   , FIRST  ) ;
SEMIRING ( MAX   , SECOND ) ;
SEMIRING ( MAX   , MIN    ) ;
SEMIRING ( MAX   , MAX    ) ;
SEMIRING ( MAX   , PLUS   ) ;
SEMIRING ( MAX   , MINUS  ) ;
SEMIRING ( MAX   , TIMES  ) ;
SEMIRING ( MAX   , DIV    ) ;
SEMIRING ( MAX   , ISEQ   ) ;
SEMIRING ( MAX   , ISNE   ) ;
SEMIRING ( MAX   , ISGT   ) ;
SEMIRING ( MAX   , ISLT   ) ;
SEMIRING ( MAX   , ISGE   ) ;
SEMIRING ( MAX   , ISLE   ) ;
SEMIRING ( MAX   , LOR    ) ;
SEMIRING ( MAX   , LAND   ) ;
SEMIRING ( MAX   , LXOR   ) ;

// 17 semirings with PLUS monoid; the 2nd argument is the multiply operator
SEMIRING ( PLUS  , FIRST  ) ;
SEMIRING ( PLUS  , SECOND ) ;
SEMIRING ( PLUS  , MIN    ) ;
SEMIRING ( PLUS  , MAX    ) ;
SEMIRING ( PLUS  , PLUS   ) ;
SEMIRING ( PLUS  , MINUS  ) ;
SEMIRING ( PLUS  , TIMES  ) ;
SEMIRING ( PLUS  , DIV    ) ;
SEMIRING ( PLUS  , ISEQ   ) ;
SEMIRING ( PLUS  , ISNE   ) ;
SEMIRING ( PLUS  , ISGT   ) ;
SEMIRING ( PLUS  , ISLT   ) ;
SEMIRING ( PLUS  , ISGE   ) ;
SEMIRING ( PLUS  , ISLE   ) ;
SEMIRING ( PLUS  , LOR    ) ;
SEMIRING ( PLUS  , LAND   ) ;
SEMIRING ( PLUS  , LXOR   ) ;

// 17 semirings with TIMES monoid; the 2nd argument is the multiply operator
SEMIRING ( TIMES , FIRST  ) ;
SEMIRING ( TIMES , SECOND ) ;
SEMIRING ( TIMES , MIN    ) ;
SEMIRING ( TIMES , MAX    ) ;
SEMIRING ( TIMES , PLUS   ) ;
SEMIRING ( TIMES , MINUS  ) ;
SEMIRING ( TIMES , TIMES  ) ;
SEMIRING ( TIMES , DIV    ) ;
SEMIRING ( TIMES , ISEQ   ) ;
SEMIRING ( TIMES , ISNE   ) ;
SEMIRING ( TIMES , ISGT   ) ;
SEMIRING ( TIMES , ISLT   ) ;
SEMIRING ( TIMES , ISGE   ) ;
SEMIRING ( TIMES , ISLE   ) ;
SEMIRING ( TIMES , LOR    ) ;
SEMIRING ( TIMES , LAND   ) ;
SEMIRING ( TIMES , LXOR   ) ;

//------------------------------------------------------------------------------
// 24 semirings of the form TxT->bool
//------------------------------------------------------------------------------

// The multiply operator has the form z=compare(x,y), where x and y are of
// type T, and z is Boolean.  These operators are combined with the four
// Boolean monoids.

// These are defined when T is one of the 10 non-Boolean types, not when T is
// BOOL

// 6 semrings with LOR monoid; the 2nd argument is the comparison operator
SEMIRING_COMPARE ( LOR  , EQ ) ;
SEMIRING_COMPARE ( LOR  , NE ) ;
SEMIRING_COMPARE ( LOR  , GT ) ;
SEMIRING_COMPARE ( LOR  , LT ) ;
SEMIRING_COMPARE ( LOR  , GE ) ;
SEMIRING_COMPARE ( LOR  , LE ) ;

// 6 semrings with LAND monoid; the 2nd argument is the comparison operator
SEMIRING_COMPARE ( LAND , EQ ) ;
SEMIRING_COMPARE ( LAND , NE ) ;
SEMIRING_COMPARE ( LAND , GT ) ;
SEMIRING_COMPARE ( LAND , LT ) ;
SEMIRING_COMPARE ( LAND , GE ) ;
SEMIRING_COMPARE ( LAND , LE ) ;

// 6 semrings with LXOR monoid; the 2nd argument is the comparison operator
SEMIRING_COMPARE ( LXOR , EQ ) ;
SEMIRING_COMPARE ( LXOR , NE ) ;
SEMIRING_COMPARE ( LXOR , GT ) ;
SEMIRING_COMPARE ( LXOR , LT ) ;
SEMIRING_COMPARE ( LXOR , GE ) ;
SEMIRING_COMPARE ( LXOR , LE ) ;

// 6 semrings with EQ monoid; the 2nd argument is the comparison operator
SEMIRING_COMPARE ( EQ   , EQ ) ;
SEMIRING_COMPARE ( EQ   , NE ) ;
SEMIRING_COMPARE ( EQ   , GT ) ;
SEMIRING_COMPARE ( EQ   , LT ) ;
SEMIRING_COMPARE ( EQ   , GE ) ;
SEMIRING_COMPARE ( EQ   , LE ) ;

#endif

#undef GB
#undef GM
#undef GRB
#undef BOOLEAN

