//------------------------------------------------------------------------------
// GrB_error: return an error string describing the last error
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB_thread_local.h"

// if dynamic allocation of memory for POSIX threads fails, use this string:
const char panic [GB_RLEN+1] = "GraphBLAS error: GrB_PANIC\n"
    "Out of memory for thread-local storage\n" ;

const char *GrB_error ( )       // return a string describing the last error
{ 
    char *p = GB_thread_local_get ( ) ;
    return (p == NULL ? panic : p) ;
}

