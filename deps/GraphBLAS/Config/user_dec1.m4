//==============================================================================
// user-defined objects defined by SuiteSparse/GraphBLAS/User/*.m4
//==============================================================================

// Declarations appended to SuiteSparse/GraphBLAS/Include/GraphBLAS.h.

#if defined __INTEL_COMPILER
#pragma warning (disable: 869 )
#elif defined __GNUC__
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif

#ifndef GxB_USER_INCLUDE
#define GxB_USER_INCLUDE
#endif

#ifndef GxB_USER_H
#define GxB_USER_H
m4_define(`GxB_Type_define',            `extern GrB_Type $1')
m4_define(`GxB_UnaryOp_define',         `extern GrB_UnaryOp $1')
m4_define(`GxB_BinaryOp_define',        `extern GrB_BinaryOp $1')
m4_define(`GxB_SelectOp_define',        `extern GxB_SelectOp $1') 
m4_define(`GxB_Monoid_define',          `extern GrB_Monoid $1')
m4_define(`GxB_Monoid_terminal_define', `extern GrB_Monoid $1')
m4_define(`GxB_Semiring_define',        `extern GrB_Semiring $1')
