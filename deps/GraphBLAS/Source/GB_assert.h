//------------------------------------------------------------------------------
// GB_assert.h: assertions
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#ifndef GB_ASSERT_H
#define GB_ASSERT_H

//------------------------------------------------------------------------------
// debugging definitions
//------------------------------------------------------------------------------

#undef ASSERT
#undef ASSERT_OK
#undef ASSERT_OK_OR_NULL

#ifdef GB_DEBUG

    // assert X is true
    #define ASSERT(X)                                                       \
    {                                                                       \
        if (!(X))                                                           \
        {                                                                   \
            GBDUMP ("assertion failed: " __FILE__ " line %d\n", __LINE__) ; \
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

#else

    // debugging disabled
    #define ASSERT(X)
    #define ASSERT_OK(X)
    #define ASSERT_OK_OR_NULL(X)

#endif

#define GB_IMPLIES(p,q) (!(p) || (q))

// for finding tests that trigger statement coverage.  If running a test
// in GraphBLAS/Tcov, the test does not terminate.
#if 1
#ifdef GBCOVER
#define GB_GOTCHA                                                   \
{                                                                   \
    fprintf (stderr, "Gotcha: " __FILE__ " line: %d\n", __LINE__) ; \
    GBDUMP ("Gotcha: " __FILE__ " line: %d\n", __LINE__) ;          \
}
#else
#define GB_GOTCHA                                                   \
{                                                                   \
    fprintf (stderr, "gotcha: " __FILE__ " line: %d\n", __LINE__) ; \
    GBDUMP ("gotcha: " __FILE__ " line: %d\n", __LINE__) ;          \
    GB_Global_abort_function ( ) ;                                  \
}
#endif
#endif

#ifndef GB_GOTCHA
#define GB_GOTCHA
#endif

#define GB_HERE GBDUMP ("%2d: Here: " __FILE__ "\n", __LINE__) ;

// ASSERT (GB_DEAD_CODE) marks code that is intentionally dead, leftover from
// prior versions of SuiteSparse:GraphBLAS but no longer used in the current
// version.  The code is kept in case it is required for future versions (in
// which case, the ASSERT (GB_DEAD_CODE) statement would be removed).
#define GB_DEAD_CODE 0

//------------------------------------------------------------------------------
// assertions for checking specific objects
//------------------------------------------------------------------------------

#define ASSERT_TYPE_OK(t,name,pr)  \
    ASSERT_OK (GB_Type_check (t, name, pr, NULL))

#define ASSERT_TYPE_OK_OR_NULL(t,name,pr)  \
    ASSERT_OK_OR_NULL (GB_Type_check (t, name, pr, NULL))

#define ASSERT_BINARYOP_OK(op,name,pr)  \
    ASSERT_OK (GB_BinaryOp_check (op, name, pr, NULL))

#define ASSERT_BINARYOP_OK_OR_NULL(op,name,pr)  \
    ASSERT_OK_OR_NULL (GB_BinaryOp_check (op, name, pr, NULL))

#define ASSERT_UNARYOP_OK(op,name,pr)  \
    ASSERT_OK (GB_UnaryOp_check (op, name, pr, NULL))

#define ASSERT_UNARYOP_OK_OR_NULL(op,name,pr)  \
    ASSERT_OK_OR_NULL (GB_UnaryOp_check (op, name, pr, NULL))

#define ASSERT_SELECTOP_OK(op,name,pr)  \
    ASSERT_OK (GB_SelectOp_check (op, name, pr, NULL))

#define ASSERT_SELECTOP_OK_OR_NULL(op,name,pr)  \
    ASSERT_OK_OR_NULL (GB_SelectOp_check (op, name, pr, NULL))

#define ASSERT_MONOID_OK(mon,name,pr)  \
    ASSERT_OK (GB_Monoid_check (mon, name, pr, NULL))

#define ASSERT_SEMIRING_OK(s,name,pr)  \
    ASSERT_OK (GB_Semiring_check (s, name, pr, NULL))

#define ASSERT_MATRIX_OK(A,name,pr)  \
    ASSERT_OK (GB_Matrix_check (A, name, pr, NULL))

#define ASSERT_MATRIX_OK_OR_NULL(A,name,pr)  \
    ASSERT_OK_OR_NULL (GB_Matrix_check (A, name, pr, NULL))

#define ASSERT_VECTOR_OK(v,name,pr)  \
    ASSERT_OK (GB_Vector_check (v, name, pr, NULL))

#define ASSERT_VECTOR_OK_OR_NULL(v,name,pr)  \
    ASSERT_OK_OR_NULL (GB_Vector_check (v, name, pr, NULL))

#define ASSERT_SCALAR_OK(s,name,pr)  \
    ASSERT_OK (GB_Scalar_check (s, name, pr, NULL))

#define ASSERT_SCALAR_OK_OR_NULL(s,name,pr)  \
    ASSERT_OK_OR_NULL (GB_Scalar_check (s, name, pr, NULL))

#define ASSERT_DESCRIPTOR_OK(d,name,pr)  \
    ASSERT_OK (GB_Descriptor_check (d, name, pr, NULL))

#define ASSERT_DESCRIPTOR_OK_OR_NULL(d,name,pr)  \
    ASSERT_OK_OR_NULL (GB_Descriptor_check (d, name, pr, NULL))

#endif

