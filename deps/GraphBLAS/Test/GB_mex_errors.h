//------------------------------------------------------------------------------
// GB_mex_errors.h: error handling macros
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#define FAIL(s)                                                             \
{                                                                           \
    fprintf (f,"\nTest failure: %s line %d\n", __FILE__, __LINE__) ;        \
    fprintf (f, "%s\n", GB_STR(s)) ;                                        \
    fclose (f) ;                                                            \
    mexErrMsgTxt (GB_STR(s) " line: " GB_XSTR(__LINE__)) ;                  \
}

#undef CHECK
#define CHECK(x)    if (!(x)) FAIL(x) ;
#define CHECK2(x,s) if (!(x)) FAIL(s) ;

// assert that a method should return a particular error code
#define ERR(method)                                                         \
{                                                                           \
    info = method ;                                                         \
    fprintf (f, "line %d: info %d\n", __LINE__, info) ;                     \
    if (info != expected) fprintf (f, "got %d expected %d\n",               \
        info, expected) ;                                                   \
    CHECK2 (info == expected, method) ;                                     \
}

// assert that a method should return a particular error code: with logger
#define ERR1(C,method)                                                      \
{                                                                           \
    info = method ;                                                         \
    fprintf (f, "\nline %d: info %d, error logger:\n", __LINE__, info) ;    \
    char *error_logger ;                                                    \
    GrB_Matrix_error_(&error_logger, ((GrB_Matrix) C)) ;                    \
    fprintf (f,"[%s]\n", error_logger) ;                                    \
    if (info != expected) fprintf (f, "got %d expected %d\n",               \
        info, expected) ;                                                   \
    CHECK2 (info == expected, method) ;                                     \
}

// assert that a method should succeed
#define OK(method)                                                          \
{                                                                           \
    info = method ;                                                         \
    if (! (info == GrB_SUCCESS || info == GrB_NO_VALUE))                    \
    {                                                                       \
        fprintf (f,"[%d] >>>>>>>>\n", info) ;                               \
        mexPrintf ("[%d] Test failed\n", info) ;                            \
        FAIL (method) ;                                                     \
    }                                                                       \
}

