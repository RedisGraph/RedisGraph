//------------------------------------------------------------------------------
// GB_check.h: check and optionally print an object
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#ifndef GB_CHECK_H
#define GB_CHECK_H

// pr values for *_check functions
#define GB0 GxB_SILENT
#define GB1 GxB_SUMMARY
#define GB2 GxB_SHORT
#define GB3 GxB_COMPLETE
#define GB4 GxB_SHORT_VERBOSE
#define GB5 GxB_COMPLETE_VERBOSE

// a NULL name is treated as the empty string
#define GB_NAME ((name != NULL) ? name : "")

GB_PUBLIC   // accessed by the MATLAB tests in GraphBLAS/Test only
GrB_Info GB_entry_check     // print a single value
(
    const GrB_Type type,    // type of value to print
    const void *x,          // value to print
    int pr,                 // print level
    FILE *f                 // file to print to
) ;

GB_PUBLIC   // accessed by the MATLAB tests in GraphBLAS/Test only
GrB_Info GB_code_check          // print and check an entry using a type code
(
    const GB_Type_code code,    // type code of value to print
    const void *x,              // entry to print
    int pr,                     // print level
    FILE *f                     // file to print to
) ;

GB_PUBLIC   // accessed by the MATLAB tests in GraphBLAS/Test only
GrB_Info GB_Type_check      // check a GraphBLAS Type
(
    const GrB_Type type,    // GraphBLAS type to print and check
    const char *name,       // name of the type from the caller; optional
    int pr,                 // print level
    FILE *f                 // file for output
) ;

GB_PUBLIC   // accessed by the MATLAB tests in GraphBLAS/Test only
GrB_Info GB_BinaryOp_check  // check a GraphBLAS binary operator
(
    const GrB_BinaryOp op,  // GraphBLAS operator to print and check
    const char *name,       // name of the operator
    int pr,                 // print level
    FILE *f                 // file for output
) ;

GB_PUBLIC   // accessed by the MATLAB tests in GraphBLAS/Test only
GrB_Info GB_UnaryOp_check   // check a GraphBLAS unary operator
(
    const GrB_UnaryOp op,   // GraphBLAS operator to print and check
    const char *name,       // name of the operator
    int pr,                 // print level
    FILE *f                 // file for output
) ;

GB_PUBLIC   // accessed by the MATLAB tests in GraphBLAS/Test only
GrB_Info GB_SelectOp_check  // check a GraphBLAS select operator
(
    const GxB_SelectOp op,  // GraphBLAS operator to print and check
    const char *name,       // name of the operator
    int pr,                 // print level
    FILE *f                 // file for output
) ;

GB_PUBLIC   // accessed by the MATLAB tests in GraphBLAS/Test only
GrB_Info GB_Monoid_check        // check a GraphBLAS monoid
(
    const GrB_Monoid monoid,    // GraphBLAS monoid to print and check
    const char *name,           // name of the monoid, optional
    int pr,                     // print level
    FILE *f                     // file for output
) ;

GB_PUBLIC   // accessed by the MATLAB tests in GraphBLAS/Test only
GrB_Info GB_Semiring_check          // check a GraphBLAS semiring
(
    const GrB_Semiring semiring,    // GraphBLAS semiring to print and check
    const char *name,               // name of the semiring, optional
    int pr,                         // print level
    FILE *f                         // file for output
) ;

GB_PUBLIC   // accessed by the MATLAB tests in GraphBLAS/Test only
GrB_Info GB_Descriptor_check    // check a GraphBLAS descriptor
(
    const GrB_Descriptor D,     // GraphBLAS descriptor to print and check
    const char *name,           // name of the descriptor, optional
    int pr,                     // print level
    FILE *f                     // file for output
) ;

GB_PUBLIC   // accessed by the MATLAB tests in GraphBLAS/Test only
GrB_Info GB_matvec_check    // check a GraphBLAS matrix or vector
(
    const GrB_Matrix A,     // GraphBLAS matrix to print and check
    const char *name,       // name of the matrix, optional
    int pr,                 // print level; if negative, ignore nzombie
                            // conditions and use GB_FLIP(pr) for diagnostics
    FILE *f,                // file for output
    const char *kind        // "matrix" or "vector"
) ;

GB_PUBLIC   // accessed by the MATLAB tests in GraphBLAS/Test only
GrB_Info GB_Matrix_check    // check a GraphBLAS matrix
(
    const GrB_Matrix A,     // GraphBLAS matrix to print and check
    const char *name,       // name of the matrix
    int pr,                 // print level
    FILE *f                 // file for output
) ;

GB_PUBLIC   // accessed by the MATLAB tests in GraphBLAS/Test only
GrB_Info GB_Vector_check    // check a GraphBLAS vector
(
    const GrB_Vector v,     // GraphBLAS vector to print and check
    const char *name,       // name of the vector
    int pr,                 // print level
    FILE *f                 // file for output
) ;

GrB_Info GB_Scalar_check    // check a GraphBLAS GxB_Scalar
(
    const GxB_Scalar v,     // GraphBLAS GxB_Scalar to print and check
    const char *name,       // name of the GxB_Scalar
    int pr,                 // print level
    FILE *f                 // file for output
) ;

#endif

