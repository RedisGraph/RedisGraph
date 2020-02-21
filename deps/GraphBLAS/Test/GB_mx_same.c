//------------------------------------------------------------------------------
// GB_mx_same: check if two arrays are equal
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB_mex.h"

bool GB_mx_same     // true if arrays X and Y are the same
(
    char *X,
    char *Y,
    int64_t len     // length of X and Y
)
{
    if (X == Y) return (true) ;
    if (X == NULL) return (false) ;
    if (Y == NULL) return (false) ;
    for (int64_t i = 0 ; i < len ; i++)
    {
        if (X [i] != Y [i]) return (false) ; 
    }
    return (true) ;
}

