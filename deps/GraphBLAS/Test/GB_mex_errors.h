//------------------------------------------------------------------------------
// GB_mex_errors.h: error handling macros
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#define FAIL(s)                                                     \
{                                                                   \
    fprintf (f,"\ntest failure: line %d\n", __LINE__) ;             \
    fprintf (f,"%s\n", GB_STR(s)) ;                                 \
    fclose (f) ;                                                    \
    mexErrMsgTxt (GB_STR(s) " line: " GB_XSTR(__LINE__)) ;          \
}

#undef CHECK
#define CHECK(x)    if (!(x)) FAIL(x) ;
#define CHECK2(x,s) if (!(x)) FAIL(s) ;

// assert that a method should return a particular error code
#define ERR(method)                                                 \
{                                                                   \
    info = method ;                                                 \
    fprintf (f,"GB_mex_errors, line %d:", __LINE__) ;               \
    fprintf (f,"%s\n", GrB_error ( )) ;                             \
    CHECK2 (info == expected, method) ;                             \
}

// assert that a method should succeed
#define OK(method)                                                  \
{                                                                   \
    info = method ;                                                 \
    if (! (info == GrB_SUCCESS || info == GrB_NO_VALUE))            \
    {                                                               \
        fprintf (f,"[%d] >>>>>>>>%s\n", info, GrB_error ( )) ;      \
        printf ("[%d] %s\n", info, GrB_error ( )) ;                 \
        FAIL (method) ;                                             \
    }                                                               \
}

