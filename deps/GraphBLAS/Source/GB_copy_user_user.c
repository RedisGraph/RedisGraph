//------------------------------------------------------------------------------
// GB_copy_user_user.c: copy user a type
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB.h"

void GB_copy_user_user (void *z, const void *x, size_t s)
{ 
    memcpy (z, x, s) ;
}

